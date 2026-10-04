## Union JIT Example

### Requirements
Needs a two .ndjson (not .json!) files (read.ndjson twice, see AOT Example for different files) near the executable

### Process
Reads both .ndjson files with two ReadOperators.
Data is passed from ReadOperators to UnionOperator and from it to the PrintOperator.

### Running System
Run was inside an GitHub Runner

### Output
```text
{"timestamp":60768000,"id":60767,"name":"Jirun Vassalos","email":"Vassalos@ac.uk","creditcard":"1429 7887 7452 6011","city":"Sarasota","state":"District Of Columbia"};60768000|oo
[PUNCTUATION watermark=60768000]
{"timestamp":60769000,"id":60768,"name":"Christopher Calvi","email":"Calvi@uni-mb.si","creditcard":"9729 8427 7434 3883","city":"Geneva","state":"Indiana"};60769000|oo
[PUNCTUATION watermark=60769000]
[PUNCTUATION FINAL]
```