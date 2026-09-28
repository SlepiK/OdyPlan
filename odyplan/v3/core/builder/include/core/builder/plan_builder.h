#ifndef SIERRA_ODYPLAN_V3_CORE_BUILDER_PLAN_BUILDER_HPP
#define SIERRA_ODYPLAN_V3_CORE_BUILDER_PLAN_BUILDER_HPP

#include <core/builder/transformation/operator_transformation_factory.h>
#include <core/logical/plan/plan.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <core/physical/operators/sink/typed_sink_operator.h>
#include <core/physical/operators/source/typed_source_operator.h>
#include <core/physical/plan/plan.h>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::core::builder {
    class PlanBuilder {
    public:
        using CodegenTypeRegistry = physical::types::CodegenTypeRegistry;
        using OperatorTypeId = shared::types::OperatorTypeId;

        PlanBuilder();
        explicit PlanBuilder(const CodegenTypeRegistry& codengenTypeRegistry);

        void registerOperatorTransformation(OperatorTypeId oprTypeId,
                                            transformation::OperatorTransformationFactory factory);

        [[nodiscard]] physical::plan::Plan build(const logical::plan::Plan& lPlan,
                                                 llvm::LLVMContext& ctx);
    private:
        const CodegenTypeRegistry* mCodegenTypeRegistry;
        std::unordered_map<OperatorTypeId, transformation::OperatorTransformationFactory> mFactories;

        [[nodiscard]] physical::port::Ports resolvePorts(const logical::port::Ports& lPorts,
                                                         llvm::LLVMContext& ctx) const;
    };
}  // namespace sierra::odyplan::v3::core::builder

#endif  // SIERRA_ODYPLAN_V3_CORE_BUILDER_PLAN_BUILDER_HPP