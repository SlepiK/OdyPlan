## Time Window AOT Example

### Requirements
Needs a read.json (not .ndjson!) near the executable

### Process
Reads a file (json) and passes it through the TimeWindow to the PrintOperator.
Uses size=5ms and advance=1ms.

### Running System
Run was inside an GitHub Runner

### Output
```text
{"timestamp":60764000,"id":60763,"name":"Warwick Melvin","email":"Melvin@compaq.com","creditcard":"7471 1885 8203 5778","city":"Santiago","state":"North Carolina"};60764000|60764005
{"timestamp":60765000,"id":60764,"name":"Afke Musikaev","email":"Musikaev@pitt.edu","creditcard":"3638 8052 3719 9895","city":"Augusta","state":"Kansas"};60765000|60765005
{"timestamp":60766000,"id":60765,"name":"Helaman Assadi","email":"Assadi@concordia.ca","creditcard":"6780 8362 4648 7548","city":"Vail","state":"Nebraska"};60766000|60766005
{"timestamp":60767000,"id":60766,"name":"Behnaam Tagansky","email":"Tagansky@cwru.edu","creditcard":"4155 9805 5706 7260","city":"Louisville","state":"Arkansas"};60767000|60767005
{"timestamp":60768000,"id":60767,"name":"Jirun Vassalos","email":"Vassalos@ac.uk","creditcard":"1429 7887 7452 6011","city":"Sarasota","state":"District Of Columbia"};60768000|60768005
{"timestamp":60769000,"id":60768,"name":"Christopher Calvi","email":"Calvi@uni-mb.si","creditcard":"9729 8427 7434 3883","city":"Geneva","state":"Indiana"};60769000|60769005
```