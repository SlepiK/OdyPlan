#ifndef SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_EXPRESSION_CODEGEN_HPP
#define SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_EXPRESSION_CODEGEN_HPP
#include <core/physical/data/data_package.h>
#include <core/physical/segment/segment_context.h>
#include <llvm/IR/Value.h>

#include <basic/expression/physical/compare_expression.h>
#include <basic/expression/physical/expression.h>
#include <basic/expression/physical/constant_expression.h>
#include <basic/expression/physical/field_reference_expression.h>
#include <basic/expression/physical/logical_expression.h>

namespace sierra::odyplan::v3::basic::expression::physical {
    llvm::Value*
    codegenExpression(core::physical::segment::SegmentContext& segmentCtx,
                                    const core::physical::data::DataPackage& tuple,
                                    const IExpression& expression);

    llvm::Value* codegenCompare(llvm::IRBuilder<>& builder,
                                 llvm::Value* left,
                                 llvm::Value* right,
                                 CompareOp opr,
                                 core::shared::types::ValueTypeId leftId,
                                 core::shared::types::ValueTypeId rightId);
}

#endif //SIERRA_ODYPLAN_V3_BASIC_EXPRESSION_PHYSICAL_EXPRESSION_CODEGEN_HPP