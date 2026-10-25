#include <algorithm>
#include <cassert>
#include <basic/operators/pipe/window/runtime/count_window_buffer.h>
#include <core/physical/time/time_point.h>

namespace sierra::odyplan::v3::basic::operators::pipe::window::runtime {
    CountWindowBuffer::CountWindowBuffer(const details::CountWindowOptions& config,
                          core::logical::port::Port mDataSchema)
        : mConfig(config), mRowLayout(mDataSchema),
          mSchema(std::move(mDataSchema)) {
        if (mConfig.advance == 0) {
            mConfig.advance = 1;
        }
        assert(mConfig.size >= mConfig.getEffectiveAdvance() && "Sry, Size < Advance is currently not implemented");
    }

    CountWindowBuffer::~CountWindowBuffer() {
        for (auto& row : mRowBuffer) {
            mRowLayout.releaseRow(row.data);
        }
    }

    void CountWindowBuffer::begin() {
        mRowLayout.begin();
    }

    void CountWindowBuffer::push(const std::size_t fieldIndex, const void *rawData) {
        mRowLayout.push(fieldIndex, rawData);
    }

    void CountWindowBuffer::end(const core::shared::types::Timestamp startTs, const core::shared::types::Timestamp endTs) {
        mRowBuffer.push_back(Row{.data=mRowLayout.pop(), .startTs=startTs, .endTs=endTs});

        if (mRowBuffer.size() == mConfig.size + 1) {
            mReady = true;
            mReadyRowCount = mConfig.size;
            mReadyEnd = mRowBuffer[mConfig.size].startTs;
            mReadyEmitCount = mConfig.getEffectiveAdvance();

            if (mConfig.usesSlideParam()) {
                mFirstElementStart = mRowBuffer.front().startTs;
            }
        }
    }

    bool CountWindowBuffer::ready() const {
        return mReady;
    }

    std::size_t CountWindowBuffer::getRowCount() const {
        return mReadyRowCount;
    }

    const std::byte *CountWindowBuffer::getData(const std::size_t rowIndex, const std::size_t fieldIndex) const {
        return mRowBuffer[rowIndex].data.data() + mRowLayout.offsetOf(fieldIndex);
    }

    core::shared::types::Timestamp CountWindowBuffer::getRowStart(const std::size_t rowIndex) const {
        if (mConfig.usesSlideParam() && mFirstElementStart.has_value()) {
            return *mFirstElementStart;
        }
        return mRowBuffer[rowIndex].startTs;
    }

    core::shared::types::Timestamp CountWindowBuffer::getWindowEnd() const {
        return mReadyEnd;
    }

    std::size_t CountWindowBuffer::getEmitCount() const {
        return mReady ? mReadyEmitCount : 0;
    }

    void CountWindowBuffer::advanceWindow() {
        const auto n = std::min<std::uint64_t>(mConfig.getEffectiveAdvance(), mRowBuffer.size());
        for (std::size_t i = 0; i < n; ++i) {
            mRowLayout.releaseRow(mRowBuffer.front().data);
            mRowBuffer.pop_front();
        }
        mReady = false;
        mReadyRowCount = 0;
        mReadyEmitCount = 0;
        mFirstElementStart.reset();
    }

    void CountWindowBuffer::close() {
        for (auto& row : mRowBuffer) {
            mRowLayout.releaseRow(row.data);
        }
        mRowBuffer.clear();
    }

    core::shared::types::Timestamp CountWindowBuffer::getMinTs() const {
        if (mRowBuffer.empty()) {
            return static_cast<core::shared::types::Timestamp>(std::numeric_limits<int64_t>::max());
        }

        return mRowBuffer.front().startTs;
    }
}  // namespace sierra::odyplan::v3::basic::window::count