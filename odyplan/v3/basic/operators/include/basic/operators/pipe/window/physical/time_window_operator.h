#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP

#include <core/physical/operators/pipe/typed_pipe_operator.h>
#include <basic/window/time/time_window_config.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::physical {
    class TimeWindowOperator final : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        TimeWindowOperator(core::physical::port::Port inputPort,
                           core::physical::port::Port outputPort,
                           const basic::window::time::TimeWindowConfig &config,
                           core::logical::port::Port schema);

        void declareState(core::physical::state::StateRegistry &stateRegistry,
                          llvm::Module &module,
                          llvm::LLVMContext &context) override;

        void consume(core::physical::segment::SegmentContext &segmentCtx,
                     core::shared::identifiers::PortId inputPort,
                     const core::physical::data::DataPackage &tuple,
                     const core::physical::segment::SegmentDataConsumers &consumers) override;

        void consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId inputPort,
                                const core::physical::data::PunctuationPackage &punctuation,
                                const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        basic::window::time::TimeWindowConfig mConfig;
        core::logical::port::Port mDataSchema;
        core::shared::identifiers::OperatorId mLogicalOperatorId = 0;
        core::physical::state::StateHandle mState;
        llvm::Function *mDrainFn = nullptr;

        static llvm::GlobalVariable *buildTypeIds(llvm::Module &module, llvm::LLVMContext &ctx,
                                                  const core::logical::port::Port &schema, const std::string &name);

        llvm::Value *loadStatePtr(core::physical::segment::SegmentContext &segmentCtx) const;

        llvm::Function *getOrCreateDrainFn(core::physical::segment::SegmentContext &segmentCtx,
                                           const core::physical::segment::SegmentConsumerHandle &consumer);

        void drainWindows(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                          llvm::Value *threshold, llvm::Value *isFinal,
                          const core::physical::segment::SegmentConsumerHandle &consumer);

        void codegenPush(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state, llvm::Value *value,
                         std::size_t fieldIndex) const;

        static llvm::Value *codegenGet(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                       llvm::Value *rowIndex, std::size_t fieldIndex,
                                       core::shared::types::ValueTypeId typeId);

        void codegenEndRow(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                           const core::physical::data::DataPackage &tuple, llvm::Value *eventTs, llvm::Value *windowStart,
                           llvm::Value *calculatedWindowEnd) const;

        void codegenPreTumbling(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                             const core::physical::data::DataPackage &tuple, llvm::Value *eventTs) const;

        void codegenPreIter(core::physical::segment::SegmentContext &segmentCtx, llvm::Value *state,
                                  const core::physical::data::DataPackage &tuple, llvm::Value *eventTs,
                                  std::uint64_t calculatedWindowCandidatesPerTuple) const;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::window::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_WINDOW_PHYSICAL_TIME_WINDOW_OPERATOR_HPP
