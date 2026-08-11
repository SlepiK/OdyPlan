#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_LOGICAL_WRITE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_LOGICAL_WRITE_OPERATOR_HPP

#include <nlohmann/json.hpp>

#include <basic/io/io_format.h>
#include <core/logical/operators/sink/typed_sink_operator.h>
#include <utility>

namespace sierra::odyplan::v3::basic::operators::sink::write::logical {
    class WriteOperator : public core::logical::operators::sink::TypedSinkOperator {
    public:
        static constexpr core::shared::types::OperatorTypeId TYPE_ID =
            std::to_underlying(core::shared::types::BuiltinOperatorType::WRITE);

        explicit WriteOperator(io::IOFormatType ioFormat, nlohmann::json ioConfig);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] io::IOFormatType getIOFormat() const;

        [[nodiscard]] const nlohmann::json& getIOConfig() const;

    protected:
        void validateInput() override;

    private:
        io::IOFormatType mIOFormat;
        nlohmann::json mIOConfig;
    };
}  // namespace sierra::odyplan::v3::basic::operators::sink::write::logical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_LOGICAL_WRITE_OPERATOR_HPP
