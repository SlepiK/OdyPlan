#ifndef SIERRA_ODYPLAN_V2_COMMON_RUNTIME_IO_IOTASK_H
#define SIERRA_ODYPLAN_V2_COMMON_RUNTIME_IO_IOTASK_H

#include "core/runtime/thread/iopool.h"

inline sierra::odyplan::v2::core::runtime::thread::IOPool gIOPool;

using IOTaskHandle = std::uint64_t;

extern "C" {
    inline IOTaskHandle rtIOTaskSpawn(void (*fn)()) {
        return gIOPool.submitTask([fn] {fn();});
    }

    inline void rtIOTaskJoin(IOTaskHandle ioTaskHandle) {
        gIOPool.joinTask(ioTaskHandle);
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_RUNTIME_IO_IOTASK_H