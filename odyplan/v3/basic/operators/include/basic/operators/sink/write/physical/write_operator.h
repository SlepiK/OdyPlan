#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_PHYSICAL_WRITE_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_PHYSICAL_WRITE_OPERATOR_HPP

#include "basic/io/io_format.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <nlohmann/json.hpp>

#include <basic/io/io_sink.h>
#include <core/logical/port/port.h>
#include <core/physical/operators/sink/typed_sink_operator.h>
#include <core/physical/port/port.h>
#include <core/shared/types/value_type_id.h>
#include <memory>
#include <utility>

namespace sierra::odyplan::v3::basic::operators::sink::write::physical {
    class WriteOperator : public core::physical::operators::sink::TypedSinkOperator {
    public:
        WriteOperator(const core::physical::port::Port& inputPort,
                      nlohmann::json ioConfig,
                      io::IOFormatType ioFormat,
                      core::logical::port::Port dataSchema);

        void consume(core::physical::segment::SegmentContext& segmentCtx,
                     core::shared::identifiers::PortId /*inputPort*/,
                     const core::physical::data::DataPackage& tuple,
                     const core::physical::segment::SegmentDataConsumers& /*consumers*/) override;

        void declareState(core::physical::state::StateRegistry& stateRegistry,
                          llvm::Module& module,
                          llvm::LLVMContext& context) override;

        void end(core::physical::segment::SegmentContext& segmentCtx,
                 core::shared::identifiers::PortId inputPort,
                 const core::physical::segment::SegmentDataConsumers& consumers) override;

    private:
        io::IOFormatType mIOFormat;
        core::logical::port::Port mDataSchema;
        core::physical::state::StateHandle mState;
        nlohmann::json mIOConfig;

        [[nodiscard]] llvm::Value*
        loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const;

        void codegenField(core::physical::segment::SegmentContext& segmentCtx,
                                 llvm::Value* state,
                                 llvm::Value* fieldName,
                                 llvm::Value* fieldValue,
                                 core::shared::types::ValueTypeId typeId);
    };
}  // namespace sierra::odyplan::v3::basic::operators::sink::write::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_WRITE_PHYSICAL_WRITE_OPERATOR_HPP
