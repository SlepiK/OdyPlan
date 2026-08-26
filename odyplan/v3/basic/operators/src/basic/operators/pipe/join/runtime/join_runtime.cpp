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
                       const std::uint64_t* leftKeyFieldIndices,
                       const std::uint64_t* rightKeyFieldIndices,
                       const core::shared::types::ValueTypeId* keyTypeIds,
                       const std::uint64_t equiKeyCnt) {
        const auto* runtimeTypeRegistry =
            static_cast<const core::runtime::types::RuntimeTypeRegistry*>(typeRegistryRawPtr);

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
                                              runtimeTypeRegistry,
                                              std::chrono::milliseconds(leftWindowSize),
                                              std::chrono::milliseconds(rightWindowSize));

        if (hasEquiKey != 0) {
            std::vector<std::size_t> leftIndices;
            std::vector<std::size_t> rightIndices;
            std::vector<core::shared::types::ValueTypeId> typeIds;

            leftIndices.reserve(equiKeyCnt);
            rightIndices.reserve(equiKeyCnt);
            typeIds.reserve(equiKeyCnt);

            for (std::uint64_t i = 0; i < equiKeyCnt; ++i) {
                leftIndices.push_back(static_cast<std::size_t>(leftKeyFieldIndices[i]));
                rightIndices.push_back(static_cast<std::size_t>(rightKeyFieldIndices[i]));
                typeIds.push_back(keyTypeIds[i]);
            }

            buffer->enableEqui(leftIndices, rightIndices, typeIds);
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

    void *rtJoinSnapshotForComputeAndUnlock(void *state, const std::uint64_t timestamp, const bool callFromLeft) {
        auto* buffer = static_cast<physical::details::JoinBuffer*>(state);
        return buffer->snapshotForComputeAndUnlock(
            physical::details::Timestamp(std::chrono::milliseconds(timestamp)),callFromLeft);
    }

    const void *rtJoinSnapshotGetOwnPtr(void *snapshot, const std::uint64_t fieldIndex) {
        const auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return s->ownFieldPtr(static_cast<std::size_t>(fieldIndex));
    }

    const void *rtJoinSnapshotGetCandidatePtr(void *snapshot, const std::uint64_t index, const std::uint64_t fieldIndex) {
        auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return s->candidateFieldPtr(static_cast<std::size_t>(index), static_cast<std::size_t>(fieldIndex));
    }

    std::uint64_t rtJoinSnapshotGetCandidateTimestamp(void *snapshot, const std::uint64_t index) {
        const auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return s->candidateTimestamps.at(static_cast<std::size_t>(index)).time_since_epoch().count();
    }

    std::uint64_t rtJoinSnapshotGetOwnTimestamp(void *snapshot) {
        auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return s->timestamp.time_since_epoch().count();
    }

    std::uint64_t rtJoinSnapshotGetCount(void *snapshot) {
        auto* s = static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot);
        return static_cast<std::uint64_t>(s->candidateCount);
    }

    void rtJoinSnapshotRelease(void *snapshot) {
        physical::details::JoinBuffer::releaseSnapshot(static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot));
    }

    bool rtJoinSnapshotGetWasLeft(void* snapshot) {
        return static_cast<physical::details::JoinBuffer::JoinSnapshot*>(snapshot)->wasLeft;
    }

    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime