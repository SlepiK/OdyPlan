#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_LOGICAL_READ_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_LOGICAL_READ_OPERATOR_HPP

#include <nlohmann/json.hpp>

#include <basic/io/io_format.h>
#include <core/logical/operators/source/typed_source_operator.h>

namespace sierra::odyplan::v3::basic::operators::source::read::logical {
    class ReadOperator : public core::logical::operators::source::TypedSourceOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::READ);

        ReadOperator(core::logical::port::Port outputPort,
                     io::IOFormatType ioFormat,
                     nlohmann::json ioConfig,
                     std::chrono::milliseconds delay = std::chrono::milliseconds(0));

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] io::IOFormatType getIOFormat() const;

        [[nodiscard]] const nlohmann::json& getIOConfig() const;

        [[nodiscard]] std::chrono::milliseconds getDelay() const;

    private:
        io::IOFormatType mIOFormat;
        nlohmann::json mIOConfig;
        std::chrono::milliseconds mDelay;
    };
}  // namespace sierra::odyplan::v3::basic::operators::source::read::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_LOGICAL_READ_OPERATOR_HPP
