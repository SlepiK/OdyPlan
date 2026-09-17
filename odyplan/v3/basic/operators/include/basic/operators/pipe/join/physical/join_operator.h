#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_OPERATOR_HPP

#include <basic/expression/physical/expression.h>
#include <basic/expression/physical/expression_codegen.h>
#include <basic/operators/pipe/join/logical/details/join_config.h>
#include <basic/operators/pipe/join/physical/details/join_predmod.h>
#include <core/logical/port/port.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical {
    class JoinOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        JoinOperator(core::physical::port::Ports inputPorts,
                     core::physical::port::Port outputPort,
                     logical::details::JoinConfig config,
                     core::logical::port::Ports schemas,
                     std::unique_ptr<basic::expression::physical::IExpression> predicate);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId inputPort,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

        void declareState(core::physical::state::StateRegistry& stateRegistry,
                          llvm::Module& module,
                          llvm::LLVMContext& context) override;

        void consumePunctuation(core::physical::segment::SegmentContext &segmentCtx,
                                core::shared::identifiers::PortId inputPort,
                                const core::physical::data::PunctuationPackage &punctuation,
                                const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        logical::details::JoinConfig mConfig;
        core::logical::port::Port mLeftSchema;
        core::logical::port::Port mRightSchema;
        const std::unique_ptr<basic::expression::physical::IExpression> mPredicate;
        const std::optional<details::EquiKeyList> mEquiKeys;

        core::physical::state::StateHandle mState;
        core::physical::state::StateHandle mWatermarkState;

        llvm::Value* loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const;

        static llvm::GlobalVariable* buildTypeIds(llvm::Module& module,
                                                  llvm::LLVMContext& ctx,
                                                  const core::logical::port::Port& schema,
                                                  const std::string& name);

        static llvm::GlobalVariable* buildEquiFieldIndices(llvm::Module& module,
                                                    llvm::LLVMContext& ctx,
                                                    const std::vector<details::EquiKeyInfo>& keys,
                                                    bool useLeftIndex,
                                                    const std::string& name);

        static llvm::GlobalVariable* buildEquiTypeIds(llvm::Module& module, llvm::LLVMContext& ctx,
                                                    const std::vector<details::EquiKeyInfo>& keys,
                                                    const std::string& name);

        void evaluatePredicateAndMark(core::physical::segment::SegmentContext& segmentCtx,
                llvm::Value* snapshot, llvm::Value* state, bool onLeft, bool extract);

        void emitMatchesFromSnapshot(core::physical::segment::SegmentContext& segmentCtx,
                llvm::Value* snapshot, bool onLeft,
                const core::physical::segment::SegmentDataConsumers& consumers);

        static void codegenEvict(core::physical::segment::SegmentContext& segmentCtx,
                                 llvm::Value* state,
                                 llvm::Value* timestamp,
                                 bool callFromLeft,
                                 llvm::Value* isFinal);
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_OPERATOR_HPP