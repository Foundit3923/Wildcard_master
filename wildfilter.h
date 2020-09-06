//
// Created by michael on 8/29/20.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>


#define LAST_BITS_ON 0x101010101010101UL
#define ALL_ON 0xFFFFFFFFFFFFFFFFUL
#define hassetbyte(v) ((~(v) - 0x0101010101010101UL) & (v) & 0x8080808080808080UL)

unsigned int v; // count bits set in this (32-bit value)
unsigned int c; // store the total here
static const int S[] = {1, 2, 4, 8, 16}; // Magic Binary Numbers
static const int B[] = {0x5555555555555555, 0x3333333333333333, 0x0F0F0F0F0F0F0F0F, 0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF};

uint64_t long_mask = {0xFFUL,0xFFFFUL,0xFFFFFFUL,0xFFFFFFFFUL,0xFFFFFFFFFFUL,0xFFFFFFFFFFFFUL,0xFFFFFFFFFFFFFFUL,0xFFFFFFFFFFFFFFFFUL};

union Window {
    uint64_t* i;
    char* c;
}Window;

union Query {
    uint64_t* i;
    char* c;
};

int Experimental_wildcard_arbitrary_length_moving_union_save (char* st,
                                                               char* query_array) {
    int text_len = (uint64_t) strlen(st);
    int query_len = (uint64_t) strlen(query_array);
    int text_modifier = query_len - 1;
    int shift_count = 0;
    int result = 0;
    int distance = 1;
    int i_check[2];
    int i;

    //char* q.c;
    char* last;
    char* string_check[2];

    union Window t_w;
    t_w.c = &st[text_len - text_modifier];
    
    union Query q;
    q.c = &query_array[text_modifier];

    uint64_t query_matches = LAST_BITS_ON;
    uint64_t value;
    uint64_t int_check[2];

    bool check = false;

    string_check[0] = &query_array[text_modifier];

    int_check[0] = LAST_BITS_ON;

    text_modifier = text_len - text_modifier;

    i_check[0] = 1;


    while(text_modifier >= 0) {
        value = ~((*t_w.i) ^ ((LAST_BITS_ON & query_matches) * (*q.i & long_mask[distance])));

        check = value > 0;

        if(hassetbyte(value)){
            query_matches = (((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL & ((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL >> 2)) &
                            (((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL & ((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL >> 2)) >> 1));
            distance = __builtin_clz(query_matches);
            distance = ceil(msb /8);
            if(distance != 7){
                for( i=0; i < distance; i++){
                    if(query_matches & (query_matches >> (i*8))){
                        distance = i;
                }
            }
        }

        if(&*q.c == &query_array[0]){
            resutl = __builtin_popcount(query_matches);
        }
            
        i_check[1] = distance;
        distance = i_check[check];

        int_check[1] = query_matches;
        query_matches = int_check[check];

        string_check[1] = q.c - distance;
        q.c = string_check[check];
        
        i_check[0] = 0;
        i_check[1] = shift_count + distance;    
        shift_count = i_check[count];

        t_w.c = &st[(text_modifier -= ((8 + shift_count) * !check) + (distance * check))];
    }
    return result;
}

