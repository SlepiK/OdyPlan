#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_LOGICAL_H

#include <utility>

#include "common/operator/pipe/interval_join/physical/internal/interval_join_config.h"
#include "core/common/expression/expression.h"
#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class IntervalJoinOperator : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::INTERVAL_JOIN);

        explicit IntervalJoinOperator(physical::internal::IntervalJoinConfig config,
                                      const core::common::expression::IExpression* predicate = nullptr)
                : mConfig(std::move(config)), mPredicate(predicate) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] const physical::internal::IntervalJoinConfig& getConfig() const {
            return mConfig;
        }

        [[nodiscard]] const core::common::expression::IExpression* getPredicate() const {
            return mPredicate;
        }

    protected:
        void bindInput(const core::common::identifier::PortId portId, const core::logical::port::Port input) override {
            auto& inputPorts = getInputPortsRef();

            if (portId > 1) {
                throw std::logic_error("IntervalJoinOperator::bindInput: port id out of range (0 and 1 only allowed)");
            }

            if (inputPorts.size() <= portId) {
                inputPorts.resize(portId + 1);
            }

            inputPorts.at(portId) = input;
        }

        void validate() override {
            const auto& inputPorts = getInputPorts();
            if (inputPorts.size() != 2 || inputPorts.at(0).empty() || inputPorts.at(1).empty()) {
                throw std::logic_error("IntervalJoinOperator::validate: expects two inputs");
            }

            const auto& left = inputPorts.at(0);
            const auto& right = inputPorts.at(1);

            core::logical::port::Port combinedPort;
            combinedPort.reserve(left.size() + right.size());
            combinedPort.insert(combinedPort.end(), left.begin(), left.end());
            combinedPort.insert(combinedPort.end(), right.begin(), right.end());
            getOutputPortsRef() = {combinedPort};
        }

    private:
        physical::internal::IntervalJoinConfig mConfig;
        const core::common::expression::IExpression* mPredicate;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_LOGICAL_H