#ifndef SIERRA_ODYPLAN_V3_BASIC_IO_IO_SOURCE_HPP
#define SIERRA_ODYPLAN_V3_BASIC_IO_IO_SOURCE_HPP

#include <basic/io/io_data.h>

namespace sierra::odyplan::v3::basic::io {
    class IIOSource {
    public:
        IIOSource() = default;

        virtual ~IIOSource() = default;

        IIOSource(IIOSource const&) = delete;

        IIOSource& operator=(IIOSource const&) = delete;

        IIOSource(IIOSource&&) = delete;

        IIOSource& operator=(IIOSource&&) = delete;

        [[nodiscard]] virtual bool available() = 0;

        [[nodiscard]] virtual const IOData& current() const = 0;

        virtual void reset() = 0;
    };
}  // namespace sierra::odyplan::v3::basic::io

#endif  // SIERRA_ODYPLAN_V3_BASIC_IO_IO_SOURCE_HPP
