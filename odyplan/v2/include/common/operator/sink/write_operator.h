#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H

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
#include "core/common/type/type_registry.h"
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

        private:
            io::IOFormat mIOFormat;
            nlohmann::json mIOConfig;
        };
    }

    namespace physical {
        class WriteOperator : public core::physical::opr::TypedSinkOperator {
        public:
            WriteOperator(const core::physical::port::Port &inputPort, const io::IOSinkId ioSinkId ,const core::common::type::TypeRegistry& typeRegistry, core::logical::port::Port dataSchema)
                : TypedSinkOperator(inputPort),
                    mIOSinkId(ioSinkId),
                    mTypeRegistry(typeRegistry),
                    mDataSchema(std::move(dataSchema)) {
                if (mDataSchema.empty()) {
                    throw std::invalid_argument("WriteOperator: dataSchema is empty");
                }
            }

            void consume(llvm::IRBuilder<> &builder, const core::physical::port::PortTuple &tuple, core::physical::port::PortTupleConsumer /*consumer*/) override {
                llvm::LLVMContext& ctx = builder.getContext();
                llvm::Module* module = builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);

                auto beginFn = module->getOrInsertFunction("rtIOSinkBeginData", llvm::FunctionType::get(voidTy, {}, false));
                builder.CreateCall(beginFn, {});

                for (std::size_t i = 0; i < mDataSchema.size(); ++i) {
                    const auto& dataField = mDataSchema.at(i);
                    llvm::Value* fieldName = builder.CreateGlobalString(dataField.fieldName, "field_" + dataField.fieldName, 0, module);
                    codegenField(builder, fieldName, tuple.at(i), dataField.typeId);
                }

                auto writeFieldFn = module->getOrInsertFunction("rtIOSinkWriteData", llvm::FunctionType::get(voidTy, {i8Ptr, i8Ptr, i8Ptr}, false));

                auto endFn = module->getOrInsertFunction("rtIOSinkEndData", llvm::FunctionType::get(voidTy, {}, false));
                builder.CreateCall(endFn, {});
            }
        protected:
        private:
            io::IOSinkId mIOSinkId;
            const core::common::type::TypeRegistry& mTypeRegistry;
            core::logical::port::Port mDataSchema;

            void codegenField(llvm::IRBuilder<>& builder, llvm::Value* fieldName, llvm::Value* fieldValue,
                core::common::identifier::TypeId typeId) {

                llvm::LLVMContext& ctx = builder.getContext();
                llvm::Module* module = builder.GetInsertBlock()->getModule();

                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                if (typeId == std::to_underlying(core::common::identifier::Type::STRING)) {
                    auto fn = module->getOrInsertFunction("rtIOSinkWriteString", llvm::FunctionType::get(voidTy, {ptrTy, ptrTy}, false));
                    builder.CreateCall(fn, {fieldName, fieldValue});
                    return;
                }

                std::string fnName;
                llvm::Type* valueTy = mTypeRegistry.resolveType(typeId, ctx);

                switch (typeId) {
                    case std::to_underlying(core::common::identifier::Type::BOOL): {
                        fnName = "rtIOSinkWriteBool";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT8): {
                        fnName = "rtIOSinkWriteInt8";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT16): {
                        fnName = "rtIOSinkWriteInt16";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT32): {
                        fnName = "rtIOSinkWriteInt32";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT64): {
                        fnName = "rtIOSinkWriteInt64";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::FLOAT): {
                        fnName = "rtIOSinkWriteFloat";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::DOUBLE): {
                        fnName = "rtIOSinkWriteDouble";
                        break;
                    }
                    default:
                        throw std::invalid_argument("ReadOperator: cant parse given type with typeid " + std::to_string(typeId));
                }

                auto* fnType = llvm::FunctionType::get(voidTy, { ptrTy, valueTy }, false);
                auto fn = module->getOrInsertFunction(fnName, fnType);
                builder.CreateCall(fn, { fieldName, fieldValue });
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H