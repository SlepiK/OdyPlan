#include <basic/operators/sink/write/runtime/write_runtime.h>

namespace sierra::odyplan::v3::basic::operators::sink::write::runtime {
    extern "C" {
        void rtIOSinkBeginData(const io::IOSinkId index) {
            gSinks.at(index)->begin();
        }

        void rtIOSinkWrite(const io::IOSinkId index, const core::shared::types::ValueTypeId typeId, const char* key, const void* value) {
            gSinks.at(index)->write(key, typeId, value);
        }

        void rtIOSinkEndData(const io::IOSinkId index) {
            gSinks.at(index)->end();
        }

        void rtIOSinkFlush(const io::IOSinkId index) {
            gSinks.at(index)->flush();
        }

        void rtIOSinkFlushAll() {
            for (const auto& sink : gSinks) {
                sink->flush();
            }
        }
    }
}