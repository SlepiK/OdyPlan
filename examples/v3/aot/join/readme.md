## Join AOT Example

### Requirements
Needs a two .ndjson (not .json!) files (nexmark.bid.10k.ndjson and nexmark.person.10k.ndjson) near the executable

### Process
Reads data from both .ndjson files with two ReadOperators.
Both ReadOperators pass the data to the JoinOperator.
The JoinOperator passes the joined data to the PrintOperator.
Used person.id = bid.bidder.

### Running System
Run was inside an GitHub Runner

### Output
```text
...
{"timestamp":50951320,"id":50914,"name":"Chitta Harbusch","email":"Harbusch@crossgain.com","creditcard":"2503 1297 7555 8540","city":"Lisbon","state":"Petersson","auction":254735,"bidder":50914,"datetime":509513,"price":298.0};50951320|oo
{"timestamp":50955740,"id":50893,"name":"Hyongmuk Hasham","email":"Hasham@umass.edu","creditcard":"3595 4339 6680 5455","city":"El","state":"Mississipi","auction":254727,"bidder":50893,"datetime":509557,"price":47.0};50955740|oo
{"timestamp":50956760,"id":50778,"name":"Kazuyuki Staveren","email":"Staveren@gatech.edu","creditcard":"9880 9956 5839 3709","city":"Moscow","state":"Piramuthu","auction":254754,"bidder":50778,"datetime":509567,"price":242.0};50956760|oo
{"timestamp":50957700,"id":50935,"name":"Puthirak Chnag","email":"Chnag@unical.it","creditcard":"6446 1700 1355 5569","city":"Montreal","state":"Camarinopoulos","auction":254748,"bidder":50935,"datetime":509577,"price":136.0};50957700|oo
{"timestamp":50961380,"id":50927,"name":"Marwan Milewski","email":"Milewski@duke.edu","creditcard":"9152 5902 1761 9091","city":"Colorado","state":"Alabama","auction":254760,"bidder":50927,"datetime":509613,"price":170.0};50961380|oo
```