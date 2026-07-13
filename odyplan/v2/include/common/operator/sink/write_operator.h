#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H

#include <memory>
#include <utility>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

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

            explicit WriteOperator(std::shared_ptr<io::IIOSink> dataSink)
                : mDataSink(std::move(dataSink)) {
                if (!mDataSink) {
                    throw std::invalid_argument("WriteOperator: dataSink must not be null");
                }
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const std::shared_ptr<io::IIOSink> getDataSink() const {
                return mDataSink;
            }
        private:
            std::shared_ptr<io::IIOSink> mDataSink;
        };
    }

    namespace physical {
        class WriteOperator : public core::physical::opr::TypedSinkOperator {
        public:
            WriteOperator(const core::physical::port::Port &inputPort, std::shared_ptr<common::io::IIOSink> ioSink ,const core::common::type::TypeRegistry& typeRegistry)
                : TypedSinkOperator(inputPort),
                    mIOSink(std::move(ioSink)),
                    mTypeRegistry(typeRegistry) {
            }

            void consume(llvm::IRBuilder<> &builder, const core::physical::port::PortTuple &tuple, core::physical::port::PortTupleConsumer /*consumer*/) override {
                llvm::LLVMContext& ctx = builder.getContext();
                llvm::Module* module = builder.GetInsertBlock()->getModule();

                auto* i8Ptr = llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 0);
                auto* voidTy = llvm::Type::getVoidTy(ctx);

                auto beginFn = module->getOrInsertFunction("rtIOSinkBeginData", llvm::FunctionType::get(voidTy, {i8Ptr}, false));

                auto writeFieldFn = module->getOrInsertFunction("rtIOSinkWriteData", llvm::FunctionType::get(voidTy, {i8Ptr, i8Ptr, i8Ptr}, false));

                auto endFn = module->getOrInsertFunction("rtIOSinkEndData", llvm::FunctionType::get(voidTy, {i8Ptr}, false));

                //ToDO: work here
            }
        protected:
        private:
            std::shared_ptr<io::IIOSink> mIOSink;
            const core::common::type::TypeRegistry& mTypeRegistry;
            core::logical::port::Port mDataSchema;

            llvm::Value* codegenField(llvm::IRBuilder<>& builder, llvm::Value* stringValue,
                core::common::identifier::TypeId typeId) {
                if (typeId == std::to_underlying(core::common::identifier::Type::STRING))
                    return stringValue;

                std::string fnName;
                llvm::Type* rType = nullptr;
                llvm::LLVMContext& ctx = builder.getContext();

                switch (typeId) {
                    case std::to_underlying(core::common::identifier::Type::BOOL): {
                        fnName = "rtToBool";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT8): {
                        fnName = "rtToInt8";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT16): {
                        fnName = "rtToInt16";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT32): {
                        fnName = "rtToInt32";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT64): {
                        fnName = "rtToInt64";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::FLOAT): {
                        fnName = "rtToFloat";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::DOUBLE): {
                        fnName = "rtToDouble";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    default:
                        throw std::invalid_argument("ReadOperator: cant parse given type with typeid " + std::to_string(typeId));
                }

                auto* ptrType = llvm::PointerType::getUnqual(ctx);
                auto* fnType = llvm::FunctionType::get(rType, { ptrType }, false);
                auto fn = builder.GetInsertBlock()->getModule()->getOrInsertFunction(fnName, fnType);
                return builder.CreateCall(fn, { stringValue });
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_WRITEOPERATOR_H