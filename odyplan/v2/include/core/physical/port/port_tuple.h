#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H

#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <vector>

namespace sierra::odyplan::v2::core::physical::port {
    using PortTuple = std::vector<llvm::Value*>;
    using PortTupleConsumer = std::function<void(llvm::IRBuilder<>&, const PortTuple&)>;
}  // namespace sierra::odyplan::v2::core::physical::port

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_PORT_PORTTUPLE_H