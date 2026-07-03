#ifndef SIERRA_ODYPLAN_V2_CORE_BUILDER_BUILDER_H
#define SIERRA_ODYPLAN_V2_CORE_BUILDER_BUILDER_H

#include "core/logical/plan/plan.h"
#include "core/physical/plan/plan.h"
#include "core/physical/port/port_type_registry.h"
#include "core/physical/transformation/operator_transformation.h"
#include "core/physical/operator/source/source_operator.h"


namespace sierra::odyplan::v2::core::builder {
    class PlanBuilder {
    public:
        explicit PlanBuilder(const physical::port::PortTypeRegistry& registry)
            : mRegistry(registry) {
        }

        void registerOperatorTransformation(common::identifier::OperatorTypeId lId, physical::transformation::OperatorTransformationFactory factory);

        [[nodiscard]] physical::plan::Plan build(const logical::plan::Plan& lPlan, llvm::LLVMContext& ctx);
    private:
        const physical::port::PortTypeRegistry& mRegistry;
        std::unordered_map<common::identifier::OperatorTypeId, physical::transformation::OperatorTransformationFactory> mFactories;

        [[nodiscard]] physical::port::Ports resolvePorts(const logical::port::Ports& lPorts, llvm::LLVMContext& ctx);
    };
}  // namespace sierra::odyplan::v2::core::builder

#endif  // SIERRA_ODYPLAN_V2_CORE_BUILDER_BUILDER_H