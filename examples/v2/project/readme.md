## Info
Running a three operator based pipeline.
First a operator creating two const values 5 (number) and "ProjectOperator_Test" (string).
Second a project operator to project to only the second incoming field
Third a print operator to print a specific field (after project the original second field is at its view the first one)

## Example Output [LOOP]
```text
Building Logical Plan...
Logical Plan: OK
Building Physical Plan...
Physical Plan: OK
Building LLVM-IR...


; ModuleID = 'exampleProject'
source_filename = "exampleProject"

@0 = private unnamed_addr constant [21 x i8] c"ProjectOperator_Test\00", align 1

define void @ody_segment_0() {
basicEntry:
  br label %loop

loop:                                             ; preds = %produce, %basicEntry
  %0 = call i1 @odyStop()
  br i1 %0, label %exit, label %produce

produce:                                          ; preds = %loop
  call void @rtOdyPrintString(ptr @0, i64 20)
  call void @rtOdyPrintNewline()
  br label %loop

exit:                                             ; preds = %loop
  ret void
}

declare i1 @odyStop()

declare void @rtOdyPrintString(ptr, i64)

declare void @rtOdyPrintNewline()

define void @ody_main() {
basicEntry:
  call void @ody_segment_0()
  ret void
}


LLVM-IR: OK
Running via LLVM-JIT...
Running (press Enter to stop:
ProjectOperator_Test
ProjectOperator_Test
ProjectOperator_Test
ProjectOperator_Test
ProjectOperator_Test
```

## Example Output [NO LOOP]
```text
Building Logical Plan...
Logical Plan: OK
Building Physical Plan...
Physical Plan: OK
Building LLVM-IR...


; ModuleID = 'exampleProject'
source_filename = "exampleProject"

@0 = private unnamed_addr constant [21 x i8] c"ProjectOperator_Test\00", align 1

define void @ody_segment_0() {
basicEntry:
  call void @rtOdyPrintString(ptr @0, i64 20)
  call void @rtOdyPrintNewline()
  ret void
}

declare void @rtOdyPrintString(ptr, i64)

declare void @rtOdyPrintNewline()

define void @ody_main() {
basicEntry:
  call void @ody_segment_0()
  ret void
}


LLVM-IR: OK
Running via LLVM-JIT...
ProjectOperator_Test
```