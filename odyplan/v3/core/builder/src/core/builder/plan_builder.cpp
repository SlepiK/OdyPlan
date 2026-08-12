#include <core/builder/plan_builder.h>

namespace sierra::odyplan::v3::core::builder {
    PlanBuilder::PlanBuilder(const CodegenTypeRegistry& codengenTypeRegistry) : mCodegenTypeRegistry(&codengenTypeRegistry) {}

    void PlanBuilder::registerOperatorTransformation(
        ValueTypeId lId, transformation::OperatorTransformationFactory factory) {
        mFactories.emplace(lId, std::move(factory));
    }

    physical::plan::Plan PlanBuilder::build(const logical::plan::Plan& lPlan,
                                            llvm::LLVMContext& ctx) {
        auto pPlan = physical::plan::Plan(*mCodegenTypeRegistry);

        for (const auto lOprId : lPlan.topologicalOrder()) {
            const logical::operators::IOperator& lOpr = lPlan.op(lOprId);

            lOpr.validate();

            auto inputPorts = resolvePorts(lOpr.getInputPorts(), ctx);
            auto outputPorts = resolvePorts(lOpr.getOutputPorts(), ctx);

            auto it = mFactories.find(lOpr.getOperatorTypeId());
            if (it == mFactories.end()) {
                throw std::invalid_argument(
                    "PlanBuilder::build: no transformation registered for typeid " +
                    std::to_string(lOpr.getOperatorTypeId()));
            }

            auto [oprs] = it->second(lOpr, inputPorts, outputPorts);
            if (oprs.empty()) {
                throw std::invalid_argument(
                    "PlanBuilder::build: transformation produced no physical operator");
            }

            for (auto& opr : oprs) {
                opr->setId(lOprId);
            }

            const auto inEdgeCount = lPlan.getInEdgesFromOperator(lOprId).size();
            if (inEdgeCount > 1 && oprs.size() > 1) {
                throw std::logic_error(
                    "PlanBuilder::build: a logical operator with multiple input can currently only "
                    "transformed to a single physical operator.");
            }

            physical::segment::Segment chain;
            chain.exitSchemas = outputPorts;
            chain.nodes.reserve(oprs.size());

            for (std::size_t i = 0; i < oprs.size(); ++i) {
                physical::segment::SegmentNode node;

                switch (oprs.at(i)->getRole()) {
                    case physical::operators::OperatorRole::SOURCE: {
                        if (i != 0) {
                            throw std::logic_error("PlanBuilder::build: source_operator needs to "
                                                   "be the first element of its chain");
                        }
                        // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                        node.asProducer =
                            static_cast<physical::operators::source::TypedSourceOperator*>(
                                oprs.at(i).get());
                        // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                        break;
                    }
                    case physical::operators::OperatorRole::PIPE: {
                        if (oprs.at(i)->isSegmentBreaker()) {
                            // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                            auto* blockingPipe =
                                static_cast<physical::operators::pipe::TypedBlockingPipeOperator*>(
                                    oprs.at(i).get());
                            // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                            node.asConsumer = blockingPipe;
                            node.asProducer = blockingPipe;
                            node.isBreaker = true;
                        } else {
                            // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                            node.asConsumer =
                                static_cast<physical::operators::pipe::TypedPipeOperator*>(
                                    oprs.at(i).get());
                            // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                        }
                        break;
                    }
                    case physical::operators::OperatorRole::SINK: {
                        if (i + 1 != oprs.size()) {
                            throw std::logic_error("PlanBuilder::build: a sink_operator must be "
                                                   "the last element of its chain");
                        }
                        // NOLINTBEGIN(cppcoreguidelines-pro-type-static-cast-downcast)
                        node.asConsumer =
                            static_cast<physical::operators::sink::TypedSinkOperator*>(
                                oprs.at(i).get());
                        // NOLINTEND(cppcoreguidelines-pro-type-static-cast-downcast)
                        break;
                    }
                }

                chain.nodes.push_back(node);
            }

            pPlan.addChain(lOprId, chain);

            std::vector<shared::operators::OperatorPortReferenceConnection> outEdges;
            for (const auto& edge : lPlan.getOutEdgesFromOperator(lOprId)) {
                outEdges.push_back(shared::operators::OperatorPortReferenceConnection{
                    .fromOperatorPortReference = edge.fromOperatorPortReference,
                    .toOperatorPortReference = edge.toOperatorPortReference});
            }

            pPlan.addOutgoingEdge(lOprId, std::move(outEdges));

            for (auto& opr : oprs) {
                pPlan.addOperator(std::move(opr));
            }
        }

        return pPlan;
    }

    physical::port::Ports PlanBuilder::resolvePorts(const logical::port::Ports& lPorts,
                                                    llvm::LLVMContext& ctx) const {
        physical::port::Ports ret;
        ret.reserve(lPorts.size());

        for (const auto& lPort : lPorts) {
            physical::port::Port pPort;
            pPort.reserve(lPort.size());

            for (const auto& lPortDescriptor : lPort) {
                pPort.push_back(physical::port::PortDescription{
                    .valueTypeId = lPortDescriptor.valueTypeId,
                    .llvmType = mCodegenTypeRegistry->resolveType(lPortDescriptor.valueTypeId, ctx)});
            }

            ret.push_back(std::move(pPort));
        }

        return ret;
    }
}  // namespace sierra::odyplan::v3::core::builder