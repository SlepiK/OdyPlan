#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_DETAILS_READ_OPERATOR_OPTIONS_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_DETAILS_READ_OPERATOR_OPTIONS_HPP

#include <optional>
#include <chrono>

namespace sierra::odyplan::v3::basic::operators::source::read::details {
    template <typename FieldRef>
    struct ReadOperatorOptions {
        std::optional<FieldRef> startTimestampPosition;
        std::optional<FieldRef> endTimestampPosition;
        std::chrono::milliseconds delay = std::chrono::milliseconds(0);
        std::chrono::milliseconds allowedLateness = std::chrono::milliseconds(0);
        bool useProgramClock = false;
    };

    using LReadOperatorOptions = ReadOperatorOptions<std::string>;
    using PReadOperatorOptions = ReadOperatorOptions<std::size_t>;

    inline PReadOperatorOptions resolve(const LReadOperatorOptions& logicalOpts, const core::logical::port::Port& schema)
    {
        auto resolveIndex = [&schema](const std::optional<std::string>& name) -> std::optional<std::size_t> {
            if (!name.has_value()) {
                return std::nullopt;
            }
            auto it = std::ranges::find(schema, *name, &core::logical::port::PortDescription::fieldName);
            if (it == schema.end()) {
                throw std::runtime_error("ReadOperatorOptions: field '" + *name + "' not found in schema");
            }
            return static_cast<std::size_t>(std::distance(schema.begin(), it));
        };

        return PReadOperatorOptions{
            .startTimestampPosition = resolveIndex(logicalOpts.startTimestampPosition),
            .endTimestampPosition = resolveIndex(logicalOpts.endTimestampPosition),
            .delay = logicalOpts.delay,
            .allowedLateness = logicalOpts.allowedLateness,
            .useProgramClock = logicalOpts.useProgramClock
        };
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_DETAILS_READ_OPERATOR_OPTIONS_HPP