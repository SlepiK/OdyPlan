#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECTOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECTOPERATOR_H

#include <string>
#include <vector>

#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"
#include "core/common/port/port_ref.h"
#include "core/common/expression/expression.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    //ToDo: Mapping from Odysseus-Operator Project (https://wiki.odysseus.informatik.uni-oldenburg.de/spaces/ODYSSEUS/pages/7111226/Select+operator)
    namespace logical {
        class SelectOperator : public core::logical::opr::TypedPipeOperator {
        public:
            static constexpr core::common::type::TypeId TYPE_ID = 21;

            explicit SelectOperator(std::unique_ptr<core::common::expression::IExpression> predicate)
                : mPredicate(std::move(predicate)) {
            }

            [[nodiscard]] core::common::type::TypeId getTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const core::common::expression::IExpression& getPredicate() const {
                return *mPredicate;
            }

        protected:
            void validate() override {
                if (mPredicate == nullptr) {
                    throw std::invalid_argument("SelectOperator: predicate must be non-null");
                }
            }

        private:
            std::unique_ptr<core::common::expression::IExpression> mPredicate;
        };
    }

    namespace physical {
        class SelectOperator : public core::physical::opr::TypedPipeOperator {
        public:
            SelectOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort)
                : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)}, core::physical::port::Ports{std::move(outputPort)}) {
            }

            void consume(llvm::IRBuilder<> &builder, const core::physical::port::PortTuple &tuple, core::physical::port::PortTupleConsumer consumer) override {

            }
        private:
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_SELECTOPERATOR_H