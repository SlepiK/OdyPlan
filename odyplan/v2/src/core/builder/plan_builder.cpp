#include "core/builder/plan_builder.h"

namespace sierra::odyplan::v2::core::builder {
    void PlanBuilder::registerOperatorTransformation(common::type::TypeId lId,
        physical::transformation::OperatorTransformationFactory factory) {
        mFactories.emplace(lId, std::move(factory));
    }

    [[nodiscard]] physical::plan::Plan PlanBuilder::build(const logical::plan::Plan& lPlan, llvm::LLVMContext& ctx) {
        physical::plan::Plan pPlan;
        physical::segment::Segment currentSegment;

        for (auto lOprId : lPlan.topologicalOrder()) {
            const logical::opr::IOperator& lOpr = lPlan.op(lOprId);

            auto inputPorts = resolvePorts(lOpr.getInputPorts(), ctx);
            auto outputPorts = resolvePorts(lOpr.getOutputPorts(), ctx);

            auto it = mFactories.find(lOpr.getTypeId());
            if (it == mFactories.end()) {
                throw std::invalid_argument("PlanBuilder::build: no transformation registered for typeid " + std::to_string(lOpr.getTypeId()));
            }

            physical::transformation::OperatorTransformation transform = it->second(lOpr, inputPorts, outputPorts);

            for (auto& op : transform.oprs) {
                switch (op->getRole()) {
                    case physical::opr::OperatorRole::SOURCE: {
                        auto* source = static_cast<physical::opr::TypedSourceOperator*>(op.get());
                        currentSegment.start = source;
                        currentSegment.type = physical::segment::SegmentType::NORMAL;
                        break;
                    }
                    case physical::opr::OperatorRole::PIPE: {
                        auto* pipe = static_cast<physical::opr::TypedPipeOperator*>(op.get());
                        if (op->isSegmentBreaker()) {
                            auto* blockingPipe = static_cast<physical::opr::TypedBlockingPipeOperator*>(op.get());
                            currentSegment.blockingTarget = blockingPipe;
                            currentSegment.type = physical::segment::SegmentType::BLOCKING;
                            pPlan.addSegment(std::move(currentSegment));
                            currentSegment = physical::segment::Segment{};
                            currentSegment.type = physical::segment::SegmentType::NORMAL;
                            currentSegment.start = blockingPipe;
                        } else {
                            currentSegment.mid.push_back(pipe);
                        }
                        break;
                    }
                    case physical::opr::OperatorRole::SINK: {
                        auto* sink = static_cast<physical::opr::TypedSinkOperator*>(op.get());
                        currentSegment.type = physical::segment::SegmentType::NORMAL;
                        currentSegment.end = sink;
                        pPlan.addSegment(std::move(currentSegment));
                        currentSegment = physical::segment::Segment{};
                        break;
                    }
                }

                pPlan.addOperator(std::move(op));
            }
        }

        return pPlan;
    }

    [[nodiscard]] physical::port::Ports PlanBuilder::resolvePorts(const logical::port::Ports& lPorts, llvm::LLVMContext& ctx) {
        physical::port::Ports ret;
        ret.reserve(lPorts.size());

        for (const auto& lPort : lPorts) {
            physical::port::Port pPort;
            pPort.reserve(lPort.size());

            for (const auto& lPortDescriptor : lPort) {
                pPort.push_back(physical::port::PortDescriptor{
                    lPortDescriptor.typeId,
                    mRegistry.resolveType(lPortDescriptor.typeId,ctx)
                });
            }

            ret.push_back(std::move(pPort));
        }

        return ret;
    }
}