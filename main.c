/* all #includes here*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
// DB can be a file or be in memory for this exercise

#define MSGSZ 128
/*Message Struct*/
typedef struct msgbuf {
	long mtype;
	char pin[10];
  char acc[10];
  char topic[10];
  char money[10];
}message_buf;

/*Defining Functions*/
void messageReceive();
void messageSend(int,int,char[],float,long);
int difference();
bool checkPin();
float checkBalance();
bool checkWithdraw();
void decrypt();
void messageReceiveEditor();

/*Global Variables*/
char store1[50] = "";
message_buf rbuf;
float temporary_balance;
char store2[50];
char store3[50];
int LineNumber = 0;

int main() {
	// vars here
  bool dbInvolved = false;
  int temp =2;
	int checker;
	int trys = 1;
	while (1) {
		messageReceive();	//receive message
		checker = difference();	//differentiate the commands
		if(checker==0){//means it's we have to comfirm pin and account
			decrypt();	// decrypt received message for the pin
			if(checkPin()){
				messageSend(0,0,"OK",0,1);
			}else{
				if(trys==3){
          trys = 0;
        }else{
          trys++;
        }
				messageSend(trys,0,"PIN_WRONG",0,1);
			}
		}else if(checker==1){
			if(checkWithdraw()){
				messageSend(0,0,"FUNDS_OK",temporary_balance,1);
				//send msg to db editor to update db
        // send temp balance in place of money and line number in pin
        messageSend(LineNumber,0,"PUT",temporary_balance,5);
        dbInvolved = true;
			}else{messageSend(0,0,"NSF",0,1);}
		}else if(checker==2){
      messageSend(0,0," ",checkBalance(),1);
		}
		//receive updated database
    if(dbInvolved){
      messageReceiveEditor();
    }
	}

	//msgctl(msg, DELETE PARAMETERS...); // Delete Message Queue
  return 0;
}

/* Check BALANCE */
float checkBalance(){
  char copy[80];
  strcpy(copy,store1);
  char * token = strtok(store1, ",");
  token = strtok(NULL, ",");
  token = strtok(NULL, ",");
  float money = atof(token);
  strcpy(store1,copy);//to ensure the string remains unchanged
  return money;
}

/* Check WITHDRAW */
bool checkWithdraw(){
	float balance = checkBalance();
	temporary_balance = balance;
	float money = atof(rbuf.money);
	if(balance>money){
		temporary_balance -= money;
		return true;
	}else{
		return false;
	}
}

/* PIN instructions */
bool checkPin(){
	FILE *fp;
	bool exists = false;
	char temp[50];
  fp = fopen("db.txt", "r");
  while(!feof(fp)){
    LineNumber++;
    fgets(temp, 50, (FILE*)fp);
		if(strstr(temp, rbuf.acc) != NULL){
			exists = true;
			break;
		}
  }
	fclose(fp);
	if(exists){
		if(strstr(temp, rbuf.pin) == NULL){// if pin does not exist in here, ie returns NULL
			exists = false;	// set exists to false even if acc is exists
      LineNumber = 0;
		}
	}
  strcpy(store1,temp);
  return exists;
}

/* Differentiate message to send */
int difference(){
  printf("The Request is: %s\n",rbuf.topic);
  if(strcmp(rbuf.topic,"PIN") == 0){
    return 0;
	}else if (strcmp(rbuf.topic,"WITHDRAW") == 0){
    return 1;
	}else if (strcmp(rbuf.topic,"BALANCE") == 0){
    return 2;
  }else{
		return -1;
  }
}
/*Message Creation*/
void messageSend(int pin, int acc, char topic[], float money, long type){
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
	key = 2235;

	(void)fprintf(stderr, "\nmsgget: Calling msgget(%#1x,\%#o)\n", key, msgflg);

	if ((msqid = msgget(key, msgflg)) < 0) {
		perror("msgget");
		exit(1);
	}
	else{
		(void)fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);
	}
	// We'll send message type
	sbuf.mtype = type;
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
		printf("Message: \"%s\" Sent\n\n", sbuf.topic);
	}
}

void messageReceive(){
	int msqid;
	key_t key;
	key = 2234;

	if ((msqid = msgget(key, 0666)) < 0) {
		perror("msgget");
		exit(1);
	}

	// Receive an answer of message type 1.
  printf("Awaiting Messages...\n");
  int temp = -1;
	while (temp==-1) {
		temp = msgrcv(msqid, &rbuf, MSGSZ, 1, 0);
	}
}
/* for receiving only from dbEditor */
void messageReceiveEditor(){
	int msqid;
	key_t key;
	key = 2233;
	if ((msqid = msgget(key, 0666)) < 0) {
		perror("msgget");
		exit(1);
	}
  printf("Awaiting Messages...\n");
  int temp = -1;
	while (temp==-1) {
		temp = msgrcv(msqid, &rbuf, MSGSZ, 5, 0);
	}
  strcpy(store1,rbuf.acc);
  strcat(store1,",");
  strcat(store1,rbuf.pin);
  strcat(store1,",");
  strcat(store1,rbuf.money);
}

/*Deccrypter*/
void decrypt(){
	int x = atoi(rbuf.pin);
	x--;
	sprintf(rbuf.pin,"%d",x);
}
