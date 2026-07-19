#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_IOSOURCE_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_IOSOURCE_H

#include <cstdint>
#include "io_data.h"

namespace sierra::odyplan::v2::common::io {
    using IOSourceId = std::uint64_t;

    class IIOSource {
    public:
        IIOSource() = default;
        virtual ~IIOSource() = default;
        IIOSource(IIOSource const&) = delete;
        IIOSource& operator =(IIOSource const&) = delete;
        IIOSource(IIOSource&&) = delete;
        IIOSource& operator=(IIOSource&&) = delete;

        [[nodiscard]] virtual bool available() = 0;

        [[nodiscard]] virtual const IOData& current() const = 0;

        virtual void reset() = 0;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_IOSOURCE_H