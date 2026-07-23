#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINTOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINTOPERATOR_H

#include <llvm/IR/Module.h>

#include "core/common/identifier/type.h"
#include "core/logical/operator/sink/sink_operator.h"
#include "core/physical/operator/sink/sink_operator.h"

namespace sierra::odyplan::v2::common::opr::sink {
    namespace logical {
        class PrintOperator : public core::logical::opr::TypedSinkOperator {
        public:
            using PortFieldIndexV = std::vector<std::size_t>;
            using PortFieldNameV = std::vector<std::string>;

            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::PRINT);

            explicit PrintOperator(PortFieldIndexV indices)
                : mPortFieldIndices(std::move(indices)) {
            }

            explicit PrintOperator(PortFieldNameV names)
                : mPortFieldNames(std::move(names)) {

            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] PortFieldIndexV getPortFieldIndices() const {
                return mPortFieldIndices;
            }

        protected:
            void validate() override {
                if (!mPortFieldNames.empty() && mPortFieldIndices.empty()) {
                    PortFieldIndexV indices;
                    indices.reserve(mPortFieldNames.size());

                    const auto& inputPort = getInputPorts().at(0);

                    for (const auto& name : mPortFieldNames) {
                        auto iter = std::ranges::find_if(inputPort,
                        [&](const core::logical::port::PortDescriptor& pd) {
                            return pd.fieldName == name && core::common::identifier::isTypePrintable(pd.typeId);
                        });

                        if (iter == inputPort.end()) {
                            throw std::invalid_argument("PrintOperator: name not found in inputPort");
                        }

                        indices.push_back(static_cast<std::size_t>(std::distance(inputPort.begin(), iter)));
                    }

                    mPortFieldIndices = std::move(indices);
                }
            }

        private:
            PortFieldIndexV mPortFieldIndices;
            PortFieldNameV mPortFieldNames;
        };
    }

    namespace physical {
        class PrintOperator : public core::physical::opr::TypedSinkOperator {
        public:
            using PortFieldIndexV = std::vector<std::size_t>;

            explicit PrintOperator(core::physical::port::Port inputPort, PortFieldIndexV indices)
                : TypedSinkOperator(std::move(inputPort)),
                    mPortFieldIndices(std::move(indices)) {
            }

            void consume(core::physical::segment::SegmentContext& segmentCtx, core::common::identifier::PortId inputPort, const core::physical::port::PortTuple &tuple, const core::physical::port::PortTupleConsumers& /*consumers*/) override {
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
                if (tuple.size() < mPortFieldIndices.size()) {
                    throw std::out_of_range("PrintOperator: given field indices dont match incoming tuple");
                }
                for (std::size_t i = 0; i < mPortFieldIndices.size(); ++i) {
                    std::size_t const fieldIndex = mPortFieldIndices.at(i);
                    codegenPrintField(segmentCtx.builder, module, getInputPorts().at(0).at(fieldIndex).typeId, tuple.at(fieldIndex));

                    if (i + 1 < mPortFieldIndices.size()) {
                        codegenPrint(segmentCtx.builder, module, "rtOdyPrintSeperator");
                    }
                }
                codegenPrint(segmentCtx.builder, module, "rtOdyPrintNewline");
            }

            void end(core::physical::segment::SegmentContext &segmentCtx, const core::common::identifier::PortId inputPort, const core::physical::port::PortTupleConsumers& consumers) override {
            }

        private:
            PortFieldIndexV mPortFieldIndices;

            static void codegenPrint(llvm::IRBuilder<> &builder, llvm::Module* module, const char* name) {
                auto* fnType = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(builder.getContext()),
                    {},
                    false
                    );
                builder.CreateCall(module->getOrInsertFunction(name, fnType), {});
            }

            static void codegenPrintField(llvm::IRBuilder<> &builder, llvm::Module* module, const core::common::identifier::TypeId typeId, llvm::Value* value) {
                llvm::LLVMContext& ctx = builder.getContext();
                switch (static_cast<core::common::identifier::Type>(typeId)) {
                    case core::common::identifier::Type::BOOL: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::identifier::Type::INT8: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::identifier::Type::INT16: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::identifier::Type::INT32: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::identifier::Type::INT64: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::identifier::Type::FLOAT: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintFloat", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::identifier::Type::DOUBLE: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintDouble", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::identifier::Type::STRING: {
                        auto* fnType = llvm::FunctionType::get(
                            llvm::Type::getVoidTy(ctx),
                            {llvm::PointerType::getUnqual(ctx)},
                            false
                            );
                        llvm::FunctionCallee const fn = module->getOrInsertFunction("rtOdyPrintString", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    default:
                        throw std::logic_error("PrintOperator: non-printable type reached");
                }
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINTOPERATOR_H