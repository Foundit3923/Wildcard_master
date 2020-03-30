//
// Created by Michael Olson on 2/11/2020.
// aims to improve on arbitrary_length_wildcard.h by implementing bulk string checks while bypassing the need to encode and check each character.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

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

bool KMP_Experimental_wildcard_arbitrary_length (char st[],
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
    int size = 0;

    uint64_t search_term = 0;
    int j;
    for (j = 7; j >= 0; --j) {
        search_term <<= 8;
        search_term |= (uint64_t) st[ j + text_modifier];
    }

    uint64_t subquery_matches = ALL_BITS_ON;
    uint64_t encoded_term = search_term;
    uint64_t last_sqm = 0;
    uint64_t prev_section = 0;


    bool changed_subqueries = true;
    bool changed_section = false;
    bool shifted_char = false;
    bool skip_encoding = false;
    bool forever_text = false;
    int subquery_count = 0;

    // Points to beginning of array
    char_ptr = subquery_array[subquery_count];
    character = *char_ptr;

    // to account for singlecards '?' we could insert it as a subquery by itself that matches anything and moves the term forwards by one
    while(character != 0 && st[text_modifier ] != 0) {
        uint64_t char_to_check = character;
        uint64_t char_mask = f_m[ char_to_check ];
        encoded_term = ~(char_mask ^ search_term);
        subquery_matches = LAST_BITS_ON;

        //Search for matches
        int match_count =0 ;
        for( match_count = 0; match_count < 8; match_count++){
            subquery_matches &= encoded_term;
            encoded_term >>= 1;
        }/*
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
        subquery_matches &= encoded_term;*/
        if (subquery_matches == 0) {
            // if text_modifer + 8 < length of text then we should be good to go.
            //Fix following if statement to reflect above thoughts

            //for infinite text, make sure we don't set the length too short
            if(forever_text){
                text_len = text_modifier + 1;
            }

            if(text_modifier < text_len) {
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
            //There is a match, have we changed subqueries?
            if (changed_subqueries) {
                changed_section = false;
                changed_subqueries = false;
                int location = 0;
                // use last_sqm to find location of previous subquery, mask subquery_matches such that everything up to that point is masked.
                // need to find a way to not hardcode these values.** (int)sizeof(long)*CHAR_BIT
                //Need to find the position of the first
               // if (last_sqm != 0) {
               last_sqm = subquery_matches;
                    if (last_sqm & 16843009) {
                        if (last_sqm & 257) {
                            if (last_sqm & 1) {
                                subquery_matches &= 18446744073709551360;
                                location =0;
                            } else {
                                subquery_matches &= 18446744073709486080;
                                location = 1;
                            }
                        } else {
                            if (last_sqm & 65536) {
                                subquery_matches &= 18446744073692774400;
                                location = 2;
                            } else {
                                subquery_matches &= 18446744069414584320;
                                location = 3;
                            }
                        }
                    } else {
                        if (last_sqm & 1103806595072) {
                            if (last_sqm & 4294967296) {
                                subquery_matches &= 18446742974197923840;
                                location = 4;
                            } else {
                                subquery_matches &= 18446462598732840960;
                                location = 5;
                            }
                        } else {
                            if (last_sqm & 281474976710656) {
                                subquery_matches &= 18374686479671623680;
                                location = 6;
                            } else {
                                subquery_matches &= 0;
                                location = 7;
                            }
                        }
                    }
                    last_sqm = subquery_matches;
                //}
                //Are there still matches?
                if (subquery_matches) {
                    last_sqm = subquery_matches;

                    //Instead of moving to the next character, we want to find the position of the first character and compare the subquery to the string at that point.
                    //All we should ned to do at this point is to line up the subquery at the right point and trim it if it extends past the end of the text segment.
                    //If the subquery extends past the end of the subquery, we trim it, save it, and check it if the first portion of the subquery match the text segment.
                    //We mark the last position of the query in the text and move to the next subquery, making sure that there is no overlap.
                    //Have to search through this separately, either at the time of creation with an associated array or count here.


                    //Should be able to do this without size?
                    //While loop until char == 0?

                    //Check if the rest of the subquery matches
                    int i = 0;
                    //Identify where first subquery char is
                    char null_check = subquery_array[subquery_count][i];
                        while(null_check != 0 ) {
                            //Does the character match?
                            //Doesn't currently start at match point
                            if (!(subquery_array[ subquery_count ][ i ] == st[ location + text_modifier ])) {
                                //No :(
                                // move to next available character
                                if (last_sqm & 16843009) {
                                    if (last_sqm & 257) {
                                        if (last_sqm & 1) {
                                            subquery_matches &= 18446744073709551360;
                                            location = 1;
                                        } else {
                                            subquery_matches &= 18446744073709486080;
                                            location = 2;
                                        }
                                    } else {
                                        if (last_sqm & 65536) {
                                            subquery_matches &= 18446744073692774400;
                                            location = 3;
                                        } else {
                                            subquery_matches &= 18446744069414584320;
                                            location = 4;
                                        }
                                    }
                                } else {
                                    if (last_sqm & 1103806595072) {
                                        if (last_sqm & 4294967296) {
                                            subquery_matches &= 18446742974197923840;
                                            location = 5;
                                        } else {
                                            subquery_matches &= 18446462598732840960;
                                            location = 6;
                                        }
                                    } else {
                                        if (last_sqm & 281474976710656) {
                                            subquery_matches &= 18374686479671623680;
                                            location = 7;
                                        } else {
                                            subquery_matches &= 0;
                                            location = 8;
                                        }
                                    }
                                }
                                //If we run out of instances
                                if (subquery_matches == 0) {
                                    //Move to the next section +8
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
                                } else {
                                    //Matched! now we need to move the window to the next location
                                    text_modifier = text_modifier + i + 1;
                                    changed_section = true;
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
                            }
                            //Yes! If there's another character check for it!
                            i++;
                            null_check = subquery_array[subquery_count][i];
                        }
                        //return true if while fails
                        if(subquery_array[subquery_count][i] == 0){
                            return true;
                        }

                    //calculate how far past the window you have gone. increase text_modifier to reflect this change
                    //This indicates the possiblity of only encoding for the first character in a subquery
                    //Possibility for multiple encodings, but i think we could bypass that.
                    //Since we have moved out of the window, we must advance text_modifier by at leasat 8, then an
                    // additional number of times described by the location variable
                    //
                    //When we do this we create a situation wherein we don't necessarily increase by 8 each time
                    //instead we have the ability to increase by 8 + (m-1), vastly increasing our step range while
                    //minimizing the amount of processing.
                    text_modifier += (8+ (size-location));

                    //Move to next character

                    //Move to next subquery if there is another
                     if (*(subquery_array + (subquery_count + 1)) != 0) {
                         subquery_count++;
                         changed_subqueries = true;
                         char_ptr = *(subquery_array + subquery_count);
                         shifted_char = false;
                     } else {
                         return true;
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
            }
            //We are in the same subquery
            else {
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
                    // check if we changed section, if yes then check if the last character in the previous sqm was in the last position and the current char is in the first position
                    if(changed_section){
                        changed_section = false;
                        if( !(prev_section & 72057594037927936) || !(subquery_matches & 1)){
                            // reset the subquery
                            char_ptr = *(subquery_array + subquery_count);
                            character = *char_ptr;
                            changed_subqueries = true;
                            shifted_char = false;
                        } else{
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
    if(st[text_modifier] == 0){
        return false;
    } else {
        return true;
    }
}