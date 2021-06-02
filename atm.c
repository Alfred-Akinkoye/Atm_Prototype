/* all #includes here*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define MSGSZ 128
/*Message Struct*/
typedef struct msgbuf {
	long mtype;
	char pin[10];
  char acc[10];
  char topic[10];
  char money[10];
}message_buf;

/*Method Declaration*/
bool checkPin(int,int);
void messageSend(int,int,char[],float);
void messageReceive();
int middle();
void withdraw();
void balance();
/*Global Variables*/
message_buf rbuf;
/*Main Function*/
int main() {
  // vars here
  int acc, pin, options;
	bool checker;
  int counter =0;

	while (1) {    // loop forever
		// get account number, get pin (from the user)
    if(counter==0){
      printf("Enter Your 5 digit Account number\n");
		  scanf("%d",&acc);
		  printf("Enter Your 3 digit PIN number\n");
		  scanf("%d",&pin);
      // Check your pin: to do that, send a message to the db_server and get result
      checker = checkPin(pin, acc);
      counter++;
    }
		if(!checker){
			if(strcmp(rbuf.pin,"3")==0){
				printf("account is blocked\n");
				// code to sleep
        sleep(10);
        counter = 0;
			}else{
				printf("Does Not Work, Try Again\n");
			}
		}else{
			options = middle();
			if(options == 3){
				//Exit
        counter=0;
			}else if(options == 2){
				//code for WITHDRAW
				withdraw();
			}else if(options == 1){
				//code for balance;
				balance();
			}
		}
		//break;
	}
}
/* To check balance */
void balance(){
  messageSend(0,0,"BALANCE",0);
  messageReceive();
	printf("/---------------------------/\n        BALANCE: %s \n/---------------------------/\n",rbuf.money);
}
/* To Exercise the Withdraw option */
void withdraw(){
	float amount;
	printf("How much would you like to withdraw?: \n");
	scanf("%f",&amount);
	messageSend(0,0,"WITHDRAW",amount);
	messageReceive();
	if(strcmp(rbuf.topic,"NSF")==0){
		printf("Insufficient Funds for such withdrawal\n");
	}else if (strcmp(rbuf.topic,"FUNDS_OK")==0){
		printf("withdrawal approved, new balance below: \n");
		printf("$%s\n",rbuf.money);
	}
}
/*checking if the options chosen matches the options presented*/
int middle(){
	int temp;
	printf("Select one of the three options: \n");
	printf("Enter 1 for BALANCE: \n\n");
	printf("Enter 2 for WITHDRAW: \n\n");
	printf("Enter 3 for EXIT: \n");
	scanf("%d",&temp);
	if(!(temp>0 && temp<4)){
		printf("You entered an inproper input.... Try Again!\n");
		return middle();
	}else{return temp;}
}
/*checking pin*/
bool checkPin(int pin, int acc){
	messageSend(pin, acc,"PIN",0);
	messageReceive();
	printf("Recieved: %s\n",rbuf.topic);
	if(strcmp(rbuf.topic,"PIN_WRONG") == 0){
		return false;
	}else if (strcmp(rbuf.topic,"OK") == 0){
		return true;
	}else{
    printf("There Was an error comparing\n");
  }
}
/*Message Creation*/
void messageSend(int pin, int acc, char topic[], float money){
	int msqid;
	int msgflg = IPC_CREAT | 0666;
	key_t key;
	message_buf sbuf;
	size_t buf_length;

  char pinText[10];
  char accText[10];
  char monText[10];
  sprintf(pinText,"%d",pin);
  sprintf(accText,"%d",acc);
  sprintf(monText,"%f",money);
	key = 2234;

	(void)fprintf(stderr, "\nmsgget: Calling msgget(%#1x,\%#o)\n", key, msgflg);

	if ((msqid = msgget(key, msgflg)) < 0) {
		perror("msgget");
		exit(1);
	}
	else{
		(void)fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);
	}
	// We'll send message type 1
	sbuf.mtype = 1;
	(void) fprintf(stderr, "msggeet: msgget succeeded: msqid = %d\n", msqid);
	(void) strcpy(sbuf.pin, pinText);
  (void) strcpy(sbuf.topic, topic);
  (void) strcpy(sbuf.acc, accText);
  (void) strcpy(sbuf.money, monText);
	(void) fprintf(stderr, "msgget: msgget succeeded: msqid = %d\n", msqid);

	// Send a message.
	if((msgsnd(msqid, &sbuf, MSGSZ, IPC_NOWAIT)) < 0){
		printf("%d, %d, %s, %d\n", msqid, sbuf.mtype, sbuf.topic, buf_length);
		perror("msgsnd");
		exit(1);
	}
	else{
		printf("Message: \"%s\" Sent\n", sbuf.topic);
	}
}

void messageReceive(){
	int msqid;
	key_t key;
	key = 2235;

	if ((msqid = msgget(key, 0666)) < 0) {
		perror("msgget");
		exit(1);
	}

	// Receive an answer of message type 1.
  int temp = -1;
	while (temp==-1) {
		temp = msgrcv(msqid, &rbuf, MSGSZ, 1, 0);
	}

  // if (msgctl(msqid, IPC_RMID, NULL) == -1) {
	// 	fprintf(stderr, "Message queue could not be deleted.\n");
	// }else{
	//   printf("Message queue was deleted.\n");
  // }
}
