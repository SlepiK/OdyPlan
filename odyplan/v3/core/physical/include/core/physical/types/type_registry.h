#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_TYPE_REGISTRY_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_TYPE_REGISTRY_HPP
#include <cstdint>
#include <functional>

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

#include <core/shared/type/type_id.h>

namespace sierra::odyplan::v3::core::physical::types {
    class TypeRegistry {
    public:
    	using TypeId = shared::types::TypeId;

        using LLVMTypeFactory = std::function<llvm::Type*(llvm::LLVMContext&)>;

        using ConstantFactory = std::function<llvm::Value*(llvm::IRBuilder<>&, const std::string&)>;

        using RetainFn = std::function<void*(void* ori)>;

        using ReleaseFn = std::function<void(void* own)>;

        using HashFn = std::function<std::size_t(const std::byte*)>;

        using EqualFn = std::function<bool(const std::byte*, const std::byte*)>;

        [[nodiscard]] static TypeRegistry withBuiltins();

        void registerType(
            TypeId typeId,
            std::string typeName,
            std::size_t typeByteSize,
            LLVMTypeFactory typeLLVMFactory,
            ConstantFactory typeConstantFactory,
            HashFn typeHashFunction,
            EqualFn typeEqualFunction
		);

        void registerPtrType(
            TypeId typeId,
            std::string typeName,
            std::size_t typeByteSize,
            LLVMTypeFactory typeLLVMFactory,
            ConstantFactory typeConstantFactory,
            HashFn typeHashFunction,
            EqualFn typeEqualFunction,
            RetainFn typeRetainFunction = defaultRetainFunction(),
            ReleaseFn typeReleaseFunction = defaultReleaseFunction()
		);

        [[nodiscard]] llvm::Type* resolveType(
			TypeId typeId,
			llvm::LLVMContext& llvmCtx
		) const;

		[[nodiscard]] llvm::Value* compileConstant(
			TypeId typeId,
			llvm::IRBuilder<>& llvmBuilder,
			const std::string& value
		) const;

		[[nodiscard]] std::size_t getByteSize(TypeId typeId) const;

		[[nodiscard]] bool isPointerType(TypeId typeId) const;

		[[nodiscard]] void* retain(TypeId typeId, void* original) const;

		void release(TypeId typeId, void* original) const;

		[[nodiscard]] HashFn getHashFunction(TypeId typeId) const;

		[[nodiscard]] EqualFn getEqualFunction(TypeId typeId) const;

    private:
        struct TypeRegistryEntry {
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

        std::unordered_map<TypeId, TypeRegistryEntry> mStorage;

        [[nodiscard]] static RetainFn defaultRetainFunction();

        [[nodiscard]] static ReleaseFn defaultReleaseFunction();

        [[nodiscard]] const TypeRegistryEntry& get(TypeId typeId) const;

    	void registerBuiltin();
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TYPES_TYPE_REGISTRY_HPP