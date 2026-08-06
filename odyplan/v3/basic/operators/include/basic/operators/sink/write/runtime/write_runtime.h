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

extern "C" {
    inline void rtIOSinkBeginData(const sierra::odyplan::v3::basic::io::IOSinkId index) {
        gSinks.at(index)->begin();
    }

    inline void rtIOSinkWrite(const sierra::odyplan::v3::basic::io::IOSinkId index, const sierra::odyplan::v3::core::shared::types::ValueTypeId typeId, const char* key, const void* value) {
        gSinks.at(index)->write(key, typeId, value);
    }

    inline void rtIOSinkEndData(const sierra::odyplan::v3::basic::io::IOSinkId index) {
        gSinks.at(index)->end();
    }

    inline void rtIOSinkFlush(const sierra::odyplan::v3::basic::io::IOSinkId index) {
        gSinks.at(index)->flush();
    }

    inline void rtIOSinkFlushAll() {
        for (const auto& sink : gSinks) {
            sink->flush();
        }
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_RUNTIME_WRITE_RUNTIME_HPP