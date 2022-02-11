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


/*
all atms have the same functionality so only the first one was commented.
The only difference is the message queue keys switching depending on atm
(ie atm1 has decide2ATM1, atm2 has decide2ATM2) and the message types sent
to the atmDecide (ie atm1 types are pin1, end1, getFunds1 and atm2 types are
pin2, end2, getfunds2) types increase by one with each ATM
*/


int main(void){
	
	long int msg_to_receive = 0;

	void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid;
   
    // created shared memory
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
    shared_stuff = (struct shared_use_st *)shared_memory;



	// send and recieve messages 
	my_msg_st send, recieve; 
	//queue_id_atm, fromATMs,
	int  queue_id_db; 
	int msgLength = sizeof(my_msg_st) -sizeof(long);

	//sets up message queue to recieve from atmDecide
	int decide2ATM1=msgget(ATM1_KEY, IPC_CREAT | 0600);
	if (decide2ATM1 == -1) {
		perror("Creating ATM message queue.");
		msgctl(queue_id_db, IPC_RMID,0);
		exit(1);
	}

	//sets up message queue to send from atmDecide
	int ATM2decide =msgget(allAtms_KEY, IPC_CREAT | 0600);
	if (ATM2decide == -1) {
		perror("Creating DB message queue.");
		exit(1);
	}

	//main loop only closes when user puts in X
	while(shared_stuff->close != 'x'){
		while(true){
			//gets account and pin
			while(true){
				printf("\nPlease enter account number: \n");
				scanf("%s",send.info.account);	
				fflush(stdin);
				
				//checks to close atm
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
			send.type = pin1;
			int goodPin = 0;

			//sends account and pin to atmDecide
			while(!goodPin){
				if (msgsnd(ATM2decide, (void *)&send,msgLength, 0) == -1) {
					fprintf(stderr, "\naccount msgsnd failed ATM\n");
					exit(EXIT_FAILURE);
				}

				if (msgrcv(decide2ATM1, (void *)&recieve, msgLength, msg_to_receive, 0) == -1)
				{
					fprintf(stderr, "\nmsgrcv failed with error: %d ATM\n", errno);
					exit(EXIT_FAILURE);
				}
				//if the account is not in the database
				if (recieve.type == accountWrong){
					printf("Account does not exist\n");
					break;
				//if recieve type is account blocked start from beginning 
				}else if(recieve.type == accountBlock){
					printf("account is blocked block\n");
					break;
				//if the pin is right set goodPin to enter next loop
				}else if (recieve.type == pinRight){
					printf("good pin");
					goodPin = 1;
				// if the account is good but the pin is bad user re-enters pin	
				}else if (recieve.type == pinWrong){
					printf("bad pin\n");
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
		
		//user choses a withdrawal, deposit or chequing
		if(shared_stuff->close == 'x'){break;}
		while(true){
			char temp[15];
			printf("\ntype in CHEQUING OR WITHDRAWAL\n");
			scanf("%s",temp);
			fflush(stdin);
			if (!strcmp(temp, "C")){
				send.type = getFunds1;
				send.info.funds = -1;
				break;
			}else if (!strcmp(temp, "W")) {
				send.type = getFunds1;
				strcpy(send.info.pin,"5X5"); 
				printf("\ntype in amount: ");
				scanf("%f",&send.info.funds);
				fflush(stdin);
				printf("\n");
				break;
			}else if (!strcmp(temp, "D")) {
				send.type = getFunds1;
				strcpy(send.info.pin,"2X2"); 
				printf("\ntype in desposit amount: ");
				scanf("%f",&send.info.funds);
				fflush(stdin);
				printf("\n");
				break;
			}else{
				printf("decision not valid try again");
			}
		}
		//send to atmDecide users decision
		if (msgsnd(ATM2decide, (void *)&send,msgLength, 0) == -1) {
			fprintf(stderr, "\naccount msgsnd failed ATM\n");
			exit(EXIT_FAILURE);
		}
		
		if (msgrcv(decide2ATM1, (void *)&recieve, msgLength, msg_to_receive, 0) == -1)
		{
			fprintf(stderr, "\nmsgrcv failed with error: %d ATM\n", errno);
			exit(EXIT_FAILURE);
		}
		//print based on recieve type
		if (recieve.type == notEnoughFunds){
			printf("\nnot enough funds\n");
		}else if(recieve.type == currentfund){
			printf("\ncurrent balance: %f\n", recieve.info.funds);
		}else if(recieve.type == EnoughFunds){
			printf("\nyour account balance is now: %f \n", recieve.info.funds);
		}

		//send end message to Server
		send.type = end1;
		if (msgsnd(ATM2decide, (void *)&send,msgLength, 0) == -1) {
			fprintf(stderr, "\naccount msgsnd failed ATM\n");
			exit(EXIT_FAILURE);
		}
	
	}
	
	//close message queues
	if (msgctl(decide2ATM1, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, " failed to clsoe decide2ATM1 msg que\n");
        exit(EXIT_FAILURE);
    }

    if (msgctl(ATM2decide, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "failed to close ATM2decide msg que\n");
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

