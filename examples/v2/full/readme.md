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