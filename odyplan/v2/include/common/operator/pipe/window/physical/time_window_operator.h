#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_PHYSICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_PHYSICAL_H

#include "window_operator_base.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical {
    class TimeWindowOperator final : public WindowOperatorBase {
    public:
        TimeWindowOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                            const window::time::TimeWindowConfig& config, core::logical::port::Port dataSchema)
            : WindowOperatorBase(std::move(inputPort), std::move(outputPort)),
              mConfig(config), mDataSchema(std::move(dataSchema)) {
        }

        void declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module,
                          llvm::LLVMContext &context) override {
            auto* i64Ty = llvm::Type::getInt64Ty(context);
            auto* ptrTy = llvm::PointerType::getUnqual(context);

            auto* typesGlobal = buildTypeIds(module, context, mDataSchema,
                "time_window_types_" + std::to_string(mLogicalOperatorId));

            const auto state = stateRegistry.createHandle(
                "time_window_" + std::to_string(mLogicalOperatorId),
                "rtCreateTimeWindowBuffer",
                {
                    llvm::ConstantInt::get(i64Ty, mConfig.size.count()),
                    llvm::ConstantInt::get(i64Ty, mConfig.slide.count()),
                    llvm::ConstantInt::get(i64Ty, mConfig.allowedLateness.count()),
                    typesGlobal, llvm::ConstantInt::get(i64Ty, mDataSchema.size())
                },
                {i64Ty, i64Ty, i64Ty, ptrTy, i64Ty},
                "rtDestroyTimeWindowBuffer");

            setWindowState(state);
        }

    private:
        window::time::TimeWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::common::identifier::OperatorId mLogicalOperatorId = 0;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_WINDOW_TIMEOPERATOR_PHYSICAL_H