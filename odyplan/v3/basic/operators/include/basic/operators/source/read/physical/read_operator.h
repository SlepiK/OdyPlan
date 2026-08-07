#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_PHYSICAL_READ_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_PHYSICAL_READ_OPERATOR_HPP

#include <llvm/IR/Module.h>
#include <nlohmann/json.hpp>

#include <basic/io/io_source.h>
#include <core/logical/port/port.h>
#include <core/physical/operators/source/typed_source_operator.h>
#include <core/physical/types/value_type_registry.h>

namespace sierra::odyplan::v3::basic::operators::source::read::physical {
    class ReadOperator : public core::physical::operators::source::TypedSourceOperator {
    public:
        ReadOperator(core::physical::port::Port outputPort, io::IOSourceId ioSourceId,
                     core::logical::port::Port dataSchema, std::optional<std::size_t> timestampFieldIndex = std::nullopt,
                     std::chrono::milliseconds rowDelay = std::chrono::milliseconds(0));

        void produce(core::physical::segment::SegmentContext &segmentCtx,
                     const core::physical::segment::SegmentDataConsumers &consumers) override;

    private:
        io::IOSourceId mIOSourceId;
        core::logical::port::Port mDataSchema;
        std::optional<std::size_t> mTimestampFieldIndex;
        std::chrono::milliseconds mRowDelay;

        static llvm::Value *codegenField(core::physical::segment::SegmentContext &segmentCtx,
            llvm::Value *stringValue,
            core::shared::types::ValueTypeId typeId);
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_PHYSICAL_READ_OPERATOR_HPP