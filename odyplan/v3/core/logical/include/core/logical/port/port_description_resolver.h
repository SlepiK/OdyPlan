#ifndef SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_RESOLVER_HPP
#define SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_RESOLVER_HPP

#include <optional>
#include <core/logical/port/port_description.h>
#include <core/logical/port/port.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>


namespace sierra::odyplan::v3::core::logical::port {
    class PortDescriptionResolver {
    public:
        explicit PortDescriptionResolver(Port  port, std::vector<std::size_t> portSizes);

        [[nodiscard]] std::size_t resolve(const std::string& fieldName, const std::optional<std::size_t>& portIndex) const;

        [[nodiscard]] std::size_t getPortCount() const;

        [[nodiscard]] std::size_t getPortSize(std::size_t portIndex) const;

        [[nodiscard]] shared::types::ValueTypeId getTypeId(std::size_t fieldIndex) const;

    private:
        Port mPort;
        std::vector<std::size_t> mPortSizes;
        std::vector<std::size_t> mPortOffsets;
        std::unordered_map<std::string, std::size_t> mFieldIndex;
        std::unordered_set<std::string> mAmbiguousFields;
    };
}  // namespace sierra::odyplan::v3::core::logical::port

#endif  // SIERRA_ODYPLAN_V3_CORE_LOGICAL_PORT_PORT_DESCRIPTION_RESOLVER_HPP