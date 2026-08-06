#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_IO_FORMAT_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_IO_FORMAT_HPP

#include <cstdint>

namespace sierra::odyplan::v3::basic::io {
    using IOFormatType = std::uint8_t;

    enum class BuiltinIOFormat : IOFormatType {
        JSON = 0,
        NDJSON = 1,
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_IO_IO_FORMAT_HPP