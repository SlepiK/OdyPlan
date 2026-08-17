#include <core/physical/plan/detail/plan_construction.h>

namespace sierra::odyplan::v3::core::physical::plan::detail {
    segment::SegmentDataConsumers PlanConstruction::buildExit(const OperatorId operatorId) {
        if (const auto it = exitCache.find(operatorId); it != exitCache.end()) {
            return it->second;
        }

        const auto& chain = chains.get().at(operatorId);
        const auto edgeIt = outgoingEdges.get().find(operatorId);
        static const std::vector<OperatorPortReferenceConnection> emptyEdges{};
        const auto& outEdges = edgeIt != outgoingEdges.get().end() ? edgeIt->second : emptyEdges;

        std::unordered_map<shared::identifiers::PortId,
                           std::vector<OperatorPortReferenceConnection>>
            byPort;
        for (const auto& edge : outEdges) {
            byPort[edge.fromOperatorPortReference.portId].push_back(edge);
        }

        segment::SegmentDataConsumers result(chain.exitSchemas.size(), noopConsumer);

        for (std::size_t portIndex = 0; portIndex < chain.exitSchemas.size(); ++portIndex) {
            const auto found = byPort.find(portIndex);
            if (found == byPort.end()) {
                continue;
            }

            const auto& edgesForPort = found->second;

            if (portIndex >= result.size()) {
                result.resize(portIndex + 1);
            }

            if (edgesForPort.size() == 1) {
                result.at(portIndex) = buildEnter(edgesForPort.front());
            } else {
                segment::SegmentDataConsumers branches;
                branches.reserve(edgesForPort.size());

                for (std::size_t branchIdx = 0; branchIdx < edgesForPort.size(); ++branchIdx) {
                    const auto& edge = edgesForPort[branchIdx];
                    const std::string key = "fanout_" + std::to_string(operatorId) + "_" +
                                            std::to_string(portIndex) + "_" +
                                            std::to_string(branchIdx);
                    branches.push_back(wrapAsComputeContinuation(
                        key, chain.exitSchemas.at(portIndex), buildEnter(edge)));
                }
                result.at(portIndex) = [branches](segment::SegmentContext& sCtx,
                                              const data::DataPackage& tuple) {
                    for (const auto& branch : branches) {
                        branch(sCtx, tuple);
                    }
                };
            }
        }

        exitCache.emplace(operatorId, result);
        return result;
    }

    segment::SegmentDataConsumer
    PlanConstruction::buildEnter(const OperatorPortReferenceConnection& edge) {
        const auto id = edge.toOperatorPortReference.operatorId;
        const auto& chain = chains.get().at(id);
        const auto inputIndex = edge.toOperatorPortReference.portId;

        flow::IConsumer* entryOpr = chain.nodes.front().asConsumer;
        if (entryOpr == nullptr) {
            throw std::logic_error("Plan::buildEnter: chain has no valid consumer entry point");
        }

        segment::SegmentDataConsumers const tail = afterEntry(id);
        return [entryOpr, inputIndex, tail](segment::SegmentContext& sCtx,
                                            const data::DataPackage& tuple) {
            entryOpr->consume(sCtx, inputIndex, tuple, tail);
        };
    }

    segment::SegmentDataConsumers PlanConstruction::afterEntry(const OperatorId operatorId) {
        if (const auto it = afterEntryCache.find(operatorId); it != afterEntryCache.end()) {
            return it->second;
        }

        const auto& chain = chains.get().at(operatorId);

        segment::SegmentDataConsumers rest;

        if (chain.nodes.size() == 1) {
            rest = buildExit(operatorId);
        } else {
            rest = {buildContinue(operatorId, chain, 1)};
        }

        const auto degreeIt = inDegree.get().find(operatorId);
        const auto degree = degreeIt != inDegree.get().end() ? degreeIt->second : std::size_t{0};
        if (degree > 1) {
            if (chain.nodes.size() > 1 && chain.exitSchemas.size() > 1) {
                throw std::logic_error(
                    "Plan::codegen: Operation transform from a logical operator with > 1 Inputs to "
                    "more then 1 physical operator is not supported!");
            }

            const auto& schemas =
                chain.nodes.size() == 1
                    ? chain.exitSchemas
                    : port::Ports{chain.exitSchemas.empty() ? port::Port{}
                                                            : chain.exitSchemas.front()};

            segment::SegmentDataConsumers wrapped;
            for (std::size_t i = 0; i < rest.size(); ++i) {
                wrapped.push_back(compileAsFunction(operatorId, i, schemas.at(i), rest.at(i)));
            }
            rest = wrapped;
        }

        afterEntryCache.emplace(operatorId, rest);
        return rest;
    }

    segment::SegmentDataConsumer PlanConstruction::buildContinue(const OperatorId operatorId,
                                                                 const segment::Segment& chain,
                                                                 const std::size_t index) {
        if (index >= chain.nodes.size()) {
            const auto exit = buildExit(operatorId);
            return exit.empty() ? noopConsumer : exit.front();
        }

        const auto& node = chain.nodes.at(index);
        segment::SegmentDataConsumers rest;

        if (index + 1 >= chain.nodes.size()) {
            rest = buildExit(operatorId);
        } else {
            rest = {buildContinue(operatorId, chain, index + 1)};
        }

        flow::IConsumer* consumerNode = node.asConsumer;
        return [consumerNode, rest](segment::SegmentContext& sCtx, const data::DataPackage& tuple) {
            consumerNode->consume(sCtx, 0, tuple, rest);
        };
    }

    segment::SegmentDataConsumer
    PlanConstruction::compileAsFunction(const OperatorId operatorId,
                                        const shared::identifiers::PortId portId,
                                        const port::Port &portSchema,
                                        const segment::SegmentDataConsumer &inner) {
        return wrapAsComputeContinuation(
            "fanin_" + std::to_string(operatorId) + "_" + std::to_string(portId),
            portSchema, inner);
    }

    state::StateHandle PlanConstruction::getOrCreateComputeQueueState(const OperatorId operatorId, const shared::identifiers::PortId portId) {
        const std::string key = "fanin_queue_" + std::to_string(operatorId) + "_" + std::to_string(portId);

        if (const auto it = computeQueueCache.find(key); it != computeQueueCache.end()) {
            return it->second;
        }

        const auto handle = stateRegistry->createHandle(key, "rtComputeSegmentNullInit", {}, {}, "rtComputeSegmentDestroyIfStarted");

        computeQueueCache.emplace(key, handle);
        return handle;
    }

    segment::SegmentDataConsumer PlanConstruction::wrapAsComputeContinuation(const std::string &uniqueKey,
                                                                             const port::Port &portSchema,
                                                                             const segment::SegmentDataConsumer &
                                                                             inner) {
        auto *i64Ty = llvm::Type::getInt64Ty(*ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(*ctx);
        auto *voidTy = llvm::Type::getVoidTy(*ctx);
        std::vector<llvm::Type *> memberTypes;
        memberTypes.reserve(portSchema.size() + 1);
        memberTypes.push_back(i64Ty); // timestamp
        for (const auto &descriptor: portSchema) {
            memberTypes.push_back(codegenTypeRegistry->resolveType(descriptor.valueTypeId, *ctx));
        }
        auto *snapshotStructTy =
                llvm::StructType::create(*ctx, memberTypes, "ody_snapshot_" + uniqueKey);

        const auto &dataLayout = module->getDataLayout();
        const std::uint64_t structByteSize = dataLayout.getTypeAllocSize(snapshotStructTy);

        const auto queueState = stateRegistry->createHandle(
            "queue_" + uniqueKey, "rtComputeSegmentNullInit", {}, {},
            "rtComputeSegmentDestroyIfStarted");

        auto *continuationFn = llvm::Function::Create(
            llvm::FunctionType::get(voidTy, {ptrTy}, false),
            llvm::Function::InternalLinkage, "ody_continuation_" + uniqueKey, module);
        continuationFn->getArg(0)->setName("snapshot");

        auto *entryBB = llvm::BasicBlock::Create(*ctx, "continuationEntry", continuationFn);
        segment::SegmentContext computeCtx(continuationFn, entryBB, *ctx, *codegenTypeRegistry);

        llvm::Value *snapshotPtr = continuationFn->getArg(0);
        auto *runtimeRegistryGlobal = stateRegistry->getRuntimeTypeRegistryGlobal();
        llvm::Value *runtimeRegistryPtr = computeCtx.builder.CreateLoad(ptrTy, runtimeRegistryGlobal);

        const auto retainFn = module->getOrInsertFunction(
            "rtTypeRegistryRetain", llvm::FunctionType::get(ptrTy, {ptrTy, i64Ty, ptrTy}, false));
        const auto releaseFn = module->getOrInsertFunction(
            "rtTypeRegistryRelease", llvm::FunctionType::get(voidTy, {ptrTy, i64Ty, ptrTy}, false));

        data::DataPackage reconstructed;
        reconstructed.timestamp = computeCtx.builder.CreateLoad(
            i64Ty, computeCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtr, 0));

        reconstructed.fields.reserve(portSchema.size());
        for (std::size_t i = 0; i < portSchema.size(); ++i) {
            llvm::Value *slot = computeCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtr, i + 1);
            reconstructed.fields.push_back(computeCtx.builder.CreateLoad(memberTypes[i + 1], slot));
        }

        inner(computeCtx, reconstructed);

        for (std::size_t i = 0; i < portSchema.size(); ++i) {
            if (!memberTypes[i + 1]->isPointerTy()) {
                continue;
            }
            llvm::Value *typeIdConst = llvm::ConstantInt::get(i64Ty, portSchema.at(i).valueTypeId);
            computeCtx.builder.CreateCall(
                releaseFn, {runtimeRegistryPtr, typeIdConst, reconstructed.fields[i]});
        }

        const auto freeFn = module->getOrInsertFunction(
            "rtFanInFree", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        computeCtx.builder.CreateCall(freeFn, {snapshotPtr});
        computeCtx.builder.CreateRetVoid();

        return [this, snapshotStructTy, structByteSize, memberTypes, portSchema,
                    queueState, continuationFn, runtimeRegistryGlobal, retainFn]
        (segment::SegmentContext &sCtx, const data::DataPackage &tuple) {
            auto *i64TyLocal = llvm::Type::getInt64Ty(*ctx);
            auto *ptrTyLocal = llvm::PointerType::getUnqual(*ctx);
            auto *voidTyLocal = llvm::Type::getVoidTy(*ctx);

            const auto allocFn = module->getOrInsertFunction(
                "rtFanInAlloc", llvm::FunctionType::get(ptrTyLocal, {i64TyLocal}, false));
            llvm::Value *snapshotPtrLocal = sCtx.builder.CreateCall(
                allocFn, {llvm::ConstantInt::get(i64TyLocal, structByteSize)});

            sCtx.builder.CreateStore(
                tuple.timestamp,
                sCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtrLocal, 0));

            llvm::Value *runtimeRegistryPtrLocal =
                    sCtx.builder.CreateLoad(ptrTyLocal, runtimeRegistryGlobal);

            for (std::size_t i = 0; i < portSchema.size(); ++i) {
                llvm::Value *fieldVal = tuple.fields.at(i);
                llvm::Value *fieldSlot =
                        sCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtrLocal, i + 1);

                if (memberTypes[i + 1]->isPointerTy()) {
                    llvm::Value *typeIdConst =
                            llvm::ConstantInt::get(i64TyLocal, portSchema.at(i).valueTypeId);
                    fieldVal = sCtx.builder.CreateCall(
                        retainFn, {runtimeRegistryPtrLocal, typeIdConst, fieldVal});
                }
                sCtx.builder.CreateStore(fieldVal, fieldSlot);
            }

            const auto getOrCreateQueueFn = module->getOrInsertFunction(
                "rtComputeSegmentGetOrCreate",
                llvm::FunctionType::get(ptrTyLocal, {ptrTyLocal, ptrTyLocal}, false));
            llvm::Value *segmentHandle = sCtx.builder.CreateCall(
                getOrCreateQueueFn, {queueState.var, continuationFn});

            const auto enqueueFn = module->getOrInsertFunction(
                "rtComputeSegmentEnqueue",
                llvm::FunctionType::get(voidTyLocal, {ptrTyLocal, ptrTyLocal}, false));
            sCtx.builder.CreateCall(enqueueFn, {segmentHandle, snapshotPtrLocal});
        };
    }
}  // namespace sierra::odyplan::v3::core::physical::plan::detail