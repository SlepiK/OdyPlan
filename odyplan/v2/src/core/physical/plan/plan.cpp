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
        if (segment.start == nullptr) {
            throw std::invalid_argument("Plan::addSegment: segment has no start!");
        }

        switch (segment.type) {
            case segment::SegmentType::NORMAL:
                if (segment.end == nullptr) {
                    throw std::invalid_argument("Plan::addSegment: segment has no end!");
                }
                break;
            case segment::SegmentType::BLOCKING:
                if (segment.blockingTarget == nullptr) {
                    throw std::invalid_argument("Plan::addSegment: segment has no blockingTarget!");
                }
                break;
        }

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

        /*port::PortTupleConsumer chain = [end = segment.end](llvm::IRBuilder<>& b, const port::PortTuple& t) {
            end->consume(b,t,[](llvm::IRBuilder<>&, const port::PortTuple&) {});
        };*/

        port::PortTupleConsumer chain;

        switch (segment.type) {
            case segment::SegmentType::NORMAL:
                chain = [end = segment.end] (llvm::IRBuilder<>& b, const port::PortTuple& t) {
                    end->consume(b,t,[](llvm::IRBuilder<>&, const port::PortTuple&) {});
                };
                break;
            case segment::SegmentType::BLOCKING:
                chain = [target = segment.blockingTarget] (llvm::IRBuilder<>& b, const port::PortTuple& t) {
                    target->consume(b, t, [](llvm::IRBuilder<>&, const port::PortTuple&) {});
                };
                break;
        }

        for (auto *it : std::ranges::reverse_view(segment.mid)) {
            opr::IPipeOperator* pipe = it;
            port::PortTupleConsumer next = chain;
            chain = [pipe, next](llvm::IRBuilder<>& b, const port::PortTuple& t) mutable {
                pipe->consume(b, t, next);
            };
        }

        segment.start->produce(builder, std::move(chain));

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

        //ToDo: do we need it and should it part here?
        /*auto* flushSinkFn = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {llvm::Type::getInt64Ty(ctx)}, false);
        for (std::uint64_t i = 0; i < gSinks.size(); ++i)
        {
            builder.CreateCall(flushSinkFn, { llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), i) });
        }*/

        builder.CreateRetVoid();
    }
}  // namespace sierra::odyplan::v2::core::physical::plan