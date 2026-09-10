#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_LOGICAL_READ_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_LOGICAL_READ_OPERATOR_HPP

#include <nlohmann/json.hpp>

#include <basic/io/io_format.h>
#include <basic/io/io_registry.h>
#include <core/logical/operators/source/typed_source_operator.h>
#include <basic/operators/source/read/details/read_operator_options.h>

namespace sierra::odyplan::v3::basic::operators::source::read::logical {
    class ReadOperator : public core::logical::operators::source::TypedSourceOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::READ);

        ReadOperator(core::logical::port::Port outputPort,
                     io::IOFormatType ioFormat,
                     nlohmann::json ioConfig,
                     details::LReadOperatorOptions options);

        static std::unique_ptr<ReadOperator> fromAutoSchema(io::IOFormatType ioFormat,
                     nlohmann::json ioConfig,
                     details::LReadOperatorOptions options);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] io::IOFormatType getIOFormat() const;

        [[nodiscard]] const nlohmann::json& getIOConfig() const;

        [[nodiscard]] details::PReadOperatorOptions getOptions() const;

    private:
        io::IOFormatType mIOFormat;
        nlohmann::json mIOConfig;
        details::LReadOperatorOptions mOptions;

    protected:
        void validate() const override;
    };
}  // namespace sierra::odyplan::v3::basic::operators::source::read::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_LOGICAL_READ_OPERATOR_HPP
