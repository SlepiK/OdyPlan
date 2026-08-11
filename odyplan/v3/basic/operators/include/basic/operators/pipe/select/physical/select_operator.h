#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_PHYSICAL_SELECT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_PHYSICAL_SELECT_OPERATOR_HPP

#include <basic/expression/compare_expression.h>
#include <basic/expression/constant_expression.h>
#include <basic/expression/expression.h>
#include <basic/expression/field_reference_expression.h>
#include <basic/expression/logical_expression.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <core/physical/types/value_type_registry.h>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::select::physical {
    class SelectOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        SelectOperator(core::physical::port::Port inputPort,
                       core::physical::port::Port outputPort,
                       const basic::expression::IExpression& predicate);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

        void end(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId inputPort,
                 const core::physical::segment::SegmentDataConsumers& consumers) override;

    private:
        const basic::expression::IExpression& mPredicate;

        static llvm::Value* codegenExpression(core::physical::segment::SegmentContext& segmentCtx,
                                              const core::physical::data::DataPackage& tuple,
                                              const basic::expression::IExpression& expression);

        static llvm::Value* codegenCompare(llvm::IRBuilder<>& builder,
                                           llvm::Value* left,
                                           llvm::Value* right,
                                           basic::expression::CompareExpression::CompareOp opr);
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::select::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_PHYSICAL_SELECT_OPERATOR_HPP