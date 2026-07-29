#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <vector>

#include "core/physical/segment/segment_context.h"

namespace sierra::odyplan::v2::core::physical::port {
    struct PortTuple {
        llvm::Value* timestamp = nullptr;
        std::vector<llvm::Value*> fields;

        [[nodiscard]] std::size_t size() const {
            return fields.size();
        }

        [[nodiscard]] llvm::Value* at(const std::size_t index) const {
            return fields.at(index);
        }

        [[nodiscard]] llvm::Value* operator[](const std::size_t index) const {
            return fields[index];
        }

        [[nodiscard]] auto begin() const {
            return fields.begin();
        }

        [[nodiscard]] auto end() const {
            return fields.end();
        }

        void push_back(llvm::Value* value) {
            fields.push_back(value);
        }
        void reserve(const std::size_t size) {
            fields.reserve(size);
        }

    };

    using PortTupleConsumer = std::function<void(segment::SegmentContext& , const PortTuple&)>;
    using PortTupleConsumers = std::vector<PortTupleConsumer>;
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H