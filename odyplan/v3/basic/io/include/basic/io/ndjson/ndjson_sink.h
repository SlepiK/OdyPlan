#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SINK_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SINK_HPP

#include <nlohmann/json.hpp>

#include <basic/io/io_sink.h>
#include <basic/io/io_sink_helper.h>
#include <fstream>
#include <string>
#include <utility>

namespace sierra::odyplan::v3::basic::io::ndjson {
    class NDJSONSink : public IIOSink {
    public:
        explicit NDJSONSink(std::string path) : mPath(std::move(path)) {
            mFileHandle.open(mPath, std::ios::out | std::ios::trunc);

            if (!mFileHandle.is_open()) {
                throw std::runtime_error("NDJSONSink: cannot open file for writing");
            }
        }

        ~NDJSONSink() override {
            try {
                if (mInUse) {
                    NDJSONSink::flush();
                }
            } catch (...) {
                std::fputs("NDJSONSink: flush failed in destructor (unknown exception)\n", stderr);
            }
        }

        NDJSONSink(NDJSONSink const&) = delete;

        NDJSONSink& operator=(NDJSONSink const&) = delete;

        NDJSONSink(NDJSONSink&&) = delete;

        NDJSONSink& operator=(NDJSONSink&&) = delete;

        void begin() override {
            mCurrent = nlohmann::json::object();
            mInUse = true;
        }

        void write(const char* key,
                   const core::shared::types::ValueTypeId typeId,
                   const void* rawData) override {
            detail::writeToJson(mCurrent, key, typeId, rawData);
            mInUse = true;
        }

        void end() override {
            mFileHandle << mCurrent.dump() << '\n';

            if (!mFileHandle) {
                throw std::runtime_error("NDJSONSink: failed to write record");
            }

            mCurrent.clear();
            mInUse = true;
        }

        void flush() override {
            flushImpl(false);
        }

    private:
        void flushImpl(const bool force) {
            if (!mCurrent.empty()) {
                end();
            }

            if (!force && !mInUse) {
                return;
            }

            mFileHandle.flush();
            mInUse = false;
        }

        std::string mPath;
        std::ofstream mFileHandle;
        nlohmann::ordered_json mCurrent;
        bool mInUse = false;
    };
}  // namespace sierra::odyplan::v3::basic::io::ndjson

#endif  // SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SINK_HPP
