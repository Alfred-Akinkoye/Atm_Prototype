/* all #includes here*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MSGSZ 128
/*Message Struct*/
typedef struct msgbuf {
	long mtype;
	char pin[10];
  char acc[10];
  char topic[10];
  char money[10];
}message_buf;

/* innit */
void messageSend(int, int, char[], float);
void messageReceive();
int decide();
void putInfo();
void storeDeletedLine();
void splitter();

/* Global Variable */
message_buf rbuf;
char store1[50] = "";
char store2[50] = "";

int main(){
   int decider;
   int counter = 0;
   while(1){
      if(counter==0){
        messageReceive();
        counter++;
      }
      decider = decide();
      if(decider == 0){
        storeDeletedLine();
        putInfo();
      }else if(decider == 1){
        //when splitter() was called in putInfo(), rbuff is also updated
        messageSend(atoi(rbuf.pin), atoi(rbuf.acc), rbuf.topic, atof(rbuf.money));
        counter=0;
      }
   }
   return 0;
}

void storeDeletedLine(){
  int line = atoi(rbuf.pin);
 	FILE *fp;
	bool exists = false;
	char temp[50];
  fp = fopen("db.txt", "r");
  fgets(temp, 50, (FILE*)fp);
  if(line==1){
    strcpy(store1,temp);
  }else{
    line--;
    for(;line>0;line--){
      fgets(temp, 50, (FILE*)fp);
    }
    strcpy(store1,temp);
  }
}

/* to decide if I'm putting an item in or taking out to send */
int decide(){
  if(strcmp(rbuf.topic,"PUT") == 0){
    return 0;
	}else if (strcmp(rbuf.topic,"UPDATE_DB") == 0){
		return 1;
	}else{
    return -1;
  }
}

void splitter(){
  char copy[50];
  char * token = strtok(store1, ",");
  strcpy(rbuf.acc,token);
  strcpy(copy,token);
  token = strtok(NULL, ",");
  strcpy(rbuf.pin,token);
  strcat(copy,",");
  strcat(copy,token);
  //money is already in rbuf
  strcat(copy,",");
  strcat(copy,rbuf.money);
  strcpy(store2,copy);
}

void putInfo(){
    FILE *fptr1, *fptr2;
    char file1[] ="db.txt";
    char file2[] ="file2.txt";
    char curr;
    int del = atoi(rbuf.pin);
    int line_number = 0;
    fptr1 = fopen(file1,"r");
    fptr2 = fopen(file2, "w");
    curr = getc(fptr1);
    if(curr!=EOF) {line_number =1;}
    while(1){
      if(del != line_number)
        putc(curr, fptr2);
        curr = getc(fptr1);
        if(curr =='\n') line_number++;
        if(curr == EOF) break;
    }
    fclose(fptr1);
    fclose(fptr2);
    remove(file1);
    //rename the file replica.c to original name
    rename(file2, file1);
    //reopen file 1 to append
    fptr1 = fopen(file1,"a");
    splitter();
    fprintf(fptr1, "%s", store2);
    fclose(fptr1);
    strcpy(rbuf.topic,"UPDATE_DB");
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
	key = 2233;

	(void)fprintf(stderr, "\nmsgget: Calling msgget(%#1x,\%#o)\n", key, msgflg);

	if ((msqid = msgget(key, msgflg)) < 0) {
		perror("msgget");
		exit(1);
	}
	else{
		(void)fprintf(stderr, "msgget: msgget succeeded: msgqid = %d\n", msqid);
	}
	// We'll send message type 1
	sbuf.mtype = 5;
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

/* receive message */
void messageReceive(){
	int msqid;
	key_t key;
	key = 2235;

	if ((msqid = msgget(key, 0666)) < 0) {
		perror("msgget");
		exit(1);
	}

	// Receive an answer of message type 5.
  int temp = -1;
	while (temp==-1) {
		temp = msgrcv(msqid, &rbuf, MSGSZ, 5, 0);
	}
}
