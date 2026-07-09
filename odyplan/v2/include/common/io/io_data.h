#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_IODATA_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_IODATA_H

#include <string>
#include <unordered_map>

namespace sierra::odyplan::v2::common::io {
    using IOData = std::unordered_map<std::string, std::string>;
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_IODATA_H