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
                JsonSink::flush();
            } catch (...) {
            }
        }

        void write(const IOData &data) override {
            nlohmann::json jsonObj;

            for (const auto& [key, value] : data) {
                jsonObj[key] = value;
            }

            mData.push_back(std::move(jsonObj));
        }

        void flush() override {
            std::ofstream file(mPath);

            if (!file.is_open()) {
                throw std::runtime_error("JsonSink: cannot open file for writing");
            }

            file << mData.dump(2);
        }

    private:
        std::string mPath;
        nlohmann::json mData;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSINK_H