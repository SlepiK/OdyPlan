#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP
#include <basic/operators/pipe/join/physical/details/join_side.h>
#include <chrono>
#include <mutex>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    class JoinBuffer {
    public:
        struct RetainPointer {
            core::shared::types::ValueTypeId typeId;
            void* ptr;
        };

        struct JoinSnapshot {
            bool wasLeft = false;
            std::vector<std::byte> data;
            std::vector<std::size_t> fieldOffsets;
            std::vector<core::shared::types::ValueTypeId> fieldTypeIds;
            Timestamp timestamp;

            std::vector<std::byte> candidateData;
            std::vector<Timestamp> candidateTimestamps;
            std::vector<std::size_t> candidateFieldOffsets;
            std::vector<core::shared::types::ValueTypeId> candidateFieldTypeIds;
            std::size_t candidateRowByteSize = 0;
            std::size_t candidateCount = 0;

            std::vector<RetainPointer> retainPointers;
            const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry = nullptr;

            [[nodiscard]] const std::byte* ownFieldPtr(std::size_t fieldIdx) const;

            [[nodiscard]] const std::byte* candidateFieldPtr(std::size_t candidateIdx, std::size_t fieldIdx) const;
        };

        JoinBuffer(core::logical::port::Port leftSchema,
                   core::logical::port::Port rightSchema,
                   const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry,
                   std::chrono::milliseconds leftWindowSize,
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

        auto pushLeft(std::size_t index, const void* data) -> void;

        void pushRight(std::size_t index, const void* data);

        void endLeft(Timestamp timestamp);

        void endRight(Timestamp timestamp);

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        JoinSnapshot* snapshotForComputeAndUnlock(Timestamp evictTimestamp, bool callFromLeft);

        static void releaseSnapshot(JoinSnapshot* snapshot);

        void enableEqui(const std::vector<std::size_t>& leftFieldIndices,
                        const std::vector<std::size_t>& rightFieldIndices,
                        const std::vector<core::shared::types::ValueTypeId>& keyTypeIds);

    private:
        JoinSide mLeftSide;
        JoinSide mRightSide;
        std::mutex mMutex;
        const core::runtime::types::RuntimeTypeRegistry* mRuntimeTypeRegistry;

        std::vector<std::size_t> mCandidates;
        bool mCandidateOnRight = false;
        bool mAllReleased = false;

        void evict(Timestamp timestamp, bool callFromLeft);

        void matchAgainst(const JoinSide& sourceSide, JoinSide& otherSide, const std::vector<std::size_t>& sourceKeyFieldIndices, Timestamp timestamp, std::chrono::milliseconds sourceWindowSize);


        void copyRowIntoSnapshot(const JoinSide& side, std::uint64_t rowIndex, std::vector<std::byte>& dst, std::size_t dstOffset,
                                 std::vector<std::size_t>& offsetsOut, std::vector<core::shared::types::ValueTypeId>& typeIdsOut,
                                 std::vector<RetainPointer>& retainedOut) const;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP