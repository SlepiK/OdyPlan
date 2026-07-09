## Info
...

## Example Output [LOOP]
```text
COMING LATER
```

## Example Output [NO LOOP]
```text
Building Logical Plan...
Logical Plan: OK
Building Physical Plan...
Physical Plan: OK
Building LLVM-IR...


; ModuleID = 'exampleSelect'
source_filename = "exampleSelect"

define void @ody_segment_0() {
basicEntry:
  br i1 true, label %select_pass, label %select_merge

select_pass:                                      ; preds = %basicEntry
  call void @rtOdyPrintInt(i32 5)
  call void @rtOdyPrintNewline()
  br label %select_merge

select_merge:                                     ; preds = %select_pass, %basicEntry
  ret void
}

declare void @rtOdyPrintInt(i32)

declare void @rtOdyPrintNewline()

define void @ody_main() {
basicEntry:
  call void @ody_segment_0()
  ret void
}


LLVM-IR: OK
Running via LLVM-JIT...
5
```