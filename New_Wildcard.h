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


// Eplore idea of array of term subsections, where any term less than 8 chars is stored in a single subsection
bool new_wildcard (uint64_t search_term,
                   char* subquery_array,
                   bool anchored_beginning,
                   bool anchored_end,
                   bool first_subquery,
                   bool last_subquery) {
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
    bool all_subqueries_match;

    // Can i just use the while loop? Introduce a counter to make up for the for loop?
    while( search_term > 0 && all_subqueries_match && character != NULL){

        char_ptr = subquery_array[subquery_num];
        character = *char_ptr;

        for (int i=0; i<BYTE_LENGTH; i++){
            if(character & BYTE_TRAILING_BIT_ON){
                subquery_matches &= (term & LAST_BITS_ON);
            }
            else{
                subquery_matches &= (term & LAST_BITS_OFF);
            }

            search_term >>= 1;
            character >>= 1;
        }




    }


}
