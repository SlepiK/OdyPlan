#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_IOSINK_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_IOSINK_H

#include <cstdint>
#include "core/common/identifier/type.h"

namespace sierra::odyplan::v2::common::io {
    using IOSinkId = std::uint64_t;

    class IIOSink {
    public:
        IIOSink() = default;
        virtual ~IIOSink() = default;
        IIOSink(IIOSink const&) = delete;
        IIOSink& operator =(IIOSink const&) = delete;
        IIOSink(IIOSink&&) = delete;
        IIOSink& operator=(IIOSink&&) = delete;

        virtual void begin() = 0;
        virtual void write(const char* key, core::common::identifier::TypeId typeId, const void* rawData) = 0;
        virtual void end() = 0;
        virtual void flush() = 0;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_IOSINK_H