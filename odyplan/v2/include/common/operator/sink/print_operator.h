#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINTOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINTOPERATOR_H

#include <llvm/IR/Module.h>

#include "core/common/ident.h"
#include "core/common/type/type.h"
#include "core/logical/operator/sink/sink_operator.h"
#include "core/physical/operator/sink/sink_operator.h"

namespace sierra::odyplan::v2::common::opr::sink {
    namespace logical {
        class PrintOperator : public core::logical::opr::TypedSinkOperator {
        public:
            using PortFieldIndexV = std::vector<std::size_t>;
            using PortFieldNameV = std::vector<std::string>;

            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 30;

            explicit PrintOperator(PortFieldIndexV indices)
                : mPortFieldIndices(std::move(indices)) {
            }

            explicit PrintOperator(PortFieldNameV names)
                : mPortFieldNames(std::move(names)) {

            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getTypeId() const override {
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
                            return pd.fieldName == name && core::common::type::isPrintable(pd.typeId);
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

            void consume(llvm::IRBuilder<> &builder, const core::physical::port::PortTuple &tuple, core::physical::port::PortTupleConsumer /*consumer*/) override {
                llvm::Module* module = builder.GetInsertBlock()->getModule();
                if (tuple.size() < mPortFieldIndices.size()) {
                    throw std::out_of_range("PrintOperator: given field indices dont match incoming tuple");
                }
                for (std::size_t i = 0; i < mPortFieldIndices.size(); ++i) {
                    std::size_t const fieldIndex = mPortFieldIndices[i];
                    codegenPrintField(builder, module, getInputPorts().at(0).at(fieldIndex).typeId, tuple.at(fieldIndex));

                    if (i + 1 < mPortFieldIndices.size()) {
                        codegenPrint(builder, module, "rtOdyPrintSeperator");
                    }
                }
                codegenPrint(builder, module, "rtOdyPrintNewline");
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

            static void codegenPrintField(llvm::IRBuilder<> &builder, llvm::Module* module, const core::common::identifier::PortTypeId typeId, llvm::Value* value) {
                llvm::LLVMContext& ctx = builder.getContext();
                switch (typeId) {
                    case core::common::type::TYPE::INT: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee fn = module->getOrInsertFunction("rtOdyPrintInt", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::type::TYPE::FLOAT: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee fn = module->getOrInsertFunction("rtOdyPrintFloat", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::type::TYPE::DOUBLE: {
                        auto* fnType = llvm::FunctionType::get(
                          llvm::Type::getVoidTy(ctx),
                          {value->getType()},
                          false
                          );
                        llvm::FunctionCallee fn = module->getOrInsertFunction("rtOdyPrintDouble", fnType);
                        builder.CreateCall(fn, {value});
                        break;
                    }
                    case core::common::type::TYPE::STRING: {
                        llvm::Value* stringPtr = builder.CreateExtractValue(value, {0});
                        llvm::Value* stringLen = builder.CreateExtractValue(value, {1});
                        auto* fnType = llvm::FunctionType::get(
                            llvm::Type::getVoidTy(ctx),
                            {stringPtr->getType(), stringLen->getType()},
                            false
                            );
                        llvm::FunctionCallee fn = module->getOrInsertFunction("rtOdyPrintString", fnType);
                        builder.CreateCall(fn, {stringPtr, stringLen});
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