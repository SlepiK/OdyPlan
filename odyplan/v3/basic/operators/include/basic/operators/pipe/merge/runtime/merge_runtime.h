#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP

#include <mutex>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime {
    extern "C" {
    void* rtCreateMergeLock(const void* /*typeRegistry*/);

    void rtDestroyMergeLock(void* state);

    void rtMergeLock(void* state);

    void rtMergeUnlock(void* state);
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_MERGE_RUNTIME_MERGE_RUNTIME_HPP