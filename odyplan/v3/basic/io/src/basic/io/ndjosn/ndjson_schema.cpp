#include <basic/io/ndjson/ndjson_schema.h>

namespace sierra::odyplan::v3::basic::io::ndjson {
    core::logical::port::Port inferSchema(const std::string& samplePath) {
        std::ifstream file(samplePath);

        if (!file.is_open()) {
            throw std::runtime_error("NDJSON::inferSchema: Could not open sample file");
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
                continue;
            }

            auto sample = nlohmann::ordered_json::parse(line);

            if (!sample.is_object()) {
                throw std::runtime_error("NDJSON::inferSchema: Could not parse sample file");
            }

            std::vector<core::logical::port::PortDescription> descriptions;
            descriptions.reserve(sample.size());

            for (auto& [key, value] : sample.items()) {
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

        throw std::runtime_error("NDJSON::inferSchema: Could not parse sample file");
    }
}