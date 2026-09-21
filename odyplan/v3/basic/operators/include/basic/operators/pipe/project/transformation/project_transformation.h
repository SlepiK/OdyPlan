#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_TRANSFORMATION_PROJECT_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_TRANSFORMATION_PROJECT_TRANSFORMATION_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <basic/operators/pipe/project/logical/project_operator.h>
#include <basic/operators/pipe/project/physical/project_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::project::transformation {
    static constexpr auto OPERATOR_TYPE_ID = logical::ProjectOperator::TYPE_ID;
    static constexpr core::builder::transformation::OperatorTransformationFactory TRANSFORMATION = [](sierra::odyplan::v3::core::logical::operators::IOperator& op,
            const sierra::odyplan::v3::core::physical::port::Ports& inputPorts,
            const sierra::odyplan::v3::core::physical::port::Ports& outputPorts) -> sierra::odyplan::v3::core::builder::transformation::OperatorTransformation {
        const auto& concrete = static_cast<const sierra::odyplan::v3::basic::operators::pipe::project::logical::ProjectOperator&>(op);

        sierra::odyplan::v3::core::builder::transformation::OperatorTransformation ot;
        ot.operators.push_back(std::make_unique<sierra::odyplan::v3::basic::operators::pipe::project::physical::ProjectOperator>(inputPorts.front(), outputPorts.front(), concrete.getMapping()));
        return ot;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_TRANSFORMATION_PROJECT_TRANSFORMATION_HPP