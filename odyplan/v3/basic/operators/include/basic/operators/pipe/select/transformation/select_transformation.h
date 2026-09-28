#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_TRANSFORMATION_SELECT_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_TRANSFORMATION_SELECT_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/pipe/select/logical/select_operator.h>
#include <basic/operators/pipe/select/physical/select_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::select::transformation {
    static constexpr auto OPERATOR_TYPE_ID = logical::SelectOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
           const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
           const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        auto& concrete = static_cast<sierra::odyplan::v3::basic::operators::pipe::select::logical::SelectOperator&>(op);
        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::select::physical::SelectOperator>(inputPorts.front(), outputPorts.front(), std::move(concrete.getPredicate())));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_SELECT_TRANSFORMATION_SELECT_TRANSFORMATION_HPP