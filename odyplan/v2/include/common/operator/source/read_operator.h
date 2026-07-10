#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_READOPERATOR_H
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

            }

        private:
            std::shared_ptr<io::IIOSource> mIOSource;
            const core::common::type::TypeRegistry& mTypeRegistry;
            core::physical::port::Port mOutputSchema;

            llvm::FunctionCallee getRtFn(llvm::IRBuilder<>& builder, const std::string& name, llvm::Type* returnType,
                std::vector<llvm::Type*> paramTypes) {
                auto* fnType = llvm::FunctionType::get(returnType, paramTypes, false);
                return builder.GetInsertBlock()->getModule()->getOrInsertFunction(name, fnType);
            }

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
                        rType = llvm::Type::getInt1Ty(ctx);
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::INT): {
                        fnName = "rtParseInt";
                        rType = llvm::Type::getIntNTy(ctx, sizeof(int));
                        break;
                    }
                    case std::to_underlying(core::common::identifier::Type::FLOAT): {
                        fnName = "rtParseFloat";
                        rType = llvm::Type::getFloatTy(ctx);
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