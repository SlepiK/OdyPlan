#ifndef SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATOR_H
#define SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATOR_H

namespace sierra::odyplan::v1::core::opr {
    class IOperator {
    public:
        IOperator() = default;
        virtual ~IOperator() = default;

        IOperator(const IOperator&) = delete;
        IOperator& operator=(const IOperator&) = delete;

        IOperator(IOperator&&) = delete;
        IOperator& operator=(IOperator&&) = delete;
    };
}  // namespace sierra::odyplan::v1::core::opr

#endif  // SIERRA_ODYPLAN_V1_CORE_OPERATOR_OPERATOR_H