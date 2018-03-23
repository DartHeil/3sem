#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include <sys/types.h>
#include <sys/ipc.h>

#include <sys/shm.h>

#include <sys/sem.h>

#include "cmdline.h"

#define PATHNAME "task3.c"
#define BUFSIZE 13
#define QSEM 8

#define KILLME \
    { \
    write(1, "\nKILL ME\n", 8); \
    sleep(2); \
    exit(-1); \
    }

#define TRY(code, errmessage) \
    do \
        { \
        if((code) < 0) \
            { \
            perror(errmessage); \
            exit(EXIT_FAILURE); \
            } \
        } \
    while(0)


enum SEMAPHORES
    {
    SEM_WR_CONTROL,
    SEM_RD_CONTROL,
    SEM_EMPTY,
    SEM_FULL,
    SEM_WR_ALIVE,
    SEM_RD_ALIVE,
    SEM_WR_EXISTED,
    SEM_RD_EXISTED,
    };

enum MODE
    {
    WRITER = 1,
    READER = 2,
    };

void writer_prologue(int semid)
    {
        struct sembuf opbuf[6];
        
        opbuf[0].sem_op = 0;
        opbuf[0].sem_flg = IPC_NOWAIT;
        opbuf[0].sem_num = SEM_WR_CONTROL;

        opbuf[1].sem_op = 1;
        opbuf[1].sem_flg = SEM_UNDO;
        opbuf[1].sem_num = SEM_WR_CONTROL;
        
        opbuf[2].sem_op = 1;
        opbuf[2].sem_flg = SEM_UNDO;
        opbuf[2].sem_num = SEM_EMPTY;
        
        opbuf[3].sem_op = 1;
        opbuf[3].sem_flg = SEM_UNDO;
        opbuf[3].sem_num = SEM_FULL;

        opbuf[4].sem_op = -1;
        opbuf[4].sem_flg = 0;
        opbuf[4].sem_num = SEM_FULL;

        opbuf[5].sem_op = 1;
        opbuf[5].sem_flg = SEM_UNDO;
        opbuf[5].sem_num = SEM_WR_ALIVE;
        
        errno = 0;
        int err = semop(semid, opbuf, 6);
        //KILLME;
        if(errno == EAGAIN)
            {
            perror("\n[error]:Connection already exist\n");
            exit(-1);
            }
        else if(err == -1)
            {
            perror("\n[error]:Can't do semop\n");
            exit(-1);
            }
    }

void reader_prologue(int semid)
    {
    struct sembuf opbuf[7];
    opbuf[0].sem_op = 0;
    opbuf[0].sem_flg = IPC_NOWAIT;
    opbuf[0].sem_num = SEM_RD_CONTROL;
    
    opbuf[1].sem_op = 1;
    opbuf[1].sem_flg = SEM_UNDO;
    opbuf[1].sem_num = SEM_RD_CONTROL;

    opbuf[2].sem_op = 1;
    opbuf[2].sem_flg = SEM_UNDO;
    opbuf[2].sem_num = SEM_FULL;

    opbuf[3].sem_op = -1;
    opbuf[3].sem_flg = 0;
    opbuf[3].sem_num = SEM_FULL;
    
    opbuf[4].sem_op = 1;
    opbuf[4].sem_flg = SEM_UNDO;
    opbuf[4].sem_num = SEM_EMPTY;

    opbuf[5].sem_op = -1;
    opbuf[5].sem_flg = 0;
    opbuf[5].sem_num = SEM_EMPTY;

    opbuf[6].sem_op = 1;
    opbuf[6].sem_flg = SEM_UNDO;
    opbuf[6].sem_num = SEM_RD_ALIVE;
    errno = 0;
    int err = semop(semid, opbuf, 7);
    //KILLME;
    if(errno == EAGAIN)
        {
        perror("\n[error]:Connection already exist\n");
        exit(-1);
        }
    else if (err == -1)
        {
        perror("\b[error]:Can't do semop\n");
        exit(-1);
        }
    }


void check_is_reader_alive(int semid)
    {
    struct sembuf opbuf[2];
    opbuf[0].sem_op = -1;
    opbuf[1].sem_op = 1;
    opbuf[0].sem_flg = IPC_NOWAIT;
    opbuf[1].sem_flg = 0;
    opbuf[0].sem_num = opbuf[1].sem_num = SEM_RD_ALIVE;
    errno = 0;
    int err = semop(semid, opbuf, 2);
    //KILLME;
    if(err == -1 && errno == EAGAIN)
        {
        perror("\n[warning]:Other side is terminated\n");
        exit(0);
        }
    else if (err == -1)
        {
        perror("\n[error]:Can't do semop\n");
        exit(-1);
        }
    }

void check_is_writer_alive(int semid)
    {
    struct sembuf opbuf[2];
    opbuf[0].sem_op = -1;
    opbuf[1].sem_op = 1;
    opbuf[0].sem_flg = IPC_NOWAIT;
    opbuf[1].sem_flg = 0;
    opbuf[0].sem_num = opbuf[1].sem_num = SEM_WR_ALIVE;
    errno = 0;
    int err = semop(semid, opbuf, 2);
    //KILLME;
    if(err == -1 && errno == EAGAIN)
        {
        perror("\n[warning]:Other side is terminated\n");
        exit(0);
        }
    else if (err == -1)
        {
        perror("\n[error]:Can't do semop\n");
        exit(-1);
        }
    }


void check_did_writer_exist(int semid)
    {
    struct sembuf opbuf[2];
    opbuf[0].sem_op = -1;
    opbuf[1].sem_op = 1;
    opbuf[0].sem_flg = IPC_NOWAIT;
    opbuf[1].sem_flg = 0;
    opbuf[1].sem_num = opbuf[0].sem_num = SEM_WR_EXISTED;
    //KILLME;
    errno = 0;
    int err = semop(semid, opbuf, 2);
    if(!err)
        {
        perror("\n[warning]:Writer has already existed\n");
        exit(-1);
        }
    if(err = -1 && errno != EAGAIN)
        {
        perror("\n[error]:Can't do semop\n");
        exit(-1);
        }
    }

void check_did_reader_exist(int semid)
    {
    struct sembuf opbuf[2];
    opbuf[0].sem_op = -1;
    opbuf[1].sem_op = 1;
    opbuf[0].sem_flg = IPC_NOWAIT;
    opbuf[1].sem_flg = 0;
    opbuf[1].sem_num = opbuf[0].sem_num = SEM_RD_EXISTED;
    errno = 0;
    int err = semop(semid, opbuf, 2);
    if(!err)
        {
        perror("\n[warning]:Writer has already existed\n");
        exit(-1);
        }
    if(err = -1 && errno != EAGAIN)
        {
        perror("\n[error]:Can't do semop\n");
        exit(-1);
        }
    }


int main(int argc, char* argv[])
    {
    long long inpMode;
    int modeErr = longIntFromCmdLine(argc, argv, &inpMode);
    if(modeErr)
        {
        perror("\n[error]:Bad input\n");
        exit(-1);
        }
    int mode = (int)inpMode;
    
    key_t key, key2;
    TRY(key = ftok(PATHNAME, 0), "\n[error]:Can't generate key\n");
    TRY(key2 = ftok(PATHNAME, 1), "\n[error]:Can't generate key2\n");
    
    int semid, shmid, shwrid;
    TRY(semid = semget(key, QSEM, 0666 | IPC_CREAT), "\n[error]:Can't get semid\n");
    TRY(shmid = shmget(key, BUFSIZE * sizeof(char), 0666 | IPC_CREAT), "\n[error]:Can't get shmget\n");
    TRY(shwrid = shmget(key2, sizeof(int), 0666 | IPC_CREAT), "\n[error]Can't get shwrid\n");

    char* shBuf;
    char buf[BUFSIZE];
    int* shWritten;
    int written;

    if((shBuf = (char*)shmat(shmid, NULL, 0)) == (char*)(-1))
        {
        perror("\n[error]:Can't attach memory\n");
        exit(-1);
        }
    if((shWritten = (int*)shmat(shwrid, NULL, 0)) == (int*)(-1))
        {
        perror("\n[error]:Can't attach memory\n");
        exit(-1);
        }

    if(mode == WRITER)
        {
        if(argc < 3)
            {
            perror("\n[error]:Too few arguments\n");
            exit(-1);
            }
        int inpFd;
        TRY(inpFd = open(argv[2], O_RDONLY), "\n[error]:Can't open input file\n");
        

        writer_prologue(semid);    
        check_did_writer_exist(semid);

        struct sembuf opbuf[3];
        opbuf[0].sem_op = -1;
        opbuf[1].sem_op = 1;
        opbuf[0].sem_flg = opbuf[1].sem_flg = 0;
        opbuf[0].sem_num = opbuf[1].sem_num = SEM_RD_CONTROL;
        TRY(semop(semid, opbuf, 2),"\n[error]:Can't do semop\n");


        int flag = 0;
        while(1)
            {
            //TRY(written = read(inpFd, buf,BUFSIZE), "\n[error]:Can't read\n");
            //KILLME;       
            opbuf[0].sem_op = 0;
            opbuf[1].sem_op = 1;
            opbuf[0].sem_flg = opbuf[1].sem_flg = 0;
            opbuf[1].sem_num = opbuf[0].sem_num = SEM_FULL;
            TRY(semop(semid, opbuf, 2), "\n[error]:Can't do semop\n");
            //KILLME;
            if(!flag)
                {
                flag++;
                opbuf[0].sem_op = 1;
                opbuf[0].sem_flg = SEM_UNDO;
                opbuf[0].sem_num = SEM_RD_EXISTED;
                TRY(semop(semid, opbuf, 1), "\n[error]:Can't do semop\n");
                }
            //KILLME;
            check_is_reader_alive(semid);
            

            TRY(written = read(inpFd, buf,BUFSIZE), "\n[error]:Can't read\n");
            *shWritten = written;
            int i = 0;
            for(i = 0;i < written; i++)shBuf[i] = buf[i];
 
            if(written <= 0)break;
            //KILLME;
            opbuf[0].sem_op = -1;
            opbuf[0].sem_flg = IPC_NOWAIT;
            opbuf[0].sem_num = SEM_EMPTY;
            errno = 0;
            //KILLME;
            int err = semop(semid, opbuf, 1);
            if(err == -1 && errno == EAGAIN)
                {
                perror("\n[error]:Can't do semop\n");
                exit(-1);
                }
            }

        }
    if(mode == READER)
        {
        reader_prologue(semid);
        //KILLME;
        check_did_reader_exist;

        struct sembuf opbuf[3];
        opbuf[0].sem_op = -1;
        opbuf[1].sem_op = 1;
        opbuf[0].sem_flg = opbuf[1].sem_flg = 0;
        opbuf[0].sem_num = opbuf[1].sem_num = SEM_WR_CONTROL;
        TRY(semop(semid, opbuf, 2),"\n[error]:Can't do semop\n");
        //KILLME;
        int flag = 0;
        while(1)
            {
            opbuf[0].sem_op = 0;
            opbuf[1].sem_op = 1;
            opbuf[0].sem_flg = opbuf[1].sem_flg = 0;
            opbuf[0].sem_num = opbuf[1].sem_num = SEM_EMPTY;
            TRY(semop(semid, opbuf, 2), "\n[error]:Can't do semop\n");
            //KILLME; 
            if(!flag)
                {
                flag++;
                opbuf[0].sem_op = 1;
                opbuf[0].sem_flg = SEM_UNDO;
                opbuf[0].sem_num = SEM_WR_EXISTED;
                TRY(semop(semid, opbuf, 1), "\n[error]:Can't do semop\n");
                }
            //check_is_writer_alive(semid);
            //KILLME;
            if(*shWritten == 0)
                {
                perror("Finished\n");
                exit(0);
                }
            written = *shWritten;
            //printf("%d\n", written);
            write(1, shBuf, written);
            //sleep(1);
            check_is_writer_alive(semid);

            opbuf[0].sem_op = -1;
            opbuf[0].sem_flg = IPC_NOWAIT;
            opbuf[0].sem_num = SEM_FULL;
            errno = 0;
            int err = semop(semid, opbuf, 1);
            if(errno == EAGAIN)
                {
                perror("\n[error]:Can't do semop\n");
                exit(-1);
                }
            //KILLME;
            }
            

        }
    if(mode != WRITER && mode != READER)
        {
        perror("\n[error]:Wrong mode\n");
        exit(-1);
        }
    }

#undef TRY
#undef PATHNAME
#undef KILLME
#undef QSEM
#undef BUFSIZE


