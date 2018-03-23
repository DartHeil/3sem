//!------------------------------------------------------------------
//! @read
//! @write
//! @main
//!------------------------------------------------------------------


#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include "cmdline.h"

//!------------------------------------------------------------------
//! Program can work in two modes - READ and WRITE (first argument)
//!------------------------------------------------------------------
enum MODE
    {
    READ = 1,
    WRITE = 2,
    };

//!------------------------------------------------------------------
//! Buffer size, size of string with pid, duration of sleep
//!------------------------------------------------------------------
enum CONSTANTS
    {
    BUFSIZE = 13, 
    PIDSIZE = 16,
    WAITTIME = 500000,
    };

//!------------------------------------------------------------------
//! Name of public fifo
//!------------------------------------------------------------------
#define PUBLICFIFOPATH "public.fifo"

//!------------------------------------------------------------------
//! Macro to print error messages and exit from program
//!------------------------------------------------------------------
#define ERRMSG(msg) \
    { \
    write(1, msg, strlen(msg)); \
    exit(1); \
    }

//!------------------------------------------------------------------
//! Macro for printing messages in case of correct completion
//!------------------------------------------------------------------
#define MSG(msg) \
    { \
    write(1, msg, strlen(msg)); \
    return 0; \
    }

//!------------------------------------------------------------------
//! Very kind macro to sleep program and kill it
//!------------------------------------------------------------------
#define KILLME \
    { \
    printf("\n***KILL ME***\n"); \
    sleep(5); \
    }

//!------------------------------------------------------------------
//! Reads pid and writes it to Pid[].
//! Example: getpid returned 4321, 
//! then Pid[] = "4321$$$$$$$$$$$$$$$" (PIDSIZE symbols in fifo name)
//!------------------------------------------------------------------
void makePid(char* Pid, const int pidsize)
    {
    int pid = getpid();
    int i;
    for(i = 0; i < pidsize; i++)
        {
        Pid[i] = '$';
        }
    sprintf(Pid, "%d", pid); 
    Pid[strlen(Pid)] = '$';
    Pid[pidsize - 1] = '\0';
    //printf(">>My PID is %s\n", Pid);
    }


//!------------------------------------------------------------------
//! Write Pid[] ot public fifo
//!------------------------------------------------------------------
void putPid(const char* Pid, const int pidsize)
    {
    int publFD = open(PUBLICFIFOPATH, O_WRONLY);
    if(publFD == -1)ERRMSG("\n!Can't open public fifo\n");
    write(publFD, Pid, pidsize);
    close(publFD);
    }

//!------------------------------------------------------------------
//! Obvious
//!------------------------------------------------------------------
void makeFifo(const char* fifoname)
    {
    umask(0);
    mkfifo(fifoname, S_IFIFO | 0666);
    }

//!------------------------------------------------------------------
//! Reads first PID from public fifo
//!------------------------------------------------------------------
void getPid(char* Pid, const int pidsize)
    {
    printf(">>Connecting to "PUBLICFIFOPATH"...");
    int publFD = open(PUBLICFIFOPATH, O_RDONLY);
    if(publFD == -1)ERRMSG("\n!Can't open public fifo\n");
    printf("OK\n");
    int letters = read(publFD, Pid, pidsize);
    if(!letters)ERRMSG("!Can't get PID\n");
    printf(">>PID is %s chosen\n", Pid);
    //...
    close(publFD);
    }
 
//!------------------------------------------------------------------
//! @write 2
//! Writing mode
//!   {
//!   getPid          - get first pid from public.fifo
//!   privateFD = open(...O_WRONLY | O_NDELAY);
//!   if(privateFD == -1) exit
//!   fcntl(...O_WRONLY)
//!   {..
//!     **WRITE FROM INPUTFILE TO FIFO** - in blocked mode
//!   ..}
//!   }
//!------------------------------------------------------------------
int writingMode(int argc, char* argv[])
    {
    int qLetters;
    int publFD;
    char Pid[PIDSIZE];
    char buffer[BUFSIZE];
    int i;
    int error;
    int privateFD;

    if(argc < 3)ERRMSG("\n!Too few arguments\n");
    const char* inpFile = argv[2];
    //! read first pid from public.fifo
    getPid(Pid, PIDSIZE);

    printf(">>Trying to connect with reader...");
    
    privateFD = open(Pid, O_WRONLY | O_NDELAY);

    if(privateFD == -1)ERRMSG("\n!Seems that connection with reader is lost\n");

    fcntl(privateFD, F_SETFL, O_WRONLY);
    
    printf("OK\n");
    
    printf(">>Begin to write...\n");

    int fdInp = open(inpFile, O_RDONLY);
    if(fdInp == -1)ERRMSG("\n!Input file doesn exist\n");

    while((qLetters = read(fdInp, buffer, BUFSIZE - 1)) > 0)
        {

        write(privateFD, buffer, qLetters);
        buffer[BUFSIZE - 1] = '\0';

        }
    if(qLetters == -1)MSG("\n>>Reader doesnt exist anymore\n");

    printf(">>Writing completed\n");
    }


//!------------------------------------------------------------------
//! @read 1
//! Reading Mode
//!   makePid
//!   makeFifo(Pid)   - private fifo
//!   privateFD = open(..O_RDONLY | O_NDELAY)
//!   putPid          - put Pid to public.fifo
//!   usleep
//!   if(read(privateFD) == 0) leave
//!   fcntl(..O_RDONLY)
//!   {..
//!     **READ FROM FIFO AND PRINT**      - in blocked mode
//!   ..}
//!------------------------------------------------------------------
int readingMode(int argc, char* argv[])
    {
    int qLetters;
    int publFD;
    char Pid[PIDSIZE];
    char buffer[BUFSIZE];
    int i;
    int error;

    makePid(Pid,PIDSIZE);
    
    makeFifo(Pid);
    
    int privateFD = open(Pid, O_RDONLY | O_NDELAY);
    
    
    putPid(Pid, PIDSIZE);
    
    usleep(WAITTIME);
    
    //trying to read
    char testchar;
    errno = 0;
    error = read(privateFD, &testchar, 1);
    if(error  == -1 || error > 0)
        {
        fcntl(privateFD, F_SETFL, O_RDONLY);
        }
    else if (error == 0)ERRMSG("\n!Timed out\n");
    if(error > 0)write(1, &testchar, 1);
    
    while((qLetters = read(privateFD, buffer, BUFSIZE - 1)) > 0)
        {
        buffer[BUFSIZE - 1] = '\0';
        write(1, buffer, qLetters);
        }
    //if(qLetters == 0)MSG("\n>>Reading is finished(writing side terminated)\n");
    //if(qLetters == -1)MSG("\n>>Reading seems finished. Fifo is empty\n");
        
    }
    

//!------------------------------------------------------------------
//! @main
//!------------------------------------------------------------------
int main(int argc, char* argv[])
    {
    //define mode - READ or WRITE (uses my function from cmdline.h) to parse argument
    long long int par;
    int errInp = longIntFromCmdLine(argc, argv, &par);
    if(errInp || par > WRITE || par < READ)ERRMSG("Bad input\n");    
    int mode = (int) par;

    umask(0);
    mkfifo(PUBLICFIFOPATH, S_IFIFO | 0666); // make public.fifo where pids
                                            // must be taken

    switch(mode)
        {
        case WRITE:
            return writingMode(argc, argv);
            break;    
        case READ:
            return readingMode(argc, argv);
            break;
        default:
            break;
        }
    }

#undef PUBLICFIFOPATH
#undef MSG
#undef ERRMSG
#undef KILLME
