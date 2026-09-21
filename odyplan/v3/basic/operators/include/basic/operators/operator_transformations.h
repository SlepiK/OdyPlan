#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_OPERATORTRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_OPERATORTRANSFORMATION_HPP

#include <core/builder/plan_builder.h>

#include <basic/operators/source/operator_transformations.h>
#include <basic/operators/sink/operator_transformations.h>
#include <basic/operators/pipe/operator_transformations.h>

namespace sierra::odyplan::v3::basic::operators {
    static void loadBuiltinOperatorTransformations(sierra::odyplan::v3::core::builder::PlanBuilder& builder) {
        source::registerTransformations(builder);
        sink::registerTransformations(builder);
        pipe::registerTransformations(builder);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_OPERATORTRANSFORMATION_HPP