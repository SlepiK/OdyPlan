#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
#include <cstdint>
#include <iostream>
#include <syncstream>

#include "core/shared/types/value_type_id.h"

namespace sierra::odyplan::v3::basic::operators::sink::print::runtime {
    struct PrintField {
        const void* value;
        const char* name;
        core::shared::types::ValueTypeId typeId;
    };

    struct PrintContext {
        const PrintField* fields;
        std::size_t fieldCount;
        std::int64_t startTs;
        std::int64_t endTs;
    };

    extern "C" {
    void rtPrintDataPackage(const PrintContext* ctx);

    void _rtOdyPrintNewline();

    void rtOdyPrintPunctuation(std::int64_t watermark, bool isFinal);
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_SINK_PRINT_RUNTIME_PRINT_RUNTIME_HPP
