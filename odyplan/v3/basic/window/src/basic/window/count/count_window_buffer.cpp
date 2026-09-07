#include <basic/window/count/count_window_buffer.h>
#include <cassert>

#include "core/physical/time/time_point.h"

namespace sierra::odyplan::v3::basic::window::count {
    CountWindowBuffer::CountWindowBuffer(const CountWindowConfig& config,
                          core::logical::port::Port mDataSchema,
                          const core::runtime::types::RuntimeTypeRegistry& runtimeTypeRegistry)
        : mConfig(config), mRowLayout(mDataSchema, runtimeTypeRegistry),
          mSchema(std::move(mDataSchema)) {
    }

    CountWindowBuffer::~CountWindowBuffer() {
        for (auto& row : mRowBuffer) {
            mRowLayout.releaseRow(row.data);
        }
    }

    void CountWindowBuffer::begin() {
        mRowLayout.begin();
    }

    void CountWindowBuffer::push(const std::uint64_t fieldIndex, const void *rawData) {
        mRowLayout.push(fieldIndex, rawData);
    }

    void CountWindowBuffer::end(const Timestamp startTs, const Timestamp endTs) {
        mRowBuffer.push_back(Row{.data=mRowLayout.pop(), .startTs=startTs, .endTs=endTs});

        if (mRowBuffer.size() == mConfig.size + 1) {
            mReady = true;
            mReadyRowCount = mConfig.size;
            mReadyEnd = mRowBuffer[mConfig.size].startTs;
        }
    }

    bool CountWindowBuffer::ready() const {
        return mReady;
    }

    std::uint64_t CountWindowBuffer::getRowCount() const {
        return mReadyRowCount;
    }

    const std::byte *CountWindowBuffer::getData(const std::uint64_t rowIndex, const std::uint64_t fieldIndex) const {
        return mRowBuffer[rowIndex].data.data() + mRowLayout.offsetOf(fieldIndex);
    }

    ICountWindowBuffer::Timestamp CountWindowBuffer::getRowStart(const std::uint64_t rowIndex) const {
        return mRowBuffer[rowIndex].startTs;
    }

    ICountWindowBuffer::Timestamp CountWindowBuffer::getWindowEnd() const {
        return mReadyEnd;
    }

    void CountWindowBuffer::advanceWindow() {
        const auto n = std::min<std::uint64_t>(mConfig.advance, mRowBuffer.size());
        for (std::size_t i = 0; i < n; ++i) {
            mRowLayout.releaseRow(mRowBuffer.front().data);
            mRowBuffer.pop_front();
        }
        mReady = false;
        mReadyRowCount = 0;
    }

    void CountWindowBuffer::flush() {
        if (mFlushed || mRowBuffer.empty()) {
            mReady = false;
            return;
        }
        mFlushed = true;
        mReady = true;
        mReadyRowCount = mRowBuffer.size();
        mReadyEnd = core::physical::time::INFINITY_SENTINEL;
    }

    void CountWindowBuffer::close() {
        for (auto& row : mRowBuffer) {
            mRowLayout.releaseRow(row.data);
        }
        mRowBuffer.clear();
    }
}  // namespace sierra::odyplan::v3::basic::window::count