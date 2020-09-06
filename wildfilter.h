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
#include <pthread.h>


#define LAST_BITS_ON 0x101010101010101UL
#define ALL_ON 0xFFFFFFFFFFFFFFFFUL
#define hassetbyte(v) ((~(v) - 0x0101010101010101UL) & (v) & 0x8080808080808080UL)

union Window {
    uint64_t* i;
    char* c;
}Window;

union Query {
    uint64_t* i;
    char* c;
};

union Query q;

union Window t_w;

bool Experimental_wildcard_arbitrary_length_moving_union_save (char* st,
                                                               char* query_array) {
    int text_len = (uint64_t) strlen(st);
    int query_len = (uint64_t) strlen(query_array);
    int text_modifier = query_len - 1;

    char* last;
    char* string_check[2];


    t_w.c = &st[text_modifier];


    q.c = &query_array[0];

    uint64_t query_matches = LAST_BITS_ON;
    uint64_t value;
    uint64_t int_check[2];

    bool check = false;

    last = &query_array[text_modifier];

    string_check[0] = &query_array[0];

    int_check[0] = LAST_BITS_ON;

    while(text_modifier <= text_len) {
        value = ~((*t_w.i) ^ ((LAST_BITS_ON & query_matches) * (*q.c)));
        if(hassetbyte(value)){
            t_w.c = &st[text_modifier + query_len-1];
            query_matches = ~((*t_w.i) ^ ((LAST_BITS_ON & query_matches) * (*last)));
            check = query_matches > 0;
            if(hassetbyte(query_matches)) {
                query_matches = value & (value >> 4) & 0xF0F0F0F0F0F0F0FUL;
                query_matches = ((query_matches & (query_matches >> 2)) &
                                  ((query_matches & (query_matches >> 2)) >> 1));
            }
        }

        if(&*q.c == &query_array[query_len] && check) {
            return true;
        }

        int_check[1] = query_matches;
        query_matches = int_check[check];

        string_check[1] = q.c + check;
        q.c = string_check[check];

        text_modifier += (8 * !check) + check;

        t_w.c = &st[text_modifier];

    }
    return false;
}

