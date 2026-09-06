#ifndef SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_INTERVAL_HPP
#define SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_INTERVAL_HPP

#include <core/physical/time/time_point.h>

/**
 * Based on Odysseus TimeInterval Class
 */
namespace sierra::odyplan::v3::core::physical::time {
    struct TimeInterval {
        TimePoint start;
        TimePoint end;

        struct MaybeTimeInterval;
        struct DifferenceResult;
        struct MinusResult;

        [[nodiscard]] llvm::Value* isPoint(llvm::IRBuilder<>& builder) const;
        [[nodiscard]] llvm::Value* isEmpty(llvm::IRBuilder<>& builder) const;
        [[nodiscard]] llvm::Value* isZeroLength(llvm::IRBuilder<>& builder) const;
        [[nodiscard]] llvm::Value* includes(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const;

        static llvm::Value* startsBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* startsBeforeOrEqual(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);

        static llvm::Value* totallyBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiTiV, const TimePoint& tpPtn);
        static llvm::Value* totallyBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* totallyAfter(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* overlaps(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);

        [[nodiscard]] llvm::Value* overlaps(llvm::IRBuilder<>& builder, const TimeInterval& tiOther) const;

        static llvm::Value* inside(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* inside(llvm::IRBuilder<>& builder, const TimeInterval& tiTiV, const TimePoint& tpPtn);

        static llvm::Value* areAdjacent(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static llvm::Value* equals(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);

        static MaybeTimeInterval intersection(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);
        static MaybeTimeInterval unionOf(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);

        static DifferenceResult difference(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht);

        static MinusResult minus(llvm::IRBuilder<>& builder, const TimeInterval& tiBase, const TimeInterval& tiSub);
    };

    struct TimeInterval::MaybeTimeInterval {
        TimeInterval interval;
        llvm::Value* valid = nullptr;
    };

    struct TimeInterval::DifferenceResult {
        MaybeTimeInterval lft;
        MaybeTimeInterval rht;
    };

    struct TimeInterval::MinusResult {
        MaybeTimeInterval fst;
        MaybeTimeInterval sec;
    };

}

#endif //SIERRA_ODYPLAN_V3_CORE_PHYSICAL_TIME_TIME_INTERVAL_HPP