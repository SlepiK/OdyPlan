## Union Measure AOT Example

### Requirements
Needs a two .ndjson (not .json!) files (read.ndjson and read2.ndjson) near the executable

### Process
Reads both .ndjson files with two ReadOperators. 
Data is passed from ReadOperators to UnionOperator and from it to the MeasureOperator.
The MeasureOperator passes it to the PrintOperator.

### Running System
Run was inside an GitHub Runner

### Output
```text
{"id":1,"value":3.14,"name":"Alice"};1791138609664|oo
[PUNCTUATION watermark=1791138609664]
[2026-10-04 20:30:09.665] [info] [after_union] 2 lines, 10.352 lines/ms (window: 0.193 ms), 10.352 lines/ms (total: 0.193 ms)
{"id":1,"value":3.14,"name":"Alice"};1791138609664|oo
{"id":2,"value":2.71,"name":"Bob"};1791138609664|oo
[2026-10-04 20:30:09.665] [info] [after_union] 4 lines, 43.567 lines/ms (window: 0.046 ms), 16.729 lines/ms (total: 0.239 ms)
{"id":2,"value":2.71,"name":"Bob"};1791138609664|oo
{"id":3,"value":1.41,"name":"Charlie"};1791138609664|oo
[2026-10-04 20:30:09.665] [info] [after_union] 6 lines, 470.810 lines/ms (window: 0.004 ms), 24.656 lines/ms (total: 0.243 ms)
{"id":3,"value":1.41,"name":"Charlie"};1791138609664|oo
{"id":4,"value":1.51,"name":"Johann"};1791138609664|oo
[2026-10-04 20:30:09.665] [info] [after_union] 8 lines, 468.604 lines/ms (window: 0.004 ms), 32.308 lines/ms (total: 0.248 ms)
{"id":5,"value":1.81,"name":"Hans"};1791138609664|oo
[PUNCTUATION FINAL]
[2026-10-04 20:30:09.665] [info] [after_union] Total: 8 lines in 0.248 ms (32.308 lines/ms)
```