#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#include <cstdint>
#include <iostream>
#include <syncstream>

#include <core/shared/types/value_type_id.h>
#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::runtime {
    struct PrintField {
        const void* value;
        const char* name;
        core::shared::types::ValueTypeId typeId;
    };

    struct PrintContext {
        const PrintField* fields;
        std::size_t fieldCount;
        core::shared::types::Timestamp startTs;
        core::shared::types::Timestamp endTs;
    };

    extern "C" {
    void rtPrintDataPackage(const PrintContext* ctx);

    void rtOdyPrintNewline();

    void rtOdyPrintPunctuation(core::shared::types::Timestamp watermark, bool isFinal);
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
