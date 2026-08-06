#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_JSON_JSON_SINK_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_JSON_JSON_SINK_HPP

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <utility>
#include <basic/io/io_sink.h>

namespace sierra::odyplan::v3::basic::io::json {
    class JSONSink : public IIOSink {
    public:
        explicit JSONSink(std::string  path)
            : mPath(std::move(path)),
                mData(nlohmann::json::array()) {
        }

        ~JSONSink() override {
            try {
                if (!mData.empty()) {
                    JSONSink::flush();
                }
            } catch (...) {
                std::fputs("JsonSink: flush failed in destructor (unknown exception)\n", stderr);
            }
        }

        JSONSink(JSONSink const&) = delete;
        JSONSink& operator =(JSONSink const&) = delete;
        JSONSink(JSONSink&&) = delete;
        JSONSink& operator=(JSONSink&&) = delete;

        void begin() override {
            mCurrent = nlohmann::json::object();
        }

        void write(const char* key, const core::common::identifier::TypeId typeId, const void* rawData) override {
            using T = core::common::identifier::Type;

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
                    throw std::invalid_argument("JsonSink: Unknown typeId");
            }
            // NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
        }

        void end() override {
            mData.push_back(mCurrent);
            mCurrent.clear();
        }

        void flush() override {
            std::ofstream file(mPath);

            if (!file.is_open()) {
                throw std::runtime_error("JsonSink: cannot open file for writing");
            }

            file << mData.dump(2);

            mData.clear();
        }

    private:
        std::string mPath;
        nlohmann::ordered_json mData;
        nlohmann::ordered_json mCurrent;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_IO_JSON_JSON_SINK_HPP