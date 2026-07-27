#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_INTERVAL_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_INTERVAL_LOGICAL_H

#include <utility>

#include "common/join/interval_join_config.h"
#include "core/common/expression/expression.h"
#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class IntervalJoinOperator : public core::logical::opr::TypedPipeOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::INTERVAL_JOIN);

        explicit IntervalJoinOperator(join::IntervalJoinConfig config,
            const core::common::expression::IExpression* predicate = nullptr)
                : mConfig(std::move(config)), mPredicate(predicate) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] const join::IntervalJoinConfig& getConfig() const {
            return mConfig;
        }

        [[nodiscard]] const core::common::expression::IExpression* getPredicate() const {
            return mPredicate;
        }

    protected:
        void bindInput(const core::common::identifier::PortId portId, core::logical::port::Port input) override {
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

            if (mConfig.leftTimeStampIdx >= left.size() || mConfig.rightTimeStampIdx >= right.size()) {
                throw std::out_of_range("IntervalJoinOperator::validate: left or right index out of range");
            }

            if (left.at(mConfig.leftTimeStampIdx).typeId != std::to_underlying(core::common::identifier::Type::INT64) ||
                right.at(mConfig.rightTimeStampIdx).typeId != std::to_underlying(core::common::identifier::Type::INT64)) {
                throw std::invalid_argument("IntervalJoinOperator::validate: timestamp fields type are not allowed");
            }

            core::logical::port::Port combinedPort;
            combinedPort.reserve(left.size() + right.size());
            combinedPort.insert(combinedPort.begin(), left.begin(), left.end());
            combinedPort.insert(combinedPort.begin(), right.begin(), right.end());
            getOutputPortsRef() = {combinedPort};
        }

    private:
        join::IntervalJoinConfig mConfig;
        const core::common::expression::IExpression* mPredicate;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_JOIN_OPERATOR_INTERVAL_LOGICAL_H