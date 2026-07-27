#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_OPERATOR_RUNTIME_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_OPERATOR_RUNTIME_H

#include <mutex>

extern "C" {
    inline void* rtCreateMergeLock(const void* /*typeRegistry*/) {
        return new std::mutex();
    }

    inline void rtDestroyMergeLock(void* state) {
        delete static_cast<std::mutex*>(state);
    }

    inline void rtMergeLock(void* state) {
        static_cast<std::mutex*>(state)->lock();
    }

    inline void rtMergeUnlock(void* state) {
        static_cast<std::mutex*>(state)->unlock();
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_MERGE_OPERATOR_RUNTIME_H