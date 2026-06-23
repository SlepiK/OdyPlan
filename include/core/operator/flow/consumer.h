#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_CONSUMER_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_CONSUMER_H

#include "core/tuple/tuple.h"
#include "emitter.h"

namespace sierra::odyplan::v1::core::opr::flow {
	template<class TOut>
	struct ConsumeContext {
    	EmitterFn<TOut>& emit;
		std::size_t iPort;
	};

    template <tuple::Tuple TIn>
    class IConsumer {
    public:
        virtual void consume(const TIn& previousData, ConsumeContext& ctx) = 0;
    };
}  // namespace sierra::odyplan::v1::core::opr::flow

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_CONSUMER_H