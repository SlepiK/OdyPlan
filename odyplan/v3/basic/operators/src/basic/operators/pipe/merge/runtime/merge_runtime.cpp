#include <basic/operators/pipe/merge/runtime/merge_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::runtime {
    extern "C" {
        void* rtCreateMergeLock(const void* /*typeRegistry*/) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            return new std::mutex();
        }

        void rtDestroyMergeLock(void* state) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete static_cast<std::mutex*>(state);
        }

        void rtMergeLock(void* state) {
            static_cast<std::mutex*>(state)->lock();
        }

        void rtMergeUnlock(void* state) {
            static_cast<std::mutex*>(state)->unlock();
        }
    }
}