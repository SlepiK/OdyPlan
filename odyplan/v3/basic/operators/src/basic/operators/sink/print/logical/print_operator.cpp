#include <stdexcept>
#include <basic/operators/sink/print/logical/print_operator.h>

namespace sierra::odyplan::v3::basic::operators::sink::print::logical {
    PrintOperator::PrintOperator(PortFieldIndexV indices)
            : mPortFieldIndices(std::move(indices)) {
    }

    PrintOperator::PrintOperator(PortFieldNameV names)
        : mPortFieldNames(std::move(names)) {
    }

    core::shared::types::OperatorTypeId PrintOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    PrintOperator::PortFieldIndexV PrintOperator::getPortFieldIndices() const {
        return mPortFieldIndices;
    }

    void PrintOperator::validateInput() {
        if (!mPortFieldNames.empty() && mPortFieldIndices.empty()) {
            PortFieldIndexV indices;
            indices.reserve(mPortFieldNames.size());

            const auto &inputPort = getInputPorts().at(0);

            for (const auto &name: mPortFieldNames) {
                auto iter = std::ranges::find_if(inputPort,
                                                 [&](const core::logical::port::PortDescription &pd) {
                                                     return pd.fieldName == name &&
                                                            core::shared::types::isBuiltinValueTypePrintable(static_cast<core::shared::types::BuiltinValueType>(pd.valueTypeId));
                                                 });

                if (iter == inputPort.end()) {
                    throw std::invalid_argument("PrintOperator: name not found in inputPort");
                }

                indices.push_back(static_cast<std::size_t>(std::distance(inputPort.begin(), iter)));
            }

            mPortFieldIndices = std::move(indices);
        }
    }
}