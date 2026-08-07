#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP

#include <cstdint>

#include <basic/operators/pipe/join/physical/details/join_buffer.h>
#include <core/physical/types/value_type_registry.h>
#include <core/logical/port/port.h>

extern "C" {
    //NOLINTNEXTLINE(readability-function-size)
    inline void* rtJoinCreate(const void* typeRegistryRawPtr,
        const sierra::odyplan::v3::core::shared::types::ValueTypeId* leftTypeIds, const std::uint64_t leftFieldSize,
        const sierra::odyplan::v3::core::shared::types::ValueTypeId* rightTypeIds, const std::uint64_t rightFieldSize,
        const std::uint64_t leftWindowSize, const std::uint64_t rightWindowSize,
        const std::uint8_t hasEquiKey, const std::uint64_t leftKeyFieldIndex,
        const std::uint64_t rightKeyFieldIndex, const sierra::odyplan::v3::core::shared::types::ValueTypeId keyTypeId) {
        const auto* typeRegistry = static_cast<const sierra::odyplan::v3::core::physical::types::ValueTypeRegistry*>(typeRegistryRawPtr);

        sierra::odyplan::v3::core::logical::port::Port leftSchema;
        sierra::odyplan::v3::core::logical::port::Port rightSchema;

        for (std::uint64_t i = 0; i < leftFieldSize; ++i) {
            leftSchema.push_back(sierra::odyplan::v3::core::logical::port::PortDescription {.valueTypeId = leftTypeIds[i]});
        }

        for (std::uint64_t i = 0; i < rightFieldSize; ++i) {
            rightSchema.push_back(sierra::odyplan::v3::core::logical::port::PortDescription {.valueTypeId = rightTypeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* buffer = new sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer(
            std::move(leftSchema), std::move(rightSchema),
            *typeRegistry,
            std::chrono::milliseconds(leftWindowSize),
            std::chrono::milliseconds(rightWindowSize)
            );

        if (hasEquiKey != 0) {
            buffer->enableEqui(leftKeyFieldIndex, rightKeyFieldIndex, typeRegistry->getHashFunction(keyTypeId), typeRegistry->getEqualFunction(keyTypeId));
        }

        return buffer;
    }

    inline void rtJoinDestroy(void* state) {
        auto* buffer = static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state);

        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    inline void rtJoinLock(void* state) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->lock();
    }

    inline void rtJoinUnlock(void* state) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->unlock();
    }

    inline void rtJoinBeginLeft(void* state) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->beginLeft();
    }

    inline void rtJoinBeginRight(void* state) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->beginRight();
    }

    inline void rtJoinPushLeft(void* state, const std::uint64_t index, const void* data) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->pushLeft(index, data);
    }

    inline void rtJoinPushRight(void* state, const std::uint64_t index, const void* data) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->pushRight(index, data);
    }

    inline void rtJoinEndLeft(void* state, const std::int64_t timestamp) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->endLeft(sierra::odyplan::v3::basic::operators::pipe::join::physical::details::Timestamp(std::chrono::milliseconds(timestamp)));
    }

    inline void rtJoinEndRight(void* state, const std::int64_t timestamp) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->endRight(sierra::odyplan::v3::basic::operators::pipe::join::physical::details::Timestamp(std::chrono::milliseconds(timestamp)));
    }

    inline std::uint64_t rtJoinGetCandidateSize(void* state) {
        return static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->getCandidateSize();
    }

    inline const void* rtJoinGetCandidatePtr(void* state, const std::uint64_t candidateIndex, const std::uint64_t fieldIndex) {
        return static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->getCandidatePtr(candidateIndex, fieldIndex);
    }

    inline std::int64_t rtJoinGetCandidateTimestamp(void* state, const std::uint64_t candidateIndex) {
        const auto timestamp = static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->getCandidateTimestamp(candidateIndex);
        return timestamp.time_since_epoch().count();
    }

    inline void rtJoinEvict(void* state, const std::int64_t timestampMs, const bool callFromLeft) {
        static_cast<sierra::odyplan::v3::basic::operators::pipe::join::physical::details::JoinBuffer*>(state)->evict(sierra::odyplan::v3::basic::operators::pipe::join::physical::details::Timestamp(std::chrono::milliseconds(timestampMs)), callFromLeft);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP