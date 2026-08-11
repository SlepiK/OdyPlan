#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_TYPE_REGISTRY_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_TYPE_REGISTRY_HPP
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <core/shared/types/value_type_id.h>
#include <cstdint>
#include <functional>

namespace sierra::odyplan::v3::core::physical::types {
    class ValueTypeRegistry {
    public:
        using ValueTypeId = shared::types::ValueTypeId;

        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;

        using ConstantFactory = std::function<llvm::Value*(llvm::IRBuilder<>&, const std::string&)>;

        using RetainFn = std::function<void*(void* ori)>;

        using ReleaseFn = std::function<void(void* own)>;

        using HashFn = std::function<std::size_t(const std::byte*)>;

        using EqualFn = std::function<bool(const std::byte*, const std::byte*)>;

        [[nodiscard]] static ValueTypeRegistry withBuiltins();

        void registerType(ValueTypeId valueTypeId,
                          std::string typeName,
                          std::size_t typeByteSize,
                          LLVMTypeFactory typeLLVMFactory,
                          ConstantFactory typeConstantFactory,
                          HashFn typeHashFunction,
                          EqualFn typeEqualFunction);

        void registerPtrType(ValueTypeId valueTypeId,
                             std::string typeName,
                             std::size_t typeByteSize,
                             LLVMTypeFactory typeLLVMFactory,
                             ConstantFactory typeConstantFactory,
                             HashFn typeHashFunction,
                             EqualFn typeEqualFunction,
                             RetainFn typeRetainFunction = defaultRetainFunction(),
                             ReleaseFn typeReleaseFunction = defaultReleaseFunction());

        [[nodiscard]] llvm::Type* resolveType(ValueTypeId valueTypeId,
                                              llvm::LLVMContext& llvmCtx) const;

        [[nodiscard]] llvm::Value* compileConstant(ValueTypeId valueTypeId,
                                                   llvm::IRBuilder<>& llvmBuilder,
                                                   const std::string& value) const;

        [[nodiscard]] std::size_t getByteSize(ValueTypeId valueTypeId) const;

        [[nodiscard]] bool isPointerType(ValueTypeId valueTypeId) const;

        [[nodiscard]] void* retain(ValueTypeId valueTypeId, void* original) const;

        void release(ValueTypeId valueTypeId, void* original) const;

        [[nodiscard]] HashFn getHashFunction(ValueTypeId valueTypeId) const;

        [[nodiscard]] EqualFn getEqualFunction(ValueTypeId valueTypeId) const;

    private:
        struct ValueTypeRegistryEntry {
            std::string typeName;
            std::size_t typeByteSize;
            bool typeIsPointerType;
            LLVMTypeFactory typeLLVMFactory;
            ConstantFactory typeConstantFactory;
            RetainFn typeRetainFunction;
            ReleaseFn typeReleaseFunction;
            HashFn typeHashFunction;
            EqualFn typeEqualFunction;
        };

        std::unordered_map<ValueTypeId, ValueTypeRegistryEntry> mStorage;

        [[nodiscard]] static RetainFn defaultRetainFunction();

        [[nodiscard]] static ReleaseFn defaultReleaseFunction();

        [[nodiscard]] const ValueTypeRegistryEntry& get(ValueTypeId valueTypeId) const;

        void registerBuiltin();
    };
}  // namespace sierra::odyplan::v3::core::physical::types

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_TYPE_REGISTRY_HPP