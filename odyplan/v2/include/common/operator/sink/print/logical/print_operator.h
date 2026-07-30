#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_LOGICAL_H

#include <utility>

#include "core/common/identifier/type.h"
#include "core/logical/operator/sink/sink_operator.h"

namespace sierra::odyplan::v2::common::opr::sink::logical {
    class PrintOperator : public core::logical::opr::TypedSinkOperator {
    public:
        using PortFieldIndexV = std::vector<std::size_t>;
        using PortFieldNameV = std::vector<std::string>;

        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::PRINT);

        explicit PrintOperator(PortFieldIndexV indices)
            : mPortFieldIndices(std::move(indices)) {
        }

        explicit PrintOperator(PortFieldNameV names)
            : mPortFieldNames(std::move(names)) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] PortFieldIndexV getPortFieldIndices() const {
            return mPortFieldIndices;
        }

    protected:
        void validateInput() override {
            if (!mPortFieldNames.empty() && mPortFieldIndices.empty()) {
                PortFieldIndexV indices;
                indices.reserve(mPortFieldNames.size());

                const auto &inputPort = getInputPorts().at(0);

                for (const auto &name: mPortFieldNames) {
                    auto iter = std::ranges::find_if(inputPort,
                                                     [&](const core::logical::port::PortDescriptor &pd) {
                                                         return pd.fieldName == name &&
                                                                core::common::identifier::isTypePrintable(pd.typeId);
                                                     });

                    if (iter == inputPort.end()) {
                        throw std::invalid_argument("PrintOperator: name not found in inputPort");
                    }

                    indices.push_back(static_cast<std::size_t>(std::distance(inputPort.begin(), iter)));
                }

                mPortFieldIndices = std::move(indices);
            }
        }

    private:
        PortFieldIndexV mPortFieldIndices;
        PortFieldNameV mPortFieldNames;
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_SINK_PRINT_OPERATOR_LOGICAL_H