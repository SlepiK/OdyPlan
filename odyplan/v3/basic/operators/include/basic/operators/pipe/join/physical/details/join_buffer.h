#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP
#include <chrono>
#include <mutex>

#include <basic/operators/pipe/join/physical/details/join_side.h>
#include <core/physical/types/value_type_registry.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    class JoinBuffer {
    public:
        JoinBuffer(core::logical::port::Port leftSchema, core::logical::port::Port rightSchema,
            const core::physical::types::ValueTypeRegistry& typeRegistry, std::chrono::milliseconds leftWindowSize,
            std::chrono::milliseconds rightWindowSize);

        ~JoinBuffer();

        JoinBuffer(const JoinBuffer&) = delete;
        JoinBuffer& operator=(const JoinBuffer&) = delete;
        JoinBuffer(JoinBuffer&&) = delete;
        JoinBuffer& operator=(JoinBuffer&&) = delete;

        void close();

        void lock();

        void unlock();

        void beginLeft();

        void beginRight();

        auto pushLeft(std::size_t index, const void *data) -> void;

        void pushRight(std::size_t index, const void* data);

        void endLeft(Timestamp timestamp);

        void endRight(Timestamp timestamp);

        void evict(Timestamp timestamp, bool callFromLeft);

        [[nodiscard]] std::size_t getCandidateSize() const;

        [[nodiscard]] const std::byte* getCandidatePtr(std::size_t candidateIndex, std::size_t fieldIndex) const;

        [[nodiscard]] Timestamp getCandidateTimestamp(std::size_t candidateIndex) const;

        void enableEqui(std::size_t leftFieldIndex, std::size_t rightFieldIndex,
            core::physical::types::ValueTypeRegistry::HashFn hashFunction, core::physical::types::ValueTypeRegistry::EqualFn equalFunction);

    private:
        JoinSide mLeftSide;
        JoinSide mRightSide;
        std::mutex mMutex;

        std::vector<std::size_t> mCandidates;
        bool mCandidateOnRight = false;
        bool mAllReleased = false;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP