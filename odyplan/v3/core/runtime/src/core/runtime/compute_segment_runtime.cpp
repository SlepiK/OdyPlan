#include <core/runtime/compute_segment_runtime.h>

namespace sierra::odyplan::v3::core::runtime {
    extern "C" {
        void* rtComputeSegmentNullInit(const void* /*typeRegistry*/) {
            return nullptr;
        }

        void* rtComputeSegmentGetOrCreate(void** slot, void (*processFn)(void*)) {
            const std::atomic_ref<void*> atomicSlot(*slot);

            if (void* existing = atomicSlot.load(std::memory_order_acquire); existing != nullptr) {
                return existing;
            }

            static std::mutex initMutex;
            const std::scoped_lock lock(initMutex);

            if (void* existing = atomicSlot.load(std::memory_order_acquire); existing != nullptr) {
                return existing;
            }

            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto* segment = new task::ComputeSegment();
            segment->worker = std::thread([segment, processFn] {
                while (true) {
                    auto item = segment->queue.pop();
                    if (!item.has_value()) {
                        break;
                    }
                    auto* envelope = static_cast<task::Envelope*>(*item);
                    envelope->fn(envelope->payload);
                    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                    delete envelope;
                }
            });

            atomicSlot.store(segment, std::memory_order_release);
            return segment;
        }

        void rtComputeSegmentEnqueue(void* segmentHandle, void* payload) {
            static_cast<task::ComputeSegment*>(segmentHandle)->queue.push(payload);
        }

        void rtComputeSegmentDestroyIfStarted(void* segmentHandle) {
            if (segmentHandle == nullptr) {
                return;
            }
            auto* segment = static_cast<task::ComputeSegment*>(segmentHandle);
            segment->queue.close();
            if (segment->worker.joinable()) {
                segment->worker.join();
            }
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete segment;
        }

        void rtComputeSegmentDrain(void *segmentHandle) {
            if (segmentHandle == nullptr) {
                return;
            }
            auto* segment = static_cast<task::ComputeSegment*>(segmentHandle);
            segment->queue.close();
            segment->worker.join();
        }
    }
}