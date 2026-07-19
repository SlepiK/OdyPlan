#ifndef SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECTOPERATOR_H
#define SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECTOPERATOR_H

#include <string>
#include <vector>

#include "core/logical/operator/pipe/pipe_operator.h"
#include "core/physical/operator/pipe/pipe_operator.h"
#include "core/common/port/port_ref.h"

namespace sierra::odyplan::v2::common::opr::pipe {
    //ToDo: Mapping from Odysseus-Operator Project (https://wiki.odysseus.informatik.uni-oldenburg.de/spaces/ODYSSEUS/pages/7111207/Project+operator)
    namespace logical {
        class ProjectOperator : public core::logical::opr::TypedPipeOperator {
        public:
            using IndexMapping = std::vector<std::size_t>;
            using NameMapping = std::vector<std::string>;
            static constexpr core::common::identifier::OperatorTypeId TYPE_ID = std::to_underlying(core::common::identifier::Operator::PROJECT);

            explicit ProjectOperator(IndexMapping mapping)
                : mMapping(std::move(mapping)){
            }

            explicit ProjectOperator(NameMapping mapping)
                : mNameMapping(std::move(mapping)) {
            }

            [[nodiscard]] core::common::identifier::OperatorTypeId getOperatorTypeId() const override {
                return TYPE_ID;
            }

            [[nodiscard]] const IndexMapping& getMapping() const {
                return mMapping;
            }

        protected:
            void validate() override {
                if (!mNameMapping.empty() && mMapping.empty()) {
                    mMapping = resolveNameMapping(getInputPorts().at(0), mNameMapping);
                }
                if (mMapping.empty()) {
                    throw std::invalid_argument("ProjectOperator: mapping is empty");
                }

                core::logical::port::Port output;
                auto& outputRef = getOutputPortsRef().at(0);
                for (const auto& index : mMapping) {
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

            static IndexMapping resolveNameMapping(const core::logical::port::Port& inputPort, const NameMapping& nameMapping) {
                IndexMapping mapping;
                mapping.reserve(nameMapping.size());

                for (const auto& name : nameMapping) {
                    auto iter = std::ranges::find_if(inputPort,
                                                     [&](const core::logical::port::PortDescriptor& pd) {
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

    namespace physical {
        class ProjectOperator : public core::physical::opr::TypedPipeOperator{
        public:
            using Mapping = std::vector<std::size_t>;

            ProjectOperator(core::physical::port::Port inputPort, core::physical::port::Port outputPort, Mapping mapping)
                : TypedPipeOperator(core::physical::port::Ports{std::move(inputPort)}, core::physical::port::Ports{std::move(outputPort)}),
                    mMapping(std::move(mapping)){
            }

            void consume(core::physical::segment::SegmentContext& segmentCtx, const core::physical::port::PortTuple &tuple, core::physical::port::PortTupleConsumer consumer) override {
                core::physical::port::PortTuple projected;
                projected.reserve(mMapping.size());

                for (const auto index : mMapping) {
                    if (index >= tuple.size()) {
                        throw std::invalid_argument("ProjectOperator: index out of range");
                    }
                    projected.push_back(tuple[index]);
                }

                consumer(segmentCtx, projected);
            }

            void end(core::physical::segment::SegmentContext &segmentCtx, core::physical::port::PortTupleConsumer consumer) override {
            }

        private:
            Mapping mMapping;
        };
    }
}

#endif //SIERRA_ODYPLAN_V2_COMMON_OPERATOR_PIPE_PROJECTOPERATOR_H