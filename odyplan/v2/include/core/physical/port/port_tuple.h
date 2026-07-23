#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <vector>

#include "core/physical/segment/segment_context.h"

namespace sierra::odyplan::v2::core::physical::port {
    using PortTuple = std::vector<llvm::Value*>;
    using PortTupleConsumer = std::function<void(segment::SegmentContext& , const PortTuple&)>;
    using PortTupleConsumers = std::vector<PortTupleConsumer>;
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H