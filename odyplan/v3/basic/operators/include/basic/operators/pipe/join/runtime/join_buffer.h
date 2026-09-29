#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_BUFFER_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_BUFFER_HPP
#include <basic/operators/pipe/join/runtime/join_side.h>
#include <chrono>
#include <mutex>
#include <shared_mutex>

#include <core/shared/types/global_types.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::runtime {
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
            core::shared::types::TiBreak startTb = 0;
            core::shared::types::TiBreak endTb = 0;
            std::vector<RetainPointer> retainPointers;
        };

        struct JoinSnapshot {
            bool wasLeft = false;
            std::vector<std::byte> data;
            std::vector<std::size_t> fieldOffsets;
            std::vector<core::shared::types::ValueTypeId> fieldTypeIds;
            core::shared::types::Timestamp startTs;
            core::shared::types::Timestamp endTs;
            core::shared::types::TiBreak startTb;
            core::shared::types::TiBreak endTb;

            std::vector<std::byte> candidateData;
            std::vector<core::shared::types::Timestamp> candidateStarts;
            std::vector<core::shared::types::Timestamp> candidateEnds;
            std::vector<core::shared::types::TiBreak> candidateStartsTb;
            std::vector<core::shared::types::TiBreak> candidateEndsTb;
            std::vector<std::size_t> candidateFieldOffsets;
            std::vector<core::shared::types::ValueTypeId> candidateFieldTypeIds;
            std::size_t candidateRowByteSize = 0;
            std::size_t candidateCount = 0;

            const core::runtime::types::RuntimeTypeRegistry* mRegistry;

            std::vector<RetainPointer> retainPointers;

            std::vector<std::size_t> markedForRemove;
            bool isExtraction = false;

            std::vector<std::size_t> matchIndicies;

            [[nodiscard]] const std::byte* ownFieldPtr(std::size_t fieldIdx) const;

            [[nodiscard]] const std::byte* candidateFieldPtr(std::size_t candidateIdx, std::size_t fieldIdx) const;
        };

        JoinBuffer(core::logical::port::Port leftSchema,
                   core::logical::port::Port rightSchema,
                   core::logical::port::Port outputSchema,
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

        void endLeft(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        void endRight(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        void matchLeft(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        void matchRight(core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs, core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        [[nodiscard]] JoinSnapshot* snapshotForCompute(bool callFromLeft) const;

        static void releaseSnapshot(JoinSnapshot* snapshot);

        void enableEqui(const std::vector<std::size_t>& leftFieldIndices,
                        const std::vector<std::size_t>& rightFieldIndices,
                        const std::vector<core::shared::types::ValueTypeId>& keyTypeIds);

        void evict(core::shared::types::Timestamp threshold, bool callFromLeft, bool isFinal);

        static void markCandidateForRemove(JoinSnapshot *snapshot, std::size_t candidateIdx);

        static void markCandidateAsMatch(JoinSnapshot *snapshot, std::size_t candidateIdx);

        static void markAllCandidateAsMatch(JoinSnapshot *snapshot, bool extract);

        void removeMarkedCandidates(const JoinSnapshot *snapshot, bool wasLeft);

        void bufferOutput(const void* ownRowPtr, std::size_t ownRowSize, const void* candRowPtr, std::size_t candRowSize, bool onLeft, core::shared::types::Timestamp startTs, core::shared::types::Timestamp endTs,
            core::shared::types::TiBreak startTb, core::shared::types::TiBreak endTb);

        std::size_t drainOutput(core::shared::types::Timestamp watermark, std::size_t maxCount, std::vector<std::size_t>& outIndices);

        void removeOutputRows(const std::vector<std::size_t>& indices);

        [[nodiscard]] const OutputRow& getOutputRow(std::size_t index) const;

        [[nodiscard]] std::size_t getOutputRowSize() const;

        [[nodiscard]] core::shared::types::Timestamp getMinStartTimestampLeft() const;

        [[nodiscard]] core::shared::types::Timestamp getMaxStartTimestampRight() const;

    private:
        const core::runtime::types::RuntimeTypeRegistry* mRegistry;
        bool mOutOfOrder;
        JoinSide mLeftSide;
        JoinSide mRightSide;
        std::shared_mutex mMutex;

        std::vector<std::size_t> mCandidates;
        bool mAllReleased = false;
        core::shared::types::Timestamp mLastHeartbeat = 0;

        std::vector<OutputRow> mOutputBuffer;
        std::vector<std::size_t> mOutputOrder;
        core::logical::port::Port mOutputSchema;
        std::mutex mOutputMutex;

        void matchAgainst(const JoinSide& sourceSide, JoinSide& otherSide, const std::vector<std::size_t>& sourceKeyFieldIndices, core::shared::types::Timestamp sourceStartTs, core::shared::types::Timestamp sourceEndTs);


        void copyRowIntoSnapshot(const JoinSide& side, std::size_t rowIndex, std::vector<std::byte>& dst, std::size_t dstOffset,
                                 std::vector<std::size_t>& offsetsOut, std::vector<core::shared::types::ValueTypeId>& typeIdsOut,
                                 std::vector<RetainPointer>& retainedOut) const;
    };
}

#endif  // SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_JOIN_RUNTIME_JOIN_BUFFER_HPP