#include <core/physical/time/time_point.h>
#include <llvm/IR/Module.h>

namespace sierra::odyplan::v3::core::physical::time {
    namespace {
        llvm::Value* infinitySentinelConstant(llvm::IRBuilder<>& builder) {
            return llvm::ConstantInt::getSigned(builder.getInt64Ty(), INFINITY_SENTINEL);
        }

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

        llvm::Value *nxtSeqNr(llvm::IRBuilder<> &builder, llvm::Module &module) {
            llvm::GlobalVariable *seqCnt = getOrCreateSeqCnt(module);

            return builder.CreateAtomicRMW(
                    llvm::AtomicRMWInst::Add, seqCnt, builder.getInt64(1),
                    llvm::MaybeAlign(8), llvm::AtomicOrdering::SequentiallyConsistent,
                    llvm::SyncScope::System
                );
        }

        llvm::Module& getModFromBdr(const llvm::IRBuilder<>& builder) {
            llvm::BasicBlock* bb = builder.GetInsertBlock();
            llvm::Module* module = bb->getModule();
            return *module;
        }

        llvm::Value *mbNwTiBreaker(llvm::IRBuilder<> &builder) {
            return nxtSeqNr(builder, getModFromBdr(builder));
        }
    }

    TimePoint::TimePoint(llvm::Value* point, llvm::Value* tiBreaker)
        : mPoint(point), mTiBreaker(tiBreaker) {
    }

    TimePoint::TimePoint(llvm::IRBuilder<>& builder, llvm::Value* point)
        : mPoint(point), mTiBreaker(mbNwTiBreaker(builder)) {
    }

    TimePoint::TimePoint(llvm::IRBuilder<>& builder, const TimePoint& tpOther)
        : mPoint(tpOther.mPoint), mTiBreaker(mbNwTiBreaker(builder)) {
    }

    TimePoint TimePoint::infinity(llvm::IRBuilder<> &builder) {
        return TimePoint{
            infinitySentinelConstant(builder),
            mbNwTiBreaker(builder)
        };
    }

    TimePoint TimePoint::zero(llvm::IRBuilder<> &builder) {
        return TimePoint{
            builder.getInt64(0),
            mbNwTiBreaker(builder)
        };
    }

    TimePoint TimePoint::one(llvm::IRBuilder<> &builder) {
        return TimePoint{
            builder.getInt64(1),
            mbNwTiBreaker(builder)
        };
    }

    TimePoint TimePoint::systemTime(llvm::IRBuilder<>& builder) {
        llvm::Module& module = getModFromBdr(builder);
        auto* i64Ty = builder.getInt64Ty();
        const llvm::FunctionCallee fn = module.getOrInsertFunction("rtTimeNow", llvm::FunctionType::get(i64Ty, false));
        llvm::Value* sysTime = builder.CreateCall(fn, {});
        return TimePoint{sysTime, mbNwTiBreaker(builder)};
    }

    TimePoint TimePoint::executionTime(llvm::IRBuilder<>& builder) {
        llvm::Module& module = getModFromBdr(builder);
        auto* i64Ty = builder.getInt64Ty();
        const llvm::FunctionCallee fn = module.getOrInsertFunction("rtProgramTimeNow", llvm::FunctionType::get(i64Ty, false));
        llvm::Value* sysTime = builder.CreateCall(fn, {});
        return TimePoint{sysTime, mbNwTiBreaker(builder)};
    }

    llvm::Value* TimePoint::hash(llvm::IRBuilder<>& builder) const {
        llvm::Value* shifted = builder.CreateLShr(mPoint, builder.getInt64(32));
        llvm::Value* xored = builder.CreateXor(mPoint, shifted);
        llvm::Value* truncated = builder.CreateTrunc(xored, builder.getInt32Ty());
        return builder.CreateAdd(builder.getInt32(31), truncated);
    }

    llvm::Value* TimePoint::getPoint() const {
        return mPoint;
    }

    llvm::Value* TimePoint::isInfinite(llvm::IRBuilder<>& builder) const {
        return builder.CreateICmpEQ(mPoint, infinitySentinelConstant(builder));
    }

    llvm::Value* TimePoint::getTiBreaker() const {
        return mTiBreaker;
    }

    TimePoint TimePoint::add(llvm::IRBuilder<> &builder, llvm::Value *rtDelta) const {
        llvm::Value *isInf = isInfinite(builder);
        llvm::Value *sum = builder.CreateAdd(mPoint, rtDelta);
        llvm::Value *pnt = builder.CreateSelect(isInf, mPoint, sum);
        llvm::Value *upd = mbNwTiBreaker(builder);
        llvm::Value *tB = builder.CreateSelect(isInf, mTiBreaker, upd);

        return TimePoint{
            pnt,
            tB
        };
    }

    TimePoint TimePoint::sub(llvm::IRBuilder<> &builder, llvm::Value *rtDelta) const {
        llvm::Value *isInf = isInfinite(builder);
        llvm::Value *diff = builder.CreateSub(mPoint, rtDelta);
        llvm::Value *pnt = builder.CreateSelect(isInf, mPoint, diff);
        llvm::Value *upd = mbNwTiBreaker(builder);
        llvm::Value *tB = builder.CreateSelect(isInf, mTiBreaker, upd);

        return TimePoint{
            pnt,
            tB
        };
    }

    TimePoint TimePoint::select(llvm::IRBuilder<>& builder, llvm::Value* condition, const TimePoint& ifTrue, const TimePoint& ifFalse) {
        return TimePoint{
            builder.CreateSelect(condition, ifTrue.mPoint, ifFalse.mPoint),
            builder.CreateSelect(condition, ifTrue.mTiBreaker, ifFalse.mTiBreaker),
        };
    }

    llvm::Value *TimePoint::before(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        llvm::Value* lftInf = isInfinite(builder);
        llvm::Value* othInf = tpOther.isInfinite(builder);
        //signedLeft < signedRight
        llvm::Value *cmpSignLft = builder.CreateICmpSLT(mPoint, tpOther.mPoint);
        //right.isInfinity ? ret true : cmpSignLft
        llvm::Value *ifNotSignLftIsInf = builder.CreateSelect(othInf, builder.getInt1(true), cmpSignLft);
        //left.isInfinity ? ret false : ifNotSignLftIsInf
        return builder.CreateSelect(lftInf, builder.getInt1(false), ifNotSignLftIsInf);
    }

    llvm::Value *TimePoint::beforeOrEquals(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        llvm::Value* lftInf = isInfinite(builder);
        llvm::Value* othInf = tpOther.isInfinite(builder);
        //signedLeft <= signedRight
        llvm::Value *cmpSignLftEq = builder.CreateICmpSLE(mPoint, tpOther.mPoint);
        //right.isInfinity ? ret true : cmpSignLftEq
        llvm::Value *ifNotSignLftIsInf = builder.CreateSelect(othInf, builder.getInt1(true), cmpSignLftEq);
        //left.isInfinity ? ret false : ifNotSignLftIsInf
        return builder.CreateSelect(lftInf, builder.getInt1(false), ifNotSignLftIsInf);
    }

    llvm::Value *TimePoint::after(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        return tpOther.before(builder, *this);
    }

    llvm::Value *TimePoint::afterOrEquals(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        return tpOther.beforeOrEquals(builder, *this);
    }

    llvm::Value *TimePoint::equals(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        llvm::Value* lftInf = isInfinite(builder);
        llvm::Value* othInf = tpOther.isInfinite(builder);
        //signedLeft == signedRight
        llvm::Value *cmpSignEq = builder.CreateICmpEQ(mPoint, tpOther.mPoint);
        //right.isInfinity ? : return false : cmpSignEq
        llvm::Value *ifNotSignLftInf = builder.CreateSelect(othInf, builder.getInt1(false), cmpSignEq);
        //left.isInfinity ? : ret right.isInfinity : ifNotSignLftInf
        return builder.CreateSelect(lftInf, othInf, ifNotSignLftInf);
    }

    llvm::Value *TimePoint::compareTo(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        llvm::Value *thisBfrOth = before(builder, tpOther);
        llvm::Value *othBfrThis = tpOther.before(builder, *this);
        llvm::Value *ifThisBfr = builder.CreateSelect(thisBfrOth, builder.getInt32(-1), builder.getInt32(0));
        return builder.CreateSelect(othBfrThis, builder.getInt32(1), ifThisBfr);
    }

    llvm::Value *TimePoint::compareOrdTo(llvm::IRBuilder<> &builder, const TimePoint &tpOther) const {
        llvm::Value *cmp = compareTo(builder, tpOther);
        llvm::Value *isZero = builder.CreateICmpEQ(cmp, builder.getInt32(0));

        llvm::Value *cmpTbLft = builder.CreateICmpSLT(mTiBreaker, tpOther.mTiBreaker);
        llvm::Value *cmpTbRgh = builder.CreateICmpSGT(mTiBreaker, tpOther.mTiBreaker);
        llvm::Value *cmpTb = builder.CreateSelect(cmpTbLft, builder.getInt32(-1),
                                                  builder.CreateSelect(cmpTbRgh, builder.getInt32(1),
                                                                       builder.getInt32(0)));

        return builder.CreateSelect(isZero, cmpTb, cmp);
    }

    TimePoint TimePoint::min(llvm::IRBuilder<> &builder, const TimePoint &tpA, const TimePoint &tpB) {
        return select(builder, tpA.before(builder, tpB), tpA, tpB);
    }

    TimePoint TimePoint::max(llvm::IRBuilder<> &builder, const TimePoint &tpA, const TimePoint &tpB) {
        return select(builder, tpA.before(builder, tpB), tpB, tpA);
    }
}
