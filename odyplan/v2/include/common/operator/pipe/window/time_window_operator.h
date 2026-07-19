#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEWINDOWOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEWINDOWOPERATOR_H

#include <common/operator/pipe/window/window_operator_base.h>
#include <common/window/time/time_window_config.h>

namespace sierra::odyplan::v2::common::opr::pipe {
    namespace logical {
        class TimeWindowOperator final : public WindowOperatorBase {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::WINDOW_TIME);

            explicit TimeWindowOperator(const window::time::TimeWindowConfig &windowConfig)
                : mWindowConfig(windowConfig) {
                checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide);
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const window::time::TimeWindowConfig& getWindowConfig() const {
                return mWindowConfig;
            }
        protected:
            void validate() override {

                if (mWindowConfig.allowedLateness.count() < 0) {
                    throw std::invalid_argument("allowedLateness must be non-negative");
                }

                if (mWindowConfig.timestampIndex >= getInputPorts().at(0).size()) {
                    throw std::invalid_argument("timestamp index out of range");
                }

                if (getInputPorts().at(0).at(mWindowConfig.timestampIndex).typeId != std::to_underlying(core::common::identifier::Type::INT64)) {
                    throw std::invalid_argument("timestamp must be int64");
                }
            }

        private:
            window::time::TimeWindowConfig mWindowConfig;
        };
    }

    namespace physical {
        class TimeWindowOperator final : public WindowOperatorBase {
        public:
            TimeWindowOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, const std::uint64_t windowHandle, const std::size_t timestampIndex)
                : WindowOperatorBase(std::move(inputPort), std::move(outputPort), windowHandle),
                    mTimestampIndex(timestampIndex) {
            }

        protected:
            llvm::Value* getTimestampValue(core::physical::segment::SegmentContext & /*segmentCtx*/, const core::physical::port::PortTuple &tuple, llvm::Type * /*i64Ty*/) override {
                return tuple.at(mTimestampIndex);
            }

        private:
            std::size_t mTimestampIndex;
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEWINDOWOPERATOR_H