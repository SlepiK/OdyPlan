#include "core/logical/plan/plan.h"

#include <algorithm>
#include <unordered_set>

namespace sierra::odyplan::v2::core::logical::plan {
    port::OperatorId Plan::addOperator(std::unique_ptr<opr::IOperator> opr) {
        if (!opr) {
            throw std::invalid_argument("Plan::addOperator: opr must not be nullptr!");
        }

        const auto id = mOperators.size();

        if (!opr->hasInputPorts()) {
            mSourceOperators.push_back(id);
        }

        mOperators.push_back(std::move(opr));
        return id;
    }


    void Plan::connect(const port::PortRef& fromOperator, const port::PortRef& toOperator) {
        validateConnectionRequest(fromOperator, toOperator);

        if (!areConnectionTypesCompatible(fromOperator, toOperator)) {
            throw std::invalid_argument("Plan::connect: incompatible port types between fromOperator and toOperator");
        }

        if (wouldCreateCycle(fromOperator.operatorId, toOperator.operatorId)) {
            throw std::invalid_argument("Plan::connect: connection would create a cycle in the plan");
        }

        mEdges.push_back(edge::Edge(fromOperator, toOperator));
    }

    [[nodiscard]] opr::IOperator& Plan::op(const port::OperatorId id) const {
        return *mOperators.at(id);
    }

    void Plan::validateConnectionRequest(const port::PortRef& fromOperator,
                                         const port::PortRef& toOperator) {
        if (fromOperator.operatorId >= mOperators.size()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: fromOperator.operatorId out of range");
        }

        if (toOperator.operatorId >= mOperators.size()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: toOperator.operatorId out of range");
        }

        const auto& from = op(fromOperator.operatorId);
        const auto& to = op(toOperator.operatorId);

        if (!from.hasOutputPorts()) {
            throw std::invalid_argument("Plan::validateConnectionRequest: fromOperator has no output ports");
        }

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
            mEdges,[&](const edge::Edge& edge) {
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

    bool Plan::areConnectionTypesCompatible(const port::PortRef& fromOperator, const port::PortRef& toOperator) const {
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

    bool Plan::wouldCreateCycle(port::OperatorId from, port::OperatorId to) const {
        std::unordered_set<port::OperatorId> visitedOperators;
        std::vector<port::OperatorId> operatorStack{to};

        while (!operatorStack.empty()) {
            const auto current = operatorStack.back();
            operatorStack.pop_back();

            if (current == from) {
                return true;
            }

            if (!visitedOperators.insert(current).second) {
                continue;
            }

            for (const edge::Edge& edge : mEdges) {
                if (edge.fromOperator.operatorId == current) {
                    operatorStack.push_back(edge.toOperator.operatorId);
                }
            }
        }

        return false;
    }
}  // namespace sierra::odyplan::v2::core::logical::plan