#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "simu.h"

int main(){
	List* ready_q0 = List_create();
	List* ready_q1 = List_create();
	List* ready_q2 = List_create();

	List* send_L = List_create();
	List* recv_L = List_create();

	prog_Admin(ready_q0, ready_q1, ready_q2, send_L, recv_L);

	return 0;
}