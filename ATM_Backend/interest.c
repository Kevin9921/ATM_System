
#include <sys/shm.h> // allows access to 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#define __USE_GNU
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include "header.h"

//used to calculate interest of the accounts in db
void interestCalc(int lines){
    char n_account[6];
	char n_pin[4];
	float n_balance;
	int inFile = 0;
	FILE *fh, *ftmp;

	//uses a temp to copy db into
	fh=fopen("db.txt","r");
    ftmp =fopen("tmp.txt","a");
   
   //reads through db and gets the files
	while (!feof(fh)) {
		fscanf(fh,"%5s,%3s,%f ",&n_account, &n_pin, &n_balance);
		//if balance is greater than zero add 0.01% interest
		if (&n_balance > 0) {
            n_balance = n_balance * 0.01 + n_balance; 
        }
		//if balance is less than zero decrement 0.02% interest
        if(&n_balance <= 0){
            n_balance =   n_balance - n_balance * 0.02; 
        }
		if (inFile==lines-1){
			fprintf(ftmp,"%5s,%3s,%.2f",&n_account, &n_pin, n_balance);
		}else{
        	fprintf(ftmp,"%5s,%3s,%.2f\n",&n_account, &n_pin, n_balance);
		}
		inFile++;
	}
    fclose(fh); fclose(ftmp);
	//switches temp name to db and deletes original db
    int a = remove("db.txt"); if (a !=0){ printf("\n Failed to delete db.txt\n"); }
    int b = rename("tmp.txt","db.txt"); if (b !=0){ printf("\n Failed to delete tmp.txt\n"); } 
}

//counts number of line sin file	
int lineCounterInFile(char *filename){
    int noLines = 0;
	FILE *fpr;

	/* Open file for reading and writing */
	fpr = fopen(filename, "r");
	while(!feof(fpr)){
    		char singleLine[100]; 
    		fgets(singleLine, 100, fpr);
    		noLines++;
  	}
    return noLines;
}


int main() {
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
    //printf("Memory attached at %X\n", (int)shared_memory);
    shared_stuff = (struct shared_use_st *)shared_memory;

	//counts lines in db
	int lines = lineCounterInFile("db.txt");
	
	sleep(60); 
	//calls interest function and then waits 60 seconds
    while(shared_stuff->close != 'x'){interestCalc(lines); sleep(60);  }

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
