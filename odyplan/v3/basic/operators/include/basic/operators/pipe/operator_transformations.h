#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_OPERATORTRANSFORMATION_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_OPERATORTRANSFORMATION_HPP

#include <core/builder/plan_builder.h>

#include <basic/operators/pipe/join/transformation/join_transformation.h>
#include <basic/operators/pipe/measure/transformation/measure_transformation.h>
#include <basic/operators/pipe/merge/transformation/merge_transformation.h>
#include <basic/operators/pipe/project/transformation/project_transformation.h>
#include <basic/operators/pipe/select/transformation/select_transformation.h>
#include <basic/operators/pipe/window/transformation/count_window_transformation.h>
#include <basic/operators/pipe/window/transformation/time_window_transformation.h>

namespace sierra::odyplan::v3::basic::operators::pipe {
    static void registerTransformations(sierra::odyplan::v3::core::builder::PlanBuilder& builder) {
        builder.registerOperatorTransformation(pipe::join::transformation::OPERATOR_TYPE_ID, pipe::join::transformation::TRANSFORMATION);
        builder.registerOperatorTransformation(pipe::measure::transformation::OPERATOR_TYPE_ID, pipe::measure::transformation::TRANSFORMATION);
        builder.registerOperatorTransformation(pipe::merge::transformation::OPERATOR_TYPE_ID, pipe::merge::transformation::TRANSFORMATION);
        builder.registerOperatorTransformation(pipe::project::transformation::OPERATOR_TYPE_ID, pipe::project::transformation::TRANSFORMATION);
        builder.registerOperatorTransformation(pipe::select::transformation::OPERATOR_TYPE_ID, pipe::select::transformation::TRANSFORMATION);
        builder.registerOperatorTransformation(pipe::window::transformation::COUNT_WINDOW_OPERATOR_TYPE_ID, pipe::window::transformation::COUNT_WINDOW_TRANSFORMATION);
        builder.registerOperatorTransformation(pipe::window::transformation::TIME_WINDOW_OPERATOR_TYPE_ID, pipe::window::transformation::TIME_WINDOW_TRANSFORMATION);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_OPERATORTRANSFORMATION_HPP