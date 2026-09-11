#include <basic/operators/sink/print/logical/print_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::basic::operators::sink::print::logical {
    PrintOperator::PrintOperator(const details::PrintOperatorOptions options) : mOptions(options) {}

    PrintOperator::PrintOperator(const details::PrintOperatorOptions options, PortFieldIndexV indices) : mPortFieldIndices(std::move(indices)), mOptions(options) {}

    PrintOperator::PrintOperator(const details::PrintOperatorOptions options, PortFieldNameV names) : mPortFieldNames(std::move(names)), mOptions(options) {}

    core::shared::types::OperatorTypeId PrintOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    PrintOperator::PortFieldIndexV PrintOperator::getPortFieldIndices() const {
        return mPortFieldIndices;
    }

    details::PrintOperatorOptions PrintOperator::getOptions() const {
        return mOptions;
    }

    void PrintOperator::validateInput() {
        if (mPortFieldIndices.empty() && mPortFieldNames.empty()) {
            PortFieldIndexV indices;
            const std::size_t size = getInputPorts().front().size();
            indices.reserve(size);

            for (std::size_t i = 0; i < size; ++i) {
                indices.push_back(i);
            }

            mPortFieldIndices = std::move(indices);
            return;
        }

        if (!mPortFieldNames.empty() && mPortFieldIndices.empty()) {
            PortFieldIndexV indices;
            indices.reserve(mPortFieldNames.size());

            const auto& inputPort = getInputPorts().at(0);

            for (const auto& name : mPortFieldNames) {
                auto iter = std::ranges::find_if(
                    inputPort, [&](const core::logical::port::PortDescription& pd) {
                        return pd.fieldName == name &&
                               core::shared::types::isBuiltinValueTypePrintable(pd.valueTypeId);
                    });

                if (iter == inputPort.end()) {
                    throw std::invalid_argument("PrintOperator: name not found in inputPort");
                }

                indices.push_back(static_cast<std::size_t>(std::distance(inputPort.begin(), iter)));
            }

            mPortFieldIndices = std::move(indices);
        }
    }
}  // namespace sierra::odyplan::v3::basic::operators::sink::print::logical