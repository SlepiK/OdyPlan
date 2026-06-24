#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H

#include <core/operator/ingest/ingest.h>

#include "core/tuple/tuple.h"

namespace sierra::odyplan::v1::core::opr::flow {
    class IEmitter {
    public:
        IEmitter() = default;

        virtual ~IEmitter() = default;

        IEmitter(const IEmitter&) = delete;
        IEmitter& operator=(const IEmitter&) = delete;

        IEmitter(IEmitter&&) = delete;
        IEmitter& operator=(IEmitter&&) = delete;
    };

    template <tuple::Tuple TOut>
    class Emitter : public IEmitter {
    public:
        Emitter() = default;

        ~Emitter() override = default;

        Emitter(const Emitter&) = delete;
        Emitter& operator=(const Emitter&) = delete;

        Emitter(Emitter&&) = delete;
        Emitter& operator=(Emitter&&) = delete;

        virtual void emit(const TOut& tuple) = 0;
    };

    template <tuple::Tuple T, std::size_t TargetPort>
    class BoundEmitter final : public Emitter<T> {
    public:
        explicit BoundEmitter(ingest::IIngestPort<TargetPort, T>* target) : mTarget(target) {}

        void emit(const T& tuple) override {
            mTarget->ingest(std::integral_constant<std::size_t, TargetPort>{}, tuple);
        }

    private:
        ingest::IIngestPort<TargetPort, T>* mTarget;
    };

    template <tuple::Tuple TOut>
    class IEmitting {
    public:
        IEmitting() = default;
        virtual ~IEmitting() = default;

        IEmitting(const IEmitting&) = delete;
        IEmitting& operator=(const IEmitting&) = delete;

        IEmitting(IEmitting&&) = delete;
        IEmitting& operator=(IEmitting&&) = delete;

        virtual void setEmitter(Emitter<TOut>* emitter) = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr::flow

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H