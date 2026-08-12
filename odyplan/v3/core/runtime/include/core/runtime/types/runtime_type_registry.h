#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_TYPE_REGISTRY_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_TYPE_REGISTRY_HPP
#include <cstddef>
#include <stdexcept>
#include <unordered_map>

#include "core/shared/types/value_type_id.h"

namespace sierra::odyplan::v3::core::runtime::types {
    class RuntimeTypeRegistry {
    public:
        using HashFunctionPtr = std::size_t (*)(const std::byte*);
        using EqualFunctionPtr = bool (*)(const std::byte*, const std::byte*);
        using RetainFunctionPtr = void* (*)(const void*);
        using ReleaseFunctionPtr = void (*)(void*);

        struct Entry {
            std::size_t byteSize;
            bool isPointerType;
            HashFunctionPtr hashFunction;
            EqualFunctionPtr equalFunction;
            RetainFunctionPtr retainFunction;
            ReleaseFunctionPtr releaseFunction;
        };

        void registerType(
            const shared::types::ValueTypeId id,
            const std::size_t byteSize,
            const bool isPointerType,
            const HashFunctionPtr hashFunction,
            const EqualFunctionPtr equalFunction,
            const RetainFunctionPtr retainFunction = &defaultRetain,
            const ReleaseFunctionPtr releaseFunction = &defaultRelease
        ) {
            mStorage.emplace(id, Entry {
                .byteSize = byteSize,
                .isPointerType = isPointerType,
                .hashFunction = hashFunction,
                .equalFunction = equalFunction,
                .retainFunction = retainFunction,
                .releaseFunction = releaseFunction
            });
        }

        [[nodiscard]] std::size_t byteSize(const shared::types::ValueTypeId id) const noexcept(false) {
            return get(id).byteSize;
        }

        [[nodiscard]] bool isPointerType(const shared::types::ValueTypeId id) const noexcept(false) {
            return get(id).isPointerType;
        }

        [[nodiscard]] std::size_t hash(const shared::types::ValueTypeId id, const std::byte* ptr) const noexcept(false) {
            return get(id).hashFunction(ptr);
        }

        [[nodiscard]] bool equal(const shared::types::ValueTypeId id, const std::byte* ptrA, const std::byte* ptrB) const noexcept(false) {
            return get(id).equalFunction(ptrA, ptrB);
        }

        [[nodiscard]] void* retain(const shared::types::ValueTypeId id, const void* ptr) const noexcept(false) {
            return get(id).retainFunction(ptr);
        }

        void release(const shared::types::ValueTypeId id, void* ptr) const noexcept(false) {
            get(id).releaseFunction(ptr);
        }

        [[nodiscard]] HashFunctionPtr getHashFnPtr(shared::types::ValueTypeId id) const {
            return get(id).hashFunction;
        }

        [[nodiscard]] EqualFunctionPtr getEqualFnPtr(shared::types::ValueTypeId id) const {
            return get(id).equalFunction;
        }

        static RuntimeTypeRegistry withBuiltins();

    private:
        std::unordered_map<shared::types::ValueTypeId, Entry> mStorage;

        [[nodiscard]] const Entry& get(const shared::types::ValueTypeId id) const {
            const auto it = mStorage.find(id);
            if (it == mStorage.end()) {
                throw std::invalid_argument("RuntimeTypeRegistry: unknown typeid \"" +std::to_string(id) + "\"");
            }
            return it->second;
        }

        static void* defaultRetain(const void* ptr) noexcept {
            return const_cast<void*>(ptr);
        }

        static void defaultRelease(void*) noexcept {}
    };
}

#endif //SIERRA_ODYPLAN_SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_TYPE_REGISTRY_HPPV3_CORE_PHYSICAL_TYPES_RUNTIME_TYPE_REGISTRY_HPP