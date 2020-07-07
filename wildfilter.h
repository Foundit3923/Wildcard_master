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

union Window {
    uint64_t* i;
    char* c;
}Window;

bool Experimental_wildcard_arbitrary_length_moving_union_1 (char st[],
                                                  char* subquery_array,
                                                  uint64_t f_m[]) {
    //----------//
    //Processing//
    //----------//

    char* char_ptr;
    char* save;

    // As text_modifier increases by 8 we move to the next section of the text
    int text_modifier = 0;
    int text_len = strlen(st);

    union Window t_w;
    t_w.c = &st[text_modifier];

    uint64_t subquery_matches = 0;
    uint64_t encoded_term;

    int subquery_count = 0;

    // Points to beginning of array
    char_ptr = &subquery_array[subquery_count];
    save = char_ptr;

    while(char_ptr != '\000' && text_len >= text_modifier) {
        if(*char_ptr == '*'){
            char_ptr++;
            save = char_ptr;
            subquery_matches = 0;
            if(!*char_ptr){
                return true;
            }
        }
        encoded_term = ~(f_m[*char_ptr] ^ *t_w.i);
        if(subquery_matches == 0) {
            subquery_matches = LAST_BITS_ON;
        }
        //Search for matches
        int match_count;
        for( match_count = 0; match_count < 8; match_count++){
            subquery_matches &= encoded_term;
            encoded_term >>= 1;
        }
        if (!subquery_matches) {
            //no matches, move on
            text_modifier += 8;
            if(text_modifier < text_len) {
                t_w.c = &st[text_modifier];
                char_ptr = save;
                subquery_matches = 0;
            } else {
                return false;
            }
        } else {
            //char_ptr matches, move to next char_ptr
            char_ptr++;
            t_w.c = &st[++text_modifier];
            //t_w.c++;
        }
    }
    if(text_len >= text_modifier){
        return true;
    } else {
        return false;
    }
}
