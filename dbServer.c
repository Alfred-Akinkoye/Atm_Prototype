#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <math.h>
#define MAX 512

struct mesg_buffer {
    long mesg_type;
    char accountNumber[10];
    char pin[10];
    char funds[3];
};

struct response {
    long mesg_type;
    char text[50];
};

void encrypt(char pin[]);

int main()
{
   FILE *fp;
   int msg_q;
   struct mesg_buffer m;
   struct response r;
   msg_q = msgget((key_t) 1234, 0666 | IPC_CREAT);
   while(1){
      printf("flag\n");
      while(1){
         msgrcv(msg_q, &m, MAX, 0, 0);
         if(m.mesg_type == 1){
            char customerInfo[50];

            m.accountNumber[strcspn(m.accountNumber, "\n")] = 0;
            m.pin[strcspn(m.pin, "\n")] = 0;
            m.funds[strcspn(m.funds, "\n")] = 0;

            fp = fopen("db.txt", "a");
            encrypt(m.pin);
            strcpy(customerInfo, m.accountNumber);
            strcat(customerInfo, ",");
            strcat(customerInfo, m.pin);
            strcat(customerInfo, ",");
            strcat(customerInfo, m.funds);
            strcat(customerInfo, "\n");
            fputs(customerInfo, fp);
            fclose(fp);
            printf("customer info: %s\n", customerInfo);
         }
         if(m.mesg_type == 2){

         }

      }
      //break;
    }
    msgctl(msg_q, IPC_RMID, 0);
    return 0;
}

void encrypt(char pin[])
{
    int x = atoi(pin);
    x++;
    sprintf(pin, "%03d", x);
}
