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

    //t_w.c = "abcaba";


    while(text_modifier >= 0) {
        value = ~((*t_w.i) ^ ((LAST_BITS_ON & query_matches) * (*q.c)));

        check = value > 0;

        if(hassetbyte(value)){
            query_matches = (((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL & ((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL >> 2)) &
                            (((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL & ((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL >> 2)) >> 1));
        }

        if(&*q.c == &query_array[0]){
            query_matches = query_matches - ((query_matches >> 1) & 0x5555555555555555);                          // reuse input as temporary
            query_matches = (query_matches & 0x3333333333333333) + ((query_matches >> 2) & 0x3333333333333333);   // temp
            result += ((query_matches + (query_matches >> 4) & 0xF0F0F0FF0F0F0F) * 0x10101011010101) >> 24;       // count
        }

        int_check[1] = query_matches;
        query_matches = int_check[check];

        string_check[1] = q.c - check;
        q.c = string_check[check];

        shift_count += check;

        t_w.c = &st[(text_modifier -= ((8 + shift_count) * !check) + check)];
    }
    return result;
}

