## Read NDJSON AOT Example

### Requirements
Needs a read.ndjson (not .json!) near the executable

### Process
Reads read.ndjson and passes it to the PrintOperator.
Uses two ReadOperators!

### Running System
Run was inside an GitHub Runner

### Output
```text
{"timestamp":50969700,"auction":254802,"bidder":47030};50969700|oo
[PUNCTUATION watermark=50969700]
{"timestamp":50969720,"auction":254822,"bidder":7451};50969720|oo
[PUNCTUATION watermark=50969720]
{"timestamp":50969740,"auction":254816,"bidder":22082};50969740|oo
[PUNCTUATION watermark=50969740]
{"timestamp":50969760,"auction":254835,"bidder":36382};50969760|oo
[PUNCTUATION watermark=50969760]
{"timestamp":50969780,"auction":254815,"bidder":26440};50969780|oo
[PUNCTUATION watermark=50969780]
[PUNCTUATION FINAL]
```