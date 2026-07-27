#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_IO_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_IO_RUNTIME_H

#include <memory>
#include <vector>
#include "common/io/io_sink.h"

inline std::vector<std::unique_ptr<sierra::odyplan::v2::common::io::IIOSink>> gSinks;

inline sierra::odyplan::v2::common::io::IOSinkId registerSink(std::unique_ptr<sierra::odyplan::v2::common::io::IIOSink> sink) {
    gSinks.push_back(std::move(sink));
    return gSinks.size() - 1;
}

extern "C" {
    inline void rtIOSinkBeginData(const sierra::odyplan::v2::common::io::IOSinkId index) {
        gSinks.at(index)->begin();
    }

    inline void rtIOSinkWrite(const sierra::odyplan::v2::common::io::IOSinkId index, const sierra::odyplan::v2::core::common::identifier::TypeId typeId, const char* key, const void* value) {
        gSinks.at(index)->write(key, typeId, value);
    }

    inline void rtIOSinkEndData(const sierra::odyplan::v2::common::io::IOSinkId index) {
        gSinks.at(index)->end();
    }

    inline void rtIOSinkFlush(const sierra::odyplan::v2::common::io::IOSinkId index) {
        gSinks.at(index)->flush();
    }

    inline void rtIOSinkFlushAll() {
        for (const auto& sink : gSinks) {
            sink->flush();
        }
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_IO_RUNTIME_H