#include <basic/operators/pipe/join/runtime/join_runtime.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime {
    extern "C" {
    // NOLINTNEXTLINE(readability-function-size)
    void* rtJoinCreate(const void* typeRegistryRawPtr,
                       const core::shared::types::ValueTypeId* leftTypeIds,
                       const std::uint64_t leftFieldSize,
                       const core::shared::types::ValueTypeId* rightTypeIds,
                       const std::uint64_t rightFieldSize,
                       const std::uint64_t leftWindowSize,
                       const std::uint64_t rightWindowSize,
                       const std::uint8_t hasEquiKey,
                       const std::uint64_t leftKeyFieldIndex,
                       const std::uint64_t rightKeyFieldIndex,
                       const core::shared::types::ValueTypeId keyTypeId) {
        const auto* typeRegistry =
            static_cast<const core::physical::types::ValueTypeRegistry*>(typeRegistryRawPtr);

        core::logical::port::Port leftSchema;
        core::logical::port::Port rightSchema;

        for (std::uint64_t i = 0; i < leftFieldSize; ++i) {
            leftSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = leftTypeIds[i]});
        }

        for (std::uint64_t i = 0; i < rightFieldSize; ++i) {
            rightSchema.push_back(
                core::logical::port::PortDescription{.valueTypeId = rightTypeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* buffer =
            new physical::details::JoinBuffer(std::move(leftSchema),
                                              std::move(rightSchema),
                                              *typeRegistry,
                                              std::chrono::milliseconds(leftWindowSize),
                                              std::chrono::milliseconds(rightWindowSize));

        if (hasEquiKey != 0) {
            buffer->enableEqui(leftKeyFieldIndex,
                               rightKeyFieldIndex,
                               typeRegistry->getHashFunction(keyTypeId),
                               typeRegistry->getEqualFunction(keyTypeId));
        }

        return buffer;
    }

    void rtJoinDestroy(void* state) {
        auto* buffer = static_cast<physical::details::JoinBuffer*>(state);

        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    void rtJoinLock(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->lock();
    }

    void rtJoinUnlock(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->unlock();
    }

    void rtJoinBeginLeft(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->beginLeft();
    }

    void rtJoinBeginRight(void* state) {
        static_cast<physical::details::JoinBuffer*>(state)->beginRight();
    }

    void rtJoinPushLeft(void* state, const std::uint64_t index, const void* data) {
        static_cast<physical::details::JoinBuffer*>(state)->pushLeft(index, data);
    }

    void rtJoinPushRight(void* state, const std::uint64_t index, const void* data) {
        static_cast<physical::details::JoinBuffer*>(state)->pushRight(index, data);
    }

    void rtJoinEndLeft(void* state, const std::int64_t timestamp) {
        static_cast<physical::details::JoinBuffer*>(state)->endLeft(
            physical::details::Timestamp(std::chrono::milliseconds(timestamp)));
    }

    void rtJoinEndRight(void* state, const std::int64_t timestamp) {
        static_cast<physical::details::JoinBuffer*>(state)->endRight(
            physical::details::Timestamp(std::chrono::milliseconds(timestamp)));
    }

    std::uint64_t rtJoinGetCandidateSize(void* state) {
        return static_cast<physical::details::JoinBuffer*>(state)->getCandidateSize();
    }

    const void* rtJoinGetCandidatePtr(void* state,
                                      const std::uint64_t candidateIndex,
                                      const std::uint64_t fieldIndex) {
        return static_cast<physical::details::JoinBuffer*>(state)->getCandidatePtr(candidateIndex,
                                                                                   fieldIndex);
    }

    std::int64_t rtJoinGetCandidateTimestamp(void* state, const std::uint64_t candidateIndex) {
        const auto timestamp =
            static_cast<physical::details::JoinBuffer*>(state)->getCandidateTimestamp(
                candidateIndex);
        return timestamp.time_since_epoch().count();
    }

    void rtJoinEvict(void* state, const std::int64_t timestampMs, const bool callFromLeft) {
        static_cast<physical::details::JoinBuffer*>(state)->evict(
            physical::details::Timestamp(std::chrono::milliseconds(timestampMs)), callFromLeft);
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime