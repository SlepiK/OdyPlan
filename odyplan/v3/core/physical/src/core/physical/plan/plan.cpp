#include <core/physical/plan/plan.h>
#include <set>

namespace sierra::odyplan::v3::core::physical::plan {
    Plan::Plan(const types::CodegenTypeRegistry& codegenTypeRegistry)
        : mCodegenTypeRegistry(&codegenTypeRegistry) {}

    void Plan::addOperator(std::unique_ptr<IOperator> opr) {
        if (!opr) {
            throw std::invalid_argument("Plan::addOperator: opr must not be nullptr!");
        }

        mOperators.push_back(std::move(opr));
    }

    void Plan::addSegment(OperatorId logicalOperatorId, segment::Segment segment) {
        if (segment.nodes.empty()) {
            throw std::invalid_argument("Plan::addChain: chain must not be empty!");
        }

        const auto& front = segment.nodes.front();
        if (front.asProducer == nullptr && front.asConsumer == nullptr) {
            throw std::invalid_argument(
                "Plan::addChain: first node in chain must have at least one valid entry point");
        }
        mOrder.push_back(logicalOperatorId);
        mSegments.emplace(logicalOperatorId, std::move(segment));
    }

    void Plan::addOutgoingConnection(OperatorId logicalOperatorId,
                               std::vector<OperatorPortReferenceConnection> connections) {
        mOutgoingConnections.emplace(logicalOperatorId, std::move(connections));
    }

    void Plan::codegen(llvm::Module& module, llvm::LLVMContext& ctx) const {
        state::StateRegistry stateRegistry(&module, &ctx);
        for (const auto& opr : mOperators) {
            opr->declareState(stateRegistry, module, ctx);
        }

        std::vector<llvm::Function*> functions;
        std::vector<llvm::Function*> ioFunctionsRestart;
        std::vector<llvm::Function*> computeFunctionsRestart;

        std::unordered_map<OperatorId, std::size_t> inDegreeCache;
        std::unordered_map<OperatorId, std::set<std::size_t>> incomingInputIndices;
        for (const auto& [fromOperatorId, edgeLists] : mOutgoingConnections) {
            for (const auto& edge : edgeLists) {
                ++inDegreeCache[edge.toOperatorPortReference.operatorId];
                incomingInputIndices[edge.toOperatorPortReference.operatorId].insert(
                    edge.toOperatorPortReference.portId);
            }
        }

        segment::SegmentDataConsumer const noopConsumer = [](segment::SegmentContext&,
                                                             const data::DataPackage&) {};

        detail::PlanConstruction detailBuilder(mSegments,
                                       mOutgoingConnections,
                                       inDegreeCache,
                                       noopConsumer,
                                       &module,
                                       &ctx,
                                       &functions,
                                       mCodegenTypeRegistry,
                                       &ioFunctionsRestart,
                                       &computeFunctionsRestart,
                                       &stateRegistry);

        for (const auto& [id, chain] : mSegments) {
            if (const bool isSource = chain.nodes.front().asConsumer == nullptr) {
                auto* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
                auto* fn = llvm::Function::Create(fnType,
                                                  llvm::Function::ExternalLinkage,
                                                  "ody_rsource_" + std::to_string(id),
                                                  &module);
                auto* entryBB = llvm::BasicBlock::Create(ctx, "basicEntry", fn);
                segment::SegmentContext segmentContext(fn, entryBB, ctx, *mCodegenTypeRegistry);

                chain.nodes.front().asProducer->produce(segmentContext,
                                                        detailBuilder.afterEntry(id));
                segmentContext.builder.CreateRetVoid();
                functions.push_back(fn);
            }
        }

        auto* endFnType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* endFn = llvm::Function::Create(
            endFnType, llvm::Function::ExternalLinkage, "ody_end_phase", &module);
        auto* endEntryBB = llvm::BasicBlock::Create(ctx, "basicEntry", endFn);

        segment::SegmentContext segmentContext(endFn, endEntryBB, ctx, *mCodegenTypeRegistry);

        for (const auto id : mOrder) {
            const auto& chain = mSegments.at(id);
            flow::IConsumer* entryOpr = chain.nodes.front().asConsumer;
            if (entryOpr == nullptr) {
                continue;
            }
            const auto downstream = detailBuilder.afterEntry(id);

            const auto indexIt = incomingInputIndices.find(id);
            if (indexIt == incomingInputIndices.end()) {
                continue;
            }

            for (const auto inputIndex : indexIt->second) {
                entryOpr->end(segmentContext, inputIndex, downstream);
            }
        }
        segmentContext.builder.CreateRetVoid();

        stateRegistry.finish();

        codegenMain(module, ctx, functions, endFn);
    }

    Plan::OperatorId Plan::getSize() const {
        return mOperators.size();
    }

    void Plan::codegenMain(llvm::Module& module,
                           llvm::LLVMContext& ctx,
                           const std::vector<llvm::Function*>& functions,
                           llvm::Function* endPhase) {
        auto* functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* mainFn = llvm::Function::Create(
            functionType, llvm::Function::ExternalLinkage, "ody_main", &module);

        auto* basicBlock = llvm::BasicBlock::Create(ctx, "basicEntry", mainFn);

        llvm::IRBuilder builder(basicBlock);

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);

        auto ioSpawnFn = module.getOrInsertFunction("rtIOTaskSpawn",
                                                    llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        auto ioJoinFn = module.getOrInsertFunction(
            "rtIOTaskJoin", llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {i64Ty}, false));

        std::vector<llvm::Value*> ioHandles;
        std::vector<llvm::Value*> computeHandles;

        ioHandles.reserve(functions.size());
        for (auto* root : functions) {
            ioHandles.push_back(builder.CreateCall(ioSpawnFn, {root}));
        }

        for (auto* handle : ioHandles) {
            builder.CreateCall(ioJoinFn, {handle});
        }

        if (endPhase != nullptr) {
            builder.CreateCall(endPhase);
        }

        const auto endFn = module.getOrInsertFunction(
            "rtEnd", llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false));
        builder.CreateCall(endFn, {});

        builder.CreateRetVoid();
    }
}  // namespace sierra::odyplan::v3::core::physical::plan