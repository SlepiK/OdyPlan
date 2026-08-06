#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP

#include <cstdint>

#include "common/operator/pipe/interval_join/physical/internal/interval_join_buffer.h"
#include "core/common/type/type_registry.h"
#include "core/logical/port/port.h"

extern "C" {
    //NOLINTNEXTLINE(readability-function-size)
    inline void* rtJoinCreate(const void* typeRegistryRawPtr,
        const sierra::odyplan::v2::core::common::identifier::TypeId* leftTypeIds, const std::uint64_t leftFieldSize,
        const sierra::odyplan::v2::core::common::identifier::TypeId* rightTypeIds, const std::uint64_t rightFieldSize,
        const std::uint64_t leftWindowSize, const std::uint64_t rightWindowSize,
        const std::uint8_t hasEquiKey, const std::uint64_t leftKeyFieldIndex,
        const std::uint64_t rightKeyFieldIndex, const sierra::odyplan::v2::core::common::identifier::TypeId keyTypeId) {
        const auto* typeRegistry = static_cast<const sierra::odyplan::v2::core::common::type::TypeRegistry*>(typeRegistryRawPtr);

        sierra::odyplan::v2::core::logical::port::Port leftSchema;
        sierra::odyplan::v2::core::logical::port::Port rightSchema;

        for (std::uint64_t i = 0; i < leftFieldSize; ++i) {
            leftSchema.push_back(sierra::odyplan::v2::core::logical::port::PortDescriptor{.typeId = leftTypeIds[i]});
        }

        for (std::uint64_t i = 0; i < rightFieldSize; ++i) {
            rightSchema.push_back(sierra::odyplan::v2::core::logical::port::PortDescriptor{.typeId = rightTypeIds[i]});
        }

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* buffer = new sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer(
            std::move(leftSchema), std::move(rightSchema),
            *typeRegistry,
            std::chrono::milliseconds(leftWindowSize),
            std::chrono::milliseconds(rightWindowSize)
            );

        if (hasEquiKey != 0) {
            buffer->enableEqui(leftKeyFieldIndex, rightKeyFieldIndex, typeRegistry->getHashFn(keyTypeId), typeRegistry->getEqualFn(keyTypeId));
        }

        return buffer;
    }

    inline void rtJoinDestroy(void* state) {
        auto* buffer = static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state);

        buffer->close();
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete buffer;
    }

    inline void rtJoinLock(void* state) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->lock();
    }

    inline void rtJoinUnlock(void* state) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->unlock();
    }

    inline void rtJoinBeginLeft(void* state) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->beginLeft();
    }

    inline void rtJoinBeginRight(void* state) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->beginRight();
    }

    inline void rtJoinPushLeft(void* state, const std::uint64_t index, const void* data) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->pushLeft(index, data);
    }

    inline void rtJoinPushRight(void* state, const std::uint64_t index, const void* data) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->pushRight(index, data);
    }

    inline void rtJoinEndLeft(void* state, const std::int64_t timestamp) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->endLeft(sierra::odyplan::v2::common::opr::pipe::physical::internal::Timestamp(std::chrono::milliseconds(timestamp)));
    }

    inline void rtJoinEndRight(void* state, const std::int64_t timestamp) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->endRight(sierra::odyplan::v2::common::opr::pipe::physical::internal::Timestamp(std::chrono::milliseconds(timestamp)));
    }

    inline std::uint64_t rtJoinGetCandidateSize(void* state) {
        return static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->getCandidateSize();
    }

    inline const void* rtJoinGetCandidatePtr(void* state, const std::uint64_t candidateIndex, const std::uint64_t fieldIndex) {
        return static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->getCandidatePtr(candidateIndex, fieldIndex);
    }

    inline std::int64_t rtJoinGetCandidateTimestamp(void* state, const std::uint64_t candidateIndex) {
        const auto timestamp = static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->getCandidateTimestamp(candidateIndex);
        return timestamp.time_since_epoch().count();
    }

    inline void rtJoinEvict(void* state, const std::int64_t timestampMs, const bool callFromLeft) {
        static_cast<sierra::odyplan::v2::common::opr::pipe::physical::internal::IntervalJoinBuffer*>(state)->evict(sierra::odyplan::v2::common::opr::pipe::physical::internal::Timestamp(std::chrono::milliseconds(timestampMs)), callFromLeft);
    }
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_RUNTIME_HPP