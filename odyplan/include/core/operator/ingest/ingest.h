#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_INGEST_INGEST_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_INGEST_INGEST_H

#include "core/port/port.h"

#include <cstddef>

namespace sierra::odyplan::v1::core::opr::ingest {
    template <std::size_t I, port::PortType T>
    class IIngestPort {
    public:
        IIngestPort() = default;
        virtual ~IIngestPort() = default;

        IIngestPort(const IIngestPort&) = delete;
        IIngestPort& operator=(const IIngestPort&) = delete;

        IIngestPort(IIngestPort&&) = delete;
        IIngestPort& operator=(IIngestPort&&) = delete;

        static constexpr std::size_t INDEX = I;
        virtual void ingest(std::integral_constant<std::size_t, I>, const T::Fields& tuple) = 0;
    };

    template <typename Seq, port::PortsType TInx>
    struct IngestPortImpl;

    template <std::size_t... Ix, port::PortType... TInx>
    struct IngestPortImpl<std::index_sequence<Ix...>, port::Ports<TInx...>> {
        struct Type : IIngestPort<Ix, TInx>... {
            Type() = default;
            ~Type() override = default;

            Type(const Type&) = delete;
            Type& operator=(const Type&) = delete;

            Type(Type&&) = delete;
            Type& operator=(Type&&) = delete;

            using IIngestPort<Ix, TInx>::ingest...;
        };
    };

    template <port::PortsType TInx>
    using IngestPorts = IngestPortImpl<std::make_index_sequence<TInx::SIZE>, TInx>::Type;

}  // namespace sierra::odyplan::v1::core::opr::ingest

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_INGEST_INGEST_H