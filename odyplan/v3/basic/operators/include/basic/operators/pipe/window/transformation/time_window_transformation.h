#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_TRANSFORMATION_TIME_WINDOW_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_TRANSFORMATION_TIME_WINDOW_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/pipe/window/logical/time_window_operator.h>
#include <basic/operators/pipe/window/physical/time_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::transformation {
    static constexpr auto TIME_WINDOW_OPERATOR_TYPE_ID = logical::TimeWindowOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TIME_WINDOW_TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::pipe::window::logical::TimeWindowOperator&>(op);
        const auto& config = concrete.getWindowConfig();

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::window::physical::TimeWindowOperator>(inputPorts.front(), outputPorts.front(), config));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_TRANSFORMATION_TIME_WINDOW_TRANSFORMATION_HPP