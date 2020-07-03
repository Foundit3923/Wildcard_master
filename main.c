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
#include "KMP_arbitrary_length_wildcard.h"

// Assumes little endian

//---------//
// GLOBALS //
//---------//

// For making things explicit and avoiding "Magic Numbers"
// All are hard coded for BYTE_LENGTH == 8 and MAX_TERMS = 8
// All assume little-endian and that Python is handling the longs properly internally

#define BYTE_TRAILING_BIT_ON 1 // 00000001
#define RIGHT_BYTE_ON 255
#define BYTE_LENGTH 8
#define ALL_BITS_ON 18446744073709551615   // 11111111 repeated 8 times
#define ALL_BITS_OFF 0                     // 00000000 repeated 8 times
#define LAST_BITS_ON 72340172838076673     // 00000001 repeated 8 times
#define LAST_BITS_OFF 18374403900871474942 // 11111110 repeated 8 times
#define TRAILING_BIT_ON 1  // 00000000...64 bits...00000001

// More like config constants
#define MAX_TERMS (sizeof(uint64_t)) // Should be 8
#define DELIMITER "*" // The wild card itself
#define DEBUG false

union Window {
    uint64_t* i;
    char* c;
}Window;

bool Experimental_wildcard_arbitrary_length_V5_2 (char st[],
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
    int subquery_position = 0;
    int uniquery_position = 0;
    int saved_subquery_position = 0;
    int saved_uniquery_position = 0;
    int text_window_shift = 8;

    uint64_t anchor_check[8] = {255,65280,16711680,4278190080,1095216660480,280375465082880,71776119061217280,18374686479671623680};

    uint64_t search_term = 0;
    int n = 8;
    if( text_len - text_modifier < 8){
        text_window_shift = text_len - text_modifier + 1;
    }
    union Window t_w;
    t_w.c = &st[text_modifier];
    t_w.c++;
    search_term = *t_w.i;

    uint64_t subquery_matches = 0;
    uint64_t encoded_term;
    uint64_t prev_section = 0;

    bool shifted_char = false;
    bool save_point = false;
    bool picking_up_save_point = false;
    int subquery_count = 0;

    // Points to beginning of array
    char_ptr = subquery_array[subquery_count];
    character = *char_ptr;

    // to account for singlecards '?' we could insert it as a subquery by itself that matches anything and moves the term forwards by one
    while(character != 0 && st[text_modifier ] != 0) {
        // 6
        uint64_t char_to_check = character;
        uint64_t char_mask = f_m[ char_to_check ];
        encoded_term = ~(char_mask ^ search_term);
        if(subquery_matches == 0) {
            subquery_matches = LAST_BITS_ON;
        }
        //Search for matches
        //4*8 = 32
        int match_count;
        for( match_count = 0; match_count < 8; match_count++){
            subquery_matches &= encoded_term;
            encoded_term >>= 1;
        }
        if (subquery_matches == 0) {
            //check if there is a save
            if(save_point){
                //reset to save
                save_point = false;
                picking_up_save_point = true;
                char_ptr = subquery_array[saved_subquery_position];
                char_ptr += saved_uniquery_position + 1;
                character = *char_ptr;
                subquery_position = saved_subquery_position;
                uniquery_position = saved_uniquery_position + 1;
                saved_subquery_position = 0;
                saved_uniquery_position = 0;

                //Move text window
                text_modifier += 8;
                search_term = 0;
                int n = 8;
                if( text_len - text_modifier < 8){
                    text_window_shift = text_len - text_modifier + 1;
                }
                t_w.c = &st[text_modifier];
                search_term = *t_w.i;
                // when we move to the next section of the text we know that whatever we find is after the last subquery
                section_shifts++;
                subquery_matches = 0;
                // Restart
            } else if(section_shifts < sections) {
                text_modifier += 8;
                search_term = 0;
                int n = 8;
                if( text_len - text_modifier < 8){
                    text_window_shift = text_len - text_modifier + 1;
                }
                t_w.c = &st[text_modifier];
                search_term = *t_w.i;
                // when we move to the next section of the text we know that whatever we find is after the last subquery
                subquery_matches = 0;
                // for when the first character of a subquery is found but the second is not
                if(shifted_char && !(prev_section & 72057594037927936)){
                    // reset the subquery
                    char_ptr = *(subquery_array + subquery_count);
                    character = *char_ptr;
                }
            } else{
                return false;
            }
        } else {
            //if there are sq matches then we know that the character we checked for is the right position
            //if there was a save point and matches we need to check if the match is in the right position
            if( picking_up_save_point) {
                picking_up_save_point = false;
                if(!(subquery_matches & 1)){
                    char_ptr = *(subquery_array + subquery_count);
                    character = *char_ptr;
                    subquery_matches = 0;
                }
            }
            //now we check if anything is in the last position, if it is then we save the query position
            //if the last position is not set then we shift sqm and restart
            if( subquery_matches & anchor_check[7]) {
                //save some how
                if (char_ptr++) {
                    char_ptr--;
                    save_point = true;
                    saved_subquery_position = subquery_position;
                    saved_uniquery_position = uniquery_position;
                }
            }
            //Move to next character
            char_ptr++;
            uniquery_position++;
            shifted_char = true;
            if (*char_ptr == 0) {
                //Move to next subquery
                if (*(subquery_array + (subquery_count + 1)) != 0) {
                    subquery_count++;
                    subquery_position++;
                    char_ptr = *(subquery_array + subquery_count);
                    subquery_matches = 0;
                } else {
                    return true;
                }
            }
            character = *char_ptr;
            /*if(subquery_matches) {
                subquery_matches <<= 8;
            }*/
            text_modifier++;
            t_w.c = &st[text_modifier];
            search_term = *t_w.i;

        }
    }
    // Did we reach the end of the text while a query character remains?
    if(st[text_modifier] == 0){
        return false;
    } else {
        return true;
    }
}
