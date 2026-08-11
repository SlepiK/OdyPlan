#include <basic/operators/pipe/window/physical/count_window_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    CountWindowOperator::CountWindowOperator(core::physical::port::Port inputPort,
                                             core::physical::port::Port outputPort,
                                             const basic::window::count::CountWindowConfig& config,
                                             core::logical::port::Port schema)
        : WindowOperatorBase(std::move(inputPort), std::move(outputPort)),
          mConfig(config),
          mDataSchema(std::move(schema)) {}

    void CountWindowOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                           llvm::Module& module,
                                           llvm::LLVMContext& context) {
        auto* i64Ty = llvm::Type::getInt64Ty(context);
        auto* ptrTy = llvm::PointerType::getUnqual(context);

        auto* typesGlobal = buildTypeIds(
            module, context, mDataSchema, "count_window_types_" + std::to_string(getId()));

        mState = stateRegistry.createHandle("count_window_" + std::to_string(getId()),
                                            "rtCreateCountWindowBuffer",
                                            {llvm::ConstantInt::get(i64Ty, mConfig.size),
                                             llvm::ConstantInt::get(i64Ty, mConfig.slide),
                                             typesGlobal,
                                             llvm::ConstantInt::get(i64Ty, mDataSchema.size())},
                                            {i64Ty, i64Ty, ptrTy, i64Ty},
                                            "rtDestroyCountWindowBuffer");

        setWindowState(mState);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical