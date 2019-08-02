//
// Created by Michael Olson on 7/25/2019.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

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

// https://stackoverflow.com/a/700184
void print_bits_u(uint64_t num) {
    int i = (int) malloc(sizeof(int)); // for C89 compatability
    for (i=sizeof(num)*8-1; i>=0; i--)
    {
        putchar(((num >> i) & 1) ? '1': '0');
    }
    free(i);
    i = NULL;

}

void print_bits(uint64_t num) {
    int i;
    char p[64];
    char* s;
    int check = num & 1;
    if( num & 1){
        p[0] = '1';
    }
    else{
        p[0] = '0';
    }
    for( i = 1; i < 64; i++){
       // printf("%i", p);

        if(((num >>= 1) & 1)){
            p[i] = '1';
        }
        else{
            p[i] = '0';
        }
    }
    s = &p[0];
    printf("%s", s);
}


// Explore idea of array of term subsections, where any term less than 8 chars is stored in a single subsection
// Anchored chars are checked in preprocessing.
bool Experimental_wildcard (uint64_t search_term,
                            char* subquery_array) {
    //-------------//
    //Preprocessing//
    //-------------//
    /* Done externally */

    //----------//
    //Processing//
    //----------//

    //
    char character = 's';
    char* char_ptr;
    uint64_t subquery_matches = ALL_BITS_ON;
    uint64_t term = 0;
    bool do_all_subqueries_match = true;
    int bit_count = 0;
    int subquery_char_count = 0;
    char_ptr = subquery_array;
    character = *char_ptr;


    // Can i just use the while loop? Introduce a counter to make up for the for loop?
    while( search_term > 0 && subquery_matches != 0){//character != NULL){
        if(bit_count == 0) {

            // points to the beginning of the array
            uint64_t char_to_check = character;
            uint64_t char_mask_1;
            uint64_t char_mask_2;
            uint64_t char_mask;

            // make character mask to test term against
            char_mask_1 = ((char_to_check << 8) | char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
            char_mask_2 = ((char_mask_1 << 16) | char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
            char_mask = ((char_mask_2 << 32) | char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

            //add functionality for larger system word sizes


            // Any match will be 11111111
            uint64_t term_check = (char_mask ^ search_term);
            term = ~(char_mask ^ search_term);
            // We check each byte of term one bit at a time
            // begins as 00000001 x 8
            subquery_matches = LAST_BITS_ON;
        }

        // subquery_matches updates the single active bit for each bit in the byte.
        // the updated subquery_matches acts as the filter each time.
        // only those bytes that == 11111111 will remain on the entire time.
        subquery_matches &= term;

        // if subquery_matches reaches 0 the loop is exited and false is returned.
        if(subquery_matches == 0){
            do_all_subqueries_match = false;
        }
        term >>= 1;

        // increase bit_count for each loop.
        if(bit_count <= 6) {
            bit_count++;
        }
        // all bits have been checked for the character.
        else{
            // update the chanacter being checked
            char_ptr++;
            character = *char_ptr;
            if (DEBUG) printf("\t}\n");

            // now check subquery matches
            // already took care of checking anchored characters in preprocessing
            uint64_t checker;
            uint64_t mask = 1;
            int shift = 8;
            int counter = 0;

            // should this be used to make sure chars are appearing next to each other? i.e. & with 1.
            if(subquery_matches & mask){
                search_term >>= shift;
            }
            else {
                for (counter = 2; counter < 8; counter++) {
                    if (subquery_matches & (mask * counter)) {
                        search_term >>= (shift * counter);
                        counter = 8;
                    }
                }
            }
            // reset bit_count for next char
            bit_count = 0;
        }

    }

    return do_all_subqueries_match;

}
