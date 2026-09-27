#include <basic/operators/sink/print/runtime/print_runtime.h>
#include <nlohmann/json.hpp>
#include <nlohmann/detail/meta/std_fs.hpp>

#include "basic/io/io_sink_helper.h"
#include "core/physical/time/time_point.h"
#include "core/shared/types/value_type_id.h"

namespace sierra::odyplan::v3::basic::operators::sink::print::runtime {
    namespace {
        thread_local std::string threadPrintBuffer;
    }

    extern "C" {

    void rtPrintDataPackage(const PrintContext* ctx) {
        if (ctx == nullptr || ctx->fields == nullptr) {
            return;
        }

        nlohmann::ordered_json json;

        for (std::size_t i = 0; i < ctx->fieldCount; ++i) {
            const auto& field = ctx->fields[i];

            if (field.name != nullptr && field.value != nullptr) {
                io::detail::writeToJson(json, field.name, field.typeId, field.value);
            }
        }

        threadPrintBuffer += json.dump();
        threadPrintBuffer += ";";
        threadPrintBuffer += std::to_string(ctx->startTs);
        threadPrintBuffer += "|";
        if (ctx->endTs == core::physical::time::INFINITY_SENTINEL) {
            threadPrintBuffer += "oo";
        } else {
            threadPrintBuffer += std::to_string(ctx->endTs);
        }
        rtOdyPrintNewline();
    }

    void rtOdyPrintNewline() {
        threadPrintBuffer += '\n';
        std::osyncstream(std::cout) << threadPrintBuffer;
        threadPrintBuffer.clear();
    }

    void rtOdyPrintPunctuation(const core::shared::types::Timestamp watermark, const bool isFinal) {
        threadPrintBuffer += "[PUNCTUATION ";
        if (isFinal) {
            threadPrintBuffer += "FINAL]";
        } else {
            threadPrintBuffer += "watermark=";
            threadPrintBuffer += std::to_string(watermark);
            threadPrintBuffer += ']';
        }
        rtOdyPrintNewline();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime