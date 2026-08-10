#include <basic/operators/sink/write/logical/write_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::write::logical {
    WriteOperator::WriteOperator(const io::IOFormatType ioFormat, nlohmann::json ioConfig)
        : mIOFormat(ioFormat), mIOConfig(std::move(ioConfig)) {}

    core::shared::types::OperatorTypeId WriteOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    [[nodiscard]] io::IOFormatType WriteOperator::getIOFormat() const {
        return mIOFormat;
    }

    [[nodiscard]] const nlohmann::json& WriteOperator::getIOConfig() const {
        return mIOConfig;
    }

    void WriteOperator::validateInput() {}
}  // namespace sierra::odyplan::v3::basic::operators::sink::write::logical