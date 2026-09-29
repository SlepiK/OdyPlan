#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_TYPE_REGISTRY_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_TYPE_REGISTRY_HPP
#include <core/shared/types/value_type_id.h>
#include <core/shared/types/global_types.h>

#include <cstddef>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <unordered_map>

namespace sierra::odyplan::v3::core::runtime::types {
    class RuntimeTypeRegistry {
    public:
        using HashFunctionPtr = std::size_t (*)(const std::byte*);
        using EqualFunctionPtr = bool (*)(const std::byte*, const std::byte*);
        using RetainFunctionPtr = void* (*)(void*);
        using ReleaseFunctionPtr = void (*)(void*);
        using DestroyFunctionPtr = void (*)(void*);

        struct Entry {
            std::size_t byteSize;
            bool isPointerType;
            HashFunctionPtr hashFunction;
            EqualFunctionPtr equalFunction;
            RetainFunctionPtr retainFunction;
            ReleaseFunctionPtr releaseFunction;
            DestroyFunctionPtr destroyFunction;
        };

        static RuntimeTypeRegistry& instance();

        RuntimeTypeRegistry(const RuntimeTypeRegistry&) = delete;
        RuntimeTypeRegistry& operator=(const RuntimeTypeRegistry&) = delete;
        RuntimeTypeRegistry(RuntimeTypeRegistry&&) = delete;
        RuntimeTypeRegistry& operator=(RuntimeTypeRegistry&&) = delete;

        void registerType(const shared::types::ValueTypeId id,
                          const std::size_t byteSize,
                          const bool isPointerType,
                          const HashFunctionPtr hashFunction,
                          const EqualFunctionPtr equalFunction,
                          const RetainFunctionPtr retainFunction = nullptr,
                          const ReleaseFunctionPtr releaseFunction = nullptr,
                          const DestroyFunctionPtr destroyFunction = nullptr) {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            mStorage.emplace(id,
                             Entry{
                                 .byteSize = byteSize,
                                 .isPointerType = isPointerType,
                                 .hashFunction = hashFunction,
                                 .equalFunction = equalFunction,
                                 .retainFunction = retainFunction,
                                 .releaseFunction = releaseFunction,
                                 .destroyFunction = destroyFunction
                             });
        }

        [[nodiscard]] std::size_t byteSize(const shared::types::ValueTypeId id) const
            noexcept(false) {
            return get(id).byteSize;
        }

        [[nodiscard]] bool isPointerType(const shared::types::ValueTypeId id) const
            noexcept(false) {
            return get(id).isPointerType;
        }

        [[nodiscard]] std::size_t hash(const shared::types::ValueTypeId id,
                                       const std::byte* ptr) const noexcept(false) {
            return get(id).hashFunction(ptr);
        }

        [[nodiscard]] bool equal(const shared::types::ValueTypeId id,
                                 const std::byte* ptrA,
                                 const std::byte* ptrB) const noexcept(false) {
            return get(id).equalFunction(ptrA, ptrB);
        }

        [[nodiscard]] void* retain(const shared::types::ValueTypeId id, void* ptr) const
            noexcept(false) {
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

        const Entry* getEntry(shared::types::ValueTypeId id) const;

    private:
        mutable std::shared_mutex m_mutex;
        std::unordered_map<shared::types::ValueTypeId, Entry> mStorage;

        RuntimeTypeRegistry() = default;
        ~RuntimeTypeRegistry() = default;

        [[nodiscard]] const Entry& get(const shared::types::ValueTypeId id) const {
            const auto it = mStorage.find(id);
            if (it == mStorage.end()) {
                throw std::invalid_argument("RuntimeTypeRegistry: unknown typeid \"" +
                                            std::to_string(id) + "\"");
            }
            return it->second;
        }

        void initializeBuiltins();
    };
}  // namespace sierra::odyplan::v3::core::runtime::types

#endif  // SIERRA_ODYPLAN_SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_TYPE_REGISTRY_HPPV3_CORE_PHYSICAL_TYPES_RUNTIME_TYPE_REGISTRY_HPP
