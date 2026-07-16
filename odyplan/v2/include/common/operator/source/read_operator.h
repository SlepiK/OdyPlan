#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
#include <llvm/IR/Module.h>
#include <nlohmann/json.hpp>

#include "common/io/io_data.h"
#include "common/io/io_format.h"
#include "common/io/io_source.h"
#include "core/common/type/type_registry.h"
#include "core/logical/operator/source/source_operator.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::common::opr::source {
    namespace logical {
        class ReadOperator : public core::logical::opr::TypedSourceOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::READ);

            ReadOperator(core::logical::port::Port outputPort, const io::IOFormat ioFormat, nlohmann::json ioConfig)
                : TypedSourceOperator(std::move(outputPort)),
                    mIOFormat(ioFormat),
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
        class ReadOperator : public core::physical::opr::TypedSourceOperator {
        public:
            ReadOperator(core::physical::port::Port outputPort, const io::IOSourceId ioSourceId,
                const core::common::type::TypeRegistry& typeRegistry, core::logical::port::Port dataSchema)
                    : TypedSourceOperator(std::move(outputPort)),
                    mIOSourceId(ioSourceId),
                    mTypeRegistry(typeRegistry),
                    mDataSchema(std::move(dataSchema)) {
                if (mDataSchema.empty()) {
                    throw std::invalid_argument("ReadOperator: dataSchema is empty");
                }
            }

            void produce(core::physical::segment::SegmentContext& segmentCtx, core::physical::port::PortTupleConsumer consumer) override {
                llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
                llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
                llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();

                auto* ptrTy = llvm::PointerType::getUnqual(ctx);
                auto* i1Ty = llvm::Type::getInt1Ty(ctx);
                auto* idTy = llvm::Type::getInt64Ty(ctx);

                llvm::Value* sourceId = llvm::ConstantInt::get(idTy, mIOSourceId);

                auto* loopBB = llvm::BasicBlock::Create(ctx, "read_loop", fn);
                auto* produceBB = llvm::BasicBlock::Create(ctx, "read_produce", fn);
                auto* exitBB = llvm::BasicBlock::Create(ctx, "read_exit", fn);

                segmentCtx.builder.CreateBr(loopBB);
                segmentCtx.builder.SetInsertPoint(loopBB);

                auto hasNextFn = module->getOrInsertFunction("rtIOSourceHasNext", llvm::FunctionType::get(i1Ty, {idTy}, false));
                llvm::Value* hasNext = segmentCtx.builder.CreateCall(hasNextFn, {sourceId});
                segmentCtx.builder.CreateCondBr(hasNext, produceBB, exitBB);

                segmentCtx.builder.SetInsertPoint(produceBB);

                auto getFieldFn = module->getOrInsertFunction("rtIOSourceGetField", llvm::FunctionType::get(ptrTy, {idTy, ptrTy}, false));

                core::physical::port::PortTuple tuple;
                tuple.reserve(mDataSchema.size());

                for (const auto& dataField : mDataSchema) {
                    llvm::Value* fieldName = segmentCtx.builder.CreateGlobalString(dataField.fieldName, "field_" + dataField.fieldName, 0, module);
                    llvm::Value* strValue = segmentCtx.builder.CreateCall(getFieldFn, {sourceId, fieldName});
                    tuple.push_back(codegenField(segmentCtx.builder, strValue, dataField.typeId));
                }

                consumer(segmentCtx, tuple);
                segmentCtx.builder.CreateBr(loopBB);

                segmentCtx.builder.SetInsertPoint(exitBB);
            }

        private:
            io::IOSourceId mIOSourceId;
            const core::common::type::TypeRegistry& mTypeRegistry;
            core::logical::port::Port mDataSchema;

            llvm::Value* codegenField(llvm::IRBuilder<>& builder, llvm::Value* stringValue,
                const core::common::identifier::TypeId typeId) {
                if (typeId == std::to_underlying(core::common::identifier::Type::STRING)) {
                    return stringValue;
                }

                llvm::LLVMContext& ctx = builder.getContext();
                llvm::Module* module = builder.GetInsertBlock()->getModule();
                auto* ptrTy = llvm::PointerType::getUnqual(ctx);

                std::string fnName;
                llvm::Type* retTy = mTypeRegistry.resolveType(typeId, ctx);

                switch (typeId) {
                    case std::to_underlying(core::common::identifier::Type::BOOL): {
                        fnName = "rtParseBool";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT8): {
                        fnName = "rtParseInt8";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT16): {
                        fnName = "rtParseInt16";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT32): {
                        fnName = "rtParseInt32";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT64): {
                        fnName = "rtParseInt64";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::FLOAT): {
                        fnName = "rtParseFloat";
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::DOUBLE): {
                        fnName = "rtParseDouble";
                        break;
                    }
                    default:
                        throw std::invalid_argument("ReadOperator: cant parse given type with typeid " + std::to_string(typeId));
                }

                auto* fnType = llvm::FunctionType::get(retTy, { ptrTy }, false);
                auto fn = module->getOrInsertFunction(fnName, fnType);
                return builder.CreateCall(fn, { stringValue });
            }
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H