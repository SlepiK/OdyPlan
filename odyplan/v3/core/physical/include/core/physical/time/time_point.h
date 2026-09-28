#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_POINT_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_POINT_HPP
#include <cstdint>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <core/shared/types/global_types.h>

/**
 * Based on Odysseus PointInTime Class
 */
namespace sierra::odyplan::v3::core::physical::time {
    class TimePoint {
    public:
        TimePoint(llvm::IRBuilder<>& builder, llvm::Value* point);
        TimePoint(llvm::Value* point, llvm::Value* tiBreaker);
        TimePoint(llvm::IRBuilder<>& builder, const TimePoint& tpOther);

        //create
        static TimePoint infinity(llvm::IRBuilder<>& builder);
        static TimePoint zero(llvm::IRBuilder<>& builder);
        static TimePoint one(llvm::IRBuilder<>& builder);

        static TimePoint systemTime(llvm::IRBuilder<>& builder);
        static TimePoint executionTime(llvm::IRBuilder<>& builder);

        [[nodiscard]] llvm::Value* hash(llvm::IRBuilder<>& builder) const;

        //hlp
        static TimePoint select(llvm::IRBuilder<>& builder, llvm::Value* condition, const TimePoint& ifTrue, const TimePoint& ifFalse);

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
        [[nodiscard]] TimePoint add(llvm::IRBuilder<>& builder, llvm::Value* rtDelta) const;
        [[nodiscard]] TimePoint sub(llvm::IRBuilder<>& builder, llvm::Value* rtDelta) const;

        //getter
        [[nodiscard]] llvm::Value* getPoint() const;
        [[nodiscard]] llvm::Value* getTiBreaker() const;
        [[nodiscard]] llvm::Value* isInfinite(llvm::IRBuilder<>& builder) const;

    private:
        //i64
        llvm::Value* mPoint = nullptr;
        //i64
        llvm::Value* mTiBreaker = nullptr;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_POINT_HPP