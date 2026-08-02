#ifndef SIERRA_ODYPLAN_V3_CORE_BUILDER_PLAN_BUILDER_HPP
#define SIERRA_ODYPLAN_V3_CORE_BUILDER_PLAN_BUILDER_HPP

#include <core/physical/types/value_type_registry.h>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::core::builder {
    class PlanBuilder {
    public:
        using ValueTypeRegistry = physical::types::ValueTypeRegistry;
        using ValueTypeId = shared::types::ValueTypeId;

        explicit PlanBuilder(const ValueTypeRegistry& registry);

        //void registerOperatorTransformation(common::identifier::TypeId lId, physical::transformation::OperatorTransformationFactory factory);

        //[[nodiscard]] physical::plan::Plan build(const logical::plan::Plan& lPlan, llvm::LLVMContext& ctx);
    private:
        const ValueTypeRegistry* mRegistry;
        //std::unordered_map<common::identifier::TypeId, physical::transformation::OperatorTransformationFactory> mFactories;

        //[[nodiscard]] physical::port::Ports resolvePorts(const logical::port::Ports& lPorts, llvm::LLVMContext& ctx) const;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_BUILDER_PLAN_BUILDER_HPP