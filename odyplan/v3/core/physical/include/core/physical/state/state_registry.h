#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STATE_STATE_REGISTRY_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STATE_STATE_REGISTRY_HPP

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <core/physical/state/state_handle.h>

namespace sierra::odyplan::v3::core::physical::state {
    class StateRegistry {
    public:
        StateRegistry(llvm::Module* module, llvm::LLVMContext* context);

        StateHandle createHandle(const std::string& uniqueName,
                                 const std::string& createFnName,
                                 const std::vector<llvm::Value*>& extraArgs,
                                 const std::vector<llvm::Type*>& extraArgTypes,
                                 const std::string& destroyFnName);

        void finish();

        [[nodiscard]] llvm::GlobalVariable* getRuntimeTypeRegistryGlobal() const;
    private:
        llvm::Module* mModule;
        llvm::LLVMContext* mContext;
        llvm::Function* mInitFn;
        llvm::Function* mShutdownFn;
        std::unique_ptr<llvm::IRBuilder<>> mInitBuilder;
        std::unique_ptr<llvm::IRBuilder<>> mShutdownBuilder;
        llvm::GlobalVariable* mRuntimeTypeRegistryGlobal = nullptr;
    };
}  // namespace sierra::odyplan::v3::core::physical::state

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_STATE_STATE_REGISTRY_HPP