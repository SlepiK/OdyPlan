#include <basic/expression/physical/expression_codegen.h>

namespace sierra::odyplan::v3::basic::expression::physical {
    llvm::Value*
    codegenExpression(core::physical::segment::SegmentContext& segmentCtx,
                                    const core::physical::data::DataPackage& tuple,
                                    const basic::expression::physical::IExpression& expression) {
        switch (expression.getExpressionType()) {
            case basic::expression::physical::ExpressionType::FIELD_REFERENCE: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& fieldRef =
                    static_cast<const basic::expression::physical::FieldReferenceExpression&>(expression);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                if (fieldRef.getFieldIndex() >= tuple.size()) {
                    throw std::out_of_range("Field index out of range");
                }
                auto* field = tuple.at(fieldRef.getFieldIndex());
                if (field == nullptr) {
                    throw std::out_of_range("Field returned nullptr");
                }
                return field;
            }
            case basic::expression::physical::ExpressionType::CONSTANT: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& constExp =
                    static_cast<const basic::expression::physical::ConstantExpression&>(expression);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                return segmentCtx.codegenTypeRegistry.compileConstant(
                    constExp.getTypeId(), segmentCtx.builder, constExp.getSerializedValue());
            }
            case basic::expression::physical::ExpressionType::COMPARE: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& cmpExp =
                    static_cast<const basic::expression::physical::CompareExpression&>(expression);
                // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)

                llvm::Value* left = codegenExpression(segmentCtx, tuple, cmpExp.getLeftPart());
                if (left == nullptr) {
                    throw std::invalid_argument(
                        "failed to codegen left operand of compare expression");
                }

                llvm::Value* right = codegenExpression(segmentCtx, tuple, cmpExp.getRightPart());
                if (right == nullptr) {
                    throw std::invalid_argument(
                        "failed to codegen right operand of compare expression");
                }

                return codegenCompare(segmentCtx.builder, left, right, cmpExp.getOpr());
            }
            case basic::expression::physical::ExpressionType::LOGICAL: {
                // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                const auto& logicExp =
                    static_cast<const basic::expression::physical::LogicalExpression&>(expression);
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
                    case LogicalOp::AND: {
                        return segmentCtx.builder.CreateAnd(left, right);
                    }
                    case LogicalOp::OR: {
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

    llvm::Value* codegenCompare(llvm::IRBuilder<>& builder,
                                 llvm::Value* left,
                                 llvm::Value* right,
                                 const CompareOp opr) {
        if (left->getType() != right->getType()) {
            throw std::invalid_argument("expression operands have mismatched types");
        }
        const bool isFloat = left->getType()->isFloatingPointTy();

        // NOLINTBEGIN(clang-analyzer-security.ArrayBound)
        switch (opr) {
            case CompareOp::EQ: {
                return isFloat ? builder.CreateFCmpOEQ(left, right)
                               : builder.CreateICmpEQ(left, right);
            }
            case CompareOp::NE: {
                return isFloat ? builder.CreateFCmpONE(left, right)
                               : builder.CreateICmpNE(left, right);
            }
            case CompareOp::LT: {
                return isFloat ? builder.CreateFCmpOLT(left, right)
                               : builder.CreateICmpSLT(left, right);
            }
            case CompareOp::LE: {
                return isFloat ? builder.CreateFCmpOLE(left, right)
                               : builder.CreateICmpSLE(left, right);
            }
            case CompareOp::GT: {
                return isFloat ? builder.CreateFCmpOGT(left, right)
                               : builder.CreateICmpSGT(left, right);
            }
            case CompareOp::GE: {
                return isFloat ? builder.CreateFCmpOGE(left, right)
                               : builder.CreateICmpSGE(left, right);
            }
            default:
                throw std::out_of_range(
                    "compare: Invalid compare expression type");
        }
        // NOLINTEND(clang-analyzer-security.ArrayBound)
    }
}