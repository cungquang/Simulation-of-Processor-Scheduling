#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "simu.h"

#define MAX_ID 3000
#define READY_MAX 3

static PCB* Active = NULL;				//pointer to active PCB
static PCB* init_Proc = NULL;			//pointer to the init PCB

//Setup Process ID:
static int new_ID = -1;			// new ID available for using
static int free_ID = -1;		// index of valid reusable ID
static int source_ID[MAX_ID];	// array of valid reusable ID

//Setup Semaphore ID:
static Semaphore* sem_Array[5];
static int num_Sem = 0;

//3 Ready queue:
static List* ready[READY_MAX];			// 0 - High; 1 - Norm; 2 - Low

//2 Waiting queue:
static List* commu[2];					// 0 - list of sending; 1 - list of receiving

//Setup Message:
static char msg_Sys[] = "System-";
static char msg_ID[] = "pID-";
static char msg_Prior[] = "priority-";
static char msg_State[] = "state-";
static char msg_Send[] = "Send Message-";
static char msg_Rec[] = "Received Message-";
static char msg_Rep[] = "Replied Message-";

//Setup user input:
static char buf[50];					// buffer to store user input
static char command;					// store user command
static char first_Args[8];				// store first arguments
static char second_Args[40];			// store initual value for Semaphore


//create_Proc(int Prior): create the PCB
//return: 0 if success - 1 otherwise
int create_Proc(int Prior){

	//check validity of priority number:
	if(Prior > 2 || Prior < 0){
		goto Failure;
	}

	//check number of process is out of limit
	if(new_ID + 1 >= MAX_ID && free_ID + 1 <= 0){
		goto Failure;
	}

	//create a new PCB
	PCB* p_temp= malloc(sizeof(PCB));
	int t_ID;	

	//check if malloc successfully
	if(p_temp == NULL){
		goto Failure;
	}		

	//check available ID:
	if(free_ID + 1 > 0){
		t_ID = source_ID[free_ID];
		free_ID--;
	}else{
		t_ID = new_ID;
		new_ID++;
	}

	//Add information into PCB:
	p_temp->pID = t_ID;	
	p_temp->priority = Prior;
	p_temp->state = 2;

	// Send message:
	p_temp->pID_send = -2;
	p_temp->send_Msg = malloc(sizeof(char)*MSG_MAX);
	memset(p_temp->send_Msg, 0, sizeof(char)*MSG_MAX);

	// Receive message:
	p_temp->pID_receive = -2;
	p_temp->receive_Msg = malloc(sizeof(char)*MSG_MAX);
	memset(p_temp->receive_Msg, 0, sizeof(char)*MSG_MAX);

	// Reply message:
	p_temp->pID_reply = -2;
	p_temp->reply_Msg = malloc(sizeof(char)*MSG_MAX);
	memset(p_temp->reply_Msg, 0, sizeof(char)*MSG_MAX);

	// Add to appropriate ready queue:
	if(Active == NULL){
		Active = p_temp;
		init_Proc = p_temp;
	}else{
		if(Active->pID == -1){
			Active = p_temp;
		}else{
			List_append(ready[p_temp->priority], p_temp);
		}
	}

	if(p_temp->pID != -1)
	printf("Success-pID-%d\n", p_temp->pID);
	fflush(stdout);
	return 0;

	Failure:
		printf("Failure\n");
		fflush(stdout);
		return 1;
}

//fork_Proc(): copy the currently running process & put it on ready queue
//return: 0 if success - 1 otherwise 
int fork_Proc(){

	//create a new PCB
	PCB* p_temp= malloc(sizeof(PCB));
	int t_ID;	

	//check if malloc successfully
	if(p_temp == NULL){
		goto Failure;
	}		

	//check available ID:
	if(free_ID + 1 > 0){
		t_ID = source_ID[free_ID];
		free_ID--;
	}else{
		t_ID = new_ID;
		new_ID++;
	}

	//Assign information to p_temp:
	p_temp->pID = t_ID;
	p_temp->priority = Active->priority;
	p_temp->state = 2;
	
	// Send message:
	p_temp->send_Msg = malloc(sizeof(char)*MSG_MAX);
	memset(p_temp->send_Msg, 0, sizeof(char)*MSG_MAX);

	// Receive message:
	p_temp->pID_receive = -2;
	p_temp->receive_Msg = malloc(sizeof(char)*MSG_MAX);
	memset(p_temp->receive_Msg, 0, sizeof(char)*MSG_MAX);

	// Reply message:
	p_temp->pID_reply = -2;
	p_temp->reply_Msg = malloc(sizeof(char)*MSG_MAX);
	memset(p_temp->reply_Msg, 0, sizeof(char)*MSG_MAX);


	//Send Message:
	strcpy(p_temp->send_Msg,Active->send_Msg);

	//Receive Message:
	p_temp->pID_receive = Active->pID_receive;
	strcpy(p_temp->receive_Msg, Active->receive_Msg);

	//Reply Message:
	p_temp->pID_reply = Active->pID_reply;
	strcpy(p_temp->reply_Msg, Active->reply_Msg);

	//Add the removed PCB to appropriate ready queue:
	List_append(ready[p_temp->priority], p_temp);

	printf("Success-pID-%d", p_temp->pID);
	fflush(stdout);
	return 0;

	Failure:
		printf("Failure\n");
		fflush(stdout);
		return 1;
}

//kill_Proc(int proc_ID): kill the process & remove it from the system
//return: 0 if success & 1 otherwise; report on screen
int kill_Proc(int proc_ID){

	if(proc_ID	< -1 && proc_ID > MAX_ID) goto Failure;

	Node* curr = NULL;
	PCB* temp = NULL;
	int f_ID;

	// CHECK IF INIT_PROCESS:
	if(Active != NULL){
		// user requests to kill INIT_PROCESS
		if(Active->pID == -1){
			if(proc_ID == -1){
				Active = NULL;
				PCB_Free(init_Proc);
				free(init_Proc);
				init_Proc = NULL;
				goto Terminate;
			}
		}

		//otherwise
		if(Active->pID == proc_ID){
			temp = Active;
			Active = NULL;
			f_ID = temp->pID;
			
			PCB_Free(temp);

			free(temp);
			temp = NULL;
		}	
	}
	

	// 3 READY QUEUES
	for(int i = 0; i < 3; i++){

		//move the current item to the head of the list
		List_first(ready[i]);
		
		//assign current item to curr variable
		curr = ready[i]->pCurrentNode;
		while(curr != NULL){
			temp = curr->pItem;

			//check if match the condition
			if(temp->pID == proc_ID){
				temp = List_remove(ready[i]);
				f_ID = temp->pID;
				PCB_Free(temp);
				
				free(temp);
				temp = NULL;
			}

			//move the current node to the next one:
			List_next(ready[i]);
			curr = ready[i]->pCurrentNode;
		}

	}

	// SENDING BLOCK LIST && RECEIVING BLOCK LIST
	for(int i = 0; i < 2; i++){
		List_first(commu[i]);

		//assign current item to curr variable
		curr = commu[i]->pCurrentNode;
		while(curr != NULL){
			temp = curr->pItem;

			//check if match the condition
			if(temp->pID == proc_ID){
				temp = List_remove(commu[i]);
				f_ID = temp->pID;
				PCB_Free(temp);

				free(temp);
				temp = NULL;
			}

			List_next(commu[i]);
			curr = commu[i]->pCurrentNode;
		}
	}

	// BLOCK-LIST SEMAPHORE
	if(num_Sem > 0){
		for(int i = 0; i < num_Sem; i++){

			//move the current item to the head of the list
			List_first(sem_Array[i]->block_List);

			//assign current item to curr variable
			curr = sem_Array[i]->block_List->pCurrentNode;
			while(curr != NULL){
				temp = curr->pItem;

				//check if match condition
				if(temp->pID == proc_ID){
					temp = List_remove(sem_Array[i]->block_List);
					f_ID = temp->pID;
					
					PCB_Free(temp);

					free(temp);
					temp = NULL;
				}

				List_next(sem_Array[i]->block_List);
				curr = sem_Array[i]->block_List->pCurrentNode;
			}
		}
	}

	source_ID[++free_ID] = f_ID;

	//Add the next available candidate:
	if(Active == NULL){
		for(int i = 0; i < 3; i++){
			if(List_count(ready[i]) > 0){
				List_first(ready[i]);
				Active = List_remove(ready[i]);
				Active->state = 1;
				goto Success;
			}
		}
	}

	//if all the ready queue is empty - active init proc
	if(Active == NULL){
		Active = init_Proc;
		goto Success;		
	}

	goto Success;

	Success:
		printf("Success Kill-pID-%d\n", f_ID);
		return 0;

	Terminate:
		printf("Success Kill-Init Process\n");
		return 2;

	Failure:
		printf("Fail Kill-pID-%d\n", proc_ID);
		return 1;
}

//exit_Proc(): kill the currently run process
//return: void & process scheduling information
void exit_Proc(){

	// user request kill INIT_PROCESS:
	int check;
	check = kill_Proc(Active->pID);
	if(check == 2){
		mem_Free();
	}
	
	Totalinfo();
	return;
}

//quant_Time(): time quantum of running process expires
//return: action taken information - eg: process scheduling info
void quant_Time(){

	PCB* p_temp = Active;

	//Assign the new process to CPU:
	for(int i = 0; i < READY_MAX; i++){
		if(List_count(ready[i]) > 0){
			//Remove the current PCB in CPU:
			List_first(ready[i]);
			Active = List_remove(ready[i]);
			Active->state = 1;					

			//Add the removed PCB into appropriate ready Q
			p_temp->state = 2;
			List_append(ready[p_temp->priority], p_temp);
			Totalinfo();	
			goto Exit;
		}
	}

	return;

	Exit:
		printf("CPU: PCB-%d\n", Active->pID);
		if(strlen(Active->receive_Msg) > 0){
			printf("Send Source: pID-%d\n", Active->pID_send);
			printf("Receive Message: %s\n", Active->receive_Msg);
		}

		if(strlen(Active->reply_Msg) > 0){
			printf("Reply Source: pID-%d\n", Active->pID_reply);
			printf("Reply Message: %s\n", Active->reply_Msg);
		}
		Totalinfo();
		return;
}

//send_Proc(int proc_ID, char* msg): send the message to another process & block it until reply
//return: 0 if success - 1 otherwise; scheduling information; reply source & text
int send_Proc(int proc_ID, char* msg){

	// Check if the PCB ID is out of bound
	if(proc_ID < 0 || proc_ID > MAX_ID){
		goto Failure;
	}
	
	// Check if Active is init_Process:
	if(Active->pID == -1) goto Failure;

	PCB* s_temp = NULL;
	Node* curr = NULL;
	PCB* temp = NULL; 

	s_temp = Active;					// set the PCB in CPU to s_temp
	s_temp->state = 3;					// set the status to blocked

	strcpy(s_temp->send_Msg, msg);		// save the sent message for the receive
	s_temp->pID_receive = proc_ID;		// track of the receiver ID

	Active = NULL;						// free the CPU
	List_append(commu[0], s_temp);		// block the sending PCB

	//Allow new PCB to enter the CPU:
	for(int i = 0; i < READY_MAX; i++){
		if(List_count(ready[i]) > 0){
			List_first(ready[i]);
			Active = List_remove(ready[i]);
			goto Success;
		}
	}

	// No more PCB on ready queue:
	if(Active == NULL){
		Active = init_Proc;
	}

	goto Success;

	Success:
		printf("Success\n");
		if(strlen(s_temp->reply_Msg) > 0){
			printf("Reply Source-%d\n", s_temp->pID_reply);
			printf("Reply message-%s\n", s_temp->reply_Msg);				
		}
		
		Totalinfo();
		return 0;

	Failure:
		printf("Failure\n");
		return 1;
}

//recv_Proc(int proc_ID, char* msg): unblock the sender and reply to the sender
//return: 0 if sucess - 1 otherwise
void recv_Proc(){

	// Check if the init process is running:
	if(Active->pID == -1) return;

	PCB* r_temp = NULL;
	Node* curr = NULL;
	PCB* temp = NULL;

	// Temporary pull the PCB out of CPU
	r_temp = Active;
	Active = NULL;

	// Check the SENDING BLOCK MESSAGE:
	curr = commu[0]->pFirstNode;
	while(curr != NULL){
		temp = curr->pItem;

		// If found appropriate sender
		// copy the send message of temp to receive message of r_temp
		if(temp->pID_receive == r_temp->pID){
			r_temp->pID_send = temp->pID;
			strcpy(r_temp->receive_Msg, temp->send_Msg);		// get the message
			Active = r_temp;									// keep current PCB at CPU
			goto Exit;
		}
		curr = curr->pNext;
	}

	// NO MESSAGE -> block in RECEIVING BLOCK MESSSAGE
	r_temp->state = 3;
	List_append(commu[1],r_temp);

	// NEW PCB OCCIPPIED CPU
	for(int i = 0; i < READY_MAX; i++){
		if(List_count(ready[i]) > 0){
			List_first(ready[i]);
			Active = List_remove(ready[i]);
			goto Exit;
		}	
	}

	// No more PCB in ready queue:
	if(Active == NULL){
		Active = init_Proc;
	}

	Totalinfo();
	return;

	Exit:
		printf("Source: PCB-%d\n", Active->pID_send);
		if(strlen(Active->receive_Msg) > 0)
			printf("Receive Message: %s\n", Active->receive_Msg);
		Totalinfo();
		return;
}

//reply_Proc(int proc_ID, char* msg): unblock the sender and deliver the reply
//return: 0 if sucess - 1 otherwise
int reply_Proc(int proc_ID, char* msg){
	
	// Check if the PCB ID is out of bound
	if(proc_ID < 0 || proc_ID > MAX_ID) goto Failure;

	// Check if the PCB is init_process:
	if(Active->pID == -1) goto Failure;

	PCB* y_temp = NULL;
	Node* curr = NULL;
	PCB* temp = NULL;

	// Temporary pull the PCB out of CPU
	y_temp = Active;

	// Loop through the sending list - find the sender to reply
	List_first(commu[0]);
	curr = commu[0]->pFirstNode;

	while(curr != NULL){
		temp = curr->pItem;

		//if found the sender - unblock and send the message to sender
		if(temp->pID == proc_ID){
			temp = List_remove(commu[0]);		// unblock the sender from sending block list		
			goto Success;
		}

		curr = List_next(commu[0]);
	}

	// if no sender found:	
	List_first(commu[0]);						// move the current to the head of the list
	temp = List_remove(commu[0]);				// remove the head of sending block list

	goto Success;

	Success:

		// add reply information to sender:
		temp->pID_reply = y_temp->pID;			// track of the replier ID
		strcpy(temp->reply_Msg, msg);			// reply to the sender
		
		// add the sender to an appropriate ready queue
		temp->state = 2;
		List_append(ready[temp->priority], temp);
		printf("Success\n");
		return 0;

	Failure:
		printf("Failure\n");
		return 1;
}

//new_Sem(int sem_ID, int init_V): initialize the named semaphore with given value init_V
//return: 0 if success - 1 otherwise
int new_Sem(int sem_ID, int init_V){

	if(num_Sem >= 5) goto Failure;

	//Check if reach maximum number of sempahore
	if(sem_ID >= 5 || sem_ID < 0) goto Failure;

	Semaphore* s_temp = malloc(sizeof(Semaphore));
	if(s_temp == NULL) goto Failure;

	//Assign info to semaphore:
	s_temp->sID = sem_ID;
	s_temp->sInit = init_V;
	s_temp->block_List = List_create();

	//add semaphore to semaphore array:
	sem_Array[num_Sem] = s_temp;
	num_Sem++;

	goto Success;

	Success:
		printf("Success\n");
		return 0;

	Failure:
		printf("Failure\n");
		return 1;
}

//sem_P(int sem_ID): execute the semaphore P operation - sem_ID = {0 to 4}
//return: 0 if sucess - 1 otherwise; action taken
int sem_P(int sem_ID){

	Semaphore* s_temp;
	if(num_Sem <= 0) goto Failure;

	int check = 0;
	for(int i = 0; i < num_Sem; i++){
		if(sem_Array[i]->sID == sem_ID){
			s_temp = sem_Array[i];
			check = 1;
		}	
	}
	
	if(check == 0) goto Failure;
	if(Active->pID == -1) goto Failure;

	s_temp->sInit--;
	PCB* p_temp = Active;

	if(s_temp->sInit < 0){

		//add the ACtive PCB to the block list
		if(Active->pID != -1){
			List_append(s_temp->block_List, Active);

			//Add new PCB to the CPU:
			for(int i = 0; i < 3; i++){
				if(List_count(ready[i]) > 0){
					List_first(ready[i]);
					Active = List_remove(ready[i]);
					goto Success;
				}
			}

			//Ready queue empty:
			Active = init_Proc;
		}
	}

	goto Success;

	Success:
		printf("Success\n");
		printf("Block-pID-%d on Semaphore-%d\n", p_temp->pID, sem_ID);
		return 0;

	Failure:
		printf("Failure\n");
		printf("Fail to Block\n");
		return 1;
}

//sem_V(int sem_ID): execute the semaphore V operation - sem_ID = {0 to 4}
//return: 0 if success - 1 otherwise; action taken
int sem_V(int sem_ID){

	Semaphore* s_temp;
	if(num_Sem <= 0) goto Failure;

	int check = 0;
	for(int i = 0; i < num_Sem; i++){
		if(sem_Array[i]->sID == sem_ID){
			s_temp = sem_Array[i];
			check = 1;
		}	
	}

	if(check == 0) goto Failure;
	if(List_count(s_temp->block_List) <= 0) goto Failure;
	

	PCB* p_temp;
	s_temp->sInit++;

	if(s_temp->sInit <= 0){

		//remove the Active PCB from the block list:
		List_first(s_temp->block_List);
		p_temp = List_remove(s_temp->block_List);

		//wake up the process - move it to the end of ready queue:
		List_append(ready[p_temp->priority],p_temp);	
	}

	goto Success;

	Success:
		printf("Success\n");
		printf("Unblock-pID-%d on Semaphore-%d\n", p_temp->pID, sem_ID);
		return 0;

	Failure:
		printf("Failure\n");
		printf("Fail to unblock\n");
		return 1;
}

//Procinfo(int proc_ID): display all info of the process to screen 
//return: void & all information to stdout
void Procinfo(int proc_ID){
	
	PCB* p_temp;
	Node* curr;

	if(Active->pID == proc_ID){
		p_temp = Active;
		printf("-------------------------------------------------------------------\n");
		printf("PCB on CPU\n");
		printf("-------------------------------------------------------------------\n");
		goto Print;
	}

	// READY QUEUE:
	for(int i = 0; i < READY_MAX; i++){

		curr = ready[i]->pFirstNode;

		while(curr != NULL){
			p_temp = curr->pItem;
			if(p_temp->pID == proc_ID){
				printf("-------------------------------------------------------------------\n");
				printf("PCB on Queue\n");
				printf("-------------------------------------------------------------------\n");
				goto Print;
			}
			curr = curr->pNext;
		}
	}


	// SEMAPHORE
	for(int i = 0; i < num_Sem; i++){
		
		curr = sem_Array[i]->block_List->pFirstNode;

		while(curr != NULL){
			p_temp = curr->pItem;
			if(p_temp->pID == proc_ID){
				printf("-------------------------------------------------------------------\n");
				printf("PCB on Semaphore\n");
				printf("-------------------------------------------------------------------\n");
				goto Print;
			}
			curr = curr->pNext;
		}
	}

	// SENDING/REACEIVING BLOCK LIST
	for(int i = 0; i < 2; i++){
		curr = commu[i]->pFirstNode;

		while(curr != NULL){
			p_temp = curr->pItem;
			if(p_temp->pID == proc_ID){
				printf("-------------------------------------------------------------------\n");
				if(i == 0) printf("PCB on SENDING BLOCK LIST\n");
				if(i == 1) printf("PCB on RECEIVING BLOCK LIST\n");
				printf("-------------------------------------------------------------------\n");
				goto Print;
			}
			curr = curr->pNext;
		}
	}	

	printf("PCB not found\n");
	return;

	Print:
		printf("%s%d\n", msg_ID, p_temp->pID);
		printf("%s%d ", msg_Prior, p_temp->priority);
		printf("%s%d\n", msg_State, p_temp->state);

		if(strlen(p_temp->send_Msg) > 0){
			printf("%s%s\n", msg_Send, p_temp->send_Msg);
		}

		if(strlen(p_temp->receive_Msg) > 0){
			printf("Sender ID-%d\n", p_temp->pID_receive);
			printf("%s%s\n", msg_Rec, p_temp->receive_Msg);
		}
		else{
			printf("%sEmpty\n", msg_Rec);
		}

		if(strlen(p_temp->reply_Msg) > 0){
			printf("Replier ID-%d\n", p_temp->pID_reply);
			printf("%s%s\n", msg_Rep, p_temp->reply_Msg);
		}
		else{
			printf("%sEmpty\n", msg_Rep);
		}

		printf("-------------------------------------------------------------------\n");
		return;
}

//Totalinfo(): display all process queues and their contents
//return: void & all information to sdout
void Totalinfo(){
	
	PCB* p_temp;
	Node* curr;

	// Pre-fix "System":
	printf("-------------------------------------------------------------------\n");
	printf("%sInformation:\n",msg_Sys);
	printf("-------------------------------------------------------------------\n");

	//print current active PCB:
	p_temp = Active;
	printf("CPU: ");
	if(p_temp != NULL){
		printf("%s%d\n", msg_ID, p_temp->pID);
	}else{
		printf("NULL\n");
	}
	
	//print all ready queues:
	printf("Ready Queue:\n");
	for(int i = 0; i < 3; i++){

		//Ready Queue:
		curr = ready[i]->pFirstNode;
		printf("Queue %d: ",i);

		while(curr != NULL){
			p_temp = curr->pItem;
			printf("%s%d ", msg_ID, p_temp->pID);
			curr = curr->pNext;
		}	
		printf("\n");
	}

	

	//print all communication queues:
	printf("Send/Receive List:\n");
	for(int i = 0; i < 2; i++){
		curr = commu[i]->pFirstNode;
		if(i == 0){
			printf("Sending queue: ");
		}else{
			printf("Receiving queue: ");
		}
		
		while(curr != NULL){
			p_temp = curr->pItem;
			printf("%s%d ", msg_ID, p_temp->pID);
			curr = curr->pNext;
		}
		printf("\n");
	}


	//print all semaphore - block list:
	printf("-------------------------------------------------------------------\n");
	printf("Semaphore - block_List:\n");
	if(num_Sem > 0){

		for(int i = 0; i < num_Sem; i++){

			if(sem_Array[i] != NULL){

				printf("Semaphore-%d-initial-%d: ", sem_Array[i]->sID, sem_Array[i]->sInit);
				if(List_count(sem_Array[i]->block_List) > 0){
					
					curr = sem_Array[i]->block_List->pFirstNode;

					while(curr != NULL){
						p_temp = curr->pItem;
						printf("%s%d ", msg_ID, p_temp->pID);
						curr = curr->pNext;
					}
					printf("\n");	
				}
				else{
					printf("Empty\n");
				}
			}

		}
	}else{
		printf("Semaphore: Empty\n");
	}

	printf("-------------------------------------------------------------------\n");

	return;
}

//#########################################################################################
//###							Prorgarm Administration									###	
//###					prog_Admin - init_Admin - regular_Admin							###
//###																					###
//#########################################################################################

//char_parameter(): get array of parameter from user input
//return: void
void char_Args(){

	// if(buf[0] == 'F' || buf[0] == 'E' || buf[0] == 'R' || buf[0] == 'T' || 
	// 	buf[0] == 'f' || buf[0] == 'e' || buf[0] == 'r' || buf[0] == 't') return;
	int i;
	memset(first_Args, 0, sizeof(char)*8);
	memset(second_Args, 0, sizeof(char)*40);

	// COMMAND NEED 1 ARGUMENT
	if(buf[0] == 'C' || buf[0] == 'K' || buf[0] == 'P' || buf[0] == 'V' || buf[0] == 'I' ||
		buf[0] == 'c' || buf[0] == 'k' || buf[0] == 'p' || buf[0] == 'v' || buf[0] == 'i'){
		
		i = 2;
		while(buf[i] != '\0' && buf[i] != '\n' && buf[i] != ' '){
			first_Args[i-2] = buf[i];
			i++;
		}
	
		return;
	}

	// COMMAND NEED 2 ARGUMENTS
	if(buf[0] == 'S' || buf[0] == 'Y' || buf[0] == 'N' ||
		buf[0] == 's' || buf[0] == 'y' || buf[0] == 'n'){

		i = 2;
		int j = 0;
		while(buf[i] != '\0' && buf[i] != '\n' && buf[i] != ' '){
			first_Args[j] = buf[i];
			i++;
			j++;
		}
		first_Args[j] = '\0';

		i++;
		j = 0;
		while(buf[i] != '\0' && buf[i] != '\n'){
			second_Args[j] = buf[i];
			i++;
			j++;
		}
		second_Args[j] = '\0';

		return;
	}
}


//PCB_Free(PCB* proc_PCB): free the process control block;
//return: void
void PCB_Free(PCB* proc_PCB){
	proc_PCB->pID = 0;
	proc_PCB->priority = 0;
	proc_PCB->state = 0;

	proc_PCB->pID_send = 0;
	if(proc_PCB->send_Msg != NULL){
		free(proc_PCB->send_Msg);
		proc_PCB->send_Msg = NULL;
	}

	proc_PCB->pID_receive = 0;
	if(proc_PCB->receive_Msg != NULL){
		free(proc_PCB->receive_Msg);
		proc_PCB->receive_Msg = NULL;
	}

	proc_PCB->pID_reply	= 0;
	if(proc_PCB->reply_Msg != NULL){
		free(proc_PCB->reply_Msg);
		proc_PCB->reply_Msg = NULL;
	}

	return;
}

//mem_Free(): free all the memorize that have been used
//Assume all ready queue is empty before memset is called
//return: void
void mem_Free(){
	
	PCB* temp = NULL;
	for(int i = 0; i < READY_MAX; i++){

		while(List_count(ready[i]) > 0){
			temp = List_trim(ready[i]);
			PCB_Free(temp);

			free(temp);
			temp = NULL;
		}
	}

	// Free SENDING/RECEIVING BLOCK LIST:
	for(int i = 0; i < 2; i++){

		while(List_count(commu[i]) > 0){
			temp = List_trim(commu[i]);
			PCB_Free(temp);

			free(temp);
			temp = NULL;
		}
	}

	// Free SEMAPHORE:
	for(int i = 0; i < num_Sem; i++){
		if(sem_Array[i] != NULL){

			sem_Array[i]->sID = 0;
			sem_Array[i]->sInit = 0;

			// FREE BLOCK LIST OF EACH SEMAPHORE
			while(List_count(sem_Array[i]->block_List) > 0){
				temp = List_trim(sem_Array[i]->block_List);
				PCB_Free(temp);
				
				free(temp);
				temp = NULL;
			}
			
			sem_Array[i]->block_List = NULL;	
			free(sem_Array[i]);
			sem_Array[i] = NULL;
		}
	}

	return;
}


//init_Admin(): the function inside init process
//return: 0 successfully terminate - 1 go to regular - 2 go to error
int init_Admin(){

	int check;
	int int_Args;
	int init_value;

	fgets(buf, sizeof(buf), stdin);
	fflush(stdin);
	command = buf[0];

	// CREATE COMMAND
	if(command == 'C' || command == 'c'){
		char_Args();
		int_Args = atoi(first_Args);
		create_Proc(int_Args);
	}

	// KILL COMMAND
	if(command == 'K' || command == 'k'){
		char_Args();
		int_Args = atoi(first_Args);
		check = kill_Proc(int_Args);

		if(check == 2){
			mem_Free();
			goto Exit_Program;
		} 
	}

	// EXIT COMMAND
	if(command == 'E' || command == 'e'){
	 	exit_Proc();
	 	goto Exit_Program;
	}

	// NEW SEMAPHORE COMMAND
	if(command == 'N' || command == 'n'){
		char_Args();
		int_Args = atoi(first_Args);
		init_value = atoi(second_Args);
		new_Sem(int_Args, init_value);
	}

	// SEMMAPHORE P COMMAND
	if(command == 'P' || command == 'p'){
		char_Args();
		int_Args = atoi(first_Args);
		sem_P(int_Args);
	}

	// PROCINFO COMMAND
	if(command == 'I' || command == 'i'){
		char_Args();
		int_Args = atoi(first_Args);
		Procinfo(int_Args);
	}

	// TOTALINFO COMMAND
	if(command == 'T' || command == 't'){
		Totalinfo();
	}

	return 0;

	Exit_Program:
		return 1;	
}

//regular_Admin(): the function inside init process
//return: 0 if success - 1 otherwise
int regular_Admin(){

	int int_Args;
	int init_value;

	fgets(buf, sizeof(buf), stdin);
	fflush(stdin);
	command = buf[0];

	// CREATE COMMAND
	if(command == 'C' || command == 'c'){
		char_Args();
		int_Args = atoi(first_Args);
		create_Proc(int_Args);
	}

	// FORK COMMAND
	if(command == 'F' || command == 'f'){
		fork_Proc();
	}

	// KILL COMMAND
	if(command == 'K' || command == 'k'){
		char_Args();
		int_Args = atoi(first_Args);
		kill_Proc(int_Args);
	}

	// EXIT COMMAND
	if(command == 'E' || command == 'e'){
	 	exit_Proc();
	}

	// QUANTUM COMMAND
	if(command == 'Q' || command == 'q'){
		quant_Time();
	}

	// SEND COMMAND
	if(command == 'S' || command == 's'){
		char_Args();
		int_Args = atoi(first_Args);
		send_Proc(int_Args,second_Args);		
	}

	// RECEIVE COMMAND
	if(command == 'R' || command == 'r'){
		recv_Proc();
	}

	// REPLY COMMAND
	if(command == 'Y' || command == 'y'){
		char_Args();
		int_Args = atoi(first_Args);
		reply_Proc(int_Args,second_Args);		
	}

	// NEW SEMAPHORE COMMAND
	if(command == 'N' || command == 'n'){
		char_Args();
		int_Args = atoi(first_Args);
		init_value = atoi(second_Args);	
		new_Sem(int_Args,init_value);
	}

	// SEMMAPHORE P COMMAND
	if(command == 'P' || command == 'p'){
		char_Args();
		int_Args = atoi(first_Args);
		sem_P(int_Args);

	}

	// SEMMAPHORE V COMMAND
	if(command == 'V' || command == 'v'){
		char_Args();
		int_Args = atoi(first_Args);
		sem_V(int_Args);
	}

	// PROCINFO COMMAND
	if(command == 'I' || command == 'i'){
		char_Args();
		int_Args = atoi(first_Args);
		Procinfo(int_Args);
	}

	// TOTALINFO COMMAND
	if(command == 'T' || command == 't'){
		Totalinfo();
	}

	return 0;
}

//prog_Active(): the function inside init process
//return: 0 if success - 2 error
int prog_Admin(List* list_0, List* list_1, List* list_2, List* send_list, List* recv_list){
	ready[0] = list_0;
	ready[1] = list_1;
	ready[2] = list_2;

	commu[0] = send_list;
	commu[1] = recv_list;

	int go;
	//Create the init PCB - and start the program
	create_Proc(2);

	while(1){
		if(Active->pID == -1){
			go = init_Admin();

			if(go == 1) goto Exit_Program;
		}
		else{
			go = regular_Admin();

		}	
	}

	Exit_Program:
		return 0;
}

	