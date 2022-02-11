#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
/* We need __USE_GNU for the MSG_EXCEPT flag */
#define __USE_GNU
#include <sys/shm.h> // allows access to 
#include <sys/msg.h>
#include <string.h>
#include<sys/sem.h>
/* Include the message definitions, etc. */
#include "header.h"
#define SEM_MODE 0644 


union semun
{
        int val;
        struct semid_ds *buf;
        unsigned short  *array;
        struct seminfo *__buf;
};


int main(int argc, char **argv)  {
	
	
   	// msg Q ids 
	int send2db, recieveFromDB, fromAllAtms, send2ATM1,send2ATM2,send2ATM3,send2ATM4, send2ATM5;
	

	// message structs from header file 
	my_msg_st msg, atm_msg, send;

	// creates shared memory for all of the c files 
    void *shared_memory = (void *)0;
    struct shared_use_st *shared_stuff;
    int shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    //makes the shmid is valid

    if (shmid == -1){ fprintf(stderr, "shmget failed\n"); exit(EXIT_FAILURE); }

    // enables acces to shared memory
    shared_memory = shmat(shmid, (void *)0, 0);
    if (shared_memory == (void *)-1) { fprintf(stderr, "shmat failed\n"); exit(EXIT_FAILURE); }

    shared_stuff = (struct shared_use_st *)shared_memory;
	

	/* Create message queues */
    
	fromAllAtms = msgget(allAtms_KEY, IPC_CREAT | 0600); 
	if ( fromAllAtms == -1)	{ perror("Creating DB message queue.\n"); exit(1); }

	send2db = msgget(toDB_KEY, IPC_CREAT | 0600);
	if (send2db == -1) { perror("FAILED: Creating send2db message queue."); exit(1); }

	recieveFromDB = msgget(Decide_KEY, IPC_CREAT | 0600);
	if (recieveFromDB == -1) { perror("FAILED: Creating recieveFromDB message queue."); exit(1); }

	send2ATM1 = msgget(ATM1_KEY, IPC_CREAT | 0600);
	if (send2ATM1 == -1){ perror("FAILED: Creating send2ATM1 message queue."); msgctl(send2db, IPC_RMID, 0); exit(1); }


	send2ATM2=msgget(ATM2_KEY, IPC_CREAT | 0600);
	if (send2ATM2 == -1) {perror("FAILED: Creating send2ATM2 message queue.\n"); msgctl(send2db, IPC_RMID,0); exit(1);}

    send2ATM3=msgget(ATM3_KEY, IPC_CREAT | 0600);
	if (send2ATM3 == -1) { perror("FAILED: Creating send2ATM3 message queue.\n"); msgctl(send2db, IPC_RMID,0); exit(1); }

    send2ATM4=msgget(ATM4_KEY, IPC_CREAT | 0600);
	if (send2ATM4 == -1) { perror("FAILED: Creating send2ATM4 message queue.\n"); msgctl(send2db, IPC_RMID,0); exit(1); }

    send2ATM5=msgget(ATM5_KEY, IPC_CREAT | 0600);
	if (send2ATM5 == -1) { perror("FAILED: Creating send2ATM5 message queue.\n"); msgctl(send2db, IPC_RMID,0); exit(1); } 


    int semSet = semget(IPC_PRIVATE,1,IPC_CREAT | SEM_MODE);
    printf("Semaphore key = %d\n",semSet); 

    union semun sem_init;
    sem_init.val = 5;
    semctl(semSet, 0, SETVAL, sem_init);
    struct sembuf sem; 
    int semVal, semVal2;


	// if this is false then close msg  Qs and shared memory 
	while(shared_stuff->close != 'x'){
		
		
        if(shared_stuff->close == 'x'){break;} 
        if ((msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), 0, 0))== -1){printf("error"); break;}

		// if we are shutting system down we need to send msg to dbserver to close it correctly 
        if(shared_stuff->close == 'x'){
            msg.type = CloseEvery;
            if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1){printf("ERROR: \n");break;}    
            printf("\nSENDING: CloseEvery\n");
            break;
        }  
         
		printf("RECIEVED ACCOUN AND PIN\n");

		// code for all cases are the same except the msg ques they use 
		// each atm has its "own case" 
		switch(msg.type){

           case pin1:

				// atm1 aquires semaphore 
               sem.sem_num = 0;
               sem.sem_flg = SEM_UNDO;
               sem.sem_op = -1;
               semop(semSet, &sem, 1);
               printf("Semaphore acquired\n");
			


               // VIEW VALUE OF THE SEMAPHORE
               semVal = semctl(semSet, 0, GETVAL, 0);
               printf("Semaphore value is = %d\n", semVal);

               msg.type = pinNumber;
               

               if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1){printf("ERROR: \n");break;}

            	printf("sent the check\n");

                if (msgrcv(recieveFromDB, (struct msgbuf *)&send, sizeof(msg.info), 0, 0) == -1){printf("recieve from server ok\n"); break;}

                printf("\ntype %d\n",send.type);
				// if the msg we recieved is either of these restart from the begining of the loop
                if (send.type == pinWrong || send.type == accountBlock || send.type == accountWrong ){

                    if (msgsnd(send2ATM1, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) {printf("send to atm 1 \n"); break;}
					// release the semaphore 
                    sem.sem_num = 0;
                    sem.sem_flg = SEM_UNDO;
                    sem.sem_op = 1;
                    semop(semSet, &sem, 1);
                    printf("Semaphore releaseed\n");

                    semVal = semctl(semSet, 0, GETVAL, 0);
                    printf("Semaphore value is = %d\n", semVal);
                    break; // break out of switch case 
                }

                if (msgsnd(send2ATM1, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) {printf("send to atm 1 ok\n"); break;}

                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (getFunds1), 0)==-1){printf("recieve from atm request funds\n");break;}
                
				// checking to see what transaction we user wants to apply
				if (msg.info.funds == -1){
                    msg.type = request;
                }else{
                    if(!strncmp(msg.info.pin,"5X5",3)){
                        strncpy(msg.info.pin,"",3);
                        msg.type = withdraw;
                    }else if(!strncmp(msg.info.pin,"2X2",3)){
                        strncpy(msg.info.pin,"",3);
                        msg.type = deposit;
                        printf("Deposit got\n");
                    }
                   
                }
                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0)==-1){printf("send to db request funds\n"); break;}
                if (msgrcv(recieveFromDB, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0, 0) == -1){printf("recieve from db request funds amount\n"); break;}
                if (msgsnd(send2ATM1, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0)== -1){printf("send to atm1 request funds message\n"); break;}
                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (end1), 0)== -1){printf("recieve from atm transaction end from atm\n"); break;}
				
                msg.type = endTrans;
    
                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0)==-1){printf("Send to db trnsaction end\n"); break;}

                // RELEASE RESOURCE
                sem.sem_num =0;
                sem.sem_flg = SEM_UNDO;
                sem.sem_op = 1;
                semop(semSet,&sem,1);
                printf("Semaphore releaseed\n");


                semVal = semctl(semSet,0,GETVAL,0);
                printf("Semaphore value is = %d\n",semVal);

				break;// breaks out of case 

            case pin2:


               sem.sem_num = 0;
               sem.sem_flg = SEM_UNDO;
               sem.sem_op = -1;
               semop(semSet, &sem, 1);
               printf("Semaphore acquired\n");

			   

               msg.type = pinNumber;

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to server pin\n"); break; }
                printf("sent the check\n");
                if (msgrcv(recieveFromDB, (struct msgbuf *)&send, sizeof(msg.info), 0, 0) == -1) { printf("recieve from server ok\n"); break; }

                if (send.type == pinWrong || send.type == accountBlock || send.type == accountWrong ){
                    if (msgsnd(send2ATM2, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) {printf("send to atm 2 ok\n"); break;}
                    sem.sem_num = 0;
                    sem.sem_flg = SEM_UNDO;
                    sem.sem_op = 1;
                    semop(semSet, &sem, 1);
                    printf("Semaphore releaseed\n");

                    semVal = semctl(semSet, 0, GETVAL, 0);
                    printf("Semaphore value is = %d\n", semVal);
                    break;
                }
                if (msgsnd(send2ATM2, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) { printf("send to atm 2 ok\n"); break; }

                if (msgrcv(fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info),(getFunds2), 0) == -1) { printf("recieve from atm request funds\n");break;}

                if (msg.info.funds == -1){
                    msg.type = request;
                }else{
                    if(!strncmp(msg.info.pin,"5X5",3)){
                        msg.type = withdraw;
                        strncpy(msg.info.pin,"",3);
                    }else if(!strncmp(msg.info.pin,"2X2",3)){
                        msg.type = deposit;
                        strncpy(msg.info.pin,"",3);
                        printf("Deposit got\n");
                        
                    }
                   
                }
                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) {printf("send to db request funds\n"); break; }

                if (msgrcv(recieveFromDB, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0, 0) == -1) { printf("recieve from db request funds amount\n"); break;}

                if (msgsnd(send2ATM2, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0) == -1){ printf("send to atm2 request funds message\n"); break; }

                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (end2), 0) == -1) { printf("recieve from atm transaction end from atm\n"); break; }

                msg.type = endTrans;

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1)
                {
                    printf("send to db trnsaction end\n");
                    break;
                }


                 // RELEASE RESOURCE
                sem.sem_num =0;
                sem.sem_flg = SEM_UNDO;
                sem.sem_op = 1;
                semop(semSet,&sem,1);
                printf("Semaphore releaseed\n");

                break;
            case pin3:
                sem.sem_num = 0;
               sem.sem_flg = SEM_UNDO;
               sem.sem_op = -1;
               semop(semSet, &sem, 1);
               printf("Semaphore acquired\n");


            


                msg.type = pinNumber;
                //strcpy(msg.u.check_pin.account,atm_msg.u.check_pin.account);
                //strcpy(msg.u.check_pin.pin,atm_msg.u.check_pin.pin);

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to server pin\n"); break; }
                printf("sent the check\n");
                if (msgrcv(recieveFromDB, (struct msgbuf *)&send, sizeof(msg.info), 0, 0) == -1) { printf("recieve from server ok\n"); break; }

                if (send.type == pinWrong || send.type == accountBlock  || send.type == accountWrong){

                    if (msgsnd(send2ATM3, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) {printf("send to atm 2 ok\n"); break;}
                    sem.sem_num = 0;
                    sem.sem_flg = SEM_UNDO;
                    sem.sem_op = 1;
                    semop(semSet, &sem, 1);
                    printf("Semaphore releaseed\n");

                    semVal = semctl(semSet, 0, GETVAL, 0);
                    printf("Semaphore value is = %d\n", semVal);
                    break;
                }
                printf("semt to send2ATM3 ok \n");

                if (msgsnd(send2ATM3, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) { printf("send to atm 3 ok\n"); break;} 

                printf("recieve request (W,D,B)\n ");
                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (getFunds3), 0) == -1) { printf("recieve from atm request funds\n"); break; }

                if (msg.info.funds == -1){
                    msg.type = request;
                    printf("\natm 3 chequing\n");
                }else{
                    if(!strncmp(msg.info.pin,"5X5",3)){
                        msg.type = withdraw;
                        strncpy(msg.info.pin,"",3);
                          printf("\natm 3 withdrawal\n");
                    }else if(!strncmp(msg.info.pin,"2X2",3)){
                        msg.type = deposit;
                        strncpy(msg.info.pin,"",3);
                        printf("Deposit got\n");
                          printf("\natm 3 chequing\n");
                    }
                   
                }
                printf("send2ATM3 sending (W, D, B) to server\n");
                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to db request funds\n"); break; }
                 printf("send2ATM3 recieving (W, D, B) to server\n");

                if (msgrcv(recieveFromDB, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0, 0) == -1) { printf("recieve from db request funds amount\n"); break; }
                printf("send2ATM3 recieving (W, D, B) to server\n");

                if (msgsnd(send2ATM3, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0) == -1) { printf("send to atm2 request funds message\n"); break; }
                printf("send2ATM3 recieving (W, D, B) to server\n");

                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (end3), 0) == -1) { printf("recieve from atm transaction end from atm\n"); break; }

                msg.type = endTrans;

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to db trnsaction end\n"); break; }


                 // RELEASE RESOURCE
                sem.sem_num =0;
                sem.sem_flg = SEM_UNDO;
                sem.sem_op = 1;
                semop(semSet,&sem,1);
                printf("Semaphore releaseed\n");

                break;



            case pin4:    
                sem.sem_num = 0;
                sem.sem_flg = SEM_UNDO;
                sem.sem_op = -1;
                semop(semSet, &sem, 1);
                printf("Semaphore acquired\n");


                msg.type = pinNumber;
               

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to server pin\n"); break; }

                printf("sent the check\n");

                if (msgrcv(recieveFromDB, (struct msgbuf *)&send, sizeof(msg.info), 0, 0) == -1) { printf("recieve from server ok\n"); break; }

                if (send.type == pinWrong || send.type == accountBlock  || send.type == accountWrong ){
                    if (msgsnd(send2ATM4, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) {printf("send to atm 4 ok\n"); break;}
                    sem.sem_num = 0;
                    sem.sem_flg = SEM_UNDO;
                    sem.sem_op = 1;
                    semop(semSet, &sem, 1);
                    printf("Semaphore releaseed\n");

                    semVal = semctl(semSet, 0, GETVAL, 0);
                    printf("Semaphore value is = %d\n", semVal);
                    break;
                }

                printf("semt to send2ATM4 ok \n");
                if (msgsnd(send2ATM4, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) { printf("send to atm 4 ok\n"); break; }

                 printf("recieve request (W,D,B)\n ");

                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (getFunds4), 0) == -1) { printf("recieve from atm request funds\n"); break; }

                if (msg.info.funds == -1){
                    msg.type = request;
                    printf("\natm 4 chequing\n");
                }else{
                    if(!strncmp(msg.info.pin,"5X5",3)){
                        msg.type = withdraw;
                        strncpy(msg.info.pin,"",3);
                          printf("\natm 4 withdrawal\n");
                    }else if(!strncmp(msg.info.pin,"2X2",3)){
                        msg.type = deposit;
                        strncpy(msg.info.pin,"",3);
                        printf("Deposit got\n");
                          printf("\natm 4 chequing\n");
                    }
                   
                }

                printf("send2ATM4 sending (W, D, B) to server\n");

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to db request funds\n"); break; }

                 printf("send2ATM4 recieving (W, D, B) to server\n");
                if (msgrcv(recieveFromDB, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0, 0) == -1) { printf("recieve from db request funds amount\n"); break; }

                printf("send2ATM4 recieving (W, D, B) to server\n");
                if (msgsnd(send2ATM4, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0) == -1) { printf("send to atm2 request funds message\n"); break; }

                printf("send2ATM4 recieving (W, D, B) to server\n");
                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (end4), 0) == -1) { printf("recieve from atm transaction end from atm\n"); break; }

                msg.type = endTrans;

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to db trnsaction end\n"); break; }


                 // RELEASE RESOURCE
                sem.sem_num =0;
                sem.sem_flg = SEM_UNDO;
                sem.sem_op = 1;
                semop(semSet,&sem,1);
                printf("Semaphore releaseed\n");

                break;


            case pin5:  
                 sem.sem_num = 0;
               sem.sem_flg = SEM_UNDO;
               sem.sem_op = -1;
               semop(semSet, &sem, 1);
               printf("Semaphore acquired by send2ATM5\n");

                msg.type = pinNumber;

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to server pin\n"); break; }

                printf("sent the check\n");
                if (msgrcv(recieveFromDB, (struct msgbuf *)&send, sizeof(msg.info), 0, 0) == -1) { printf("recieve from server ok\n"); break; }

                if (send.type == pinWrong || send.type == accountBlock  || send.type == accountWrong ){

                    if (msgsnd(send2ATM5, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) {printf("send to atm 5 ok\n"); break;}
					
                    sem.sem_num = 0;
                    sem.sem_flg = SEM_UNDO;
                    sem.sem_op = 1;
                    semop(semSet, &sem, 1);
                    printf("Semaphore releaseed by send2ATM5\n");

                    semVal = semctl(semSet, 0, GETVAL, 0);
                    printf("Semaphore value is = %d\n", semVal);
                    break;
                }
                printf("semt to send2ATM4 ok \n");
                if (msgsnd(send2ATM5, (struct msgbuf *)&send, sizeof(msg.info), 0) == -1) { printf("send to atm 4 ok\n"); break; }

                printf("recieve request (W,D,B)\n ");
                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (getFunds5), 0) == -1) { printf("recieve from atm request funds\n"); break; }

                if (msg.info.funds == -1){ msg.type = request;
                    printf("\natm 4 chequing\n");

                }else{

                    if(!strncmp(msg.info.pin,"5X5",3)){
                        msg.type = withdraw;
                        strncpy(msg.info.pin,"",3);
                          printf("\natm 4 withdrawal\n");

                    }else if(!strncmp(msg.info.pin,"2X2",3)){
                        msg.type = deposit;
                        strncpy(msg.info.pin,"",3);
                        printf("Deposit got\n");
                          printf("\natm 4 chequing\n");
                    }
                   
                }
                printf("send2ATM4 sending (W, D, B) to server\n");
                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to db request funds\n"); break; }
                 printf("send2ATM4 recieving (W, D, B) to server\n");

                if (msgrcv(recieveFromDB, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0, 0) == -1) { printf("recieve from db request funds amount\n"); break; }
                printf("send2ATM4 recieving (W, D, B) to server\n");

                if (msgsnd(send2ATM5, (struct msgbuf *)&atm_msg, sizeof(msg.info), 0) == -1) { printf("send to atm2 request funds message\n"); break; }
                printf("send2ATM4 recieving (W, D, B) to server\n");

                if (msgrcv( fromAllAtms, (struct msgbuf *)&msg, sizeof(msg.info), (end5), 0) == -1) { printf("recieve from atm transaction end from atm\n"); break; }

                msg.type = endTrans;

                if (msgsnd(send2db, (struct msgbuf *)&msg, sizeof(msg.info), 0) == -1) { printf("send to db trnsaction end\n"); break; }

                 // RELEASE RESOURCE
                sem.sem_num =0;
                sem.sem_flg = SEM_UNDO;
                sem.sem_op = 1;
                semop(semSet,&sem,1);
                printf("Semaphore releaseed\n");

                break;


            default:
				break;
        }
	 }

	
    // close msg ques
    if (msgctl(recieveFromDB, IPC_RMID, 0) == -1) { fprintf(stderr, " failed to clsoe recieveFromDB msg que\n"); exit(EXIT_FAILURE); }


    if (msgctl(send2db, IPC_RMID, 0) == -1) { fprintf(stderr, "failed to close send2db msg que\n"); exit(EXIT_FAILURE); }

    if (msgctl(send2ATM1, IPC_RMID, 0) == -1){ fprintf(stderr, " failed to clsoe send2ATM1 msg que\n"); exit(EXIT_FAILURE); }
 
    if (msgctl(send2ATM2, IPC_RMID, 0) == -1) { fprintf(stderr, " failed to clsoe send2ATM2 msg que\n"); exit(EXIT_FAILURE); }

    if (msgctl(send2ATM3, IPC_RMID, 0) == -1) { fprintf(stderr, " failed to clsoe send2ATM3 msg que\n"); exit(EXIT_FAILURE); }

     if (msgctl(send2ATM4, IPC_RMID, 0) == -1) { fprintf(stderr, " failed to clsoe send2ATM4 msg que\n"); exit(EXIT_FAILURE); }

     if (msgctl(send2ATM5, IPC_RMID, 0) == -1) { fprintf(stderr, " failed to clsoe send2ATM5 msg que\n"); exit(EXIT_FAILURE); }

    if (msgctl( fromAllAtms, IPC_RMID, 0) == -1) { fprintf(stderr, "failed to close fromAllAtms msg que\n"); exit(EXIT_FAILURE); }

    //detach shared memory 
    if (shmdt(shared_memory) == -1) { fprintf(stderr, "shmdt failed\n"); exit(EXIT_FAILURE); }

    // close shared memory 
    if (shmctl(shmid, IPC_RMID, 0) == -1) { fprintf(stderr, "shmctl(IPC_RMID) failed\n"); exit(EXIT_FAILURE); }
	return 0;
}
