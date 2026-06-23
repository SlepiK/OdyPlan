#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_INGEST_INGEST_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_INGEST_INGEST_H

#include "core/tuple/tuple.h"

#include <cstddef>

namespace sierra::odyplan::v1::core::opr::ingest {
    // ToDo: Clean later
    template <std::size_t I, tuple::Tuple T>
    class IIngestPort {
    public:
        IIngestPort() = default;
        virtual ~IIngestPort() = default;

        IIngestPort(const IIngestPort&) = delete;
        IIngestPort& operator=(const IIngestPort&) = delete;

        IIngestPort(IIngestPort&&) = delete;
        IIngestPort& operator=(IIngestPort&&) = delete;

        static constexpr std::size_t INDEX = I;
        virtual void ingest(std::integral_constant<std::size_t, I>, const T& tuple) = 0;
    };

    template <typename Seq, tuple::Tuple... TInx>
    struct IngestPortImpl;

    template <std::size_t... Ix, tuple::Tuple... TInx>
    struct IngestPortImpl<std::index_sequence<Ix...>, TInx...> {
        struct Type : public IIngestPort<Ix, TInx>... {
            Type() = default;
            virtual ~Type() = default;

            Type(const Type&) = delete;
            Type& operator=(const Type&) = delete;

            Type(Type&&) = delete;
            Type& operator=(Type&&) = delete;

            using IIngestPort<Ix, TInx>::ingest...;
        };
    };

    template <tuple::Tuple... TInx>
    using IngestPorts = typename IngestPortImpl<std::index_sequence_for<TInx...>, TInx...>::Type;

}  // namespace sierra::odyplan::v1::core::opr::ingest

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_INGEST_INGEST_H