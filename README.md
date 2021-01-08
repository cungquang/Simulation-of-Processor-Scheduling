# Process-Scheduling
******************************************************************************************************************
******              	  										                                                                ******
******                  	              README FOR PROCESS SCHEDULING SIMULATION 				                    ******
******													                                                                            ******		
******------------------------------------------------------------------------------------------------------******
******------------------------------------------------------------------------------------------------------******
******              	  										                                                                ******
******					                              CREATOR NAME: HONG QUANG CUNG					                        ******
******              	  										                                                                ******
******************************************************************************************************************

---------------------------------------- Descripton of simulation ---------------------------------------------


This program contain 6 files - zip in folder Process Scheduling Simulation:
- main.c
- simu.c
- simu.h
- list.h
- list.o (from instructor)
- makefile


---------------------------------------- Instruction for makefile --------------------------------------------


make all - compile all the file for general use
make demo - this will run the simulation
make valgrind-check - run in valgrind mode to check memory leak
make valgrind-for - run in valgrind mode to track the error && memory leak
make clean - clean executable file of simulation program


-------------------------------------- Instruction for Command Line ------------------------------------------


GENERAL SYNTAX: [character]<space>[number/character]<space>[string/number/character]

Command line instruction:
Create: [c/C]<space>[number]
Fork: [f/F]
Kill: [k/K]<space>[number]
Exit: [e/E]
Quantum: [q/Q]
Send: [s/S]<space>[number]<space>[string/character]
Receive: [r/R]
Reply: [y/Y]<space>[number]<space>[string/character]
New Semaphore: [n/N]<space>[number]<space>[number]
Semaphore P: [p/P]<space>[number]
Semaphore V: [v/V]<space>[number]
Procinfo: [i/I]<space>[number]
Totalinfo: [t/T]


------------------------------------------------- Notice -----------------------------------------------------


Priority level: 
0 - highest level
1 - normal level
2 - lowest level
The level of Process will be fixed over the program. There is no permission for evolving level of priority.
The priority will remain unchanged throughout the cycle life of PCB.

Quantum:
When users execute Quantum command - q/Q, it'll release the current active PCB, send it back to Ready queue.
If there is no available PCB on Ready queue, the current active PCB will stay in the CPU - until users kill it.

Semaphore:
Maximum number of semaphores that users are allowed to use is 5. Any attemp to increase number of semaphore.
will crash the simulation.

Send/Receive/Reply message:
Max length of message is 40 characters.
The simulation is designed to only receive the last message (either reply/receive).
The message will only send to stdout when it occupied CPU - otherwise, it will not be shown on screen.
The user can also see the message through command - Procinfor - i/I.
The user allow to send to itself or other invalid process ID
In case, the user decide to send the message to unavailable process ID (on the Receiving Block-list), it will
unblock the first PCB (on the Receiving Block-list) in order to prevent the simulation crash.



-------------------------------------- Major Problems of simulation ------------------------------------------


Memory leaks problem:
- Simulation still remain 2 memory locations that can not be free when ERROR OCCURRED.


##############################################################################################################
##############################################################################################################

