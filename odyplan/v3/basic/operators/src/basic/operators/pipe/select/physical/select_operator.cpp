#include <stdexcept>
#include <basic/operators/pipe/select/physical/select_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::select::physical {
    SelectOperator::SelectOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                       const basic::expression::IExpression &predicate)
            : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)},
                                core::physical::port::Ports{std::move(outputPort)}),
              mPredicate(predicate) {
    }

    void SelectOperator::consume(core::physical::segment::SegmentContext &segmentCtx,
                 core::shared::identifiers::PortId /*inputPort*/, const core::physical::data::DataPackage &tuple,
                 const core::physical::segment::SegmentDataConsumers &consumers) {
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

    void SelectOperator::end(core::physical::segment::SegmentContext &segmentCtx, const core::shared::identifiers::PortId inputPort,
             const core::physical::segment::SegmentDataConsumers &consumers) {
    }

    llvm::Value *SelectOperator::codegenExpression(core::physical::segment::SegmentContext &segmentCtx,
                                              const core::physical::data::DataPackage &tuple,
                                              const basic::expression::IExpression &expression) {
            switch (expression.getExpressionType()) {
                case basic::expression::ExpressionType::FIELD_REFERENCE: {
                    // NOLINTNEXTLINE
                    const auto &fieldRefExpression = static_cast<const
                        basic::expression::FieldReferenceExpression &>(expression);
                    if (fieldRefExpression.getFieldIndex() >= tuple.size()) {
                        throw std::out_of_range("SelectOperator: field index out of range");
                    }
                    return tuple.at(fieldRefExpression.getFieldIndex());
                }
                case basic::expression::ExpressionType::CONSTANT: {
                    // NOLINTNEXTLINE
                    const auto &constExpression = static_cast<const basic::expression::ConstantExpression &>(
                        expression);
                    return segmentCtx.valueTypeRegistry.compileConstant(constExpression.getTypeId(), segmentCtx.builder,
                                                                   constExpression.getSerializedValue());
                }
                case basic::expression::ExpressionType::COMPARE: {
                    // NOLINTNEXTLINE
                    const auto &compareExpression = static_cast<const basic::expression::CompareExpression &>(
                        expression);
                    llvm::Value *left = codegenExpression(segmentCtx, tuple, compareExpression.getLeftPart());
                    llvm::Value *right = codegenExpression(segmentCtx, tuple, compareExpression.getRightPart());
                    return codegenCompare(segmentCtx.builder, left, right, compareExpression.getOpr());
                }
                case basic::expression::ExpressionType::LOGICAL: {
                    // NOLINTNEXTLINE
                    const auto &logicalExpression = static_cast<const basic::expression::LogicalExpression &>(
                        expression);
                    llvm::Value *left = codegenExpression(segmentCtx, tuple, logicalExpression.getLeftPart());
                    if (logicalExpression.isUnary()) {
                        return segmentCtx.builder.CreateNot(left, "not");
                    }
                    llvm::Value *right = codegenExpression(segmentCtx, tuple, *logicalExpression.getRightPart());
                    switch (logicalExpression.getOpr()) {
                        case basic::expression::LogicalExpression::LogicalOp::AND: {
                            return segmentCtx.builder.CreateAnd(left, right, "and");
                        }
                        case basic::expression::LogicalExpression::LogicalOp::OR: {
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

        llvm::Value *SelectOperator::codegenCompare(llvm::IRBuilder<> &builder, llvm::Value *left, llvm::Value *right,
                                           const basic::expression::CompareExpression::CompareOp opr) {
            const bool isFloat = left->getType()->isFloatingPointTy();
            // NOLINTBEGIN(clang-analyzer-security.ArrayBound)
            switch (opr) {
                case basic::expression::CompareExpression::CompareOp::EQ: {
                    return isFloat ? builder.CreateFCmpOEQ(left, right, "eq") : builder.CreateICmpEQ(left, right, "eq");
                }
                case basic::expression::CompareExpression::CompareOp::NE: {
                    return isFloat ? builder.CreateFCmpONE(left, right, "ne") : builder.CreateICmpNE(left, right, "ne");
                }
                case basic::expression::CompareExpression::CompareOp::LT: {
                    return isFloat
                               ? builder.CreateFCmpOLT(left, right, "lt")
                               : builder.CreateICmpSLT(left, right, "lt");
                }
                case basic::expression::CompareExpression::CompareOp::LE: {
                    return isFloat
                               ? builder.CreateFCmpOLE(left, right, "le")
                               : builder.CreateICmpSLE(left, right, "le");
                }
                case basic::expression::CompareExpression::CompareOp::GT: {
                    return isFloat
                               ? builder.CreateFCmpOGT(left, right, "gt")
                               : builder.CreateICmpSGT(left, right, "gt");
                }
                case basic::expression::CompareExpression::CompareOp::GE: {
                    return isFloat
                               ? builder.CreateFCmpOGE(left, right, "ge")
                               : builder.CreateICmpSGE(left, right, "ge");
                }
                default:
                    throw std::invalid_argument("SelectOperator: unsupported compare type");
            }
            // NOLINTEND(clang-analyzer-security.ArrayBound)
        }
}