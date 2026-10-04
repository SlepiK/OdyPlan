## Join with TimeWindow AOT Example

### Requirements
Needs a two .ndjson (not .json!) files (nexmark.bid.10k.ndjson and nexmark.person.10k.ndjson) near the executable

### Process
Reads data from both .ndjson files with two ReadOperators.
Both ReadOperators pass the data to the TimeWindows which passes it to JoinOperator.
The JoinOperator passes the joined data to the PrintOperator.
Used person.id = bid.bidder and size=15000ms, advance=1ms.

### Running System
Run was inside an GitHub Runner

### Output
```text
...
{"timestamp":50843400,"id":50830,"name":"Hongsing Sereno","email":"Sereno@unl.edu","creditcard":"5656 1101 3319 6058","city":"Orlando","state":"Alabama","auction":254169,"bidder":50830,"datetime":508434,"price":208.0};50843400|50846000
{"timestamp":50910440,"id":50907,"name":"Iordana Rahm","email":"Rahm@tue.nl","creditcard":"6049 4714 1688 6680","city":"Kiev","state":"Connecticut","auction":254499,"bidder":50907,"datetime":509104,"price":255.0};50910440|50923000
```