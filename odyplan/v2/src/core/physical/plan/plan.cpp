#include "core/physical/plan/plan.h"

#include <ranges>

#include "core/physical/segment/segment_context.h"

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

    void Plan::codegen(llvm::Module& module, llvm::LLVMContext& ctx) const {
        std::vector<llvm::Function*> functions;
        functions.reserve(mSegments.size());

        for (std::size_t i = 0; i < mSegments.size(); ++i) {
            functions.push_back(codegenSegment(mSegments[i], module, ctx, i));
        }

        codegenMain(module, ctx, functions);
    }

    llvm::Function* Plan::codegenSegment(const segment::Segment& segment, llvm::Module& module, llvm::LLVMContext& ctx, const std::size_t index) const {
        auto* functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "ody_segment_" + std::to_string(index), &module);

        auto* basicBlock = llvm::BasicBlock::Create(ctx, "basicEntry", function);

        segment::SegmentContext segmentCtx(function, basicBlock, ctx, *mTypeRegistry);

        port::PortTupleConsumer const noopConsumer = [](segment::SegmentContext&, const port::PortTuple&) {};

        opr::IConsumer* target = nullptr;

        switch (segment.type) {
            case segment::SegmentType::NORMAL:
                target = segment.end;
                break;
            case segment::SegmentType::BLOCKING:
                target = segment.blockingTarget;
                break;
        }

        port::PortTupleConsumer chain = [target, noopConsumer](segment::SegmentContext& segmentCtx, const port::PortTuple& tuple) {
            target->consume(segmentCtx, tuple, noopConsumer);
        };

        struct EndEntry {
            opr::IPipeOperator* pipe;
            port::PortTupleConsumer downstream;
        };

        std::vector<EndEntry> endEntries;

        for (auto *it : std::ranges::reverse_view(segment.mid)) {
            opr::IPipeOperator* pipeOpr = it;
            port::PortTupleConsumer const next = chain;
            endEntries.push_back(EndEntry{.pipe=pipeOpr, .downstream=next});
            chain = [pipeOpr, next](segment::SegmentContext& sCtx, const port::PortTuple& t) mutable {
                pipeOpr->consume(sCtx, t, next);
            };
        }

        std::ranges::reverse(endEntries);

        segment.start->produce(segmentCtx, chain);

        for (const auto&[pipe, downstream] : endEntries) {
            pipe->end(segmentCtx, downstream);
        }

        if (target != nullptr) {
            target->end(segmentCtx, noopConsumer);
        }

        segmentCtx.builder.CreateRetVoid();
        return function;
    }

    void Plan::codegenMain(llvm::Module& module, llvm::LLVMContext& ctx, const std::vector<llvm::Function*>& functions) {
        auto* functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false);
        auto* mainFn = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, "ody_main", &module);

        auto* basicBlock = llvm::BasicBlock::Create(ctx, "basicEntry", mainFn);

        llvm::IRBuilder<> builder(basicBlock);

        for (auto* function : functions) {
            builder.CreateCall(function, {});
        }

        const auto endFn = module.getOrInsertFunction("rtEnd", llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), {}, false));
        builder.CreateCall(endFn, {});

        builder.CreateRetVoid();
    }
}  // namespace sierra::odyplan::v2::core::physical::plan