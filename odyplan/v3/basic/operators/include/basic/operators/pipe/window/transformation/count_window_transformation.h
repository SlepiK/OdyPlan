#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_TRANSFORMATION_COUNT_WINDOW_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_TRANSFORMATION_COUNT_WINDOW_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/pipe/window/logical/count_window_operator.h>
#include <basic/operators/pipe/window/physical/count_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::transformation {
    static constexpr auto COUNT_WINDOW_OPERATOR_TYPE_ID = logical::CountWindowOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory COUNT_WINDOW_TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::pipe::window::logical::CountWindowOperator&>(op);
        const auto& config = concrete.getWindowConfig();

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::window::physical::CountWindowOperator>(
            inputPorts.front(),
            outputPorts.front(),
            config,
            concrete.getInputPorts().front(),
            concrete.isOutOfOrder()));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_TRANSFORMATION_COUNT_WINDOW_TRANSFORMATION_HPP