#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include "hashtable.h"

#define ALL_BITS_ON 18446744073709551615   // 11111111 repeated 8 times
#define ALL_BITS_OFF 0                     // 00000000 repeated 8 times
#define LAST_BITS_ON 72340172838076673     // 00000001 repeated 8 times
#define LAST_BITS_OFF 18374403900871474942 // 11111110 repeated 8 times
#define TRAILING_BIT_ON 1  // 00000000...64 bits...00000001

// More like config constants
#define MAX_TERMS (sizeof(uint64_t)) // Should be 8
#define DELIMITER "*" // The wild card itself
#define DEBUG false



int* find_location(uint64_t subquery_matches){
    int* result = (int*) malloc(sizeof(int));
    bool one_left = false;
    int shift_length = 0;
    int location = 0;
    // first we find it
    if (subquery_matches & 16843009) {
        if (subquery_matches & 257) {
            if (subquery_matches & 1) {
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }

            } else if (subquery_matches & 256) {
                shift_length = 8;
                location = 1;
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }

            }
        } else {
            if (subquery_matches & 65536) {
                shift_length = 16;
                location = 2;
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }
            } else {
                shift_length = 24;
                location = 3;
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }
            }
        }
    } else {

        if (subquery_matches & 1103806595072) {
            if (subquery_matches & 4294967296) {
                shift_length = 32;
                location = 4;
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }
            } else if (subquery_matches & 1099511627776) {
                shift_length = 40;
                location = 5;
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }
            }
        } else {
            if (subquery_matches & 281474976710656) {
                shift_length = 48;
                location = 6;
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }
            } else {
                shift_length = 56;
                location = 7;
                if ((~(subquery_matches) ^ (subquery_matches & anchor_check[ location ])) == ALL_BITS_ON) {
                    one_left = true;
                }
            }
        }
    }
    result[0] = shift_length;
    result[1] = location;
    if(one_left) {
        result[ 2 ] = 1;
    } else{
        result[2] = 0;
    }
    return result;

}

bool Experimental_wildcard (uint64_t orig_search_term,
                            char** subquery_array,
                            struct DataItem query_64){
    int position = 0;
    int location = 0;
    int bit_count = 0;
    int shift_length;
    int* found;
    int subquery_count = 0;
    int query_length = search(135)->data;
    int term_length = search(136)->data;
    int check_term_length = term_length;

    char* char_ptr;
    char character;

    bool changed_subs = false;
    bool all_match = true;

    uint64_t term;
    uint64_t copy_term;
    uint64_t search_term = orig_search_term;
    uint64_t subquery_matches;
    uint64_t last_sq_match;

    char_ptr = subquery_array[subquery_count];
    character = *char_ptr;

    // do until only one remaining in mask or end of byte
    // only searching for the first char in a subquery
    do{
        if(bit_count == 0) {
            //main_loop_clock = clock();

            // retrieves full mask for current character
            uint64_t char_mask = search(character)->data;

            // XNOR comparison between character full mask and term, results in any character match appearing as 1111111
            term = ~(char_mask ^ search_term);
            copy_term = term;
            //term = ~term;

            // We check each byte of term one bit at a time
            // begins as 00000001 x 8
            subquery_matches = LAST_BITS_ON;
        }

        // subquery_matches updates the single active bit for each bit in the byte.
        // the updated subquery_matches acts as the filter each time.
        // only those bytes that == 11111111 will remain on the entire time.
        subquery_matches &= term;
        term >>= 1;
        search_term >>= 1;
        if(subquery_matches == 0){
            // if the first uniquery isn't there we can quit
            return false;
        }
        // let's make sure we aren't redoing actions when the results will be the same
        else if(last_sq_match != subquery_matches) {

            // check if there's only one bit remaining, also get shift_length and the location of the bit
            found = find_location(subquery_matches);

            // if one bit left
            if (found[ 2 ]) {
                // find location
                shift_length = found[ 0 ];
                location = found[ 1 ];
                // if selected byte is active
                if ((copy_term & anchor_check[ location ]) == anchor_check[ location ]) {
                    // move the term the rest of the way over
                    term >>= (8 - (bit_count + 1));

                    //move search_term the rest of the way over
                    search_term >>= (8 - (bit_count + 1));
                    search_term >>= shift_length;
                    // move to next uniquery
                    char_ptr++;
                    // if current subquery is finished
                    if (*char_ptr == 0) {
                        // if next subquery
                        if (*(subquery_array + (subquery_count + 1)) != 0) {
                            // move to next subquery
                            subquery_count++;
                            char_ptr = *(subquery_array + subquery_count);
                            changed_subs = true;
                            bit_count = -1;
                            character = *char_ptr;
                        } else{
                            break;
                        }
                    } else {
                        // we don't need the rest of the subquery individually
                        uint64_t test_query;
                        int j;
                        test_query = 0;
                        for (j = 7; j >= 0; --j) {
                            test_query <<= 8;
                            test_query |= (uint64_t) char_ptr[ j ];
                        }
                        // check if rest of subquery matches
                        if ((test_query & search_term) == test_query) {
                            if (*(subquery_array + (subquery_count + 1)) != 0) {
                                // move to next subquery
                                subquery_count++;
                                char_ptr = *(subquery_array + subquery_count);
                                changed_subs = true;
                                bit_count = -1;
                                character = *char_ptr;
                            } else{
                                return true;
                            }
                            // MATCH
                            // check for next subquery
                            // if no next subquery then return true
                            // else

                        } else{

                        }
                    } /*else{
                        character = *char_ptr;
                        bit_count = -1;
                    }*/

                } else {
                    return false;
                }
            }
            // check if we have finished the byte
            else if( bit_count == 6){
                // the byte is finished
                // this means there are more than one instance of the char in the term
                // shift to the location of the first instance

                // move the term the rest of the way over
                term >>= (8 - (bit_count + 1));

                //move search_term the rest of the way over
                search_term >>= (8 - (bit_count + 1));
                search_term >>= shift_length;
            }
        }

        bit_count++;
        last_sq_match = subquery_matches;
    }while(subquery_matches != 0 && bit_count != 7);


    //find position of first char
    return all_match;

}