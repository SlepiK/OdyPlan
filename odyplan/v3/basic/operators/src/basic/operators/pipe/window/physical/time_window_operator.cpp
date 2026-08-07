#include <basic/operators/pipe/window/physical/time_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    TimeWindowOperator::TimeWindowOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort,
                            const basic::window::time::TimeWindowConfig& config, core::logical::port::Port dataSchema)
            : WindowOperatorBase(std::move(inputPort), std::move(outputPort)),
              mConfig(config), mDataSchema(std::move(dataSchema)) {
    }

    void TimeWindowOperator::declareState(core::physical::state::StateRegistry &stateRegistry, llvm::Module &module,
                      llvm::LLVMContext &context) {
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
}