## Read NDJSON JIT Example

### Requirements
Needs a read.ndjson (not .json!) near the executable

### Process
Reads read.ndjson and passes it to the PrintOperator.
Uses two ReadOperators!

### Running System
Run was inside an GitHub Runner

### Output
```text
{"id":1,"value":3.14,"name":"Alice"};1791137574621|oo
{"id":1,"value":3.14,"name":"Alice"};1791137574621|oo
[PUNCTUATION watermark=1791137574621]
[PUNCTUATION watermark=1791137574621]
{"id":2,"value":2.71,"name":"Bob"};1791137574621|oo
{"id":2,"value":2.71,"name":"Bob"};1791137574621|oo
{"id":3,"value":1.41,"name":"Charlie"};1791137574621|oo
{"id":3,"value":1.41,"name":"Charlie"};1791137574621|oo
[PUNCTUATION FINAL]
[PUNCTUATION FINAL]
```