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

    void Plan::addChain(OperatorId logicalOperatorId, segment::Segment chain) {
        if (chain.nodes.empty()) {
            throw std::invalid_argument("Plan::addChain: chain must not be empty!");
        }

        const auto& front = chain.nodes.front();
        if (front.asProducer == nullptr && front.asConsumer == nullptr) {
            throw std::invalid_argument(
                "Plan::addChain: first node in chain must have at least one valid entry point");
        }
        mOrder.push_back(logicalOperatorId);
        mChains.emplace(logicalOperatorId, std::move(chain));
    }

    void Plan::addOutgoingEdge(OperatorId logicalOperatorId,
                               std::vector<OperatorPortReferenceConnection> edges) {
        mOutgoingEdges.emplace(logicalOperatorId, std::move(edges));
    }

    void Plan::codegen(llvm::Module& module, llvm::LLVMContext& ctx) const {
        state::StateRegistry stateRegistry(&module, &ctx);
        for (const auto& opr : mOperators) {
            opr->declareState(stateRegistry, module, ctx);
        }
        stateRegistry.finish();

        std::vector<llvm::Function*> functions;
        std::vector<llvm::Function*> ioFunctionsRestart;
        std::vector<llvm::Function*> computeFunctionsRestart;

        std::unordered_map<OperatorId, std::size_t> inDegreeCache;
        std::unordered_map<OperatorId, std::set<std::size_t>> incomingInputIndices;
        for (const auto& [fromOperatorId, edgeLists] : mOutgoingEdges) {
            for (const auto& edge : edgeLists) {
                ++inDegreeCache[edge.toOperatorPortReference.operatorId];
                incomingInputIndices[edge.toOperatorPortReference.operatorId].insert(
                    edge.toOperatorPortReference.portId);
            }
        }

        segment::SegmentDataConsumer const noopConsumer = [](segment::SegmentContext&,
                                                             const data::DataPackage&) {};

        detail::PlanConstruction detailBuilder{.chains = mChains,
                                               .outgoingEdges = mOutgoingEdges,
                                               .inDegree = inDegreeCache,
                                               .noopConsumer = noopConsumer,
                                               .module = &module,
                                               .ctx = &ctx,
                                               .functions = &functions,
                                               .codegenTypeRegistry = mCodegenTypeRegistry,
                                               .ioRestartRoots = &ioFunctionsRestart,
                                               .computeRestartRoots = &computeFunctionsRestart};

        for (const auto& [id, chain] : mChains) {
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
            const auto& chain = mChains.at(id);
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

        std::vector<llvm::Function*> ioRoots = functions;
        for (auto* fn : ioFunctionsRestart) {
            ioRoots.push_back(fn);
        }

        codegenMain(module, ctx, ioRoots, computeFunctionsRestart, endFn);
    }

    Plan::OperatorId Plan::getSize() const {
        return mOperators.size();
    }

    void Plan::codegenMain(llvm::Module& module,
                           llvm::LLVMContext& ctx,
                           const std::vector<llvm::Function*>& ioRoots,
                           const std::vector<llvm::Function*>& computeRoots,
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
        auto computeSpawnFn = module.getOrInsertFunction(
            "rtComputeTaskSpawn", llvm::FunctionType::get(i64Ty, {ptrTy}, false));
        auto computeJoinFn = module.getOrInsertFunction(
            "rtComputeTaskJoin",
            llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {i64Ty}, false));

        std::vector<llvm::Value*> ioHandles;
        std::vector<llvm::Value*> computeHandles;

        ioHandles.reserve(ioRoots.size());
        for (auto* root : ioRoots) {
            ioHandles.push_back(builder.CreateCall(ioSpawnFn, {root}));
        }

        computeHandles.reserve(computeRoots.size());
        for (auto* root : computeRoots) {
            computeHandles.push_back(builder.CreateCall(computeSpawnFn, {root}));
        }

        for (auto* handle : ioHandles) {
            builder.CreateCall(ioJoinFn, {handle});
        }

        for (auto* handle : computeHandles) {
            builder.CreateCall(computeJoinFn, {handle});
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