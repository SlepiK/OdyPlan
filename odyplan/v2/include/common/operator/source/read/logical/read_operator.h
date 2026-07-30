#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READ_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READ_OPERATOR_LOGICAL_H

#include <nlohmann/json.hpp>
#include "common/io/io_format.h"
#include "core/logical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::common::opr::source::logical {
    class ReadOperator : public core::logical::opr::TypedSourceOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::READ);

        ReadOperator(core::logical::port::Port outputPort, const io::IOFormat ioFormat, nlohmann::json ioConfig, const std::chrono::milliseconds delay = std::chrono::milliseconds(0))
            : TypedSourceOperator(std::move(outputPort)),
              mIOFormat(ioFormat),
              mIOConfig(std::move(ioConfig)),
                mDelay(delay) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] io::IOFormat getIOFormat() const {
            return mIOFormat;
        }

        [[nodiscard]] const nlohmann::json &getIOConfig() const {
            return mIOConfig;
        }

        [[nodiscard]] std::chrono::milliseconds getDelay() const {
            return mDelay;
        }

    private:
        io::IOFormat mIOFormat;
        nlohmann::json mIOConfig;
        std::chrono::milliseconds mDelay;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READ_OPERATOR_LOGICAL_H