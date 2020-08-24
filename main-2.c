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
#define hasvalue(x,n) \
(haszero((x) ^ (~0UL/255 * (n))))

union Window {
    uint64_t* i;
    char* c;
}Window;

bool Experimental_wildcard_arbitrary_length_moving_union_save (char* st,
                                                               char* query_array) {

    char* char_ptr;
    char* last;
    char* copy_array = (char*) malloc(sizeof(char));

    strcpy(copy_array, query_array);

    int text_len = strlen(st);
    int query_len = strlen(query_array);
    int text_modifier = query_len - 1;
    int backtrack_location = 0;
    int backtrack_count = 0;
    int word_size = sizeof(size_t);

    union Window t_w;
    t_w.c = &st[text_modifier];

    uint64_t query_matches = LAST_BITS_ON;

    bool result = false;

    if(text_modifier>=0) {
        last = &query_array[text_modifier];
        copy_array[text_modifier] = '\000';
    }
    char_ptr = last;

    while(text_modifier <= text_len)  {
        //if possible match, and previous match
        //try to match
        query_matches &= frshift(frshift(frshift(~hasvalue(*t_w.i, *char_ptr),4),2),1);
        if (query_matches) {
            //get in position
            if(!backtrack_count) {
                char_ptr = copy_array;
                t_w.c = &st[backtrack_count = backtrack_location];
            }
            //move forward
            char_ptr ++;
            t_w.c = &st[backtrack_count++];
            if(!*char_ptr){
                //match found!
                return true;
            }
        } else {
            //move to next section and reset
            text_modifier += word_size*2;
            backtrack_location += word_size*2;
            backtrack_count = 0;
            char_ptr = last;
            t_w.c = &st[text_modifier];
            query_matches = LAST_BITS_ON;
        }
    }
    return result;
}
