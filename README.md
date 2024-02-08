Tomasulo Speculative Algorithm Implementation
===============================================================

Introduction

In this project, I implemented Tomasulo Speculative algorithm with C++. This
simulator takes assembly instructions in a text file and then it decoded the
instructions to drive the Tomasulo Speculative simulator. It also takes a
configuration text file to setup some parameter for the algorithm. we can specify the
number of reservation stations for each execution units, execution time for each
execution units, the number of memory latency cycles for load and store instructions,
the number of execution units, and the number of ROB entries. Also, we can initialize
the memory and registers using our configuration file. The output of the simulator is
a table which specify issue, execution, memory and write-back cycle for each
instruction.
