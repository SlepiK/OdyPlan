#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_DETAIL_PLAN_CONSTRUCTION_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_DETAIL_PLAN_CONSTRUCTION_HPP

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <core/physical/operators/operator.h>
#include <core/physical/segment/segment.h>
#include <core/physical/segment/segment_data_consumer.h>
#include <core/physical/state/state_registry.h>
#include <core/shared/operators/operator_port_reference.h>
#include <core/shared/operators/operator_port_reference_connection.h>
#include <memory>
#include <vector>

namespace sierra::odyplan::v3::core::physical::plan::detail {
    struct PlanConstruction {
        using OperatorId = shared::identifiers::OperatorId;
        using OperatorPortReference = shared::operators::OperatorPortReference;
        using OperatorPortReferenceConnection = shared::operators::OperatorPortReferenceConnection;
        using IOperator = operators::IOperator;

        PlanConstruction(
        std::reference_wrapper<const std::unordered_map<OperatorId, segment::Segment>> chainsIn,
        std::reference_wrapper<const std::unordered_map<
            OperatorId, std::vector<OperatorPortReferenceConnection>>> outgoingEdgesIn,
        std::reference_wrapper<const std::unordered_map<OperatorId, std::size_t>> inDegreeIn,
        segment::SegmentDataConsumer noopConsumerIn,
        llvm::Module* moduleIn,
        llvm::LLVMContext* ctxIn,
        std::vector<llvm::Function*>* functionsIn,
        const types::CodegenTypeRegistry* codegenTypeRegistryIn,
        std::vector<llvm::Function*>* ioRestartRootsIn,
        std::vector<llvm::Function*>* computeRestartRootsIn,
        state::StateRegistry* stateRegistryIn)
        : chains(chainsIn),
          outgoingEdges(outgoingEdgesIn),
          inDegree(inDegreeIn),
          noopConsumer(std::move(noopConsumerIn)),
          module(moduleIn),
          ctx(ctxIn),
          functions(functionsIn),
          codegenTypeRegistry(codegenTypeRegistryIn),
          ioRestartRoots(ioRestartRootsIn),
          computeRestartRoots(computeRestartRootsIn),
          stateRegistry(stateRegistryIn) {}

        std::reference_wrapper<const std::unordered_map<OperatorId, segment::Segment>> chains;
        std::reference_wrapper<
            const std::unordered_map<OperatorId, std::vector<OperatorPortReferenceConnection>>>
            outgoingEdges;
        std::reference_wrapper<const std::unordered_map<OperatorId, std::size_t>> inDegree;
        segment::SegmentDataConsumer noopConsumer;
        llvm::Module* module;
        llvm::LLVMContext* ctx;
        std::vector<llvm::Function*>* functions;
        const types::CodegenTypeRegistry* codegenTypeRegistry;

        std::vector<llvm::Function*>* ioRestartRoots;
        std::vector<llvm::Function*>* computeRestartRoots;
        state::StateRegistry* stateRegistry;

        std::unordered_map<std::string, state::StateHandle> computeQueueCache;

        segment::SegmentDataConsumers buildExit(OperatorId operatorId);

        void spawnBlockingRoot(OperatorId operatorId,
                               const std::string& suffix,
                               flow::IProducer* producer,
                               const segment::SegmentDataConsumers& next) const;

        segment::SegmentDataConsumer buildEnter(const OperatorPortReferenceConnection& edge);

        segment::SegmentDataConsumers afterEntry(OperatorId operatorId);

        segment::SegmentDataConsumer
        buildContinue(OperatorId operatorId, const segment::Segment& chain, std::size_t index);

        segment::SegmentDataConsumer
        compileAsFunction(OperatorId operatorId,
                          shared::identifiers::PortId portId,
                          const port::Port& portSchema,
                          const segment::SegmentDataConsumer& inner);

    private:
        std::unordered_map<OperatorId, segment::SegmentDataConsumers> exitCache;
        std::unordered_map<OperatorId, segment::SegmentDataConsumers> afterEntryCache;

        state::StateHandle getOrCreateComputeQueueState(OperatorId operatorId, shared::identifiers::PortId portId);

        segment::SegmentDataConsumer wrapAsComputeContinuation(const std::string& uniqueKey, const port::Port& portSchema,
            const segment::SegmentDataConsumer& inner);

    };
}  // namespace sierra::odyplan::v3::core::physical::plan::detail

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_DETAIL_PLAN_CONSTRUCTION_HPP