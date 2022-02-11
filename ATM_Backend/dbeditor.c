#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <sys/shm.h> // allows access to 
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#define MAX 256
#include "header.h"

/*
method to get user input and put it all into once nice string to send to the Database
*/


int main() {

    my_msg_st msg;
	void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid;
    //char buffer[BUFSIZ];
    // created shared memory
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    //makes the shmid is valid
    if (shmid == -1) { fprintf(stderr, "shmget failed\n"); exit(EXIT_FAILURE); }

    // enables acces to shared memory
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) { fprintf(stderr, "shmat failed\n"); exit(EXIT_FAILURE); }
    
    shared_stuff = (struct shared_use_st *)shared_memory;

	// create msg Q
	int msgid = msgget((key_t)toDB_KEY, 0666 | IPC_CREAT);
	if (msgid == -1) { fprintf(stderr, "\nmsgget failed with error: %d\n", errno); exit(EXIT_FAILURE); }


	while(shared_stuff->close != 'x'){
		printf("\nEnter Account Number to be added to DB!!! \n");
		if(shared_stuff->close == 'x'){break;}
		scanf("%s",msg.info.account);	
		fflush(stdin);

		printf("\nEnter Pin to be added to DB!!! \n");
		if(shared_stuff->close == 'x'){break;}
		scanf("%s",msg.info.pin);
		fflush(stdin);
		
		printf("\nEnter Amount of Funds to account\n ");
		if(shared_stuff->close == 'x'){break;}
		scanf("%f", &(msg.info.funds));
		fflush(stdin);
		msg.type = updateDB;
		
		
		// just the UPDATE_DB MSG GETS SENT to DBSERVER
		if ( msgsnd(msgid, (void *)&msg, sizeof(msg.info), 0) == -1) { fprintf(stderr, "\nUPDATE_DB MSG msgsnd failed\n"); exit(EXIT_FAILURE); } 
	}

	// close msg Q
	if (msgctl(msgid, IPC_RMID, 0) == -1) { fprintf(stderr, "failed to close ATM2decide msg que\n"); exit(EXIT_FAILURE); }

    // detach shared memory 
    if (shmdt(shared_memory) == -1) { fprintf(stderr, "shmdt failed\n"); exit(EXIT_FAILURE); } 
    // close shared memory 
    if (shmctl(shmid, IPC_RMID, 0) == -1) { fprintf(stderr, "shmctl(IPC_RMID) failed\n"); exit(EXIT_FAILURE); }

	return 0; 
}