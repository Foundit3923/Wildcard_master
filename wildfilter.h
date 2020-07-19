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

// Assumes little endian

//---------//
// GLOBALS //
//---------//

#define LAST_BITS_ON 72340172838076673     // 00000001 repeated 8 times
#define filter(a,b) (b &= (a & (a & (a & (a>>4)>>2)>>1)))
#define encode(a,b,c) (c = ~(a ^ b))
#define move(a,b,c) (a = &b[c])
#define sub(a,b) (a = a - b)

union Window {
    uint64_t* i;
    char* c;
    uint8_t e_i;
}Window;

bool Experimental_wildcard_arbitrary_length_moving_union_save (char st[],
                                                               char* subquery_array,
                                                               uint64_t f_m[]) {
    //----------//
    //Processing//
    //----------//

    char* char_ptr;


    int text_len = strlen(st);
    int query_len = strlen(subquery_array);
    int text_modifier = query_len-1;

    int backtrack_location = text_modifier - (query_len-1);
    int backtrack_count = 0;
    int n = text_modifier;
    union Window t_w;
    t_w.c = &st[text_modifier];

    uint64_t query_matches = LAST_BITS_ON;
    uint64_t encoded_text;
    uint64_t encoded_text_2 = 0;
    char_ptr = subquery_array;

    while(*char_ptr != '\000' && text_len >= text_modifier) {
        encoded_text = ~(f_m[*char_ptr] ^ *t_w.i);
        encoded_text_2 = encoded_text & encoded_text>>4;
        encoded_text_2 = encoded_text_2 & encoded_text_2>>2;
        encoded_text = encoded_text_2 & encoded_text_2>>1;
        query_matches &= encoded_text;
        if (query_matches) {
            if(backtrack_count) {
                char_ptr++;
                backtrack_count++;
                t_w.c = &st[backtrack_count];
            } else {
                char_ptr++;
                backtrack_count = backtrack_location;
                t_w.c = &st[backtrack_count];
            }
        } else {
            text_modifier += 8;
            backtrack_location += 8;
            backtrack_count = 0;
            char_ptr = subquery_array;
            t_w.c = &st[text_modifier];
            query_matches = LAST_BITS_ON;
        }
    }
    return text_len >= text_modifier;
}
