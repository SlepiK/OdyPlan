#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSOURCE_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSOURCE_H

#include <fstream>
#include <nlohmann/json.hpp>
#include "common/io/io_source.h"

namespace sierra::odyplan::v2::common::io {
    class JsonSource : public IIOSource{
    public:
        explicit JsonSource(const std::string& path) {
            std::ifstream file(path);

            if (!file.is_open()) {
                throw std::invalid_argument("JsonSource: Could not open json file: " + path);
            }

            mData = nlohmann::json::parse(file);

            if (!mData.is_array()) {
                throw std::invalid_argument("JsonSource: root of the json source must be a json array");
            }
        }

        [[nodiscard]] std::optional<IOData> next() override {
            if (mIndex >= mData.size()) {
                return std::nullopt;
            }

            const auto& dataObj = mData[mIndex++];
            if (!dataObj.is_object()) {
                throw std::runtime_error("JsonSource: expected JSON object");
            }

            IOData data;
            for (const auto& [key, value] : dataObj.items()) {
                data.emplace(key, value.dump());
            }

            return data;
        }

        void reset() override {
            mIndex = 0;
        }

    private:
        nlohmann::json mData;
        std::size_t mIndex;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSOURCE_H