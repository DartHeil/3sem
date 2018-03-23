#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <unistd.h>
#include "errhand.h"

#define PATHNAME __FILE__

#define ERR_KEY "! Can't generate key\n"
#define ERR_MSGQID "! Can't get msgqid\n"
#define ERR_CANTFORK "! Can't fork\n"
#define ERR_SEND "! Can't send\n"
#define ERR_RCV "! Can`t recieve\n"

typedef struct msg
    {
    long mtype;
    }msg;	

int main(int argc, char* argv[])
    {
    msg message;
    msg recieveMsg;
    key_t key;

    long long int par;
    int msgqid, i, retfork;

    int errInp = iclread(argc, argv, &par);
    if(errInp != 0)
        {
        printf("! Bad input\n");
        exit(-1);
        }
    int n = (int) par;

    if(n <= 0)
        {
        printf("! Bad input(Argument less or equal to zero)\n");
        exit(-1);        
        }

    if((key = ftok(PATHNAME, 0)) < 0)
       {
       perror(ERR_KEY"\n"); 
       exit(-1);
       }
       
    if((msgqid = msgget(key, 0666 | IPC_CREAT)) < 0)
        {
        perror(ERR_MSGQID"\n"); 
        exit(-1);
        }

    for(i = 1; i < n + 1; i++)
        {
        retfork = fork();
        if(retfork < 0)
            {
            n = i - 1;
            retfork = 1;
            break;
            }
        if(!retfork)
            {
            break;
            }
        }

    if(retfork > 0)/*parent*/
        {
        message.mtype = 1;
        if((msgsnd(msgqid, (msg *) &message, 0, 0)) < 0)
            {
            perror(ERR_SEND"\n"); 
            exit(-1);
            } 

        if((msgrcv(msgqid, (msg *) &recieveMsg, 0,n + 1, 0)) < 0)
            {
            perror(ERR_RCV"\n"); 
            exit(-1);
            } 
        }

    if(retfork == 0)/*child*/
        {
        if((msgrcv(msgqid, (msg *) &recieveMsg, 0, i, 0)) < 0)
            {
            perror(ERR_RCV"\n"); 
            exit(-1);
            }      

        printf("%d \n", i);
        fflush(stdout);
        
        message.mtype = i + 1;
        
        if((msgsnd(msgqid, (msg *) &message, 0, 0)) < 0)
            {
            perror(ERR_SEND"\n"); 
            exit(-1);
            }       
 
        return 0;
        }

    msgctl(msgqid, IPC_RMID, NULL);
    return 0;
    }

#undef PATHNAME

#undef ERR_KEY
#undef ERR_MSGQID
#undef ERR_SEND
#undef ERR_RCV
