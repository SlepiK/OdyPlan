#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_RUNTIME_READ_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_RUNTIME_READ_RUNTIME_HPP

#include <cassert>
#include <memory>
#include <vector>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <core/runtime/task_runtime.h>
#include <basic/io/io_source.h>

inline std::vector<std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSource>> gSources;

inline sierra::odyplan::v3::basic::io::IOSourceId registerSource(std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSource> source) {
    gSources.push_back(std::move(source));
    return gSources.size() - 1;
}

namespace sierra::odyplan::v3::basic::operators::source::read::runtime {
    extern "C" {
        bool rtIOSourceHasNext(io::IOSourceId index);

        const char* rtIOSourceGetField(io::IOSourceId index, const char* fieldName);

        void rtIOSourceReset(io::IOSourceId index);

        bool rtParseBool(const char* str);

        std::int8_t rtParseInt8(const char* str);

        std::int16_t rtParseInt16(const char* str);

        std::int32_t rtParseInt32(const char* str);

        std::int64_t rtParseInt64(const char* str);

        std::float_t rtParseFloat(const char* str);

        std::double_t rtParseDouble(const char* str);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_RUNTIME_READ_RUNTIME_HPP