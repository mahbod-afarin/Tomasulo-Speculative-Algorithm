Tomasulo Speculative Algorithm Implementation
===============================================================

## Introduction

 <div align="justify">
In this project, I implemented the Tomasulo Speculative algorithm with C++. This simulator takes assembly instructions in a text file and then decodes the instructions to drive the Tomasulo Speculative simulator. It also takes a configuration text file to set up some parameters for the algorithm. We can specify the number of reservation stations for each execution unit, execution time for each execution unit, the number of memory latency cycles for load and store instructions, the number of execution units, and the number of ROB entries. Also, we can initialize the memory and registers using our configuration file. The output of the simulator is a table that specifies the issue, execution, memory, and write-back cycle for each instruction.
</div>

## Implementation

 <div align="justify">
In this section, we aim to provide a quick overview of the speculative Tomasulo architecture before delving into its implementation. To achieve this, we will first explore the architecture of a simple Tomasulo algorithm. Thus, the first section will focus on examining the simple Tomasulo architecture, followed by a discussion on the speculative Tomasulo architecture in the second section. Finally, in the last section, we will delve into the implementation details of the algorithm.
</div>

## Tomasulo's Architecture

 <div align="justify">
The Tomasulo algorithm is a hardware algorithm designed for dynamically scheduling instructions, facilitating out-of-order execution, and enhancing the efficiency of multiple execution units. In this algorithm, the issue stage occurs in order, while execution and write-back stages are performed out of order. The Tomasulo Algorithm employs register renaming to facilitate accurate out-of-order execution. In this process, all general-purpose and reservation station registers store either a real value or a placeholder value. If a real value isn't available to a destination register during the issue stage, it initially holds a placeholder value. This placeholder value acts as a tag indicating which reservation station will produce the real value. Upon completion of execution and broadcasting of the result on the common data bus, the placeholder is replaced with the real value.
</div>

## Speculative Tomasulo Architecture

 <div align="justify">
A significant challenge with the Tomasulo algorithm arises when encountering branch instructions. Since it cannot determine whether the branch is taken or not taken, it stalls all subsequent instructions to await the branch result. This stalling results in wasted CPU time. To address this issue, speculative Tomasulo architecture is employed.

Speculative Tomasulo utilizes the Re-order Buffer (ROB) to issue instructions following the branch instruction. In speculative Tomasulo, a commit stage is introduced into the pipeline. During this stage, results are written into the ROB instead of directly into the register file or memory. Consequently, when instructions are no longer speculative, speculative Tomasulo permits them to update the register file or memory.

The addition of the commit stage enables the completion of instructions in order, thereby allowing the execution of instructions after the branch in speculative Tomasulo. As illustrated in Table 1, the primary difference between simple Tomasulo and speculative Tomasulo lies in their completion strategy: simple Tomasulo completes instructions out-of-order, while speculative Tomasulo completes instructions in-order. In this project, I implemented the speculative Tomasulo algorithm.
</div>

## Implementation of the speculative Tomasulo

<div align="justify">
Speculative Tomasulo consists of 5 main stages. These five stages are issue, execution, memory, write-back, and commit. In the issue stage, the simulator fetches the instructions from the instruction queue and then puts the instructions into the reservation stations. In the execution stage, the simulator retrieves the instructions from the reservation stations and then executes them. The memory stage is used only for load and store instructions. If we have a load instruction, the simulator loads the data from memory and then broadcasts the data using the common data bus. For the store instruction, it writes the result in the ROB (Reorder Buffer), and when the instruction has committed, it stores the data in the memory. In the write-back stage, the simulator writes the results into the ROB and also broadcasts the results for other instructions. In the commit stage, the simulator writes the results to the memory and register files, and then it deletes the instructions from the reservation stations. For branch instructions, the simulator assumes that the branch is taken, and then it issues the instruction which is the destination of the branch. If the prediction is wrong, it flushes the incorrect instructions and then proceeds to issue the normal instructions after the branch instruction. The simulator has 16 integer registers and 16 floating-point registers. This simulator takes two inputs: the first input is the assembly input instructions, and the second is the configuration text file for the speculative Tomasulo architecture. The instruction set of the simulator is detailed in Table 2. The speculative Tomasulo simulator stores the results of the simulation in the output text file. The output format resembles the table format used in our course lectures. It includes the register file values and the memory values after executing the simulator. In the next section, we will examine a sample output of the simulator.
</div>

