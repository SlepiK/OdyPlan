#include <core/physical/time/time_interval.h>

namespace sierra::odyplan::v3::core::physical::time {
    TimeInterval::TimeInterval(const TimePoint start, const TimePoint end)
        : mStart(start), mEnd(end) {
    }

    TimeInterval::TimeInterval(llvm::IRBuilder<>&  /*builder*/, const TimePoint tpStart, const TimePoint tpEnd)
        : TimeInterval(tpStart, tpEnd) {
    }

    TimeInterval::TimeInterval(llvm::IRBuilder<>& builder, TimePoint tpStart)
        : TimeInterval(builder, tpStart, TimePoint::infinity(builder)) {
    }

    TimeInterval::TimeInterval(llvm::IRBuilder<>& builder)
        : TimeInterval(builder, TimePoint::infinity(builder), TimePoint::infinity(builder)) {
    }

    TimeInterval TimeInterval::forever(llvm::IRBuilder<>& builder) {
        return TimeInterval{builder, TimePoint::zero(builder), TimePoint::infinity(builder)};
    }

    const TimePoint &TimeInterval::getStart() const {
        return mStart;
    }

    const TimePoint &TimeInterval::getEnd() const {
        return mEnd;
    }

    void TimeInterval::setStart(llvm::IRBuilder<>&  /*builder*/, const TimePoint tpStart){
        mStart = tpStart;
    }

    void TimeInterval::setEnd(llvm::IRBuilder<>&  /*builder*/, const TimePoint tpEnd) {
        mEnd = tpEnd;
    }

    void TimeInterval::setStartAndEnd(llvm::IRBuilder<> & /*builder*/, const TimePoint tpStart, const TimePoint tpEnd) {
        mStart = tpStart;
        mEnd = tpEnd;
    }

    void TimeInterval::update(llvm::IRBuilder<> &builder, const TimePoint tpStart, const TimePoint tpEnd) {
        setStartAndEnd(builder, tpStart, tpEnd);
    }

    llvm::Value *TimeInterval::isEndInfinite(llvm::IRBuilder<> &builder) const {
        return mEnd.isInfinite(builder);
    }

    llvm::Value *TimeInterval::isZeroLength(llvm::IRBuilder<> &builder) const {
        return mStart.equals(builder, mEnd);
    }

    llvm::Value *TimeInterval::isPoint(llvm::IRBuilder<> &builder) const {
        return mStart.equals(builder, mEnd);
    }

    llvm::Value *TimeInterval::isEmpty(llvm::IRBuilder<> &builder) const {
        return mStart.equals(builder, mEnd);
    }

    llvm::Value *TimeInterval::includes(llvm::IRBuilder<> &builder, const TimePoint &tpPtn) const {
        llvm::Value* bfrEnd = tpPtn.before(builder, mEnd);
        llvm::Value* afrOrEqStart = tpPtn.afterOrEquals(builder, mStart);
        return builder.CreateAnd(bfrEnd, afrOrEqStart);
    }

    TimeInterval TimeInterval::select(llvm::IRBuilder<> &builder, llvm::Value *condition, const TimeInterval &ifTrue, const TimeInterval &ifFalse) {
        return TimeInterval{
            TimePoint::select(builder, condition, ifTrue.mStart, ifFalse.mStart),
            TimePoint::select(builder, condition, ifTrue.mEnd, ifFalse.mEnd)
        };
    }

    llvm::Value *TimeInterval::startBefore(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        return tiLft.mStart.before(builder, tiRht.mStart);
    }

    llvm::Value *TimeInterval::startsBeforeOrEqual(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        return tiLft.mStart.beforeOrEquals(builder, tiRht.mStart);
    }

    llvm::Value *TimeInterval::totallyBefore(llvm::IRBuilder<> &builder, const TimeInterval &tiTiv, const TimePoint &tpPtn) {
        return tiTiv.mEnd.beforeOrEquals(builder, tpPtn);
    }

    llvm::Value *TimeInterval::totallyBefore(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        return totallyBefore(builder, tiLft, tiRht.mStart);
    }

    llvm::Value *TimeInterval::totallyAfter(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        return totallyBefore(builder, tiRht, tiLft);
    }

    llvm::Value *TimeInterval::overlaps(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        llvm::Value* eth = builder.CreateOr(totallyBefore(builder, tiLft, tiRht), totallyAfter(builder, tiLft, tiRht));
        return builder.CreateNot(eth);
    }

    llvm::Value *TimeInterval::overlaps(llvm::IRBuilder<> &builder, const TimeInterval &tiOther) const {
        return overlaps(builder, *this, tiOther);
    }

    llvm::Value *TimeInterval::inside(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        llvm::Value* startOk = tiRht.mStart.beforeOrEquals(builder, tiLft.mStart);
        llvm::Value* endOk = tiLft.mEnd.beforeOrEquals(builder, tiRht.mEnd);
        return builder.CreateAnd(startOk, endOk);
    }

    llvm::Value *TimeInterval::inside(llvm::IRBuilder<> &builder, const TimeInterval &tiTiv, const TimePoint &tpPtn) {
        llvm::Value* startOk = tiTiv.mStart.beforeOrEquals(builder, tpPtn);
        llvm::Value* endOk = tiTiv.mEnd.after(builder, tpPtn);
        return builder.CreateAnd(startOk, endOk);
    }

    llvm::Value *TimeInterval::areAdjacent(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        llvm::Value* lft = tiLft.mEnd.equals(builder, tiRht.mStart);
        llvm::Value* rht = tiRht.mEnd.equals(builder, tiLft.mStart);
        return builder.CreateOr(lft, rht);
    }

    llvm::Value *TimeInterval::equals(llvm::IRBuilder<> &builder, const TimeInterval &tiOther) const {
        llvm::Value* sEq = mStart.equals(builder, tiOther.mStart);
        llvm::Value* eEq = mEnd.equals(builder, tiOther.mEnd);
        return builder.CreateAnd(sEq, eEq);
    }

    llvm::Value *TimeInterval::compareTo(llvm::IRBuilder<> &builder, const TimeInterval &tiOther) const {
        llvm::Value* sCmp = mStart.compareTo(builder, tiOther.mStart);
        llvm::Value* eCmp = mEnd.compareTo(builder, tiOther.mEnd);
        llvm::Value* sIsZero = builder.CreateICmpEQ(sCmp, builder.getInt32(0));
        return builder.CreateSelect(sIsZero, eCmp, sCmp);
    }

    llvm::Value *TimeInterval::hashCode(llvm::IRBuilder<> &builder) const {
        llvm::Value* res = builder.getInt32(1);
        llvm::Value* eHash = mEnd.hash(builder);
        res = builder.CreateAdd(builder.CreateMul(builder.getInt32(31), res), eHash);
        llvm::Value* sHash = mStart.hash(builder);
        res = builder.CreateAdd(builder.CreateMul(builder.getInt32(31), res), sHash);
        return res;
    }

    TimeInterval::MaybeTimeInterval TimeInterval::intersection(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        const TimePoint newStart = TimePoint::max(builder, tiLft.mStart, tiRht.mStart);
        const TimePoint newEnd = TimePoint::min(builder, tiLft.mEnd, tiRht.mEnd);
        llvm::Value* val = builder.CreateAnd(overlaps(builder, tiLft, tiRht), newStart.before(builder, newEnd));
        return MaybeTimeInterval {
            .interval=TimeInterval{
                newStart,
                newEnd
            },
            .valid=val
        };
    }

    TimeInterval::MaybeTimeInterval TimeInterval::unionOf(llvm::IRBuilder<> &builder, const TimeInterval &tiLft, const TimeInterval &tiRht) {
        const TimePoint newStart = TimePoint::min(builder, tiLft.mStart, tiRht.mStart);
        const TimePoint newEnd = TimePoint::max(builder, tiLft.mEnd, tiRht.mEnd);
        return MaybeTimeInterval{
            .interval=TimeInterval{
                newStart,
                newEnd
            },
            .valid=overlaps(builder, tiLft, tiRht)
        };
    }

    TimeInterval::DifferenceResult TimeInterval::difference(llvm::IRBuilder<> &builder, const TimeInterval &tiBase, const TimeInterval &tiSub) {
        llvm::Value* eq = tiBase.equals(builder, tiSub);
        llvm::Value* notOverlap = builder.CreateNot(overlaps(builder, tiBase, tiSub));
        llvm::Value* insideM = inside(builder, tiBase, tiSub);
        llvm::Value* earlyExit = builder.CreateOr(builder.CreateOr(eq, notOverlap), insideM);
        llvm::Value* notEarlyExit = builder.CreateNot(earlyExit);

        llvm::Value* startEq = tiBase.mStart.equals(builder, tiSub.mStart);
        llvm::Value* startBfrStart = tiBase.mStart.before(builder, tiSub.mStart);
        llvm::Value* endSBfrEnd = tiSub.mEnd.before(builder, tiBase.mEnd);
        llvm::Value* endBfrEnd = tiBase.mEnd.equals(builder, tiSub.mEnd);

        const TimePoint cAStart = TimePoint::select(builder, endSBfrEnd, tiSub.mEnd, tiBase.mEnd);
        const TimePoint cAEnd = TimePoint::select(builder, endSBfrEnd, tiBase.mEnd, tiSub.mEnd);

        const TimePoint s0Start = TimePoint::select(builder, startEq, cAStart, TimePoint::select(builder, startBfrStart, tiBase.mStart, tiSub.mStart));
        const TimePoint s0End = TimePoint::select(builder, startEq, cAEnd, TimePoint::select(builder, startBfrStart, tiSub.mStart, tiBase.mStart));

        llvm::Value* s0Val = notEarlyExit;

        llvm::Value* s1ValNotA = builder.CreateSelect(startBfrStart, endSBfrEnd, endBfrEnd);
        llvm::Value* s1ValRaw = builder.CreateSelect(startEq, builder.getInt1(false), s1ValNotA);
        llvm::Value* s1Val = builder.CreateAnd(s1ValRaw, notOverlap);

        const TimePoint s1Start = TimePoint::select(builder, startBfrStart, tiSub.mEnd, tiBase.mEnd);
        const TimePoint s1End = TimePoint::select(builder, startBfrStart, tiBase.mEnd, tiSub.mEnd);

        return DifferenceResult{
          .first=MaybeTimeInterval{
              .interval=TimeInterval{
                  s0Start,
                  s0End
              },
              .valid=s0Val
          },
            .second=MaybeTimeInterval{
                .interval=TimeInterval{
                    s1Start,
                    s1End
                },
                .valid=s1Val
            }
        };
    }
}