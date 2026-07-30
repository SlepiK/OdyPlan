#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_BUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_BUFFER_H
#include <chrono>
#include <mutex>

#include "interval_join_side.h"

namespace sierra::odyplan::v2::common::opr::pipe::physical::internal {
    class IntervalJoinBuffer {
    public:
        IntervalJoinBuffer(core::logical::port::Port leftSchema, core::logical::port::Port rightSchema,
            const core::common::type::TypeRegistry& typeRegistry, const std::chrono::milliseconds leftWindowSize,
            const std::chrono::milliseconds rightWindowSize)
                : mLeftSide(std::move(leftSchema), typeRegistry, leftWindowSize),
                    mRightSide(std::move(rightSchema), typeRegistry, rightWindowSize) {
        }

        ~IntervalJoinBuffer() {
            assert(mAllReleased && "IntervalJoinBuffer: left and right side was never released!");
        }

        void close() {
            mLeftSide.release();
            mRightSide.release();
            mAllReleased = true;
        }

        void lock() {
            mMutex.lock();
        }

        void unlock() {
            mMutex.unlock();
        }

        void beginLeft() {
            mLeftSide.begin();
        }

        void beginRight() {
            mRightSide.begin();
        }

        void pushLeft(const std::size_t index, const void* data) {
            mLeftSide.push(index, data);
        }

        void pushRight(const std::size_t index, const void* data) {
            mRightSide.push(index, data);
        }

        void endLeft(const Timestamp timestamp) {
            mLeftSide.end(timestamp);
            const auto newAbsIndex = mLeftSide.getLastAbsoluteIndex();

            mCandidates.clear();

            if (mLeftSide.hasKeyIndex() && mRightSide.hasKeyIndex()) {
                //HASH
                const auto* key = mLeftSide.getFieldPtr(newAbsIndex, *mLeftSide.getKeyFieldIndex());
                for (const auto candidate : mRightSide.hashLookup(key)) {
                    if (mRightSide.isOver(timestamp, mLeftSide.getWindowSize(), candidate)) {
                        mCandidates.push_back(candidate);
                    }
                }
            } else {
                //DEFAULT
                const auto size = mRightSide.getSize();
                const auto base = mRightSide.getBaseIndex();
                for (std::size_t i = base; i < base + size; ++i) {
                    if (mRightSide.isOver(timestamp, mLeftSide.getWindowSize(), i)) {
                        mCandidates.push_back(i);
                    }
                }
            }

            mCandidateOnRight = true;
        }

        void endRight(const Timestamp timestamp) {
            mRightSide.end(timestamp);
            const auto newAbsIndex = mRightSide.getLastAbsoluteIndex();

            mCandidates.clear();
            if (mLeftSide.hasKeyIndex() && mRightSide.hasKeyIndex()) {
                //HASH
                const auto* key = mRightSide.getFieldPtr(newAbsIndex, *mRightSide.getKeyFieldIndex());
                for (const auto candidate : mLeftSide.hashLookup(key)) {
                    if (mLeftSide.isOver(timestamp, mRightSide.getWindowSize(), candidate)) {
                        mCandidates.push_back(candidate);
                    }
                }
            } else {
                //DEFAULT
                const auto size = mLeftSide.getSize();
                const auto base = mLeftSide.getBaseIndex();
                for (std::size_t i = base; i < base + size; ++i) {
                    if (mLeftSide.isOver(timestamp, mRightSide.getWindowSize(), i)) {
                        mCandidates.push_back(i);
                    }
                }

            }

            mCandidateOnRight = false;
        }

        void evict(const Timestamp timestamp, const bool callFromLeft) {
            if (callFromLeft) {
                mRightSide.evict(timestamp);
            } else {
                mLeftSide.evict(timestamp);
            }
        }

        [[nodiscard]] std::size_t getCandidateSize() const {
            return mCandidates.size();
        }

        [[nodiscard]] const std::byte* getCandidatePtr(const std::size_t candidateIndex, const std::size_t fieldIndex) const {
            const auto rowIndex = mCandidates.at(candidateIndex);
            return mCandidateOnRight ? mRightSide.getFieldPtr(rowIndex, fieldIndex) : mLeftSide.getFieldPtr(rowIndex, fieldIndex);
        }

        [[nodiscard]] Timestamp getCandidateTimestamp(const std::size_t candidateIndex) const {
            const auto rowIndex = mCandidates.at(candidateIndex);
            return mCandidateOnRight ? mRightSide.getTimestamp(rowIndex) : mLeftSide.getTimestamp(rowIndex);
        }

        void enableEqui(const std::size_t leftFieldIndex, const std::size_t rightFieldIndex,
            core::common::type::TypeRegistry::HashFn hashFunction, core::common::type::TypeRegistry::EqualFn equalFunction) {
            mLeftSide.enableKeyIdx(leftFieldIndex, hashFunction, equalFunction);
            mRightSide.enableKeyIdx(rightFieldIndex, std::move(hashFunction), std::move(equalFunction));
        }
    private:
        IntervalJoinSide mLeftSide;
        IntervalJoinSide mRightSide;
        std::mutex mMutex;

        std::vector<std::size_t> mCandidates;
        bool mCandidateOnRight = false;
        bool mAllReleased = false;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_INTERVAL_JOIN_OPERATOR_PHYSICAL_INTERNAL_BUFFER_H