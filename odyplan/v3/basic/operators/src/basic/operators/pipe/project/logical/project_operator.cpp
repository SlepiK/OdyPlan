#include <algorithm>
#include <basic/operators/pipe/project/logical/project_operator.h>
#include <stdexcept>

namespace sierra::odyplan::v3::basic::operators::pipe::project::logical {
    ProjectOperator::ProjectOperator(IndexMapping mapping) : mMapping(std::move(mapping)) {}

    ProjectOperator::ProjectOperator(NameMapping mapping) : mNameMapping(std::move(mapping)) {}

    [[nodiscard]] core::shared::types::OperatorTypeId ProjectOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    [[nodiscard]] const ProjectOperator::IndexMapping& ProjectOperator::getMapping() const {
        return mMapping;
    }

    void ProjectOperator::validateInput() {
        if (!mNameMapping.empty() && mMapping.empty()) {
            mMapping = resolveNameMapping(getInputPorts().at(0), mNameMapping);
        }
        if (mMapping.empty()) {
            throw std::invalid_argument("ProjectOperator: mapping is empty");
        }

        core::logical::port::Port output;
        auto& inputRef = getInputPortsRef().at(0);
        auto& outputRef = getOutputPortsRef().at(0);
        std::ranges::sort(mMapping);
        for (const auto& index : mMapping) {
            if (index >= inputRef.size()) {
                throw std::invalid_argument("ProjectOperator: index out of range");
            }

            output.push_back(std::move(inputRef.at(index)));
        }

        outputRef = std::move(output);

        if (mMapping.size() != getOutputPorts().at(0).size()) {
            throw std::invalid_argument("ProjectOperator: mapping size must match outputPort");
        }
    }

    ProjectOperator::IndexMapping
    ProjectOperator::resolveNameMapping(const core::logical::port::Port& inputPort,
                                        const NameMapping& nameMapping) {
        IndexMapping mapping;
        mapping.reserve(nameMapping.size());

        for (const auto& name : nameMapping) {
            auto iter = std::ranges::find_if(inputPort,
                                             [&](const core::logical::port::PortDescription& pd) {
                                                 return pd.fieldName == name;
                                             });
            if (iter == inputPort.end()) {
                throw std::invalid_argument("ProjectOperator: name not found in inputOperator");
            }
            mapping.push_back(static_cast<std::size_t>(std::distance(inputPort.begin(), iter)));
        }

        return mapping;
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::project::logical