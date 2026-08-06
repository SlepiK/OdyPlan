#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP

#include <mutex>

extern "C" {
    inline void* rtCreateMergeLock(const void* /*typeRegistry*/) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return new std::mutex();
    }

    inline void rtDestroyMergeLock(void* state) {
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete static_cast<std::mutex*>(state);
    }

    inline void rtMergeLock(void* state) {
        static_cast<std::mutex*>(state)->lock();
    }

    inline void rtMergeUnlock(void* state) {
        static_cast<std::mutex*>(state)->unlock();
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP