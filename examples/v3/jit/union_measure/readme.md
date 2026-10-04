## Union Measure JIT Example

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
{"id":1,"value":3.14,"name":"Alice"};1791137851961|oo
[PUNCTUATION watermark=1791137851961]
[2026-10-04 20:17:31.961] [info] [after_union] 2 lines, 10.111 lines/ms (window: 0.198 ms), 10.111 lines/ms (total: 0.198 ms)
{"id":1,"value":3.14,"name":"Alice"};1791137851961|oo
{"id":2,"value":2.71,"name":"Bob"};1791137851961|oo
[2026-10-04 20:17:31.962] [info] [after_union] 4 lines, 25.892 lines/ms (window: 0.077 ms), 14.543 lines/ms (total: 0.275 ms)
{"id":2,"value":2.71,"name":"Bob"};1791137851961|oo
{"id":3,"value":1.41,"name":"Charlie"};1791137851961|oo
[2026-10-04 20:17:31.962] [info] [after_union] 6 lines, 338.926 lines/ms (window: 0.006 ms), 21.356 lines/ms (total: 0.281 ms)
{"id":3,"value":1.41,"name":"Charlie"};1791137851961|oo
{"id":4,"value":1.51,"name":"Johann"};1791137851961|oo
[2026-10-04 20:17:31.962] [info] [after_union] 8 lines, 374.532 lines/ms (window: 0.005 ms), 27.944 lines/ms (total: 0.286 ms)
{"id":5,"value":1.81,"name":"Hans"};1791137851961|oo
[PUNCTUATION FINAL]
[2026-10-04 20:17:31.962] [info] [after_union] Total: 8 lines in 0.286 ms (27.944 lines/ms)
```