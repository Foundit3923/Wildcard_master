//
// Created by Michael Olson on 10/11/2019.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

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

bool Experimental_wildcard_a (uint64_t orig_search_term,
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

    uint64_t search_term = orig_search_term;
    uint64_t subquery_matches = ALL_BITS_ON;
    uint64_t encoded_term = search_term;
    uint64_t last_sqm = 0;


    bool changed_subqueries = true;
    int subquery_count = 0;

    // Points to beginning of array
    char_ptr = subquery_array[subquery_count];
    character = *char_ptr;

    // to account for singlecards '?' we could insert it as a subquery by itself that matches anything and moves the term forwards by one
    while(character != 0) {
        uint64_t char_to_check = character;
        uint64_t char_mask= f_m[char_to_check];
        //uint64_t char_mask = 0;
        /*uint64_t char_mask_1 = ((char_to_check << 8) |
                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
        uint64_t char_mask_2 = ((char_mask_1 << 16) |
                       char_mask_1);                                    // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
        char_mask = ((char_mask_2 << 32) |
                     char_mask_2);                                      // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
                     */
        encoded_term = ~(char_mask ^ search_term);
        subquery_matches = LAST_BITS_ON;

        //Search for matches
        subquery_matches &= encoded_term;
        encoded_term >>= 1;
        subquery_matches &= encoded_term;
        encoded_term >>= 1;
        subquery_matches &= encoded_term;
        encoded_term >>= 1;
        subquery_matches &= encoded_term;
        encoded_term >>= 1;
        subquery_matches &= encoded_term;
        encoded_term >>= 1;
        subquery_matches &= encoded_term;
        encoded_term >>= 1;
        subquery_matches &= encoded_term;
        encoded_term >>= 1;
        subquery_matches &= encoded_term;
        //encoded_term >>= 1;
        if (subquery_matches == 0) {
            /*
             if (*(subquery_array + (subquery_count + 1)) != 0) {
                subquery_count++;
                changed_subqueries = true;
                char_ptr = *(subquery_array + subquery_count);
            } else {
                return false;
            }
             */
            return false;
        }

        if (changed_subqueries) {
            changed_subqueries = false;
            // use last_sqm to find location of previous subquery, mask subquery_matches such that everything up to that point is masked.
            if( last_sqm != 0) {
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
            if(subquery_matches) {
                last_sqm = subquery_matches;
            } else {
                /*
                 * Move to next subquery
                 */
                return false;
            }
        } else {
            //Is the match in the right order?
            uint64_t check = (last_sqm * 256) & subquery_matches;
            if (check) {
                last_sqm = check;
            } else {
                /*

                 */
                return false;
            }
        }

        //Move to next character
        char_ptr++;
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
    }
    return true;
}

