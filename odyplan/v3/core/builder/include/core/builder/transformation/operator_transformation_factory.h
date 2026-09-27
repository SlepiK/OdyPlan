#ifndef SIERRA_ODYPLAN_V3_CORE_BUILDER_TRANSFORMATION_OPERATOR_TRANSFORMATION_FACTORY_HPP
#define SIERRA_ODYPLAN_V3_CORE_BUILDER_TRANSFORMATION_OPERATOR_TRANSFORMATION_FACTORY_HPP

#include <core/builder/transformation/operator_transformation.h>
#include <core/logical/operators/operator.h>
#include <core/physical/port/port.h>

namespace sierra::odyplan::v3::core::builder::transformation {
    using OperatorTransformationFactory =
        OperatorTransformation (*)(logical::operators::IOperator&, const physical::port::Ports&, const physical::port::Ports&);
}

#endif  // SIERRA_ODYPLAN_V3_CORE_BUILDER_TRANSFORMATION_OPERATOR_TRANSFORMATION_FACTORY_HPP