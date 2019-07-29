//
// Created by Michael Olson on 7/25/2019.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

// This might be great for splitting up queries across many cores, doing checks asyncronously?

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


// Explore idea of array of term subsections, where any term less than 8 chars is stored in a single subsection
// Anchored chars are checked in preprocessing.
bool new_wildcard (uint64_t search_term,
                   char* subquery_array) {
    //-------------//
    //Preprocessing//
    //-------------//
    /* Done externally */

    //----------//
    //Processing//
    //----------//

    //
    char character;
    char* char_ptr;
    uint64_t subquery_matches = ALL_BITS_ON;
    bool do_all_subqueries_match;
    int bit_count = 0;
    int subquery_char_count = 0;

    // Can i just use the while loop? Introduce a counter to make up for the for loop?
    while( search_term > 0 && do_all_subqueries_match && character != NULL){
        if(bit_count == 0) {
            char_ptr = subquery_array;
            // points to the beginning of the array
            character = *char_ptr;
        }


        if(character & BYTE_TRAILING_BIT_ON){
            if((subquery_matches &= (term & LAST_BITS_ON)) == 0){
                do_all_subqueries_match = false;
            }
        }
        else{
            if((subquery_matches &= (term & LAST_BITS_OFF)) == 0 ){
                do_all_subqueries_match = false;
            }
        }

        search_term >>= 1;
        character >>= 1;

        if(bit_count < 8) {
            bit_count++;
        }
        else{
            char_ptr++;
            character = *char_ptr;

            // now check subquery matches
            // already took care of checking anchored characters in preprocessing

            bit_count = 0;
        }

    }

    return do_all_subqueries_match;

}
