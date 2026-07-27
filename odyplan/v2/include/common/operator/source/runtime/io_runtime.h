#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_IO_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_IO_RUNTIME_H
#include <cassert>
#include <memory>
#include <vector>

#include "common/io/io_source.h"

inline std::vector<std::unique_ptr<sierra::odyplan::v2::common::io::IIOSource>> gSources;

inline sierra::odyplan::v2::common::io::IOSourceId registerSource(std::unique_ptr<sierra::odyplan::v2::common::io::IIOSource> source) {
    gSources.push_back(std::move(source));
    return gSources.size() - 1;
}

extern "C" {
    inline bool rtIOSourceHasNext(const sierra::odyplan::v2::common::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size()) {
            return false;
        }

        return gSources.at(index)->available();
    }

    inline const char* rtIOSourceGetField(const sierra::odyplan::v2::common::io::IOSourceId index, const char* fieldName) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size() || fieldName == nullptr) {
            return nullptr;
        }

        const sierra::odyplan::v2::common::io::IOData& current = gSources.at(index)->current();
        const auto iter = current.find(fieldName);
        return iter != current.end() ? iter->second.c_str() : nullptr;
    }

    inline void rtIOSourceReset(const sierra::odyplan::v2::common::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index < gSources.size()) {
            (*gSources.at(index)).reset();
        }
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SOURCE_IO_RUNTIME_H