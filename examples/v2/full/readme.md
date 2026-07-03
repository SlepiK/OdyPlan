## Info
Dont run the looped-version as check for building the library ;) 

## Example Output [LOOP]
```text
Building Logical Plan...
Logical Plan: OK
Building Physical Plan...
Physical Plan: OK
Building LLVM-IR...


; ModuleID = 'exampleFull'
source_filename = "exampleFull"

define void @ody_segment_0() {
basicEntry:
  br label %loop

loop:                                             ; preds = %produce, %basicEntry
  %0 = call i1 @odyStop()
  br i1 %0, label %exit, label %produce

produce:                                          ; preds = %loop
  call void @odyExamplePrintUint32(i32 5)
  br label %loop

exit:                                             ; preds = %loop
  ret void
}

declare i1 @odyStop()

declare void @odyExamplePrintUint32(i32)

define void @ody_main() {
basicEntry:
  call void @ody_segment_0()
  ret void
}


LLVM-IR: OK
Running via LLVM-JIT...
Running (press Enter to stop:
5
5
5
5
5
...
'''

## Example Output [NO LOOP]
```text
Building Logical Plan...
Logical Plan: OK
Building Physical Plan...
Physical Plan: OK
Building LLVM-IR...


; ModuleID = 'exampleFull'
source_filename = "exampleFull"

define void @ody_segment_0() {
basicEntry:
call void @odyExamplePrintUint32(i32 5)
ret void
}

declare void @odyExamplePrintUint32(i32)

define void @ody_main() {
basicEntry:
call void @ody_segment_0()
ret void
}


LLVM-IR: OK
Running via LLVM-JIT...
Running:
5
'''