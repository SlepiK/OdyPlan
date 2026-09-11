#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_INTERVAL_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_INTERVAL_HPP

#include <core/physical/time/time_point.h>

/**
 * Based on Odysseus TimeInterval Class
 */
namespace sierra::odyplan::v3::core::physical::time {
    class TimeInterval {
    public:
        TimeInterval(llvm::IRBuilder<>& builder, TimePoint tpStart, TimePoint tpEnd);
        TimeInterval(llvm::IRBuilder<>& builder, TimePoint tpStart);
        explicit TimeInterval(llvm::IRBuilder<>& builder);

        static TimeInterval forever(llvm::IRBuilder<>& builder);

        [[nodiscard]] const TimePoint& getStart() const;
        [[nodiscard]] const TimePoint& getEnd() const;

        void setStart(llvm::IRBuilder<>& builder, TimePoint tpStart);
        void setEnd(llvm::IRBuilder<>& builder, TimePoint tpEnd);
        void setStartAndEnd(llvm::IRBuilder<>& builder, TimePoint tpStart, TimePoint tpEnd);

        void update(llvm::IRBuilder<>& builder, TimePoint tpStart, TimePoint tpEnd);

        [[nodiscard]] llvm::Value* isEndInfinite(llvm::IRBuilder<>& builder) const;
        [[nodiscard]] llvm::Value* isPoint(llvm::IRBuilder<>& builder) const;
        [[nodiscard]] llvm::Value* isEmpty(llvm::IRBuilder<>& builder) const;
        [[nodiscard]] llvm::Value* isZeroLength(llvm::IRBuilder<>& builder) const;
        [[nodiscard]] llvm::Value* includes(llvm::IRBuilder<>& builder, const TimePoint& tpPtn) const;

        static TimeInterval select(llvm::IRBuilder<>& builder, llvm::Value* condition, const TimeInterval& ifTrue, const TimeInterval& ifFalse);

        static llvm::Value* startBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* startsBeforeOrEqual(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* totallyBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiTiv, const TimePoint& tpPtn);
        static llvm::Value* totallyBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* totallyAfter(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* overlaps(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        [[nodiscard]] llvm::Value* overlaps(llvm::IRBuilder<>& builder, const TimeInterval& tiOther) const;
        static llvm::Value* inside(llvm::IRBuilder<>& builder, const TimeInterval& tiTiv, const TimePoint& tpPtn);
        static llvm::Value* inside(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* areAdjacent(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        [[nodiscard]] llvm::Value* equals(llvm::IRBuilder<>& builder, const TimeInterval& tiOther) const;
        [[nodiscard]] llvm::Value* compareTo(llvm::IRBuilder<>& builder, const TimeInterval& tiOther) const;

        [[nodiscard]] llvm::Value* hashCode(llvm::IRBuilder<>& builder) const;

        struct MaybeTimeInterval;

        static MaybeTimeInterval intersection(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static MaybeTimeInterval unionOf(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);

        struct DifferenceResult;

        static DifferenceResult difference(llvm::IRBuilder<>& builder, const TimeInterval& tiBase, const TimeInterval& tiSub);

    private:
        TimePoint mStart;
        TimePoint mEnd;

        TimeInterval(TimePoint start, TimePoint end);
    };

    struct TimeInterval::MaybeTimeInterval {
        TimeInterval interval;
        llvm::Value* valid = nullptr;
    };

    struct TimeInterval::DifferenceResult {
        MaybeTimeInterval first;
        MaybeTimeInterval second;
    };
}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_INTERVAL_HPP