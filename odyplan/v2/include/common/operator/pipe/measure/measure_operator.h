#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_MEASUREOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MEASURE_MEASUREOPERATOR_H
#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"
#include "spdlog/fmt/bundled/base.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    namespace logical {
        class MeasureOperator : public core::logical::opr::TypedPipeOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::MEASURE);

            explicit MeasureOperator(std::string label)
                : mLabel(std::move(label)) {
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const std::string& getLabel() const {
                return mLabel;
            }

        protected:
            void validate() override {
            }

        private:
            std::string mLabel;

        };
    }

    namespace physical {
        class MeasureOperator : public core::physical::opr::TypedPipeOperator {
        public:
            using Mapping = std::vector<std::size_t>;

            MeasureOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, std::string label)
                : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)}, core::physical::port::Ports{std::move(outputPort)}),
                    mLabel(std::move(label)) {
            }

            void consume(core::physical::segment::SegmentContext& segmentCtx, core::common::identifier::PortId  /*inputPort*/, const core::physical::port::PortTuple &tuple, const core::physical::port::PortTupleConsumers& consumers) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                llvm::Value* state = loadStatePtr(segmentCtx);
                auto tickFn = module->getOrInsertFunction("rtMeasureTick", llvm::FunctionType::get(voidTy, {ptrTy}, false));
                segmentCtx.builder.CreateCall(tickFn, {state});

                consumers.at(0)(segmentCtx, tuple);
            }

            void end(core::physical::segment::SegmentContext &segmentCtx, const core::common::identifier::PortId  /*inputPort*/, const core::physical::port::PortTupleConsumers&  /*consumers*/) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                llvm::Value* state = loadStatePtr(segmentCtx);
                const auto finalFn = module->getOrInsertFunction("rtMeasureFinalReport", llvm::FunctionType::get(voidTy, {ptrTy}, false));
                segmentCtx.builder.CreateCall(finalFn, {state});
            }

            void declareState(core::physical::state::StateRegistry& stateRegistry, llvm::Module& module, llvm::LLVMContext& context) override {
                auto* ptrTy = llvm::PointerType::getUnqual(context);
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
                auto* labelGlobalVariable = new llvm::GlobalVariable(
                        module,
                        llvm::ArrayType::get(llvm::Type::getInt8Ty(context), mLabel.size() + 1),
                        true,
                        llvm::GlobalValue::InternalLinkage,
                        llvm::ConstantDataArray::getString(context, mLabel, true)
                    );


                llvm::Value* labelGlobal = llvm::ConstantExpr::getInBoundsGetElementPtr(
                    llvm::ArrayType::get(llvm::Type::getInt8Ty(context), mLabel.size() + 1),
                    labelGlobalVariable,
                    std::vector<llvm::Value*>{
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0),
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0)
                });

                mStateHandle = stateRegistry.createHandle("measure_" + std::to_string(getId()),
                    "rtCreateMeasure",
                    {labelGlobal}, {ptrTy},
                    "rtDestroyMeasure");
            }

        private:
            std::string mLabel;
            core::physical::state::StateHandle mStateHandle;

            llvm::Value* loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
                auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
                return segmentCtx.builder.CreateLoad(ptrTy, mStateHandle.var);
            }
        };
    }
}

#endif //ODYPLAN_MEASURE_OPERATOR_H