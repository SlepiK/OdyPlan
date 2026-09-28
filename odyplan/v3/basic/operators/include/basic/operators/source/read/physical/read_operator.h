#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_PHYSICAL_READ_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_PHYSICAL_READ_OPERATOR_HPP

#include <llvm/IR/Module.h>

#include <nlohmann/json.hpp>

#include <basic/io/io_format.h>
#include <basic/operators/source/read/logical/read_operator.h>
#include <core/logical/port/port.h>
#include <core/physical/operators/source/typed_source_operator.h>
#include <basic/operators/source/read/details/read_operator_options.h>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::source::read::physical {
    class ReadOperator : public core::physical::operators::source::TypedSourceOperator {
    public:
        ReadOperator(core::physical::port::Port outputPort,
                     nlohmann::json ioConfig,
                     io::IOFormatType ioFormat,
                     core::logical::port::Port dataSchema,
                     details::PReadOperatorOptions options);

        void produce(core::physical::segment::SegmentContext& segmentCtx,
                     const core::physical::segment::SegmentDataConsumers& consumers) override;

        void declareState(core::physical::state::StateRegistry& stateRegistry,
                          llvm::Module& module,
                          llvm::LLVMContext& context) override;

    private:
        io::IOFormatType mIOFormat;
        core::logical::port::Port mDataSchema;
        details::PReadOperatorOptions mOptions;
        core::physical::state::StateHandle mState;
        nlohmann::json mIOConfig;

        llvm::GlobalVariable* mWatermarkState = nullptr;

        [[nodiscard]] llvm::Value*
        loadStatePtr(core::physical::segment::SegmentContext& segmentCtx) const;

        static llvm::Value* codegenField(core::physical::segment::SegmentContext& segmentCtx,
                                         llvm::Value* stringValue,
                                         core::shared::types::ValueTypeId typeId);
    };
}  // namespace sierra::odyplan::v3::basic::operators::source::read::physical

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SOURCE_READ_PHYSICAL_READ_OPERATOR_HPP
