#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_LOGICAL_H

#include <utility>

#include "window_operator_base.h"
#include <common/window/count/count_window_config.h>

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class CountWindowOperator final : public WindowOperatorBase {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::WINDOW_COUNT);

        explicit CountWindowOperator(const window::count::CountWindowConfig &windowConfig)
            : mWindowConfig(windowConfig) {
            checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide);
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] const window::count::CountWindowConfig &getWindowConfig() const {
            return mWindowConfig;
        }

    protected:
        void validateInput() override {
        }

    private:
        window::count::CountWindowConfig mWindowConfig;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_LOGICAL_H