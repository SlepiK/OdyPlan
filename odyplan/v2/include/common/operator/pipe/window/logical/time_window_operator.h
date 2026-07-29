#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_LOGICAL_H

#include <utility>

#include "window_operator_base.h"
#include <common/window/time/time_window_config.h>

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class TimeWindowOperator final : public WindowOperatorBase {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::WINDOW_TIME);

        explicit TimeWindowOperator(const window::time::TimeWindowConfig &windowConfig)
            : mWindowConfig(windowConfig) {
            checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide);
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] const window::time::TimeWindowConfig &getWindowConfig() const {
            return mWindowConfig;
        }

    protected:
        void validateInput() override {
            if (mWindowConfig.allowedLateness.count() < 0) {
                throw std::invalid_argument("allowedLateness must be non-negative");
            }

            if (mWindowConfig.timestampIndex >= getInputPorts().at(0).size()) {
                throw std::invalid_argument("timestamp index out of range");
            }

            if (getInputPorts().at(0).at(mWindowConfig.timestampIndex).typeId != std::to_underlying(
                    core::common::identifier::Type::INT64)) {
                throw std::invalid_argument("timestamp must be int64");
            }
        }

    private:
        window::time::TimeWindowConfig mWindowConfig;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_LOGICAL_H