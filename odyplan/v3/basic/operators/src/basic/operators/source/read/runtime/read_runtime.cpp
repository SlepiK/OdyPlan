#include "basic/io/io_registry.h"

#include <basic/operators/source/read/runtime/read_runtime.h>

namespace sierra::odyplan::v3::basic::operators::source::read::runtime {

    extern "C" {
    void* rtIOSourceCreate(const void* /*typeRegistryRawPtr*/,
                           const std::uint8_t ioFormat,
                           const char* path) {
        static const io::IORegistry ioRegistry = io::IORegistry::withBuiltins();
        const nlohmann::json config = {{"path", path}};
        auto source = ioRegistry.createSourceIO(
            static_cast<sierra::odyplan::v3::basic::io::IOFormatType>(ioFormat), config);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return source.release();
    }

    void rtIOSourceDestroy(void* state) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<io::IIOSource*>(state);
    }

    bool rtIOSourceHasNext(void* state) {
        return static_cast<io::IIOSource*>(state)->available();
    }

    const char* rtIOSourceGetField(void* state, const char* fieldName) {
        const auto& current = static_cast<io::IIOSource*>(state)->current();
        const auto it = current.find(fieldName);
        return it != current.end() ? it->second.c_str() : nullptr;
    }

    void rtIOSourceReset(void* state) {
        static_cast<io::IIOSource*>(state)->reset();
    }

    bool rtParseBool(const char* str) {
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

    std::int8_t rtParseInt8(const char* str) {
        return detail::parseInt<std::int8_t>(str, "rtParseInt8");
    }

    std::int16_t rtParseInt16(const char* str) {
        return detail::parseInt<std::int16_t>(str, "rtParseInt16");
    }

    std::int32_t rtParseInt32(const char* str) {
        return detail::parseInt<std::int32_t>(str, "rtParseInt32");
    }

    std::int64_t rtParseInt64(const char* str) {
        return detail::parseInt<std::int64_t>(str, "rtParseInt64");
    }

    std::float_t rtParseFloat(const char* str) {
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

    std::double_t rtParseDouble(const char* str) {
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
}  // namespace sierra::odyplan::v3::basic::operators::source::read::runtime
