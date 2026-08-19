#include <basic/operators/pipe/join/physical/join_operator.h>

#include <core/physical/segment/segment_builder.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical {
    JoinOperator::JoinOperator(core::physical::port::Port inputPort,
                               core::physical::port::Port outputPort,
                               const logical::details::JoinConfig config,
                               core::logical::port::Ports schemas,
                               const basic::expression::IExpression* predicate)
        : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                            core::physical::port::Ports{std::move(outputPort)}),
          mConfig(config),
          mLeftSchema(std::move(schemas.at(0))),
          mRightSchema(std::move(schemas.at(1))),
          mPredicate(predicate) {}

    void JoinOperator::consume(core::physical::segment::SegmentContext& segmentCtx,
                               core::shared::identifiers::PortId inputPort,
                               const core::physical::data::DataPackage& tuple,
                               const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i1Ty = llvm::Type::getInt1Ty(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        const auto lockFn = module->getOrInsertFunction(
            "rtJoinLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(lockFn, {state});

        const bool onLeft = (inputPort == 0);

        if (onLeft) {
            // left
            codegenSide(segmentCtx,
                        state,
                        tuple,
                        mLeftSchema,
                        "rtJoinBeginLeft",
                        "rtJoinPushLeft",
                        "rtJoinEndLeft");
        } else {
            // right
            codegenSide(segmentCtx,
                        state,
                        tuple,
                        mRightSchema,
                        "rtJoinBeginRight",
                        "rtJoinPushRight",
                        "rtJoinEndRight");
        }

        const auto snapshotFn = module->getOrInsertFunction("rtJoinSnapshotForComputeAndUnlock",
            llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty, i1Ty}, false));

        llvm::Value* snapshot = segmentCtx.builder.CreateCall(snapshotFn, {state, tuple.timestamp, llvm::ConstantInt::get(i1Ty, onLeft ? 1 : 0)});

        emitMatchFromSnapshot(segmentCtx, snapshot, onLeft, consumers);

        const auto releaseFn = module->getOrInsertFunction(
            "rtJoinSnapshotRelease", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(releaseFn, {snapshot});
    }

    void JoinOperator::emitMatchFromSnapshot(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* snapshot,
        const bool onLeft, const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        const auto countFn = module->getOrInsertFunction(
            "rtJoinSnapshotGetCount", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        llvm::Value* count = segmentCtx.builder.CreateCall(countFn, {snapshot});

        const auto& ownSchema = onLeft ? mLeftSchema : mRightSchema;
        const auto getOwnFn = module->getOrInsertFunction(
            "rtJoinSnapshotGetOwnPtr", llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty}, false));

        core::physical::data::DataPackage ownTuple;
        ownTuple.reserve(ownSchema.size());
        for (std::size_t i = 0; i < ownSchema.size(); ++i) {
            llvm::Value* fieldPtr = segmentCtx.builder.CreateCall(
                getOwnFn, {snapshot, llvm::ConstantInt::get(i64Ty, i)});
            llvm::Type* fieldTy =
                segmentCtx.codegenTypeRegistry.resolveType(ownSchema.at(i).valueTypeId, ctx);
            ownTuple.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
        }

        const auto getOwnTsFn = module->getOrInsertFunction(
        "rtJoinSnapshotGetOwnTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        llvm::Value* ownTimestamp = segmentCtx.builder.CreateCall(getOwnTsFn, {snapshot});

        auto* indexSlot = segmentCtx.storage.getNamed(
            "computeIndexSlot." + std::to_string(getPId()), i64Ty);
        segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), indexSlot);

        auto* headerBB = llvm::BasicBlock::Create(ctx, "compute_match_header", fn);
        auto* bodyBB = llvm::BasicBlock::Create(ctx, "compute_match_body", fn);
        auto* exitBB = llvm::BasicBlock::Create(ctx, "compute_match_exit", fn);

        segmentCtx.builder.CreateBr(headerBB);
        segmentCtx.builder.SetInsertPoint(headerBB);

        llvm::Value* index = segmentCtx.builder.CreateLoad(i64Ty, indexSlot);

        segmentCtx.builder.CreateCondBr(
            segmentCtx.builder.CreateICmpSLT(index, count), bodyBB, exitBB);

        segmentCtx.builder.SetInsertPoint(bodyBB);

        const auto& otherSchema = onLeft ? mRightSchema : mLeftSchema;
        const auto getCandFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidatePtr",
            llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty, i64Ty}, false));

        core::physical::data::DataPackage otherTuple;
        otherTuple.reserve(otherSchema.size());
        for (std::size_t i = 0; i < otherSchema.size(); ++i) {
            llvm::Value* fieldPtr = segmentCtx.builder.CreateCall(
                getCandFn, {snapshot, index, llvm::ConstantInt::get(i64Ty, i)});
            llvm::Type* fieldTy =
                segmentCtx.codegenTypeRegistry.resolveType(otherSchema.at(i).valueTypeId, ctx);
            otherTuple.pushBack(segmentCtx.builder.CreateLoad(fieldTy, fieldPtr));
        }

        const auto getCandTsFn = module->getOrInsertFunction("rtJoinSnapshotGetCandidateTimestamp",
            llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
        llvm::Value* otherTimestamp = segmentCtx.builder.CreateCall(getCandTsFn, {snapshot, index});

        core::physical::data::DataPackage combined;
        if (onLeft) {
            combined.fields.insert(combined.end(), ownTuple.begin(), ownTuple.end());
            combined.fields.insert(combined.end(), otherTuple.begin(), otherTuple.end());
        } else {
            combined.fields.insert(combined.end(), otherTuple.begin(), otherTuple.end());
            combined.fields.insert(combined.end(), ownTuple.begin(), ownTuple.end());
        }

        llvm::Value* isNewer = segmentCtx.builder.CreateICmpSGT(ownTimestamp, otherTimestamp);
        combined.timestamp = segmentCtx.builder.CreateSelect(isNewer, ownTimestamp, otherTimestamp);

        if (mPredicate != nullptr) {
            llvm::Value* predicateResult = codegenExpression(segmentCtx, combined, *mPredicate);
            auto* passBB = llvm::BasicBlock::Create(ctx, "compute_predicate_pass", fn);
            auto* mergeBB = llvm::BasicBlock::Create(ctx, "compute_predicate_merge", fn);
            segmentCtx.builder.CreateCondBr(predicateResult, passBB, mergeBB);

            segmentCtx.builder.SetInsertPoint(passBB);
            consumers.front()(segmentCtx, combined);
            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(mergeBB);
        } else {
            consumers.front()(segmentCtx, combined);
        }

        llvm::Value* next = segmentCtx.builder.CreateAdd(index, llvm::ConstantInt::get(i64Ty, 1));
        segmentCtx.builder.CreateStore(next, indexSlot);
        segmentCtx.builder.CreateBr(headerBB);

        segmentCtx.builder.SetInsertPoint(exitBB);
    }

    void JoinOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                    llvm::Module& module,
                                    llvm::LLVMContext& context) {
        auto* i64Ty = llvm::Type::getInt64Ty(context);
        auto* i8Ty = llvm::Type::getInt8Ty(context);
        auto* ptrTy = llvm::PointerType::getUnqual(context);

        auto* leftSideTypesGlobal = buildTypeIds(
            module, context, mLeftSchema, "join_left_side_types_" + std::to_string(getPId()));
        auto* rightSideTypesGlobal = buildTypeIds(
            module, context, mRightSchema, "join_right_side_types_" + std::to_string(getPId()));

        const auto equiKey = details::tryGetEquiKey(mPredicate, mLeftSchema, mRightSchema);
        const bool hasEquiKey = equiKey.has_value();

        mState = stateRegistry.createHandle(
            "join_" + std::to_string(getPId()),
            "rtJoinCreate",
            {leftSideTypesGlobal,
             llvm::ConstantInt::get(i64Ty, mLeftSchema.size()),
             rightSideTypesGlobal,
             llvm::ConstantInt::get(i64Ty, mRightSchema.size()),
             llvm::ConstantInt::get(i64Ty, mConfig.leftWindowSize.count()),
             llvm::ConstantInt::get(i64Ty, mConfig.rightWindowSize.count()),
             llvm::ConstantInt::get(i8Ty, hasEquiKey ? 1 : 0),
             llvm::ConstantInt::get(i64Ty, hasEquiKey ? equiKey->leftFieldIndex : 0),
             llvm::ConstantInt::get(i64Ty, hasEquiKey ? equiKey->rightFieldIndex : 0),
             llvm::ConstantInt::get(i8Ty, hasEquiKey ? equiKey->typeId : 0)},
            {ptrTy, i64Ty, ptrTy, i64Ty, i64Ty, i64Ty, i8Ty, i64Ty, i64Ty, i8Ty},
            "rtJoinDestroy");
    }

    llvm::Value*
    JoinOperator::loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
        auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
    }

    void JoinOperator::codegenEvict(core::physical::segment::SegmentContext& segmentCtx,
                                    llvm::Value* state,
                                    llvm::Value* timestamp,
                                    const bool callFromLeft) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i1Ty = llvm::Type::getInt1Ty(ctx);

        const auto evictFn = module->getOrInsertFunction(
            "rtJoinEvict", llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i1Ty}, false));
        segmentCtx.builder.CreateCall(
            evictFn, {state, timestamp, llvm::ConstantInt::get(i1Ty, callFromLeft ? 1 : 0)});
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

    void JoinOperator::codegenSide(core::physical::segment::SegmentContext& segmentCtx,
                                   llvm::Value* state,
                                   const core::physical::data::DataPackage& tuple,
                                   const core::logical::port::Port& schema,
                                   const char* beginFnName,
                                   const char* pushFnName,
                                   const char* endFnName) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

        auto* voidTy = llvm::Type::getVoidTy(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i64Ty = llvm::Type::getInt64Ty(ctx);

        const auto beginFn = module->getOrInsertFunction(
            beginFnName, llvm::FunctionType::get(voidTy, {ptrTy}, false));
        segmentCtx.builder.CreateCall(beginFn, {state});

        const auto pushFn = module->getOrInsertFunction(
            pushFnName, llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, ptrTy}, false));
        for (std::size_t i = 0; i < schema.size(); ++i) {
            auto *slot = segmentCtx.storage.getNamed("slot." +std::to_string(getPId()), schema.at(i).valueTypeId);
            segmentCtx.builder.CreateStore(tuple.at(i), slot);
            segmentCtx.builder.CreateCall(pushFn, {state, llvm::ConstantInt::get(i64Ty, i), slot});
        }

        const auto endFn = module->getOrInsertFunction(
            endFnName, llvm::FunctionType::get(voidTy, {ptrTy, i64Ty}, false));
        segmentCtx.builder.CreateCall(endFn, {state, tuple.timestamp});
    }

    llvm::Value*
    JoinOperator::codegenExpression(core::physical::segment::SegmentContext& segmentCtx,
                                    const core::physical::data::DataPackage& tuple,
                                    const basic::expression::IExpression& expression) {
        switch (expression.getExpressionType()) {
            case basic::expression::ExpressionType::FIELD_REFERENCE: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& fieldRef =
                    static_cast<const basic::expression::FieldReferenceExpression&>(expression);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                if (fieldRef.getFieldIndex() >= tuple.size()) {
                    throw std::out_of_range("IntervalJoinOperator: Field index out of range");
                }
                auto* field = tuple.at(fieldRef.getFieldIndex());
                if (field == nullptr) {
                    throw std::out_of_range("IntervalJoinOperator: Field returned nullptr");
                }
                return field;
            }
            case basic::expression::ExpressionType::CONSTANT: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& constExp =
                    static_cast<const basic::expression::ConstantExpression&>(expression);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                return segmentCtx.codegenTypeRegistry.compileConstant(
                    constExp.getTypeId(), segmentCtx.builder, constExp.getSerializedValue());
            }
            case basic::expression::ExpressionType::COMPARE: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& cmpExp =
                    static_cast<const basic::expression::CompareExpression&>(expression);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

                llvm::Value* left = codegenExpression(segmentCtx, tuple, cmpExp.getLeftPart());
                if (left == nullptr) {
                    throw std::invalid_argument(
                        "JoinOperator: failed to codegen left operand of compare expression");
                }

                llvm::Value* right = codegenExpression(segmentCtx, tuple, cmpExp.getRightPart());
                if (right == nullptr) {
                    throw std::invalid_argument(
                        "JoinOperator: failed to codegen right operand of compare expression");
                }

                return codegenCompare(segmentCtx.builder, left, right, cmpExp.getOpr());
            }
            case basic::expression::ExpressionType::LOGICAL: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& logicExp =
                    static_cast<const basic::expression::LogicalExpression&>(expression);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                llvm::Value* left = codegenExpression(segmentCtx, tuple, logicExp.getLeftPart());

                if (left == nullptr) {
                    throw std::invalid_argument(
                        "JoinOperator: failed to codegen left operand of logical expression");
                }

                if (logicExp.isUnary()) {
                    return segmentCtx.builder.CreateNot(left);
                }

                const auto* rightPart = logicExp.getRightPart();
                if (rightPart == nullptr) {
                    throw std::invalid_argument(
                        "JoinOperator: binary logical expression missing right part");
                }

                llvm::Value* right = codegenExpression(segmentCtx, tuple, *rightPart);

                if (right == nullptr) {
                    throw std::invalid_argument(
                        "JoinOperator: failed to codegen right operand of logical expression");
                }

                switch (logicExp.getOpr()) {
                    case basic::expression::LogicalExpression::LogicalOp::AND: {
                        return segmentCtx.builder.CreateAnd(left, right);
                    }
                    case basic::expression::LogicalExpression::LogicalOp::OR: {
                        return segmentCtx.builder.CreateOr(left, right);
                    }
                    default:
                        throw std::out_of_range(
                            "IntervalJoinOperator::logic: Type of expression not supported");
                }
            }
            default:
                throw std::invalid_argument("IntervalJoinOperator: Invalid expression type");
        }
    }

    llvm::Value*
    JoinOperator::codegenCompare(llvm::IRBuilder<>& builder,
                                 llvm::Value* left,
                                 llvm::Value* right,
                                 const basic::expression::CompareExpression::CompareOp opr) {
        if (left->getType() != right->getType()) {
            throw std::invalid_argument("JoinOperator: expression operands have mismatched types");
        }
        const bool isFloat = left->getType()->isFloatingPointTy();

        // NOLINTBEGIN(clang-analyzer-security.ArrayBound)
        switch (opr) {
            case basic::expression::CompareExpression::CompareOp::EQ: {
                return isFloat ? builder.CreateFCmpOEQ(left, right)
                               : builder.CreateICmpEQ(left, right);
            }
            case basic::expression::CompareExpression::CompareOp::NE: {
                return isFloat ? builder.CreateFCmpONE(left, right)
                               : builder.CreateICmpNE(left, right);
            }
            case basic::expression::CompareExpression::CompareOp::LT: {
                return isFloat ? builder.CreateFCmpOLT(left, right)
                               : builder.CreateICmpSLT(left, right);
            }
            case basic::expression::CompareExpression::CompareOp::LE: {
                return isFloat ? builder.CreateFCmpOLE(left, right)
                               : builder.CreateICmpSLE(left, right);
            }
            case basic::expression::CompareExpression::CompareOp::GT: {
                return isFloat ? builder.CreateFCmpOGT(left, right)
                               : builder.CreateICmpSGT(left, right);
            }
            case basic::expression::CompareExpression::CompareOp::GE: {
                return isFloat ? builder.CreateFCmpOGE(left, right)
                               : builder.CreateICmpSGE(left, right);
            }
            default:
                throw std::out_of_range(
                    "IntervalJoinOperator::compare: Invalid compare expression type");
        }
        // NOLINTEND(clang-analyzer-security.ArrayBound)
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical
