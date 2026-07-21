## Info
Needs a read.ndjson (not .json!) near the executable.
Creates a write.ndjson, which contains the same data as read.ndjson (ReadOperator → WriteOperator)

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


; ModuleID = 'example_write_json'
source_filename = "example_write_json"

@field_id = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name = private unnamed_addr constant [5 x i8] c"name\00", align 1
@field_id.1 = private unnamed_addr constant [3 x i8] c"id\00", align 1
@field_value.2 = private unnamed_addr constant [6 x i8] c"value\00", align 1
@field_name.3 = private unnamed_addr constant [5 x i8] c"name\00", align 1

define void @ody_segment_0() {
basicEntry:
  %Stg.Typ.11 = alloca double, align 8
  %Stg.Typ.4 = alloca i64, align 8
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
  call void @rtIOSinkBeginData(i64 0)
  store i64 %5, ptr %Stg.Typ.4, align 4
  call void @rtIOSinkWrite(i64 0, i64 4, ptr @field_id.1, ptr %Stg.Typ.4)
  store double %7, ptr %Stg.Typ.11, align 8
  call void @rtIOSinkWrite(i64 0, i64 11, ptr @field_value.2, ptr %Stg.Typ.11)
  call void @rtIOSinkWrite(i64 0, i64 20, ptr @field_name.3, ptr %8)
  call void @rtIOSinkEndData(i64 0)
  br label %read_loop

read_exit:                                        ; preds = %read_loop
  ret void
}

declare i1 @rtIOSourceHasNext(i64)

declare i1 @odyStop()

declare ptr @rtIOSourceGetField(i64, ptr)

declare i64 @rtParseInt64(ptr)

declare double @rtParseDouble(ptr)

declare void @rtIOSinkBeginData(i64)

declare void @rtIOSinkWrite(i64, i64, ptr, ptr)

declare void @rtIOSinkEndData(i64)

define void @ody_main() {
basicEntry:
  call void @ody_segment_0()
  call void @rtEnd()
  ret void
}

declare void @rtEnd()


LLVM-IR: OK
Running via LLVM-JIT...
```