## Logical Plan Example

### Requirements
N/A

### Process
Checks the logical Plan building process (no physical Plan or LLVM IR is built!)

### Running System
Run was inside an GitHub Runner

### Output
```text
Running creation of successful pipeline]
OK: logical plan built [SOURCE >-PORT_0-INT-PORT_0-> PIPE >-PORT_0-INT-PORT_0-> SINK]
Running creation of unsuccessful pipeline - Fan-in]
OK (Fan-in found): ISinkOperator: cant bind more than one input
Running creation of unsuccessful pipeline - Cycle]
OK (Cycle found): Plan::validateConnectionRequest: fromOperator has no output schema
```