#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SINK_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SINK_HPP

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <utility>
#include <basic/io/io_sink.h>

namespace sierra::odyplan::v3::basic::io::ndjson {
    class NDJSONSink : public IIOSink {
    public:
        explicit NDJSONSink(std::string  path)
            : mPath(std::move(path)) {
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
        NDJSONSink& operator =(NDJSONSink const&) = delete;
        NDJSONSink(NDJSONSink&&) = delete;
        NDJSONSink& operator=(NDJSONSink&&) = delete;

        void begin() override {
            mCurrent = nlohmann::json::object();
            mInUse = true;
        }

        void write(const char* key, const core::shared::types::ValueTypeId typeId, const void* rawData) override {
            using T = core::shared::types::BuiltinValueType;

            // NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
            switch (typeId) {
                case std::to_underlying(T::BOOL):
                    mCurrent[key] = *static_cast<const bool*>(rawData);
                    break;
                case std::to_underlying(T::INT8):
                    mCurrent[key] = *static_cast<const std::int8_t*>(rawData);
                    break;
                case std::to_underlying(T::INT16):
                    mCurrent[key] = *static_cast<const std::int16_t*>(rawData);
                    break;
                case std::to_underlying(T::INT32):
                    mCurrent[key] = *static_cast<const std::int32_t*>(rawData);
                    break;
                case std::to_underlying(T::INT64):
                    mCurrent[key] = *static_cast<const std::int64_t*>(rawData);
                    break;
                case std::to_underlying(T::FLOAT):
                    mCurrent[key] = *static_cast<const std::float_t*>(rawData);
                    break;
                case std::to_underlying(T::DOUBLE):
                    mCurrent[key] = *static_cast<const std::double_t*>(rawData);
                    break;
                case std::to_underlying(T::STRING):
                    mCurrent[key] = static_cast<const char*>(rawData);
                    break;
                default:
                    throw std::invalid_argument("NDJSONSink: Unknown typeId");
            }
            // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
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
}

#endif //SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SINK_HPP