#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_IOFORMAT_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_IOFORMAT_H

#include <cstdint>

namespace sierra::odyplan::v2::common::io {
    using IOFormatType = std::uint8_t;
    enum class IOFormat : IOFormatType {
        JSON = 0,
        NDJSON = 1,
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_IOFORMAT_H