#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_IO_SINK_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_IO_SINK_HPP

#include <cstdint>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::basic::io {
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
        virtual void write(const char* key, core::shared::types::ValueTypeId typeId, const void* rawData) = 0;
        virtual void end() = 0;
        virtual void flush() = 0;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_IO_IO_SINK_HPP