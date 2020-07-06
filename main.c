//
// Created by Michael Olson on 06/15/2020.
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

union Window {
    uint64_t* i;
    char* c;
}Window;

bool Experimental_wildcard_arbitrary_length_moving_union_1 (char st[],
                                                  char** subquery_array,
                                                  uint64_t f_m[]) {
    //-------------//
    //Preprocessing//
    //-------------//
    /* Done externally */

    //----------//
    //Processing//
    //----------//


    char character;
    char* char_ptr;

    // As text_modifier increases by 8 we move to the next section of the text
    int text_modifier = 0;
    int text_len = strlen(st);
    float sections = text_len/8.00;
    sections = ceilf(sections);
    int section_shifts = 1;

    union Window t_w;
    t_w.c = &st[text_modifier+1];

    uint64_t subquery_matches = 0;
    uint64_t encoded_term;

    int subquery_count = 0;

    // Points to beginning of array
    char_ptr = subquery_array[subquery_count];
    character = *char_ptr;

    // to account for singlecards '?' we could insert it as a subquery by itself that matches anything and moves the term forwards by one
    while(character != 0 && st[text_modifier ] != 0) {
        encoded_term = ~(f_m[character] ^ *t_w.i);
        if(subquery_matches == 0) {
            subquery_matches = LAST_BITS_ON;
        }
        //Search for matches
        int match_count;
        for( match_count = 0; match_count < 8; match_count++){
            subquery_matches &= encoded_term;
            encoded_term >>= 1;
        }
        if (subquery_matches == 0) {
            //no matches, move on
            if(section_shifts < sections) {
                text_modifier += 8;
                t_w.c = &st[text_modifier];
                subquery_matches = 0;
            } else{
                return false;
            }
        } else {
            //character matches, move to next character
            char_ptr++;
            if (*char_ptr == 0) {
                //Move to next subquery
                if (*(subquery_array + (subquery_count + 1)) != 0) {
                    subquery_count++;
                    char_ptr = *(subquery_array + subquery_count);
                    subquery_matches = 0;
                } else {
                    return true;
                }
            }
            character = *char_ptr;
            t_w.c = &st[++text_modifier];
            t_w.c++;
        }
    }
}
