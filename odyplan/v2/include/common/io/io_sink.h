#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_IOSINK_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_IOSINK_H

#include <cstdint>

#include "io_data.h"

namespace sierra::odyplan::v2::common::io {
    using IOSinkId = std::uint64_t;

    class IIOSink {
    public:
        virtual ~IIOSink() = default;

        virtual void write(const IOData& data) = 0;
        virtual void flush() = 0;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_IOSINK_H