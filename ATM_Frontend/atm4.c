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
#include <errno.h>
#include "header.h"
#include <sys/ipc.h>

int main(void){
	long int msg_to_receive = 0;
	void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid;

    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

    //makes the shmid is valid
    if (shmid == -1)
    {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }
    // enables acces to shared memory
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    //printf("Memory attached at %X\n", (int)shared_memory);
    shared_stuff = (struct shared_use_st *)shared_memory;

	// stuff we need 
	my_msg_st send, recieve; 
	//queue_id_atm, fromATMs,
	int  queue_id_db; 
	int msgLength = sizeof(my_msg_st) -sizeof(long);


	int decide2ATM4=msgget(ATM4_KEY, IPC_CREAT | 0600);
	if (decide2ATM4 == -1) {
		perror("Creating ATM message queue.");
		msgctl(queue_id_db, IPC_RMID,0);
		exit(1);
	}
	int ATMtodecide =msgget(allAtms_KEY, IPC_CREAT | 0600);
	if (ATMtodecide == -1) {
		perror("Creating DB message queue.");
		exit(1);
	}

	while(shared_stuff->close != 'x'){
		while(true){
			while(true){
				printf("\nPlease enter account number: \n");
				
				scanf("%s",send.info.account);	
				fflush(stdin);
				
				if( send.info.account[0] == 'X' || send.info.account[0] == 'x' ){
					printf("\nATM: Have a good day!\n"); 
					shared_stuff->close = 'x';
					break;
				}
				else{
					printf("\nPlease enter pin number: \n");
					scanf("%s",send.info.pin);
					fflush(stdin);
					printf("\nATM: Inputed Account %s \n",send.info.account);
					printf("\nATM: Inputed Pin  %s\n",send.info.pin);
					break; 
				}
			}
			if(shared_stuff->close == 'x'){break;}
			send.type = pin4;
			int goodPin = 0;
			while(!goodPin){
				if (msgsnd(ATMtodecide, (void *)&send,msgLength, 0) == -1) {
					fprintf(stderr, "\naccount msgsnd failed ATM\n");
					exit(EXIT_FAILURE);
				}

				if (msgrcv(decide2ATM4, (void *)&recieve, msgLength, msg_to_receive, 0) == -1)
				{
					fprintf(stderr, "\nmsgrcv failed with error: %d ATM\n", errno);
					exit(EXIT_FAILURE);
				}
				if (recieve.type == accountWrong){
					printf("\nwrong account");
					break;
				}else if(recieve.type == accountBlock){
					printf("\naccount block");
					break;
				}else if (recieve.type == pinRight){
					printf("\ngood pin");
					goodPin = 1;
				}else if (recieve.type == pinWrong){
					printf("\nbad pin");
					printf("\nPlease enter pin number: \n");
					scanf("%s",send.info.pin);
					fflush(stdin);
					printf("\nATM: Inputed Account %s \n",send.info.account);
					printf("\nATM: Inputed Pin  %s\n",send.info.pin);
				}
					
			}
			if (goodPin == 1){
				break;
			}
		}
		if(shared_stuff->close == 'x'){break;}
		while(true){
			char temp[15];
			printf("type in CHEQUING OR WITHDRAWAL\n");
			scanf("%s",temp);
			fflush(stdin);
			if (!strcmp(temp, "C")){
				send.type = getFunds4;
				send.info.funds = -1;
				break;
			}else if (!strcmp(temp, "W")) {
				send.type = getFunds4;
				strcpy(send.info.pin,"5X5"); 
				printf("\ntype in amount:");
				scanf("%f",&send.info.funds);
				fflush(stdin);
				printf("\n");
				break;
			}else if (!strcmp(temp, "D")) {
				send.type = getFunds4;
				strcpy(send.info.pin,"2X2"); 
				printf("\ntype in desposit amount:");
				scanf("%f",&send.info.funds);
				fflush(stdin);
				printf("\n");
				break;
			}else{
				printf("decision not valid try again\n");
			}
		}
		if (msgsnd(ATMtodecide, (void *)&send,msgLength, 0) == -1) {
			fprintf(stderr, "\naccount msgsnd failed ATM\n");
			exit(EXIT_FAILURE);
		}
		
		if (msgrcv(decide2ATM4, (void *)&recieve, msgLength, msg_to_receive, 0) == -1)
		{
			fprintf(stderr, "\nmsgrcv failed with error: %d ATM\n", errno);
			exit(EXIT_FAILURE);
		}
		if (recieve.type == notEnoughFunds){
			printf("\nnot enough funds\n");
		}else if(recieve.type == currentfund){
			printf("\ncurrent balance: %f\n", recieve.info.funds);
		}else if(recieve.type == EnoughFunds){
			printf("\nyour account balance is now: %f \n", recieve.info.funds);
		}

		send.type = end4;
		if (msgsnd(ATMtodecide, (void *)&send,msgLength, 0) == -1) {
			fprintf(stderr, "\naccount msgsnd failed ATM\n");
			exit(EXIT_FAILURE);
		}

	}
	if (msgctl(decide2ATM4, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, " failed to clsoe decide2ATM1 msg que\n");
        exit(EXIT_FAILURE);
    }

    if (msgctl(ATMtodecide, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "failed to close ATMtodecide msg que\n");
        exit(EXIT_FAILURE);
    }

    // detach shared memory 
    if (shmdt(shared_memory) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    // close shared memory 
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
	return 0;
}

