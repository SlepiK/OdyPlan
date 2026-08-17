#include <algorithm>
#include <core/logical/plan/plan.h>
#include <stdexcept>
#include <unordered_set>

namespace sierra::odyplan::v3::core::logical::plan {
    void Plan::connect(const OperatorPortReference& fromOperator,
                       const OperatorPortReference& toOperator) {
        validateConnectionRequest(fromOperator, toOperator);

        if (!areConnectionTypesCompatible(fromOperator, toOperator)) {
            throw std::invalid_argument(
                "Plan::connect: incompatible port types between fromOperator and toOperator");
        }

        if (wouldCreateCycle(fromOperator.operatorId, toOperator.operatorId)) {
            throw std::invalid_argument(
                "Plan::connect: connection would create a cycle in the plan");
        }

        const auto edge = OperatorPortReferenceConnection(fromOperator, toOperator);
        mConnections.push_back(edge);
        mIncomingConnections[toOperator.operatorId].push_back(edge);
        mOutgoingConnections[fromOperator.operatorId].push_back(edge);
    }

    [[nodiscard]] Plan::IOperator& Plan::op(const OperatorId id) const {
        if (id >= mOperators.size()) {
            throw std::invalid_argument("Plan::op: id is out of range");
        }

        return *mOperators.at(id);
    }

    void Plan::validateConnectionRequest(const OperatorPortReference& fromOperator,
                                         const OperatorPortReference& toOperator) {
        if (fromOperator.operatorId >= mOperators.size()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: fromOperator.operatorId out of range");
        }

        if (toOperator.operatorId >= mOperators.size()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: toOperator.operatorId out of range");
        }

        const auto& from = op(fromOperator.operatorId);

        if (!from.hasOutputPorts()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: fromOperator has no output ports");
        }

        if (from.getOutputPorts().empty()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: fromOperator has no output schema");
        }

        if (std::ranges::find(mSourceOperators, toOperator.operatorId) != mSourceOperators.end()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: source operator cant get a connection");
        }

        if (fromOperator.portId > from.getOutputPorts().size() - 1) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: source operator dont support the given port");
        }

        const auto& bindable = mBindables.at(toOperator.operatorId);

        bindable->bindInput(toOperator.portId, from.getOutputPorts().at(fromOperator.portId));

        bindable->validateInput();

        const auto& to = op(toOperator.operatorId);

        if (!to.hasInputPorts()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: toOperator has no input ports");
        }

        if (fromOperator.portId >= from.getOutputPorts().size()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: fromOperator.portId is out of range");
        }

        if (toOperator.portId >= to.getInputPorts().size()) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: toOperator.portId is out of range");
        }

        // No Fan-in # only one edge to one input port allowed!
        const bool alreadyConnected =
            std::ranges::any_of(mConnections, [&](const OperatorPortReferenceConnection& edge) {
                return edge.toOperatorPortReference.operatorId == toOperator.operatorId &&
                       edge.toOperatorPortReference.portId == toOperator.portId;
            });

        if (alreadyConnected) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: input port is already used");
        }

        if (fromOperator.operatorId == toOperator.operatorId) {
            throw std::invalid_argument(
                "Plan::validateConnectionRequest: self-loops are not allowed");
        }
    }

    [[nodiscard]] std::vector<Plan::OperatorId> Plan::topologicalOrder() const {
        std::unordered_map<OperatorId, std::size_t> inDegree;
        std::vector<OperatorId> ready;
        std::vector<OperatorId> result;

        result.reserve(mOperators.size());

        for (std::size_t operatorId = 0; operatorId < mOperators.size(); ++operatorId) {
            // Get all edges coming in the operator (source_operator has 0!)
            const auto operatorDegree = getInConnectionsFromOperator(operatorId).size();
            inDegree[operatorId] = operatorDegree;
            if (operatorDegree == 0) {
                // source_operator dont need further processing
                ready.push_back(operatorId);
            }
        }

        while (!ready.empty()) {
            const auto currentOperatorId = ready.back();
            ready.pop_back();
            result.push_back(currentOperatorId);

            for (const auto& currentOperatorOutEdge : getOutConnectionsFromOperator(currentOperatorId)) {
                const auto nextOperatorId =
                    currentOperatorOutEdge.toOperatorPortReference.operatorId;
                if (--inDegree[nextOperatorId] == 0) {
                    ready.push_back(nextOperatorId);
                }
            }
        }

        if (result.size() != mOperators.size()) {
            // If a operator is added but not connected, it should also exit here
            throw std::logic_error("Plan::topologicalOrder: Missmatch of size. Cycle or missing "
                                   "connected operator detected");
        }

        return result;
    }

    bool Plan::areConnectionTypesCompatible(const OperatorPortReference& fromOperator,
                                            const OperatorPortReference& toOperator) const {
        const auto& outPort = op(fromOperator.operatorId).getOutputPorts().at(fromOperator.portId);
        const auto& inPort = op(toOperator.operatorId).getInputPorts().at(toOperator.portId);

        if (outPort.size() != inPort.size()) {
            return false;
        }

        for (std::size_t i = 0; i < outPort.size(); ++i) {
            if (outPort.at(i).valueTypeId != inPort.at(i).valueTypeId) {
                return false;
            }
        }

        return true;
    }

    bool Plan::wouldCreateCycle(const OperatorId from, const OperatorId to) const {
        std::unordered_set<OperatorId> visitedOperators;
        std::vector operatorStack{to};

        while (!operatorStack.empty()) {
            const auto current = operatorStack.back();
            operatorStack.pop_back();

            if (current == from) {
                return true;
            }

            if (!visitedOperators.insert(current).second) {
                continue;
            }

            for (const auto& [fromOperator, toOperator] : mConnections) {
                if (fromOperator.operatorId == current) {
                    operatorStack.push_back(toOperator.operatorId);
                }
            }
        }

        return false;
    }

    [[nodiscard]] std::vector<Plan::OperatorPortReferenceConnection>
    Plan::getInConnectionsFromOperator(const OperatorId id) const {
        const auto it = mIncomingConnections.find(id);
        return it != mIncomingConnections.end() ? it->second
                                          : std::vector<OperatorPortReferenceConnection>();
    }

    [[nodiscard]] std::vector<Plan::OperatorPortReferenceConnection>
    Plan::getOutConnectionsFromOperator(const OperatorId id) const {
        const auto it = mOutgoingConnections.find(id);
        return it != mOutgoingConnections.end() ? it->second
                                          : std::vector<OperatorPortReferenceConnection>();
    }
}  // namespace sierra::odyplan::v3::core::logical::plan