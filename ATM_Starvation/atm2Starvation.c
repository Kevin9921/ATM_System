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
#define SEM_MODE 0644 

static int set_semvalue(int);
static void del_semvalue(int);
static int semaphore_p(int);
static int semaphore_v(int); 
union semun
{
        int val;
        struct semid_ds *buf;
        unsigned short  *array;
        struct seminfo *__buf;
};

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

	my_msg_st send, recieve; 
	//sets up the message queues for recieveing from decide sending to decide
	int  queue_id_db; 
	int msgLength = sizeof(my_msg_st) -sizeof(long);


	int decide2ATM2=msgget(ATM2_KEY, IPC_CREAT | 0600);
	if (decide2ATM2 == -1) {
		perror("Creating ATM message queue.");
		msgctl(queue_id_db, IPC_RMID,0);
		exit(1);
	}
	int ATMtodecide =msgget(allAtms_KEY, IPC_CREAT | 0600);
	if (ATMtodecide == -1) {
		perror("Creating DB message queue.");
		exit(1);
	}
	int sem_id1 = semget((key_t)1336, 1, 0666 | IPC_CREAT);
    int sem_id2 = semget((key_t)1337, 1, 0666 | IPC_CREAT);
	 printf("Semaphore key = %d\n",sem_id1); 
     printf("Semaphore key = %d\n",sem_id2); 
	FILE *fpr2;
	fpr2 = fopen("logS.txt", "a+");
	while(shared_stuff->close != 'x'){

		
		
		while(true){
			while(true){
				semaphore_p(sem_id1);
				semaphore_p(sem_id2);
                 printf("acquired semaphore 2\n");
				printf("\nPlease enter account number: \n");
				//fpr2 = fopen("log.txt", "a");
			
				//fclose(fpr2);
				scanf("%s",send.info.account);	
				fflush(stdin);
				fprintf(fpr2, "atm2 acquired account %s\n", &send.info.account);
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
					fprintf(fpr2, "atm2 got pin %s\n", &send.info.pin);
					break; 
				}
			}

			sleep(10);
			semaphore_v(sem_id1);
			semaphore_v(sem_id2);
			fprintf(fpr2, "atm2 released semaphore 1\n");
			fprintf(fpr2, "atm2 released semaphore 2\n");
			fclose(fpr2);
			if(shared_stuff->close == 'x'){break;}
			send.type = pin2;
			int goodPin = 0;
			while(!goodPin){
				if (msgsnd(ATMtodecide, (void *)&send,msgLength, 0) == -1) {
					fprintf(stderr, "\naccount msgsnd failed ATM\n");
					exit(EXIT_FAILURE);
				}

				if (msgrcv(decide2ATM2, (void *)&recieve, msgLength, msg_to_receive, 0) == -1)
				{
					fprintf(stderr, "\nmsgrcv failed with error: %d ATM\n", errno);
					exit(EXIT_FAILURE);
				}
				if (recieve.type == accountWrong){
					printf("wrong account\n");
					break;
				}else if(recieve.type == accountBlock){
					printf("account block\n");
					break;
				}else if (recieve.type == pinRight){
					printf("good pin\n");
					goodPin = 1;
				}else if (recieve.type == pinWrong){
					printf("bad pin");
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
				send.type = getFunds2;
				send.info.funds = -1;
				break;
			}else if (!strcmp(temp, "W")) {
				send.type = getFunds2;
				strcpy(send.info.pin,"5X5"); 
				printf("\ntype in amount: ");
				scanf("%f",&send.info.funds);
				fflush(stdin);
				printf("\n");
				break;
			}else if (!strcmp(temp, "D")) {
				send.type = getFunds2;
				strcpy(send.info.pin,"2X2"); 
				printf("\ntype in desposit amount: ");
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
		
		if (msgrcv(decide2ATM2, (void *)&recieve, msgLength, msg_to_receive, 0) == -1)
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

		send.type = end2;
		if (msgsnd(ATMtodecide, (void *)&send,msgLength, 0) == -1) {
			fprintf(stderr, "\naccount msgsnd failed ATM\n");
			exit(EXIT_FAILURE);
		}

	}
	if (msgctl(decide2ATM2, IPC_RMID, 0) == -1)
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

static int set_semvalue(int sem_id)
{
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
        return (0);
    return (1);
}

static void del_semvalue(int sem_id)
{
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_p failed\n");
        return (0);
    }
    return (1);
}

static int semaphore_v(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_v failed\n");
        return (0);
    }
    return (1);
}