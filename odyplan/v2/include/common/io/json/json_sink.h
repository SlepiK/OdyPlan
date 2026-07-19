#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSINK_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSINK_H

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "common/io/io_sink.h"

namespace sierra::odyplan::v2::common::io {
    class JsonSink : public IIOSink {
    public:
        explicit JsonSink(const std::string& path)
            : mPath(path),
                mData(nlohmann::json::array()) {
        }

        ~JsonSink() override {
            try {
                if (!mData.empty()) {
                    JsonSink::flush();
                }
            } catch (...) {
            }
        }

        JsonSink(JsonSink const&) = delete;
        JsonSink& operator =(JsonSink const&) = delete;
        JsonSink(JsonSink&&) = delete;
        JsonSink& operator=(JsonSink&&) = delete;

        void begin() override {
            mCurrent = nlohmann::json::object();
        }

        void write(const char* key, core::common::identifier::TypeId typeId, const void* rawData) override {
            using T = core::common::identifier::Type;

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
        nlohmann::json mData;
        nlohmann::json mCurrent;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSINK_H