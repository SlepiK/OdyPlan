#ifndef SIERRA_ODYPLAN_V2_CORE_BUILDER_BUILDER_H
#define SIERRA_ODYPLAN_V2_CORE_BUILDER_BUILDER_H

#include "core/logical/plan/plan.h"
#include "core/physical/plan/plan.h"
#include "../common/type/value/type_registry.h"
#include "core/physical/transformation/operator_transformation.h"
#include "core/physical/operator/source/source_operator.h"


namespace sierra::odyplan::v2::core::builder {
    class PlanBuilder {
    public:
        explicit PlanBuilder(const common::type::value::TypeRegistry& registry)
            : mRegistry(registry) {
        }

        void registerOperatorTransformation(common::type::value::TypeId lId, physical::transformation::OperatorTransformationFactory factory);

        [[nodiscard]] physical::plan::Plan build(const logical::plan::Plan& lPlan, llvm::LLVMContext& ctx);
    private:
        const common::type::value::TypeRegistry& mRegistry;
        std::unordered_map<common::type::value::TypeId, physical::transformation::OperatorTransformationFactory> mFactories;

        [[nodiscard]] physical::port::Ports resolvePorts(const logical::port::Ports& lPorts, llvm::LLVMContext& ctx);
    };
}  // namespace sierra::odyplan::v2::core::builder

#endif  // SIERRA_ODYPLAN_V2_CORE_BUILDER_BUILDER_H