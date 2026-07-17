#ifndef SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H
#define SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H
#include <any>
#include <cstdint>
#include <vector>

#include "window_config.h"
#include "window_range.h"

namespace sierra::odyplan::v2::common::window {
    using WindowValue = std::any;
    using WindowRow = std::vector<WindowValue>;

    struct BufferedWindowRow {
        WindowRow rows;
        std::uint64_t timestamp; //ToDo or chrono?
    };

    class WindowBuffer {
    public:
        explicit WindowBuffer(WindowConfig config)
            : mConfig(std::move(config)) {
        }

        void begin() {
            mCurrentRow.clear();
        }

        void end(std::uint64_t timestamp) {
            mRows.push_back(BufferedWindowRow{std::move(mCurrentRow), timestamp});
            mCurrentRow.clear();
        }

        void push(core::common::identifier::TypeId typeId, const void* rawData) {

        }


    private:
        WindowConfig mConfig;
        std::vector<BufferedWindowRow> mRows;
        WindowRow mCurrentRow;
        std::vector<WindowRange> mWindows;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_WINDOW_WINDOWBUFFER_H