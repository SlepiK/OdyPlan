#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_MERGEOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_MERGEOPERATOR_H

#include <vector>

#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    namespace logical {
        class MergeOperator : public core::logical::opr::TypedPipeOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::MERGE);

            explicit MergeOperator() = default;

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

        protected:
            void validate() override {
                const auto& inputPortsRef = getInputPortsRef();
                const auto& outputPortRef = getOutputPortsRef().at(0);

                for (const auto& inputPortRef : inputPortsRef) {
                    if (inputPortRef != outputPortRef) {
                        throw std::logic_error("MergeOperator::validate: all InputPorts and the first OutputPort need to be the same");
                    }
                }
            }
        };
    }

    namespace physical {
        class MergeOperator : public core::physical::opr::TypedPipeOperator {
        public:
            MergeOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort)
                : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)}, core::physical::port::Ports{std::move(outputPort)}) {
            }

            void declareState(core::physical::state::StateRegistry& stateRegistry, llvm::Module& /*module*/, llvm::LLVMContext& /*context*/) override {
                mStateHandle = stateRegistry.createHandle(
                    "ody_merge_" + std::to_string(getId()),
                    "rtCreateMergeLock",
                    {},
                    {},
                    "rtDestroyMergeLock");
            }

            void consume(core::physical::segment::SegmentContext &segmentCtx, core::common::identifier::PortId /*inputPort*/,
                const core::physical::port::PortTuple& tuple, const core::physical::port::PortTupleConsumers& consumers) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                llvm::Value* state = loadStatePtr(segmentCtx);

                auto lockFn = module->getOrInsertFunction("rtMergeLock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
                segmentCtx.builder.CreateCall(lockFn, {state});

                consumers.at(0)(segmentCtx, tuple);

                auto unlockFn = module->getOrInsertFunction("rtMergeUnlock", llvm::FunctionType::get(voidTy, {ptrTy}, false));
                segmentCtx.builder.CreateCall(unlockFn, {state});
            }

        private:
            core::physical::state::StateHandle mStateHandle;

            llvm::Value* loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
                auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
                return segmentCtx.builder.CreateLoad(ptrTy, mStateHandle.var);
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_MERGEOPERATOR_H