#include <basic/operators/pipe/join/physical/join_operator.h>

#include <core/physical/segment/segment_builder.h>

#include "basic/operators/pipe/join/runtime/join_runtime.h"

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical {
    namespace {
        bool _shouldExtract(const details::JoinOperatorCard card, const bool onLeft) {
            switch (card) {
                case details::JoinOperatorCard::ONE_ONE: return true;
                case details::JoinOperatorCard::MANY_ONE: return !onLeft;
                case details::JoinOperatorCard::ONE_MANY: return onLeft;
                default: return false;
            }
        }

        bool _shouldInsert(const details::JoinOperatorCard card, const bool onLeft, const bool hit) {
            switch (card) {
                case details::JoinOperatorCard::ONE_ONE: return !hit;
                case details::JoinOperatorCard::ONE_MANY: return onLeft || !hit;
                case details::JoinOperatorCard::MANY_ONE: return !onLeft || !hit;
                default: return true;
            }
        }
    }

    JoinOperator::JoinOperator(core::physical::port::Ports inputPorts,
                               core::physical::port::Port outputPort,
                               const details::JoinOperatorOptions config,
                               core::logical::port::Ports schemas,
                               std::unique_ptr<basic::expression::physical::IExpression> predicate,
                               const bool outOfOrder)
        : TypedPipeOperator(std::move(inputPorts),
                            core::physical::port::Ports{std::move(outputPort)}), mOutOfOrder(outOfOrder),
          mConfig(config),
          mLeftSchema(std::move(schemas.at(0))),
          mRightSchema(std::move(schemas.at(1))),
          mPredicate(std::move(predicate)),
          mEquiKeys(tryGetEquiKeyList(mPredicate.get(), mLeftSchema, mRightSchema)),
          mIsEquiJoin(mEquiKeys.has_value()) {
        mRuntimeTypeRegistry = core::runtime::types::RuntimeTypeRegistry::withBuiltins();
    }

    void JoinOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                               const core::shared::identifiers::PortId inputPort,
                               const core::physical::data::DataPackage& tuple,
                               const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();

        auto *voidTy = llvm::Type::getVoidTy(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        auto *i1Ty = llvm::Type::getInt1Ty(ctx);
        auto *sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        const bool onLeft = (compactInputIndex(inputPort) == 0);
        const auto& schema = onLeft ? mLeftSchema : mRightSchema;

        //Lock
        const auto lockFn = module->getOrInsertFunction(
            "rtJoinLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(lockFn, {state});

        //Begin and Push
        const auto beginFn = module->getOrInsertFunction(
                    onLeft ? "rtJoinBeginLeft" : "rtJoinBeginRight",
                    llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(beginFn, {state});

        const auto pushFn = module->getOrInsertFunction(
            onLeft ? "rtJoinPushLeft" : "rtJoinPushRight",
            llvm::FunctionType::get(voidTy, {ptrTy, sizeTTy, ptrTy}, false));
        for (std::size_t i = 0; i < schema.size(); ++i) {
            auto *slot = segmentCtx.storage.getNamed("join_slot." +std::to_string(getPId()), schema.at(i).valueTypeId);
            segmentCtx.builder.CreateStore(tuple.at(i), slot);
            segmentCtx.builder.CreateCall(pushFn, {state, llvm::ConstantInt::get(sizeTTy, i), slot});
        }

        //Match (fills Candidates-List!)
        const auto matchFn = module->getOrInsertFunction(
            onLeft ? "rtJoinMatchLeft" : "rtJoinMatchRight",
            llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i64Ty, i64Ty, i64Ty}, false));
        segmentCtx.builder.CreateCall(matchFn, {state,
            tuple.timeInterval.getStart().getPoint(),
            tuple.timeInterval.getEnd().getPoint(),
            tuple.timeInterval.getStart().getTiBreaker(),
            tuple.timeInterval.getEnd().getTiBreaker()});

        //Snapshot (no extraction is executed here!)
        const bool extract = _shouldExtract(mConfig.card, onLeft);
        const auto snapshotFn = module->getOrInsertFunction("rtJoinSnapshotForCompute",
            llvm::FunctionType::get(ptrTy, {ptrTy, i8Ty, i8Ty}, false));
        llvm::Value* snapshot = segmentCtx.builder.CreateCall(snapshotFn, {state,
            llvm::ConstantInt::get(i8Ty, onLeft ? 1 : 0),
            llvm::ConstantInt::get(i8Ty, extract ? 1 : 0)});

        //Check predicate and mark
        llvm::Value* hasMatches = nullptr;
        if (mIsEquiJoin) {

            const auto markAllFn = module->getOrInsertFunction("rtJoinMarkAllCandidatesAsMatch",
                        llvm::FunctionType::get(voidTy, {ptrTy, ptrTy, i8Ty}, false));
            segmentCtx.builder.CreateCall(markAllFn, {state, snapshot, llvm::ConstantInt::get(i8Ty, extract ? 1 : 0)});

            const auto countFn = module->getOrInsertFunction("rtJoinSnapshotGetCount",
                        llvm::FunctionType::get(sizeTTy, {ptrTy}, false));
            llvm::Value* count = segmentCtx.builder.CreateCall(countFn, {snapshot});
            hasMatches = segmentCtx.builder.CreateICmpSGT(count, llvm::ConstantInt::get(sizeTTy, 0));

        } else {
            evaluatePredicateAndMark(segmentCtx, snapshot, state, onLeft, extract);

            auto* predicateHitSlot = segmentCtx.storage.getNamed("predicateJoinHitSlot." + std::to_string(getPId()), sizeTTy);
            llvm::Value* predicateHitCount = segmentCtx.builder.CreateLoad(sizeTTy, predicateHitSlot);
            hasMatches = segmentCtx.builder.CreateICmpSGT(predicateHitCount, llvm::ConstantInt::get(sizeTTy, 0));
        }

        //If_Insert like in Odysseus
        const bool insertAlways = _shouldInsert(mConfig.card, onLeft, true);
        const bool insertNever = _shouldInsert(mConfig.card, onLeft, false);

        auto* insertBB = llvm::BasicBlock::Create(ctx, "join_insert", fn);
        auto* removeBB = llvm::BasicBlock::Create(ctx, "join_remove", fn);
        auto* unlockBB = llvm::BasicBlock::Create(ctx, "join_unlock", fn);
        auto* mergeBB = llvm::BasicBlock::Create(ctx, "join_merge", fn);

        if (insertAlways && insertNever) {
            //many_many, one_many-left, many_one-right
            segmentCtx.builder.CreateBr(insertBB);
        } else {
            //one_one, one_many-right, many_one-left
            segmentCtx.builder.CreateCondBr(hasMatches, removeBB, insertBB);
        }

        segmentCtx.builder.SetInsertPoint(insertBB);

        //End (commit current tuple)
        const auto endFn = module->getOrInsertFunction(
            onLeft ? "rtJoinEndLeft" : "rtJoinEndRight",
            llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i64Ty, i64Ty, i64Ty}, false));
        segmentCtx.builder.CreateCall(endFn,
            {
                state,
                tuple.timeInterval.getStart().getPoint(),
                tuple.timeInterval.getEnd().getPoint(),
                tuple.timeInterval.getStart().getTiBreaker(),
                tuple.timeInterval.getEnd().getTiBreaker()
            });

        segmentCtx.builder.CreateBr(removeBB);

        segmentCtx.builder.SetInsertPoint(removeBB);

        //Remove marked
        if (extract) {
            const auto removeFn = module->getOrInsertFunction("rtJoinRemoveMarkedCandidates", llvm::FunctionType::get(voidTy, {ptrTy, ptrTy}, false));
            segmentCtx.builder.CreateCall(removeFn, {state, snapshot});
        }

        //Heartbeat
        codegenCreateHeartbeat(segmentCtx, state, consumers);

        segmentCtx.builder.CreateBr(unlockBB);

        segmentCtx.builder.SetInsertPoint(unlockBB);

        //Unlock
        const auto unlockFn = module->getOrInsertFunction("rtJoinUnlock",llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(unlockFn, {state});

        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);

        //Emit
        emitMatchesFromSnapshot(segmentCtx, snapshot, onLeft, consumers);

        //Release snapshot
        const auto releaseFn = module->getOrInsertFunction(
            "rtJoinSnapshotRelease", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(releaseFn, {snapshot});
    }

    void JoinOperator::codegenCreateHeartbeat(core::physical::segment::SegmentContext &segmentCtx,
                                               llvm::Value *joinState,
                                               const core::physical::segment::SegmentDataConsumers &consumers) const {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        const auto getMinLeftFn = module->getOrInsertFunction("rtJoinGetMinStartTimestampLeft",
                                                              llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto getMinRightFn = module->getOrInsertFunction("rtJoinGetMinStartTimestampRight",
                                                               llvm::FunctionType::get(i64Ty, {ptrTy}, false));

        llvm::Value* minLeft = segmentCtx.builder.CreateCall(getMinLeftFn, {joinState});
        llvm::Value* minRight = segmentCtx.builder.CreateCall(getMinRightFn, {joinState});

        llvm::Value* heartbeat = segmentCtx.builder.CreateICmpSGT(minLeft, minRight);
        heartbeat = segmentCtx.builder.CreateSelect(heartbeat, minLeft, minRight);

        llvm::Value *lastHb = segmentCtx.builder.CreateLoad(i64Ty, mLastHeartbeatState.var);

        llvm::Value *shouldEmit = segmentCtx.builder.CreateICmpSGT(heartbeat, lastHb);

        auto *emitBB = llvm::BasicBlock::Create(ctx, "join_emit_heartbeat",
                                                segmentCtx.builder.GetInsertBlock()->getParent());
        auto *mergeBB = llvm::BasicBlock::Create(ctx, "join_merge_heartbeat",
                                                 segmentCtx.builder.GetInsertBlock()->getParent());

        segmentCtx.builder.CreateCondBr(shouldEmit, emitBB, mergeBB);

        segmentCtx.builder.SetInsertPoint(emitBB);

        segmentCtx.builder.CreateStore(heartbeat, mLastHeartbeatState.var);

        const auto watermark = core::physical::data::PunctuationPackage::createHeartbeat(segmentCtx.builder, heartbeat);

        consumers.front().consumePunctuation(segmentCtx, watermark);

        segmentCtx.builder.CreateBr(mergeBB);

        segmentCtx.builder.SetInsertPoint(mergeBB);
    }

    // NOLINTNEXTLINE(readability-function-size)
    void JoinOperator::evaluatePredicateAndMark(core::physical::segment::SegmentContext& segmentCtx,
            llvm::Value* snapshot, llvm::Value* state, bool onLeft, bool extract) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto *sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        auto* predicateHitSlot = segmentCtx.storage.getNamed("predicateJoinHitSlot." + std::to_string(getPId()), sizeTTy);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(sizeTTy, 0), predicateHitSlot);

        auto* indexSlot = segmentCtx.storage.getNamed(
                    "computeIndexSlot." + std::to_string(getPId()), sizeTTy);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(sizeTTy, 0), indexSlot);

        const auto countFn = module->getOrInsertFunction(
            "rtJoinSnapshotGetCount", llvm::FunctionType::get(sizeTTy, {ptrTy}, false));
        llvm::Value* count = segmentCtx.builder.CreateCall(countFn, {snapshot});

        const auto& ownSchema = onLeft ? mLeftSchema : mRightSchema;
        const auto getOwnFn = module->getOrInsertFunction(
        "rtJoinSnapshotGetOwnPtr", llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy}, false));
        const auto getCandFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidatePtr",
            llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy, sizeTTy}, false));

        const auto getOwnStartTsFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnStartTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto getOwnEndTsFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnEndTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto getOwnStartTbFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnStartTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto getOwnEndTbFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnEndTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        llvm::Value* ownStartTs = segmentCtx.builder.CreateCall(getOwnStartTsFn, {snapshot});
        llvm::Value* ownEndTs = segmentCtx.builder.CreateCall(getOwnEndTsFn, {snapshot});
        llvm::Value* ownStartTb = segmentCtx.builder.CreateCall(getOwnStartTbFn, {snapshot});
        llvm::Value* ownEndTb = segmentCtx.builder.CreateCall(getOwnEndTbFn, {snapshot});

        const auto getCandidateStartTsFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateStartTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getCandidateEndTsFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateEndTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getCandidateStartTbFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateStartTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getCandidateEndTbFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateEndTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));

        auto* headerBB = llvm::BasicBlock::Create(ctx, "predicate_header", fn);
        auto* bodyBB = llvm::BasicBlock::Create(ctx, "predicate_body", fn);
        auto* exitBB = llvm::BasicBlock::Create(ctx, "predicate_exit", fn);

        segmentCtx.builder.CreateBr(headerBB);
        segmentCtx.builder.SetInsertPoint(headerBB);

        llvm::Value* index = segmentCtx.builder.CreateLoad(sizeTTy, indexSlot);

        segmentCtx.builder.CreateCondBr(
            segmentCtx.builder.CreateICmpSLT(index, count), bodyBB, exitBB);

        segmentCtx.builder.SetInsertPoint(bodyBB);

        const auto& otherSchema = onLeft ? mRightSchema : mLeftSchema;

        llvm::Value* candidateStartTs = segmentCtx.builder.CreateCall(getCandidateStartTsFn, {snapshot, index});
        llvm::Value* candidateEndTs = segmentCtx.builder.CreateCall(getCandidateEndTsFn, {snapshot, index});
        llvm::Value* candidateStartTb = segmentCtx.builder.CreateCall(getCandidateStartTbFn, {snapshot, index});
        llvm::Value* candidateEndTb = segmentCtx.builder.CreateCall(getCandidateEndTbFn, {snapshot, index});

        const core::physical::time::TimePoint ownStartTp(ownStartTs, ownStartTb);
        const core::physical::time::TimePoint otherStartTp(candidateStartTs, candidateStartTb);
        const core::physical::time::TimePoint ownEndTp(ownEndTs, ownEndTb);
        const core::physical::time::TimePoint otherEndTp(candidateEndTs, candidateEndTb);

        llvm::Value* ownStartIsLater = otherStartTp.before(segmentCtx.builder, ownStartTp);
        llvm::Value* resultStartTs = segmentCtx.builder.CreateSelect(ownStartIsLater, ownStartTs, candidateStartTs);
        llvm::Value* resultStartTb = segmentCtx.builder.CreateSelect(ownStartIsLater, ownStartTb, candidateStartTb);
        llvm::Value* ownEndIsEarlier = ownEndTp.before(segmentCtx.builder, otherEndTp);
        llvm::Value* resultEndTs = segmentCtx.builder.CreateSelect(ownEndIsEarlier, ownEndTs, candidateEndTs);
        llvm::Value* resultEndTb = segmentCtx.builder.CreateSelect(ownEndIsEarlier, ownEndTb, candidateEndTb);

        core::physical::data::DataPackage combined(core::physical::time::TimeInterval(segmentCtx.builder,
            core::physical::time::TimePoint(resultStartTs, resultStartTb),
            core::physical::time::TimePoint(resultEndTs, resultEndTb)));

        combined.reserve(ownSchema.size() + otherSchema.size());

        if (onLeft) {
            for (std::size_t i = 0; i < ownSchema.size(); ++i) {
                llvm::Value* fieldPtr = segmentCtx.builder.CreateCall(
                    getOwnFn, {snapshot, llvm::ConstantInt::get(sizeTTy, i)});
                llvm::Type* fieldTy =
                    segmentCtx.codegenTypeRegistry.resolveType(ownSchema.at(i).valueTypeId, ctx);
                combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
            }

            for (std::size_t i = 0; i < otherSchema.size(); ++i) {
                llvm::Value* fieldPtr = segmentCtx.builder.CreateCall(
                    getCandFn, {snapshot, index, llvm::ConstantInt::get(sizeTTy, i)});
                llvm::Type* fieldTy =
                    segmentCtx.codegenTypeRegistry.resolveType(otherSchema.at(i).valueTypeId, ctx);
                combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
            }
        } else {
            for (std::size_t i = 0; i < otherSchema.size(); ++i) {
                llvm::Value* fieldPtr = segmentCtx.builder.CreateCall(
                    getCandFn, {snapshot, index, llvm::ConstantInt::get(sizeTTy, i)});
                llvm::Type* fieldTy =
                    segmentCtx.codegenTypeRegistry.resolveType(otherSchema.at(i).valueTypeId, ctx);
                combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
            }

            for (std::size_t i = 0; i < ownSchema.size(); ++i) {
                llvm::Value* fieldPtr = segmentCtx.builder.CreateCall(
                    getOwnFn, {snapshot, llvm::ConstantInt::get(sizeTTy, i)});
                llvm::Type* fieldTy =
                    segmentCtx.codegenTypeRegistry.resolveType(ownSchema.at(i).valueTypeId, ctx);
                combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
            }
        }

        const auto isEqui = mEquiKeys.has_value();

        const auto markMatchFn = module->getOrInsertFunction("rtJoinMarkCandidateAsMatch", llvm::FunctionType::get(voidTy, {ptrTy, ptrTy, sizeTTy}, false));

        if (mPredicate != nullptr && !isEqui) {
            llvm::Value* predicateResult = expression::physical::codegenExpression(segmentCtx, combined, *mPredicate);

            auto* passBB = llvm::BasicBlock::Create(ctx, "predicate_pass", fn);
            auto* failBB = llvm::BasicBlock::Create(ctx, "predicate_fail", fn);
            auto* mergeBB = llvm::BasicBlock::Create(ctx, "predicate_merge", fn);

            segmentCtx.builder.CreateCondBr(predicateResult, passBB, failBB);

            segmentCtx.builder.SetInsertPoint(passBB);

            segmentCtx.builder.CreateCall(markMatchFn, {state, snapshot, index});

            if (extract) {
                const auto markRemoveFn = module->getOrInsertFunction("rtJoinMarkCandidateForRemove", llvm::FunctionType::get(voidTy, {ptrTy, ptrTy, sizeTTy}, false));
                segmentCtx.builder.CreateCall(markRemoveFn, {state, snapshot, index});
            }

            llvm::Value* currentHitCount = segmentCtx.builder.CreateLoad(sizeTTy, predicateHitSlot);
            llvm::Value* newHitCount = segmentCtx.builder.CreateAdd(currentHitCount, llvm::ConstantInt::get(sizeTTy, 1));
            segmentCtx.builder.CreateStore(newHitCount, predicateHitSlot);

            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(failBB);
            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(mergeBB);
        } else {
            segmentCtx.builder.CreateCall(markMatchFn, {state, snapshot, index});

            if (extract) {
                const auto markRemoveFn = module->getOrInsertFunction("rtJoinMarkCandidateForRemove", llvm::FunctionType::get(voidTy, {ptrTy, ptrTy, sizeTTy}, false));
                segmentCtx.builder.CreateCall(markRemoveFn, {state, snapshot, index});
            }

            llvm::Value* currentHitCount = segmentCtx.builder.CreateLoad(sizeTTy, predicateHitSlot);
            llvm::Value* newHitCount = segmentCtx.builder.CreateAdd(currentHitCount, llvm::ConstantInt::get(sizeTTy, 1));
            segmentCtx.builder.CreateStore(newHitCount, predicateHitSlot);
        }

        llvm::Value* next = segmentCtx.builder.CreateAdd(index, llvm::ConstantInt::get(sizeTTy, 1));

        segmentCtx.builder.CreateStore(next, indexSlot);
        segmentCtx.builder.CreateBr(headerBB);

        segmentCtx.builder.SetInsertPoint(exitBB);
    }

    // NOLINTNEXTLINE(readability-function-size)
    void JoinOperator::emitMatchesFromSnapshot(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* snapshot,
        const bool onLeft, const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        auto *voidTy = llvm::Type::getVoidTy(ctx);
        auto *sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        const auto getMatchCountFn = module->getOrInsertFunction("rtJoinSnapshotGetMatchCount", llvm::FunctionType::get(sizeTTy, {ptrTy}, false));
        llvm::Value* matchCount = segmentCtx.builder.CreateCall(getMatchCountFn, {snapshot});

        const auto getMatchIndexFn = module->getOrInsertFunction("rtJoinSnapshotGetMatchIndex", llvm::FunctionType::get(sizeTTy, {ptrTy, sizeTTy}, false));

        const auto& ownSchema = onLeft ? mLeftSchema : mRightSchema;

        const auto getOwnFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnPtr", llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy}, false));

        const auto getCandidateFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidatePtr", llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy, sizeTTy}, false));

        const auto getOwnStartTsFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnStartTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto getOwnEndTsFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnEndTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto getOwnStartTbFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnStartTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        const auto getOwnEndTbFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnEndTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        llvm::Value* ownStartTs = segmentCtx.builder.CreateCall(getOwnStartTsFn, {snapshot});
        llvm::Value* ownEndTs = segmentCtx.builder.CreateCall(getOwnEndTsFn, {snapshot});
        llvm::Value* ownStartTb = segmentCtx.builder.CreateCall(getOwnStartTbFn, {snapshot});
        llvm::Value* ownEndTb = segmentCtx.builder.CreateCall(getOwnEndTbFn, {snapshot});

        const auto getCandidateStartTsFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateStartTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getCandidateEndTsFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateEndTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getCandidateStartTbFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateStartTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getCandidateEndTbFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateEndTibreaker", llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));

        auto* indexSlot = segmentCtx.storage.getNamed(
                    "emitIndexSlot." + std::to_string(getPId()), sizeTTy);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(sizeTTy, 0), indexSlot);

        auto* headerBB = llvm::BasicBlock::Create(ctx, "emit_match_header", fn);
        auto* bodyBB = llvm::BasicBlock::Create(ctx, "emit_match_body", fn);
        auto* exitBB = llvm::BasicBlock::Create(ctx, "emit_match_exit", fn);

        segmentCtx.builder.CreateBr(headerBB);
        segmentCtx.builder.SetInsertPoint(headerBB);

        llvm::Value* index = segmentCtx.builder.CreateLoad(sizeTTy, indexSlot);

        segmentCtx.builder.CreateCondBr(
            segmentCtx.builder.CreateICmpSLT(index, matchCount), bodyBB, exitBB);

        segmentCtx.builder.SetInsertPoint(bodyBB);

        const auto& otherSchema = onLeft ? mRightSchema : mLeftSchema;

        llvm::Value* candidateIndex = segmentCtx.builder.CreateCall(getMatchIndexFn, {snapshot, index});

        llvm::Value* candidateStartTs = segmentCtx.builder.CreateCall(getCandidateStartTsFn, {snapshot, candidateIndex});
        llvm::Value* candidateEndTs = segmentCtx.builder.CreateCall(getCandidateEndTsFn, {snapshot, candidateIndex});
        llvm::Value* candidateStartTb = segmentCtx.builder.CreateCall(getCandidateStartTbFn, {snapshot, candidateIndex});
        llvm::Value* candidateEndTb = segmentCtx.builder.CreateCall(getCandidateEndTbFn, {snapshot, candidateIndex});

        const core::physical::time::TimePoint ownStartTp(ownStartTs, ownStartTb);
        const core::physical::time::TimePoint otherStartTp(candidateStartTs, candidateStartTb);
        const core::physical::time::TimePoint ownEndTp(ownEndTs, ownEndTb);
        const core::physical::time::TimePoint otherEndTp(candidateEndTs, candidateEndTb);

        llvm::Value* ownStartIsLater = otherStartTp.before(segmentCtx.builder, ownStartTp);
        llvm::Value* resultStartTs = segmentCtx.builder.CreateSelect(ownStartIsLater, ownStartTs, candidateStartTs);
        llvm::Value* resultStartTb = segmentCtx.builder.CreateSelect(ownStartIsLater, ownStartTb, candidateStartTb);
        llvm::Value* ownEndIsEarlier = ownEndTp.before(segmentCtx.builder, otherEndTp);
        llvm::Value* resultEndTs = segmentCtx.builder.CreateSelect(ownEndIsEarlier, ownEndTs, candidateEndTs);
        llvm::Value* resultEndTb = segmentCtx.builder.CreateSelect(ownEndIsEarlier, ownEndTb, candidateEndTb);

        if (!mOutOfOrder) {
            const auto getOwnRowPtrFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnRowPtr", llvm::FunctionType::get(ptrTy, {ptrTy}, false));
            const auto getOwnRowSizeFn = module->getOrInsertFunction("rtJoinSnapshotGetOwnRowSize", llvm::FunctionType::get(sizeTTy, {ptrTy}, false));
            const auto getCandRowPtrFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateRowPtr", llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy}, false));
            const auto getCandRowSizeFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateRowSize", llvm::FunctionType::get(sizeTTy, {ptrTy}, false));

            llvm::Value* ownRowPtr = segmentCtx.builder.CreateCall(getOwnRowPtrFn, {snapshot});
            llvm::Value* ownRowSize = segmentCtx.builder.CreateCall(getOwnRowSizeFn, {snapshot});
            llvm::Value* candRowPtr = segmentCtx.builder.CreateCall(getCandRowPtrFn, {snapshot, candidateIndex});
            llvm::Value* candRowSize = segmentCtx.builder.CreateCall(getCandRowSizeFn, {snapshot});

            const auto bufferOutputFn = module->getOrInsertFunction("rtJoinBufferOutput", llvm::FunctionType::get(
                                                                        voidTy,
                                                                        {
                                                                            ptrTy, ptrTy, sizeTTy, ptrTy, sizeTTy, i8Ty,
                                                                            i64Ty, i64Ty, i64Ty, i64Ty
                                                                        }, false));
            llvm::Value* state = loadStatePtr(segmentCtx);

            segmentCtx.builder.CreateCall(bufferOutputFn, {state, ownRowPtr, ownRowSize, candRowPtr, candRowSize,
                llvm::ConstantInt::get(i8Ty, onLeft ? 1 : 0), resultStartTs, resultEndTs, resultStartTb, resultEndTb});
        } else {
            core::physical::data::DataPackage combined(core::physical::time::TimeInterval(segmentCtx.builder,
                core::physical::time::TimePoint(resultStartTs, resultStartTb),
                core::physical::time::TimePoint(resultEndTs, resultEndTb)));

            combined.reserve(ownSchema.size() + otherSchema.size());

            if (onLeft) {
                for (std::size_t i = 0; i < ownSchema.size(); ++i) {
                    llvm::Value *fieldPtr = segmentCtx.builder.CreateCall(
                        getOwnFn, {snapshot, llvm::ConstantInt::get(sizeTTy, i)});
                    llvm::Type *fieldTy =
                            segmentCtx.codegenTypeRegistry.resolveType(ownSchema.at(i).valueTypeId, ctx);
                    combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
                }

                for (std::size_t i = 0; i < otherSchema.size(); ++i) {
                    llvm::Value *fieldPtr = segmentCtx.builder.CreateCall(
                        getCandidateFn, {snapshot, candidateIndex, llvm::ConstantInt::get(sizeTTy, i)});
                    llvm::Type *fieldTy =
                            segmentCtx.codegenTypeRegistry.resolveType(otherSchema.at(i).valueTypeId, ctx);
                    combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
                }
            } else {
                for (std::size_t i = 0; i < otherSchema.size(); ++i) {
                    llvm::Value *fieldPtr = segmentCtx.builder.CreateCall(
                        getCandidateFn, {snapshot, candidateIndex, llvm::ConstantInt::get(sizeTTy, i)});
                    llvm::Type *fieldTy =
                            segmentCtx.codegenTypeRegistry.resolveType(otherSchema.at(i).valueTypeId, ctx);
                    combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
                }

                for (std::size_t i = 0; i < ownSchema.size(); ++i) {
                    llvm::Value *fieldPtr = segmentCtx.builder.CreateCall(
                        getOwnFn, {snapshot, llvm::ConstantInt::get(sizeTTy, i)});
                    llvm::Type *fieldTy =
                            segmentCtx.codegenTypeRegistry.resolveType(ownSchema.at(i).valueTypeId, ctx);
                    combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
                }
            }

            consumers.front().consume(segmentCtx, combined);
        }

        llvm::Value* next = segmentCtx.builder.CreateAdd(index, llvm::ConstantInt::get(sizeTTy, 1));
        segmentCtx.builder.CreateStore(next, indexSlot);
        segmentCtx.builder.CreateBr(headerBB);

        segmentCtx.builder.SetInsertPoint(exitBB);
    }

    std::size_t JoinOperator::getTypeSize(const core::shared::types::ValueTypeId typeId) const {
        return mRuntimeTypeRegistry.byteSize(typeId);
    }

    std::size_t JoinOperator::getRowByteSize(const core::logical::port::Port& schema) const {
        const auto rtTypeReg = core::runtime::types::RuntimeTypeRegistry::withBuiltins();

        std::size_t size = 0;
        for (const auto& field : schema) {
            size += mRuntimeTypeRegistry.byteSize(field.valueTypeId);
        }
        return size;
    }

    void JoinOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                    llvm::Module& module,
                                    llvm::LLVMContext& context) {
        auto *i64Ty = llvm::Type::getInt64Ty(context);
        auto *i8Ty = llvm::Type::getInt8Ty(context);
        auto *ptrTy = llvm::PointerType::getUnqual(context);
        auto *sizeTTy = module.getDataLayout().getIntPtrType(context);

        auto* leftSideTypesGlobal = buildTypeIds(
            module, context, mLeftSchema, "join_left_side_types_" + std::to_string(getPId()));
        auto* rightSideTypesGlobal = buildTypeIds(
            module, context, mRightSchema, "join_right_side_types_" + std::to_string(getPId()));

        const auto isEqui = mEquiKeys.has_value();
        const auto& equiKeys = isEqui ? mEquiKeys->keys : std::vector<EquiKeyInfo>{};

        auto* leftEquiIndices = buildEquiFieldIndices(module, context, equiKeys,
            true, "join_left_equi_indices_" +std::to_string(getPId()));
        auto* rightEquiIndices = buildEquiFieldIndices(module, context, equiKeys,
            false, "join_right_equi_indices_" +std::to_string(getPId()));
        auto* equiTypeIds = buildEquiTypeIds(module, context, equiKeys, "join_equi_typeids_" + std::to_string(getPId()));

        mState = stateRegistry.createHandle(
            "join_" + std::to_string(getPId()),
            "rtJoinCreate",
            {leftSideTypesGlobal,
             llvm::ConstantInt::get(sizeTTy, mLeftSchema.size()),
             rightSideTypesGlobal,
             llvm::ConstantInt::get(sizeTTy, mRightSchema.size()),
             llvm::ConstantInt::get(i8Ty, isEqui ? 1 : 0),
             leftEquiIndices,
             rightEquiIndices,
             equiTypeIds,
             llvm::ConstantInt::get(sizeTTy, equiKeys.size()),
             llvm::ConstantInt::get(i8Ty, mOutOfOrder ? 1 : 0)},
            {ptrTy, sizeTTy, ptrTy, sizeTTy, i8Ty, ptrTy, ptrTy, ptrTy, sizeTTy, i8Ty},
            "rtJoinDestroy");

        mWatermarkState = stateRegistry.createHandle(
            "join_watermark_" + std::to_string(getPId()),
            "rtJoinWatermarkCreate", {}, {},
            "rtJoinWatermarkDestroy");

        mLastHeartbeatState = stateRegistry.createPrimitiveHandle(
            "join_last_heartbeat_" + std::to_string(getPId()),
            i64Ty, llvm::ConstantInt::get(i64Ty, 0));
    }

    llvm::Value*
    JoinOperator::loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
        auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
    }

    void JoinOperator::codegenEvict(core::physical::segment::SegmentContext& segmentCtx,
                                    llvm::Value* state,
                                    llvm::Value* timestamp,
                                    const bool callFromLeft, llvm::Value* isFinal) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);

        const auto evictFn = module->getOrInsertFunction(
            "rtJoinEvict", llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i8Ty, i8Ty}, false));
        segmentCtx.builder.CreateCall(
            evictFn, {state, timestamp, llvm::ConstantInt::get(i8Ty, callFromLeft ? 1 : 0), isFinal});
    }

    llvm::GlobalVariable* JoinOperator::buildTypeIds(llvm::Module& module,
                                                     llvm::LLVMContext& ctx,
                                                     const core::logical::port::Port& schema,
                                                     const std::string& name) {
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);
        std::vector<llvm::Constant*> values;
        values.reserve(schema.size());
        for (const auto& slot : schema) {
            values.push_back(llvm::ConstantInt::get(i8Ty, slot.valueTypeId));
        }
        auto* arrTy = llvm::ArrayType::get(i8Ty, values.size());
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        return new llvm::GlobalVariable(module, arrTy, true,
                                        llvm::GlobalValue::PrivateLinkage,
                                        llvm::ConstantArray::get(arrTy, values),name);
    }

    llvm::GlobalVariable* JoinOperator::buildEquiFieldIndices(llvm::Module& module,
                                                    llvm::LLVMContext& ctx,
                                                    const std::vector<EquiKeyInfo>& keys,
                                                    bool useLeftIndex,
                                                    const std::string& name) {
        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *sizeTTy = module.getDataLayout().getIntPtrType(ctx);

        std::vector<llvm::Constant*> values;
        values.reserve(keys.size());
        for (const auto& key : keys) {
            values.push_back(llvm::ConstantInt::get(sizeTTy, useLeftIndex ? key.leftFieldIndex : key.rightFieldIndex));
        }

        auto* arrTy = llvm::ArrayType::get(sizeTTy, values.size());

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        return new llvm::GlobalVariable(module, arrTy, true, llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantArray::get(arrTy, values), name);
    }

    llvm::GlobalVariable* JoinOperator::buildEquiTypeIds(llvm::Module& module, llvm::LLVMContext& ctx,
                                                const std::vector<EquiKeyInfo>& keys,
                                                const std::string& name) {
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);
        std::vector<llvm::Constant*> values;
        values.reserve(keys.size());
        for (const auto& key : keys) {
            values.push_back(llvm::ConstantInt::get(i8Ty, key.typeId));
        }

        auto* arrTy = llvm::ArrayType::get(i8Ty, values.size());

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        return new llvm::GlobalVariable(module, arrTy, true, llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantArray::get(arrTy, values), name);
    }

    void JoinOperator::consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                            core::shared::identifiers::PortId inputPort,
                            const core::physical::data::PunctuationPackage &punctuation,
                            const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
        llvm::Function *fn = segmentCtx.builder.GetInsertBlock()->getParent();

        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *voidTy = llvm::Type::getVoidTy(ctx);

        llvm::Value *watermarkStatePtr = segmentCtx.builder.CreateLoad(ptrTy, mWatermarkState.var);
        llvm::Value *joinState = loadStatePtr(segmentCtx);

        auto *mergeBB = llvm::BasicBlock::Create(ctx, "join_watermark_merge", fn);
        auto *emitBB = llvm::BasicBlock::Create(ctx, "join_emit_watermark", fn);
        auto *afterMergeBB = llvm::BasicBlock::Create(ctx, "join_after_merge", fn);
        auto *returnBB = llvm::BasicBlock::Create(ctx, "join_return", fn);

        if (!mOutOfOrder) {
            auto *checkHeartbeatBB = llvm::BasicBlock::Create(ctx, "join_check_heartbeat", fn);
            auto *checkBehindBB = llvm::BasicBlock::Create(ctx, "join_check_behind", fn);
            auto *suppressBB = llvm::BasicBlock::Create(ctx, "join_heartbeat_suppress", fn);
            auto *continueBB = llvm::BasicBlock::Create(ctx, "join_heartbeat_continue", fn);

            segmentCtx.builder.CreateBr(checkHeartbeatBB);
            segmentCtx.builder.SetInsertPoint(checkHeartbeatBB);

            llvm::Value *isHeartbeatTrue = segmentCtx.builder.CreateICmpEQ(
                punctuation.isHeartbeat, llvm::ConstantInt::getTrue(ctx));

            segmentCtx.builder.CreateCondBr(isHeartbeatTrue, checkBehindBB, continueBB);

            segmentCtx.builder.SetInsertPoint(checkBehindBB);

            const auto getGlobalWmFn = module->getOrInsertFunction("rtJoinGetGlobalWatermark",
                                                                   llvm::FunctionType::get(i64Ty, {ptrTy}, false));
            llvm::Value *currentWatermark = segmentCtx.builder.CreateCall(getGlobalWmFn, {watermarkStatePtr});

            llvm::Value *isBehind = segmentCtx.builder.CreateICmpSLT(punctuation.watermark, currentWatermark);

            segmentCtx.builder.CreateCondBr(isBehind, suppressBB, continueBB);

            segmentCtx.builder.SetInsertPoint(suppressBB);

            segmentCtx.builder.CreateBr(returnBB);

            segmentCtx.builder.SetInsertPoint(continueBB);

            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(mergeBB);
        } else {
            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(mergeBB);
        }

        llvm::Value *isFinal = segmentCtx.builder.CreateZExt(punctuation.isFinal, i8Ty);
        llvm::Value* isLeft = llvm::ConstantInt::get(i8Ty, (compactInputIndex(inputPort) == 0) ? 1 : 0);
        llvm::Value* allowedLateness = llvm::ConstantInt::get(i64Ty, mConfig.allowedLateness);

        llvm::FunctionType *updateFnType = llvm::FunctionType::get(i8Ty, {ptrTy, i8Ty, i64Ty, i8Ty, i64Ty, ptrTy, ptrTy, ptrTy, ptrTy}, false);
        const llvm::FunctionCallee updateFn = module->getOrInsertFunction("rtJoinUpdateWatermark", updateFnType);

        auto *outEvictLeft = segmentCtx.storage.getNamed("evict_left_threshold", i64Ty);
        auto *outEvictRight = segmentCtx.storage.getNamed("evict_right_threshold", i64Ty);
        auto *outCurrentWatermark = segmentCtx.storage.getNamed("current_watermark", i64Ty);
        auto *outIsFinal = segmentCtx.storage.getNamed("out_is_final", i8Ty);

        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i8Ty, 0), outIsFinal);

        llvm::Value *sEmit = segmentCtx.builder.CreateCall(updateFn, {
                                                               watermarkStatePtr, isLeft, punctuation.watermark,
                                                               isFinal, allowedLateness, outEvictLeft, outEvictRight,
                                                               outCurrentWatermark, outIsFinal
                                                           }, "should_emit");

        llvm::Value *condition = segmentCtx.builder.CreateICmpNE(sEmit, llvm::ConstantInt::get(i8Ty, 0));

        segmentCtx.builder.CreateCondBr(condition, emitBB, afterMergeBB);

        segmentCtx.builder.SetInsertPoint(emitBB);

        llvm::Value *currentWatermark = segmentCtx.builder.CreateLoad(i64Ty, outCurrentWatermark);
        llvm::Value *currentIsFinal = segmentCtx.builder.CreateLoad(i8Ty, outIsFinal);
        llvm::Value *newIsFinal = segmentCtx.builder.CreateICmpNE(currentIsFinal, llvm::ConstantInt::get(i8Ty, 0));

        const auto outgoingPunctuation = core::physical::data::PunctuationPackage::create(segmentCtx.builder, currentWatermark, newIsFinal);

        consumers.front().consumePunctuation(segmentCtx, outgoingPunctuation);

        segmentCtx.builder.CreateBr(afterMergeBB);

        segmentCtx.builder.SetInsertPoint(afterMergeBB);

        const auto lockFn = module->getOrInsertFunction("rtJoinLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        const auto unlockFn = module->getOrInsertFunction("rtJoinUnlock", llvm::FunctionType::get(voidTy, {ptrTy}, false));

        segmentCtx.builder.CreateCall(lockFn, {joinState});

        llvm::Value *evictLeftTh = segmentCtx.builder.CreateLoad(i64Ty, outEvictLeft);
        llvm::Value *evictRightTh = segmentCtx.builder.CreateLoad(i64Ty, outEvictRight);
        llvm::Value *isFinalOut = segmentCtx.builder.CreateLoad(i8Ty, outIsFinal);

        codegenEvict(segmentCtx, joinState, evictLeftTh, false, isFinalOut);
        codegenEvict(segmentCtx, joinState, evictRightTh, true, isFinalOut);

        if (!mOutOfOrder) {
            codegenDrainAndEmitOutput(segmentCtx, joinState, outCurrentWatermark, consumers);
        }

        segmentCtx.builder.CreateCall(unlockFn, {joinState});

        segmentCtx.builder.CreateBr(returnBB);

        segmentCtx.builder.SetInsertPoint(returnBB);
    }

    void JoinOperator::codegenDrainAndEmitOutput(core::physical::segment::SegmentContext &segmentCtx,
                                                 llvm::Value *joinState,
                                                 llvm::Value *watermark,
                                                 const core::physical::segment::SegmentDataConsumers &consumers) {
        llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
        llvm::Function *fn = segmentCtx.builder.GetInsertBlock()->getParent();
        llvm::Module *module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto *ptrTy = llvm::PointerType::getUnqual(ctx);
        auto *i64Ty = llvm::Type::getInt64Ty(ctx);
        auto *i8Ty = llvm::Type::getInt8Ty(ctx);
        auto *voidTy = llvm::Type::getVoidTy(ctx);
        auto *sizeTTy = module->getDataLayout().getIntPtrType(ctx);

        constexpr std::size_t MAX_DRAIN = 64;

        auto *indicesArr = segmentCtx.storage.getNamed("join_drain_indices." + std::to_string(getPId()),
                                                       llvm::ArrayType::get(sizeTTy, MAX_DRAIN));

        llvm::Value* watermarkValue = segmentCtx.builder.CreateLoad(i64Ty, watermark);

        const auto drainFn = module->getOrInsertFunction("rtJoinDrainOutput",
                                                         llvm::FunctionType::get(
                                                             sizeTTy, {ptrTy, i64Ty, sizeTTy, ptrTy}, false));
        llvm::Value *drainedCount = segmentCtx.builder.CreateCall(drainFn, {
                                                                      joinState, watermarkValue,
                                                                      llvm::ConstantInt::get(sizeTTy, MAX_DRAIN),
                                                                      indicesArr
                                                                  });

        auto *loopHeaderBB = llvm::BasicBlock::Create(ctx, "join_drain_loop_header", fn);
        auto *loopBodyBB = llvm::BasicBlock::Create(ctx, "join_drain_loop_body", fn);
        auto *loopExitBB = llvm::BasicBlock::Create(ctx, "join_drain_loop_exit", fn);

        auto *loopIdxSlot = segmentCtx.storage.getNamed("drain_loop_idx." + std::to_string(getPId()), sizeTTy);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(sizeTTy, 0), loopIdxSlot);

        segmentCtx.builder.CreateBr(loopHeaderBB);
        segmentCtx.builder.SetInsertPoint(loopHeaderBB);

        llvm::Value *loopIdx = segmentCtx.builder.CreateLoad(i64Ty, loopIdxSlot);
        segmentCtx.builder.CreateCondBr(segmentCtx.builder.CreateICmpSLT(loopIdx, drainedCount), loopBodyBB,
                                        loopExitBB);

        segmentCtx.builder.SetInsertPoint(loopBodyBB);

        llvm::Value *rowIdx = segmentCtx.builder.CreateLoad(
            sizeTTy, segmentCtx.builder.CreateGEP(llvm::ArrayType::get(sizeTTy, MAX_DRAIN), indicesArr,
                                                {llvm::ConstantInt::get(sizeTTy, 0), loopIdx}));

        const auto getRowPtrFn = module->getOrInsertFunction("rtJoinGetOutputRowPtr",
                                                             llvm::FunctionType::get(ptrTy, {ptrTy, sizeTTy}, false));
        llvm::Value *rowPtr = segmentCtx.builder.CreateCall(getRowPtrFn, {joinState, rowIdx});

        const auto getStartTsFn = module->getOrInsertFunction("rtJoinGetOutputRowStartTs",
                                                              llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getEndTsFn = module->getOrInsertFunction("rtJoinGetOutputRowEndTs",
                                                            llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getStartTbFn = module->getOrInsertFunction("rtJoinGetOutputRowStartTb",
                                                              llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));
        const auto getEndTbFn = module->getOrInsertFunction("rtJoinGetOutputRowEndTb",
                                                            llvm::FunctionType::get(i64Ty, {ptrTy, sizeTTy}, false));

        llvm::Value* rowStartTs = segmentCtx.builder.CreateCall(getStartTsFn, {joinState, rowIdx});
        llvm::Value* rowEndTs = segmentCtx.builder.CreateCall(getEndTsFn, {joinState, rowIdx});
        llvm::Value* rowStartTb = segmentCtx.builder.CreateCall(getStartTbFn, {joinState, rowIdx});
        llvm::Value* rowEndTb = segmentCtx.builder.CreateCall(getEndTbFn, {joinState, rowIdx});

        core::physical::data::DataPackage combined(core::physical::time::TimeInterval(
        segmentCtx.builder,
        core::physical::time::TimePoint(rowStartTs, rowStartTb),
        core::physical::time::TimePoint(rowEndTs, rowEndTb)));

        combined.reserve(mLeftSchema.size() + mRightSchema.size());

        std::size_t offset = 0;
        for (std::size_t i = 0; i < mLeftSchema.size(); ++i) {
            llvm::Value* fieldPtr = segmentCtx.builder.CreateGEP(
                i8Ty, rowPtr, llvm::ConstantInt::get(sizeTTy, offset));
            llvm::Type* fieldTy = segmentCtx.codegenTypeRegistry.resolveType(
                mLeftSchema.at(i).valueTypeId, ctx);
            combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
            offset += getTypeSize(mLeftSchema.at(i).valueTypeId);
        }

        for (std::size_t i = 0; i < mRightSchema.size(); ++i) {
            llvm::Value* fieldPtr = segmentCtx.builder.CreateGEP(
                i8Ty, rowPtr, llvm::ConstantInt::get(sizeTTy, offset));
            llvm::Type* fieldTy = segmentCtx.codegenTypeRegistry.resolveType(
                mRightSchema.at(i).valueTypeId, ctx);
            combined.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
            offset += getTypeSize(mRightSchema.at(i).valueTypeId);
        }

        consumers.front().consume(segmentCtx, combined);

        llvm::Value* next = segmentCtx.builder.CreateAdd(loopIdx, llvm::ConstantInt::get(sizeTTy, 1));
        segmentCtx.builder.CreateStore(next, loopIdxSlot);
        segmentCtx.builder.CreateBr(loopHeaderBB);

        segmentCtx.builder.SetInsertPoint(loopExitBB);

        const auto removeFn = module->getOrInsertFunction("rtJoinRemoveOutputRows",
                                                          llvm::FunctionType::get(
                                                              voidTy, {ptrTy, ptrTy, sizeTTy}, false));
        segmentCtx.builder.CreateCall(removeFn, {joinState, indicesArr, drainedCount});
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical
