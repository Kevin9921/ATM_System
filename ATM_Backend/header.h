
#define allAtms_KEY 12345
#define ATM1_KEY 12346
#define ATM2_KEY 12347
#define ATM3_KEY 12350
#define ATM4_KEY 12351
#define ATM5_KEY 12352
#define toDB_KEY 12348
#define Decide_KEY 12349

//for server
#define pinNumber 1
#define endTrans 2
#define withdraw 3
#define request 4
#define deposit 29

//for atm 1
#define pin1 5
#define end1 6
#define getFunds1 7

//for atm 2
#define pin2 8
#define end2 9
#define getFunds2 10

//for atm 3
#define pin3 20
#define end3 21
#define getFunds3 22

//for atm 4
#define pin4 23
#define end4 24
#define getFunds4 25

//for atm 5
#define pin5 26
#define end5 27
#define getFunds5 28

#define accountWrong 11
#define accountBlock 12
#define currentfund 13
#define notEnoughFunds 14
#define pinRight 15
#define pinWrong 16
#define finished 17
#define updateDB 18
#define EnoughFunds 19
#define  CloseEvery 29
typedef struct {
	long int type;
	struct {
		char account[6];
		char pin[4];
		float funds;
	} info;
} my_msg_st;

struct shared_use_st{
	char close;
};