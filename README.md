# Process-Scheduling

## Description of Simulation

This program contains 6 files zipped in the folder Process Scheduling Simulation:

- `main.c`
- `simu.c`
- `simu.h`
- `list.h`
- `list.o` (from instructor)
- `makefile`

## Instruction for Makefile

- `make all`: Compile all the files for general use.
- `make demo`: Run the simulation.
- `make valgrind-check`: Run in valgrind mode to check for memory leaks.
- `make valgrind-for`: Run in valgrind mode to track errors and memory leaks.
- `make clean`: Clean executable files of the simulation program.

## Instruction for Command Line

### GENERAL SYNTAX: [character][number/character][string/number/character]

- Create: `[c/C][number]`
- Fork: `[f/F]`
- Kill: `[k/K][number]`
- Exit: `[e/E]`
- Quantum: `[q/Q]`
- Send: `[s/S][number][string/character]`
- Receive: `[r/R]`
- Reply: `[y/Y][number][string/character]`
- New Semaphore: `[n/N][number][number]`
- Semaphore P: `[p/P][number]`
- Semaphore V: `[v/V][number]`
- Procinfo: `[i/I][number]`
- Totalinfo: `[t/T]`

## Notice

- Priority levels: 0 - highest level, 1 - normal level, 2 - lowest level.
- The priority level of a process remains fixed throughout its lifecycle.
- Quantum: Releasing the current active PCB sends it back to the Ready queue.
- Semaphore: Maximum allowed semaphores are 5. Attempting to exceed this limit will crash the simulation.
- Send/Receive/Reply message: Maximum message length is 40 characters. The last message sent will be shown only when the process occupies the CPU.
- Sending a message to an invalid process ID unblocks the first PCB on the Receiving Block-list to prevent simulation crashes.

## Major Problems of Simulation

### Memory Leaks Problem

The simulation retains 2 memory locations that cannot be freed when an ERROR OCCURS.




