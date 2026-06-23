#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H

#include "core/tuple/tuple.h"

#include <functional>

namespace sierra::odyplan::v1::core::opr::flow {
	template<class T>
	using EmitterFn = void(*)(const T&, void* state);
}  // namespace sierra::odyplan::v1::core::opr::flow

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_FLOW_EMITTER_H