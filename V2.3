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

bool Experimental_wildcard_arbitrary_length_V2_3 (char st[],
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

    uint64_t anchor_check[8] = {255,65280,16711680,4278190080,1095216660480,280375465082880,71776119061217280,18374686479671624000};

    uint64_t search_term = 0;
    int j;
    /*
    for (j = 7; j >= 0; --j) {
        search_term <<= 8;
        search_term |= (uint64_t) st[ j + text_modifier ];
    }
    */

    // 8 * 8 = 64
    for (j = 0; j < 8; j++) {
        if(st[j+text_modifier] != '\0') {
            search_term |= (f_m[st[j + text_modifier]] & anchor_check[j]);
        } else {
            break;
        }
    }

    uint64_t subquery_matches = ALL_BITS_ON;
    uint64_t encoded_term = search_term;
    uint64_t last_sqm = 0;
    uint64_t prev_section = 0;


    bool changed_subqueries = true;
    bool changed_section = false;
    bool shifted_char = false;
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
        subquery_matches = LAST_BITS_ON;

        //Search for matches
        //4*8 = 32
        int match_count;
        for( match_count = 0; match_count < 8; match_count++){
            subquery_matches &= encoded_term;
            encoded_term >>= 1;
        }
        if (subquery_matches == 0) {
            if(section_shifts < sections) {
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

            // We only find the location of the first char in a subquery
            if (changed_subqueries) {
                changed_section = false;
                changed_subqueries = false;
                // use last_sqm to find location of previous subquery, mask subquery_matches such that everything up to that point is masked.
                //3
                if (last_sqm != 0) {
                    if (last_sqm & 16843009) {
                        if (last_sqm & 257) {
                            if (last_sqm & 1) {
                                subquery_matches &= 18446744073709551360;
                            } else {
                                subquery_matches &= 18446744073709486080;
                            }
                        } else {
                            if (last_sqm & 65536) {
                                subquery_matches &= 18446744073692774400;
                            } else {
                                subquery_matches &= 18446744069414584320;
                            }
                        }
                    } else {
                        if (last_sqm & 1103806595072) {
                            if (last_sqm & 4294967296) {
                                subquery_matches &= 18446742974197923840;
                            } else {
                                subquery_matches &= 18446462598732840960;
                            }
                        } else {
                            if (last_sqm & 281474976710656) {
                                subquery_matches &= 18374686479671623680;
                            } else {
                                subquery_matches &= 0;
                            }
                        }
                    }
                }
                //Are there still matches?
                if (subquery_matches) {
                    last_sqm = subquery_matches;

                    //Move to next character
                    char_ptr++;
                    shifted_char = true;
                    if (*char_ptr == 0) {
                        //Move to next subquery
                        if (*(subquery_array + (subquery_count + 1)) != 0) {
                            subquery_count++;
                            changed_subqueries = true;
                            char_ptr = *(subquery_array + subquery_count);

                        } else {
                            return true;
                        }
                    }
                    character = *char_ptr;
                } else {
                    /*
                     * Move to next section
                     */
                    if( section_shifts < sections) {
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
                        // reset the subquery
                        char_ptr = *(subquery_array + subquery_count);
                        character = *char_ptr;
                        changed_subqueries = true;
                        shifted_char = false;
                    }
                    else{
                        return false;
                    }
                }
            } else {
                // If the char is not the first, we check if it's next to the previous confirmed character
                //Is the match in the right order?
                uint64_t check = (last_sqm * 256) & subquery_matches;
                if (check) {
                    last_sqm = check;

                    //Move to next character
                    char_ptr++;
                    if (*char_ptr == 0) {
                        //Move to next subquery
                        if (*(subquery_array + (subquery_count + 1)) != 0) {
                            subquery_count++;
                            changed_subqueries = true;
                            char_ptr = *(subquery_array + subquery_count);
                            shifted_char = false;

                        } else {
                            return true;
                        }
                    }
                    character = *char_ptr;
                } else {
                    // The character was not next to the previous confirmed character. This is not an auto fail, we check for a variety of conditions before failing or moving on
                    // check for this character in the next section?
                    // check if we changed section, if yes then check if the last character in the previous sqm was in the last position and the current char is in the first position
                    if(changed_section){
                        changed_section = false;
                        if( !(prev_section & 72057594037927936) || !(subquery_matches & 1)){
                            // True mismatch
                            // reset the subquery
                            char_ptr = *(subquery_array + subquery_count);
                            character = *char_ptr;
                            changed_subqueries = true;
                            shifted_char = false;
                        }
                        // No mismatch, move on to the next char
                        else{
                            //set last_sqm as 1
                            last_sqm = 1;
                            //Move to next character
                            char_ptr++;
                            if (*char_ptr == 0) {
                                //Move to next subquery
                                if (*(subquery_array + (subquery_count + 1)) != 0) {
                                    subquery_count++;
                                    changed_subqueries = true;
                                    char_ptr = *(subquery_array + subquery_count);
                                    shifted_char = false;

                                } else {
                                    return true;
                                }
                            }
                            character = *char_ptr;
                        }

                    } else {
                        // Check if last_sqm matched in the first position
                        if(last_sqm & 72057594037927936){
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
                        }
                        // this resets subquery erroniously when second caracter in subquery appears in current section,
                        // should move to next section and check if second character is in first position.
                        else if(section_shifts < sections) {
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
                            // reset the subquery
                            char_ptr = *(subquery_array + subquery_count);
                            character = *char_ptr;
                            changed_subqueries = true;
                            shifted_char = false;
                        }
                        else{
                            return false;
                        }
                    }
                }
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
