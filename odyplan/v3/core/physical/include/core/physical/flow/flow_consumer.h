#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_CONSUMER_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_CONSUMER_HPP

#include <core/physical/data/data_package.h>
#include <core/physical/segment/segment_context.h>
#include <core/physical/segment/segment_data_consumer.h>
#include <core/shared/identifiers/port_id.h>

#include <algorithm>

namespace sierra::odyplan::v3::core::physical::flow {
    class IConsumer {
    public:
        IConsumer() = default;
        virtual ~IConsumer() = default;
        IConsumer(IConsumer const&) = delete;
        IConsumer& operator=(IConsumer const&) = delete;
        IConsumer(IConsumer&&) = delete;
        IConsumer& operator=(IConsumer&&) = delete;

        virtual void consume(segment::SegmentContext& segmentCtx,
                             shared::identifiers::PortId inputPort,
                             const data::DataPackage& data,
                             const segment::SegmentDataConsumers& consumers) = 0;

        virtual void end(segment::SegmentContext& segmentCtx,
                         shared::identifiers::PortId inputPort,
                         const segment::SegmentDataConsumers& consumers) {}

        virtual void consumePunctuation(segment::SegmentContext& segmentCtx,
            shared::identifiers::PortId inputPort,
            const data::PunctuationPackage& punctuation,
            const segment::SegmentDataConsumers& consumers) {
            for (const auto& consumer : consumers) {
                consumer.consumePunctuation(segmentCtx, punctuation);
            }
        }

        virtual void configureInputPorts(std::vector<shared::identifiers::PortId> connectedPorts) {
            std::ranges::sort(connectedPorts);
            connectedPorts.erase(std::ranges::unique(connectedPorts).begin(), connectedPorts.end());
            mConnectedPorts = std::move(connectedPorts);
        }

    protected:
        [[nodiscard]] std::int64_t compactInputIndex(shared::identifiers::PortId portId) const {
            const auto it = std::ranges::find(mConnectedPorts, portId);
            if (it == mConnectedPorts.end()) {
                throw std::logic_error("IConsumer::compactInputIndex: unconfigured PortId used (" + std::to_string(portId) + ")");
            }
            return static_cast<std::int64_t>(std::distance(mConnectedPorts.begin(), it));
        }

        [[nodiscard]] std::size_t sizeOfConnectedPorts() const noexcept {
            return mConnectedPorts.size();
        }
    private:
        std::vector<shared::identifiers::PortId> mConnectedPorts;
    };
}  // namespace sierra::odyplan::v3::core::physical::flow

#endif  // SIERRA_ODYPLAN_V3_CORE_PHYSICAL_FLOW_FLOW_CONSUMER_HPP