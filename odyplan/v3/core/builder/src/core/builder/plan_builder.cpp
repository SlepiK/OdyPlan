#include <core/builder/plan_builder.h>

namespace sierra::odyplan::v3::core::builder {
    PlanBuilder::PlanBuilder(const ValueTypeRegistry& registry)
          : mRegistry(&registry) {
     }
}