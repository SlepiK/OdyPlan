## Info
Run was inside of an GitHub Runner 

## Example Output [Raw]
```text
; ModuleID = 'exampleSelect'
source_filename = "exampleSelect"

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
  br i1 true, label %select_pass, label %select_merge

select_pass:                                      ; preds = %basicEntry
  call void @rtOdyPrintTimeStamp(i64 %0)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintInt(i32 5)
  call void @rtOdyPrintNewline()
  br label %select_merge

select_merge:                                     ; preds = %select_pass, %basicEntry
  ret void
}

declare i64 @rtNow()

declare void @rtOdyPrintTimeStamp(i64)

declare void @rtOdyPrintSeperator()

declare void @rtOdyPrintInt(i32)

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

## Example Output [Optimized]
```text
; ModuleID = 'exampleSelect'
source_filename = "exampleSelect"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

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
  tail call void @rtOdyPrintInt(i32 5)
  tail call void @rtOdyPrintNewline()
  ret void
}

declare i64 @rtNow() local_unnamed_addr

declare void @rtOdyPrintTimeStamp(i64) local_unnamed_addr

declare void @rtOdyPrintSeperator() local_unnamed_addr

declare void @rtOdyPrintInt(i32) local_unnamed_addr

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