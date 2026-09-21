#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_TRANSFORMATION_MERGE_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_TRANSFORMATION_MERGE_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/pipe/merge/logical/merge_operator.h>
#include <basic/operators/pipe/merge/physical/merge_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::transformation {
    static constexpr auto OPERATOR_TYPE_ID = logical::MergeOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        // NOLINTNEXTLINE
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::pipe::merge::logical::MergeOperator&>(op);

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::merge::physical::MergeOperator>(
            inputPorts.front(), outputPorts.front()
            ));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_TRANSFORMATION_MERGE_TRANSFORMATION_HPP