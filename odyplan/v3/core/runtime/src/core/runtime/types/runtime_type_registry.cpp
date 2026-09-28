#include <cmath>
#include <core/runtime/types/runtime_type_registry.h>
#include <cstring>
#include <mutex>
#include <utility>

namespace sierra::odyplan::v3::core::runtime::types {
    RuntimeTypeRegistry &RuntimeTypeRegistry::instance() {
        static RuntimeTypeRegistry instance;
        static std::once_flag initFl;
        std::call_once(initFl, []() {
            instance.initializeBuiltins();
        });
        return instance;
    }

    // NOLINTNEXTLINE(readability-function-size)
    void RuntimeTypeRegistry::initializeBuiltins() {
        using T = shared::types::BuiltinValueType;

        registerType(
            std::to_underlying(T::BOOL),
            sizeof(bool),
            false,
            [](const std::byte* ptr) -> std::size_t {
                bool v{};
                std::memcpy(&v, ptr, sizeof(v));
                return std::hash<bool>{}(v);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                return std::memcmp(a, b, sizeof(bool)) == 0;
            });

        registerType(
            std::to_underlying(T::INT8),
            sizeof(std::int8_t),
            false,
            [](const std::byte* ptr) -> std::size_t {
                std::int8_t v{};
                std::memcpy(&v, ptr, sizeof(v));
                return std::hash<std::int8_t>{}(v);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                return std::memcmp(a, b, sizeof(std::int8_t)) == 0;
            });

        registerType(
            std::to_underlying(T::INT16),
            sizeof(std::int16_t),
            false,
            [](const std::byte* ptr) -> std::size_t {
                std::int16_t v{};
                std::memcpy(&v, ptr, sizeof(v));
                return std::hash<std::int16_t>{}(v);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                return std::memcmp(a, b, sizeof(std::int16_t)) == 0;
            });

        registerType(
            std::to_underlying(T::INT32),
            sizeof(std::int32_t),
            false,
            [](const std::byte* ptr) -> std::size_t {
                std::int32_t v{};
                std::memcpy(&v, ptr, sizeof(v));
                return std::hash<std::int32_t>{}(v);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                return std::memcmp(a, b, sizeof(std::int32_t)) == 0;
            });

        registerType(
            std::to_underlying(T::INT64),
            sizeof(std::int64_t),
            false,
            [](const std::byte* ptr) -> std::size_t {
                std::int64_t v{};
                std::memcpy(&v, ptr, sizeof(v));
                return std::hash<std::int64_t>{}(v);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                return std::memcmp(a, b, sizeof(std::int64_t)) == 0;
            });

        registerType(
            std::to_underlying(T::FLOAT),
            sizeof(std::float_t),
            false,
            [](const std::byte* ptr) -> std::size_t {
                float f{};
                std::memcpy(&f, ptr, sizeof(f));
                if (std::isnan(f)) {
                    return std::hash<std::float_t>{}(
                        std::numeric_limits<std::float_t>::quiet_NaN());
                }
                return std::hash<std::float_t>{}(f + 0.0F);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                std::float_t fA{};
                std::float_t fB{};
                std::memcpy(&fA, a, sizeof(std::float_t));
                std::memcpy(&fB, b, sizeof(std::float_t));
                return fA == fB;
            });

        registerType(
            std::to_underlying(T::DOUBLE),
            sizeof(std::double_t),
            false,
            [](const std::byte* ptr) -> std::size_t {
                std::double_t v{};
                std::memcpy(&v, ptr, sizeof(v));
                if (std::isnan(v)) {
                    return std::hash<std::double_t>{}(
                        std::numeric_limits<std::double_t>::quiet_NaN());
                }
                return std::hash<std::double_t>{}(v + 0.0F);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                std::double_t dA{};
                std::double_t dB{};
                std::memcpy(&dA, a, sizeof(std::double_t));
                std::memcpy(&dB, b, sizeof(std::double_t));
                return dA == dB;
            });

        registerType(
            std::to_underlying(T::STRING),
            sizeof(const char*),
            true,
            [](const std::byte* ptr) -> std::size_t {
                const auto* v = reinterpret_cast<const char*>(ptr);
                return v != nullptr ? std::hash<std::string_view>{}(v)
                                    : std::hash<std::string_view>{}("");
            },
            [](const std::byte* aPtr, const std::byte* bPtr) -> bool {
                const auto *a = reinterpret_cast<const char*>(aPtr);
                const auto *b = reinterpret_cast<const char*>(bPtr);
                if (a == nullptr || b == nullptr) {
                    return a == b;
                }
                return std::strcmp(a, b) == 0;
            },
            [](void* ori) -> void* {
                auto* cpy = strdup(static_cast<const char*>(ori));
                if (!cpy) {
                    throw std::bad_alloc();
                }
                return cpy;
            },
            [](void* own) {
                // NOLINTNEXTLINE
                free(own);
            });


        registerType(
            std::to_underlying(T::TIMESTAMP),
            sizeof(shared::types::Timestamp),
            false,
            [](const std::byte* ptr) -> std::size_t {
                shared::types::Timestamp v{};
                std::memcpy(&v, ptr, sizeof(v));
                return std::hash<shared::types::Timestamp>{}(v);
            },
            [](const std::byte* a, const std::byte* b) -> bool {
                return std::memcmp(a, b, sizeof(shared::types::Timestamp)) == 0;
            });
    }
}  // namespace sierra::odyplan::v3::core::runtime::types
