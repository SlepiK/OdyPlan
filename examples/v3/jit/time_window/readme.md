## Time Window JIT Example

### Requirements
Needs a read.json (not .ndjson!) near the executable

### Process
Reads a file (json) and passes it through the TimeWindow to the PrintOperator.
Uses size=5ms and advance=1ms.

### Running System
Run was inside an GitHub Runner

### Output
```text
{"id":1,"value":3.14,"name":"Alice"};1791137688948|1791137688953
[PUNCTUATION watermark=1791137688948]
{"id":2,"value":2.71,"name":"Bob"};1791137688948|1791137688953
{"id":3,"value":1.41,"name":"Charlie"};1791137688948|1791137688953
[PUNCTUATION FINAL]
```