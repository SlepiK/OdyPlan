#include <basic/operators/pipe/join/physical/details/join_buffer.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::physical::details {
    JoinBuffer::JoinBuffer(core::logical::port::Port leftSchema, core::logical::port::Port rightSchema,
            const core::physical::types::ValueTypeRegistry& typeRegistry, const std::chrono::milliseconds leftWindowSize,
            const std::chrono::milliseconds rightWindowSize)
                : mLeftSide(std::move(leftSchema), typeRegistry, leftWindowSize),
                    mRightSide(std::move(rightSchema), typeRegistry, rightWindowSize) {
        }

        JoinBuffer::~JoinBuffer() {
            assert(mAllReleased && "JoinBuffer: left and right side was never released!");
        }

        void JoinBuffer::close() {
            mLeftSide.release();
            mRightSide.release();
            mAllReleased = true;
        }

        void JoinBuffer::lock() {
            mMutex.lock();
        }

        void JoinBuffer::unlock() {
            mMutex.unlock();
        }

        void JoinBuffer::beginLeft() {
            mLeftSide.begin();
        }

        void JoinBuffer::beginRight() {
            mRightSide.begin();
        }

        void JoinBuffer::pushLeft(const std::size_t index, const void* data) {
            mLeftSide.push(index, data);
        }

        void JoinBuffer::pushRight(const std::size_t index, const void* data) {
            mRightSide.push(index, data);
        }

        void JoinBuffer::endLeft(const Timestamp timestamp) {
            mLeftSide.end(timestamp);
            const auto newAbsIndex = mLeftSide.getLastAbsoluteIndex();

            mCandidates.clear();

            if (mLeftSide.hasKeyIndex() && mRightSide.hasKeyIndex()) {
                //HASH
                //NOLINTNEXTLINE(bugprone-unchecked-optional-access)
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

        void JoinBuffer::endRight(const Timestamp timestamp) {
            mRightSide.end(timestamp);
            const auto newAbsIndex = mRightSide.getLastAbsoluteIndex();

            mCandidates.clear();
            if (mLeftSide.hasKeyIndex() && mRightSide.hasKeyIndex()) {
                //HASH
                //NOLINTNEXTLINE(bugprone-unchecked-optional-access)
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

        void JoinBuffer::evict(const Timestamp timestamp, const bool callFromLeft) {
            if (callFromLeft) {
                mRightSide.evict(timestamp);
            } else {
                mLeftSide.evict(timestamp);
            }
        }

        std::size_t JoinBuffer::getCandidateSize() const {
            return mCandidates.size();
        }

        const std::byte* JoinBuffer::getCandidatePtr(const std::size_t candidateIndex, const std::size_t fieldIndex) const {
            const auto rowIndex = mCandidates.at(candidateIndex);
            return mCandidateOnRight ? mRightSide.getFieldPtr(rowIndex, fieldIndex) : mLeftSide.getFieldPtr(rowIndex, fieldIndex);
        }

        Timestamp JoinBuffer::getCandidateTimestamp(const std::size_t candidateIndex) const {
            const auto rowIndex = mCandidates.at(candidateIndex);
            return mCandidateOnRight ? mRightSide.getTimestamp(rowIndex) : mLeftSide.getTimestamp(rowIndex);
        }

        void JoinBuffer::enableEqui(const std::size_t leftFieldIndex, const std::size_t rightFieldIndex,
            core::physical::types::ValueTypeRegistry::HashFn hashFunction, core::physical::types::ValueTypeRegistry::EqualFn equalFunction) {
            mLeftSide.enableKeyIdx(leftFieldIndex, hashFunction, equalFunction);
            mRightSide.enableKeyIdx(rightFieldIndex, std::move(hashFunction), std::move(equalFunction));
        }
}