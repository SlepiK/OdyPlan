#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTWINDOWOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTWINDOWOPERATOR_H

#include <common/operator/pipe/window/window_operator_base.h>
#include <common/window/count/count_window_config.h>

namespace sierra::odyplan::v2::common::opr::pipe {
    namespace logical {
        class CountWindowOperator final : public WindowOperatorBase {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::WINDOW_COUNT);

            explicit CountWindowOperator(window::count::CountWindowConfig windowConfig)
                : mWindowConfig(std::move(windowConfig)) {
                checkBaseConfig(mWindowConfig.type, mWindowConfig.size, mWindowConfig.slide);
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const window::count::CountWindowConfig& getWindowConfig() const {
                return mWindowConfig;
            }
        protected:
            void validate() override {
            }

        private:
            window::count::CountWindowConfig mWindowConfig;
        };
    }

    namespace physical {
        class CountWindowOperator final : public WindowOperatorBase {
        public:
            using WindowOperatorBase::WindowOperatorBase;

        protected:
            llvm::Value* getTimestampValue(core::physical::segment::SegmentContext &segmentCtx, const core::physical::port::PortTuple &tuple, llvm::Type *i64Ty) override {
                return llvm::ConstantInt::get(i64Ty, 0);
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTWINDOWOPERATOR_H