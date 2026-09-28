#include <basic/io/json/json_schema.h>

namespace sierra::odyplan::v3::basic::io::json {
    core::logical::port::Port inferSchema(const std::string& samplePath) {
        std::ifstream file(samplePath);

        if (!file.is_open()) {
            throw std::runtime_error("JSON::inferSchema: Could not open sample file");
        }

        nlohmann::ordered_json root;
        try {
            root = nlohmann::ordered_json::parse(file);
        } catch (const nlohmann::json::parse_error& e) {
            throw std::runtime_error("JSON::inferSchema: Could not parse schema");
        }

        nlohmann::ordered_json sample;
        if (root.is_array()) {
            if (root.empty()) {
                throw std::runtime_error("JSON::inferSchema: Empty array");
            }
            sample = root.front();
        } else if (root.is_object()) {
            sample = root;
        } else {
            throw std::runtime_error("JSON::inferSchema: root is neither object nor array");
        }

        if (!sample.is_object()) {
            throw std::runtime_error("JSON::inferSchema: first element is not a object");
        }

        std::vector<core::logical::port::PortDescription> descriptions;
        descriptions.reserve(sample.size());

        for (const auto& [key, value] : sample.items()) {
            core::shared::types::BuiltinValueType type;
            switch (value.type()) {
                case nlohmann::json::value_t::number_integer:
                case nlohmann::json::value_t::number_unsigned:
                    type = core::shared::types::BuiltinValueType::INT64;
                    break;
                case nlohmann::json::value_t::number_float:
                    type = core::shared::types::BuiltinValueType::DOUBLE;
                    break;
                case nlohmann::json::value_t::boolean:
                    type = core::shared::types::BuiltinValueType::BOOL;
                    break;
                case nlohmann::json::value_t::string:
                    type = core::shared::types::BuiltinValueType::STRING;
                    break;
                default:
                    throw std::runtime_error("NDJSON::inferSchema: Unknown value type");
            }

            descriptions.push_back(core::logical::port::PortDescription{
                .valueTypeId = std::to_underlying(type),
                .fieldName = key
            });
        }

        return core::logical::port::Port{descriptions};
    }
}