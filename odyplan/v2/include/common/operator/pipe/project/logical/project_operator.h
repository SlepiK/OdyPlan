#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECT_OPERATOR_LOGICAL_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECT_OPERATOR_LOGICAL_H

#include <string>
#include <utility>
#include <vector>
#include "core/logical/operator/pipe/pipe_operator.h"

namespace sierra::odyplan::v2::common::opr::pipe::logical {
    class ProjectOperator : public core::logical::opr::TypedPipeOperator {
    public:
        using IndexMapping = std::vector<std::size_t>;
        using NameMapping = std::vector<std::string>;
        static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(
            core::common::identifier::Operator::PROJECT);

        explicit ProjectOperator(IndexMapping mapping)
            : mMapping(std::move(mapping)) {
        }

        explicit ProjectOperator(NameMapping mapping)
            : mNameMapping(std::move(mapping)) {
        }

        [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
            return TYPE_ID;
        }

        [[nodiscard]] const IndexMapping &getMapping() const {
            return mMapping;
        }

    protected:
        void validateInput() override {
            if (!mNameMapping.empty() && mMapping.empty()) {
                mMapping = resolveNameMapping(getInputPorts().at(0), mNameMapping);
            }
            if (mMapping.empty()) {
                throw std::invalid_argument("ProjectOperator: mapping is empty");
            }

            core::logical::port::Port output;
            auto &outputRef = getOutputPortsRef().at(0);
            for (const auto &index: mMapping) {
                if (index >= outputRef.size()) {
                    throw std::invalid_argument("ProjectOperator: index out of range");
                }

                output.push_back(std::move(outputRef.at(index)));
            }

            outputRef = std::move(output);

            if (mMapping.size() != getOutputPorts().at(0).size()) {
                throw std::invalid_argument("ProjectOperator: mapping size must match outputPort");
            }
        }

    private:
        IndexMapping mMapping;
        NameMapping mNameMapping;

        static IndexMapping resolveNameMapping(const core::logical::port::Port &inputPort,
                                               const NameMapping &nameMapping) {
            IndexMapping mapping;
            mapping.reserve(nameMapping.size());

            for (const auto &name: nameMapping) {
                auto iter = std::ranges::find_if(inputPort,
                                                 [&](const core::logical::port::PortDescriptor &pd) {
                                                     return pd.fieldName == name;
                                                 });
                if (iter == inputPort.end()) {
                    throw std::invalid_argument("ProjectOperator: name not found in inputOperator");
                }
                mapping.push_back(static_cast<std::size_t>(std::distance(inputPort.begin(), iter)));
            }

            return mapping;
        }
    };
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECT_OPERATOR_LOGICAL_H