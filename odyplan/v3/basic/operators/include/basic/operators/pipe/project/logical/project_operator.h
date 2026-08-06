#ifndef SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_LOGICAL_PROJECT_OPERATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_LOGICAL_PROJECT_OPERATOR_HPP

#include <string>
#include <utility>
#include <vector>
#include <core/logical/operators/pipe/typed_pipe_operator.h>

namespace sierra::odyplan::v3::basic::operators::pipe::project::logical {
    class ProjectOperator : public core::logical::operators::pipe::TypedPipeOperator {
    public:
        using IndexMapping = std::vector<std::size_t>;
        using NameMapping = std::vector<std::string>;

        static constexpr core::shared::types::OperatorTypeId TYPE_ID = std::to_underlying(
            core::shared::types::BuiltinOperatorType::PROJECT);

        explicit ProjectOperator(IndexMapping mapping);

        explicit ProjectOperator(NameMapping mapping);

        [[nodiscard]] core::shared::types::OperatorTypeId getOperatorTypeId() const override;

        [[nodiscard]] const IndexMapping &getMapping() const;

    protected:
        void validateInput() override;

    private:
        IndexMapping mMapping;
        NameMapping mNameMapping;

        static IndexMapping resolveNameMapping(const core::logical::port::Port &inputPort,
                                               const NameMapping &nameMapping);
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_OPERATORS_PIPE_PROJECT_LOGICAL_PROJECT_OPERATOR_HPP