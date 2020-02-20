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

//-----//
//Debug//
//-----//
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

void print_bits_t(uint64_t num) {
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
bool KMP_Experimental_wildcard_arbitrary_length_test (char st[],
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
    char *char_ptr;

    // As text_modifier increases by 8 we move to the next section of the text
    int text_modifier = 0;
    int save_text_modifier = 0;
    int text_len = strlen(st);
    float sections = text_len / 8.00;
    sections = ceilf(sections);
    int size = 0;
    int ls_location = 0;

    uint64_t text_window = 0;
    int j;
    for (j = 7; j >= 0; --j) {
        text_window <<= 8;
        text_window |= (uint64_t) st[ j + text_modifier ];
    }

    uint64_t subquery_matches = ALL_BITS_ON;
    uint64_t encoded_window = text_window;
    uint64_t last_sqm = 0;
    uint64_t prev_section = 0;
    uint64_t location_mask[8] = {18446744073709551360, 18446744073709486080, 18446744073692774400, 18446744069414584320, 18446742974197923840, 18446462598732840960, 18374686479671623680, 0};
    uint64_t last_mask = LAST_BITS_ON;


    bool changed_subqueries = true;
    bool forever_text = false;
    bool bypass = false;
    bool null_force = false;
    bool single_sq = true;
    bool no_match = false;
    int subquery_count = 0;

    // Points to beginning of array
    char_ptr = subquery_array[ subquery_count ];
    character = *char_ptr;

    // to account for singlecards '?' we could insert it as a subquery by itself that matches anything and moves the term forwards by one
    while (character != 0 && st[ text_modifier ] != 0) {
        text_modifier = save_text_modifier;
        if(!bypass) {
            uint64_t char_to_check = character;

            uint64_t char_mask = f_m[ char_to_check ];
            encoded_window = ~(char_mask ^ text_window);
            subquery_matches = LAST_BITS_ON;
            if(DEBUG){
                printf("IF: !bypass\n");
                printf("  Character to check: %c = ", character);
                print_bits(char_to_check);
                printf("\n");
                printf("  Text Window(TW):        ");
                print_bits(text_window);
                printf("\n");
                printf("  Character Mask(CM):     ");
                print_bits(char_mask);
                printf("\n");
                printf("  Encoded Window(EW):     ");
                print_bits(encoded_window);
                printf("\n");
                printf("\n");
                printf("  EW:                     ");
                print_bits(encoded_window);
                printf("\n");
                printf("  Subquery Matches(SM):   ");
                print_bits(subquery_matches);
                printf("\n");
                printf("\n");
            }

            //------------------//
            //Search for matches//
            //------------------//

            subquery_matches &= encoded_window;
            encoded_window >>= 1;
            subquery_matches &= encoded_window;
            encoded_window >>= 1;
            subquery_matches &= encoded_window;
            encoded_window >>= 1;
            subquery_matches &= encoded_window;
            encoded_window >>= 1;
            subquery_matches &= encoded_window;
            encoded_window >>= 1;
            subquery_matches &= encoded_window;
            encoded_window >>= 1;
            subquery_matches &= encoded_window;
            encoded_window >>= 1;
            subquery_matches &= encoded_window;
            if(DEBUG){
                printf("  Search Results---------------------------------------------------------------------------------\n");
                printf("  EW:                     ");
                print_bits(encoded_window);
                printf("\n");
                printf("  SM & EW *8:             ");
                print_bits(subquery_matches);
                printf("\n");
                printf("\n");
            }

        }



        //-----------------//
        //If no first match//
        //-----------------//
        if (subquery_matches == 0) {
            // if text_modifer + 8 < length of text then we should be good to go.
            //Fix following if statement to reflect above thoughts

            //for infinite text, make sure we don't set the length too short
            if(forever_text){
                text_len = text_modifier + 1;
            }

            if((text_modifier + 8) < text_len) {
                text_modifier += 8;
                text_window = 0;
                //-----------------------------------//
                //Move window to end of explored area//
                //-----------------------------------//
                int j;
                for (j = 7; j >= 0; --j) {
                    text_window <<= 8;
                    text_window |= (uint64_t) st[ j + text_modifier ];
                }
                // when we move to the next section of the text we know that whatever we find is after the last subquery
                last_sqm = 0;
                save_text_modifier = text_modifier;
                last_mask = LAST_BITS_ON;

                //------------------//
                //Restart the search//
                //------------------//

            } else {
                return false;
            }
        }

        //--------------//
        //If first match//
        //--------------//
        else {
            //There is a match, have we changed subqueries?
            //Does this matter? does changing subqueries need it's own section?
            if (changed_subqueries) {
                if(DEBUG){
                    printf("IF: First Char of Subquery Matched\n");
                    printf("  IF: Changed Subqueries\n");
                }
                changed_subqueries = false;
                int location = 0;
                // use last_sqm to find location of previous subquery, mask subquery_matches such that everything up to that point is masked.
                // need to find a way to not hardcode these values.** (int)sizeof(long)*CHAR_BIT
                //Need to find the position of the first

                //-------------------------//
                //Find location of instance//
                //-------------------------//
                //Can use this to find the position of the first instance, we locate and remove from subquery_matches
                //But do we need to do this?
                //if (last_sqm == 0) {
                if(DEBUG) {
                    printf("    Setup for finding location of Character to check in SM-----------------------------------------\n");
                    printf("    last_sqm(ls):           ");
                    print_bits(last_sqm);
                    printf("\n");
                }
                last_sqm = subquery_matches;
                //}
                last_sqm &= last_mask;
                if(DEBUG) {
                    printf("    ls <- SM:               ");
                    print_bits(subquery_matches);
                    printf("\n");
                    printf("    ls:                     ");
                    print_bits(last_sqm);
                    printf("\n");
                    printf("    last_mask(lm):          ");
                    print_bits(last_mask);
                    printf("\n");
                    printf("    ls &= lm:               ");
                    print_bits(last_sqm & last_mask);
                    printf("\n");
                    printf("\n");
                }

                //last_sqm is used to identify the location of the first instance
                if (last_sqm & 16843009) {
                    if (last_sqm & 257) {
                        if (last_sqm & 1) {
                            //The masks are messing up, might need to rethink this process 2/18/20 10:36
                                //Last mask system?
                            location = 0;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];

                        } else {
                            location = 1;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];
                        }
                    } else {
                        if (last_sqm & 65536) {
                            location = 2;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];
                        } else {
                            location = 3;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];
                        }
                    }
                } else {
                    if (last_sqm & 1103806595072) {
                        if (last_sqm & 4294967296) {
                            location = 4;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];
                        } else {
                            location = 5;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];
                        }
                    } else {
                        if (last_sqm & 281474976710656) {
                            location = 6;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];
                        } else if(last_sqm & 72057594037927936){
                            location = 7;
                            last_mask &= location_mask[location];
                            subquery_matches &= location_mask[location];
                        } else {
                            location = 7;
                            last_mask = LAST_BITS_ON;
                            subquery_matches = 0;
                            no_match = true;
                            changed_subqueries = true;
                        }
                    }
                }
                if(DEBUG) {
                    printf("    Results of location identification-------------------------------------------------------------\n");
                    printf("    location(l):            %i", location);
                    printf("\n");
                    printf("    lm:                     ");
                    print_bits(last_mask);
                    printf("\n");
                    printf("    last_mask(lm):          ");
                    print_bits(last_mask);
                    printf("\n");
                    printf("    ls &= lm:               ");
                    print_bits(last_sqm & last_mask);
                    printf("\n");
                    printf("    ls_location:            %i", location);
                    printf("\n");
                    printf("\n");
                }
                ls_location = location;
                // This check makes sure that we aren't dealing with a single char subquery, needs to be if charptr+1 != 0;
                if(subquery_array[subquery_count][1] != 0 && location != -1) {

                    //----------------------//
                    //Check rest of Subquery//
                    //----------------------//
                    if(DEBUG){
                        printf("  IF: More than one char in subquery\n");
                    }
                    //This is the heart of the code. Check if rest of subquery exists at the position. Record offset.
                    single_sq = false;
                    int i = 1;
                    char null_check = subquery_array[ subquery_count ][ i ];
                    char query;
                    char text;
                    null_force = false;
                    //text_modifier += location;
                    location += 1;
                    location += text_modifier;
                    int last_location;
                    if(DEBUG){
                        printf("  WHILE: null_check != 0\n");
                        printf("    Check if rest of characters match\n");
                    }
                    while (null_check != 0) {
                        //Does the character match?
                        //Doesn't currently start at match point
                        char query = subquery_array[ subquery_count ][ i ];
                        char text = st[ (i - 1) + location ];
                        last_location = (i - 1) + location;
                        if(DEBUG){
                            printf("      Query char to check:  %c", query);
                            printf("\n");
                            printf("      Text char to check:   %c", text);
                            printf("\n");
                            printf("      Last location in text:%i", last_location);
                            printf("\n");
                        }
                        if (query != text) {
                            if(DEBUG){
                                printf("      Query != Text\n");
                                printf("      Find next instance of first char\n");
                                printf("\n");
                            }
                            //No :(
                            // move to next available character
                            last_sqm = subquery_matches;
                            if (subquery_matches & 16843009) {
                                if (subquery_matches & 257) {
                                    if (subquery_matches & 1) {
                                        location = 0;
                                        subquery_matches &= location_mask[location];
                                    } else {
                                        location = 1;
                                        subquery_matches &= location_mask[location];
                                    }
                                } else {
                                    if (subquery_matches & 65536) {
                                        location = 2;
                                        subquery_matches &= location_mask[location];
                                    } else {
                                        location = 3;
                                        subquery_matches &= location_mask[location];
                                    }
                                }
                            } else {
                                if (subquery_matches & 1103806595072) {
                                    if (subquery_matches & 4294967296) {
                                        location = 4;
                                        subquery_matches &= location_mask[location];
                                    } else {
                                        location = 5;
                                        subquery_matches &= location_mask[location];
                                    }
                                } else {
                                    if (subquery_matches & 281474976710656) {
                                        location = 6;
                                        subquery_matches &= location_mask[location];
                                    } else {
                                        location = 7;
                                        subquery_matches &= location_mask[location];
                                    }
                                }
                            }
                            if(DEBUG) {
                                printf("    Results of location identification-------------------------------------------------------------\n");
                                printf("    location(l):            %i", location);
                                printf("\n");
                                printf("    ls:                     ");
                                print_bits(last_sqm);
                                printf("\n");
                                printf("    sm(lm):                 ");
                                print_bits(subquery_matches);
                                printf("\n");
                                printf("\n");
                            }
                            //If we run out of instances
                            if (last_sqm == 0) {
                                if(DEBUG){
                                    printf("      If: No more instances of first char\n");
                                    printf("        text_modifier += 1 to account for moving to the next char\n");
                                }
                                text_modifier +=1;
                                //If the text is long enough
                                if((text_modifier + 8) < text_len) {
                                    if(DEBUG){
                                        printf("      If: We aren't at the end of the text\n");
                                        printf("        Set text_window = 0\n");
                                        printf("        Initialize counter j\n");
                                        printf("        Initialize initial = 8\n");
                                    }
                                    //text_modifier += 8;
                                    text_window = 0;

                                    int j;
                                    int initial = 8;
                                    if( (text_len - text_modifier) < 8 ){
                                        if(DEBUG){
                                            printf("      If: The remaining text is less than 8 chars\n");
                                            printf("        Set initial to the length of the remaining text\n");
                                            printf("        Subtract 1 from initial to account for ending at 0 rather than 1\n");
                                        }
                                        initial = text_len - text_modifier;
                                        initial--;
                                    }
                                    text_modifier += initial;
                                    for (j = initial; j >= 0; --j) {
                                        text_window <<= 8;
                                        text_window |= (uint64_t) st[ j + text_modifier ];
                                    }
                                    if(DEBUG){
                                        printf("        Set text_modifier += initial so that it accurately indicates the end of the explored area\n");
                                        printf("        Move the text window to the begining of the unexplored area\n");
                                        printf("        Reset last_sqm because we have moved to an unexplored area\n");
                                        printf("        Set null_force = true so that we skip over moving the window additional times\n");
                                        printf("        Set changed_subqueries = true so that the first character is looked for again\n");
                                        printf("        Set null_check = 0 so that we can break out of the while loop\n");
                                    }
                                    // when we move to the next section of the text we know that whatever we find is after the last subquery
                                    last_sqm = subquery_matches;

                                    last_sqm = 0;
                                    null_force = true;
                                    changed_subqueries = true;
                                    null_check = 0;
                                    //------------------//
                                    //Restart the search//
                                    //------------------//
                                    //Let it run its course

                                } else {
                                    if(DEBUG){
                                        printf("        If: We've reached the end of the text\n");
                                        printf("          RETURN false\n");
                                    }
                                    return false;
                                }
                            } else {
                                if(DEBUG){
                                    printf("      If: We find another instance of the first char in the text window\n");
                                    printf("        Set location += text_modifier + 1 to move the initial location in the text to one after the instance we just found\n");
                                    printf("        Reset the counter 'i' so that we move out from the instance as it increases\n");
                                }
                                //We have another location! now we need to reset 'i' and adjust the text_modifier
                                location += text_modifier + 1;
                                i = 0;
                            }
                        }
                        //Yes! If there's another character check for it!
                        if(DEBUG){
                            printf("      If: Query == Text\n");
                            printf("        Increase counter 'i'\n");
                        }
                        i++;
                        if(!null_force) {
                            if(DEBUG){
                                printf("      If: !null_force check if there is another char in the subquery\n");
                            }
                            null_check = subquery_array[ subquery_count ][ i ];
                        }
                    }
                    save_text_modifier = text_modifier;
                    text_modifier = last_location + 1;
                    ls_location += i;
                    // This is a pain point, not fun. simplify into a table
                    // Simplifying did not significantly reduce time 2/18/20 10:01
                    //this setup makes it so that when we move past the end of the window we don't get the right mask
                    if(text_modifier > (save_text_modifier + 8)){
                        last_mask = LAST_BITS_ON;
                    } else {
                        last_mask &= location_mask[i-1];
                        subquery_matches &= location_mask[i-1];
                    }

                }
                //--------//
                //If match//
                //--------//
                if(last_sqm){
                    if(DEBUG){
                        printf("  IF: First char of Subquery is found\n");
                    }

                    //-------------------//
                    //if no next subquery//
                    //-------------------//
                    if (*(subquery_array + (subquery_count + 1)) == 0) {
                        if(DEBUG){
                            printf("    IF: There are no more subqueries\n");
                            printf("      Return TRUE\n");
                        }
                        return true;
                    }

                    //----------------//
                    //If next subquery//
                    //----------------//
                    else{
                        // if text_modifer + 8 < length of text then we should be good to go.
                        //Fix following if statement to reflect above thoughts
                        if(DEBUG){
                            printf("    IF: There are more subqueries\n");
                        }

                        //-------------------------------//
                        // do we need to move the window?//
                        //-------------------------------//
                        if(ls_location == 7 || (text_modifier-1) >= 7) {
                            if(DEBUG){
                                printf("    IF: The last char in the subquery is in the last position or beyond the text window\n");
                            }


                            //-----------------------------------//
                            //Move window to end of explored area//
                            //-----------------------------------//
                            //for infinite text, make sure we don't set the length too short
                            if (forever_text) {
                                if(DEBUG){
                                    printf("    IF: The text has no end\n");
                                    printf("      text_len(tl) = text_modifier(tm) + 9\n");
                                }
                                text_len = text_modifier + 9;
                            }

                            //should just be text_modifier right?
                            if (text_modifier < text_len) {
                                if(DEBUG){
                                    printf("    IF: The new window would start before the end of the text\n");
                                    printf("      Set tw, initialize counter variable, and initial value of counter\n");
                                }
                                //text_modifier += 8;
                                text_window = 0;

                                int j;
                                int initial = 8;
                                if ((text_len - text_modifier) < 8) {
                                    if(DEBUG){
                                        printf("      IF: The remaining text is less than 8\n");
                                        printf("        Alter initial counter variable so that only that many characters are made in the window\n");
                                    }
                                    initial = text_len - text_modifier;
                                    initial--;
                                }
                                text_modifier += initial;
                                for (j = initial; j >= 0; --j) {
                                    text_window <<= 8;
                                    text_window |= (uint64_t) st[ j + text_modifier ];
                                }
                                if(DEBUG){
                                    printf("      tw =                  ");
                                    //print_bits_t(text_window);
                                    printf("\n");
                                }
                                // when we move to the next section of the text we know that whatever we find is after the last subquery
                                if(DEBUG){
                                    printf("        Set text_modifier += initial so that it accurately indicates the end of the explored area\n");
                                    printf("        Move the text window to the begining of the unexplored area\n");
                                    printf("        Reset last_sqm because we have moved to an unexplored area\n");
                                    printf("        Move to the next subquery\n");
                                    printf("        Set changed_subqueries = true so that the first character is looked for again\n");
                                    printf("        Set save_text_modifier = text_modifier\n");

                                }
                                last_sqm = 0;

                                //---------------------//
                                //Move to next subquery//
                                //---------------------//
                                if (*(subquery_array + (subquery_count + 1)) != 0) {
                                    subquery_count++;
                                    changed_subqueries = true;
                                    char_ptr = *(subquery_array + subquery_count);
                                }
                                character = *char_ptr;
                                save_text_modifier = text_modifier;

                                //------------------//
                                //Restart the search//
                                //------------------//
                                //Let it run its course

                            } else {
                                return false;
                            }
                        } else {
                            //---------------------//
                            //Move to next subquery//
                            //---------------------//
                            if(DEBUG){
                                printf("    IF: There are more unexplored characters in the text window\n");
                            }
                            if (*(subquery_array + (subquery_count + 1)) != 0) {
                                subquery_count++;
                                changed_subqueries = true;
                                char_ptr = *(subquery_array + subquery_count);
                            }
                            character = *char_ptr;
                            if(DEBUG){
                                printf("      Move to next subquery\n");
                                printf("      character =             %c\n", character);
                                printf("      Restart the search\n");
                            }
                            // sometimes the subquery_matches
                            //last_sqm = subquery_matches;
                            //if(single_sq){
                            //    last_sqm = last_mask;
                            //}

                            //------------------//
                            //Restart the search//
                            //------------------//
                            //Let it run its course
                        }
                    }
                }

                //-----------//
                //If mismatch//
                //-----------//
                else {
                    if(!null_force) {

                        //------------//
                        //If more text//
                        //------------//
                        //for infinite text, make sure we don't set the length too short
                        if (forever_text) {
                            text_len = text_modifier + 1;
                        }

                        if (text_modifier < text_len) {

                            //-------------------------------//
                            //If more instances of first char//
                            //-------------------------------//
                            if (subquery_matches) {
                                //-----------------------------------//
                                //Restart search at: check rest of sq//
                                //-----------------------------------//
                                bypass = true;
                            }

                                //----------------------------------//
                                //If no more instances of first char//
                                //----------------------------------//
                            else {
                                // if text_modifer + 8 < length of text then we should be good to go.
                                //Fix following if statement to reflect above thoughts

                                //-----------------------------------//
                                //Move window to end of explored area//
                                //-----------------------------------//
                                //for infinite text, make sure we don't set the length too short
                                if (forever_text) {
                                    text_len = text_modifier + 1;
                                }

                                if (text_modifier < text_len) {
                                    text_modifier += 8;
                                    text_window = 0;

                                    int j;
                                    for (j = 7; j >= 0; --j) {
                                        text_window <<= 8;
                                        text_window |= (uint64_t) st[ j + text_modifier ];
                                    }
                                    // when we move to the next section of the text we know that whatever we find is after the last subquery
                                    prev_section = last_sqm;
                                    last_sqm = 0;


                                    //---------------------//
                                    //Move to next subquery//
                                    //---------------------//
                                    if(!no_match) {
                                        if (*(subquery_array + (subquery_count + 1)) != 0) {
                                            subquery_count++;
                                            changed_subqueries = true;
                                            char_ptr = *(subquery_array + subquery_count);
                                        }
                                        character = *char_ptr;
                                    }
                                    no_match = false;

                                    //------------------//
                                    //Restart the search//
                                    //------------------//
                                    //Let it run its course
                                }
                            }

                        }

                            //---------------//
                            //If no more text//
                            //---------------//
                        else {
                            return false;
                        }
                    } else {
                        last_mask = LAST_BITS_ON;
                    }
                }
            }
        }
    }
}
