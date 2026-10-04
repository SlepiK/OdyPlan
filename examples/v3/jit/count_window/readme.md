## Count Window JIT Example

### Requirements
Needs a nexmark.person.10k.ndjson (not .json!) near the executable

### Process
Reads a file (ndjson) and passes it through the CountWindow to the PrintOperator.

### Running System
Run was inside an GitHub Runner

### Output (Snippet)
```text
...
{"timestamp":60763000,"id":60762,"name":"Wentong Altis","email":"Altis@hp.com","creditcard":"9451 4053 3713 4378","city":"Providence","state":"Washington"};60763000|60766000
{"timestamp":60764000,"id":60763,"name":"Warwick Melvin","email":"Melvin@compaq.com","creditcard":"7471 1885 8203 5778","city":"Santiago","state":"North Carolina"};60764000|60767000
{"timestamp":60765000,"id":60764,"name":"Afke Musikaev","email":"Musikaev@pitt.edu","creditcard":"3638 8052 3719 9895","city":"Augusta","state":"Kansas"};60765000|60768000
{"timestamp":60766000,"id":60765,"name":"Helaman Assadi","email":"Assadi@concordia.ca","creditcard":"6780 8362 4648 7548","city":"Vail","state":"Nebraska"};60766000|60769000
...
```