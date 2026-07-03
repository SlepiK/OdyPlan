## Info
Can be run as check to verify the logical area of the library works

## Example Output
```text
Running creation of successful pipeline]
OK: logical plan built [SOURCE >-PORT_0-INT-PORT_0-> PIPE >-PORT_0-INT-PORT_0-> SINK]
Running creation of unsuccessful pipeline - type-mismatch]
OK (Type-Mismatch found): Plan::connect: incompatible port types between fromOperator and toOperator
Running creation of unsuccessful pipeline - Fan-in]
OK (Fan-in found): Plan::validateConnectionRequest: input port is already used
Running creation of unsuccessful pipeline - Cycle]
OK (Cycle found): Plan::connect: connection would create a cycle in the plan
```