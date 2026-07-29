#include "core/builder/plan_builder.h"

namespace sierra::odyplan::v2::core::builder {
    void PlanBuilder::registerOperatorTransformation(common::identifier::TypeId lId,
        physical::transformation::OperatorTransformationFactory factory) {
        mFactories.emplace(lId, std::move(factory));
    }

    [[nodiscard]] physical::plan::Plan PlanBuilder::build(const logical::plan::Plan& lPlan, llvm::LLVMContext& ctx) {
        auto pPlan = physical::plan::Plan(*mRegistry);

        for (const auto lOprId : lPlan.topologicalOrder()) {
            const logical::opr::IOperator& lOpr = lPlan.op(lOprId);

            lOpr.validate();

            auto inputPorts = resolvePorts(lOpr.getInputPorts(), ctx);
            auto outputPorts = resolvePorts(lOpr.getOutputPorts(), ctx);

            auto it = mFactories.find(lOpr.getOperatorTypeId());
            if (it == mFactories.end()) {
                throw std::invalid_argument("PlanBuilder::build: no transformation registered for typeid " + std::to_string(lOpr.getOperatorTypeId()));
            }

            auto [oprs] = it->second(lOpr, inputPorts, outputPorts);
            if (oprs.empty()) {
                throw std::invalid_argument("PlanBuilder::build: transformation produced no physical operator");
            }

            for (auto& opr : oprs) {
                opr->setId(lOprId);
            }

            const auto inEdgeCount = lPlan.getInEdgesFromOperator(lOprId).size();
            if (inEdgeCount > 1 && oprs.size() > 1) {
                throw std::logic_error(
                    "PlanBuilder::build: a logical operator with multiple input can currently only transformed to a single physical operator.");
            }

            physical::chain::Chain chain;
            chain.exitSchemas = outputPorts;
            chain.nodes.reserve(oprs.size());

            for (std::size_t i = 0; i < oprs.size(); ++i) {
                physical::chain::ChainNode node;

                switch (oprs.at(i)->getRole()) {
                    case physical::opr::OperatorRole::SOURCE: {
                        if (i != 0) {
                            throw std::logic_error("PlanBuilder::build: source_operator needs to be the first element of its chain");
                        }

                        node.asProducer = static_cast<physical::opr::TypedSourceOperator*>(oprs.at(i).get());
                        break;
                    }
                    case physical::opr::OperatorRole::PIPE: {
                        if (oprs.at(i)->isSegmentBreaker()) {
                            auto* blockingPipe = static_cast<physical::opr::TypedBlockingPipeOperator*>(oprs.at(i).get());
                            node.asConsumer = blockingPipe;
                            node.asProducer = blockingPipe;
                            node.isBreaker = true;
                        } else {
                            node.asConsumer = static_cast<physical::opr::TypedPipeOperator*>(oprs.at(i).get());
                        }
                        break;
                    }
                    case physical::opr::OperatorRole::SINK: {
                        if ( i+1 != oprs.size()) {
                            throw std::logic_error("PlanBuilder::build: a sink_operator must be the last element of its chain");
                        }
                        node.asConsumer = static_cast<physical::opr::TypedSinkOperator*>(oprs.at(i).get());
                        break;
                    }
                }

                chain.nodes.push_back(node);
            }

            pPlan.addChain(lOprId, chain);

            std::vector<physical::edge::Edge> outEdges;
            for (const auto& edge : lPlan.getOutEdgesFromOperator(lOprId)) {
                outEdges.push_back(physical::edge::Edge{
                    .fromOperatorOutputPortId = edge.fromOperator.portId,
                    .toOperatorId = edge.toOperator.operatorId,
                    .toOperatorInputPortId = edge.toOperator.portId
                });
            }

            pPlan.addOutgoingEdge(lOprId, std::move(outEdges));

            for (auto& opr : oprs) {
                pPlan.addOperator(std::move(opr));
            }
        }

        return pPlan;
    }

    [[nodiscard]] physical::port::Ports PlanBuilder::resolvePorts(const logical::port::Ports& lPorts, llvm::LLVMContext& ctx) const {
        physical::port::Ports ret;
        ret.reserve(lPorts.size());

        for (const auto& lPort : lPorts) {
            physical::port::Port pPort;
            pPort.reserve(lPort.size());

            for (const auto& lPortDescriptor : lPort) {
                pPort.push_back(physical::port::PortDescriptor{
                    .typeId=lPortDescriptor.typeId,
                    .llvmType=mRegistry->resolveType(lPortDescriptor.typeId,ctx)
                });
            }

            ret.push_back(std::move(pPort));
        }

        return ret;
    }
}