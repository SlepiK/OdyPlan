#include "core/physical/plan/plan.h"

#include <ranges>

namespace sierra::odyplan::v2::core::physical::plan {
    void Plan::addOperator(std::unique_ptr<opr::IOperator> opr) {
        if (!opr) {
            throw std::invalid_argument("Plan::addOperator: opr must not be nullptr!");
        }

        mOperators.push_back(std::move(opr));
    }

    void Plan::addSegment(segment::Segment segment) {
        //ToDo: Verify no empty segments
        mSegments.push_back(std::move(segment));
    }

    void Plan::codegen(llvm::Module& module, llvm::LLVMContext& ctx) {
        std::vector<llvm::Function*> functions;
        functions.reserve(mSegments.size());

        for (std::size_t i = 0; i < mSegments.size(); ++i) {
            functions.push_back(codegenSegment(mSegments[i], module, ctx, i));
        }

        codegenMain(module, ctx, functions);
    }

    llvm::Function* Plan::codegenSegment(const segment::Segment& segment, llvm::Module& module, llvm::LLVMContext& ctx, const std::size_t index) {
        auto* functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "ody_segment_" + std::to_string(index), &module);

        auto* basicBlock = llvm::BasicBlock::Create(ctx, "basicEntry", function);

        llvm::IRBuilder<> builder(basicBlock);

        port::PortTupleConsumer consumerChain = [](llvm::IRBuilder<>&, const port::PortTuple&) {};

        for (auto *consumer : std::ranges::reverse_view(segment.midEnd)) {
            port::PortTupleConsumer const nextConsumer = consumerChain;
            consumerChain = [consumer, nextConsumer](llvm::IRBuilder<>& cBuilder, const port::PortTuple& cTuple) mutable {
                consumer->consume(cBuilder, cTuple, nextConsumer);
            };
        }

        segment.start->produce(builder, std::move(consumerChain));

        builder.CreateRetVoid();
        return function;
    }

    void Plan::codegenMain(llvm::Module& module, llvm::LLVMContext& ctx, const std::vector<llvm::Function*>& functions) {
        auto* functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* mainFunction = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "ody_main", &module);

        auto* basicBlock = llvm::BasicBlock::Create(ctx, "basicEntry", mainFunction);

        llvm::IRBuilder<> builder(basicBlock);

        for (auto* function : functions) {
            builder.CreateCall(function, {});
        }

        builder.CreateRetVoid();
    }
}  // namespace sierra::odyplan::v2::core::physical::plan