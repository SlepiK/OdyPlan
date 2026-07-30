#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_H
#include "core/common/expression/expression.h"
#include "core/logical/port/port.h"
#include "core/physical/operator/pipe/pipe_operator.h"
#include "internal/interval_join_config.h"
#include "internal/interval_join_predmod.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class IntervalJoinOperator : public core::physical::opr::TypedPipeOperator {
    public:
        IntervalJoinOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
            const internal::IntervalJoinConfig config, core::logical::port::Ports schemas,
            const core::common::expression::IExpression* predicate)
            : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                                core::physical::port::Ports{std::move(outputPort)}),
                mConfig(config), mLeftSchema(std::move(schemas.at(0))),
                mRightSchema(std::move(schemas.at(1))), mPredicate(predicate) {
        }

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::common::identifier::PortId inputPort, const core::physical::port::PortTuple &tuple,
                     const core::physical::port::PortTupleConsumers &consumers) override {
            llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
            llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto* voidTy = llvm::Type::getVoidTy(ctx);
            auto* ptrTy = llvm::PointerType::getUnqual(ctx);

            llvm::Value* state = loadStatePtr(segmentCtx);

            const auto lockFn = module->getOrInsertFunction("rtJoinLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(lockFn, {state});

            if (inputPort == 0) {
                //left
                codegenSide(segmentCtx, state, tuple, mLeftSchema, "rtJoinBeginLeft", "rtJoinPushLeft", "rtJoinEndLeft");
                emitMatch(segmentCtx, state, tuple, true, consumers);
                codegenEvict(segmentCtx, state, tuple.timestamp, true);
            } else {
                //right
                codegenSide(segmentCtx, state, tuple, mRightSchema, "rtJoinBeginRight", "rtJoinPushRight", "rtJoinEndRight");
                emitMatch(segmentCtx, state, tuple, false, consumers);
                codegenEvict(segmentCtx, state, tuple.timestamp, false);
            }

            const auto unlockFn = module->getOrInsertFunction("rtJoinUnlock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
            segmentCtx.builder.CreateCall(unlockFn, {state});
        }

        void declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module,
                          llvm::LLVMContext &context) override {
            auto* i64Ty = llvm::Type::getInt64Ty(context);
            auto* i8Ty = llvm::Type::getInt8Ty(context);
            auto* ptrTy = llvm::PointerType::getUnqual(context);

            auto* leftSideTypesGlobal = buildTypeIds(module, context, mLeftSchema, "join_left_side_types_" + std::to_string(getId()));
            auto* rightSideTypesGlobal = buildTypeIds(module, context, mRightSchema, "join_right_side_types_" + std::to_string(getId()));

            const auto equiKey = internal::tryGetEquiKey(mPredicate, mLeftSchema, mRightSchema);
            const bool hasEquiKey = equiKey.has_value();

            mState = stateRegistry.createHandle(
                "join_" + std::to_string(getId()),
                "rtJoinCreate", {
                    leftSideTypesGlobal, llvm::ConstantInt::get(i64Ty, mLeftSchema.size()),
                    rightSideTypesGlobal, llvm::ConstantInt::get(i64Ty, mRightSchema.size()),
                    llvm::ConstantInt::get(i64Ty, mConfig.leftWindowSize.count()),
                    llvm::ConstantInt::get(i64Ty, mConfig.rightWindowSize.count()),
                    llvm::ConstantInt::get(i8Ty, hasEquiKey ? 1 : 0),
                    llvm::ConstantInt::get(i64Ty, hasEquiKey ? equiKey->leftFieldIndex : 0),
                    llvm::ConstantInt::get(i64Ty, hasEquiKey ? equiKey->rightFieldIndex : 0),
                    llvm::ConstantInt::get(i8Ty, hasEquiKey ? equiKey->typeId : 0)
                },
                {ptrTy, i64Ty, ptrTy, i64Ty, i64Ty, i64Ty, i8Ty, i64Ty, i64Ty, i8Ty},
                "rtJoinDestroy"
                );
        }

    private:
        internal::IntervalJoinConfig mConfig;
        core::logical::port::Port mLeftSchema;
        core::logical::port::Port mRightSchema;
        const core::common::expression::IExpression* mPredicate;
        core::physical::state::StateHandle mState;

        llvm::Value* loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
            auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
            return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
        }

        static void codegenEvict(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* state, llvm::Value* timestamp, const bool callFromLeft) {
            llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
            llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto* voidTy = llvm::Type::getVoidTy(ctx);
            auto* ptrTy = llvm::PointerType::getUnqual(ctx);
            auto* i64Ty = llvm::Type::getInt64Ty(ctx);
            auto* i1Ty = llvm::Type::getInt1Ty(ctx);

            const auto evictFn = module->getOrInsertFunction("rtJoinEvict", llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, i1Ty}, false));
            segmentCtx.builder.CreateCall(evictFn, {state, timestamp, llvm::ConstantInt::get(i1Ty, callFromLeft ? 1 : 0)});
        }

        static llvm::GlobalVariable* buildTypeIds(llvm::Module& module, llvm::LLVMContext& ctx, const core::logical::port::Port& schema, const std::string& name) {
            auto* i8Ty = llvm::Type::getInt8Ty(ctx);
            std::vector<llvm::Constant*> values;
            values.reserve(schema.size());
            for (const auto& slot : schema) {
                values.push_back(llvm::ConstantInt::get(i8Ty, slot.typeId));
            }
            auto* arrTy = llvm::ArrayType::get(i8Ty, values.size());
            return new llvm::GlobalVariable(module, arrTy, true, llvm::GlobalValue::InternalLinkage, llvm::ConstantArray::get(arrTy, values), name);
        }

        void emitMatch(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* state,
            const core::physical::port::PortTuple& tuple, const bool onLeft, const core::physical::port::PortTupleConsumers& consumers) {
            llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
            llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();
            llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto* i64Ty = llvm::Type::getInt64Ty(ctx);
            auto* ptrTy = llvm::PointerType::getUnqual(ctx);

            auto sizeFn = module->getOrInsertFunction("rtJoinGetCandidateSize", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
            llvm::Value* size = segmentCtx.builder.CreateCall(sizeFn, {state});

            auto* indexSlot = segmentCtx.storage.get(std::to_underlying(core::common::identifier::Type::INT64));
            segmentCtx.builder.CreateStore(llvm::ConstantInt::get(i64Ty, 0), indexSlot);

            auto* headerBB = llvm::BasicBlock::Create(ctx, "join_match_header", fn);
            auto* bodyBB = llvm::BasicBlock::Create(ctx, "join_match_body", fn);
            auto* exitBB = llvm::BasicBlock::Create(ctx, "join_match_exit", fn);

            segmentCtx.builder.CreateBr(headerBB);
            segmentCtx.builder.SetInsertPoint(headerBB);
            llvm::Value* index = segmentCtx.builder.CreateLoad(i64Ty, indexSlot);
            llvm::Value* condition = segmentCtx.builder.CreateICmpSLT(index, size);
            segmentCtx.builder.CreateCondBr(condition, bodyBB, exitBB);

            segmentCtx.builder.SetInsertPoint(bodyBB);

            const auto& otherSchema = onLeft ? mRightSchema : mLeftSchema;

            auto getCandidateFn = module->getOrInsertFunction("rtJoinGetCandidatePtr", llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty, i64Ty}, false));

            core::physical::port::PortTuple otherTuple;
            otherTuple.reserve(otherSchema.size());

            for (std::size_t i = 0; i < otherSchema.size(); ++i) {
                llvm::Value* candidatePtr = segmentCtx.builder.CreateCall(getCandidateFn, {state, index, llvm::ConstantInt::get(i64Ty, i)});
                llvm::Type* candidateTy = segmentCtx.typeRegistry.resolveType(otherSchema.at(i).typeId, ctx);
                otherTuple.pushBack(segmentCtx.builder.CreateLoad(candidateTy, candidatePtr));
            }

            auto getCandidateTimestampFn = module->getOrInsertFunction("rtJoinGetCandidateTimestamp", llvm::FunctionType::get(i64Ty, {ptrTy, i64Ty}, false));
            llvm::Value* otherTimestamp = segmentCtx.builder.CreateCall(getCandidateTimestampFn, {state, index});

            core::physical::port::PortTuple combined;
            if (onLeft) {
                combined.fields.insert(combined.end(), tuple.begin(), tuple.end());
                combined.fields.insert(combined.end(), otherTuple.begin(), otherTuple.end());
            } else {
                combined.fields.insert(combined.end(), otherTuple.begin(), otherTuple.end());
                combined.fields.insert(combined.end(), tuple.begin(), tuple.end());
            }

            llvm::Value* isNewerCondition = segmentCtx.builder.CreateICmpSGT(tuple.timestamp, otherTimestamp);
            combined.timestamp = segmentCtx.builder.CreateSelect(isNewerCondition, tuple.timestamp, otherTimestamp);

            if (mPredicate != nullptr) {
                llvm::Value* predicateResult = codegenExpression(segmentCtx, combined, *mPredicate);

                auto* passBB = llvm::BasicBlock::Create(ctx,"join_predicate_pass", fn);
                auto* mergeBB = llvm::BasicBlock::Create(ctx,"join_predicate_merge", fn);
                segmentCtx.builder.CreateCondBr(predicateResult, passBB, mergeBB);

                segmentCtx.builder.SetInsertPoint(passBB);
                consumers.at(0)(segmentCtx, combined);
                segmentCtx.builder.CreateBr(mergeBB);

                segmentCtx.builder.SetInsertPoint(mergeBB);
            } else {
                consumers.at(0)(segmentCtx, combined);
            }

            llvm::Value* next = segmentCtx.builder.CreateAdd(index, llvm::ConstantInt::get(i64Ty, 1));
            segmentCtx.builder.CreateStore(next, indexSlot);
            segmentCtx.builder.CreateBr(headerBB);

            segmentCtx.builder.SetInsertPoint(exitBB);
        }

        void codegenSide(core::physical::segment::SegmentContext &segmentCtx, llvm::Value* state, const core::physical::port::PortTuple& tuple,
                         const core::logical::port::Port& schema, const char* beginFnName,
                         const char* pushFnName, const char* endFnName) {
            llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
            llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

            auto* voidTy = llvm::Type::getVoidTy(ctx);
            auto* ptrTy = llvm::PointerType::getUnqual(ctx);
            auto* i64Ty = llvm::Type::getInt64Ty(ctx);

            const auto beginFn = module->getOrInsertFunction(beginFnName, llvm::FunctionType::get(voidTy, { ptrTy }, false));
            segmentCtx.builder.CreateCall(beginFn, {state});

            const auto pushFn = module->getOrInsertFunction(pushFnName, llvm::FunctionType::get(voidTy, { ptrTy, i64Ty, ptrTy }, false));
            for (std::size_t i = 0; i < schema.size(); ++i) {
                llvm::Value* slot = segmentCtx.storage.get(schema.at(i).typeId);
                segmentCtx.builder.CreateStore(tuple.at(i), slot);
                segmentCtx.builder.CreateCall(pushFn, {state, llvm::ConstantInt::get(i64Ty, i), slot});
            }

            const auto endFn = module->getOrInsertFunction(endFnName, llvm::FunctionType::get(voidTy, {ptrTy, i64Ty}, false));
            segmentCtx.builder.CreateCall(endFn, {state, tuple.timestamp});
        }

        llvm::Value* codegenExpression(core::physical::segment::SegmentContext& segmentCtx, const core::physical::port::PortTuple& tuple,
            const core::common::expression::IExpression& expression) {
            switch (expression.getExpressionType()) {
                case core::common::expression::ExpressionType::FIELD_REFERENCE: {
                    //NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                    const auto& fieldRef = static_cast<const core::common::expression::FieldReferenceExpression&>(expression);
                    if (fieldRef.getFieldIndex() >= tuple.size()) {
                        throw std::out_of_range("IntervalJoinOperator: Field index out of range");
                    }
                    return tuple.at(fieldRef.getFieldIndex());
                }
                case core::common::expression::ExpressionType::CONSTANT: {
                    //NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                    const auto& constExp = static_cast<const core::common::expression::ConstantExpression&>(expression);
                    return segmentCtx.typeRegistry.compileConstant(constExp.getTypeId(), segmentCtx.builder, constExp.getSerializedValue());
                }
                case core::common::expression::ExpressionType::COMPARE: {
                    //NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                    const auto& cmpExp = static_cast<const core::common::expression::CompareExpression&>(expression);
                    llvm::Value* left = codegenExpression(segmentCtx, tuple, cmpExp.getLeftPart());
                    llvm::Value* right = codegenExpression(segmentCtx, tuple, cmpExp.getRightPart());
                    return codegenCompare(segmentCtx.builder, left, right, cmpExp.getOpr());
                }
                case core::common::expression::ExpressionType::LOGICAL: {
                    //NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
                    const auto& logicExp = static_cast<const core::common::expression::LogicalExpression&>(expression);
                    llvm::Value* left = codegenExpression(segmentCtx, tuple, logicExp.getLeftPart());
                    if (logicExp.isUnary()) {
                        return segmentCtx.builder.CreateNot(left);
                    }

                    llvm::Value* right = codegenExpression(segmentCtx, tuple, *logicExp.getRightPart());

                    switch (logicExp.getOpr()) {
                        case core::common::expression::LogicalExpression::LogicalOp::AND: {
                            return segmentCtx.builder.CreateAnd(left, right);
                        }
                        case core::common::expression::LogicalExpression::LogicalOp::OR: {
                            return segmentCtx.builder.CreateOr(left, right);
                        }
                        default:
                            throw std::out_of_range("IntervalJoinOperator::logic: Type of expression not supported");
                    }
                }
                default:
                    throw std::invalid_argument("IntervalJoinOperator: Invalid expression type");
            }
        }

        static llvm::Value* codegenCompare(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right, core::common::expression::CompareExpression::CompareOp opr) {
            const bool isFloat = left->getType()->isFloatingPointTy();
            // NOLINTBEGIN(clang-analyzer-security.ArrayBound)
            switch (opr) {
                case core::common::expression::CompareExpression::CompareOp::EQ: {
                    return isFloat ? builder.CreateFCmpOEQ(left, right) : builder.CreateICmpEQ(left, right);
                }
                case core::common::expression::CompareExpression::CompareOp::NE: {
                    return isFloat ? builder.CreateFCmpONE(left, right) : builder.CreateICmpNE(left, right);
                }
                case core::common::expression::CompareExpression::CompareOp::LT: {
                    return isFloat ? builder.CreateFCmpOLT(left, right) : builder.CreateICmpSLT(left, right);
                }
                case core::common::expression::CompareExpression::CompareOp::LE: {
                    return isFloat ? builder.CreateFCmpOLE(left, right) : builder.CreateICmpSLE(left, right);
                }
                case core::common::expression::CompareExpression::CompareOp::GT: {
                    return isFloat ? builder.CreateFCmpOGT(left, right) : builder.CreateICmpSGT(left, right);
                }
                case core::common::expression::CompareExpression::CompareOp::GE: {
                    return isFloat ? builder.CreateFCmpOGE(left, right) : builder.CreateICmpSGE(left, right);
                }
                default:
                    throw std::out_of_range("IntervalJoinOperator::compare: Invalid compare expression type");
            }
            // NOLINTEND(clang-analyzer-security.ArrayBound)
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_H