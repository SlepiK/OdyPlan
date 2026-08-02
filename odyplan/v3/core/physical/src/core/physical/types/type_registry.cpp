#include <cmath>
#include <core/physical/types/type_registry.h>

namespace sierra::odyplan::v3::core::physical::types {
    TypeRegistry TypeRegistry::withBuiltins() {
        TypeRegistry registry;

        registry.registerBuiltin();

        return registry;
    }

    // NOLINTNEXTLINE(readability-function-size)
    void TypeRegistry::registerType(
        TypeId typeId,
        std::string typeName,
        const std::size_t typeByteSize,
        LLVMTypeFactory typeLLVMFactory,
        ConstantFactory typeConstantFactory,
        HashFn typeHashFunction,
        EqualFn typeEqualFunction) {
        mStorage.emplace(
            typeId,
            TypeRegistryEntry
            {
                .typeName = std::move(typeName),
                .typeByteSize = typeByteSize,
                .typeIsPointerType = false,
                .typeLLVMFactory = std::move(typeLLVMFactory),
                .typeConstantFactory = std::move(typeConstantFactory),
                .typeRetainFunction = defaultRetainFunction(),
                .typeReleaseFunction = defaultReleaseFunction(),
                .typeHashFunction = std::move(typeHashFunction),
                .typeEqualFunction = std::move(typeEqualFunction)
            });
    }

    // NOLINTNEXTLINE(readability-function-size)
    void TypeRegistry::registerPtrType(
        TypeId typeId,
        std::string typeName,
        std::size_t typeByteSize,
        LLVMTypeFactory typeLLVMFactory,
        ConstantFactory typeConstantFactory,
        HashFn typeHashFunction,
        EqualFn typeEqualFunction,
        RetainFn typeRetainFunction,
        ReleaseFn typeReleaseFunction) {
        mStorage.emplace(
            typeId,
            TypeRegistryEntry
            {
                .typeName = std::move(typeName),
                .typeByteSize = typeByteSize,
                .typeIsPointerType = true,
                .typeLLVMFactory = std::move(typeLLVMFactory),
                .typeConstantFactory = std::move(typeConstantFactory),
                .typeRetainFunction = std::move(typeRetainFunction),
                .typeReleaseFunction = std::move(typeReleaseFunction),
                .typeHashFunction = std::move(typeHashFunction),
                .typeEqualFunction = std::move(typeEqualFunction)
            });
    }


    llvm::Type *TypeRegistry::resolveType(
        const TypeId typeId,
        llvm::LLVMContext &llvmCtx) const {
        return get(typeId).typeLLVMFactory(llvmCtx);
    }


    llvm::Value *TypeRegistry::compileConstant(
        const TypeId typeId,
        llvm::IRBuilder<> &llvmBuilder,
        const std::string &value) const {
        return get(typeId).typeConstantFactory(llvmBuilder, value);
    }


    std::size_t TypeRegistry::getByteSize(const TypeId typeId) const {
        return get(typeId).typeByteSize;
    }

    bool TypeRegistry::isPointerType(const TypeId typeId) const {
        return get(typeId).typeIsPointerType;
    }


    void *TypeRegistry::retain(const TypeId typeId, void *original) const {
        return get(typeId).typeRetainFunction(original);
    }


    void TypeRegistry::release(const TypeId typeId, void *original) const {
        get(typeId).typeReleaseFunction(original);
    }


    TypeRegistry::HashFn TypeRegistry::getHashFunction(const TypeId typeId) const {
        return get(typeId).typeHashFunction;
    }


    TypeRegistry::EqualFn TypeRegistry::getEqualFunction(const TypeId typeId) const {
        return get(typeId).typeEqualFunction;
    }


    TypeRegistry::RetainFn TypeRegistry::defaultRetainFunction() {
        return [](void *ptr) {
            return ptr;
        };
    }

    TypeRegistry::ReleaseFn TypeRegistry::defaultReleaseFunction() {
        return [](void *) {
        };
    }

    const TypeRegistry::TypeRegistryEntry &TypeRegistry::get(const TypeId typeId) const {
        const auto iter = mStorage.find(typeId);
        if (iter == mStorage.end()) {
            throw std::invalid_argument("TypeRegistry: unknown typeId " + std::to_string(typeId));
        }
        return iter->second;
    }

    // NOLINTNEXTLINE(readability-function-size)
    void TypeRegistry::registerBuiltin() {
        registerType(
            std::to_underlying(shared::types::BuiltinType::BOOL), "bool",
            sizeof(bool),
            [](llvm::LLVMContext &ctx) {
                return llvm::Type::getInt1Ty(ctx);
            },
            [](const llvm::IRBuilder<> &b, const std::string &v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt1Ty(b.getContext()), std::stoi(v));
            },
            [](const std::byte *ptr) -> std::size_t {
                bool value = false;
                std::memcpy(&value, ptr, sizeof(bool));
                return std::hash<bool>{}(value);
            },
            [](const std::byte *a, const std::byte *b) -> bool {
                return std::memcmp(a, b, sizeof(bool)) == 0;
            }
        );

        registerType(
            std::to_underlying(shared::types::BuiltinType::INT8), "int8",
            sizeof(std::int8_t),
            [](llvm::LLVMContext &ctx) {
                return llvm::Type::getInt8Ty(ctx);
            },
            [](const llvm::IRBuilder<> &b, const std::string &v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt8Ty(b.getContext()), std::stoi(v));
            },
            [](const std::byte *ptr) -> std::size_t {
                std::int8_t value = 0;
                std::memcpy(&value, ptr, sizeof(std::int8_t));
                return std::hash<std::int8_t>{}(value);
            },
            [](const std::byte *a, const std::byte *b) -> bool {
                return std::memcmp(a, b, sizeof(std::int8_t)) == 0;
            }
        );

        registerType(
            std::to_underlying(shared::types::BuiltinType::INT16), "int16",
            sizeof(std::int16_t),
            [](llvm::LLVMContext &ctx) {
                return llvm::Type::getInt16Ty(ctx);
            },
            [](const llvm::IRBuilder<> &b, const std::string &v) {
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
            std::to_underlying(shared::types::BuiltinType::INT32), "int32",
            sizeof(std::int32_t),
            [](llvm::LLVMContext &ctx) {
                return llvm::Type::getInt32Ty(ctx);
            },
            [](const llvm::IRBuilder<> &b, const std::string &v) {
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
            std::to_underlying(shared::types::BuiltinType::INT64), "int64",
            sizeof(std::int64_t),
            [](llvm::LLVMContext &ctx) {
                return llvm::Type::getInt64Ty(ctx);
            },
            [](const llvm::IRBuilder<> &b, const std::string &v) {
                return llvm::ConstantInt::get(
                    llvm::Type::getInt64Ty(b.getContext()), std::stoll(v));
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
            std::to_underlying(shared::types::BuiltinType::FLOAT), "float",
            sizeof(std::float_t),
            [](llvm::LLVMContext &ctx) {
                return llvm::Type::getFloatTy(ctx);
            },
            [](const llvm::IRBuilder<> &b, const std::string &v) {
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
            std::to_underlying(shared::types::BuiltinType::DOUBLE), "double",
            sizeof(std::double_t),
            [](llvm::LLVMContext &ctx) {
                return llvm::Type::getDoubleTy(ctx);
            },
            [](const llvm::IRBuilder<> &b, const std::string &v) {
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
            std::to_underlying(shared::types::BuiltinType::STRING), "string",
            sizeof(const char *),
            [](llvm::LLVMContext &ctx) {
                return llvm::PointerType::getUnqual(ctx);
            },
            [](llvm::IRBuilder<> &b, const std::string &v) {
                return b.CreateGlobalString(v);
            },
            [](const std::byte *ptr) -> std::size_t {
                const char *value = nullptr;
                std::memcpy(static_cast<void *>(&value), ptr, sizeof(value));
                return value != nullptr ? std::hash<std::string_view>{}(value) : std::hash<std::string_view>{}("");
            },
            [](const std::byte *aPtr, const std::byte *bPtr) -> bool {
                const char *a = nullptr;
                const char *b = nullptr;
                std::memcpy(static_cast<void *>(&a), aPtr, sizeof(a));
                std::memcpy(static_cast<void *>(&b), bPtr, sizeof(b));
                if (a == nullptr || b == nullptr) {
                    return a == b;
                }
                return std::strcmp(a, b) == 0;
            },
            [](const void *ori) -> void * {
                const auto *source = static_cast<const char *>(ori);
                auto *cpy = strdup(source);
                if (!cpy) {
                    throw std::bad_alloc();
                }
                return cpy;
            },
            [](void *own) {
                // NOLINTNEXTLINE
                free(own);
            }
        );
    }
}
