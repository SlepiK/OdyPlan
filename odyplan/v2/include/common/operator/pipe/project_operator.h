#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECTOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECTOPERATOR_H

#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"
#include "core/common/port/port_ref.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    //ToDo: Mapping from Odysseus-Operator Project (https://wiki.odysseus.informatik.uni-oldenburg.de/spaces/ODYSSEUS/pages/7111207/Project+operator)
    namespace logical {
        class ProjectOperator : public core::logical::opr::TypedPipeOperator {
        public:
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = 20;

            ProjectOperator(core::logical::port::Port inputPort, core::logical::port::Port outputPort)
                : TypedPipeOperator(core::logical::port::Ports{std::move(inputPort)},core::logical::port::Ports{std::move(outputPort)}) {
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getTypeId() const override {
                return TYPE_ID;
            }
        };
    }

    namespace physical {
        class ProjectOperator : public core::physical::opr::TypedPipeOperator{
        public:
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECTOPERATOR_H