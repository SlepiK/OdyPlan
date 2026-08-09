#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_RUNTIME_WRITE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_RUNTIME_WRITE_RUNTIME_HPP

#include <memory>
#include <vector>
#include <basic/io/io_sink.h>

inline std::vector<std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSink>> gSinks;

inline sierra::odyplan::v3::basic::io::IOSinkId registerSink(std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSink> sink) {
    gSinks.push_back(std::move(sink));
    return gSinks.size() - 1;
}

namespace sierra::odyplan::v3::basic::operators::sink::write::runtime {
    extern "C" {
        void rtIOSinkBeginData(io::IOSinkId index);

        void rtIOSinkWrite(io::IOSinkId index, core::shared::types::ValueTypeId typeId, const char* key, const void* value);

        void rtIOSinkEndData(io::IOSinkId index);

        void rtIOSinkFlush(io::IOSinkId index);

        void rtIOSinkFlushAll();
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_RUNTIME_WRITE_RUNTIME_HPP