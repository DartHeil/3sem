#ifndef ERRHAND_CPP
#define ERRHAND_CPP

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>

enum ERRORS
    {
    DEFAULT,
    TOO_FEW_ARGS,
    TOO_MANY_ARGS,
    OUTOFRNG,
    BAD_INP,
    EXTRA,
    };

int iclread(int argc, char** input, long long* quantity)
    {
    errno = 0;
    *quantity = 0;
    char* endptr;
  

    if(argc < 2)
        {
        printf("Too few arguments\n");
        return TOO_FEW_ARGS;
        }

    if(argc > 3)
        {
        printf("Too many arguments\n");
        return TOO_MANY_ARGS;
        }

//    if((*quantity == LONG_MAX || *quantity == LONG_MIN) && errno == ERANGE)
  //      {
    //    printf("Out of range\n");
      //  return OUTOFRNG;
        //}
   
    *quantity = strtoll(input[1], &endptr, 10);
 
    if(endptr == input[1])
        {
        printf("Haven't been found any digits. Bad input\n");
        return BAD_INP;
        }
    if(*endptr != '\0')
        {
        printf("Not correct input(Some extra symbols)\n");
        return EXTRA;
        }

    return DEFAULT;
    }
#endif
