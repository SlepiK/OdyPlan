## Count Window AOT Example

### Requirements
Needs a nexmark.person.10k.ndjson (not .json!) near the executable

### Process
Reads a file (ndjson) and passes it through the CountWindow to the PrintOperator.

### Running System
Run was inside an GitHub Runner

### Output (Snippet)
```text
...
{"timestamp":60761000,"id":60760,"name":"Uldis Stefano","email":"Stefano@uic.edu","creditcard":"8996 4834 7720 2273","city":"Nassau","state":"Arkansas"};60761000|60766000
{"timestamp":60762000,"id":60761,"name":"Bran Moumoutzis","email":"Moumoutzis@baylor.edu","creditcard":"7119 5198 1269 3594","city":"Mazatlan","state":"Minnesota"};60762000|60767000
{"timestamp":60763000,"id":60762,"name":"Wentong Altis","email":"Altis@hp.com","creditcard":"9451 4053 3713 4378","city":"Providence","state":"Washington"};60763000|60768000
{"timestamp":60764000,"id":60763,"name":"Warwick Melvin","email":"Melvin@compaq.com","creditcard":"7471 1885 8203 5778","city":"Santiago","state":"North Carolina"};60764000|60769000
```