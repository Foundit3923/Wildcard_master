//
// Created by Michael Olson and Daniel Davis on 07/05/2020.
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

// Assumes little endian

//---------//
// GLOBALS //
//---------//

#define LAST_BITS_ON 72340172838076673     // 00000001 repeated 8 times

uint64_t anchor_check[8] = {255,65280,16711680,4278190080,1095216660480,280375465082880,71776119061217280,18374686479671624000};

uint64_t length_mask[7] = {0xFF,0xFFFF,0xFFFFFF,0xFFFFFFFF,0xFFFFFFFFFF,0xFFFFFFFFFFFF,0xFFFFFFFFFFFFFF};

union Window {
    uint64_t* i;
    char* c;
}Window;

union Mask {
    uint64_t* i;
    char* c;
}Mask;

bool Experimental_wildcard_arbitrary_length_moving_union_save (char st[],
                                                               char** subquery_array,
                                                               uint64_t f_m[]) {
    //----------//
    //Processing//
    //----------//

    char* char_ptr;

    // As text_modifier increases by 8 we move to the next section of the text
    int text_modifier = 0;
    int text_len = strlen(st);
    int count = 0;
    int match_count = 0;
    int char_count = 0;
    int move_value = 0;
    bool skip = false;
    bool status = true;

    union Window t_w;
    t_w.c = &st[text_modifier];

    union Mask s_m;

    uint64_t subquery_matches = LAST_BITS_ON;
    uint64_t encoded_term;

    int subquery_count = 0;

    // Points to beginning of array
    char_ptr = subquery_array[subquery_count];
    s_m.c = &char_ptr[1];

    while(char_ptr != NULL && text_len >= text_modifier) {
        if(!skip) {
            subquery_matches = LAST_BITS_ON;
            //find first char in subquery
            encoded_term = ~(f_m[*char_ptr] ^ *t_w.i);
            //Search for first char matches
            match_count = 0;
            while (subquery_matches && match_count < 8){
                subquery_matches &= encoded_term;
                encoded_term >>= 1;
                match_count++;
            }
        }
        if(subquery_matches){
            skip = false;
            move_value = 0;
            //check if another char in subquery
            int position = 0;
            if(*s_m.c != '\000') {
                //line up text window
                if (subquery_matches & 16843009) {
                    if (subquery_matches & 257) {
                        if (subquery_matches & 1) {
                            position = 1;
                        } else {
                            position = 2;
                        }
                    } else {
                        if (subquery_matches & 65536) {
                            position = 3;
                        } else {
                            position = 4;
                        }
                    }
                } else {
                    if (subquery_matches & 1103806595072) {
                        if (subquery_matches & 4294967296) {
                            position = 5;
                        } else {
                            position = 6;
                        }
                    } else {
                        if (subquery_matches & 281474976710656) {
                            position = 7;
                        } else {
                            position = 8;
                        }
                    }
                }
                text_modifier += position;
                t_w.c = &st[text_modifier];
                //check for rest of subquery

                //if the subquery is longer than the window then we must move the window and keep checking
                //also influences how we check

                int subquery_len = strlen(s_m.c)-1;
                //doubles as a way to move through the text quickly
                for (char_count = subquery_len; char_count >= 0; char_count -= 8) {
                    //XNOR window and text
                    encoded_term = ~(*s_m.i ^ *t_w.i);
                    if (char_count >= 8) {
                        if (encoded_term == 0xFFFFFFFFFFFFFFFF) {
                            //this should always match because the whole integer should be set
                            //move everything by 8
                            move_value += 8;
                            s_m.c += move_value;
                            subquery_count++;
                            t_w.c = &st[text_modifier + move_value];
                        } else {
                            //if there isn't a match we reset
                            s_m.c = &char_ptr[1];
                            status = false;
                            break;
                        }
                    } else {
                        if ( (encoded_term & length_mask[char_count]) == length_mask[char_count]) {
                            //this should match because only the bytes that
                            //move to end
                            move_value += char_count+1;
                            subquery_count++;
                            char_ptr = subquery_array[subquery_count];
                            s_m.c = &char_ptr[1];
                            t_w.c = &st[text_modifier + move_value];
                        } else {
                            s_m.c = &char_ptr[1];
                            status = false;
                            break;
                        }
                    }
                }
                //when we finish with this section, we have 3 options.
                //1. move to next match in subquery_matches
                subquery_matches = ~(~(subquery_matches) ^ (subquery_matches & anchor_check[position-1]));
                if( subquery_matches){
                    //there was more than one match and now we have it
                    if(!status){
                        //restart this section
                        skip = true;
                        text_modifier -= position;
                    } else {
                        text_modifier += move_value;
                        char_ptr = subquery_array[subquery_count];
                    }
                }
                //2. there are no more matches in subquery_matches so we shift by 8 and try again
                else {
                    //there was only one match
                    //we've either moved to a new area
                    if(status){
                        text_modifier += move_value;
                        char_ptr = subquery_array[subquery_count];
                        s_m.c = &char_ptr[1];
                    } else {
                        // The window has been completely searched, time to move
                        text_modifier -= position;
                        text_modifier += 8;
                        // if we still have room left to shift
                        if (text_modifier < text_len) {
                            // Move window
                            t_w.c = &st[text_modifier];
                        } else {
                            return false;
                        }
                    }
                }
            } else {
                text_modifier -= position;
                subquery_count++;
                char_ptr = subquery_array[subquery_count];
                text_modifier++;
                t_w.c = &st[text_modifier];
                s_m.c = &char_ptr[1];
            }
        }
        //3. everything matched and we've moved forward in the text and are ready to look for the next subquery
        else{
            // The window has been completely searched, time to move
            text_modifier += 8;
            // if we still have room left to shift
            if (text_modifier < text_len) {
                // Move window and reset subquery_matches
                t_w.c = &st[text_modifier];
                subquery_matches = LAST_BITS_ON;
            } else {
                return false;
            }
        }
    }
    if(text_len >= text_modifier){
        return true;
    } else {
        return false;
    }
}
