#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_TRANSFORMATION_PRINT_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_TRANSFORMATION_PRINT_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/sink/print/logical/print_operator.h>
#include <basic/operators/sink/print/physical/print_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::transformation {
    static constexpr auto OPERATOR_TYPE_ID = logical::PrintOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports&) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::sink::print::logical::PrintOperator&>(op);

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::sink::print::physical::PrintOperator>(inputPorts.front(),concrete.getPortFieldIndices(), concrete.getInputPorts().front(), concrete.getOptions()));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_TRANSFORMATION_PRINT_TRANSFORMATION_HPP