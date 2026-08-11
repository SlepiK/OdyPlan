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
                for (const auto& edge : edgesForPort) {
                    branches.push_back(buildEnter(edge));
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

    void PlanConstruction::spawnBlockingRoot(const OperatorId operatorId,
                                             const std::string& suffix,
                                             flow::IProducer* producer,
                                             const segment::SegmentDataConsumers& next) const {
        auto* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx), {}, false);
        auto* fn =
            llvm::Function::Create(fnType,
                                   llvm::Function::ExternalLinkage,
                                   "ody_root_blocking_" + std::to_string(operatorId) + "_" + suffix,
                                   module);
        auto* entryBB = llvm::BasicBlock::Create(*ctx, "basicEntry", fn);

        segment::SegmentContext segmentContext(fn, entryBB, *ctx, *valueTypeRegistry);
        producer->produce(segmentContext, next);
        segmentContext.builder.CreateRetVoid();

        if (producer->isComputeBound()) {
            computeRestartRoots->push_back(fn);
        } else {
            ioRestartRoots->push_back(fn);
        }
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

        if (chain.nodes.front().isBreaker) {
            spawnBlockingRoot(operatorId, "0", chain.nodes.front().asProducer, rest);
        }

        afterEntryCache.emplace(operatorId, rest);
        return rest;
    }

    segment::SegmentDataConsumer PlanConstruction::buildContinue(const OperatorId operatorId,
                                                                 const segment::Segment& chain,
                                                                 const std::size_t index) {
        if (index >= chain.nodes.size()) {
            const auto exit = buildExit(operatorId);
            return exit.empty() ? noopConsumer.get() : exit.front();
        }

        const auto& node = chain.nodes.at(index);
        segment::SegmentDataConsumers rest;

        if (index + 1 >= chain.nodes.size()) {
            rest = buildExit(operatorId);
        } else {
            rest = {buildContinue(operatorId, chain, index + 1)};
        }

        if (node.isBreaker) {
            spawnBlockingRoot(operatorId, std::to_string(index), node.asProducer, rest);
            return noopConsumer;
        }

        flow::IConsumer* consumerNode = node.asConsumer;
        return [consumerNode, rest](segment::SegmentContext& sCtx, const data::DataPackage& tuple) {
            consumerNode->consume(sCtx, 0, tuple, rest);
        };
    }

    segment::SegmentDataConsumer
    PlanConstruction::compileAsFunction(const OperatorId operatorId,
                                        const shared::identifiers::PortId portId,
                                        const port::Port& portSchema,
                                        const segment::SegmentDataConsumer& inner) const {
        auto* i64Ty = llvm::Type::getInt64Ty(*ctx);

        std::vector<llvm::Type*> paramTypes;
        paramTypes.reserve(portSchema.size() + 1);
        paramTypes.push_back(i64Ty);
        for (const auto& descriptor : portSchema) {
            paramTypes.push_back(descriptor.llvmType);
        }

        auto* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx), paramTypes, false);
        auto* fn = llvm::Function::Create(fnType,
                                          llvm::Function::InternalLinkage,
                                          "ody_fanin_exit_" + std::to_string(operatorId) + "_" +
                                              std::to_string(portId),
                                          module);
        auto* entryBB = llvm::BasicBlock::Create(*ctx, "fanInEntry", fn);

        segment::SegmentContext segmentContext(fn, entryBB, *ctx, *valueTypeRegistry);

        data::DataPackage reconstructed;
        reconstructed.timestamp = fn->getArg(0);
        reconstructed.fields.reserve(portSchema.size());
        for (std::size_t i = 0; i < portSchema.size(); ++i) {
            reconstructed.fields.push_back(fn->getArg(i + 1));
        }

        inner(segmentContext, reconstructed);

        segmentContext.builder.CreateRetVoid();

        return [fn](segment::SegmentContext& sCtx, const data::DataPackage& tuple) {
            std::vector<llvm::Value*> args;
            args.reserve(tuple.fields.size() + 1);
            args.push_back(tuple.timestamp);
            args.insert(args.end(), tuple.fields.begin(), tuple.fields.end());
            sCtx.builder.CreateCall(fn, args);
        };
    }
}  // namespace sierra::odyplan::v3::core::physical::plan::detail