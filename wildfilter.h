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
#define haszero(v) (((v) - 0x0101010101010101UL) & ~(v) & 0x8080808080808080UL)

union Window {
    uint64_t* i;
    char* c;
}Window;

bool Experimental_wildcard_arbitrary_length_moving_union_save (char* st,
                                                               char* query_array) {
    int text_len = (uint64_t) strlen(st);
    int query_len = (uint64_t) strlen(query_array);
    int text_modifier = query_len - 1;
    int shift_count = 0;

    char* char_ptr;
    char* last;
    char* string_check[2];

    union Window t_w;
    t_w.c = &st[text_len - text_modifier];

    uint64_t query_matches = LAST_BITS_ON;
    uint64_t value;

    bool check = false;

    last = &query_array[text_modifier];

    char_ptr = last;

    string_check[0] = last;

    text_modifier = text_len - text_modifier;

    while(text_modifier >= 0) {
        value = ~((*t_w.i) ^ (LAST_BITS_ON * (*char_ptr)));
        if(haszero(~value)){
            query_matches = (((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL & ((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL >> 2)) &
                            (((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL & ((value & (value >> 4)) & 0xF0F0F0F0F0F0F0FUL >> 2)) >> 1));
        }

        check = query_matches > 0;

        if(&*char_ptr == &query_array[0]){
            return true;
        }

        string_check[1] = char_ptr - check;
        char_ptr = string_check[check];

        shift_count += check;

        t_w.c = &st[(text_modifier -= ((8 + shift_count) * !check) + check)];
    }
    return false;
}
