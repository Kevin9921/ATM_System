#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#define __USE_GNU
#include <sys/msg.h>
#include <string.h>
#include <stdbool.h>
#include <sys/shm.h> // allows access to 
#include "header.h"


typedef struct info
{
	char accountNo[6];
	char pin[4];
	float funds;
} info_t;


/*
Initalizes the struct array pointed by ptr , by reading through the file that fPolonger points too, 
*/
void intializeProcesses(info_t *ptr)
{
	FILE *fPolonger = fopen("db.txt", "r"); // opens up notepad, reads inputted file
	int numberOfLines = 0;
	while (!feof(fPolonger))
	{ // if file polonger hasn't reached end keep running

		char singleLine[100]; //stores single line of the text

		fgets(singleLine, 100, fPolonger); //gets single

		// assigns the account number to the array of structs 
		strcpy(ptr[numberOfLines].accountNo, strtok(singleLine, ","));
		printf("Account Number: %s\n", ptr[numberOfLines].accountNo);

		// assigns the pin number to the pin member of the array of structs 
		strcpy(ptr[numberOfLines].pin, strtok(NULL, ","));
		printf("Pin:  %s\n", ptr[numberOfLines].pin);

		// assigns the funds number to the funds member of the array of structs 
		ptr[numberOfLines].funds = atoi(strtok(NULL, " "));
		printf("Avaliable Funds: %.2f\n", ptr[numberOfLines].funds);
		numberOfLines++;
	}
}


/*
if the account and pin are in the database it will return run this method which will do withdraw
deposit, Balance or give a different return type depending on the funds in the account
*/
int goodAccPin(info_t *data, int input, int output, my_msg_st send, my_msg_st receive, info_t temp, int lines)
{
	int msgLength = sizeof(my_msg_st) - sizeof(long);
	printf("\nPIN is in database\n");
	send.type = pinRight;
	printf("\nSEND OK MSG\n"); 
	printf("\ntype: %d\n", send.type); 
	if (msgsnd(output, (void *)&send, msgLength, 0) == -1) { fprintf(stderr, "\nOK msgsnd failed\n"); exit(EXIT_FAILURE); }


	printf("\nWAITED FOR TRANS MSG\n");
	if (msgrcv(input, (void *)&receive, msgLength, updateDB, MSG_EXCEPT) == -1) { fprintf(stderr, "\nmsgrcv failed with error: %d\n", errno); exit(EXIT_FAILURE); }
	printf("\nTRANS MSG Received\n");

	//if message type is withdraw
	if (receive.type == withdraw)
	{
		printf("\nGOT WITHDRAWAL AMOUNTM\n");

		//checks if there are enough funds
		if (temp.funds >= receive.info.funds)
		{
			printf("\nEnough funds for withdraw! \n ");
			
			//subtracts amount be account stored in temp
			temp.funds = temp.funds - receive.info.funds;
			
			//writes them all back into DB
			for (int i = 0;i<lines;i++){
				FILE *fPolonger = fopen("db.txt", "w");
				for (int i = 0; i < lines; i++)
				{
					if (!strcmp(data[i].accountNo, temp.accountNo))
					{
						data[i].funds = temp.funds;
					}
					if (i == lines-1) {
						fprintf(fPolonger, "%5s,%3s,%.2f", &data[i].accountNo, &data[i].pin, data[i].funds);
					}else{
						fprintf(fPolonger, "%5s,%3s,%.2f\n", &data[i].accountNo, &data[i].pin, data[i].funds);
					}
				}
				fclose(fPolonger);
			}
			send.type = EnoughFunds;
			send.info.funds =temp.funds; 
				
			//sends message back
			printf("\n %d\n",send.info.funds);

			if (msgsnd(output, (void *)&send, msgLength, 0) == -1) { fprintf(stderr, "\n actionS msgsnd failed\n"); exit(EXIT_FAILURE); }

			printf("\n DB: FUNDS_OK message sent \n");

			if (msgrcv(input,(void *)&receive,msgLength,updateDB,MSG_EXCEPT) == -1) { fprintf(stderr, "\nmsgrcv failed with error: %d\n", errno); exit(EXIT_FAILURE);}

			if (receive.type == endTrans) { printf("\nEND TRANS msg received\n"); return 0; }
		}else{
			//send not enough funds back
				printf("\nDB: Not Enough funds for withdraw! \n ");

				send.type = notEnoughFunds;
				if (msgsnd(output, (void *)&send, msgLength, 0) == -1) { fprintf(stderr, "\n actionS msgsnd failed\n"); exit(EXIT_FAILURE); } 

				printf("\n DB: NSF message sent \n");
				
				if (msgrcv(input, (void *)&receive, msgLength, updateDB, MSG_EXCEPT)== -1){fprintf(stderr, "\nmsgrcv failed with error: %d\n", errno); exit(EXIT_FAILURE);}

				//end transaction
				if (receive.type == endTrans) {	printf("\nEND TRANS msg received\n"); return 1; }
		}
	//if type is balance	
	}else if (receive.type == request){
		printf("\nBALANCE msg received \n");
		send.type = currentfund;
		//copy funds into return message
		send.info.funds = temp.funds; 

		printf("\nI recieved msg type: %d\n",send.type);
		printf("\n %f\n",send.info.funds);
		//sends funds back
		if (msgsnd(output, (void *)&send, msgLength, 0) == -1) { fprintf(stderr, "\n actionS msgsnd failed\n"); exit(EXIT_FAILURE);	}

		if (msgrcv(input, (void *)&receive, msgLength, updateDB, MSG_EXCEPT) == -1)	{ fprintf(stderr, "\nmsgrcv failed with error: %d\n", errno); exit(EXIT_FAILURE);}

		//end transaction
		if (receive.type == endTrans) { printf("\nEND TRANS msg received\n"); return 1; }
	
	//if type is deposit
	}else if(receive.type == deposit){
		printf("\nGOT Deposit AMOUNTM\n");
		//add recieve funds to temp

		temp.funds = temp.funds + receive.info.funds;	

		//write it into the database
		for (int i = 0;i<lines;i++){
			FILE *fPolonger = fopen("db.txt", "w");
			for (int i = 0; i < lines; i++)
			{
				if (!strcmp(data[i].accountNo, temp.accountNo))
				{
					data[i].funds = temp.funds;
				}
				if (i == lines-1) {
					fprintf(fPolonger, "%5s,%3s,%.2f", &data[i].accountNo, &data[i].pin, data[i].funds);
				}else{
					fprintf(fPolonger, "%5s,%3s,%.2f\n", &data[i].accountNo, &data[i].pin, data[i].funds);
					
				}
			}
			fclose(fPolonger);
		}
		send.type = EnoughFunds;
		send.info.funds =temp.funds; 

		//send the funds back
		printf("\n %d\n",send.info.funds);
		if (msgsnd(output, (void *)&send, msgLength, 0) == -1) { fprintf(stderr, "\n actionS msgsnd failed\n"); exit(EXIT_FAILURE); }

		printf("\n DB: FUNDS_OK message sent \n");

		if (msgrcv(input, (void *)&receive, msgLength, updateDB, MSG_EXCEPT) == -1) { fprintf(stderr, "\nmsgrcv failed with error: %d\n", errno);exit(EXIT_FAILURE);}

		if (receive.type == endTrans) { printf("\nEND TRANS msg received\n"); return 0; }
	}
		
}


// counts and returns number of liens in a file 
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

int main()
{
	// creating shared memory stuff 
   
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

	int msgLength = sizeof(my_msg_st) - sizeof(long);


	int input = msgget(toDB_KEY, 0666 | IPC_CREAT);
	if (input == -1) { 	fprintf(stderr, "\nmsgget failed with error: %d\n", errno); exit(EXIT_FAILURE); }

	int output = msgget(Decide_KEY, 0666 | IPC_CREAT);
	if (output == -1){ fprintf(stderr, "\nmsgget failed with error: %d\n", errno); exit(EXIT_FAILURE); }


	my_msg_st send, recieve; // creates structs for the msg Q's 

	// if this is false then close msg  Qs and shared memory 
	while (shared_stuff->close != 'x') 
	{	
		info_t temp;
		int lines = lineCounterInFile("db.txt");
		info_t data[lines];
		intializeProcesses(data);
		
		if(shared_stuff->close == 'x'){break;}

		if (msgrcv(input, (void *)&recieve, msgLength, 0, 0) == -1) { fprintf(stderr, "\nmsgrcv failed with error: %d\n", errno);exit(EXIT_FAILURE);}

		if(recieve.type == CloseEvery){ printf("\nclosing the server\n"); break; }


		switch (recieve.type)
		{
			//if the type is update then add accounts to the Db or edit the accoutns
			case updateDB:
			;
				bool accountInDB = false;
				FILE *fPolonger = fopen("db.txt", "r+"); 
				intializeProcesses(data);
				//edits accounts already in db.txt
				for (int i = 0; i< lines ;i++)
				{
					if(!strcmp(data[i].accountNo,recieve.info.account)){
						data[i].funds = recieve.info.funds;
						accountInDB = true;
					}
					if(i == lines-1){
						fprintf(fPolonger,"%5s,%3s,%.2f",&data[i].accountNo,&data[i].pin,data[i].funds);
					}else{
						fprintf(fPolonger,"%5s,%3s,%.2f\n",&data[i].accountNo,&data[i].pin,data[i].funds);
					}
				}
				fclose(fPolonger);
				//adds new account to db,txt
				if (accountInDB == false){
					int temp = atoi(recieve.info.pin)-1;
					FILE *fPolonger = fopen("db.txt", "a"); 
					fprintf(fPolonger,"\n%5s,%03d,%.2f",&recieve.info.account,temp,recieve.info.funds);
					fclose(fPolonger);
				}
				break;
			//if the type is pinNumber an atm is making a request
			case pinNumber:
				;
				while(true){
				bool pinGood = true;
				bool goodAccount = false;
				intializeProcesses(data);
				for (int i = 0; i < lines; i++)
				{	
					//checks our struct array that has a copy of the DB for account
					if (strncmp(data[i].accountNo, recieve.info.account,5) == 0)
					{
						printf("\n Account number is in DB \n");
						goodAccount = true;
						temp = data[i];
						//checks our struct array that has a copy of the DB for pin
						if ((atoi(data[i].pin)) == (atoi(recieve.info.pin)-1))
						{
							printf("\n PIN number is in DB \n");
							printf("\n db %d\n", atoi(data[i].pin));
							printf("\n recieve %d\n", atoi(recieve.info.pin)-1);
							goodAccPin(data, input,output, send, recieve,temp,lines);       //if good account
							pinGood = false;
							break;
						}
					}
				}

				//if account is not in db send type and break
				if (!goodAccount){
					send.type = accountWrong;
					if (msgsnd(output, (void *)&send, msgLength, 0) == -1) { fprintf(stderr, "\n account wrong \n"); exit(EXIT_FAILURE); }
					break;
				}
				int wrongPinCount = 1;
				
				//if the pin is wrong
				while (pinGood)
				{
					send.type = pinWrong;
						// want to get pin message since account matched
						if (msgsnd(output, (void *)&send, msgLength, 0) == -1) 	{ fprintf(stderr, "\n PIN_WRONG msgsnd failed\n"); exit(EXIT_FAILURE); }
						printf("\n SEND type %d\n",send.type);
				
						if (msgrcv(input,(void*)&recieve,msgLength,updateDB,MSG_EXCEPT)==-1){fprintf(stderr,"\nmsgrcv failed with error: %d\n", errno);exit(EXIT_FAILURE);}
						
						printf("\n recieve type %d\n",recieve.type);

						//top loop stored good account now we just match it with the one we recive
						// its a second pin good check
						if (strncmp(temp.accountNo, recieve.info.account, 5) == 0)
						{
							printf("\n Account number is in DB \n");
							if ((atoi(temp.pin)) == (atoi(recieve.info.pin)-1))
							{
								printf("pin in Db ");
								goodAccPin(data, input,output, send, recieve,temp,lines); //if pin is good do this 
								pinGood = false;
							}else{
								wrongPinCount++;
								printf("\n Ping wrong count %d\n",wrongPinCount);
							}
						//if its a different accoutn pin is reset
						}else{
							wrongPinCount = 0;
						}
						//if the wrong pin account is 3 means they entered the same account wrong pin 3 times so block
						if (wrongPinCount == 3)
						{
							intializeProcesses(data);
							//loops through array of structs finds accoutn blocks it and writes all structs back to db
							FILE *fPolonger = fopen("db.txt", "w");
							for (int i = 0; i < lines; i++)
							{
								if (!strcmp(data[i].accountNo, recieve.info.account))
								{
									data[i].accountNo[0] = 'X'; 
								}
								if (i == lines - 1)
								{
									fprintf(fPolonger, "%5s,%3s,%.2f", &data[i].accountNo, &data[i].pin, data[i].funds);
								}else{
									fprintf(fPolonger, "%5s,%3s,%.2f\n", &data[i].accountNo, &data[i].pin, data[i].funds);
								}
							}
							fclose(fPolonger);
							send.type = accountBlock;
							
							//send account block message
							if (msgsnd(output, (void *)&send, msgLength, 0) == -1) { fprintf(stderr, "\n ACCOUNT BLOCKED msgsnd failed\n"); exit(EXIT_FAILURE);}
							pinGood = false;
						}
				
				}
				break;
			}
			break;
	}

	}
	// closing msg Qs 
	if (msgctl(input, IPC_RMID, 0) == -1)  { fprintf(stderr, " failed to clsoe decide2ATM1 msg que\n"); exit(EXIT_FAILURE); }

    if (msgctl(output, IPC_RMID, 0) == -1) { fprintf(stderr, "failed to close ATM2decide msg que\n"); exit(EXIT_FAILURE); }

    // detach shared memory 
    if (shmdt(shared_memory) == -1) { fprintf(stderr, "shmdt failed\n"); exit(EXIT_FAILURE); }
    // close shared memory 
    if (shmctl(shmid, IPC_RMID, 0) == -1) { fprintf(stderr, "shmctl(IPC_RMID) failed\n"); exit(EXIT_FAILURE); }
	
	return 0;
}
