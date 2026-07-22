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
        std::unordered_map<common::identifier::OperatorId, std::size_t> degree;
        std::vector<common::identifier::OperatorId> ready;
        std::vector<common::identifier::OperatorId> result;

        result.reserve(mOperators.size());

        for (const auto& opr : mOperators) {
            const auto degree = getInEdgesFromOperator(opr->)
        }

        /*kahn
        *std::vector<common::identifier::OperatorId> Plan::topologicalOrder() const {
    std::unordered_map<common::identifier::OperatorId, std::size_t> inDegree;
    std::vector<common::identifier::OperatorId> ready;
    std::vector<common::identifier::OperatorId> result;
    result.reserve(mOperators.size()); // oder was auch immer eure Operator-Sammlung heißt

    for (const auto& [id, op] : mOperators) {
        const auto degree = getInEdgesFromOperator(id).size();
        inDegree[id] = degree;
        if (degree == 0) {
            ready.push_back(id);
        }
    }

    while (!ready.empty()) {
        const auto current = ready.back();
        ready.pop_back();
        result.push_back(current);

        for (const auto& edge : getOutEdgesFromOperator(current)) {
            const auto successor = edge.toOperator.operatorId;
            if (--inDegree[successor] == 0) {
                ready.push_back(successor);
            }
        }
    }

    if (result.size() != mOperators.size()) {
        throw std::logic_error("Plan::topologicalOrder: cycle detected (should be impossible due to wouldCreateCycle checks in connect())");
    }

    return result;
}
         **/


        const std::size_t n = mOperators.size();

        enum class State : std::uint8_t {TO_DO, IN_PROGRESS, FINISHED};

        std::vector state(n, State::TO_DO);
        std::vector<common::identifier::OperatorId> order;
        order.reserve(n);

        std::function<void(common::identifier::OperatorId)> depthfirstsearch = [&](const common::identifier::OperatorId cId) {
            state.at(cId) = State::IN_PROGRESS;

            for (const auto&[fromOperator, toOperator] : mEdges) {
                if (fromOperator.operatorId == cId) {
                    const auto next = toOperator.operatorId;
                    if (state.at(next) == State::IN_PROGRESS) {
                        throw std::runtime_error(
                            "Plan::topologicalOrder: Late cycle detected - something went wrong");
                    }
                    if (state.at(next) == State::TO_DO) {
                        depthfirstsearch(next);
                    }
                }
            }

            state.at(cId) = State::FINISHED;
            order.push_back(cId);
        };

        for (const auto sourceOprId : mSourceOperators) {
            depthfirstsearch(sourceOprId);
        }

        //To have the sources at the beginning
        std::ranges::reverse(order);
        return order;
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