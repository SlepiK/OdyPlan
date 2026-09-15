#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_TRANSLATE_LOGICAL_TO_PHYSICAL_TRANSLATOR_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_TRANSLATE_LOGICAL_TO_PHYSICAL_TRANSLATOR_HPP
#include <cstdint>
#include <memory>
#include <stdexcept>

#include <core/logical/port/port_description_resolver.h>

#include <basic/expression/logical/expression.h>
#include <basic/expression/physical/expression.h>
#include <core/logical/port/port.h>

namespace sierra::odyplan::v3::basic::expression::translate {
    class LogicalToPhysicalTranslator {
    public:
        explicit LogicalToPhysicalTranslator(const core::logical::port::PortDescriptionResolver& resolver);

        [[nodiscard]] std::unique_ptr<physical::IExpression> translate(logical::IExpression& expr);

    private:
        const core::logical::port::PortDescriptionResolver& mResolver;
    };
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_TRANSLATE_LOGICAL_TO_PHYSICAL_TRANSLATOR_HPP