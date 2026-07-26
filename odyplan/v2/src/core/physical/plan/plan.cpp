#include "core/physical/plan/plan.h"

#include <ranges>
#include <set>

#include "core/physical/edge/edge.h"
#include "core/physical/segment/segment_context.h"

namespace sierra::odyplan::v2::core::physical::plan {
    namespace detail {

        struct Builder {
            const std::unordered_map<common::identifier::OperatorId, chain::Chain>& chains;
            const std::unordered_map<common::identifier::OperatorId, std::vector<edge::Edge>>& outgoingEdges;
            const std::unordered_map<common::identifier::OperatorId, std::size_t>& inDegree;
            const port::PortTupleConsumer& noopConsumer;
            llvm::Module& module;
            llvm::LLVMContext& ctx;
            std::vector<llvm::Function*>& functions;
            const common::type::TypeRegistry* typeRegistry;
            std::vector<llvm::Function*> blockingFunctionsRestart;

            std::unordered_map<common::identifier::OperatorId, port::PortTupleConsumers> exitCache;
            std::unordered_map<common::identifier::OperatorId, port::PortTupleConsumers> afterEntryCache;

            port::PortTupleConsumers buildExit(const common::identifier::OperatorId operatorId) {
                if (const auto it = exitCache.find(operatorId); it != exitCache.end()) {
                    return it->second;
                }

                const auto& chain = chains.at(operatorId);
                const auto edgeIt = outgoingEdges.find(operatorId);
                constexpr std::vector<edge::Edge> emptyEdges{};
                const auto& outEdges = edgeIt != outgoingEdges.end() ? edgeIt->second : emptyEdges;

                std::unordered_map<common::identifier::PortId, std::vector<edge::Edge>> byPort;
                for (const auto& edge : outEdges) {
                    byPort[edge.fromOperatorOutputPortId].push_back(edge);
                }

                port::PortTupleConsumers result(chain.exitSchemas.size(), noopConsumer);

                for (std::size_t portIndex = 0; portIndex < chain.exitSchemas.size(); ++portIndex) {
                    const auto found = byPort.find(portIndex);
                    if (found == byPort.end()) {
                        continue;
                    }

                    const auto& edgesForPort = found->second;

                    if (outEdges.size() == 1) {
                        result[portIndex] = buildEnter(edgesForPort.front());
                    } else {
                        port::PortTupleConsumers branches;
                        branches.reserve(outEdges.size());
                        for (const auto& edge : outEdges) {
                            branches.push_back(buildEnter(edge));
                        }
                        result[portIndex] = [branches](segment::SegmentContext& sCtx, const port::PortTuple& tuple) {
                            for (const auto& branch : branches) {
                                branch(sCtx, tuple);
                            }
                        };
                    }
                }

                exitCache.emplace(operatorId, result);
                return result;
            }

            port::PortTupleConsumer buildEnter(const edge::Edge& edge) {
                const auto id = edge.toOperatorId;
                const auto& chain = chains.at(id);
                const auto inputIndex = edge.toOperatorInputPortId;

                opr::IConsumer* entryOpr = chain.nodes.front().asConsumer;
                if (entryOpr == nullptr) {
                    throw std::logic_error("Plan::buildEnter: chain has no valid consumer entry point");
                }

                port::PortTupleConsumers tail = afterEntry(id);
                return [entryOpr, inputIndex, tail](segment::SegmentContext& sCtx, const port::PortTuple& tuple) {
                    entryOpr->consume(sCtx, inputIndex, tuple, tail);
                };
            }

            port::PortTupleConsumers afterEntry(const common::identifier::OperatorId operatorId) {
                if (const auto it = afterEntryCache.find(operatorId); it != afterEntryCache.end()) {
                    return it->second;
                }

                const auto& chain = chains.at(operatorId);

                port::PortTupleConsumers rest;

                if (chain.nodes.size() == 1) {
                    rest = buildExit(operatorId);
                } else {
                    rest = {buildContinue(operatorId, chain, 1)};
                }

                const auto degreeIt = inDegree.find(operatorId);
                const auto degree = degreeIt != inDegree.end() ? degreeIt->second : std::size_t{0};
                if (degree > 1) {
                    if (chain.nodes.size() > 1 && chain.exitSchemas.size() > 1) {
                        throw std::logic_error("Plan::codegen: Operation transform from a logical operator with > 1 Inputs to more then 1 physical operator is not supported!");
                    }

                    const auto& schemas = chain.nodes.size() == 1 ? chain.exitSchemas : port::Ports{chain.exitSchemas.empty() ? port::Port{} : chain.exitSchemas.front()};

                    port::PortTupleConsumers wrapped;
                    for (std::size_t i = 0; i < rest.size(); ++i) {
                        wrapped.push_back(compileAsFunction(operatorId, i, schemas.at(i), rest[i]));
                    }
                    rest = wrapped;
                }

                if (chain.nodes.front().isBreaker) {
                    spawnRoot(operatorId, "0", chain.nodes.front().asProducer, rest);
                }

                afterEntryCache.emplace(operatorId, rest);
                return rest;
            }

            port::PortTupleConsumer buildContinue(const common::identifier::OperatorId operatorId, const chain::Chain& chain, const std::size_t index) {
                if (index >= chain.nodes.size()) {
                    const auto exit = buildExit(operatorId);
                    return exit.empty() ? noopConsumer : exit.front();
                }

                const auto& node = chain.nodes[index];
                port::PortTupleConsumers rest;

                if (index + 1 >= chain.nodes.size()) {
                    rest = buildExit(operatorId);
                } else {
                    rest = {buildContinue(operatorId, chain, index + 1)};
                }

                if (node.isBreaker) {
                    spawnRoot(operatorId, std::to_string(index), node.asProducer, rest);
                    return noopConsumer;
                }

                opr::IConsumer* consumerNode = node.asConsumer;
                return [consumerNode, rest](segment::SegmentContext& sCtx, const port::PortTuple& tuple) {
                    consumerNode->consume(sCtx, 0, tuple, rest);
                };
            }

            void spawnRoot(const common::identifier::OperatorId operatorId, const std::string& suffix, opr::IProducer* producer, const port::PortTupleConsumers& rest) {
                auto* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
                auto* fn = llvm::Function::Create(fnType, llvm::Function::InternalLinkage, "ody_rblock_" +std::to_string(operatorId) + "_" + suffix, &module);
                auto* entryBB = llvm::BasicBlock::Create(ctx, "basicEntry", fn);

                segment::SegmentContext segmentContext(fn, entryBB, ctx, *typeRegistry);

                producer->produce(segmentContext, rest);
                segmentContext.builder.CreateRetVoid();
                functions.push_back(fn);
            }


            port::PortTupleConsumer compileAsFunction(const common::identifier::OperatorId operatorId, const common::identifier::PortId portId, const port::Port& portSchema, const port::PortTupleConsumer& inner) {
                std::vector<llvm::Type*> paramTypes;
                paramTypes.reserve(portSchema.size());
                for (const auto& descriptor : portSchema) {
                    paramTypes.push_back(descriptor.llvmType);
                }

                auto* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), paramTypes, false);
                auto* fn = llvm::Function::Create(fnType, llvm::Function::InternalLinkage, "ody_fanin_exit_" + std::to_string(operatorId) + "_" + std::to_string(portId), &module);
                auto* entryBB = llvm::BasicBlock::Create(ctx, "fanInEntry", fn);

                segment::SegmentContext segmentContext(fn, entryBB, ctx, *typeRegistry);

                port::PortTuple reconstructed;
                reconstructed.reserve(portSchema.size());
                for (std::size_t i = 0; i < portSchema.size(); ++i) {
                    reconstructed.push_back(fn->getArg(i));
                }

                inner(segmentContext, reconstructed);

                segmentContext.builder.CreateRetVoid();

                return [fn](segment::SegmentContext& sCtx, const port::PortTuple& tuple) {
                    sCtx.builder.CreateCall(fn, llvm::ArrayRef<llvm::Value*>(tuple));
                };
            }
        };

    }

    void Plan::addOperator(std::unique_ptr<opr::IOperator> opr) {
        if (!opr) {
            throw std::invalid_argument("Plan::addOperator: opr must not be nullptr!");
        }

        mOperators.push_back(std::move(opr));
    }

    void Plan::addChain(common::identifier::OperatorId logicalOperatorId, chain::Chain chain) {
        if (chain.nodes.empty()) {
            throw std::invalid_argument("Plan::addChain: chain must not be empty!");
        }

        const auto& front = chain.nodes.front();
        if (front.asProducer == nullptr && front.asConsumer == nullptr) {
            throw std::invalid_argument("Plan::addChain: first node in chain must have at least one valid entry point");
        }
        mOrder.push_back(logicalOperatorId);
        mChains.emplace(logicalOperatorId, std::move(chain));
    }

    void Plan::addOutgoingEdge(common::identifier::OperatorId logicalOperatorId, std::vector<edge::Edge> edges) {
        mOutgoingEdges.emplace(logicalOperatorId, edges);
    }

    void Plan::codegen(llvm::Module& module, llvm::LLVMContext& ctx) const {
        state::StateRegistry stateRegistry(&module, &ctx);
        for (const auto& opr : mOperators) {
            opr->declareState(stateRegistry, module, ctx);
        }
        stateRegistry.finish();

        std::vector<llvm::Function*> functions;
        std::vector<llvm::Function*> blockingFunctionsRestart;

        std::unordered_map<common::identifier::OperatorId, std::size_t> inDegreeCache;
        std::unordered_map<common::identifier::OperatorId, std::set<std::size_t>> incomingInputIndices;
        for (const auto& [fromOperatorId, edgeLists] : mOutgoingEdges) {
            for (const auto& edge : edgeLists) {
                ++inDegreeCache[edge.toOperatorId];
                incomingInputIndices[edge.toOperatorId].insert(edge.toOperatorInputPortId);
            }
        }

        port::PortTupleConsumer noopConsumer = [](segment::SegmentContext&, const port::PortTuple&) {};

        detail::Builder detailBuilder{mChains, mOutgoingEdges, inDegreeCache, noopConsumer, module, ctx, functions, mTypeRegistry, blockingFunctionsRestart};

        for (const auto& [id, chain] : mChains) {
            if (const bool isSource = chain.nodes.front().asConsumer == nullptr) {
                auto* fnType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
                auto* fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, "ody_rsource_" + std::to_string(id), &module);
                auto* entryBB = llvm::BasicBlock::Create(ctx, "basicEntry", fn);
                segment::SegmentContext segmentContext(fn, entryBB, ctx, *mTypeRegistry);

                chain.nodes.front().asProducer->produce(segmentContext, detailBuilder.afterEntry(id));
                segmentContext.builder.CreateRetVoid();
                functions.push_back(fn);
            }
        }

        auto* endFnType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* endFn = llvm::Function::Create(endFnType, llvm::Function::ExternalLinkage, "ody_end_phase", &module);
        auto* endEntryBB = llvm::BasicBlock::Create(ctx, "basicEntry", endFn);

        segment::SegmentContext segmentContext(endFn, endEntryBB, ctx, *mTypeRegistry);

        for (const auto id : mOrder) {
            const auto& chain = mChains.at(id);
            opr::IConsumer* entryOpr = chain.nodes.front().asConsumer;
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

        std::vector<llvm::Function*> allRoots = functions;
        for (auto* fn : blockingFunctionsRestart) {
            allRoots.push_back(fn);
        }

        codegenMain(module, ctx, allRoots, endFn);
    }

    void Plan::codegenMain(llvm::Module& module, llvm::LLVMContext& ctx, const std::vector<llvm::Function*>& functions, llvm::Function* endPhase) {
        auto* functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* mainFn = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "ody_main", &module);

        auto* basicBlock = llvm::BasicBlock::Create(ctx, "basicEntry", mainFn);

        llvm::IRBuilder builder(basicBlock);

        auto* i64Ty = llvm::Type::getInt64Ty(ctx);
        auto* i8Ty = llvm::Type::getInt8Ty(ctx);
        auto* ptrTy = llvm::PointerType::getUnqual(ctx);
        auto spawnFn = module.getOrInsertFunction("rtTaskSpawn", llvm::FunctionType::get(i64Ty, {ptrTy, i8Ty}, false));
        auto joinFn = module.getOrInsertFunction("rtTaskJoin", llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {i64Ty}, false));

        constexpr std::uint8_t ioBound = 0;
        std::vector<llvm::Value*> handles;

        for (auto* function : functions) {
            handles.push_back(builder.CreateCall(spawnFn, {function, llvm::ConstantInt::get(i8Ty, ioBound)}));
            //builder.CreateCall(function, {});
        }

        for (auto* handle : handles) {
            builder.CreateCall(joinFn, {handle});
        }

        if (endPhase != nullptr) {
            builder.CreateCall(endPhase);
        }

        const auto endFn = module.getOrInsertFunction("rtEnd", llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false));
        builder.CreateCall(endFn, {});

        builder.CreateRetVoid();
    }
}  // namespace sierra::odyplan::v2::core::physical::plan