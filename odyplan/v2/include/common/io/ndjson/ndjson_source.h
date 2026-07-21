#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_JSON_NDJSONSOURCE_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_JSON_NDJSONSOURCE_H

#include <fstream>
#include <nlohmann/json.hpp>
#include "common/io/io_source.h"

namespace sierra::odyplan::v2::common::io {
    class NDJSONSource : public IIOSource{
    public:
        explicit NDJSONSource(const std::string& path) {
            mFileHandle.open(path);

            if (!mFileHandle.is_open()) {
                throw std::invalid_argument("NDJSONSource: Could not open json file: " + path);
            }
        }

        ~NDJSONSource() override = default;

        NDJSONSource(NDJSONSource const&) = delete;
        NDJSONSource& operator =(NDJSONSource const&) = delete;
        NDJSONSource(NDJSONSource&&) = delete;
        NDJSONSource& operator=(NDJSONSource&&) = delete;

        [[nodiscard]] bool available() override {
            std::string currentLine;

            while (std::getline(mFileHandle, currentLine)) {
                ++mIndex;

                if (currentLine.find_first_not_of("\t\r\n") == std::string::npos) {
                    continue;
                }

                nlohmann::json dataObj;
                try {
                    dataObj = nlohmann::json::parse(currentLine);
                } catch ( const nlohmann::json::parse_error& err) {
                    throw std::runtime_error("NDJSONSource: parsing error: " + std::string(err.what()));
                }

                if (!dataObj.is_object()) {
                    throw std::runtime_error("NDJSONSource: object expected");
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

            return false;

        }

        [[nodiscard]] const IOData& current() const override {
            return mCurrentData;
        }

        void reset() override {
            mFileHandle.clear();
            mFileHandle.seekg(0, std::ios::beg);
            mIndex = 0;
            mCurrentData.clear();
        }

    private:
        std::ifstream mFileHandle;
        nlohmann::ordered_json mData;
        std::size_t mIndex{0};
        IOData mCurrentData;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_JSON_NDJSONSOURCE_H