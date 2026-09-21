#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_OPERATORTRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_OPERATORTRANSFORMATION_HPP

#include <core/builder/plan_builder.h>

#include <basic/operators/source/read/transformation/read_transformation.h>

namespace sierra::odyplan::v3::basic::operators::source {
    static void registerTransformations(sierra::odyplan::v3::core::builder::PlanBuilder& builder) {
        builder.registerOperatorTransformation(source::read::transformation::OPERATOR_TYPE_ID, source::read::transformation::TRANSFORMATION);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_OPERATORTRANSFORMATION_HPP