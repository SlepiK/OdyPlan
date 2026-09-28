#include "basic/io/io_registry.h"

#include <basic/operators/sink/write/runtime/write_runtime.h>

namespace sierra::odyplan::v3::basic::operators::sink::write::runtime {
    extern "C" {
    void*
    rtIOSinkCreate(const void* /*typeRegistryRawPtr*/, io::IOFormatType ioFormat, const char* path) {
        static const io::IORegistry ioRegistry = io::IORegistry::withBuiltins();
        const nlohmann::json config = {{"path", path}};
        auto sink = ioRegistry.createSinkIO(
            static_cast<sierra::odyplan::v3::basic::io::IOFormatType>(ioFormat), config);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        return sink.release();
    }

    void rtIOSinkDestroy(void* state) {
        const auto* sink = static_cast<io::IIOSink*>(state);
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        delete sink;
    }

    void rtIOSinkBeginData(void* state) {
        static_cast<io::IIOSink*>(state)->begin();
    }

    void rtIOSinkWrite(void* state,
                       const core::shared::types::ValueTypeId typeId,
                       const char* key,
                       const void* value) {
        static_cast<io::IIOSink*>(state)->write(key, typeId, value);
    }

    void rtIOSinkEndData(void* state) {
        static_cast<io::IIOSink*>(state)->end();
    }

    void rtIOSinkFlush(void* state) {
        static_cast<io::IIOSink*>(state)->flush();
    }
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::write::runtime
