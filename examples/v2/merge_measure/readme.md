## Info
COMING LATER

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


; ModuleID = 'example_read_json'
source_filename = "example_read_json"

@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name = private unnamed_addr constant [5 x i8] c"name\00", align 1
@field_id.1 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value.2 = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name.3 = private unnamed_addr constant [5 x i8] c"name\00", align 1

define void @ody_rsource_0() {
basicEntry:
  br label %read_loop

read_loop:                                        ; preds = %read_produce, %basicEntry
  %0 = call i1 @rtIOSourceHasNext(i64 1)
  %1 = call i1 @odyStop()
  %2 = xor i1 %1, true
  %3 = and i1 %0, %2
  br i1 %3, label %read_produce, label %read_exit

read_produce:                                     ; preds = %read_loop
  %4 = call ptr @rtIOSourceGetField(i64 1, ptr @field_id)
  %5 = call i64 @rtParseInt64(ptr %4)
  %6 = call ptr @rtIOSourceGetField(i64 1, ptr @field_value)
  %7 = call double @rtParseDouble(ptr %6)
  %8 = call ptr @rtIOSourceGetField(i64 1, ptr @field_name)
  call void @ody_fanin_exit_2_0(i64 %5, double %7, ptr %8)
  br label %read_loop

read_exit:                                        ; preds = %read_loop
  ret void
}

define internal void @ody_fanin_exit_2_0(i64 %0, double %1, ptr %2) {
fanInEntry:
  call void @rtOdyPrintInt(i64 %0)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintDouble(double %1)
  call void @rtOdyPrintSeperator()
  call void @rtOdyPrintString(ptr %2)
  call void @rtOdyPrintNewline()
  ret void
}

declare void @rtOdyPrintInt(i64)

declare void @rtOdyPrintSeperator()

declare void @rtOdyPrintDouble(double)

declare void @rtOdyPrintString(ptr)

declare void @rtOdyPrintNewline()

declare i1 @rtIOSourceHasNext(i64)

declare i1 @odyStop()

declare ptr @rtIOSourceGetField(i64, ptr)

declare i64 @rtParseInt64(ptr)

declare double @rtParseDouble(ptr)

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
  call void @ody_fanin_exit_2_0(i64 %5, double %7, ptr %8)
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
  call void @ody_rsource_0()
  call void @ody_rsource_1()
  call void @ody_end_phase()
  call void @rtEnd()
  ret void
}

declare void @rtEnd()


LLVM-IR: OK
Running via LLVM-JIT...
1 3.14 Alice
2 2.71 Bob
3 1.41 Charlie
1 3.14 Alice
2 2.71 Bob
3 1.41 Charlie
```