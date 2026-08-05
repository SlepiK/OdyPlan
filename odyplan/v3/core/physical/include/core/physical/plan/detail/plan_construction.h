#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_DETAIL_PLAN_CONSTRUCTION_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_PLAN_DETAIL_PLAN_CONSTRUCTION_HPP

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>

#include <memory>
#include <vector>

#include <core/physical/operators/operator.h>
#include <core/physical/segment/segment.h>
#include <core/physical/segment/segment_data_consumer.h>
#include <core/physical/types/value_type_registry.h>
#include <core/shared/operators/operator_port_reference.h>
#include <core/shared/operators/operator_port_reference_connection.h>

namespace sierra::odyplan::v3::core::physical::plan::detail {
    struct PlanConstruction {
        using OperatorId = shared::identifiers::OperatorId;
        using OperatorPortReference = shared::operators::OperatorPortReference;
        using OperatorPortReferenceConnection = shared::operators::OperatorPortReferenceConnection;
        using IOperator = operators::IOperator;

        const std::unordered_map<OperatorId, segment::Segment>& chains;
        const std::unordered_map<OperatorId, std::vector<OperatorPortReferenceConnection>>& outgoingEdges;
        const std::unordered_map<OperatorId, std::size_t>& inDegree;
        const segment::SegmentDataConsumer& noopConsumer;
        llvm::Module& module;
        llvm::LLVMContext& ctx;
        std::vector<llvm::Function*>& functions;
        const types::ValueTypeRegistry* valueTypeRegistry;

        std::vector<llvm::Function*>& ioRestartRoots;
        std::vector<llvm::Function*>& computeRestartRoots;

        std::unordered_map<OperatorId, segment::SegmentDataConsumers> exitCache;
        std::unordered_map<OperatorId, segment::SegmentDataConsumers> afterEntryCache;

        segment::SegmentDataConsumers buildExit(const OperatorId operatorId);

        void spawnBlockingRoot(const OperatorId operatorId, const std::string& suffix,
                flow::IProducer* producer, const segment::SegmentDataConsumers& next);

        segment::SegmentDataConsumer buildEnter(const OperatorPortReferenceConnection& edge);

        segment::SegmentDataConsumers afterEntry(const OperatorId operatorId);

        segment::SegmentDataConsumer buildContinue(const OperatorId operatorId, const segment::Segment& chain, const std::size_t index);

        segment::SegmentDataConsumer compileAsFunction(const OperatorId operatorId, const shared::identifiers::PortId portId, const port::Port& portSchema, const segment::SegmentDataConsumer& inner);
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_LOGICAL_PLAN_DETAIL_PLAN_CONSTRUCTION_HPP