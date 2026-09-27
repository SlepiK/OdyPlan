#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP
#include <basic/operators/pipe/join/physical/details/join_side.h>
#include <chrono>
#include <mutex>
#include <shared_mutex>

#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    class JoinBuffer {
    public:
        struct RetainPointer {
            core::shared::types::ValueTypeId typeId;
            void* ptr;
        };

        struct OutputRow {
            std::vector<std::byte> data;
            core::shared::types::Timestamp startTs = 0;
            core::shared::types::Timestamp endTs = 0;
            std::int64_t startTb = 0;
            std::int64_t endTb = 0;
            std::vector<RetainPointer> retainPointers;
        };

        struct JoinSnapshot {
            bool wasLeft = false;
            std::vector<std::byte> data;
            std::vector<std::uint64_t> fieldOffsets;
            std::vector<core::shared::types::ValueTypeId> fieldTypeIds;
            core::shared::types::Timestamp startTs;
            core::shared::types::Timestamp endTs;
            std::int64_t startTb;
            std::int64_t endTb;

            std::vector<std::byte> candidateData;
            std::vector<core::shared::types::Timestamp> candidateStarts;
            std::vector<core::shared::types::Timestamp> candidateEnds;
            std::vector<std::int64_t> candidateStartsTb;
            std::vector<std::int64_t> candidateEndsTb;
            std::vector<std::uint64_t> candidateFieldOffsets;
            std::vector<core::shared::types::ValueTypeId> candidateFieldTypeIds;
            std::size_t candidateRowByteSize = 0;
            std::size_t candidateCount = 0;

            std::vector<RetainPointer> retainPointers;
            const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry = nullptr;

            std::vector<std::size_t> markedForRemove;
            bool isExtraction = false;

            std::vector<std::size_t> matchIndicies;

            [[nodiscard]] const std::byte* ownFieldPtr(std::size_t fieldIdx) const;

            [[nodiscard]] const std::byte* candidateFieldPtr(std::size_t candidateIdx, std::size_t fieldIdx) const;
        };

        JoinBuffer(core::logical::port::Port leftSchema,
                   core::logical::port::Port rightSchema,
                   core::logical::port::Port outputSchema,
                   const core::runtime::types::RuntimeTypeRegistry* runtimeTypeRegistry,
                   bool outOfOrder);

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

        void pushLeft(std::size_t index, const void* data);

        void pushRight(std::size_t index, const void* data);

        void endLeft(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, std::int64_t startTb, std::int64_t endTb);

        void endRight(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, std::int64_t startTb, std::int64_t endTb);

        void matchLeft(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, std::int64_t startTb, std::int64_t endTb);

        void matchRight(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, std::int64_t startTb, std::int64_t endTb);

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        JoinSnapshot* snapshotForCompute(bool callFromLeft, bool extract);

        static void releaseSnapshot(JoinSnapshot* snapshot);

        void enableEqui(const std::vector<std::size_t>& leftFieldIndices,
                        const std::vector<std::size_t>& rightFieldIndices,
                        const std::vector<core::shared::types::ValueTypeId>& keyTypeIds);

        void evict(core::shared::types::Timestamp threshold, bool callFromLeft, bool isFinal);

        void markCandidateForRemove(JoinSnapshot *snapshot, std::size_t candidateIdx);

        void markCandidateAsMatch(JoinSnapshot *snapshot, std::size_t candidateIdx);

        void markAllCandidateAsMatch(JoinSnapshot *snapshot, bool extract);

        void removeMarkedCandidates(JoinSnapshot *snapshot, bool wasLeft);

        void bufferOutput(const void* ownRowPtr, std::size_t ownRowSize, const void* candRowPtr, std::size_t candRowSize, bool onLeft, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs,
            std::int64_t startTb, std::int64_t endTb);

        std::size_t drainOutput(core::shared::types::Timestamp watermark, std::size_t maxCount, std::vector<std::size_t>& outIndices);

        void removeOutputRows(const std::vector<std::size_t>& indices);

        [[nodiscard]] const OutputRow& getOutputRow(std::size_t index) const;

        [[nodiscard]] std::size_t getOutputRowSize() const;

        [[nodiscard]] core::shared::types::Timestamp getMinStartTimestampLeft() const;

        [[nodiscard]] core::shared::types::Timestamp getMaxStartTimestampRight() const;

    private:
        bool mOutOfOrder;
        JoinSide mLeftSide;
        JoinSide mRightSide;
        std::shared_mutex mMutex;
        const core::runtime::types::RuntimeTypeRegistry* mRuntimeTypeRegistry;

        std::vector<std::size_t> mCandidates;
        bool mAllReleased = false;
        core::shared::types::Timestamp mLastHeartbeat = 0;

        std::vector<OutputRow> mOutputBuffer;
        std::vector<std::size_t> mOutputOrder;
        core::logical::port::Port mOutputSchema;
        std::vector<std::byte> mOutputTempBuf;
        std::mutex mOutputMutex;

        void matchAgainst(const JoinSide& sourceSide, JoinSide& otherSide, const std::vector<std::uint64_t>& sourceKeyFieldIndices, core::shared::types::Timestamp sourceStartTs, core::shared::types::Timestamp sourceEndTs, bool extract);


        void copyRowIntoSnapshot(const JoinSide& side, std::uint64_t rowIndex, std::vector<std::byte>& dst, std::size_t dstOffset,
                                 std::vector<std::size_t>& offsetsOut, std::vector<core::shared::types::ValueTypeId>& typeIdsOut,
                                 std::vector<RetainPointer>& retainedOut) const;
    };
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_PHYSICAL_DETAILS_JOIN_BUFFER_HPP