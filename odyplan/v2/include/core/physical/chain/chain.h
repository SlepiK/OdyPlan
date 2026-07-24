#ifndef SIERRA_ODYPLAN_V2_CORE_PHYSICAL_CHAIN_CHAIN_H
#define SIERRA_ODYPLAN_V2_CORE_PHYSICAL_CHAIN_CHAIN_H
#include <vector>

#include "core/physical/operator/operator.h"
#include "core/physical/operator/operator_ability.h"

namespace sierra::odyplan::v2::core::physical::chain {
    struct ChainNode {
        opr::IConsumer* asConsumer = nullptr;
        opr::IProducer* asProducer = nullptr;
        bool isBreaker = false;
    };

    struct Chain {
        std::vector<ChainNode> nodes;
        port::Ports exitSchemas;
    };

}  // namespace sierra::odyplan::v2::core::physical::chain

#endif  // SIERRA_ODYPLAN_V2_CORE_PHYSICAL_CHAIN_CHAIN_H