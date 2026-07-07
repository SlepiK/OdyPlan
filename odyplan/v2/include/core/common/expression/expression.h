#ifndef SIERRA_ODYPLAN_V2_CORE_COMMON_EXPRESSION_EXPRESSION_H
#define SIERRA_ODYPLAN_V2_CORE_COMMON_EXPRESSION_EXPRESSION_H

#include <cstddef>
#include <memory>

#include "core/common/ident.h"

namespace sierra::odyplan::v2::core::common::expression {
    class IExpression {
    public:
        virtual ~IExpression() = default;
    };

    class ConstantExpression : public IExpression {
    public:
        explicit ConstantExpression() {
        }

        [[nodiscard]] std::size_t getFieldIndex() const {
            return 0;
        }

    private:
    };
}  // namespace sierra::odyplan::v2::core::common::expression

#endif  // SIERRA_ODYPLAN_V2_CORE_COMMON_EXPRESSION_EXPRESSION_H