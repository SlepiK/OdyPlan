## Project Example

### Requirements
N/A

### Process
Passes via a custom operator data to the ProjectOperator and prints the result

### Running System
Run was inside an GitHub Runner

### Output
```text
1785453853993] ProjectOperator_Test
```

### LLVM IR [Raw]
```text
; ModuleID = 'exampleProject'
source_filename = "exampleProject"

@0 = private unnamed_addr constant [21 x i8] c"ProjectOperator_Test\00", align 1

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
  %0 = call i64 @rtNow()
  call void @rtOdyPrintTimeStamp(i64 %0)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintString({ ptr, i64 } { ptr @0, i64 20 })
  call void @rtOdyPrintNewline()
  ret void
}

declare i64 @rtNow()

declare void @rtOdyPrintTimeStamp(i64)

declare void @rtOdyPrintSeperator()

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
; ModuleID = 'exampleProject'
source_filename = "exampleProject"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

@0 = private unnamed_addr constant [21 x i8] c"ProjectOperator_Test\00", align 1

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
  %0 = tail call i64 @rtNow()
  tail call void @rtOdyPrintTimeStamp(i64 %0)
  tail call void @rtOdyPrintSeperator()
  tail call void @rtOdyPrintString({ ptr, i64 } { ptr @0, i64 20 })
  tail call void @rtOdyPrintNewline()
  ret void
}

declare i64 @rtNow() local_unnamed_addr

declare void @rtOdyPrintTimeStamp(i64) local_unnamed_addr

declare void @rtOdyPrintSeperator() local_unnamed_addr

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