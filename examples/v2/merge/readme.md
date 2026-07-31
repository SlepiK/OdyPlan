## Merge Example

### Requirements
Needs a two .ndjson (not .json!) files (read.ndjson and read2.ndjson) near the executable

### Process
Reads both .ndjson files with two ReadOperators.
Data is passed from ReadOperators to MergeOperator and from it to the PrintOperator.

### Running System
Run was inside an GitHub Runner

### Output
```text
1785453947100] 1 3.14 Alice
1785453947100] 2 2.71 Bob
1785453947100] 3 1.41 Charlie
1785453947100] 1 3.14 Alice
1785453947100] 2 2.71 Bob
1785453947100] 3 1.41 Charlie
```

### LLVM IR [Raw]
```text
; ModuleID = 'example_merge'
source_filename = "example_merge"

@ody_state_ody_merge_2 = internal global ptr null
@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name = private unnamed_addr constant [5 x i8] c"name\00", align 1
@field_id.1 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value.2 = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name.3 = private unnamed_addr constant [5 x i8] c"name\00", align 1

define void @ody_init(ptr %typeRegistry) {
entry:
  %0 = call ptr @rtCreateMergeLock(ptr %typeRegistry)
  store ptr %0, ptr @ody_state_ody_merge_2, align 8
  ret void
}

define void @ody_shutdown() {
entry:
  %0 = load ptr, ptr @ody_state_ody_merge_2, align 8
  call void @rtDestroyMergeLock(ptr %0)
  ret void
}

declare ptr @rtCreateMergeLock(ptr)

declare void @rtDestroyMergeLock(ptr)

define void @ody_rsource_0() {
basicEntry:

declare i1 @rtIOSourceHasNext(i64)

declare i1 @odyStop()

declare ptr @rtIOSourceGetField(i64, ptr)

declare i64 @rtParseInt64(ptr)

declare double @rtParseDouble(ptr)

declare i64 @rtNow()

declare void @rtMergeLock(ptr)

declare void @rtMergeUnlock(ptr)

define void @ody_rsource_1() {
basicEntry:
  br label %read_loop

read_loop:                                        ; preds = %read_produce, %basicEntry
  %0 = call i1 @rtIOSourceHasNext(i64 0)
  %1 = call i1 @odyStop()
  %2 = xor i1 %1, true
  %3 = and i1 %0, %2
  br i1 %3, label %read_produce, label %read_exit

read_produce:                                     ; preds = %read_loop
  %4 = call ptr @rtIOSourceGetField(i64 0, ptr @field_id.1)
  %5 = call i64 @rtParseInt64(ptr %4)
  %6 = call ptr @rtIOSourceGetField(i64 0, ptr @field_value.2)
  %7 = call double @rtParseDouble(ptr %6)
  %8 = call ptr @rtIOSourceGetField(i64 0, ptr @field_name.3)
  %9 = call i64 @rtNow()
  %10 = load ptr, ptr @ody_state_ody_merge_2, align 8
  call void @rtMergeLock(ptr %10)
  call void @ody_fanin_exit_2_0(i64 %9, i64 %5, double %7, ptr %8)
  call void @rtMergeUnlock(ptr %10)
  br label %read_loop

read_exit:                                        ; preds = %read_loop
  ret void
}

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
; ModuleID = 'example_merge'
source_filename = "example_merge"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

@ody_state_ody_merge_2 = internal unnamed_addr global ptr null
@field_id.1 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value.2 = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name.3 = private unnamed_addr constant [5 x i8] c"name\00", align 1

define void @ody_init(ptr %typeRegistry) local_unnamed_addr {
entry:
  %0 = tail call ptr @rtCreateMergeLock(ptr %typeRegistry)
  store ptr %0, ptr @ody_state_ody_merge_2, align 8
  ret void
}

define void @ody_shutdown() local_unnamed_addr {
entry:
  %0 = load ptr, ptr @ody_state_ody_merge_2, align 8
  tail call void @rtDestroyMergeLock(ptr %0)
  ret void
}

declare void @rtOdyPrintNewline() local_unnamed_addr

declare i1 @rtIOSourceHasNext(i64) local_unnamed_addr

declare i1 @odyStop() local_unnamed_addr

declare ptr @rtIOSourceGetField(i64, ptr) local_unnamed_addr

declare i64 @rtParseInt64(ptr) local_unnamed_addr

declare double @rtParseDouble(ptr) local_unnamed_addr

declare i64 @rtNow() local_unnamed_addr

declare void @rtMergeLock(ptr) local_unnamed_addr

declare void @rtMergeUnlock(ptr) local_unnamed_addr

define void @ody_rsource_1() {
basicEntry:
  %0 = tail call i1 @rtIOSourceHasNext(i64 0)
  %1 = tail call i1 @odyStop()
  %2 = xor i1 %1, true
  %3 = and i1 %0, %2
  br i1 %3, label %read_produce, label %read_exit

read_produce:                                     ; preds = %basicEntry, %read_produce
  %4 = tail call ptr @rtIOSourceGetField(i64 0, ptr nonnull @field_id.1)
  %5 = tail call i64 @rtParseInt64(ptr %4)
  %6 = tail call ptr @rtIOSourceGetField(i64 0, ptr nonnull @field_value.2)
  %7 = tail call double @rtParseDouble(ptr %6)
  %8 = tail call ptr @rtIOSourceGetField(i64 0, ptr nonnull @field_name.3)
  %9 = tail call i64 @rtNow()
  %10 = load ptr, ptr @ody_state_ody_merge_2, align 8
  tail call void @rtMergeLock(ptr %10)
  tail call void @rtOdyPrintTimeStamp(i64 %9)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintInt(i64 %5)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintDouble(double %7)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintString(ptr %8)
  tail call void @rtOdyPrintNewline()
  tail call void @rtMergeUnlock(ptr %10)
  %11 = tail call i1 @rtIOSourceHasNext(i64 0)
  %12 = tail call i1 @odyStop()
  %13 = xor i1 %12, true
  %14 = and i1 %11, %13
  br i1 %14, label %read_produce, label %read_exit

read_exit:                                        ; preds = %read_produce, %basicEntry
  ret void
}

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

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
```