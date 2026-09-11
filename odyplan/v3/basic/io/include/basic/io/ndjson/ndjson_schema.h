#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SCHEMA_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SCHEMA_HPP

#include <fstream>

#include <nlohmann/json.hpp>
#include <core/logical/port/port.h>
#include <core/shared/types/value_type_id.h>

namespace sierra::odyplan::v3::basic::io::ndjson {
    core::logical::port::Port inferSchema(const std::string& samplePath);
}

#endif //SIERRA_ODYPLAN_V3_BASIC_IO_NDJSON_NDJSON_SCHEMA_HPP