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

extern "C" {
    inline bool rtIOSourceHasNext(const sierra::odyplan::v3::basic::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size()) {
            return false;
        }

        return gSources.at(index)->available();
    }

    inline const char* rtIOSourceGetField(const sierra::odyplan::v3::basic::io::IOSourceId index, const char* fieldName) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size() || fieldName == nullptr) {
            return nullptr;
        }

        const sierra::odyplan::v3::basic::io::IOData& current = gSources.at(index)->current();
        const auto iter = current.find(fieldName);
        return iter != current.end() ? iter->second.c_str() : nullptr;
    }

    inline void rtIOSourceReset(const sierra::odyplan::v3::basic::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index < gSources.size()) {
            (*gSources.at(index)).reset();
        }
    }

    inline bool rtParseBool(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseBool: invalid argument");
        }

        if (std::strcmp(str, "true") == 0 || std::strcmp(str, "1") == 0) {
            return true;
        }
        if (std::strcmp(str, "false") == 0 || std::strcmp(str, "0") == 0) {
            return false;
        }

        throw std::invalid_argument("rtParseBool: invalid argument");
    }

    inline std::int8_t rtParseInt8(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt8: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int8_t>::min() || value > std::numeric_limits<std::int8_t>::max()) {
            throw std::invalid_argument("rtParseInt8: invalid argument or out of range");
        }

        return static_cast<std::int8_t>(value);
    }

    inline std::int16_t rtParseInt16(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt16: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int16_t>::min() || value > std::numeric_limits<std::int16_t>::max()) {
            throw std::invalid_argument("rtParseInt16: invalid argument or out of range");
        }

        return static_cast<std::int16_t>(value);
    }

    inline std::int32_t rtParseInt32(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt32: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int32_t>::min() || value > std::numeric_limits<std::int32_t>::max()) {
            throw std::invalid_argument("rtParseInt32: invalid argument or out of range");
        }

        return static_cast<std::int32_t>(value);
    }

    inline std::int64_t rtParseInt64(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseInt64: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const long value = std::strtol(str, &end, 10);

        if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<std::int64_t>::min() || value > std::numeric_limits<std::int64_t>::max()) {
            throw std::invalid_argument("rtParseInt64: invalid argument or out of range");
        }

        return value;
    }

    inline std::float_t rtParseFloat(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseFloat: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const std::float_t value = std::strtof(str, &end);

        if (end == str || *end != '\0' || errno == ERANGE) {
            throw std::invalid_argument("rtParseFloat: invalid argument");
        }

        return value;
    }

    inline std::double_t rtParseDouble(const char* str) {
        if (str == nullptr) {
            throw std::invalid_argument("rtParseDouble: invalid argument");
        }

        char* end = nullptr;
        errno = 0;
        const std::double_t value = std::strtod(str, &end);

        if (end == str || *end != '\0' || errno == ERANGE) {
            throw std::invalid_argument("rtParseDouble: invalid argument");
        }

        return value;
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_RUNTIME_READ_RUNTIME_HPP