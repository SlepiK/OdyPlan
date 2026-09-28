#include <core/physical/plan/detail/plan_construction.h>

#include "core/physical/segment/segment_builder.h"

namespace sierra::odyplan::v3::core::physical::plan::detail {
    segment::SegmentDataConsumers PlanConstruction::buildExit(const OperatorId operatorId) {
        if (const auto it = exitCache.find(operatorId); it != exitCache.end()) {
            return it->second;
        }

        const auto& chain = segments.get().at(operatorId);
        const auto edgeIt = outgoingConnections.get().find(operatorId);
        static const std::vector<OperatorPortReferenceConnection> emptyEdges{};
        const auto& outEdges = edgeIt != outgoingConnections.get().end() ? edgeIt->second : emptyEdges;

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

            if (edgesForPort.size() == 1 && chain.exitSchemas.size() == 1) {
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
                result.at(portIndex) = segment::SegmentConsumerHandle{
                    [branches](segment::SegmentContext& sCtx,
                                              const data::DataPackage& tuple) {
                        for (const auto& branch : branches) {
                            branch.consume(sCtx, tuple);
                        }
                    },
                    [branches](segment::SegmentContext& sCtx,
                                              const data::PunctuationPackage& punctuation) {
                        for (const auto& branch : branches) {
                            branch.consumePunctuation(sCtx, punctuation);
                        }
                    }
                };



            }
        }

        exitCache.emplace(operatorId, result);
        return result;
    }

    segment::SegmentConsumerHandle
    PlanConstruction::buildEnter(const OperatorPortReferenceConnection& edge) {
        const auto id = edge.toOperatorPortReference.operatorId;
        const auto& chain = segments.get().at(id);
        const auto inputIndex = edge.toOperatorPortReference.portId;

        flow::IConsumer* entryOpr = chain.nodes.front().asConsumer;
        if (entryOpr == nullptr) {
            throw std::logic_error("Plan::buildEnter: chain has no valid consumer entry point");
        }

        segment::SegmentDataConsumers const tail = afterEntry(id);

        return segment::SegmentConsumerHandle{
            [entryOpr, inputIndex, tail](segment::SegmentContext& sCtx,
                                            const data::DataPackage& tuple) {
                entryOpr->consume(sCtx, inputIndex, tuple, tail);
            },
            [entryOpr, inputIndex, tail](segment::SegmentContext& sCtx,
                                            const data::PunctuationPackage& punctuation) {
                entryOpr->consumePunctuation(sCtx, inputIndex, punctuation, tail);
            }
        };
    }

    segment::SegmentDataConsumers PlanConstruction::afterEntry(const OperatorId operatorId) {
        if (const auto it = afterEntryCache.find(operatorId); it != afterEntryCache.end()) {
            return it->second;
        }

        const auto& chain = segments.get().at(operatorId);

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

    segment::SegmentConsumerHandle PlanConstruction::buildContinue(const OperatorId operatorId,
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

        return segment::SegmentConsumerHandle{
            [consumerNode, rest](segment::SegmentContext& sCtx, const data::DataPackage& tuple) {
                consumerNode->consume(sCtx, 0, tuple, rest);
            },
            [consumerNode, rest](segment::SegmentContext& sCtx, const data::PunctuationPackage& punctuation) {
                consumerNode->consumePunctuation(sCtx, 0, punctuation, rest);
            }
        };
    }

    segment::SegmentConsumerHandle
    PlanConstruction::compileAsFunction(const OperatorId operatorId,
                                        const shared::identifiers::PortId portId,
                                        const port::Port &portSchema,
                                        const segment::SegmentConsumerHandle &inner) {
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

        mComputeSegmentSlots.push_back(handle.var);

        computeQueueCache.emplace(key, handle);
        return handle;
    }

    segment::SegmentConsumerHandle PlanConstruction::wrapAsComputeContinuation(const std::string &uniqueKey,
                                                                             const port::Port &portSchema,
                                                                             const segment::SegmentConsumerHandle &
                                                                             inner) {
        auto *i64Ty = llvm::Type::getInt64Ty(*ctx);
        auto *i1Ty = llvm::Type::getInt1Ty(*ctx);
        auto *i8Ty = llvm::Type::getInt8Ty(*ctx);
        auto *ptrTy = llvm::PointerType::getUnqual(*ctx);
        auto *voidTy = llvm::Type::getVoidTy(*ctx);
        auto *sizeTTy = module->getDataLayout().getIntPtrType(*ctx);

        std::vector<llvm::Type *> memberTypes;
        memberTypes.reserve(portSchema.size() + 4);
        memberTypes.push_back(i64Ty); // startTs
        memberTypes.push_back(i64Ty); // startTB
        memberTypes.push_back(i64Ty); // endTs
        memberTypes.push_back(i64Ty); // endTB
        for (const auto &descriptor: portSchema) {
            memberTypes.push_back(codegenTypeRegistry->resolveType(descriptor.valueTypeId, *ctx));
        }
        auto *snapshotStructTy =
                llvm::StructType::create(*ctx, memberTypes, "ody_snapshot_" + uniqueKey);

        const auto &dataLayout = module->getDataLayout();
        const std::size_t structByteSize = dataLayout.getTypeAllocSize(snapshotStructTy);

        auto* punctStructTy = llvm::StructType::create(*ctx, {i64Ty, i1Ty}, "ody_punct_" + uniqueKey);
        const std::size_t punctByteSize = dataLayout.getTypeAllocSize(punctStructTy);

        const auto queueState = stateRegistry->createHandle(
            "queue_" + uniqueKey, "rtComputeSegmentNullInit", {}, {},
            "rtComputeSegmentDestroyIfStarted");

        mComputeSegmentSlots.push_back(queueState.var);

        auto *runtimeRegistryGlobal = stateRegistry->getRuntimeTypeRegistryGlobal();

        const auto retainFn = module->getOrInsertFunction(
            "rtTypeRegistryRetain", llvm::FunctionType::get(ptrTy, {ptrTy, i8Ty, ptrTy}, false));
        const auto releaseFn = module->getOrInsertFunction(
            "rtTypeRegistryRelease", llvm::FunctionType::get(voidTy, {ptrTy, i8Ty, ptrTy}, false));
        const auto freeFn = module->getOrInsertFunction(
            "rtFanInFree", llvm::FunctionType::get(voidTy, {ptrTy}, false));
        const auto allocFnDecl = module->getOrInsertFunction(
            "rtFanInAlloc", llvm::FunctionType::get(ptrTy, {sizeTTy}, false));

        auto *dataContinuationFn = segment::SegmentBuilder::createContinuation(
            *module, *ctx, *codegenTypeRegistry, "ody_continuation_" + uniqueKey,
            [&](segment::SegmentContext &computeCtx, llvm::Value *snapshotPtr) {
                llvm::Value *runtimeRegistryPtr = computeCtx.builder.CreateLoad(ptrTy, runtimeRegistryGlobal);

                llvm::Value* sTs = computeCtx.builder.CreateLoad(i64Ty, computeCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtr, 0));
                llvm::Value* sTB = computeCtx.builder.CreateLoad(i64Ty, computeCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtr, 1));
                llvm::Value* eTs = computeCtx.builder.CreateLoad(i64Ty, computeCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtr, 2));
                llvm::Value* eTB = computeCtx.builder.CreateLoad(i64Ty, computeCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtr, 3));

                const time::TimePoint reconstructedStart(sTs, sTB);
                const time::TimePoint reconstructedEnd(eTs, eTB);

                data::DataPackage reconstructed(time::TimeInterval(computeCtx.builder, reconstructedStart, reconstructedEnd));

                reconstructed.fields.reserve(portSchema.size());
                for (std::size_t i = 0; i < portSchema.size(); ++i) {
                    llvm::Value *slot = computeCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtr, i + 4);
                    reconstructed.fields.push_back(computeCtx.builder.CreateLoad(memberTypes[i + 4], slot));
                }

                inner.consume(computeCtx, reconstructed);

                for (std::size_t i = 0; i < portSchema.size(); ++i) {
                    if (!memberTypes[i + 4]->isPointerTy()) {
                        continue;
                    }
                    llvm::Value *typeIdConst = llvm::ConstantInt::get(i8Ty, portSchema.at(i).valueTypeId);
                    computeCtx.builder.CreateCall(
                        releaseFn, {runtimeRegistryPtr, typeIdConst, reconstructed.fields[i]});
                }

                computeCtx.builder.CreateCall(freeFn, {snapshotPtr});
            });

        auto *punctuationContinuationFn = segment::SegmentBuilder::createContinuation(
            *module, *ctx, *codegenTypeRegistry, "ody_punct_continuation_" + uniqueKey,
            [&](segment::SegmentContext &computeCtx, llvm::Value *punctPtr) {
                data::PunctuationPackage reconstructed;
                reconstructed.watermark = computeCtx.builder.CreateLoad(
                    i64Ty, computeCtx.builder.CreateStructGEP(punctStructTy, punctPtr, 0));
                reconstructed.isFinal = computeCtx.builder.CreateLoad(
                    i1Ty, computeCtx.builder.CreateStructGEP(punctStructTy, punctPtr, 1));

                inner.consumePunctuation(computeCtx, reconstructed);

                computeCtx.builder.CreateCall(freeFn, {punctPtr});
            }
        );

        return segment::SegmentConsumerHandle{
            [this, snapshotStructTy, structByteSize, memberTypes, portSchema,
                    queueState, dataContinuationFn, runtimeRegistryGlobal, retainFn, ptrTy, sizeTTy, i8Ty, allocFnDecl]
        (segment::SegmentContext &sCtx, const data::DataPackage &tuple) {
                llvm::Value *snapshotPtrLocal = sCtx.builder.CreateCall(
                    allocFnDecl, {llvm::ConstantInt::get(sizeTTy, structByteSize)});

                sCtx.builder.CreateStore(
                    tuple.timeInterval.getStart().getPoint(),
                    sCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtrLocal, 0));
                sCtx.builder.CreateStore(
                    tuple.timeInterval.getStart().getTiBreaker(),
                    sCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtrLocal, 1));
                sCtx.builder.CreateStore(
                    tuple.timeInterval.getEnd().getPoint(),
                    sCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtrLocal, 2));
                sCtx.builder.CreateStore(
                    tuple.timeInterval.getEnd().getTiBreaker(),
                    sCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtrLocal, 3));

                llvm::Value *runtimeRegistryPtrLocal =
                        sCtx.builder.CreateLoad(ptrTy, runtimeRegistryGlobal);

                for (std::size_t i = 0; i < portSchema.size(); ++i) {
                    llvm::Value *fieldVal = tuple.fields.at(i);
                    llvm::Value *fieldSlot =
                            sCtx.builder.CreateStructGEP(snapshotStructTy, snapshotPtrLocal, i + 4);

                    if (memberTypes[i + 4]->isPointerTy()) {
                        llvm::Value *typeIdConst =
                                llvm::ConstantInt::get(i8Ty, portSchema.at(i).valueTypeId);
                        fieldVal = sCtx.builder.CreateCall(
                            retainFn, {runtimeRegistryPtrLocal, typeIdConst, fieldVal});
                    }
                    sCtx.builder.CreateStore(fieldVal, fieldSlot);
                }

                segment::SegmentBuilder::enqueueToComputeSegment(sCtx, queueState.var, dataContinuationFn, snapshotPtrLocal);
            },
            [this, punctStructTy, punctByteSize, queueState, punctuationContinuationFn, allocFnDecl, sizeTTy]
    (segment::SegmentContext &sCtx, const data::PunctuationPackage &punctuation) {
                llvm::Value *punctPtrLocal = sCtx.builder.CreateCall(
                    allocFnDecl, {llvm::ConstantInt::get(sizeTTy, punctByteSize)});

                sCtx.builder.CreateStore(
                    punctuation.watermark, sCtx.builder.CreateStructGEP(punctStructTy, punctPtrLocal, 0));

                sCtx.builder.CreateStore(
                    punctuation.isFinal,sCtx.builder.CreateStructGEP(punctStructTy, punctPtrLocal, 1));

                segment::SegmentBuilder::enqueueToComputeSegment(sCtx, queueState.var, punctuationContinuationFn,
                                                                 punctPtrLocal);
            }
        };
    }


    const std::vector<llvm::GlobalVariable*>& PlanConstruction::getComputeSegmentSlots() const {
        return mComputeSegmentSlots;
    }
}  // namespace sierra::odyplan::v3::core::physical::plan::detail