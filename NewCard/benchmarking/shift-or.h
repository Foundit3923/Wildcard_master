#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#define WORD 64
#define EOS 0
#define B 1
#define MAXSYM 128

int Search(char* text, char* pattern){
    register unsigned int state, lim, first, inital;
    unsigned int T[MAXSYM];
    int i,j,matches;

    if(strlen(pattern)>WORD){
        printf("Use patter size <= word size");
    }
    for(i=0; i<MAXSYM;i++) T[i] = ~0;
    lim = 0;
    for(j=1; *pattern != EOS; j <<= B){
        T[*pattern] &= ~j;
        lim |= j;
        pattern++;
    }
    lim = ~(lim >> B);
    matches = 0;
    inital = ~0; first = *pattern;
    do{
        while(*text != EOS && *text != first) text++;
        state = inital;
        do{
            state = (state << B) | T[*text];
            if(state < lim) matches++;
            text++;
        }while(state != inital);
    }while(*(text-1) != EOS);
    return( matches );
}