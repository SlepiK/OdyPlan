#include <basic/operators/source/read/physical/read_operator.h>

namespace sierra::odyplan::v3::basic::operators::source::read::physical {
    ReadOperator::ReadOperator(core::physical::port::Port outputPort,
                               nlohmann::json ioConfig,
                               const io::IOFormatType ioFormat,
                               core::logical::port::Port dataSchema,
                               details::PReadOperatorOptions options)
        : TypedSourceOperator(std::move(outputPort)),
          mIOFormat(ioFormat),
          mDataSchema(std::move(dataSchema)),
          mOptions(options),
          mIOConfig(std::move(ioConfig)) {
        if (mDataSchema.empty()) {
            throw std::invalid_argument("ReadOperator: dataSchema is empty");
        }
        if (mOptions.startTimestampPosition.has_value() && *mOptions.startTimestampPosition >= mDataSchema.size()) {
            throw std::out_of_range("ReadOperator: startTimestamp field index out of range");
        }
        if (mOptions.endTimestampPosition.has_value() && *mOptions.endTimestampPosition >= mDataSchema.size()) {
            throw std::out_of_range("ReadOperator: endTimestamp field index out of range");
        }
        if (mOptions.startTimestampPosition.has_value() && core::shared::types::isTimestamp(mDataSchema.at(*mOptions.startTimestampPosition).valueTypeId)) {
            if (core::shared::types::isIntegral(mDataSchema.at(*mOptions.startTimestampPosition).valueTypeId)) {
                mDataSchema.at(*mOptions.startTimestampPosition).valueTypeId = std::to_underlying(core::shared::types::BuiltinValueType::TIMESTAMP);
            }
            throw std::invalid_argument("ReadOperator: startTimestamp field wrong type");
        }
        if (mOptions.endTimestampPosition.has_value() && core::shared::types::isTimestamp(mDataSchema.at(*mOptions.endTimestampPosition).valueTypeId)) {
            if (core::shared::types::isIntegral(mDataSchema.at(*mOptions.endTimestampPosition).valueTypeId)) {
                mDataSchema.at(*mOptions.endTimestampPosition).valueTypeId = std::to_underlying(core::shared::types::BuiltinValueType::TIMESTAMP);
            }
            throw std::invalid_argument("ReadOperator: endTimestamp field wrong type");
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

        if (mOptions.delay.count() > 0) {
            auto* i64Ty = llvm::Type::getInt64Ty(ctx);
            auto* voidTy = llvm::Type::getVoidTy(ctx);
            const auto sleepFn = module->getOrInsertFunction(
                "rtSleepMs", llvm::FunctionType::get(voidTy, {i64Ty}, false));
            segmentCtx.builder.CreateCall(sleepFn,
                                          {llvm::ConstantInt::get(i64Ty, mOptions.delay.count())});
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

        std::vector<llvm::Value*> rawValues;
        rawValues.reserve(mDataSchema.size());

        for (const auto& dataField : mDataSchema) {
            llvm::Value* fieldName = segmentCtx.builder.CreateGlobalString(
                dataField.fieldName, "field_" + dataField.fieldName, 0, module);
            llvm::Value* strValue = segmentCtx.builder.CreateCall(getFieldFn, {state, fieldName});
            rawValues.push_back(codegenField(segmentCtx, strValue, dataField.valueTypeId));
        }

        const core::physical::time::TimePoint stp = [&]() {
            if (mOptions.startTimestampPosition.has_value()) {
                return core::physical::time::TimePoint(segmentCtx.builder, rawValues.at(*mOptions.startTimestampPosition));
            }
            if (mOptions.useProgramClock) {
                return core::physical::time::TimePoint::executionTime(segmentCtx.builder);
            }
            return core::physical::time::TimePoint::systemTime(segmentCtx.builder);
        }();

        const core::physical::time::TimeInterval ti = [&]() {
            if (mOptions.endTimestampPosition.has_value()) {
                return core::physical::time::TimeInterval(segmentCtx.builder, stp, core::physical::time::TimePoint(segmentCtx.builder, rawValues.at(*mOptions.endTimestampPosition)));
            }
            return core::physical::time::TimeInterval(segmentCtx.builder, stp);
        }();

        core::physical::data::DataPackage tuple(ti);
        tuple.fields = std::move(rawValues);

        consumers.front().consume(segmentCtx, tuple);

        llvm::Value* allowedLatenessConst = llvm::ConstantInt::get(idTy, mOptions.allowedLateness.count());
        llvm::Function* smaxFn = llvm::Intrinsic::getOrInsertDeclaration(module, llvm::Intrinsic::smax, {idTy});
        llvm::Value* guardedTs = segmentCtx.builder.CreateCall(smaxFn, {tuple.timeInterval.getStart().getPoint(), llvm::ConstantInt::get(idTy, 0)});
        llvm::Value *candidateWatermark = segmentCtx.builder.CreateSub(guardedTs, allowedLatenessConst);

        llvm::Value* lastWatermark = segmentCtx.builder.CreateLoad(idTy, mWatermarkState);
        llvm::Value* sAdvance = segmentCtx.builder.CreateICmpSGT(candidateWatermark, lastWatermark);

        auto* advanceBB = llvm::BasicBlock::Create(ctx, "watermark_advance", fn);
        auto* afterWatermarkBB = llvm::BasicBlock::Create(ctx, "watermark_after", fn);
        segmentCtx.builder.CreateCondBr(sAdvance, advanceBB, afterWatermarkBB);

        segmentCtx.builder.SetInsertPoint(advanceBB);
        segmentCtx.builder.CreateStore(candidateWatermark, mWatermarkState);

        const core::physical::data::PunctuationPackage punctuation{
            .watermark = candidateWatermark,
            .isFinal = llvm::ConstantInt::getFalse(ctx)
        };

        consumers.front().consumePunctuation(segmentCtx, punctuation);

        segmentCtx.builder.CreateBr(afterWatermarkBB);
        segmentCtx.builder.SetInsertPoint(afterWatermarkBB);

        segmentCtx.builder.CreateBr(loopBB);
        segmentCtx.builder.SetInsertPoint(exitBB);

        llvm::Value* finalWatermark = llvm::ConstantInt::get(idTy, std::numeric_limits<std::int64_t>::max());
        const core::physical::data::PunctuationPackage finalPunctuation{
            .watermark = finalWatermark,
            .isFinal = llvm::ConstantInt::getTrue(ctx)
        };
        consumers.front().consumePunctuation(segmentCtx, finalPunctuation);
    }

    void ReadOperator::declareState(core::physical::state::StateRegistry& stateRegistry,
                                    llvm::Module& module,
                                    llvm::LLVMContext& context) {
        auto* i8Ty = llvm::Type::getInt8Ty(context);
        auto* i64Ty = llvm::Type::getInt64Ty(context);
        auto* ptrTy = llvm::PointerType::getUnqual(context);

        const std::string path = mIOConfig.at("path").get<std::string>();

        auto* strConstant = llvm::ConstantDataArray::getString(context, path, true);

        // NOLINTBEGIN(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        auto* gPath = new llvm::GlobalVariable(module,
                                               strConstant->getType(),
                                               true,
                                               llvm::GlobalValue::PrivateLinkage,
                                               strConstant,
                                               "read_path" + std::to_string(getPId()));

        llvm::Value* pathPtr = llvm::ConstantExpr::getInBoundsGetElementPtr(
            gPath->getValueType(),
            gPath,
            std::vector<llvm::Value*>{llvm::ConstantInt::get(i64Ty, 0),
                                      llvm::ConstantInt::get(i64Ty, 0)});

        // NOLINTEND(cppcoreguidelines-owning-memory,clang-analyzer-cplusplus.NewDeleteLeaks)
        mState = stateRegistry.createHandle(
            "read_source_" + std::to_string(getPId()),
            "rtIOSourceCreate",
            {llvm::ConstantInt::get(i8Ty, static_cast<std::uint8_t>(mIOFormat)), pathPtr},
            {i8Ty, ptrTy},
            "rtIOSourceDestroy");

        mWatermarkState = new llvm::GlobalVariable(module,
            i64Ty, false, llvm::GlobalValue::PrivateLinkage,
            llvm::ConstantInt::get(i64Ty, std::numeric_limits<std::int64_t>::min()),
            "read_watermark_" + std::to_string(getPId()));
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