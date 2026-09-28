#include <core/logical/port/port_description_resolver.h>

#include <utility>

namespace sierra::odyplan::v3::core::logical::port {
    PortDescriptionResolver::PortDescriptionResolver(Port  port, std::vector<std::size_t> portSizes) : mPort(std::move(port)), mPortSizes(std::move(portSizes)) {
        mPortOffsets.reserve(mPort.size());

        //collect offsets for multi-port
        std::size_t offset = 0;
        for (const std::size_t size : mPortSizes) {
            mPortOffsets.push_back(offset);
            offset += size;
        }

        //check for unique field names
        for (std::size_t i = 0; i < mPort.size(); ++i) {
            const auto& fieldName = mPort.at(i).fieldName;
            if (mFieldIndex.contains(fieldName)) {
                mAmbiguousFields.insert(fieldName);
            } else {
                mFieldIndex[fieldName] = i;
            }
        }
    }

    std::size_t PortDescriptionResolver::resolve(const std::string &fieldName, const std::optional<std::size_t>& portIndex) const {
        if (portIndex.has_value()) {
            if (*portIndex >= mPortSizes.size()) {
                throw std::invalid_argument("invalid portIndex");
            }

            const std::size_t portStart = mPortOffsets.at(*portIndex);
            const std::size_t portEnd = portStart + mPortSizes.at(*portIndex);

            for (std::size_t i = portStart; i < portEnd; ++i) {
                if (mPort.at(i).fieldName == fieldName) {
                    return i;
                }
            }
            throw std::invalid_argument("fieldName not found in port " + std::to_string(*portIndex));
        }

        const auto it = mFieldIndex.find(fieldName);
        if (it == mFieldIndex.end()) {
            throw std::invalid_argument("invalid fieldName");
        }

        if (mAmbiguousFields.contains(fieldName)) {
            throw std::invalid_argument("ambiguous fieldName");
        }

        return it->second;
    }

    std::size_t PortDescriptionResolver::getPortCount() const {
        return mPortSizes.size();
    }

    std::size_t PortDescriptionResolver::getPortSize(const std::size_t portIndex) const {
        if (portIndex >= mPortSizes.size()) {
            throw std::out_of_range("invalid portIndex");
        }
        return mPortSizes.at(portIndex);
    }

    shared::types::ValueTypeId PortDescriptionResolver::getTypeId(const std::size_t fieldIndex) const {
        if (fieldIndex >= mPort.size()) {
            throw std::out_of_range("invalid fieldIndex");
        }
        return mPort.at(fieldIndex).valueTypeId;
    }
}
