#include <core/physical/flow/flow_producer.h>

namespace sierra::odyplan::v3::core::physical::flow {
    bool IProducer::isComputeBound() const {
        return false;
    }
}