#ifndef SIERRA_ODYPLAN_V2_COMMON_IO_IOSOURCE_H
#define SIERRA_ODYPLAN_V2_COMMON_IO_IOSOURCE_H

#include <optional>
#include "io_data.h"

namespace sierra::odyplan::v2::common::io {
    class IIOSource {
    public:
        virtual ~IIOSource() = default;

        [[nodiscard]] virtual std::optional<IOData> next() = 0;

        virtual void reset() = 0;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_IO_IOSOURCE_H