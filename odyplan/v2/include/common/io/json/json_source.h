#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSOURCE_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSOURCE_H

#include <fstream>
#include <nlohmann/json.hpp>
#include "common/io/io_source.h"

namespace sierra::odyplan::v2::common::io {
    class JSONSource : public IIOSource{
    public:
        explicit JSONSource(const std::string& path) {
            std::ifstream file(path);

            if (!file.is_open()) {
                throw std::invalid_argument("JsonSource: Could not open json file: " + path);
            }

            mData = nlohmann::json::parse(file);

            if (!mData.is_array()) {
                throw std::invalid_argument("JsonSource: root of the json source must be a json array");
            }
        }

        ~JSONSource() override = default;

        JSONSource(JSONSource const&) = delete;
        JSONSource& operator =(JSONSource const&) = delete;
        JSONSource(JSONSource&&) = delete;
        JSONSource& operator=(JSONSource&&) = delete;

        [[nodiscard]] bool available() override {
            if (mIndex >= mData.size()) {
                return false;
            }

            const auto& dataObj = mData.at(mIndex++);
            if (!dataObj.is_object()) {
                throw std::runtime_error("JsonSource: expected JSON object");
            }

            mCurrentData.clear();
            for (const auto& [key, value] : dataObj.items()) {
                if (value.is_string()) {
                    mCurrentData.emplace(key, value.get<std::string>());
                } else {
                    mCurrentData.emplace(key, value.dump());
                }
            }

            return true;

        }

        [[nodiscard]] const IOData& current() const override {
            return mCurrentData;
        }

        void reset() override {
            mIndex = 0;
            mCurrentData.clear();
        }

    private:
        nlohmann::ordered_json mData;
        std::size_t mIndex{0};
        IOData mCurrentData;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_JSON_JSONSOURCE_H