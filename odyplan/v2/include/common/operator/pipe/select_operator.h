#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECTOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECTOPERATOR_H

#include <string>
#include <vector>

#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"
#include "core/common/port/port_ref.h"
#include "core/common/expression/expression.h"
#include "core/common/type/type_registry.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    //ToDo: Mapping from Odysseus-Operator Project (https://wiki.odysseus.informatik.uni-oldenburg.de/spaces/ODYSSEUS/pages/7111226/Select+operator)
    namespace logical {
        class SelectOperator : public core::logical::opr::TypedPipeOperator {
        public:
            static constexpr core::common::type::TypeId TYPE_ID = 21;

            explicit SelectOperator(std::unique_ptr<core::common::expression::IExpression> predicate)
                : mPredicate(std::move(predicate)) {
            }

            [[nodiscard]] core::common::type::TypeId getTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const core::common::expression::IExpression& getPredicate() const {
                return *mPredicate;
            }

        protected:
            void validate() override {
                if (mPredicate == nullptr) {
                    throw std::invalid_argument("SelectOperator: predicate must be non-null");
                }
            }

        private:
            std::unique_ptr<core::common::expression::IExpression> mPredicate;
        };
    }

    namespace physical {
        class SelectOperator : public core::physical::opr::TypedPipeOperator {
        public:
            SelectOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, const core::common::expression::IExpression& predicate, const core::common::type::TypeRegistry& typeRegistry)
                : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)}, core::physical::port::Ports{std::move(outputPort)}),
                    mPredicate(predicate),
                    mRegistry(typeRegistry) {
            }

            void consume(llvm::IRBuilder<> &builder, const core::physical::port::PortTuple &tuple, core::physical::port::PortTupleConsumer consumer) override {

            }

        private:
            const core::common::expression::IExpression& mPredicate;
            const core::common::type::TypeRegistry& mRegistry;

            llvm::Value* codegenExpression(llvm::IRBuilder<>& builder, const core::physical::port::PortTuple& tuple, const core::common::expression::IExpression& expression) {
                switch (expression.getExpressionType()) {
                    case core::common::expression::ExpressionType::FIELDREFERENCE: {
                        const auto& fieldRefExpression = static_cast<const core::common::expression::FieldReferenceExpression&>(expression);
                        if (fieldRefExpression.getFieldIndex() >= tuple.size()) {
                            throw std::out_of_range("SelectOperator: field index out of range");
                        }
                        return tuple[fieldRefExpression.getFieldIndex()];
                    }
                    case core::common::expression::ExpressionType::CONSTANT: {
                        const auto& constExpression = static_cast<const core::common::expression::ConstantExpression&>(expression);
                        return mRegistry.compileConstant(constExpression.getTypeId(), builder, constExpression.getSerializedValue());
                    }
                    case core::common::expression::ExpressionType::COMPARE: {
                        const auto& compareExpression = static_cast<const core::common::expression::CompareExpression&>(expression);
                        llvm::Value* left = codegenExpression(builder, tuple, compareExpression.getLeftPart());
                        llvm::Value* right = codegenExpression(builder, tuple, compareExpression.getRightPart());
                        return codegenCompare(builder, left, right, compareExpression.getOpr());
                    }
                    case core::common::expression::ExpressionType::LOGICAL: {
                        const auto& logicalExpression = static_cast<const core::common::expression::LogicalExpression&>(expression);
                        llvm::Value* left = codegenExpression(builder, tuple, logicalExpression.getLeftPart());
                        if (logicalExpression.isUnary())
                            return builder.CreateNot(left, "not");
                        llvm::Value* right = codegenExpression(builder, tuple, *logicalExpression.getRightPart());
                        switch (logicalExpression.getOpr()) {
                            case core::common::expression::LogicalExpression::LogicalOp::AND: {
                                return builder.CreateAnd(left, right, "and");
                            }
                            case core::common::expression::LogicalExpression::LogicalOp::OR: {
                                return builder.CreateOr(left, right, "or");
                            }
                            default:
                                throw std::invalid_argument("SelectOperator: unsupported logical expression type");
                        }
                    }
                    default:
                        throw std::invalid_argument("SelectOperator: unsupported expression type");
                }
            }

            llvm::Value* codegenCompare(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right, core::common::expression::CompareExpression::CompareOp opr) {
                bool isFloat = left->getType()->isFloatingPointTy();
                switch (opr) {
                    case core::common::expression::CompareExpression::CompareOp::EQ: {
                        return isFloat ? builder.CreateFCmpOEQ(left, right, "eq") : builder.CreateICmpEQ(left, right, "eq");
                    }
                    case core::common::expression::CompareExpression::CompareOp::NE: {
                        return isFloat ? builder.CreateFCmpONE(left, right, "ne") : builder.CreateICmpNE(left, right, "ne");
                    }
                    case core::common::expression::CompareExpression::CompareOp::LT: {
                        return isFloat ? builder.CreateFCmpOLT(left, right, "lt") : builder.CreateICmpSLT(left, right, "lt");
                    }
                    case core::common::expression::CompareExpression::CompareOp::LE: {
                        return isFloat ? builder.CreateFCmpOLE(left, right, "le") : builder.CreateICmpSLE(left, right, "le");
                    }
                    case core::common::expression::CompareExpression::CompareOp::GT: {
                        return isFloat ? builder.CreateFCmpOGT(left, right, "gt") : builder.CreateICmpSGT(left, right, "gt");
                    }
                    case core::common::expression::CompareExpression::CompareOp::GE: {
                        return isFloat ? builder.CreateFCmpOGE(left, right, "ge") : builder.CreateICmpSGE(left, right, "ge");
                    }
                    default:
                        throw std::invalid_argument("SelectOperator: unsupported compare type");
                }
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECTOPERATOR_H