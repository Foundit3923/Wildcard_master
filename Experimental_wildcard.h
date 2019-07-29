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
    bool do_all_subqueries_match = true;
    int bit_count = 0;
    int subquery_char_count = 0;

    // Can i just use the while loop? Introduce a counter to make up for the for loop?
    while( search_term > 0 && do_all_subqueries_match && character != NULL){
        if(bit_count == 0) {
            char_ptr = subquery_array;
            // points to the beginning of the array
            character = *char_ptr;
            //character &= 255;
        }
        if (DEBUG) {
            printf("\t\tTerm: %" PRIu64 "\n", search_term);
            printf("\t\tQuery: %s\n", subquery_array);

        }

        if(character & BYTE_TRAILING_BIT_ON){
            uint64_t temp = search_term;
            if (DEBUG) {
                printf("\t\tTerm bits:                ");
                print_bits(temp);
                printf("\n");
                printf("\t\tCharacter:                ");
                print_bits( character);
                printf("\n");
                printf("\t\tTrailing Bit:             ");
                print_bits(BYTE_TRAILING_BIT_ON);
                printf("\n");
                printf("\t\tCharacter & Trailing bit: ");
                print_bits(character & BYTE_TRAILING_BIT_ON);
                printf("\n");
                printf("\t\tMask bits:                ");
                print_bits(LAST_BITS_ON);
                printf("\n");
                printf("\t\t(term & LAST_BITS_ON):    ");
                print_bits(search_term & LAST_BITS_ON);
                printf("\n");
                printf("\t\tSubquery_matches:         ");
                print_bits(subquery_matches);
                printf("\n");
            }
            subquery_matches &= (search_term & LAST_BITS_ON);
            if(subquery_matches == 0){
                do_all_subqueries_match = false;
            }
        }
        else{
            if (DEBUG) {
                printf("\t\tTerm bits:                ");
                print_bits(search_term);
                printf("\n");
                printf("\t\tCharacter:                ");
                print_bits(character);
                printf("\n");
                printf("\t\tTrailing Bit:             ");
                print_bits(BYTE_TRAILING_BIT_ON);
                printf("\n");
                printf("\t\tCharacter & Trailing bit: ");
                print_bits(character & BYTE_TRAILING_BIT_ON);
                printf("\n");
                printf("\t\tMask bits:                ");
                print_bits(LAST_BITS_OFF);
                printf("\n");
                printf("\t\t~(term | LAST_BITS_OFF):  ");
                print_bits(~(search_term | LAST_BITS_OFF));
                printf("\n");
                printf("\t\tSubquery_matches:         ");
                print_bits(subquery_matches);
                printf("\n");
            }
            subquery_matches &= ~(search_term | LAST_BITS_OFF);
            if(subquery_matches == 0 ){
                do_all_subqueries_match = false;
            }
        }

        if (DEBUG) {
            printf("\t\tNew subquery_matches:     ");
            print_bits(subquery_matches);
            printf("\n");
        }

        search_term >>= 1;
        character >>= 1;
        if (DEBUG) printf("\t\t\n");

        if(bit_count < 8) {
            bit_count++;
        }
        else{
            char_ptr++;
            character = *char_ptr;
            if (DEBUG) printf("\t}\n");

            // now check subquery matches
            // already took care of checking anchored characters in preprocessing

            // check if character exists in term
            for( int i = 1; i < 8; i++){
                if( subquery_matches & TRAILING_BIT_ON){
                        subquery_matches = ALL_BITS_ON;
                }
                else{
                    subquery_matches >>= 8;
                    search_term >>= 8;
                }
            }
            bit_count = 0;

        }

    }

    return do_all_subqueries_match;

}
