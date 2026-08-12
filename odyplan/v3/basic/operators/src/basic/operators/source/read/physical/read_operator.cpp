#include <basic/operators/source/read/physical/read_operator.h>

namespace sierra::odyplan::v3::basic::operators::source::read::physical {
    ReadOperator::ReadOperator(core::physical::port::Port outputPort,
                               const core::shared::identifiers::OperatorId lOpId,
                               nlohmann::json ioConfig,
                               const io::IOFormatType ioFormat,
                               core::logical::port::Port dataSchema,
                               const bool useProgramClock,
                               const std::optional<std::size_t> timestampFieldIndex,
                               const std::chrono::milliseconds rowDelay)
        : TypedSourceOperator(std::move(outputPort)),
          mLogicalOperatorId(lOpId),
          mIOFormat(ioFormat),
          mDataSchema(std::move(dataSchema)),
          mTimestampFieldIndex(timestampFieldIndex),
          mRowDelay(rowDelay),
          mIOConfig(std::move(ioConfig)),
          mUseProgramClock(useProgramClock) {
        if (mDataSchema.empty()) {
            throw std::invalid_argument("ReadOperator: dataSchema is empty");
        }
        if (mTimestampFieldIndex.has_value() && *mTimestampFieldIndex >= mDataSchema.size()) {
            throw std::out_of_range("ReadOperator: timestamp field index out of range");
        }
        if (mTimestampFieldIndex.has_value() &&
            mDataSchema.at(*mTimestampFieldIndex).valueTypeId !=
                std::to_underlying(core::shared::types::BuiltinValueType::INT64)) {
            throw std::invalid_argument("ReadOperator: timestamp field wrong type");
        }
    }

    void ReadOperator::produce(core::physical::segment::SegmentContext& segmentCtx,
                               const core::physical::segment::SegmentDataConsumers& consumers) {
        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
        llvm::Function* fn = segmentCtx.builder.GetInsertBlock()->getParent();

        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto* i1Ty = llvm::Type::getInt1Ty(ctx);
        auto* idTy = llvm::Type::getInt64Ty(ctx);

        llvm::Value* state = loadStatePtr(segmentCtx);

        auto* loopBB = llvm::BasicBlock::Create(ctx, "read_loop", fn);
        auto* produceBB = llvm::BasicBlock::Create(ctx, "read_produce", fn);
        auto* exitBB = llvm::BasicBlock::Create(ctx, "read_exit", fn);

        segmentCtx.builder.CreateBr(loopBB);
        segmentCtx.builder.SetInsertPoint(loopBB);

        const auto hasNextFn = module->getOrInsertFunction(
            "rtIOSourceHasNext", llvm::FunctionType::get(i1Ty, {ptrTy}, false));

        llvm::Value* hasNext = segmentCtx.builder.CreateCall(hasNextFn, {state});

        if (mRowDelay.count() > 0) {
            auto* i64Ty = llvm::Type::getInt64Ty(ctx);
            auto* voidTy = llvm::Type::getVoidTy(ctx);
            const auto sleepFn = module->getOrInsertFunction(
                "rtSleepMs", llvm::FunctionType::get(voidTy, {i64Ty}, false));
            segmentCtx.builder.CreateCall(sleepFn,
                                          {llvm::ConstantInt::get(i64Ty, mRowDelay.count())});
        }

        const auto stopFn =
            module->getOrInsertFunction("odyStop", llvm::FunctionType::get(i1Ty, {}, false));
        llvm::Value* stopped = segmentCtx.builder.CreateCall(stopFn, {});
        llvm::Value* notStopped = segmentCtx.builder.CreateNot(stopped);
        llvm::Value* shouldContinue = segmentCtx.builder.CreateAnd(hasNext, notStopped);

        segmentCtx.builder.CreateCondBr(shouldContinue, produceBB, exitBB);

        segmentCtx.builder.SetInsertPoint(produceBB);

        const auto getFieldFn = module->getOrInsertFunction(
            "rtIOSourceGetField", llvm::FunctionType::get(ptrTy, {ptrTy, ptrTy}, false));

        core::physical::data::DataPackage tuple;
        tuple.reserve(mDataSchema.size());

        for (const auto& dataField : mDataSchema) {
            llvm::Value* fieldName = segmentCtx.builder.CreateGlobalString(
                dataField.fieldName, "field_" + dataField.fieldName, 0, module);
            llvm::Value* strValue = segmentCtx.builder.CreateCall(getFieldFn, {state, fieldName});
            tuple.pushBack(codegenField(segmentCtx, strValue, dataField.valueTypeId));
        }

        if (mTimestampFieldIndex.has_value()) {
            tuple.timestamp = tuple.at(*mTimestampFieldIndex);
        } else {
            if (mUseProgramClock) {
                const auto nowFn =
                    module->getOrInsertFunction("rtProgramTimeNow", llvm::FunctionType::get(idTy, {}, false));
                tuple.timestamp = segmentCtx.builder.CreateCall(nowFn, {});
            } else {
                const auto nowFn =
                    module->getOrInsertFunction("rtTimeNow", llvm::FunctionType::get(idTy, {}, false));
                tuple.timestamp = segmentCtx.builder.CreateCall(nowFn, {});
            }
        }

        consumers.front()(segmentCtx, tuple);
        segmentCtx.builder.CreateBr(loopBB);

        segmentCtx.builder.SetInsertPoint(exitBB);
    }

    void ReadOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                    llvm::Module& module,
                                    llvm::LLVMContext& context) {
        auto* i8Ty = llvm::Type::getInt8Ty(context);
        auto* ptrTy = llvm::PointerType::getUnqual(context);

        const std::string path = mIOConfig.at("path").get<std::string>();

        auto* strConstant = llvm::ConstantDataArray::getString(context, path, true);

        // NOLINTBEGIN(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* gPath = new llvm::GlobalVariable(module,
                                               strConstant->getType(),
                                               true,
                                               llvm::GlobalValue::ExternalLinkage,
                                               strConstant,
                                               "read_path" + std::to_string(mLogicalOperatorId));

        llvm::Value* pathPtr = llvm::ConstantExpr::getInBoundsGetElementPtr(
            gPath->getValueType(),
            gPath,
            std::vector<llvm::Value*>{llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0),
                                      llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), 0)});
        // NOLINTEND(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)

        mState = stateRegistry.createHandle(
            "read_source_" + std::to_string(mLogicalOperatorId),
            "rtIOSourceCreate",
            {llvm::ConstantInt::get(i8Ty, static_cast<std::uint8_t>(mIOFormat)), pathPtr},
            {i8Ty, ptrTy},
            "rtIOSourceDestroy");
    }

    [[nodiscard]] llvm::Value*
    ReadOperator::loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const {
        auto* ptrTy = llvm::PointerType::getUnqual(segmentCtx.builder.getContext());
        return segmentCtx.builder.CreateLoad(ptrTy, mState.var);
    }

    llvm::Value* ReadOperator::codegenField(core::physical::segment::SegmentContext& segmentCtx,
                                            llvm::Value* stringValue,
                                            const core::shared::types::ValueTypeId typeId) {
        if (typeId == std::to_underlying(core::shared::types::BuiltinValueType::STRING)) {
            return stringValue;
        }

        llvm::LLVMContext& ctx = segmentCtx.builder.getContext();
        llvm::Module* module = segmentCtx.builder.GetInsertBlock()->getModule();
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        std::string fnName;
        llvm::Type* retTy = segmentCtx.codegenTypeRegistry.resolveType(typeId, ctx);

        switch (typeId) {
            case std::to_underlying(core::shared::types::BuiltinValueType::BOOL): {
                fnName = "rtParseBool";
                break;
            }
            case std::to_underlying(core::shared::types::BuiltinValueType::INT8): {
                fnName = "rtParseInt8";
                break;
            }
            case std::to_underlying(core::shared::types::BuiltinValueType::INT16): {
                fnName = "rtParseInt16";
                break;
            }
            case std::to_underlying(core::shared::types::BuiltinValueType::INT32): {
                fnName = "rtParseInt32";
                break;
            }
            case std::to_underlying(core::shared::types::BuiltinValueType::INT64): {
                fnName = "rtParseInt64";
                break;
            }
            case std::to_underlying(core::shared::types::BuiltinValueType::FLOAT): {
                fnName = "rtParseFloat";
                break;
            }
            case std::to_underlying(core::shared::types::BuiltinValueType::DOUBLE): {
                fnName = "rtParseDouble";
                break;
            }
            default:
                throw std::invalid_argument("ReadOperator: cant parse given type with typeid " +
                                            std::to_string(typeId));
        }

        auto* fnType = llvm::FunctionType::get(retTy, {ptrTy}, false);
        const auto fn = module->getOrInsertFunction(fnName, fnType);
        return segmentCtx.builder.CreateCall(fn, {stringValue});
    }
}  // namespace sierra::odyplan::v3::basic::operators::source::read::physical