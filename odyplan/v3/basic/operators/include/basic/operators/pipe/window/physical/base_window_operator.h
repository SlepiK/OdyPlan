#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_BASE_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_BASE_WINDOW_OPERATOR_HPP

#include <llvm/IR/Module.h>

#include <core/logical/port/port.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class WindowOperatorBase : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        WindowOperatorBase(core::physical::port::Port inputPort,
                           core::physical::port::Port outputPort);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) final;

        void end(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId /*inputPort*/,
                 const core::physical::segment::SegmentDataConsumers& consumers) final;

    protected:
        void setWindowState(core::physical::state::StateHandle state);

        static llvm::GlobalVariable* buildTypeIds(llvm::Module& module,
                                                  llvm::LLVMContext& ctx,
                                                  const core::logical::port::Port& schema,
                                                  const std::string& name);

    private:
        core::physical::state::StateHandle mState;
        llvm::Function* mDrainFn = nullptr;

        llvm::Value* loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const;

        llvm::Function*
        getOrCreateDrainFn(const core::physical::segment::SegmentContext& segmentCtx,
                           const core::physical::segment::SegmentDataConsumer& consumer);

        void drainWindows(core::physical::segment::SegmentContext& segmentCtx,
                          llvm::Value* state,
                          const core::physical::segment::SegmentDataConsumer& consumer) const;

        static llvm::Value*
        codegenGetTimestamp(core::physical::segment::SegmentContext& segmentContext,
                            llvm::Value* state,
                            llvm::Value* windowIndex,
                            llvm::Value* rowIndex);

        static void codegenPush(core::physical::segment::SegmentContext& segmentCtx,
                                llvm::Value* state,
                                llvm::Value* value,
                                core::shared::types::ValueTypeId typeId,
                                std::size_t fieldIndex);

        static llvm::Value* codegenGet(core::physical::segment::SegmentContext& segmentCtx,
                                       llvm::Value* state,
                                       llvm::Value* windowIndex,
                                       llvm::Value* rowIndex,
                                       llvm::Value* dataIndex,
                                       core::shared::types::ValueTypeId typeId);
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_BASE_WINDOW_OPERATOR_HPP
