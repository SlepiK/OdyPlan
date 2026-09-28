#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_PHYSICAL_SELECT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_PHYSICAL_SELECT_OPERATOR_HPP

#include <basic/expression/physical/expression.h>
#include <basic/expression/physical/expression_codegen.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::pipe::select::physical {
    class SelectOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        SelectOperator(core::physical::port::Port inputPort,
                       core::physical::port::Port outputPort,
                       std::unique_ptr<basic::expression::physical::IExpression> predicate);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

    private:
        const std::unique_ptr<basic::expression::physical::IExpression> mPredicate;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::select::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_PHYSICAL_SELECT_OPERATOR_HPP