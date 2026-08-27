#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_COUNT_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_COUNT_WINDOW_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <basic/window/count/count_window_config.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class CountWindowOperator final : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        CountWindowOperator(core::physical::port::Port inputPort,
                            core::physical::port::Port outputPort,
                            const basic::window::count::CountWindowConfig& config,
                            core::logical::port::Port schema);

        void declareState(core::physical::state::StateRegistry& stateRegistry,
                          llvm::Module& module,
                          llvm::LLVMContext& context) override;

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId inputPort,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

        void end(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId inputPort,
                 const core::physical::segment::SegmentDataConsumers& consumers) override;

    private:
        basic::window::count::CountWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::physical::state::StateHandle mState;
        llvm::Function* mDrainFn = nullptr;

        static llvm::GlobalVariable* buildTypeIds(llvm::Module& module,
                                                  llvm::LLVMContext& ctx,
                                                  const core::logical::port::Port& schema,
                                                  const std::string& name);

        llvm::Value* loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const;

        llvm::Function*
        getOrCreateDrainFn(const core::physical::segment::SegmentContext& segmentCtx,
                           const core::physical::segment::SegmentConsumerHandle& consumer);

        void drainWindows(core::physical::segment::SegmentContext& segmentCtx,
                          llvm::Value* state,
                          const core::physical::segment::SegmentConsumerHandle& consumer) const;

        void codegenPush(core::physical::segment::SegmentContext& segmentCtx,
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

        static llvm::Value*
        codegenGetTimestamp(core::physical::segment::SegmentContext& segmentContext,
                            llvm::Value* state,
                            llvm::Value* windowIndex,
                            llvm::Value* rowIndex);
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_COUNT_WINDOW_OPERATOR_HPP
