#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_TRANSFORMATION_READ_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_TRANSFORMATION_READ_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/source/read/logical/read_operator.h>
#include <basic/operators/source/read/physical/read_operator.h>

namespace sierra::odyplan::v3::basic::operators::source::read::transformation {
    static constexpr auto OPERATOR_TYPE_ID = logical::ReadOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports&,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::source::read::logical::ReadOperator&>(op);

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::source::read::physical::ReadOperator>(
            outputPorts.front(),
            concrete.getIOConfig(),
            concrete.getIOFormat(),
            concrete.getOutputPorts().at(0),
            concrete.getOptions()
            ));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_TRANSFORMATION_READ_TRANSFORMATION_HPP