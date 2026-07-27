#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITE_WRITEOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITE_WRITEOPERATOR_H

#include <memory>
#include <utility>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <nlohmann/json.hpp>

#include "common/io/io_format.h"
#include "common/io/io_sink.h"
#include "core/common/identifier/operator.h"
#include "core/common/identifier/type.h"
#include "core/logical/operator/sink/sink_operator.h"
#include "core/logical/port/port.h"
#include "core/physical/operator/sink/sink_operator.h"
#include "core/physical/port/port.h"

namespace sierra::odyplan::v2::common::opr::sink {
    namespace logical {
        class WriteOperator : public core::logical::opr::TypedSinkOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::WRITE);

            explicit WriteOperator(const io::IOFormat ioFormat, nlohmann::json ioConfig)
                : mIOFormat(ioFormat),
                    mIOConfig(std::move(ioConfig)) {
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] io::IOFormat getIOFormat() const {
                return mIOFormat;
            }

            [[nodiscard]] const nlohmann::json& getIOConfig() const {
                return mIOConfig;
            }

        protected:
            void validate() override {
            }

        private:
            io::IOFormat mIOFormat;
            nlohmann::json mIOConfig;
        };
    }

    namespace physical {
        class WriteOperator : public core::physical::opr::TypedSinkOperator {
        public:
            WriteOperator(const core::physical::port::Port &inputPort, const io::IOSinkId ioSinkId, core::logical::port::Port dataSchema)
                : TypedSinkOperator(inputPort),
                    mIOSinkId(ioSinkId),
                    mDataSchema(std::move(dataSchema)) {
                if (mDataSchema.empty()) {
                    throw std::invalid_argument("WriteOperator: dataSchema is empty");
                }
            }

            void consume(core::physical::segment::SegmentContext& segmentCtx, core::common::identifier::PortId  /*inputPort*/, const core::physical::port::PortTuple &tuple, const core::physical::port::PortTupleConsumers& /*consumers*/) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* idTy = llvm::Type::getInt64Ty(ctx);
                llvm::Value* index = llvm::ConstantInt::get(idTy, mIOSinkId);

                const auto beginFn = module->getOrInsertFunction("rtIOSinkBeginData", llvm::FunctionType::get(voidTy, { idTy }, false));
                segmentCtx.builder.CreateCall(beginFn, {index});

                for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
                    const auto& dataField = mDataSchema.at(i);
                    llvm::Value* fieldName = segmentCtx.builder.CreateGlobalString(dataField.fieldName, "field_" + dataField.fieldName, 0, module);
                    codegenField(segmentCtx, index, fieldName, tuple.at(i), dataField.typeId);
                }

                const auto endFn = module->getOrInsertFunction("rtIOSinkEndData", llvm::FunctionType::get(voidTy, { idTy }, false));
                segmentCtx.builder.CreateCall(endFn, {index});
            }

            void end(core::physical::segment::SegmentContext &segmentCtx, core::common::identifier::PortId inputPort, const core::physical::port::PortTupleConsumers& consumers) override {
            }

        private:
            io::IOSinkId mIOSinkId;
            core::logical::port::Port mDataSchema;

            static void codegenField(core::physical::segment::SegmentContext& segmentCtx, llvm::Value* index, llvm::Value* fieldName, llvm::Value* fieldValue,
                const core::common::identifier::TypeId typeId) {

                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);
                auto* idTy = llvm::Type::getInt64Ty(ctx);
                auto* typeIdTy = llvm::Type::getInt64Ty(ctx);

                llvm::Value* ptrFieldValue = nullptr;

                if (fieldValue->getType()->isPointerTy()) {
                    ptrFieldValue = fieldValue;
                } else {
                    auto* slot = segmentCtx.storage.get(typeId);
                    segmentCtx.builder.CreateStore(fieldValue, slot);
                    ptrFieldValue = slot;
                }

                const auto fn = module->getOrInsertFunction(
                    "rtIOSinkWrite",
                    llvm::FunctionType::get(voidTy, { idTy, typeIdTy, ptrTy, ptrTy }, false)
                );

                segmentCtx.builder.CreateCall(fn, { index, llvm::ConstantInt::get(idTy, typeId), fieldName, ptrFieldValue });
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITE_WRITEOPERATOR_H