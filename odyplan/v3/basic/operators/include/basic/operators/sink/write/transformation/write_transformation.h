#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_TRANSFORMATION_WRITE_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_TRANSFORMATION_WRITE_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/sink/write/logical/write_operator.h>
#include <basic/operators/sink/write/physical/write_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::write::transformation {
    static constexpr auto OPERATOR_TYPE_ID = logical::WriteOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports&) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::sink::write::logical::WriteOperator&>(op);

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::sink::write::physical::WriteOperator>(
        inputPorts.front(),
        concrete.getIOConfig(),
        concrete.getIOFormat(),
            concrete.getInputPorts().at(0)
            ));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_TRANSFORMATION_WRITE_TRANSFORMATION_HPP