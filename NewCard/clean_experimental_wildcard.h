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
#define SIZE 255

clock_t loop_clock = 0;

clock_t main_loop_clock = 0;

clock_t init_clock = 0;

clock_t all_clock = 0;


//uint64_t anchor_check[8] = {255,65280,16711680,4278190080,1095216660480,280375465082880,71776119061217280,18374686479671624000};


struct DataItem {
    uint64_t data;
    int key;
};

struct DataItem* hashArray[SIZE];
struct DataItem* query_64;
struct DataItem* full_masks;
struct DataItem* item;
struct DataItem* dummyItem;

int hashCode(int key){
    return key % SIZE;
}

struct DataItem *search(int key){
    //get the hash
    int hashIndex = hashCode(key);

    //move in array until an empty
    while(hashArray[hashIndex] != NULL) {
        if(hashArray[hashIndex]->key == key)
            return hashArray[hashIndex];

        //go to next cell
        ++hashIndex;

        //wrap around the table
        hashIndex %= SIZE;
    }
    return NULL;
}

void insert(int key, uint64_t data){
    struct DataItem *item = (struct DataItem*) malloc(sizeof(struct DataItem));
    item->data = data;
    item->key = key;

    //get the hash
    int hashIndex = hashCode(key);

    //move in array until an empty or deleted cell
    while(hashArray[hashIndex] != NULL && hashArray[hashIndex]->key != -1){
        //go to next cell
        ++hashIndex;

        //wrap around the table
        hashIndex %= SIZE;
    }

    hashArray[hashIndex] = item;
}

struct DataItem* delete(struct DataItem *item){
    int key = item->key;

    //get the hash
    int hashIndex = hashCode(key);

    //move in array until an empty
    while(hashArray[hashIndex] != NULL){
        if(hashArray[hashIndex]->key == key){
            struct DataItem* temp = hashArray[hashIndex];

            //assign dummy item at deleted position
            hashArray[hashIndex] = dummyItem;
            return temp;
        }

        //go to next cell
        ++hashIndex;

        //wrap around the table
        hashIndex %= SIZE;
    }
    return NULL;
}

void display(){
    int i = 0;

    for(i = 0; i<SIZE; i++) {

        if(hashArray[i] != NULL)
            printf(" (%d,%" PRIu64 ")",hashArray[i]->key,hashArray[i]->data);
        else
            printf(" ~~ ");
    }

    printf("\n");
}

// https://stackoverflow.com/a/700184
void print_bits(uint64_t num) {
    int i = (int) malloc(sizeof(int)); // for C89 compatability
    int eight_count = 0;
    for (i=sizeof(num)*8-1; i>=0; i--)
    {
        if( eight_count == 8 || eight_count == 8*2 || eight_count == 8*3 || eight_count == 8*4 || eight_count == 8*5 || eight_count == 8*6 || eight_count == 8*7 ){
            putchar(' ');
        }
        putchar(((num >> i) & 1) ? '1': '0');
        eight_count++;
    }
    //   free(i);
    i = NULL;

}



// Explore idea of array of term subsections, where any term less than 8 chars is stored in a single subsection
// Anchored chars are checked in preprocessing.
bool Experimental_wildcard (uint64_t orig_search_term,
                            char** subquery_array) {
    //-------------//
    //Preprocessing//
    //-------------//
    /* Done externally */

    //----------//
    //Processing//
    //----------//

    //
    //all_clock = clock();
    //init_clock = clock();
    char character = 's';
    char* char_ptr;
    uint64_t search_term = orig_search_term;
    uint64_t subquery_matches = ALL_BITS_ON;
    uint64_t term = search_term;
    uint64_t last_match = 0;
    uint64_t copy_term;
    uint64_t last_sqm;

    int query_length = search(511)->data;
    int term_length = search(512)->data;
    int check_term_length = term_length;

    bool do_all_subqueries_match = true;
    bool changed_subs = false;
    bool one_left = false;
    int bit_count = 0;
    int subquery_count = 0;
    int location = 0;
    int shift_length = 0;

    // Points to beginning of array
    char_ptr = subquery_array[subquery_count];
    character = *char_ptr;

    //clock_t start;
    //init_clock = ((double) (clock() - init_clock)) * CLOCKS_PER_SEC;

    while( (search_term > 0 || subquery_matches >= 1 ) && character != 0){


        if(bit_count == 0) {

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


        // it occurs to me that once there is only one active bit in subquery_matches we could just check that byte
        // might save some processing time


        // if subquery_matches reaches 0 the loop is exited and false is returned.
        if(subquery_matches == 0){
            if(last_match > 0){

                //if there is another instance of the first char in the term then we need to start at that char
                subquery_matches = last_match;
                int found = 0;
                while( found <= 1 && subquery_matches > 0) {
                    // first we find it
                    if (subquery_matches & 16843009) {
                        if (subquery_matches & 257) {
                            if (subquery_matches & 1) {
                                term_length = check_term_length - 1;
                                shift_length = 0;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= 1;
                                }


                            } else if (subquery_matches & 256) {
                                term_length = check_term_length - 2;
                                shift_length = 8;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= 256;
                                }
                            }
                        } else {
                            if (subquery_matches & 65536) {
                                term_length = check_term_length - 3;
                                shift_length = 16;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= 65536;
                                }
                            } else {
                                term_length = check_term_length - 4;
                                shift_length = 24;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= (65536 << 8);
                                }
                            }
                        }
                    } else {

                        if (subquery_matches & 1103806595072) {
                            if (subquery_matches & 4294967296) {
                                term_length = check_term_length - 5;
                                shift_length = 32;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= 4294967296;
                                }
                            } else if (subquery_matches & 1099511627776) {
                                term_length = check_term_length - 6;
                                shift_length = 40;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= 1099511627776;
                                }
                            }
                        } else {
                            if (subquery_matches & 281474976710656) {
                                term_length = check_term_length - 7;
                                shift_length = 48;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= 281474976710656;
                                }
                            } else {
                                term_length = check_term_length - 8;
                                shift_length = 56;
                                found++;
                                if( found == 1){
                                    subquery_matches ^= (281474976710656 << 8);
                                }
                            }
                        }
                    }
                }


                //search_term >>= shift_length - ((check_term_length - term_length) * 8);
                search_term = orig_search_term;
                search_term >>= shift_length;
                char_ptr = subquery_array[ subquery_count ];
                character = *char_ptr;
                last_match = 0;
                bit_count = -1;
            }
            else {
                return false;
            }
        }
        else {
            term >>= 1;
            search_term >>= 1;
            if(search_term == 0 && bit_count == 6 && subquery_matches >= 1){
                *char_ptr = 0;
            }
        }
        last_sqm = subquery_matches;
        // increase bit_count for each loop.
        if(bit_count <= 6 ) {
            bit_count++;

        }
            // all bits have been checked for the character.
        else {

            query_length--;

            // More questions than answers
            if (query_length > term_length) {
                return false;
            }

            else {
                // make sure to get the last bit in the sequence
                //term >>= 1;
                //main_loop_clock += ((double) (clock()-main_loop_clock)) * CLOCKS_PER_SEC;
                // move to next char in subquery
                char_ptr++;
                //Update subquery

                // At this point we are checking if there is another subquery, then checks if the current subquery == 0
                // if the current subquery == 0 then the end has been reached and we shoudl move to the next subqquery
                // the char_ptr is then set to the beginning of the next subquery.
                //
                // In future iterations we will only search for the first char in a subquery, all other chars will be
                // searched for as if they are fixed chars.
                // in query te*m, this causes e to be skipped


                // if current subquery is finished
                if (*char_ptr == 0) {
                    // if next subquery
                    if (*(subquery_array + (subquery_count + 1)) != 0) {
                        // move to next subquery
                        subquery_count++;
                        char_ptr = *(subquery_array + subquery_count);
                        changed_subs = true;
                        last_match = 0;
                    }
                        // if not 0, then is varaibly fixed char, get mask from hashtable and check.
                    else {

                    }
                }

                // Only set the character variable to *char_ptr when there are chars left in subqueries
                // this should only happen when we are at the end of the last subquery. This shortciruits
                //if( *char_ptr != 0 ) {
                character = *char_ptr;

                // now check subquery matches
                // already took care of checking anchored characters in preprocessing

                int counter;

                // should this be used to make sure chars are appearing next to each other? i.e. & with 1.
                // I think this section is killing our times.

                // the first char has been shifted out. this is ok because that char was either the one we are looking for,
                // or needed to be removed anyways
                //loop_clock = clock();
                // this if statement reduces possible variations by half
                // shoudl this be replaced by a numerical comparison? How to do numerical comparison?
                int shift_length = 0;


                // check if this char was preceeded by another char
                if (last_match != 0) {
                    // is it in the right position?
                    if (subquery_matches & 1) {
                        if (character != 0) {
                            shift_length = 0;
                            term_length -= 1;
                            // if the term_length is ever too small to meet the requirements of the query, return false
                            if (query_length > term_length && term_length >= 0 && character != 0) {
                                return false;
                            }
                            last_match = subquery_matches;
                            subquery_matches >>= shift_length + 8;
                        }


                    } else {
                        // we reached a point in the term where the pattern no longer matches, before we go back to the
                        // beginning of the pattern let's check if the first uniquery in the subquery exists elsewhere
                        // in the term
                        if (last_match > 0) {

                            //if there is another instance of the first char in the term then we need to start at that char

                            // first we find it
                            // Check subquery_matchs directly for specific hits
                            // first check if in right half
                            if (subquery_matches & 16843009) {
                                if (subquery_matches & 257) {
                                    if (subquery_matches & 1) {
                                        term_length -= 1;
                                        shift_length = 0;
                                    } else {
                                        term_length -= 2;
                                        shift_length = 8;
                                    }
                                } else {
                                    if (subquery_matches & 65536) {
                                        term_length -= 3;
                                        shift_length = 16;
                                    } else {
                                        term_length -= 4;
                                        shift_length = 24;
                                    }
                                }
                            }
                                // Then in left half
                            else {

                                if (subquery_matches & 1103806595072) {
                                    if (subquery_matches & 4294967296) {
                                        term_length -= 5;
                                        shift_length = 32;
                                    } else {
                                        term_length -= 6;
                                        shift_length = 40;
                                    }
                                } else {
                                    if (subquery_matches & 281474976710656) {
                                        term_length -= 7;
                                        shift_length = 48;
                                    } else {
                                        term_length -= 8;
                                        shift_length = 56;
                                    }
                                }
                            }
                            //this needs optimized
                            //search_term >>= shift_length - ((check_term_length - term_length) * 8);
                            search_term = orig_search_term;
                            search_term >>= shift_length;
                            term_length = check_term_length - (shift_length/ 8);
                            int old_len = strlen(char_ptr);
                            char_ptr = subquery_array[ subquery_count ];
                            character = *char_ptr;
                            last_match = 0;
                            query_length += ( strlen(char_ptr) - old_len);
                        } else {
                            return false;
                        }
                    }
                } else {
                    if (subquery_matches & 16843009) {
                        if (subquery_matches & 257) {
                            if (subquery_matches & 1) {
                                term_length -= 1;
                                shift_length = 0;
                            } else {
                                term_length -= 2;
                                shift_length = 8;
                            }
                        } else {
                            if (subquery_matches & 65536) {
                                term_length -= 3;
                                shift_length = 16;
                            } else {
                                term_length -= 4;
                                shift_length = 24;
                            }
                        }
                    } else {

                        if (subquery_matches & 1103806595072) {
                            if (subquery_matches & 4294967296) {
                                term_length -= 5;
                                shift_length = 32;
                            } else {
                                term_length -= 6;
                                shift_length = 40;
                            }
                        } else {
                            if (subquery_matches & 281474976710656) {
                                term_length -= 7;
                                shift_length = 48;
                            } else {
                                term_length -= 8;
                                shift_length = 56;
                            }
                        }
                    }

                    // if the term_length is ever too small to meet the requirements of the query, return false
                    if (query_length > term_length && term_length >= 0 && character != 0) {
                        return false;
                    }
                    if (character != 0) {

                        search_term >>= shift_length;
                        subquery_matches >>= shift_length + 8;
                        last_match = subquery_matches;
                    }
                }

                //clock_t loop_end = clock();
                //loop_clock += ((double) (loop_end - loop_clock)) * CLOCKS_PER_SEC;
                // this should be ok for the last iteration because the loop breaks when character == 0
                if (changed_subs) {
                    last_match = 0;
                    changed_subs = false;
                }
                bit_count = 0;
            }
        }
        //decode term


    }
    if(search_term == 0 && character != 0){
        return false;
    }

    //free(char_ptr);
    //all_clock = ((double) (clock() - all_clock)) * CLOCKS_PER_SEC;
    return do_all_subqueries_match;

}
