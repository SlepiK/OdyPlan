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

namespace sierra::odyplan::v3::basic::operators::source::read::runtime {
    namespace detail {
        template <typename T>
        T parseInt(const char* str, const char* func) {
            if (str == nullptr) {
                throw std::invalid_argument(std::string(func) +": invalid argument");
            }

            char* end = nullptr;
            errno = 0;
            const long value = std::strtoll(str, &end, 10);

            if (end == str || *end != '\0' || errno == ERANGE || value < std::numeric_limits<T>::min() || value > std::numeric_limits<T>::max()) {
                throw std::invalid_argument(std::string(func) +": invalid argument or out of range");
            }

            return static_cast<T>(value);
        }
    }

    extern "C" {
        void* rtIOSourceCreate(const void* typeRegistryRawPtr, std::uint8_t ioFormat, const char* path);

        void rtIOSourceDestroy(void* state);

        bool rtIOSourceHasNext(void* state);

        const char* rtIOSourceGetField(void* state, const char* fieldName);

        void rtIOSourceReset(void* state);

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