#ifndef SIERRA_ODYPLAN_V3_CORE_BUILDER_TRANSFORMATION_OPERATOR_TRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_CORE_BUILDER_TRANSFORMATION_OPERATOR_TRANSFORMATION_HPP

#include <core/physical/operators/operator.h>

namespace sierra::odyplan::v3::core::builder::transformation {
    struct OperatorTransformation {
        std::vector<std::unique_ptr<physical::operators::IOperator>> operators;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_BUILDER_TRANSFORMATION_OPERATOR_TRANSFORMATION_HPP