#ifndef SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H
#define SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "port_value.h"

namespace sierra::odyplan::v2::core::logical::port {
    class Port {
    public:
        Port() = default;

        void push(std::unique_ptr<PortValueBase> value) {
            assert(value);
            values.push_back(std::move(value));
        }

        std::size_t size() const {
            return values.size();
        }

        bool empty() const {
            return values.empty();
        }

        const PortValueBase* get(const std::size_t index) const {
            assert(index < values.size());
            return values.at(index).get();
        }

        PortValueBase* get(const std::size_t index) {
            assert(index < values.size());
            return values.at(index).get();
        }

        std::uint32_t getStableId(const std::size_t index) const {
            return get(index)->getStableId();
        }

        template <typename T>
        const T* getAs(std::size_t index) const {
            return cast<T>(get(index));
        }

        template <typename T>
        T* getAs(std::size_t index) {
            return cast<T>(get(index));
        }

        template <typename T>
        const T* tryGetAs(std::size_t index) {
            return dCast<T>(get(index));
        }

        bool isCompatible(const Port& other) const {
            if (values.size() != other.values.size()) {
                return false;
            }
            for (std::size_t i = 0; i < values.size(); ++i) {
                if (values.at(i)->getStableId() != other.values.at(i)->getStableId()) {
                    return false;
                }
            }
            return true;
        }

        void assignFrom(const Port& other) {
            values.clear();
            values.reserve(other.values.size());
            for (const auto& value : other.values) {
                values.push_back(std::move(value->clone()));
            }
        }

        auto begin() const {
            return values.cbegin();
        }

        auto end() const {
            return values.cend();
        }


    private:
        std::vector<std::unique_ptr<PortValueBase>> values;
    };
}  // namespace sierra::odyplan::v2::core::logical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_LOGICAL_PORT_PORT_H