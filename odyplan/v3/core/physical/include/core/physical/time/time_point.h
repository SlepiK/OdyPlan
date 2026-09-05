#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_POINT__HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_POINT__HPP
#include <cstdint>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>

/**
 * Based on Odysseus PointInTime Class
 */
namespace sierra::odyplan::v3::core::physical::time {
    struct TimePoint {
        //i64
        llvm::Value* point = nullptr;
        //i1
        llvm::Value* isInfinity = nullptr;
        //i64
        llvm::Value* tiBreaker = nullptr;

        //create
        static TimePoint infinity(llvm::IRBuilder<>& builder, llvm::Module& module, bool track = false, bool threadSafe = true);
        static TimePoint fromValue(llvm::IRBuilder<>& builder, llvm::Module& module, llvm::Value* rtPoint, bool track = false, bool threadSafe = true);
        static TimePoint constant(llvm::IRBuilder<>& builder, llvm::Module& module, std::int64_t point, bool track = false, bool threadSafe = true);
        static TimePoint zero(llvm::IRBuilder<>& builder, llvm::Module& module, bool track = false, bool threadSafe = true);

        //compare
        [[nodiscard]] llvm::Value* before(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;
        [[nodiscard]] llvm::Value* beforeOrEquals(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;
        [[nodiscard]] llvm::Value* after(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;
        [[nodiscard]] llvm::Value* afterOrEquals(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;
        [[nodiscard]] llvm::Value* equals(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;
        [[nodiscard]] llvm::Value* compareTo(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;
        [[nodiscard]] llvm::Value* compareOrdTo(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;

        //min/max
        static TimePoint min(llvm::IRBuilder<>& builder, const TimePoint& tpA, const TimePoint& tpB);
        static TimePoint max(llvm::IRBuilder<>& builder, const TimePoint& tpA, const TimePoint& tpB);

        //add/sub
        [[nodiscard]] TimePoint add(llvm::IRBuilder<>& builder, llvm::Module& module, llvm::Value* rtDelta, bool threadSafe = true) const;
        [[nodiscard]] TimePoint sub(llvm::IRBuilder<>& builder, llvm::Module& module, llvm::Value* rtDelta, bool threadSafe = true) const;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_POINT__HPP