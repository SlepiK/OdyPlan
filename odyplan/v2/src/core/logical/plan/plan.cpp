#include "core/logical/plan/plan.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <unordered_set>

namespace sierra::odyplan::v2::core::logical::plan {
    void Plan::connect(const common::port::PortRef& fromOperator, const common::port::PortRef& toOperator) {
        validateConnectionRequest(fromOperator, toOperator);

        if (!areConnectionTypesCompatible(fromOperator, toOperator)) {
            throw std::invalid_argument("Plan::connect: incompatible port types between fromOperator and toOperator");
        }

        if (wouldCreateCycle(fromOperator.operatorId, toOperator.operatorId)) {
            throw std::invalid_argument("Plan::connect: connection would create a cycle in the plan");
        }

        const auto edge = common::edge::Edge(fromOperator, toOperator);
        mEdges.push_back(edge);
        mInEdges[toOperator.operatorId].push_back(edge);
        mOutEdges[fromOperator.operatorId].push_back(edge);
    }

    [[nodiscard]] opr::IOperator& Plan::op(const common::identifier::OperatorId id) const {
        if (id >= mOperators.size()) {
            throw std::invalid_argument("Plan::op: id is out of range");
        }

        return *mOperators.at(id);
    }

    void Plan::validateConnectionRequest(const common::port::PortRef& fromOperator,
                                         const common::port::PortRef& toOperator) {
        if (fromOperator.operatorId >= mOperators.size()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: fromOperator.operatorId out of range");
        }

        if (toOperator.operatorId >= mOperators.size()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: toOperator.operatorId out of range");
        }

        const auto& from = op(fromOperator.operatorId);

        if (!from.hasOutputPorts()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: fromOperator has no output ports");
        }

        if (from.getOutputPorts().empty()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: fromOperator has no output schema");
        }

        if (std::ranges::find(mSourceOperators, toOperator.operatorId) != mSourceOperators.end()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: source operator cant get a connection");
        }

        const auto& bindable = mBindables.at(toOperator.operatorId);

        bindable->bindInput(toOperator.portId, from.getOutputPorts().at(fromOperator.portId));

        bindable->validate();

        const auto& to = op(toOperator.operatorId);

        if (!to.hasInputPorts()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: toOperator has no input ports");
        }

        if (fromOperator.portId >= from.getOutputPorts().size()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: fromOperator.portId is out of range");
        }

        if (toOperator.portId >= to.getInputPorts().size()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: toOperator.portId is out of range");
        }

        // No Fan-in # only one edge to one input port allowed!
        const bool alreadyConnected = std::ranges::any_of(
            mEdges,[&](const common::edge::Edge& edge) {
                return edge.toOperator.operatorId == toOperator.operatorId &&
                    edge.toOperator.portId == toOperator.portId;
            }
        );

        if (alreadyConnected) {
            throw std::invalid_argument("Plan::validateConnectionRequest: input port is already used");
        }

        if (fromOperator.operatorId == toOperator.operatorId) {
            throw std::invalid_argument("Plan::validateConnectionRequest: self-loops are not allowed");
        }
    }

    [[nodiscard]] std::vector<common::identifier::OperatorId> Plan::topologicalOrder() const {
        std::unordered_map<common::identifier::OperatorId, std::size_t> inDegree;
        std::vector<common::identifier::OperatorId> ready;
        std::vector<common::identifier::OperatorId> result;

        result.reserve(mOperators.size());

        for (std::size_t operatorId = 0; operatorId < mOperators.size(); ++operatorId) {
            //Get all edges coming in the operator (source_operator has 0!)
            const auto operatorDegree = getInEdgesFromOperator(operatorId).size();
            inDegree[operatorId] = operatorDegree;
            if (operatorDegree == 0) {
                //source_operator dont need further processing
                ready.push_back(operatorId);
            }
        }

        while (!ready.empty()) {
            const auto currentOperatorId = ready.back();
            ready.pop_back();
            result.push_back(currentOperatorId);

            for (const auto& currentOperatorOutEdge : getOutEdgesFromOperator(currentOperatorId)) {
                const auto nextOperatorId = currentOperatorOutEdge.toOperator.operatorId;
                if (--inDegree[nextOperatorId] == 0) {
                    ready.push_back(nextOperatorId);
                }
            }
        }

        if (result.size() != mOperators.size()) {
            //If a operator is added but not connected, it should also exit here
            throw std::logic_error("Plan::topologicalOrder: Missmatch of size. Cycle or missing connected operator detected");
        }

        return result;
    }

    bool Plan::areConnectionTypesCompatible(const common::port::PortRef& fromOperator, const common::port::PortRef& toOperator) const {
        const auto& outPort = op(fromOperator.operatorId).getOutputPorts().at(fromOperator.portId);
        const auto& inPort = op(toOperator.operatorId).getInputPorts().at(toOperator.portId);

        if (outPort.size() != inPort.size()) {
            return false;
        }

        for (std::size_t i = 0; i < outPort.size(); ++i) {
            if (outPort.at(i).typeId != inPort.at(i).typeId) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] std::vector<common::edge::Edge> Plan::getInEdgesFromOperator(const common::identifier::OperatorId id) const {
        const auto it = mInEdges.find(id);
        return it != mInEdges.end() ? it->second : std::vector<common::edge::Edge>();
    }

    [[nodiscard]] std::vector<common::edge::Edge> Plan::getOutEdgesFromOperator(const common::identifier::OperatorId id) const {
        const auto it = mOutEdges.find(id);
        return it != mOutEdges.end() ? it->second : std::vector<common::edge::Edge>();
    }

    bool Plan::wouldCreateCycle(const common::identifier::OperatorId from, const common::identifier::OperatorId to) const {
        std::unordered_set<common::identifier::OperatorId> visitedOperators;
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

            for (const auto&[fromOperator, toOperator] : mEdges) {
                if (fromOperator.operatorId == current) {
                    operatorStack.push_back(toOperator.operatorId);
                }
            }
        }

        return false;
    }
}  // namespace sierra::odyplan::v2::core::logical::plan