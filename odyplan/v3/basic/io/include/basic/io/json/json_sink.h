#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_JSON_JSON_SINK_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_JSON_JSON_SINK_HPP

#include <nlohmann/json.hpp>

#include <basic/io/io_sink.h>
#include <basic/io/io_sink_helper.h>
#include <fstream>
#include <string>
#include <utility>

namespace sierra::odyplan::v3::basic::io::json {
    class JSONSink : public IIOSink {
    public:
        explicit JSONSink(std::string path)
            : mPath(std::move(path)), mData(nlohmann::ordered_json::array()) {}

        ~JSONSink() override {
            try {
                if (mInUse) {
                    JSONSink::flush();
                }
            } catch (...) {
                std::fputs("JsonSink: flush failed in destructor (unknown exception)\n", stderr);
            }
        }

        JSONSink(JSONSink const&) = delete;

        JSONSink& operator=(JSONSink const&) = delete;

        JSONSink(JSONSink&&) = delete;

        JSONSink& operator=(JSONSink&&) = delete;

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
            mData.push_back(mCurrent);
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

            std::ofstream file(mPath);

            if (!file.is_open()) {
                throw std::runtime_error("JsonSink: cannot open file for writing");
            }

            file << mData.dump(2);

            mData.clear();
            mInUse = false;
        }

        std::string mPath;
        nlohmann::ordered_json mData;
        nlohmann::ordered_json mCurrent;
        bool mInUse = false;
    };
}  // namespace sierra::odyplan::v3::basic::io::json

#endif  // SIERRA_ODYPLAN_V3_BASIC_IO_JSON_JSON_SINK_HPP
