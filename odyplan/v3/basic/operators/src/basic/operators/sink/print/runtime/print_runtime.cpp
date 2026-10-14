#include <basic/operators/sink/print/runtime/print_runtime.h>
#include <nlohmann/json.hpp>
#include <nlohmann/detail/meta/std_fs.hpp>

#include "basic/io/io_sink_helper.h"
#include "core/physical/time/time_point.h"
#include "core/shared/types/value_type_id.h"

namespace sierra::odyplan::v3::basic::operators::sink::print::runtime {
    /*namespace {
        thread_local std::string threadPrintBuffer;
    }*/

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
        std::string bfr;
        bfr += json.dump();
        bfr += ";";
        bfr += std::to_string(ctx->startTs);
        bfr += "|";
        if (ctx->endTs == core::shared::types::INFINITY_TIMESTAMP_SENTINEL) {
            bfr += "oo";
        } else {
            bfr += std::to_string(ctx->endTs);
        }
        rtOdyPrintNewline(bfr);
    }

    void rtOdyPrintNewline(const std::string& bfr) {
        std::osyncstream(std::cout) << bfr << '\n';
        //threadPrintBuffer.clear();
    }

    void rtOdyPrintPunctuation(const core::shared::types::Timestamp watermark, const bool isFinal) {
        std::string bfr;
        bfr += "[PUNCTUATION ";
        if (isFinal) {
            bfr += "FINAL]";
        } else {
            bfr += "watermark=";
            bfr += std::to_string(watermark);
            bfr += ']';
        }
        rtOdyPrintNewline(bfr);
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::runtime