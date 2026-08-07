## Count Window Example

### Requirements
Needs a read.ndjson (not .json!) near the executable

### Process
Reads a file (ndjson) and passes it through the CountWindow to the PrintOperator

### Running System
Run was inside an GitHub Runner

### Output
```text
1785453919648] 1 3.14 Alice
1785453919648] 2 2.71 Bob
1785453919648] 3 1.41 Charlie
```

### LLVM IR [Raw]
```text
; ModuleID = 'example_count_window'
source_filename = "example_count_window"

@count_window_types_1 = internal constant [3 x i8] c"\04\0B\14"
@ody_state_count_window_1 = internal global ptr null
@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name = private unnamed_addr constant [5 x i8] c"name\00", align 1

define void @ody_init(ptr %typeRegistry) {
entry:
  %0 = call ptr @rtCreateCountWindowBuffer(ptr %typeRegistry, i64 1, i64 1, ptr @count_window_types_1, i64 3)
  store ptr %0, ptr @ody_state_count_window_1, align 8
  ret void
}

define void @ody_shutdown() {
entry:
  %0 = load ptr, ptr @ody_state_count_window_1, align 8
  call void @rtDestroyCountWindowBuffer(ptr %0)
  ret void
}

declare ptr @rtCreateCountWindowBuffer(ptr, i64, i64, ptr, i64)

declare void @rtDestroyCountWindowBuffer(ptr)

define void @ody_rsource_0() {
basicEntry:
  %Stg.Typ.20 = alloca ptr, align 8
  %Stg.Typ.11 = alloca double, align 8
  %14 = add i64 %1, 1
  store i64 %14, ptr %Stg.Typ.4, align 4
  br label %windowHeader
}

declare i64 @rtWindowGetCount(ptr)

declare i64 @rtWindowGetRowCountInWindow(ptr, i64)

declare i64 @rtWindowGetTimestamp(ptr, i64, i64)

declare ptr @rtWindowGetData(ptr, i64, i64, i64)

declare void @rtOdyPrintTimeStamp(i64)

declare void @rtOdyPrintSeperator()

declare void @rtOdyPrintInt(i64)

declare void @rtOdyPrintDouble(double)

declare void @rtOdyPrintString(ptr)

declare void @rtOdyPrintNewline()

define void @ody_end_phase() {
basicEntry:
  %0 = load ptr, ptr @ody_state_count_window_1, align 8
  call void @rtWindowFlush(ptr %0)
  call void @ody_window_drain_1(ptr %0)
  ret void
}

declare void @rtWindowFlush(ptr)

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
; ModuleID = 'example_count_window'
source_filename = "example_count_window"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

@count_window_types_1 = internal constant [3 x i8] c"\04\0B\14"
@ody_state_count_window_1 = internal unnamed_addr global ptr null
@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name = private unnamed_addr constant [5 x i8] c"name\00", align 1

define void @ody_init(ptr %typeRegistry) local_unnamed_addr {
entry:
  %0 = tail call ptr @rtCreateCountWindowBuffer(ptr %typeRegistry, i64 1, i64 1, ptr nonnull @count_window_types_1, i64 3)
  store ptr %0, ptr @ody_state_count_window_1, align 8
  ret void
}

define void @ody_shutdown() local_unnamed_addr {
entry:
  %0 = load ptr, ptr @ody_state_count_window_1, align 8
  tail call void @rtDestroyCountWindowBuffer(ptr %0)
  ret void
}

declare ptr @rtCreateCountWindowBuffer(ptr, i64, i64, ptr, i64) local_unnamed_addr

declare void @rtDestroyCountWindowBuffer(ptr) local_unnamed_addr

define void @ody_rsource_0() {
basicEntry:
  %Stg.Typ.20 = alloca ptr, align 8
  %Stg.Typ.11 = alloca double, align 8
  %Stg.Typ.4 = alloca i64, align 8
  %0 = tail call i1 @rtIOSourceHasNext(i64 0)
  %1 = tail call i1 @odyStop()
  %2 = xor i1 %1, true
  %3 = and i1 %0, %2
  br i1 %3, label %read_produce, label %read_exit

read_produce:                                     ; preds = %basicEntry, %read_produce
  %1 = icmp sgt i64 %0, 0
  br i1 %1, label %windowBody, label %windowExit

windowBody:                                       ; preds = %drainEntry, %windowRowExit
  %Stg.Typ.4.03 = phi i64 [ %12, %windowRowExit ], [ 0, %drainEntry ]
  %2 = tail call i64 @rtWindowGetRowCountInWindow(ptr %windowHandle, i64 %Stg.Typ.4.03)
  %3 = icmp sgt i64 %2, 0
  br i1 %3, label %windowRowBody, label %windowRowExit

windowExit:                                       ; preds = %windowRowExit, %drainEntry
  ret void

windowRowBody:                                    ; preds = %windowBody, %windowRowBody
  %Stg.Typ.4.12 = phi i64 [ %11, %windowRowBody ], [ 0, %windowBody ]
  %4 = tail call i64 @rtWindowGetTimestamp(ptr %windowHandle, i64 %Stg.Typ.4.03, i64 %Stg.Typ.4.12)
  %5 = tail call ptr @rtWindowGetData(ptr %windowHandle, i64 %Stg.Typ.4.03, i64 %Stg.Typ.4.12, i64 0)
  %6 = load i64, ptr %5, align 4
  %7 = tail call ptr @rtWindowGetData(ptr %windowHandle, i64 %Stg.Typ.4.03, i64 %Stg.Typ.4.12, i64 1)
  %8 = load double, ptr %7, align 8
  %9 = tail call ptr @rtWindowGetData(ptr %windowHandle, i64 %Stg.Typ.4.03, i64 %Stg.Typ.4.12, i64 2)
  %10 = load ptr, ptr %9, align 8
  tail call void @rtOdyPrintTimeStamp(i64 %4)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintInt(i64 %6)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintDouble(double %8)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintString(ptr %10)
  tail call void @rtOdyPrintNewline()
  %11 = add nuw nsw i64 %Stg.Typ.4.12, 1
  %exitcond.not = icmp eq i64 %11, %2
  br i1 %exitcond.not, label %windowRowExit, label %windowRowBody

windowRowExit:                                    ; preds = %windowRowBody, %windowBody
  %12 = add nuw nsw i64 %Stg.Typ.4.03, 1
  %exitcond4.not = icmp eq i64 %12, %0
  br i1 %exitcond4.not, label %windowExit, label %windowBody
}

declare i64 @rtWindowGetCount(ptr) local_unnamed_addr

declare i64 @rtWindowGetRowCountInWindow(ptr, i64) local_unnamed_addr

declare i64 @rtWindowGetTimestamp(ptr, i64, i64) local_unnamed_addr

declare ptr @rtWindowGetData(ptr, i64, i64, i64) local_unnamed_addr

declare void @rtOdyPrintTimeStamp(i64) local_unnamed_addr

declare void @rtOdyPrintSeperator() local_unnamed_addr

declare void @rtOdyPrintInt(i64) local_unnamed_addr

declare void @rtOdyPrintDouble(double) local_unnamed_addr

declare void @rtOdyPrintString(ptr) local_unnamed_addr

declare void @rtOdyPrintNewline() local_unnamed_addr

define void @ody_end_phase() local_unnamed_addr {
basicEntry:
  %0 = load ptr, ptr @ody_state_count_window_1, align 8
  tail call void @rtWindowFlush(ptr %0)
  tail call fastcc void @ody_window_drain_1(ptr %0)
  ret void
}

declare void @rtWindowFlush(ptr) local_unnamed_addr

define void @ody_main() local_unnamed_addr {
basicEntry:
  %0 = tail call i64 @rtIOTaskSpawn(ptr nonnull @ody_rsource_0)
  tail call void @rtIOTaskJoin(i64 %0)
  %1 = load ptr, ptr @ody_state_count_window_1, align 8
  tail call void @rtWindowFlush(ptr %1)
  tail call fastcc void @ody_window_drain_1(ptr %1)
  tail call void @rtEnd()
  ret void
}

declare i64 @rtIOTaskSpawn(ptr) local_unnamed_addr

declare void @rtIOTaskJoin(i64) local_unnamed_addr

declare void @rtEnd() local_unnamed_addr
```