#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_TRANSFORMATION_JOIN_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_TRANSFORMATION_JOIN_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/pipe/join/logical/join_operator.h>
#include <basic/operators/pipe/join/physical/join_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::transformation {
    static constexpr auto OPERATOR_TYPE_ID = logical::JoinOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        auto& concrete = static_cast<sierra::odyplan::v3::basic::operators::pipe::join::logical::JoinOperator&>(op);

        const sierra::odyplan::v3::core::logical::port::Ports schemas{
            concrete.getInputPorts().at(0), concrete.getInputPorts().at(1)
        };

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::join::physical::JoinOperator>(
            inputPorts, outputPorts.front(), concrete.getConfig(), schemas, std::move(concrete.getPredicate()), concrete.isOutOfOrder()
        ));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_TRANSFORMATION_JOIN_TRANSFORMATION_HPP