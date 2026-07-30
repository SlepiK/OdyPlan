#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITE_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITE_OPERATOR_LOGICAL_H

#include <utility>
#include <nlohmann/json.hpp>

#include "common/io/io_format.h"
#include "core/common/identifier/operator.h"
#include "core/logical/operator/sink/sink_operator.h"

namespace sierra::odyplan::v2::common::opr::sink::logical {
    class WriteOperator : public core::logical::opr::TypedSinkOperator {
    public:
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::WRITE);

        explicit WriteOperator(const io::IOFormat ioFormat, nlohmann::json ioConfig)
            : mIOFormat(ioFormat),
              mIOConfig(std::move(ioConfig)) {
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

    protected:
        void validateInput() override {
        }

    private:
        io::IOFormat mIOFormat;
        nlohmann::json mIOConfig;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITE_OPERATOR_LOGICAL_H