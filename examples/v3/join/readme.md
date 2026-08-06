## Interval Join Example

### Requirements
Needs a two .ndjson (not .json!) files (left.ndjson and right.ndjson) near the executable

### Process
Reads data from both .ndjson files with two ReadOperators.
Both ReadOperators pass the data to the IntervalJoinOperator.
The IntervalJoinOperator passes the joined data to the PrintOperator.

### Running System
Run was inside an GitHub Runner

### Output
```text
1785453975197] 1 Alice Wright
1785453975203] 2 Johann Jauernig
1785453975208] 3 Bob Bright
```

### LLVM IR [Raw]
```text
; ModuleID = 'example_read_json'
source_filename = "example_read_json"

@join_left_side_types_2 = internal constant [3 x i8] c"\04\04\14"
@join_right_side_types_2 = internal constant [2 x i8] c"\04\14"
@ody_state_join_2 = internal global ptr null
@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_l_id = private unnamed_addr constant [5 x i8] c"l_id\00", align 1
@field_first_name = private unnamed_addr constant [11 x i8] c"first_name\00", align 1
@field_id.1 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_last_name = private unnamed_addr constant [10 x i8] c"last_name\00", align 1

define void @ody_init(ptr %typeRegistry) {
entry:
  %0 = call ptr @rtJoinCreate(ptr %typeRegistry, ptr @join_left_side_types_2, i64 3, ptr @join_right_side_types_2, i64 2, i64 10, i64 10, i8 1, i64 1, i64 0, i8 4)
  store ptr %0, ptr @ody_state_join_2, align 8
  ret void
}

define void @ody_shutdown() {
entry:
  %0 = load ptr, ptr @ody_state_join_2, align 8
  call void @rtJoinDestroy(ptr %0)
  ret void
}

declare ptr @rtJoinCreate(ptr, ptr, i64, ptr, i64, i64, i64, i8, i64, i64, i8)

declare void @rtJoinDestroy(ptr)

define void @ody_rsource_0() {
  br label %join_match_header

read_exit:                                        ; preds = %read_loop
  ret void

join_match_header:                                ; preds = %join_predicate_merge, %read_produce
  %10 = load i64, ptr %Stg.Typ.4, align 4
  %11 = icmp slt i64 %10, %9
  br i1 %11, label %join_match_body, label %join_match_exit

join_match_body:                                  ; preds = %join_match_header
  %12 = call ptr @rtJoinGetCandidatePtr(ptr %8, i64 %10, i64 0)
  %13 = load i64, ptr %12, align 4
  %14 = call ptr @rtJoinGetCandidatePtr(ptr %8, i64 %10, i64 1)
  %15 = load i64, ptr %14, align 4
  %16 = call ptr @rtJoinGetCandidatePtr(ptr %8, i64 %10, i64 2)
  %17 = load ptr, ptr %16, align 8
  %18 = call i64 @rtJoinGetCandidateTimestamp(ptr %8, i64 %10)
  %19 = icmp sgt i64 %7, %18
  %20 = select i1 %19, i64 %7, i64 %18
  %21 = icmp eq i64 %15, %5
  br i1 %21, label %join_predicate_pass, label %join_predicate_merge

join_match_exit:                                  ; preds = %join_match_header
  call void @rtJoinEvict(ptr %8, i64 %7, i1 false)
  call void @rtJoinUnlock(ptr %8)
  br label %read_loop

join_predicate_pass:                              ; preds = %join_match_body
  call void @ody_fanin_exit_2_0(i64 %20, i64 %13, i64 %15, ptr %17, i64 %5, ptr %6)
  br label %join_predicate_merge

join_predicate_merge:                             ; preds = %join_predicate_pass, %join_match_body
  %22 = add i64 %10, 1
  store i64 %22, ptr %Stg.Typ.4, align 4
  br label %join_match_header
}

declare void @rtJoinBeginRight(ptr)

declare void @rtJoinPushRight(ptr, i64, ptr)

declare void @rtJoinEndRight(ptr, i64)

define void @ody_end_phase() {
basicEntry:
  ret void
}

define void @ody_main() {
basicEntry:
  %0 = call i64 @rtIOTaskSpawn(ptr @ody_rsource_0)
  %1 = call i64 @rtIOTaskSpawn(ptr @ody_rsource_1)
  call void @rtIOTaskJoin(i64 %0)
  call void @rtIOTaskJoin(i64 %1)
  call void @ody_end_phase()
  call void @rtEnd()
  ret void
}

declare i64 @rtIOTaskSpawn(ptr)

declare void @rtIOTaskJoin(i64)

declare i64 @rtComputeTaskSpawn(ptr)

declare void @rtComputeTaskJoin(i64)

declare void @rtEnd()
```

### LLVM IR [Optimized]
```text
; ModuleID = 'example_read_json'
source_filename = "example_read_json"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

@join_left_side_types_2 = internal constant [3 x i8] c"\04\04\14"
@join_right_side_types_2 = internal constant [2 x i8] c"\04\14"
@ody_state_join_2 = internal unnamed_addr global ptr null
@field_l_id = private unnamed_addr constant [5 x i8] c"l_id\00", align 1
@field_first_name = private unnamed_addr constant [11 x i8] c"first_name\00", align 1
@field_id.1 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_last_name = private unnamed_addr constant [10 x i8] c"last_name\00", align 1

define void @ody_init(ptr %typeRegistry) local_unnamed_addr {
entry:
  %0 = tail call ptr @rtJoinCreate(ptr %typeRegistry, ptr nonnull @join_left_side_types_2, i64 3, ptr nonnull @join_right_side_types_2, i64 2, i64 10, i64 10, i8 1, i64 1, i64 0, i8 4)
  store ptr %0, ptr @ody_state_join_2, align 8
  ret void
}

define void @ody_shutdown() local_unnamed_addr {
entry:
  %0 = load ptr, ptr @ody_state_join_2, align 8
  tail call void @rtJoinDestroy(ptr %0)
  ret void
read_exit:                                        ; preds = %join_match_exit, %basicEntry
  ret void

join_match_body:                                  ; preds = %read_produce, %join_predicate_merge
  %storemerge3 = phi i64 [ %24, %join_predicate_merge ], [ 0, %read_produce ]
  %11 = call ptr @rtJoinGetCandidatePtr(ptr %8, i64 %storemerge3, i64 0)
  %12 = load i64, ptr %11, align 4
  %13 = call ptr @rtJoinGetCandidatePtr(ptr %8, i64 %storemerge3, i64 1)
  %14 = load i64, ptr %13, align 4
  %15 = call ptr @rtJoinGetCandidatePtr(ptr %8, i64 %storemerge3, i64 2)
  %16 = load ptr, ptr %15, align 8
  %17 = call i64 @rtJoinGetCandidateTimestamp(ptr %8, i64 %storemerge3)
  %18 = icmp eq i64 %14, %5
  br i1 %18, label %join_predicate_pass, label %join_predicate_merge

join_match_exit:                                  ; preds = %join_predicate_merge, %read_produce
  call void @rtJoinEvict(ptr %8, i64 %7, i1 false)
  call void @rtJoinUnlock(ptr %8)
  %19 = call i1 @rtIOSourceHasNext(i64 0)
  call void @rtSleepMs(i64 2)
  %20 = call i1 @odyStop()
  %21 = xor i1 %20, true
  %22 = and i1 %19, %21
  br i1 %22, label %read_produce, label %read_exit

join_predicate_pass:                              ; preds = %join_match_body
  %23 = call i64 @llvm.smax.i64(i64 %7, i64 %17)
  call void @rtOdyPrintTimeStamp(i64 %23)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintInt(i64 %12)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintString(ptr %16)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintString(ptr %6)
  call void @rtOdyPrintNewline()
  br label %join_predicate_merge

join_predicate_merge:                             ; preds = %join_predicate_pass, %join_match_body
  %24 = add nuw nsw i64 %storemerge3, 1
  store i64 %24, ptr %Stg.Typ.4, align 8
  %exitcond.not = icmp eq i64 %24, %9
  br i1 %exitcond.not, label %join_match_exit, label %join_match_body
}

declare void @rtJoinBeginRight(ptr) local_unnamed_addr

declare void @rtJoinPushRight(ptr, i64, ptr) local_unnamed_addr

declare void @rtJoinEndRight(ptr, i64) local_unnamed_addr

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @ody_end_phase() local_unnamed_addr #0 {
basicEntry:
  ret void
}

define void @ody_main() local_unnamed_addr {
basicEntry:
  %0 = tail call i64 @rtIOTaskSpawn(ptr nonnull @ody_rsource_0)
  %1 = tail call i64 @rtIOTaskSpawn(ptr nonnull @ody_rsource_1)
  tail call void @rtIOTaskJoin(i64 %0)
  tail call void @rtIOTaskJoin(i64 %1)
  tail call void @rtEnd()
  ret void
}

declare i64 @rtIOTaskSpawn(ptr) local_unnamed_addr

declare void @rtIOTaskJoin(i64) local_unnamed_addr

declare void @rtEnd() local_unnamed_addr

; Function Attrs: nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #1

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
attributes #1 = { nocallback nocreateundeforpoison nofree nosync nounwind speculatable willreturn memory(none) }
```