#ifndef _SIMU_H_
#define _SIMU_H_

#include <stdio.h>
#include <string.h>

#include "list.h"

#define MSG_MAX 40

//enum state of the process
enum Proc_State{
	Running,
	Ready,
	Blocked
};

//Struct Process Control Block
typedef struct pcb PCB;
struct pcb{
	int pID;						// pID = {from 0 - MAX_ID} 
	int priority;					// priority = {0 - high, 1 - norm, 2 - low}
	enum Proc_State state;			// state  = {0 - Running, 1 - Ready, 2 - Blocked}
	
	int pID_send;					// pID of source PCB - which will send the message					
	char* send_Msg;					// send_Msg = array of character - fixed size = 40
	
	int pID_receive;				// pID of destination PCB - which will receive the message
	char* receive_Msg;				// receive_Msg = array of character - fixed size = 40
	
	int pID_reply;					// pID of reply PCB - which relpy to this PCB
	char* reply_Msg;				// reply_Msg = array of character - fixed size = 40
};

//struct Semaphore
typedef struct semaphore Semaphore;
struct semaphore{
	int sID;
	int sInit;
	List* block_List;
};


//create_Proc(int Prior): create the PCB
//return: 0 if success - 1 otherwise
int create_Proc(int Prior);

//fork_Proc(): copy the currently running process & put it on ready queue
//return: 0 if success - 1 otherwise 
int fork_Proc();

//kill_Proc(int proc_ID): kill the process & remove it from the system
//return: 0 if success & 1 otherwise; report on screen
int kill_Proc(int proc_ID);

//exit_Proc(): kill the currently run process
//return: void & process scheduling information
void exit_Proc();

//quant_Time(): time quantum of running process expires
//return: action taken information - eg: process scheduling info
void quant_Time();

//send_Proc(int proc_ID, char* msg): send the message to another process & block it until reply
//return: 0 if success - 1 otherwise; scheduling information; reply source & text
int send_Proc(int proc_ID, char* msg);

//recv_Proc(int proc_ID, char* msg): receive the message - block until one arrives
//return: 0 if sucess - 1 otherwise
void recv_Proc();

//reply_Proc(int proc_ID, char* msg): unblock the sender and deliver the reply
//return: 0 if sucess - 1 otherwise
int reply_Proc(int proc_ID, char* msg);

//new_Sem(int sem_ID, int init_V): initialize the named semaphore with given value init_V
//return: 0 if success - 1 otherwise
int new_Sem(int sem_ID, int init_V);

//sem_P(int sem_ID): execute the semaphore P operation - sem_ID = {0 to 4}
//return: 0 if sucess - 1 otherwise; action taken
int sem_P(int sem_ID);

//sem_V(int sem_ID): execute the semaphore V operation - sem_ID = {0 to 4}
//return: 0 if success - 1 otherwise; action taken
int sem_V(int sem_ID);

//Procinfo(int proc_ID): display all info of the process to screen 
//return: void & all information to stdout
void Procinfo(int proc_ID);

//Totalinfo(): display all process queues and their contents
//return: void & all information to sdout
void Totalinfo();

//#########################################################################################
//###							Prorgarm Administration									###	
//###					prog_Admin - init_Admin - regular_Admin							###
//###																					###
//#########################################################################################

//char_parameter(): get array of parameter from user input
//return: void
void char_Args();

//PCB_Free(PCB* proc_PCB): free the process control block;
//return: void
void PCB_Free(PCB* proc_PCB);

//mem_Free(): free all the memorize that have been used
//return: void
void mem_Free();

//init_Admin(): the function inside init process
//return: 0 successfully terminate - 1 go to regular - 2 go to error
int init_Admin();

//regular_Admin(): the function inside init process
//return: 0 if success - 1 otherwise
int regular_Admin();

//prog_Active(): the function inside init process
//return: 0 if success - 1 otherwise
int prog_Admin(List* list_1, List* list_2, List* list_3,
 List* send_list, List* recv_list);

#endif