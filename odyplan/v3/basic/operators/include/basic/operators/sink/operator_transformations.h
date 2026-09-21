#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_OPERATORTRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_OPERATORTRANSFORMATION_HPP

#include <core/builder/plan_builder.h>

#include <basic/operators/sink/write/transformation/write_transformation.h>
#include <basic/operators/sink/print/transformation/print_transformation.h>

namespace sierra::odyplan::v3::basic::operators::sink {
    static void registerTransformations(sierra::odyplan::v3::core::builder::PlanBuilder& builder) {
        builder.registerOperatorTransformation(sink::write::transformation::OPERATOR_TYPE_ID, sink::write::transformation::TRANSFORMATION);
        builder.registerOperatorTransformation(sink::print::transformation::OPERATOR_TYPE_ID, sink::print::transformation::TRANSFORMATION);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_OPERATORTRANSFORMATION_HPP