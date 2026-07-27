#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECT_OPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECT_OPERATOR_PHYSICAL_H

#include <vector>
#include "core/physical/operator/pipe/pipe_operator.h"
#include "core/common/expression/expression.h"
#include "core/common/type/type_registry.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class SelectOperator : public core::physical::opr::TypedPipeOperator {
    public:
        SelectOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                       const core::common::expression::IExpression &predicate)
            : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                                core::physical::port::Ports{std::move(outputPort)}),
              mPredicate(predicate) {
        }

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::common::identifier::PortId /*inputPort*/, const core::physical::port::PortTuple &tuple,
                     const core::physical::port::PortTupleConsumers &consumers) override {
            llvm::LLVMContext &ctx = segmentCtx.builder.getContext();
            llvm::Function *fn = segmentCtx.builder.GetInsertBlock()->getParent();

            llvm::Value *cnd = codegenExpression(segmentCtx, tuple, mPredicate);

            auto *passBB = llvm::BasicBlock::Create(ctx, "select_pass", fn);
            auto *mergeBB = llvm::BasicBlock::Create(ctx, "select_merge", fn);

            segmentCtx.builder.CreateCondBr(cnd, passBB, mergeBB);

            segmentCtx.builder.SetInsertPoint(passBB);
            consumers.front()(segmentCtx, tuple);
            segmentCtx.builder.CreateBr(mergeBB);

            segmentCtx.builder.SetInsertPoint(mergeBB);
        }

        void end(core::physical::segment::SegmentContext &segmentCtx, const core::common::identifier::PortId inputPort,
                 const core::physical::port::PortTupleConsumers &consumers) override {
        }

    private:
        const core::common::expression::IExpression &mPredicate;

        static llvm::Value *codegenExpression(core::physical::segment::SegmentContext &segmentCtx,
                                              const core::physical::port::PortTuple &tuple,
                                              const core::common::expression::IExpression &expression) {
            switch (expression.getExpressionType()) {
                case core::common::expression::ExpressionType::FIELD_REFERENCE: {
                    // NOLINTNEXTLINE
                    const auto &fieldRefExpression = static_cast<const
                        core::common::expression::FieldReferenceExpression &>(expression);
                    if (fieldRefExpression.getFieldIndex() >= tuple.size()) {
                        throw std::out_of_range("SelectOperator: field index out of range");
                    }
                    return tuple.at(fieldRefExpression.getFieldIndex());
                }
                case core::common::expression::ExpressionType::CONSTANT: {
                    // NOLINTNEXTLINE
                    const auto &constExpression = static_cast<const core::common::expression::ConstantExpression &>(
                        expression);
                    return segmentCtx.typeRegistry.compileConstant(constExpression.getTypeId(), segmentCtx.builder,
                                                                   constExpression.getSerializedValue());
                }
                case core::common::expression::ExpressionType::COMPARE: {
                    // NOLINTNEXTLINE
                    const auto &compareExpression = static_cast<const core::common::expression::CompareExpression &>(
                        expression);
                    llvm::Value *left = codegenExpression(segmentCtx, tuple, compareExpression.getLeftPart());
                    llvm::Value *right = codegenExpression(segmentCtx, tuple, compareExpression.getRightPart());
                    return codegenCompare(segmentCtx.builder, left, right, compareExpression.getOpr());
                }
                case core::common::expression::ExpressionType::LOGICAL: {
                    // NOLINTNEXTLINE
                    const auto &logicalExpression = static_cast<const core::common::expression::LogicalExpression &>(
                        expression);
                    llvm::Value *left = codegenExpression(segmentCtx, tuple, logicalExpression.getLeftPart());
                    if (logicalExpression.isUnary()) {
                        return segmentCtx.builder.CreateNot(left, "not");
                    }
                    llvm::Value *right = codegenExpression(segmentCtx, tuple, *logicalExpression.getRightPart());
                    switch (logicalExpression.getOpr()) {
                        case core::common::expression::LogicalExpression::LogicalOp::AND: {
                            return segmentCtx.builder.CreateAnd(left, right, "and");
                        }
                        case core::common::expression::LogicalExpression::LogicalOp::OR: {
                            return segmentCtx.builder.CreateOr(left, right, "or");
                        }
                        default:
                            throw std::invalid_argument("SelectOperator: unsupported logical expression type");
                    }
                }
                default:
                    throw std::invalid_argument("SelectOperator: unsupported expression type");
            }
        }

        static llvm::Value *codegenCompare(llvm::IRBuilder<> &builder, llvm::Value *left, llvm::Value *right,
                                           const core::common::expression::CompareExpression::CompareOp opr) {
            const bool isFloat = left->getType()->isFloatingPointTy();
            // NOLINTBEGIN(clang-analyzer-security.ArrayBound)
            switch (opr) {
                case core::common::expression::CompareExpression::CompareOp::EQ: {
                    return isFloat ? builder.CreateFCmpOEQ(left, right, "eq") : builder.CreateICmpEQ(left, right, "eq");
                }
                case core::common::expression::CompareExpression::CompareOp::NE: {
                    return isFloat ? builder.CreateFCmpONE(left, right, "ne") : builder.CreateICmpNE(left, right, "ne");
                }
                case core::common::expression::CompareExpression::CompareOp::LT: {
                    return isFloat
                               ? builder.CreateFCmpOLT(left, right, "lt")
                               : builder.CreateICmpSLT(left, right, "lt");
                }
                case core::common::expression::CompareExpression::CompareOp::LE: {
                    return isFloat
                               ? builder.CreateFCmpOLE(left, right, "le")
                               : builder.CreateICmpSLE(left, right, "le");
                }
                case core::common::expression::CompareExpression::CompareOp::GT: {
                    return isFloat
                               ? builder.CreateFCmpOGT(left, right, "gt")
                               : builder.CreateICmpSGT(left, right, "gt");
                }
                case core::common::expression::CompareExpression::CompareOp::GE: {
                    return isFloat
                               ? builder.CreateFCmpOGE(left, right, "ge")
                               : builder.CreateICmpSGE(left, right, "ge");
                }
                default:
                    throw std::invalid_argument("SelectOperator: unsupported compare type");
            }
            // NOLINTEND(clang-analyzer-security.ArrayBound)
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COSIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECT_OPERATOR_PHYSICAL_HMMON_OPERATOR_PIPE_SELECT_SELECTOPERATOR_H