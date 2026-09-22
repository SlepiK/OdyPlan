#ifndef SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPE_RUNTIME_HPP
#define SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPE_RUNTIME_HPP

#include <core/runtime/types/runtime_type_registry.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
        void* odyCreateRtTypeRegistryExt();

        void odyDestroyRtTypeRegistryExt(types::RuntimeTypeRegistry* registry);

        void* rtTypeRegistryRetain(const void *registry, shared::types::ValueTypeId typeId, void *ptr);

        void rtTypeRegistryRelease(const void* registry, shared::types::ValueTypeId typeId, void* ptr);
    }
}  // namespace sierra::odyplan::v3::core::runtime

#endif  // SIERRA_ODYPLAN_V3_CORE_RUNTIME_TYPE_RUNTIME_HPP