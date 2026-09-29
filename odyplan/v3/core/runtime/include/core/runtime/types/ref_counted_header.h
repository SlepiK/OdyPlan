#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_REF_COUNTED_HEADER_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_REF_COUNTED_HEADER_HPP

#include <atomic>
#include <cstdint>
#include <cstddef>

#include "core/shared/types/value_type_id.h"

namespace sierra::odyplan::v3::core::runtime::types {
    struct RefCountedHeader {
        std::atomic<std::uint32_t> refCount;
        shared::types::ValueTypeId typeId;
    };

    inline RefCountedHeader* getRefHeader(void* data) {
        return reinterpret_cast<RefCountedHeader*>(static_cast<char*>(data) - sizeof(RefCountedHeader));
    }

    inline void* getRefData(RefCountedHeader* header) {
        return reinterpret_cast<char*>(header) + sizeof(RefCountedHeader);
    }
}

#endif //SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPES_RUNTIME_REF_COUNTED_HEADER_HPP