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

    uint64_t anchor_check[8] = {255,65280,16711680,4278190080,1095216660480,280375465082880,71776119061217280,18374686479671623680};

    uint64_t search_term = 0;
    int j;
    // 8 * 8 = 64
    for (j = 0; j < 8; j++) {
        if(st[j+text_modifier] != '\0') {
            search_term |= (f_m[st[j + text_modifier]] & anchor_check[j]);
        } else {
            break;
        }
    }

    uint64_t subquery_matches = 0;
    uint64_t encoded_term = search_term;
    uint64_t last_sqm = 0;
    uint64_t prev_section = 0;


    bool changed_subqueries = true;
    bool changed_section = false;
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
                changed_section = true;
                text_modifier += 8;
                search_term = 0;
                int j;
                for (j = 7; j >= 0; --j) {
                    search_term <<= 8;
                    search_term |= (uint64_t) st[ j + text_modifier ];
                }

                // when we move to the next section of the text we know that whatever we find is after the last subquery
                section_shifts++;
                subquery_matches = 0;
                // Restart
            } else if(section_shifts < sections) {
                changed_section = true;
                text_modifier += 8;
                search_term = 0;
                int j;
                for (j = 7; j >= 0; --j) {
                    search_term <<= 8;
                    search_term |= (uint64_t) st[ j + text_modifier ];
                }
                // when we move to the next section of the text we know that whatever we find is after the last subquery
                prev_section = last_sqm;
                last_sqm = 0;
                section_shifts++;
                // for when the first character of a subquery is found but the second is not
                if(shifted_char && !(prev_section & 72057594037927936)){

                    // reset the subquery
                    char_ptr = *(subquery_array + subquery_count);
                    character = *char_ptr;
                    changed_subqueries = true;
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
                    changed_subqueries = true;
                    char_ptr = *(subquery_array + subquery_count);
                    subquery_matches = 0;

                } else {
                    return true;
                }
            }
            character = *char_ptr;
            if(subquery_matches) {
                subquery_matches <<= 8;
            }

        }

    }
    // Did we reach the end of the text while a query character remains?
    if(st[text_modifier] == 0){
        return false;
    } else {
        return true;
    }
}
