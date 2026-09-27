#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_TRANSFORMATION_UNION_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_TRANSFORMATION_UNION_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/pipe/union/logical/union_operator.h>
#include <basic/operators/pipe/union/physical/union_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::transformation {// NOLINT(readability-identifier-naming)
    static constexpr auto OPERATOR_TYPE_ID = logical::UnionOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::pipe::union_::logical::UnionOperator&>(op);

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::union_::physical::UnionOperator>(
            inputPorts.front(), outputPorts.front(), concrete.getOutputPorts().front(), concrete.isOutOfOrder()
            ));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_UNION_TRANSFORMATION_UNION_TRANSFORMATION_HPP