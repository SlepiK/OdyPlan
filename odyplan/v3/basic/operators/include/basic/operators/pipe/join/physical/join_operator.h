#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_OPERATOR_HPP

#include <basic/expression/compare_expression.h>
#include <basic/expression/constant_expression.h>
#include <basic/expression/expression.h>
#include <basic/expression/field_reference_expression.h>
#include <basic/expression/logical_expression.h>
#include <basic/operators/pipe/join/logical/details/join_config.h>
#include <basic/operators/pipe/join/physical/details/join_predmod.h>
#include <core/logical/port/port.h>
#include <core/physical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical {
    class JoinOperator : public core::physical::operators::pipe::TypedPipeOperator {
    public:
        JoinOperator(core::physical::port::Port inputPort,
                     core::physical::port::Port outputPort,
                     logical::details::JoinConfig config,
                     core::logical::port::Ports schemas,
                     const basic::expression::IExpression* predicate);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId inputPort,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

        void declareState(core::physical::state::StateRegistry& stateRegistry,
                          llvm::Module& module,
                          llvm::LLVMContext& context) override;

    private:
        logical::details::JoinConfig mConfig;
        core::logical::port::Port mLeftSchema;
        core::logical::port::Port mRightSchema;
        const basic::expression::IExpression* mPredicate;

        core::physical::state::StateHandle mState;

        llvm::Value* loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const;

        static llvm::GlobalVariable* buildTypeIds(llvm::Module& module,
                                                  llvm::LLVMContext& ctx,
                                                  const core::logical::port::Port& schema,
                                                  const std::string& name);

        void codegenSide(core::physical::segment::SegmentContext& segmentCtx,
                                llvm::Value* state,
                                const core::physical::data::DataPackage& tuple,
                                const core::logical::port::Port& schema,
                                const char* beginFnName,
                                const char* pushFnName,
                                const char* endFnName);

        void emitMatchFromSnapshot(core::physical::segment::SegmentContext& segmentCtx,
                llvm::Value* snapshot, bool onLeft,
                const core::physical::segment::SegmentDataConsumers& consumers);

        llvm::Value* codegenExpression(core::physical::segment::SegmentContext& segmentCtx,
                                       const core::physical::data::DataPackage& tuple,
                                       const expression::IExpression& expression);

        static void codegenEvict(core::physical::segment::SegmentContext& segmentCtx,
                                 llvm::Value* state,
                                 llvm::Value* timestamp,
                                 bool callFromLeft);

        static llvm::Value* codegenCompare(llvm::IRBuilder<>& builder,
                                           llvm::Value* left,
                                           llvm::Value* right,
                                           basic::expression::CompareExpression::CompareOp opr);
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_JOIN_OPERATOR_HPP