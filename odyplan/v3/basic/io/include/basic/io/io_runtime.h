#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_IO_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_IO_RUNTIME_HPP
#include <cassert>
#include <memory>
#include <vector>

#include <basic/io/io_source.h>
#include <basic/io/io_sink.h>

inline std::vector<std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSource>> gSources;

inline sierra::odyplan::v3::basic::io::IOSourceId registerSource(std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSource> source) {
    gSources.push_back(std::move(source));
    return gSources.size() - 1;
}

inline std::vector<std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSink>> gSinks;

inline sierra::odyplan::v3::basic::io::IOSinkId registerSink(std::unique_ptr<sierra::odyplan::v3::basic::io::IIOSink> sink) {
    gSinks.push_back(std::move(sink));
    return gSinks.size() - 1;
}

extern "C" {
    inline bool rtIOSourceHasNext(const sierra::odyplan::v3::basic::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size()) {
            return false;
        }

        return gSources.at(index)->available();
    }

    inline const char* rtIOSourceGetField(const sierra::odyplan::v3::basic::io::IOSourceId index, const char* fieldName) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index >= gSources.size() || fieldName == nullptr) {
            return nullptr;
        }

        const sierra::odyplan::v3::basic::io::IOData& current = gSources.at(index)->current();
        const auto iter = current.find(fieldName);
        return iter != current.end() ? iter->second.c_str() : nullptr;
    }

    inline void rtIOSourceReset(const sierra::odyplan::v3::basic::io::IOSourceId index) {
        assert(index < gSources.size() && "invalid IOSource");
        if (index < gSources.size()) {
            (*gSources.at(index)).reset();
        }
    }

    inline void rtIOSinkBeginData(const sierra::odyplan::v3::basic::io::IOSinkId index) {
        gSinks.at(index)->begin();
    }

    inline void rtIOSinkWrite(const sierra::odyplan::v3::basic::io::IOSinkId index, const sierra::odyplan::v3::core::shared::types::ValueTypeId typeId, const char* key, const void* value) {
        gSinks.at(index)->write(key, typeId, value);
    }

    inline void rtIOSinkEndData(const sierra::odyplan::v3::basic::io::IOSinkId index) {
        gSinks.at(index)->end();
    }

    inline void rtIOSinkFlush(const sierra::odyplan::v3::basic::io::IOSinkId index) {
        gSinks.at(index)->flush();
    }

    inline void rtIOSinkFlushAll() {
        for (const auto& sink : gSinks) {
            sink->flush();
        }
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_IO_IO_RUNTIME_HPP