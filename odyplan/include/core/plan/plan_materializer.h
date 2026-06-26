#ifndef SIERRA_ODYPLAN_V1_CORE_PLAN_PLANMATERIALIZER_H
#define SIERRA_ODYPLAN_V1_CORE_PLAN_PLANMATERIALIZER_H
#include "plan.h"

namespace sierra::odyplan::v1::core::plan {
    class MaterializedPlan {
    public:
        MaterializedPlan(std::vector<std::unique_ptr<opr::IOperator>> operators, std::vector<std::size_t> sourceOperatorIndices)
            : mOperators(std::move(operators)),
            mSourceOperators(std::move(sourceOperatorIndices)) {
        }

        MaterializedPlan(const MaterializedPlan&) = delete;
        MaterializedPlan& operator=(const MaterializedPlan&) = delete;
        MaterializedPlan(MaterializedPlan&&) = default;
        MaterializedPlan& operator=(MaterializedPlan&&) = default;

        [[nodiscard]] std::size_t getOperatorCount() const {
            return mOperators.size();
        }

        [[nodiscard]] opr::IOperator& operatorAt(std::size_t index) {
            return *mOperators.at(index);
        }

        [[nodiscard]] const opr::IOperator& operatorAt(std::size_t index) const {
            return *mOperators.at(index);
        }

        [[nodiscard]] const std::vector<std::size_t>& getSourceOperators() const {
            return mSourceOperators;
        }

        void takeOwnership(std::unique_ptr<opr::flow::IEmitter> emitter) {
            mEmitters.push_back(std::move(emitter));
        }

    private:
        std::vector<std::unique_ptr<opr::IOperator>> mOperators;
        std::vector<std::size_t> mSourceOperators;
        std::vector<std::unique_ptr<opr::flow::IEmitter>> mEmitters;
    };

    class PlanMaterializer {
    public:
        PlanMaterializer() = default;

        [[nodiscard]] MaterializedPlan materialize(Plan&& plan) const {
            std::vector<edge::Edge*> edges;
            edges.reserve(plan.mEdges.size());
            for (auto& edge : plan.mEdges) {
                edges.push_back(edge.get());
            }

            MaterializedPlan materializedPlan(std::move(plan.mOperators), std::move(plan.mSourceOperators));

            for (auto* edge : edges) {
                auto* fromOperator = &materializedPlan.operatorAt(edge->from());
                auto* toOperator = &materializedPlan.operatorAt(edge->to());

                auto emitter = edge->bind(fromOperator, toOperator);

                if (!emitter) {
                    throw std::runtime_error("PlanMaterializer: failed to bind edge -- operator types do not match expected port payload");
                }

                materializedPlan.takeOwnership(std::move(emitter));
            }

            return materializedPlan;
        }
    };

}  // namespace sierra::odyplan::v1::core::plan

#endif  // SIERRA_ODYPLAN_V1_CORE_PLAN_PLANMATERIALIZER_H