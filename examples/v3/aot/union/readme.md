## Union AOT Example

### Requirements
Needs a two .ndjson (not .json!) files (nexmark.person.10k.2.ndjson and nexmark.person.10k.ndjson which share the schema and some entries) near the executable

### Process
Reads both .ndjson files with two ReadOperators.
Data is passed from ReadOperators to UnionOperator and from it to the PrintOperator.

### Running System
Run was inside an GitHub Runner

### Output
```text
{"timestamp":69901000,"id":69900,"name":"Spencer Esbelin","email":"Esbelin@broadquest.com","creditcard":"7518 4688 3066 7473","city":"Louisville","state":"Nevada"};69901000|oo
{"timestamp":69902000,"id":69901,"name":"Paicheng Brundage","email":"Brundage@unl.edu","creditcard":"5897 7279 3428 8693","city":"Cody","state":"Iowa"};69902000|oo
{"timestamp":69903000,"id":69902,"name":"Ahuva Holburn","email":"Holburn@uni-freiburg.de","creditcard":"6219 1073 7675 1130","city":"Indianapolis","state":"Sifakis"};69903000|oo
{"timestamp":69904000,"id":69903,"name":"Frederik Angel","email":"Angel@pi.it","creditcard":"5643 6173 4642 2080","city":"Greensboro","state":"Wyoming"};69904000|oo
{"timestamp":69905000,"id":69904,"name":"Herschel Wallrath","email":"Wallrath@clustra.com","creditcard":"6977 6586 6838 7534","city":"Honolulu","state":"Bishr"};69905000|oo
```