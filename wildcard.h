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
void print_bits(uint64_t num) {
    int i = (int) malloc(sizeof(int)); // for C89 compatability
    for (i=sizeof(num)*8-1; i>=0; i--)
    {
        putchar(((num >> i) & 1) ? '1': '0');
    }
 //   free(i);
    i = NULL;

}

bool wildcard (char search_term[], char query_string[])
{
    // Undefined behavior, so return false
    if (
        (strlen(search_term) == 0 ||
        strlen(search_term) > MAX_TERMS ||
        strlen(query_string) == 0) &&
        query_string[0] != '*'
    ) return false;

    //---------------//
    // PREPROCESSING //
    //---------------//

    // Make copy of search_term so we can safely mutate as new type
    //uint64_t term = (uint64_t) malloc(sizeof(uint64_t));
    //strcpy(&term, search_term);
    int i;
    unsigned char input[8] = { search_term[0], search_term[1], search_term[2], search_term[3], search_term[4], search_term[5], search_term[6], search_term[7] };
    uint64_t term = 0;
    for (i = 7; i >= 0; --i)
    {
        term <<= 8;
        term |= (uint64_t)input[i];
    }

    int max_length = 8 * sizeof(char);
    bool split_term = false;

/*    if ( *search_term > max_length) {
        //check if the term has more than 8 chars.
        // if yes, split into array of arrays. We should be able to move through it the same as small scale
        // there will be some conditionals to account for splitting a subquery, and moving to a new section of the term
        split_term = true;
        char** term_list = (char**) malloc(sizeof(char*));
        int count = (int) malloc(sizeof(int));
        count = 0;
        while( term > 0) {
            term_list[count][0] = term & ALL_BITS_ON;
            term >>= max_length;
            count++;
        }
        free(count);
        count = NULL;
    }
    */

    // Make copy of query_string so strtok can mutate it without a seg fault
    char *query = (char*) malloc(sizeof(char));
    strcpy(query, query_string);


    // A lack of delimiter in first or last position of the query signifies the
    // beginning and end of the term, respectively.
    bool anchored_beginning = (query_string[0] != DELIMITER[0]);
    bool anchored_end = (query_string[strlen(query_string)-1] != DELIMITER[0]);

    // Split up query into subqueries. Every group of contiguous characters is a
    // single subquery. This also filters out all blank subqueries to account
    // for multiple delimiters in a row and beginning/end delimiters

    // get the location of the first subquery
    char *subquery;
    subquery = strtok(query, DELIMITER);

    // Used for testing of anchored queries
    bool first_subquery = (subquery != NULL);
    bool last_subquery = (subquery == NULL);

    //------------//
    // PROCESSING //
    //------------//

    // walk through other tokens
    char character;
    char *char_ptr;
    // Reset matches for each subquery
    // Initial value for mask reduction
    uint64_t subquery_matches = ALL_BITS_ON;
    int subquery_length;
    bool all_subqueries_match = true; // innocent until proven guilty

    // Iterate through subqueries. Shortcircuit when a subquery mismatches
    while ((first_subquery || !last_subquery) && all_subqueries_match)
    {

        // Iterate through masks. There are BYTE_LENGTH masks per subquery character.
        // No shortcircuit because when subquery_matches
        // is 0, the term still needs to be shifted for each remaining bit
        char_ptr = subquery;
        character = *char_ptr;
        if (DEBUG) {
            printf("\t\tTerm: %s\n", search_term);
            printf("\t\tQuery: %s\n", query_string);

        }
        while (character != NULL)
        {
            for (int i=0; i<BYTE_LENGTH; i++)
            {

                // Check for 1 in trailing byte positions
                if (character & BYTE_TRAILING_BIT_ON) {
                    if (DEBUG) {
                        printf("\t\tTerm bits:                ");
                        print_bits(term);
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
                        print_bits(LAST_BITS_ON);
                        printf("\n");
                        printf("\t\t(term & LAST_BITS_ON):    ");
                        print_bits(term & LAST_BITS_ON);
                        printf("\n");
                        printf("\t\tSubquery_matches:         ");
                        print_bits(subquery_matches);
                        printf("\n");
                    }
                    subquery_matches &= (term & LAST_BITS_ON);
                }

                // Check for 1 in leading byte positions
                else {
                    if (DEBUG) {
                        printf("\t\tTerm bits:                ");
                        print_bits(term);
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
                        print_bits(~(term | LAST_BITS_OFF));
                        printf("\n");
                        printf("\t\tSubquery_matches:         ");
                        print_bits(subquery_matches);
                        printf("\n");
                    }
                    subquery_matches &= ~(term | LAST_BITS_OFF);
                }

                if (DEBUG) {
                    printf("\t\tNew subquery_matches:     ");
                    print_bits(subquery_matches);
                    printf("\n");
                }

                // Update for next loop
                term >>= 1;
                character >>= 1;

                if (DEBUG) printf("\t\t\n");
            }
            //Need if statement here, if no match, move to next section of term
            char_ptr++;
            character = *char_ptr;
            if (DEBUG) printf("\t}\n");
        }


        // At this point, two things have happend:
        //   1) Every byte which began a subquery match will be BYTE_LEADING_BIT_ON.
        //      So for a term of "abababa" and a subquery of "aba", subquery_matches is:
        //      0o128 0o0 0o128 0o0 0o128 0o0 0o0 ...and any more trailing zeroes
        //   2) The bitshifts have thrown out len(subquery) characters of term
        //      If the leading bit of subquery_matches is active, then those
        //      thrown away characters were the first subquery match. If not, the
        //      next active bit is the beginning of the first subquery match.

        // If first bit of subquery_matches isn't active, there was no subquery
        // match at the first byte, so if the query is supposed to start at the
        // beginning of the string, this is a deal breaker. Shortcircuit
        // everything and return false
        bool trailing_bit_inactive = (subquery_matches & TRAILING_BIT_ON) == 0;

        if (first_subquery && anchored_beginning && trailing_bit_inactive) {
            if (DEBUG) printf("Left-anchored subquery failed\n");
            all_subqueries_match = false;
            subquery_matches = 0;
        }

        subquery = strtok(NULL, DELIMITER);
        last_subquery = (subquery == NULL);

        // If it's the last subquery and it's supposed to match the end of the string,
        // keep shifting until you run out of term or run out of subquery_matches
        if (last_subquery && anchored_end)
        {
            // While we still have characters left in the term and while subquery_matches is > 1
            while ( term > 0 && subquery_matches > TRAILING_BIT_ON) {

                subquery_matches >>= BYTE_LENGTH;
                term >>= BYTE_LENGTH;
            }

            if (term != 0 || subquery_matches != 1) {
                all_subqueries_match = false;
                if (DEBUG) printf("Right-anchored subquery failed\n");
            }
        }

        // For all other cases, empty subquery_matches means we've run out
        else if (subquery_matches == 0) {
            if (DEBUG) printf("No matches for subquery\n");
            all_subqueries_match = false;
        }

        if (first_subquery) {
            first_subquery = false;
        }
        //free(trailing_bit_inactive);
        //trailing_bit_inactive = NULL;

    }

    //-----------------------//
    // Free Allocated Memory //
    //-----------------------//
    //free(term);
    //term = NULL;
    //free(max_length);
    //max_length = NULL;
    //free(query);
    //query = NULL;
    //free(anchored_beginning);
    //anchored_beginning = NULL;
    //free(anchored_end);
    //anchored_end = NULL;
    //free(subquery);
    //subquery = NULL;
    //free(first_subquery);
    //first_subquery = NULL;
    //free(last_subquery);
    //last_subquery = NULL;
    //free(character);
    //character = NULL;
    //free(char_ptr);
    //char_ptr = NULL;
    //free(subquery_matches);
    //subquery_matches = NULL;
    //free(subquery_length);
    //subquery_length = NULL;
    //free(all_subqueries_match);
    //all_subqueries_match = NULL;

    //--------//
    // Output //
    //--------//

    return all_subqueries_match;

}
