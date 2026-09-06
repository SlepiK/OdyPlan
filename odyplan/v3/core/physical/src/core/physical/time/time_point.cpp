#include <core/physical/time/time_point.h>
#include <llvm/IR/Module.h>

namespace sierra::odyplan::v3::core::physical::time {
    namespace {
        constexpr std::int64_t INFINITY_SENTINEL = -std::numeric_limits<std::int64_t>::max();

        llvm::GlobalVariable *getOrCreateSeqCnt(llvm::Module &module) {
            const auto *seqCnt = "__tp_seq_cnt";
            if (auto *existing = module.getGlobalVariable((seqCnt))) {
                return existing;
            }

            auto *i64Ty = llvm::Type::getInt64Ty(module.getContext());

            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            return new llvm::GlobalVariable(
                module,
                i64Ty,
                false,
                llvm::GlobalVariable::InternalLinkage,
                llvm::ConstantInt::get(i64Ty, 0),
                seqCnt
            );
        }

        llvm::Value *nxtSeqNr(llvm::IRBuilder<> &builder, llvm::Module &module, const bool threadSafe) {
            llvm::GlobalVariable *seqCnt = getOrCreateSeqCnt(module);
            auto *i64Ty = llvm::Type::getInt64Ty(module.getContext());

            if (threadSafe) {
                return builder.CreateAtomicRMW(
                    llvm::AtomicRMWInst::Add, seqCnt, builder.getInt64(1),
                    llvm::MaybeAlign(8), llvm::AtomicOrdering::SequentiallyConsistent,
                    llvm::SyncScope::System
                );
            }

            llvm::Value *old = builder.CreateLoad(i64Ty, seqCnt);
            llvm::Value *upd = builder.CreateAdd(old, builder.getInt64(1));

            builder.CreateStore(upd, seqCnt);
            return old;
        }

        llvm::Value *mbNwTiBreaker(llvm::IRBuilder<> &builder, llvm::Module &module, const bool track,
                                   const bool threadSafe) {
            if (!track) {
                return nullptr;
            }

            return nxtSeqNr(builder, module, threadSafe);
        }
    }

    TimePoint TimePoint::infinity(llvm::IRBuilder<> &builder, llvm::Module &module, const bool track,
                                  const bool threadSafe) {
        return TimePoint{
            .point = builder.getInt64(INFINITY_SENTINEL),
            .isInfinity = builder.getInt1(true),
            .tiBreaker = mbNwTiBreaker(builder, module, track, threadSafe)
        };
    }

    TimePoint TimePoint::fromValue(llvm::IRBuilder<> &builder, llvm::Module &module, llvm::Value *rtPoint,
                                   const bool track, const bool threadSafe) {
        return TimePoint{
            .point = rtPoint,
            .isInfinity = builder.getInt1(false),
            .tiBreaker = mbNwTiBreaker(builder, module, track, threadSafe)
        };
    }

    TimePoint TimePoint::constant(llvm::IRBuilder<> &builder, llvm::Module &module, const std::int64_t point,
                                  const bool track, const bool threadSafe) {
        return fromValue(builder, module,
                         llvm::ConstantInt::getSigned(llvm::Type::getInt64Ty(builder.getContext()), point), track,
                         threadSafe);
    }

    TimePoint TimePoint::zero(llvm::IRBuilder<> &builder, llvm::Module &module, const bool track,
                              const bool threadSafe) {
        return constant(builder, module, 0, track, threadSafe);
    }

    TimePoint TimePoint::select(llvm::IRBuilder<>& builder, llvm::Value* condition, const TimePoint& ifTrue, const TimePoint& ifFalse) {
        assert((ifTrue.tiBreaker == nullptr) && (ifFalse.tiBreaker == nullptr) && "both points needs to track or none");

        llvm::Value* tb = (ifTrue.tiBreaker != nullptr) ? builder.CreateSelect(condition, ifTrue.tiBreaker, ifFalse.tiBreaker) : nullptr;

        return TimePoint{
            .point = builder.CreateSelect(condition, ifTrue.point, ifFalse.point),
            .isInfinity = builder.CreateSelect(condition, ifTrue.isInfinity, ifFalse.isInfinity),
            .tiBreaker = tb
        };
    }

    llvm::Value *TimePoint::before(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        //signedLeft < signedRight
        llvm::Value *cmpSignLft = builder.CreateICmpSLT(point, tpOther.point);
        //right.isInfinity ? ret true : cmpSignLft
        llvm::Value *ifNotSignLftIsInf = builder.CreateSelect(tpOther.isInfinity, builder.getInt1(true), cmpSignLft);
        //left.isInfinity ? ret false : ifNotSignLftIsInf
        return builder.CreateSelect(isInfinity, builder.getInt1(false), ifNotSignLftIsInf);
    }

    llvm::Value *TimePoint::beforeOrEquals(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        //signedLeft <= signedRight
        llvm::Value *cmpSignLftEq = builder.CreateICmpSLE(point, tpOther.point);
        //right.isInfinity ? ret true : cmpSignLftEq
        llvm::Value *ifNotSignLftIsInf = builder.CreateSelect(tpOther.isInfinity, builder.getInt1(true), cmpSignLftEq);
        //left.isInfinity ? ret false : ifNotSignLftIsInf
        return builder.CreateSelect(isInfinity, builder.getInt1(false), ifNotSignLftIsInf);
    }

    llvm::Value *TimePoint::after(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        return tpOther.before(builder, *this);
    }

    llvm::Value *TimePoint::afterOrEquals(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        return tpOther.beforeOrEquals(builder, *this);
    }

    llvm::Value *TimePoint::equals(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        //signedLeft == signedRight
        llvm::Value *cmpSignEq = builder.CreateICmpEQ(point, tpOther.point);
        //right.isInfinity ? : return false : cmpSignEq
        llvm::Value *ifNotSignLftInf = builder.CreateSelect(tpOther.isInfinity, builder.getInt1(false), cmpSignEq);
        //left.isInfinity ? : ret right.isInfinity : ifNotSignLftInf
        return builder.CreateSelect(isInfinity, tpOther.isInfinity, ifNotSignLftInf);
    }

    llvm::Value *TimePoint::compareTo(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        llvm::Value *thisBfrOth = before(builder, tpOther);
        llvm::Value *othBfrThis = tpOther.before(builder, *this);
        llvm::Value *ifThisBfr = builder.CreateSelect(thisBfrOth, builder.getInt32(-1), builder.getInt32(0));
        return builder.CreateSelect(othBfrThis, builder.getInt32(1), ifThisBfr);
    }

    llvm::Value *TimePoint::compareOrdTo(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        assert(tiBreaker && tpOther.tiBreaker && "both points needs to track");

        llvm::Value *cmp = compareTo(builder, tpOther);
        llvm::Value *isZero = builder.CreateICmpEQ(cmp, builder.getInt32(0));

        llvm::Value *cmpTbLft = builder.CreateICmpSLT(tiBreaker, tpOther.tiBreaker);
        llvm::Value *cmpTbRgh = builder.CreateICmpSGT(tiBreaker, tpOther.tiBreaker);
        llvm::Value *cmpTb = builder.CreateSelect(cmpTbLft, builder.getInt32(-1),
                                                  builder.CreateSelect(cmpTbRgh, builder.getInt32(1),
                                                                       builder.getInt32(0)));

        return builder.CreateSelect(isZero, cmpTb, cmp);
    }

    TimePoint TimePoint::min(llvm::IRBuilder<> &builder, const TimePoint &tpA, const TimePoint &tpB) {
        assert(
            (tpA.tiBreaker == nullptr) == (tpB.tiBreaker == nullptr) && "both points needs to track or to not track");

        llvm::Value *aBefB = tpA.before(builder, tpB);
        llvm::Value *tB = (tpA.tiBreaker != nullptr)
                              ? builder.CreateSelect(aBefB, tpA.tiBreaker, tpB.tiBreaker)
                              : nullptr;

        return TimePoint{
            .point = builder.CreateSelect(aBefB, tpA.point, tpB.point),
            .isInfinity = builder.CreateSelect(aBefB, tpA.isInfinity, tpB.isInfinity),
            .tiBreaker = tB
        };
    }

    TimePoint TimePoint::max(llvm::IRBuilder<> &builder, const TimePoint &tpA, const TimePoint &tpB) {
        assert(
            (tpA.tiBreaker == nullptr) == (tpB.tiBreaker == nullptr) && "both points needs to track or to not track");

        llvm::Value *aBefB = tpA.before(builder, tpB);
        llvm::Value *tB = (tpA.tiBreaker != nullptr)
                              ? builder.CreateSelect(aBefB, tpB.tiBreaker, tpA.tiBreaker)
                              : nullptr;

        return TimePoint{
            .point = builder.CreateSelect(aBefB, tpB.point, tpA.point),
            .isInfinity = builder.CreateSelect(aBefB, tpB.isInfinity, tpA.isInfinity),
            .tiBreaker = tB
        };
    }

    TimePoint TimePoint::add(llvm::IRBuilder<> &builder, llvm::Module &module, llvm::Value *rtDelta,
                             const bool threadSafe) const {
        llvm::Value *sum = builder.CreateAdd(point, rtDelta);
        llvm::Value *pnt = builder.CreateSelect(isInfinity, point, sum);

        llvm::Value *tB = nullptr;
        if (tiBreaker != nullptr) {
            llvm::Value *upd = nxtSeqNr(builder, module, threadSafe);
            tB = builder.CreateSelect(isInfinity, tiBreaker, upd);
        }

        return TimePoint{
            .point = pnt,
            .isInfinity = isInfinity,
            .tiBreaker = tB
        };
    }

    TimePoint TimePoint::sub(llvm::IRBuilder<> &builder, llvm::Module &module, llvm::Value *rtDelta,
                             const bool threadSafe) const {
        llvm::Value *diff = builder.CreateSub(point, rtDelta);
        llvm::Value *pnt = builder.CreateSelect(isInfinity, point, diff);

        llvm::Value *tB = nullptr;
        if (tiBreaker != nullptr) {
            llvm::Value *upd = nxtSeqNr(builder, module, threadSafe);
            tB = builder.CreateSelect(isInfinity, tiBreaker, upd);
        }

        return TimePoint{
            .point = pnt,
            .isInfinity = isInfinity,
            .tiBreaker = tB
        };
    }
}
