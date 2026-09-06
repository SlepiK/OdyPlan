#include <core/physical/time/time_interval.h>

namespace sierra::odyplan::v3::core::physical::time {
    llvm::Value* TimeInterval::isPoint(llvm::IRBuilder<>& builder) const {
        return start.equals(builder, end);
    }

    llvm::Value* TimeInterval::isEmpty(llvm::IRBuilder<>& builder) const {
        return start.equals(builder, end);
    }

    llvm::Value* TimeInterval::isZeroLength(llvm::IRBuilder<>& builder) const {
        return start.equals(builder, end);
    }

    llvm::Value* TimeInterval::includes(llvm::IRBuilder<>& builder, const TimePoint& tpOther) const {
        llvm::Value* bfrEnd = tpOther.before(builder, end);
        llvm::Value* afrOrEqStart = tpOther.afterOrEquals(builder, start);
        return builder.CreateAnd(bfrEnd, afrOrEqStart);
    }

    llvm::Value* TimeInterval::startsBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        return tiLft.start.before(builder, tiRht.start);
    }

    llvm::Value* TimeInterval::startsBeforeOrEqual(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        return tiLft.start.beforeOrEquals(builder, tiRht.start);
    }

    llvm::Value* TimeInterval::totallyBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiTiV, const TimePoint& tpPtn) {
        return tiTiV.end.beforeOrEquals(builder, tpPtn);
    }

    llvm::Value* TimeInterval::totallyBefore(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        return totallyBefore(builder, tiLft, tiRht.start);
    }

    llvm::Value* TimeInterval::totallyAfter(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        return totallyBefore(builder, tiRht, tiLft);
    }

    llvm::Value* TimeInterval::overlaps(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        llvm::Value* eth = builder.CreateOr(totallyBefore(builder, tiLft, tiRht), totallyAfter(builder, tiLft, tiRht));
        return builder.CreateNot(eth);
    }

    llvm::Value* TimeInterval::overlaps(llvm::IRBuilder<>& builder, const TimeInterval& tiOther) const {
        return overlaps(builder, *this, tiOther);
    }

    llvm::Value* TimeInterval::inside(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        llvm::Value* chkStart = tiRht.start.beforeOrEquals(builder, tiLft.start);
        llvm::Value* chkEnd = tiLft.end.beforeOrEquals(builder, tiRht.end);
        return builder.CreateAnd(chkStart, chkEnd);
    }

    llvm::Value* TimeInterval::inside(llvm::IRBuilder<>& builder, const TimeInterval& tiTiV, const TimePoint& tpPtn) {
        llvm::Value* chkStart = tiTiV.start.beforeOrEquals(builder, tpPtn);
        llvm::Value* chkEnd = tiTiV.end.after(builder, tpPtn);
        return builder.CreateAnd(chkStart, chkEnd);
    }

    llvm::Value* TimeInterval::areAdjacent(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        llvm::Value* lftEndEqRhtStart = tiLft.end.equals(builder, tiRht.start);
        llvm::Value* rhtEndEqLftStart = tiRht.end.equals(builder, tiLft.start);
        return builder.CreateOr(lftEndEqRhtStart, rhtEndEqLftStart);
    }

    llvm::Value* TimeInterval::equals(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        llvm::Value* startEq = tiLft.start.equals(builder, tiRht.start);
        llvm::Value* endEq = tiLft.end.equals(builder, tiRht.end);
        return builder.CreateAnd(startEq, endEq);
    }

    TimeInterval::MaybeTimeInterval TimeInterval::intersection(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        const TimePoint start = TimePoint::max(builder, tiLft.start, tiRht.start);
        const TimePoint end = TimePoint::max(builder, tiLft.end, tiRht.end);

        llvm::Value* overlapsLftRht = overlaps(builder, tiLft, tiRht);
        llvm::Value* notEmpty = start.before(builder, end);
        llvm::Value* valid = builder.CreateAnd(overlapsLftRht, notEmpty);

        return MaybeTimeInterval{
            .interval = TimeInterval {
                .start = start,
                .end = end
            },
            .valid = valid
        };
    }

    TimeInterval::MaybeTimeInterval TimeInterval::unionOf(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        const TimePoint start = TimePoint::min(builder, tiLft.start, tiRht.start);
        const TimePoint end = TimePoint::max(builder, tiLft.end, tiRht.end);

        llvm::Value* val = overlaps(builder, tiLft, tiRht);

        return MaybeTimeInterval{
            .interval = TimeInterval {
                .start = start,
                .end = end
            },
            .valid = val
        };
    }

    TimeInterval::DifferenceResult TimeInterval::difference(llvm::IRBuilder<>& builder, const TimeInterval& tiLft, const TimeInterval& tiRht) {
        llvm::Value* insideLftRht = inside(builder, tiLft, tiRht);
        llvm::Value* overlapsLftRht = overlaps(builder, tiLft, tiRht);

        llvm::Value* aConOverlap = tiLft.start.before(builder, tiRht.start);
        llvm::Value* aValNotInside = builder.CreateSelect(overlapsLftRht, aConOverlap, builder.getInt1(true));
        llvm::Value* aVal = builder.CreateSelect(insideLftRht, builder.getInt1(false), aValNotInside);

        const TimePoint tp = TimePoint::select(builder, overlapsLftRht, tiRht.start, tiLft.end);
        const TimeInterval aInterval{
            .start = tiLft.start,
            .end = tp
        };

        llvm::Value* bConOverlap = tiLft.end.after(builder, tiRht.end);
        llvm::Value* bValNotInside = builder.CreateSelect(overlapsLftRht, bConOverlap, builder.getInt1(false));
        llvm::Value* bVal = builder.CreateSelect(insideLftRht, builder.getInt1(false), bValNotInside);

        const TimeInterval bInterval{
            .start = tiRht.end,
            .end = tiLft.end
        };

        return DifferenceResult {
            .lft = MaybeTimeInterval{
                .interval = aInterval,
                .valid = aVal
            },
            .rht = MaybeTimeInterval{
                .interval = bInterval,
                .valid = bVal
            }
        };
    }

    TimeInterval::MinusResult TimeInterval::minus(llvm::IRBuilder<>& builder, const TimeInterval& tiBase, const TimeInterval& tiSub) {
        llvm::Value* eq = equals(builder, tiBase, tiSub);
        llvm::Value* notOverlap = builder.CreateNot(overlaps(builder, tiBase, tiSub));
        llvm::Value* insideBase = inside(builder, tiBase, tiSub);
        llvm::Value* earlyExit = builder.CreateOr(builder.CreateOr(eq, notOverlap), insideBase);
        llvm::Value* notEarlyExit = builder.CreateNot(earlyExit);

        llvm::Value* startEq = tiBase.start.equals(builder, tiSub.start);
        llvm::Value* startBfrStart = tiBase.start.before(builder, tiSub.start);

        llvm::Value* endBfrEnd = tiSub.end.before(builder, tiBase.end);
        llvm::Value* endBfrSubEnd = tiBase.end.before(builder, tiSub.end);

        const TimePoint cStart = TimePoint::select(builder, endBfrEnd, tiSub.end, tiBase.end);
        const TimePoint cEnd = TimePoint::select(builder, endBfrEnd, tiBase.end, tiSub.end);

        const TimePoint sAStart = TimePoint::select(builder, startEq, cStart, TimePoint::select(builder, startBfrStart, tiBase.start, tiSub.start));
        const TimePoint sAEnd = TimePoint::select(builder, startEq, cEnd, TimePoint::select(builder, startBfrStart, tiSub.start, tiBase.start));

        llvm::Value* sAVal = notEarlyExit;

        llvm::Value* sBValNotA = builder.CreateSelect(startBfrStart, endBfrEnd, endBfrSubEnd);
        llvm::Value* sBValRaw = builder.CreateSelect(startEq, builder.getInt1(false), sBValNotA);
        llvm::Value* sBVal = builder.CreateAnd(sBValRaw, notEarlyExit);

        const TimePoint sBStart = TimePoint::select(builder, startBfrStart, tiSub.end, tiBase.end);
        const TimePoint sBEnd = TimePoint::select(builder, startBfrStart, tiBase.end, tiSub.end);

        return MinusResult{
            .fst = MaybeTimeInterval{
                .interval = TimeInterval{
                    .start = sAStart,
                    .end = sAEnd
                },
                .valid = sAVal
            },
            .sec = MaybeTimeInterval{
                .interval = TimeInterval{
                    .start = sBStart,
                    .end = sBEnd
                },
                .valid = sBVal
            }
        };
    }
}