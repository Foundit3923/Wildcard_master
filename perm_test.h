//
// Created by olson on 9/15/2020.
//

//
// Created by Michael Olson and Daniel Davis on 07/05/2020.
// Moves through a string looking for a wildcard pattern
// Experiment with overlapping text windows
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

//---------//
// GLOBALS //
//---------//

#define LAST_BITS_ON 72340172838076673     // 00000001 repeated 8 times
#define frshift(x,n) (x & (x>>n))
#define haszero(v) (((v) - 0x0101010101010101UL) & ~(v) & 0x8080808080808080UL)
#define hasvalue(x,n) ((x) ^ (LAST_BITS_ON * (n)))

#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED   "\x1b[31m"

union Window {
    uint64_t* i;
    char* c;
}Window;

union BoolInt {
    uint64_t i;
    bool b[8];
};

void printBits(uint64_t num) {
    int i;
    char spac = ' ';
    char one = '1';
    char zero = '0';
    int space = 7;
    if(num & 0x8000000000000000UL){
        printf("%c",one);
    } else{
        printf("%c",zero);
    }
    for( i = 1; i < 72; i++){
        // printf("%i", p);
        if(space == 0){
            printf("%c",spac);
            space = 9;
        }
        else if(((num <<= 1) & 0x8000000000000000UL)){
            printf(COLOR_RED "%c" COLOR_RESET,one);

        }
        else{
            printf("%c",zero);
        }
        space--;
    }
}

bool Experimental_wildcard_arbitrary_length_moving_union_save (char* st,
                                                               char* query_array) {
    char** test = (char**) malloc(sizeof(char*));

    *test = "stringe";


    uint64_t perms[255];
    uint64_t temp = 0;
    uint64_t c;
    uint64_t singles[8];
    singles[0] = 0x1UL;
    singles[1] = 0x100UL;
    singles[2] = 0x10000UL;
    singles[3] = 0x1000000UL;
    singles[4] = 0x100000000UL;
    singles[5] = 0x10000000000UL;
    singles[6] = 0x1000000000000UL;
    singles[7] = 0x100000000000000UL;

    int i;

    int pop[255];
    int known_pop;
    int check_pop;

    bool match = true;

    for(i = 1; i < 256; i++){
        temp = 0;
        c = i;
        if(c & 1){
            temp |= singles[0];
        }
        c >>= 1;
        if(c & 1){
            temp |= singles[1];
        }
        c >>= 1;
        if(c & 1){
            temp |= singles[2];
        }
        c >>= 1;
        if(c & 1){
            temp |= singles[3];
        }
        c >>= 1;
        if(c & 1){
            temp |= singles[4];
        }
        c >>= 1;
        if(c & 1){
            temp |= singles[5];
        }
        c >>= 1;
        if(c & 1){
            temp |= singles[6];
        }
        c >>= 1;
        if(c & 1){
            temp |= singles[7];
        }
        perms[i] = temp;
        pop[i] = __builtin_popcountll(temp);

        printf("\nPermutation %i: \n", i);

        printBits(temp);

        printf("\nPopCount of Perm %i: %i\n", i, pop[i]);

        printf("PopCount to check: %i\n", (uint8_t) ((perms[i] / 255 ) + (perms[i] & 1)));

        printBits((perms[i] / 255 ) + (perms[i] & 1));

        if(pop[i] == (uint8_t) (perms[i] / 255 ) + (perms[i] & 1)){
            printf("\nPop Match\n");
        } else {
            printf("\nPop Mismatch\n");
            match = false;
        }

    }

    if(match){
        printf("\nAll Match\n");
    } else {
        printf("\nMismatch Found\n");
    }



}

