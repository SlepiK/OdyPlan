#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_TRANSFORMATION_OPERATORTRANSFORMATION_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_TRANSFORMATION_OPERATORTRANSFORMATION_H

#include <vector>
#include <memory>
#include "core/physical/operator/operator.h"
#include "core/common/edge/edge.h"
#include "core/logical/operator/operator.h"

namespace sierra::odyplan::v2::core::physical::transformation {
    struct OperatorTransformation {
        std::vector<std::unique_ptr<opr::IOperator>> oprs;
    };

    using OperatorTransformationFactory = std::function<
        OperatorTransformation(
            const logical::opr::IOperator&,
            const port::Ports& input,
            const port::Ports& output
        )>;
}  // namespace sierra::odyplan::v2::core::physical::transformation

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_TRANSFORMATION_OPERATORTRANSFORMATION_H