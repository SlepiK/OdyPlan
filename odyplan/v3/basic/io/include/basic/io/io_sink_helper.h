#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_IO_SINK_HELPER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_IO_SINK_HELPER_HPP

#include <utility>
#include <cstdint>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::basic::io::detail {
    template<typename JsonType>
    inline void writeToJson(JsonType target,
        const char* key,
        const core::shared::types::ValueTypeId typeId,
        const void* rawData)
    {
        using T = core::shared::types::BuiltinValueType;

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        switch (typeId) {
            case std::to_underlying(T::BOOL):
                target[key] = *static_cast<const bool*>(rawData);
                break;
            case std::to_underlying(T::INT8):
                target[key] = *static_cast<const std::int8_t*>(rawData);
                break;
            case std::to_underlying(T::INT16):
                target[key] = *static_cast<const std::int16_t*>(rawData);
                break;
            case std::to_underlying(T::INT32):
                target[key] = *static_cast<const std::int32_t*>(rawData);
                break;
            case std::to_underlying(T::INT64):
                target[key] = *static_cast<const std::int64_t*>(rawData);
                break;
            case std::to_underlying(T::FLOAT):
                target[key] = *static_cast<const std::float_t*>(rawData);
                break;
            case std::to_underlying(T::DOUBLE):
                target[key] = *static_cast<const std::double_t*>(rawData);
                break;
            case std::to_underlying(T::STRING):
                target[key] = static_cast<const char*>(rawData);
                break;
            default:
                throw std::invalid_argument("SinkWriteHelper: Unknown typeId " +
                                            std::to_string(typeId));
        }
        // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    }
}  // namespace sierra::odyplan::v3::basic::io

#endif  // SIERRA_ODYPLAN_V3_BASIC_IO_IO_SINK_HELPER_HPP
