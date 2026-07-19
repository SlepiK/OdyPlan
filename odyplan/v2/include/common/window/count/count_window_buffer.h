#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWBUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWBUFFER_H

#include <deque>
#include <vector>
#include <common/window/window_buffer.h>
#include <common/window/window_row_layout.h>
#include <common/window/count/count_window_config.h>
#include "common/window/window_range.h"

namespace sierra::odyplan::v2::common::window::count {
    class CountWindowBuffer final : public IWindowBuffer {
    public:
        CountWindowBuffer(const CountWindowConfig &config, core::logical::port::Port mDataSchema, const core::common::type::TypeRegistry& typeRegistry)
            : mConfig(config),
                mRowLayout(std::move(mDataSchema), typeRegistry) {
        }

        ~CountWindowBuffer() override {
            assert(mClosed && "CountWindowBuffer::close() was never called");
        }

        CountWindowBuffer(CountWindowBuffer const&) = delete;
        CountWindowBuffer& operator =(CountWindowBuffer const&) = delete;
        CountWindowBuffer(CountWindowBuffer&&) = delete;
        CountWindowBuffer& operator=(CountWindowBuffer&&) = delete;

        void begin() override {
            mRowLayout.begin();
        }

        void push(std::size_t index, const void *rawData) override {
            mRowLayout.push(index, rawData);
        }

        void end(std::chrono::sys_time<std::chrono::milliseconds>  /*timestamp*/) override {
            mRows.push_back(mRowLayout.pop());
            ++mTotalPushed;
        }

        void compute() override {
            evict();
            mWindows.clear();

            while (mNextWindowStart + mConfig.size <= mTotalPushed) {
                mWindows.push_back(WindowRange{mNextWindowStart, mConfig.size});
                mNextWindowStart += mConfig.slide;
            }
        }

        void flush() override {
            evict();
            mWindows.clear();

            if (const std::uint64_t remaining = mTotalPushed - mNextWindowStart; remaining > 0) {
                mWindows.push_back(WindowRange{.start=mNextWindowStart, .count=remaining});
                mNextWindowStart = mTotalPushed;
            }
        }

        [[nodiscard]] std::size_t getWindowCount() const override {
            return mWindows.size();
        }

        [[nodiscard]] std::size_t getRowCountInWindow(const std::size_t index) const override {
            return mWindows.at(index).count;
        }

        [[nodiscard]] const std::byte* getData(const std::size_t windowIndex, const std::size_t rowIndex, const std::size_t fieldIndex) const override {
            const auto& range = mWindows.at(windowIndex);
            const auto relIndex = range.start - mBaseIndex + rowIndex;
            return mRows.at(relIndex).data() + mRowLayout.offsetOf(fieldIndex);
        }

        void close() override {
            for (auto& row : mRows) {
                mRowLayout.releaseRow(row);
            }

            mRows.clear();
            mClosed = true;
        }

    private:
        CountWindowConfig mConfig;
        WindowRowLayout mRowLayout;

        std::deque<std::vector<std::byte>> mRows;
        std::vector<WindowRange> mWindows;

        std::uint64_t mBaseIndex = 0;
        std::uint64_t mTotalPushed = 0;
        std::uint64_t mNextWindowStart = 0;
        bool mClosed = false;

        void evict() {
            while (mBaseIndex < mNextWindowStart && !mRows.empty()) {
                mRowLayout.releaseRow(mRows.front());
                mRows.pop_front();
                ++mBaseIndex;
            }
        }

    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_COUNT_WINDOWBUFFER_H