#include <stdexcept>
#include <basic/operators/pipe/merge/logical/merge_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::merge::logical {
    core::shared::types::OperatorTypeId MergeOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    void MergeOperator::validateInput() {
        const auto &inputPortsRef = getInputPortsRef();
        const auto &outputPortRef = getOutputPortsRef().at(0);

        for (const auto &inputPortRef: inputPortsRef) {
            if (inputPortRef != outputPortRef) {
                throw std::logic_error(
                    "MergeOperator::validate: all InputPorts and the first OutputPort need to be the same");
            }
        }
    }
}