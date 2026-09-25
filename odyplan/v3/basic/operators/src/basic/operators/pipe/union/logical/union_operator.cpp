#include <basic/operators/pipe/union/logical/union_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::basic::operators::pipe::union_::logical {
    core::shared::types::OperatorTypeId UnionOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    void UnionOperator::validateInput() {
        const auto& inputPortsRef = getInputPortsRef();
        const auto& outputPortRef = getOutputPortsRef().at(0);

        for (const auto& inputPortRef : inputPortsRef) {
            if (inputPortRef != outputPortRef) {
                throw std::logic_error("UnionOperator::validate: all InputPorts and the first "
                                       "OutputPort need to be the same");
            }
        }
    }

    bool UnionOperator::getOutOfOrder() const {
        return mOutOfOrder;
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::merge::logical