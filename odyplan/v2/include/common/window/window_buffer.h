#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H
#include <any>
#include <chrono>
#include <cstdint>
#include <vector>

#include "window_config.h"
#include "window_range.h"

namespace sierra::odyplan::v2::common::window {
    class WindowBuffer {
    public:
        explicit WindowBuffer(WindowConfig config, core::logical::port::Port schema, const core::common::type::TypeRegistry& typeRegistry)
            : mConfig(std::move(config)),
                mSchema(std::move(schema)),
                mTypeRegistry(typeRegistry) {
            std::size_t offset = 0;
            mOffset.reserve(mSchema.size());
            for (const auto& field : mSchema) {
                mOffset.push_back(offset);
                offset += mTypeRegistry.getByteSize(field.typeId);
            }
            mRowByteSize = offset;
        }

        void begin() {
            mCurrentRow.assign(mRowByteSize, std::byte{0});
        }

        void end(const std::chrono::sys_time<std::chrono::milliseconds> timestamp) {
            mRows.push_back(BufferedWindowRow{std::move(mCurrentRow), timestamp});
            mCurrentRow.clear();
        }

        void push(const std::size_t index, const void* rawData) {
            const auto typeId = mSchema.at(index).typeId;
            const auto size = mTypeRegistry.getByteSize(typeId);
            std::memcpy(mCurrentRow.data() + mOffset.at(index), rawData, size);
        }

        void compute() {
            mWindows.clear();

            if (mRows.empty()) {
                return;
            }

            std::visit([this](const auto& config) {computeWindows(config);}, mConfig);
        }

        [[nodiscard]] std::size_t getWindowCount() const {
            return mWindows.size();
        }

        [[nodiscard]] std::size_t getRowCountInWindow(const std::size_t index) const {
            return mWindows.at(index).count;
        }

        [[nodiscard]] const std::byte* getData(const std::size_t windowIndex, const std::size_t rowIndex) const {
            const auto& range = mWindows.at(windowIndex);
            return mRows.at(range.start + rowIndex).values.data();
        }

    private:
        struct BufferedWindowRow {
            std::vector<std::byte> values;
            std::chrono::sys_time<std::chrono::milliseconds> timestamp;
        };

        WindowConfig mConfig;
        core::logical::port::Port mSchema;
        const core::common::type::TypeRegistry& mTypeRegistry;

        std::vector<std::size_t> mOffset;
        std::size_t mRowByteSize = 0;

        std::vector<BufferedWindowRow> mRows;
        std::vector<std::byte> mCurrentRow;
        std::vector<WindowRange> mWindows;

        void computeWindows(const CountWindowConfig& config) {
            std::size_t index = 0;
            while (index + config.size <= mRows.size()) {
                mWindows.push_back(WindowRange{index, config.size});
                index += config.slide;
            }
        }

        void computeWindows(const TimeWindowConfig& config) {
            auto wStart = mRows.front().timestamp;
            const auto wEnd = mRows.back().timestamp;

            while (wStart <= wEnd) {
                const auto wCalcEnd = wStart + config.size;
                std::size_t start = SIZE_MAX;
                std::size_t count = 0;

                for (std::size_t i = 0; i < mRows.size(); ++i) {
                    const auto timestamp = mRows[i].timestamp;
                    if (timestamp >= wStart && timestamp < wCalcEnd) {
                        if (start == SIZE_MAX) {
                            start = i;
                        }
                        ++count;
                    }
                }

                if (start != SIZE_MAX) {
                    mWindows.push_back(WindowRange{start, count});
                }

                wStart += config.slide;
            }
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H