#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_RUNTIME_WRITE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_RUNTIME_WRITE_RUNTIME_HPP

#include <basic/io/io_sink.h>
#include <basic/io/io_format.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v3::basic::operators::sink::write::runtime {
    extern "C" {
    void* rtIOSinkCreate(const void* typeRegistryRawPtr, io::IOFormatType ioFormat, const char* path);

    void rtIOSinkDestroy(void* state);

    void rtIOSinkBeginData(void* state);

    void rtIOSinkWrite(void* state,
                       core::shared::types::ValueTypeId typeId,
                       const char* key,
                       const void* value);

    void rtIOSinkEndData(void* state);

    void rtIOSinkFlush(void* state);
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::write::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_RUNTIME_WRITE_RUNTIME_HPP
