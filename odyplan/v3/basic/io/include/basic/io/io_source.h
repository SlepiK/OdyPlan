#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_IO_SOURCE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_IO_SOURCE_HPP

#include <cstdint>
#include <basic/io/io_data.h>

namespace sierra::odyplan::v3::basic::io {
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

#endif //SIERRA_ODYPLAN_V3_BASIC_IO_IO_SOURCE_HPP