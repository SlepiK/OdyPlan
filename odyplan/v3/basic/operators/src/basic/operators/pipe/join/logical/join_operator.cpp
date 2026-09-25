#include <stdexcept>
#include <basic/operators/pipe/join/logical/join_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::join::logical {
    JoinOperator::JoinOperator(const details::JoinConfig config,
                               std::unique_ptr<basic::expression::logical::IExpression> predicate, const bool assureOrder)
        : mConfig(config), mLPredicate(std::move(predicate)), mAssureOrder(assureOrder) {}

    core::shared::types::OperatorTypeId JoinOperator::getOperatorTypeId() const {
        return TYPE_ID;
    }

    const details::JoinConfig& JoinOperator::getConfig() const {
        return mConfig;
    }

    std::unique_ptr<basic::expression::physical::IExpression> JoinOperator::getPredicate() {
        if (mPPredicate == nullptr) {
            return nullptr;
        }

        return std::move(mPPredicate);
    }

    void JoinOperator::bindInput(const core::shared::identifiers::PortId portId,
                                 const core::logical::port::Port input) {
        auto& inputPorts = getInputPortsRef();

        if (portId > 1) {
            throw std::logic_error(
                "IntervalJoinOperator::bindInput: port id out of range (0 and 1 only allowed)");
        }

        if (inputPorts.size() <= portId) {
            inputPorts.resize(portId + 1);
        }

        inputPorts.at(portId) = input;
    }

    void JoinOperator::validateInput() {
        const auto& inputPorts = getInputPorts();
        if (inputPorts.size() != 2 || inputPorts.at(0).empty() || inputPorts.at(1).empty()) {
            return;
        }

        const auto& left = inputPorts.at(0);
        const auto& right = inputPorts.at(1);

        core::logical::port::Port combinedPort;
        combinedPort.reserve(left.size() + right.size());
        combinedPort.insert(combinedPort.end(), left.begin(), left.end());
        combinedPort.insert(combinedPort.end(), right.begin(), right.end());
        getOutputPortsRef() = {combinedPort};

        if (mLPredicate->getTypeId() != std::to_underlying(core::shared::types::BuiltinValueType::BOOL)) {
            throw std::invalid_argument("JoinOperator: predicate must return bool");
        }

        const core::logical::port::PortDescriptionResolver resolver(combinedPort,{left.size(), right.size()});
        expression::translate::LogicalToPhysicalTranslator translator(resolver);
        mPPredicate = translator.translate(*mLPredicate);
    }

    void JoinOperator::validate() const {
        const auto& inputPorts = getInputPorts();
        if (inputPorts.size() != 2 || inputPorts.at(0).empty() || inputPorts.at(1).empty()) {
            throw std::logic_error("IntervalJoinOperator::validate: expects two inputs");
        }
        if (getOutputPorts().empty() || getOutputPorts().front().empty()) {
            throw std::logic_error("IntervalJoinOperator::validate: output schema never computed");
        }
    }

    bool JoinOperator::isOutOfOrder() const {
        return mAssureOrder;
    }
}  // namespace sierra::odyplan::v3::basic::operators::pipe::join::logical