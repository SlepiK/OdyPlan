#include <basic/operators/source/read/logical/read_operator.h>

namespace sierra::odyplan::v3::basic::operators::source::read::logical {
    ReadOperator::ReadOperator(core::logical::port::Port outputPort,
                               const io::IOFormatType ioFormat,
                               nlohmann::json ioConfig,
                               const std::chrono::milliseconds delay)
        : TypedSourceOperator(std::move(outputPort)),
          mIOFormat(ioFormat),
          mIOConfig(std::move(ioConfig)),
          mDelay(delay) {}

    core::shared::types::OperatorTypeId ReadOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    io::IOFormatType ReadOperator::getIOFormat() const {
        return mIOFormat;
    }

    const nlohmann::json& ReadOperator::getIOConfig() const {
        return mIOConfig;
    }

    std::chrono::milliseconds ReadOperator::getDelay() const {
        return mDelay;
    }
}  // namespace sierra::odyplan::v3::basic::operators::source::read::logical