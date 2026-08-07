## Read NDJSON Example

### Requirements
Needs a read.ndjson (not .json!) near the executable

### Process
Reads read.ndjson and passes it to the PrintOperator

### Running System
Run was inside an GitHub Runner

### Output
```text
10930757] 1 3.14 Alice
10930757] 1 3.14 Alice
10984618] 2 2.71 Bob
10984618] 2 2.71 Bob
11035895] 3 1.41 Charlie
11035895] 3 1.41 Charlie
```

### LLVM IR [Raw]
```text
; ModuleID = 'example_read_ndjson'
source_filename = "example_read_ndjson"

@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name = private unnamed_addr constant [5 x i8] c"name\00", align 1

define void @ody_init(ptr %typeRegistry) {
entry:
  ret void
}

define void @ody_shutdown() {
entry:
  ret void
}

define void @ody_rsource_0() {
basicEntry:
  br label %read_loop

read_loop:                                        ; preds = %read_produce, %basicEntry
  %0 = call i1 @rtIOSourceHasNext(i64 0)
  %1 = call i1 @odyStop()
  %2 = xor i1 %1, true
  %3 = and i1 %0, %2
  br i1 %3, label %read_produce, label %read_exit

read_produce:                                     ; preds = %read_loop
  %4 = call ptr @rtIOSourceGetField(i64 0, ptr @field_id)
  %5 = call i64 @rtParseInt64(ptr %4)
  %6 = call ptr @rtIOSourceGetField(i64 0, ptr @field_value)
  %7 = call double @rtParseDouble(ptr %6)
  %8 = call ptr @rtIOSourceGetField(i64 0, ptr @field_name)
  %9 = call i64 @rtNow()
  call void @rtOdyPrintTimeStamp(i64 %9)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintInt(i64 %5)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintDouble(double %7)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintString(ptr %8)
  call void @rtOdyPrintNewline()
  call void @rtOdyPrintTimeStamp(i64 %9)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintInt(i64 %5)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintDouble(double %7)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintString(ptr %8)
  call void @rtOdyPrintNewline()
  br label %read_loop

read_exit:                                        ; preds = %read_loop
  ret void
}

declare i1 @rtIOSourceHasNext(i64)

declare i1 @odyStop()

declare ptr @rtIOSourceGetField(i64, ptr)

declare i64 @rtParseInt64(ptr)

declare double @rtParseDouble(ptr)

declare i64 @rtNow()

declare void @rtOdyPrintTimeStamp(i64)

declare void @rtOdyPrintSeperator()

declare void @rtOdyPrintInt(i64)

declare void @rtOdyPrintDouble(double)

declare void @rtOdyPrintString(ptr)

declare void @rtOdyPrintNewline()

define void @ody_end_phase() {
basicEntry:
  ret void
}

define void @ody_main() {
basicEntry:
  %0 = call i64 @rtIOTaskSpawn(ptr @ody_rsource_0)
  call void @rtIOTaskJoin(i64 %0)
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
; ModuleID = 'example_read_ndjson'
source_filename = "example_read_ndjson"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name = private unnamed_addr constant [5 x i8] c"name\00", align 1

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @ody_init(ptr readnone captures(none) %typeRegistry) local_unnamed_addr #0 {
entry:
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @ody_shutdown() local_unnamed_addr #0 {
entry:
  ret void
}

define void @ody_rsource_0() {
basicEntry:
  %0 = tail call i1 @rtIOSourceHasNext(i64 0)
  %1 = tail call i1 @odyStop()
  %2 = xor i1 %1, true
  %3 = and i1 %0, %2
  br i1 %3, label %read_produce, label %read_exit

read_produce:                                     ; preds = %basicEntry, %read_produce
  %4 = tail call ptr @rtIOSourceGetField(i64 0, ptr nonnull @field_id)
  %5 = tail call i64 @rtParseInt64(ptr %4)
  %6 = tail call ptr @rtIOSourceGetField(i64 0, ptr nonnull @field_value)
  %7 = tail call double @rtParseDouble(ptr %6)
  %8 = tail call ptr @rtIOSourceGetField(i64 0, ptr nonnull @field_name)
  %9 = tail call i64 @rtNow()
  tail call void @rtOdyPrintTimeStamp(i64 %9)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintInt(i64 %5)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintDouble(double %7)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintString(ptr %8)
  tail call void @rtOdyPrintNewline()
  tail call void @rtOdyPrintTimeStamp(i64 %9)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintInt(i64 %5)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintDouble(double %7)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintString(ptr %8)
  tail call void @rtOdyPrintNewline()
  %10 = tail call i1 @rtIOSourceHasNext(i64 0)
  %11 = tail call i1 @odyStop()
  %12 = xor i1 %11, true
  %13 = and i1 %10, %12
  br i1 %13, label %read_produce, label %read_exit

read_exit:                                        ; preds = %read_produce, %basicEntry
  ret void
}

declare i1 @rtIOSourceHasNext(i64) local_unnamed_addr

declare i1 @odyStop() local_unnamed_addr

declare ptr @rtIOSourceGetField(i64, ptr) local_unnamed_addr

declare i64 @rtParseInt64(ptr) local_unnamed_addr

declare double @rtParseDouble(ptr) local_unnamed_addr

declare i64 @rtNow() local_unnamed_addr

declare void @rtOdyPrintTimeStamp(i64) local_unnamed_addr

declare void @rtOdyPrintSeperator() local_unnamed_addr

declare void @rtOdyPrintInt(i64) local_unnamed_addr

declare void @rtOdyPrintDouble(double) local_unnamed_addr

declare void @rtOdyPrintString(ptr) local_unnamed_addr

declare void @rtOdyPrintNewline() local_unnamed_addr

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(none)
define void @ody_end_phase() local_unnamed_addr #0 {
basicEntry:
  ret void
}

define void @ody_main() local_unnamed_addr {
basicEntry:
  %0 = tail call i64 @rtIOTaskSpawn(ptr nonnull @ody_rsource_0)
  tail call void @rtIOTaskJoin(i64 %0)
  tail call void @rtEnd()
  ret void
}

declare i64 @rtIOTaskSpawn(ptr) local_unnamed_addr

declare void @rtIOTaskJoin(i64) local_unnamed_addr

declare void @rtEnd() local_unnamed_addr

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }
```