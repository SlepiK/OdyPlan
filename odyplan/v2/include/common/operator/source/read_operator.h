#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
#include <llvm/IR/Module.h>

#include "common/io/io_data.h"
#include "common/io/io_source.h"
#include "core/common/type/type_registry.h"
#include "core/logical/operator/source/source_operator.h"
#include "core/physical/operator/source/source_operator.h"

namespace sierra::odyplan::v2::common::opr::source {
    namespace logical {
        class ReadOperator : core::logical::opr::TypedSourceOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::READ);

            ReadOperator(core::logical::port::Port outputPort, std::shared_ptr<io::IIOSource> dataSource)
                : TypedSourceOperator(std::move(outputPort)),
                    mDataSource(std::move(dataSource)) {
                if (!mDataSource) {
                    throw std::invalid_argument("ReadOperator: dataSource must not be null");
                }
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const std::shared_ptr<io::IIOSource> getDataSource() const {
                return mDataSource;
            }

        private:
            std::shared_ptr<io::IIOSource> mDataSource;
        };
    }

    namespace physical {
        class ReadOperator : core::physical::opr::TypedSourceOperator {
        public:
            ReadOperator(core::physical::port::Port outputPort, std::shared_ptr<common::io::IIOSource> ioSource,
                const core::common::type::TypeRegistry& typeRegistry)
                    : TypedSourceOperator(std::move(outputPort)),
                    mIOSource(std::move(ioSource)),
                    mTypeRegistry(typeRegistry){
            }

            void produce(llvm::IRBuilder<>& builder, core::physical::port::PortTupleConsumer consumer) override {
                llvm::LLVMContext& ctx = builder.getContext();
                llvm::Function* fn = builder.GetInsertBlock()->getParent();

                auto* ptrTy = llvm::PointerType::getUnqual(ctx);
                auto* voidTy = llvm::Type::getVoidTy(ctx);
                auto* i1Ty = llvm::Type::getInt1Ty(ctx);
                auto* module = builder.GetInsertBlock()->getModule();

                auto* loopBB = llvm::BasicBlock::Create(ctx, "read_loop", fn);
                auto* produceBB = llvm::BasicBlock::Create(ctx, "read_produce", fn);
                auto* exitBB = llvm::BasicBlock::Create(ctx, "read_exit", fn);

                builder.CreateBr(loopBB);
                builder.SetInsertPoint(loopBB);

                auto hasNextFn = module->getOrInsertFunction("rtIOSourceHasNext", llvm::FunctionType::get(i1Ty, {}, false));
                llvm::Value* hasNext = builder.CreateCall(hasNextFn, {});
                builder.CreateCondBr(hasNext, produceBB, exitBB);

                builder.SetInsertPoint(produceBB);

                auto getFieldFn = module->getOrInsertFunction("rtIOSourceGetField", llvm::FunctionType::get(i1Ty, {ptrTy}, false));

                core::physical::port::PortTuple tuple;
                tuple.reserve(mDataSchema.size());

                for (const auto& dataField : mDataSchema) {
                    llvm::Value* fieldName = builder.CreateGlobalString(dataField.fieldName, "field_" + dataField.fieldName);
                    llvm::Value* strValue = builder.CreateCall(getFieldFn, {fieldName});
                    tuple.push_back(codegenField(builder, strValue, dataField.typeId));
                }

                consumer(builder, tuple);

                builder.CreateBr(loopBB);
                builder.SetInsertPoint(exitBB);
            }

        private:
            std::shared_ptr<io::IIOSource> mIOSource;
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
                        fnName = "rtParseBool";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT8): {
                        fnName = "rtParseInt8";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT16): {
                        fnName = "rtParseInt16";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT32): {
                        fnName = "rtParseInt32";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT64): {
                        fnName = "rtParseInt64";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::FLOAT): {
                        fnName = "rtParseFloat";
                        rType = mTypeRegistry.resolveType(typeId, ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::DOUBLE): {
                        fnName = "rtParseDouble";
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

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H