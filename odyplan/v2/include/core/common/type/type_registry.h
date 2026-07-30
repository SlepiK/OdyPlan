#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H

#include <cmath>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <math.h>
#include <functional>
#include <string>
#include <utility>

#include "core/common/identifier/type.h"

namespace sierra::odyplan::v2::core::common::type {
    class TypeRegistry {
    public:
        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;
        using ConstantFactory = std::function<llvm::Value*(llvm::IRBuilder<>&, const std::string&)>;

        using RetainFn = std::function<void*(void* ori)>;
        using ReleaseFn = std::function<void(void* own)>;

        using HashFn = std::function<std::size_t(const std::byte*)>;
        using EqualFn = std::function<bool(const std::byte*, const std::byte*)>;

        static TypeRegistry withDefaults() {
            TypeRegistry registry;
            registry.registerDefaults();
            return registry;
        }

        void registerType(identifier::TypeId id, std::string name, const std::size_t byteSize, LLVMTypeFactory typeFactory, ConstantFactory constantFactory, HashFn hashFunction, EqualFn equalFunction) {
            mTypeStorage.emplace(id, TypeRegistryEntry {.name=std::move(name), .byteSize = byteSize, .isPointerType = false, .typeFactory=std::move(typeFactory), .constantFactory=std::move(constantFactory), .retain=defaultRetain(), .release=defaultRelease(), .hash=std::move(hashFunction), .equal=std::move(equalFunction)});
        }

        void registerPtrType(identifier::TypeId id, std::string name, const std::size_t byteSize, LLVMTypeFactory typeFactory, ConstantFactory constantFactory, HashFn hashFunction, EqualFn equalFunction, RetainFn retain = defaultRetain(), ReleaseFn release = defaultRelease()) {
            mTypeStorage.emplace(id, TypeRegistryEntry {.name=std::move(name), .byteSize = byteSize, .isPointerType = true, .typeFactory=std::move(typeFactory), .constantFactory=std::move(constantFactory), .retain=std::move(retain), .release=std::move(release), .hash=std::move(hashFunction), .equal=std::move(equalFunction)});
        }

        [[nodiscard]] llvm::Type* resolveType(const identifier::TypeId id, llvm::LLVMContext& ctx) const {
            return get(id).typeFactory(ctx);
        }

        [[nodiscard]] llvm::Value* compileConstant(const identifier::TypeId id, llvm::IRBuilder<>& builder, const std::string& value) const {
            return get(id).constantFactory(builder, value);
        }

        [[nodiscard]] std::size_t getByteSize(const identifier::TypeId id) const {
            return get(id).byteSize;
        }

        [[nodiscard]] bool isPointerType(const identifier::TypeId id) const {
            return get(id).isPointerType;
        }

        [[nodiscard]] void* retain(const identifier::TypeId id, void* ori) const {
            return get(id).retain(ori);
        }

        void release(const identifier::TypeId id, void* own) const {
            get(id).release(own);
        }

        [[nodiscard]] const HashFn& getHashFn(const identifier::TypeId id) const {
            return get(id).hash;
        }

        [[nodiscard]] const EqualFn& getEqualFn(const identifier::TypeId id) const {
            return get(id).equal;
        }

    private:
        struct TypeRegistryEntry {
            std::string name;
            std::size_t byteSize;
            bool isPointerType;
            LLVMTypeFactory typeFactory;
            ConstantFactory constantFactory;
            RetainFn retain;
            ReleaseFn release;
            HashFn hash;
            EqualFn equal;
        };

        std::unordered_map<identifier::TypeId, TypeRegistryEntry> mTypeStorage;

        static RetainFn defaultRetain() {
            return [](void* ptr) {
                return ptr;
            };
        }

        static ReleaseFn defaultRelease() {
            return [](void*) {};
        }

        [[nodiscard]] const TypeRegistryEntry& get(const identifier::TypeId id) const {
            const auto iter = mTypeStorage.find(id);
            if (iter == mTypeStorage.end()) {
                throw std::invalid_argument("TypeRegistry: unknown typeId " + std::to_string(id));
            }
            return iter->second;
        }

        void registerDefaults() {
            registerType(
                std::to_underlying(identifier::Type::BOOL), "bool",
                sizeof(bool),
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt1Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt1Ty(b.getContext()), std::stoi(v));
                },
                [](const std::byte* ptr) -> std::size_t {
                    bool value = false;
                    std::memcpy(&value, ptr, sizeof(bool));
                    return std::hash<bool>{}(value);
                },
                [](const std::byte* a, const std::byte* b) -> bool {
                    return std::memcmp(a,b,sizeof(bool))==0;
                }
                );
            registerType(
                std::to_underlying(identifier::Type::INT8), "int8",
                sizeof(std::int8_t),
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt8Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt8Ty(b.getContext()), std::stoi(v));
                },
                [](const std::byte* ptr) -> std::size_t {
                    std::int8_t value = 0;
                    std::memcpy(&value, ptr, sizeof(std::int8_t));
                    return std::hash<std::int8_t>{}(value);
                },
                [](const std::byte* a, const std::byte* b) -> bool {
                    return std::memcmp(a,b,sizeof(std::int8_t))==0;
                }
                );
            registerType(
                std::to_underlying(identifier::Type::INT16), "int16",
                sizeof(std::int16_t),
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt16Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt16Ty(b.getContext()), std::stoi(v));
                },
                [](const std::byte *ptr) -> std::size_t {
                    std::int16_t value = 0;
                    std::memcpy(&value, ptr, sizeof(std::int16_t));
                    return std::hash<std::int16_t>{}(value);
                },
                [](const std::byte *a, const std::byte *b) -> bool {
                    return std::memcmp(a, b, sizeof(std::int16_t)) == 0;
                }
                );
            registerType(
                std::to_underlying(identifier::Type::INT32), "int32",
                sizeof(std::int32_t),
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt32Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(b.getContext()), std::stoi(v));
                },
                [](const std::byte *ptr) -> std::size_t {
                    std::int32_t value = 0;
                    std::memcpy(&value, ptr, sizeof(std::int32_t));
                    return std::hash<std::int32_t>{}(value);
                },
                [](const std::byte *a, const std::byte *b) -> bool {
                    return std::memcmp(a, b, sizeof(std::int32_t)) == 0;
                }
                );
            registerType(
                std::to_underlying(identifier::Type::INT64), "int64",
                sizeof(std::int64_t),
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getInt64Ty(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt64Ty(b.getContext()), std::stoi(v));
                },
                [](const std::byte *ptr) -> std::size_t {
                    std::int64_t value = 0;
                    std::memcpy(&value, ptr, sizeof(std::int64_t));
                    return std::hash<std::int64_t>{}(value);
                },
                [](const std::byte *a, const std::byte *b) -> bool {
                    return std::memcmp(a, b, sizeof(std::int64_t)) == 0;
                }
                );
            registerType(
                std::to_underlying(identifier::Type::FLOAT), "float",
                sizeof(std::float_t),
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getFloatTy(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getFloatTy(b.getContext()), std::stoi(v));
                },
                [](const std::byte *ptr) -> std::size_t {
                    std::float_t value = 0.0F;
                    std::memcpy(&value, ptr, sizeof(std::float_t));
                    if (value == 0.0F) {
                        value = 0.0F;
                    }
                    return std::hash<std::float_t>{}(value);
                },
                [](const std::byte *a, const std::byte *b) -> bool {
                    std::float_t fA = 0.0F;
                    std::float_t fB = 0.0F;
                    std::memcpy(&fA, a, sizeof(std::float_t));
                    std::memcpy(&fB, b, sizeof(std::float_t));
                    return fA == fB;
                }
                );
            registerType(
                std::to_underlying(identifier::Type::DOUBLE), "double",
                sizeof(std::double_t),
                [](llvm::LLVMContext& ctx) {
                    return llvm::Type::getDoubleTy(ctx);
                },
                [](const llvm::IRBuilder<>&b, const std::string& v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getDoubleTy(b.getContext()), std::stoi(v));
                },
                [](const std::byte *ptr) -> std::size_t {
                    std::double_t value = 0.0;
                    std::memcpy(&value, ptr, sizeof(std::double_t));
                    if (value == 0.0) {
                        value = 0.0;
                    }
                    return std::hash<std::double_t>{}(value);
                },
                [](const std::byte *a, const std::byte *b) -> bool {
                    std::double_t dA = 0.0;
                    std::double_t dB = 0.0;
                    std::memcpy(&dA, a, sizeof(std::double_t));
                    std::memcpy(&dB, b, sizeof(std::double_t));
                    return dA == dB;
                }
            );
            registerPtrType(
                std::to_underlying(identifier::Type::STRING), "string",
                sizeof(const char*),
                [](llvm::LLVMContext& ctx) {
                    return llvm::PointerType::getUnqual(ctx);
                },
                [](llvm::IRBuilder<>&b, const std::string& v) {
                    return b.CreateGlobalString(v);
                },
                [](const std::byte* ptr) -> std::size_t {
                    const char* value = nullptr;
                    std::memcpy(&value, ptr, sizeof(value));
                    return value != nullptr ? std::hash<std::string_view>{}(value) : std::hash<std::string_view>{}("");
                },
                [](const std::byte* aPtr, const std::byte* bPtr) -> bool {
                    const char* a = nullptr;
                    const char* b = nullptr;
                    std::memcpy(&a, aPtr, sizeof(a));
                    std::memcpy(&b, bPtr, sizeof(b));
                    if (a == nullptr || b == nullptr) {
                        return a == b;
                    }
                    return std::strcmp(a, b) == 0;
                },
                [](const void* ori) -> void* {
                    const auto *source = static_cast<const char*>(ori);
                    auto* cpy = strdup(source);
                    if (!cpy) {
                        throw std::bad_alloc();
                    }
                    return cpy;
                },
                [](void* own) {
                    // NOLINTNEXTLINE
                    free(own);
                }
            );
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_CORE_COMMON_TYPE_TYPEREGISTRY_H