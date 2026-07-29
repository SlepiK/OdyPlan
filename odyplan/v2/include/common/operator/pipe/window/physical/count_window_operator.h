#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_PHYSICAL_H

#include "window_operator_base.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class CountWindowOperator final : public WindowOperatorBase {
    public:
        CountWindowOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                            const window::count::CountWindowConfig& config, core::logical::port::Port schema)
            : WindowOperatorBase(std::move(inputPort), std::move(outputPort)),
              mConfig(config), mDataSchema(std::move(schema)) {
        }

        void declareState(core::physical::state::StateRegistry& stateRegistry, llvm::Module& module, llvm::LLVMContext& context) override {
            auto* i64Ty = llvm::Type::getInt64Ty(context);
            auto* ptrTy = llvm::PointerType::getUnqual(context);

            auto* typesGlobal = buildTypeIds(module, context, mDataSchema, "count_window_types_" + std::to_string(getId()));

            mState = stateRegistry.createHandle(
                "count_window_" + std::to_string(getId()),
                "rtCreateCountWindowBuffer",
                {
                    llvm::ConstantInt::get(i64Ty, mConfig.size),
                    llvm::ConstantInt::get(i64Ty, mConfig.slide),
                    typesGlobal, llvm::ConstantInt::get(i64Ty, mDataSchema.size())
                },
                {i64Ty, i64Ty, ptrTy, i64Ty},
                "rtDestroyCountWindowBuffer");

            setWindowState(mState);
        }

    private:
        window::count::CountWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::physical::state::StateHandle mState;

    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_COUNTOPERATOR_PHYSICAL_H