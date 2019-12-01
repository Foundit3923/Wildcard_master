#include "clean_experimental_wildcard.h"
#include "arbitrary_location.h"
#include "arbitrary_length_wildcard.h"
#include "benchmarking/krauss.h"
#include "benchmarking/original_krauss.h"
#include "benchmarking/shift-or.h"
#include "rabin_karp.h"

// For printing with colors
#define COLOR_RED   "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"

// Feature set to test for
#define ARBITRARY_TERM_LENGTH true
#define SINGLE_CHARACATER_WILDCARDS false
#define WILDCARD_IN_TERM true
#define SIZE 130
#define ONE 255
#define TWO 65280
#define THREE 16711680



//struct DataItem* hashArray[SIZE];
//struct DataItem* query_64;
//struct DataItem* item;

int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;
int test = 0;
int my_count = 0;
int krauss_count = 0;
int wild_count = 0;
int test_count = 0;
uint64_t full_mask[123];

char** subquery;
char* query;

double my_time[76];
double krauss_time[76];
uint64_t term = 0;
char* t_init_term;
char* t_init_query;
bool criteria_are_met = true;
bool anchored_beginning = false;
bool anchored_end = false;
bool same_string = false;
bool first_subquery;
bool last_subquery;

int wildcmp(const char *wild, const char *string) {
    // Written by Jack Handy - <A href="mailto:jakkhandy@hotmail.com">jakkhandy@hotmail.com</A>
    const char *cp = NULL, *mp = NULL;

    while ((*string) && (*wild != '*')) {
        if ((*wild != *string) && (*wild != '?')) {
            return 0;
        }
        wild++;
        string++;
    }

    while (*string) {
        if (*wild == '*') {
            if (!*++wild) {
                return 1;
            }
            mp = wild;
            cp = string+1;
        } else if ((*wild == *string) || (*wild == '?')) {
            wild++;
            string++;
        } else {
            wild = mp;
            string = cp++;
        }
    }

    while (*wild == '*') {
        wild++;
    }
    return !*wild;
}

bool isMatch(char* str, char* pattern){
    int writeIndex = 0;
    bool isFirst = true;
    int i;
    int j;
    for( i = 0; i < strlen(pattern); i++){
        if(pattern[i] == '*'){
            if(isFirst) {
                pattern[ writeIndex++ ] = pattern[ i ];
                isFirst = false;
            }
        } else {
            pattern[writeIndex++] = pattern[i];
            isFirst = true;
        }

    }
    bool T[strlen(str)+1][writeIndex+1];
    if(writeIndex > 0 && pattern[0] == '*'){
        T[0][1] = true;
    }
    int T_length = strlen(str)+1;
    int T_0_length = sizeof(T[0]) / sizeof(bool);
    for(i = 1; i < T_length; i++){
        for( j = 1; j < T_0_length; j++){
            if(pattern[j-1] == '?' || str[i-1] == pattern[j-1]){
                T[i][j] = T[i-1][j-1];
            } else if(pattern[j-1] == '*'){
                T[i][j] = T[i-j][j] || T[i][j-1];
            }
        }

    }
    return T[strlen(str)][writeIndex];
}





void remove_all_chars(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

struct DataItem* preprocessing( char search_term[],  char query_string[], char *temp_query){

    bool check = criteria_are_met;
    subquery = (char**) malloc(sizeof(char*));
    //char *temp_query = (char *) malloc(sizeof(char));f
    strncpy(temp_query, query_string,strlen(query_string));

    uint64_t expected = 0;
    uint64_t tester = 0;

    // make hashtable of chars and their uint64 values
    query_64 = (struct DataItem*) malloc(sizeof(struct DataItem));
    query_64->data = -1;
    query_64->key = -1;

    //Check if there is a query
    if (strlen(temp_query) > 0 && strlen(search_term) <= 8) {


        wild_count = strlen(temp_query);
        remove_all_chars(temp_query, '*');
        size_t query_len = strlen(temp_query);
        size_t term_len = strlen(search_term);

        //clean the search term of * at the beginning and end
        if( search_term[0] == '*' || search_term[strlen(search_term) - 1] == '*'){
            if( search_term[0] == '*' ){
                if( search_term[strlen(search_term) - 1] == '*' ){
                    char query_input[6] = {search_term[ 1 ],
                                           search_term[ 2 ],
                                           search_term[ 3 ],
                                           search_term[ 4 ],
                                           search_term[ 5 ],
                                           search_term[ 6 ],
                                           search_term[ 7 ]};
                    query_input[term_len - 2] = 0;
                    search_term = query_input;
                } else {
                    char query_input[7] = {search_term[ 1 ],
                                           search_term[ 2 ],
                                           search_term[ 3 ],
                                           search_term[ 4 ],
                                           search_term[ 5 ],
                                           search_term[ 6 ],
                                           search_term[ 7 ]};
                    search_term = query_input;
                }
            } else{
                search_term[term_len - 1] = 0;
            }
        }

        // check if cleaned query and cleaned term are a match
        if( strcmp(search_term, temp_query) != 0) {

            if(query_len != term_len) {
                wild_count = wild_count - query_len;
                bool same = query_len >= term_len;
                insert(511, query_len);
                insert(512, term_len);
                if (search(511)->data != query_len || search(512)->data != term_len) {
                    delete(search(511));
                    delete(search(512));
                    insert(511, query_len);
                    insert(512, term_len);
                }

                //Make sure that the term is long enough to satisfy the query
                if (query_len > term_len) {
                    criteria_are_met = false;
                }

                    //The query can only be shorter than the term if the query has a wildcard
                else if (query_len < term_len && wild_count == 0) {
                    criteria_are_met = false;
                }

                    //The query can only be of length 0 if the query is a wildcard
                else if (query_len == 0 && wild_count > 0) {
                    criteria_are_met = true;
                } else {
                    uint64_t test_query;
                    int j;
                    test_query = 0;
                    for (j = 7; j >= 0; --j) {
                        test_query <<= 8;
                        test_query |= (uint64_t) temp_query[ j ];
                    }

                    int i;
                    for (i = 7; i >= 0; --i) {
                        term <<= 8;
                        term |= (uint64_t) search_term[ i ];
                    }

                    // Make char_mask
                    uint64_t char_to_check = 0;
                    uint64_t char_mask_1 = 0;
                    uint64_t char_mask_2 = 0;
                    uint64_t char_mask = 0;
                    int to_shift_query = 0;

                    //Check for anchored chars
                    anchored_beginning = false;
                    anchored_end = false;
                    char *beginningbuff = (char *) malloc(sizeof(char));
                    char *endbuff = (char *) malloc(sizeof(char));

                    // Creates masks for whole subqueries and integrates them into the tester and expected masks
                    if (strlen(query_string) >= 1 && strlen(search_term) >= 1 && query_string[ 0 ] != '*') {
                        if (strlen(query_string) >= 2 && strlen(search_term) >= 2 && query_string[ 1 ] != '*') {
                            if (strlen(query_string) >= 3 && strlen(search_term) >= 3 &&
                                query_string[ 2 ] != '*') {
                                if (strlen(query_string) >= 4 && strlen(search_term) >= 4 &&
                                    query_string[ 3 ] != '*') {
                                    if (strlen(query_string) >= 5 && strlen(search_term) >= 5 &&
                                        query_string[ 4 ] != '*') {
                                        if (strlen(query_string) >= 6 && strlen(search_term) >= 6 &&
                                            query_string[ 5 ] != '*') {
                                            if (strlen(query_string) >= 7 && strlen(search_term) >= 7 &&
                                                query_string[ 6 ] != '*') {
                                                if (strlen(query_string) >= 8 && strlen(search_term) >= 8 &&
                                                    query_string[ 7 ] != '*') {
                                                    char_to_check = test_query & anchor_check[ 7 ];
                                                    char_mask = 0;
                                                    char_mask_1 = ((char_to_check << 8) |
                                                                   char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                    char_mask_2 = ((char_mask_1 << 16) |
                                                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                    char_mask = ((char_mask_2 << 32) |
                                                                 char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                    tester |= anchor_check[ 7 ];
                                                    expected |= char_mask & anchor_check[ 7 ];
                                                }
                                                char_to_check = test_query & anchor_check[ 6 ];
                                                char_mask = 0;
                                                char_mask_1 = ((char_to_check << 8) |
                                                               char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                char_mask_2 = ((char_mask_1 << 16) |
                                                               char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                char_mask = ((char_mask_2 << 32) |
                                                             char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                tester |= anchor_check[ 6 ];
                                                expected |= char_mask & anchor_check[ 6 ];
                                            }
                                            char_to_check = test_query & anchor_check[ 5 ];
                                            char_mask = 0;
                                            char_mask_1 = ((char_to_check << 8) |
                                                           char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                            char_mask_2 = ((char_mask_1 << 16) |
                                                           char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                            char_mask = ((char_mask_2 << 32) |
                                                         char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                            tester |= anchor_check[ 5 ];
                                            expected |= char_mask & anchor_check[ 5 ];
                                        }
                                        char_to_check = test_query & anchor_check[ 4 ];
                                        char_mask = 0;
                                        char_mask_1 = ((char_to_check << 8) |
                                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                        char_mask_2 = ((char_mask_1 << 16) |
                                                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                        char_mask = ((char_mask_2 << 32) |
                                                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                        tester |= anchor_check[ 4 ];
                                        expected |= char_mask & anchor_check[ 4 ];
                                    }
                                    char_to_check = test_query & anchor_check[ 3 ];
                                    char_mask = 0;
                                    char_mask_1 = ((char_to_check << 8) |
                                                   char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                    char_mask_2 = ((char_mask_1 << 16) |
                                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                    char_mask = ((char_mask_2 << 32) |
                                                 char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                    tester |= anchor_check[ 3 ];
                                    expected |= char_mask & anchor_check[ 3 ];
                                }
                                char_to_check = test_query & anchor_check[ 2 ];
                                char_mask = 0;
                                char_mask_1 = ((char_to_check << 8) |
                                               char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                char_mask_2 = ((char_mask_1 << 16) |
                                               char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                char_mask = ((char_mask_2 << 32) |
                                             char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                tester |= anchor_check[ 2 ];
                                expected |= test_query & anchor_check[ 2 ];
                            }
                            char_to_check = test_query & anchor_check[ 1 ];
                            char_mask = 0;
                            char_mask_1 = ((char_to_check << 8) |
                                           char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                            char_mask_2 = ((char_mask_1 << 16) |
                                           char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                            char_mask = ((char_mask_2 << 32) |
                                         char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                            tester |= anchor_check[ 1 ];
                            expected |= char_mask & anchor_check[ 1 ];
                        }
                        char_to_check = test_query & anchor_check[ 0 ];
                        char_mask = 0;
                        char_mask_1 = ((char_to_check << 8) |
                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                        char_mask_2 = ((char_mask_1 << 16) |
                                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                        char_mask = ((char_mask_2 << 32) |
                                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                        tester |= anchor_check[ 0 ];
                        expected |= char_mask & anchor_check[ 0 ];
                        anchored_beginning = true;
                        if (expected != term) {
                            if (strlen(query_string - 1) >= 0 &&
                                query_string[ strlen(query_string) - 1 ] != '*') {
                                to_shift_query = 1;
                                if (strlen(query_string - 2) >= 0 &&
                                    query_string[ strlen(query_string) - 2 ] != '*') {
                                    to_shift_query = 2;
                                    if (strlen(query_string - 3) >= 0 &&
                                        query_string[ strlen(query_string) - 3 ] != '*') {
                                        to_shift_query = 3;
                                        if (strlen(query_string - 4) >= 0 &&
                                            query_string[ strlen(query_string) - 4 ] != '*') {
                                            to_shift_query = 4;
                                            if (strlen(query_string - 5) >= 0 &&
                                                query_string[ strlen(query_string) - 5 ] != '*') {
                                                to_shift_query = 5;
                                                if (strlen(query_string - 6) >= 0 &&
                                                    query_string[ strlen(query_string) - 6 ] != '*') {
                                                    to_shift_query = 6;
                                                    if (strlen(query_string - 7) >= 0 &&
                                                        query_string[ strlen(query_string) - 7 ] != '*') {
                                                        to_shift_query = 7;
                                                        if (strlen(query_string - 8) >= 0 &&
                                                            query_string[ strlen(query_string) - 8 ] != '*') {
                                                            to_shift_query = 8;
                                                            char_to_check =
                                                                    test_query & anchor_check[ query_len - 7 ];
                                                            char_mask = 0;
                                                            char_mask_1 = ((char_to_check << 8) |
                                                                           char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                            char_mask_2 = ((char_mask_1 << 16) |
                                                                           char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                            char_mask = ((char_mask_2 << 32) |
                                                                         char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                            tester |= anchor_check[ strlen(search_term) - 7 ];
                                                            expected |= (char_mask &
                                                                         anchor_check[ strlen(search_term) -
                                                                                       7 ]);
                                                        } else {
                                                            char_to_check =
                                                                    test_query & anchor_check[ query_len - 8 ];
                                                            char_mask = 0;
                                                            char_mask_1 = ((char_to_check << 8) |
                                                                           char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                            char_mask_2 = ((char_mask_1 << 16) |
                                                                           char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                            char_mask = ((char_mask_2 << 32) |
                                                                         char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                            tester |= anchor_check[ strlen(search_term) - 8 ];
                                                            expected |= (char_mask &
                                                                         anchor_check[ strlen(search_term) -
                                                                                       8 ]);
                                                        }
                                                        char_to_check =
                                                                test_query & anchor_check[ query_len - 6 ];
                                                        char_mask = 0;
                                                        char_mask_1 = ((char_to_check << 8) |
                                                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                        char_mask_2 = ((char_mask_1 << 16) |
                                                                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                        char_mask = ((char_mask_2 << 32) |
                                                                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                        tester |= anchor_check[ strlen(search_term) - 6 ];
                                                        expected |= (char_mask &
                                                                     anchor_check[ strlen(search_term) - 6 ]);
                                                    }
                                                    char_to_check = test_query & anchor_check[ query_len - 5 ];
                                                    char_mask = 0;
                                                    char_mask_1 = ((char_to_check << 8) |
                                                                   char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                    char_mask_2 = ((char_mask_1 << 16) |
                                                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                    char_mask = ((char_mask_2 << 32) |
                                                                 char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                    tester |= anchor_check[ strlen(search_term) - 5 ];
                                                    expected |= (char_mask &
                                                                 anchor_check[ strlen(search_term) - 5 ]);
                                                }
                                                char_to_check = test_query & anchor_check[ query_len - 4 ];
                                                char_mask = 0;
                                                char_mask_1 = ((char_to_check << 8) |
                                                               char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                char_mask_2 = ((char_mask_1 << 16) |
                                                               char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                char_mask = ((char_mask_2 << 32) |
                                                             char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                tester |= anchor_check[ strlen(search_term) - 4 ];
                                                expected |= (char_mask &
                                                             anchor_check[ strlen(search_term) - 4 ]);
                                            }
                                            char_to_check = test_query & anchor_check[ query_len - 3 ];
                                            char_mask = 0;
                                            char_mask_1 = ((char_to_check << 8) |
                                                           char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                            char_mask_2 = ((char_mask_1 << 16) |
                                                           char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                            char_mask = ((char_mask_2 << 32) |
                                                         char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                            tester |= anchor_check[ strlen(search_term) - 3 ];
                                            expected |= (char_mask & anchor_check[ strlen(search_term) - 3 ]);
                                        }
                                        char_to_check = test_query & anchor_check[ query_len - 2 ];
                                        char_mask = 0;
                                        char_mask_1 = ((char_to_check << 8) |
                                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                        char_mask_2 = ((char_mask_1 << 16) |
                                                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                        char_mask = ((char_mask_2 << 32) |
                                                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                        tester |= anchor_check[ strlen(search_term) - 2 ];
                                        expected |= (char_mask & anchor_check[ strlen(search_term) - 2 ]);
                                    }
                                    char_to_check = test_query & anchor_check[ query_len - 1 ];
                                    char_mask = 0;
                                    char_mask_1 = ((char_to_check << 8) |
                                                   char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                    char_mask_2 = ((char_mask_1 << 16) |
                                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                    char_mask = ((char_mask_2 << 32) |
                                                 char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                    tester |= anchor_check[ strlen(search_term) - 1 ];
                                    expected |= (char_mask & anchor_check[ strlen(search_term) - 1 ]);
                                }
                                anchored_end = true;

                                //3, *m*k* -> m*k* -> m*k, still need 2 sections so only decrease if both anchors
                            }
                        } else {
                            anchored_end = true;
                        }

                    }
                    if (anchored_end == false) {
                        if (strlen(query_string - 1) >= 0 && query_string[ strlen(query_string) - 1 ] != '*') {
                            to_shift_query = 1;
                            if (strlen(query_string - 2) >= 0 &&
                                query_string[ strlen(query_string) - 2 ] != '*') {
                                to_shift_query = 2;
                                if (strlen(query_string - 3) >= 0 &&
                                    query_string[ strlen(query_string) - 3 ] != '*') {
                                    to_shift_query = 3;
                                    if (strlen(query_string - 4) >= 0 &&
                                        query_string[ strlen(query_string) - 4 ] != '*') {
                                        to_shift_query = 4;
                                        if (strlen(query_string - 5) >= 0 &&
                                            query_string[ strlen(query_string) - 5 ] != '*') {
                                            to_shift_query = 5;
                                            if (strlen(query_string - 6) >= 0 &&
                                                query_string[ strlen(query_string) - 6 ] != '*') {
                                                to_shift_query = 6;
                                                if (strlen(query_string - 7) >= 0 &&
                                                    query_string[ strlen(query_string) - 7 ] != '*') {
                                                    to_shift_query = 7;
                                                    if (strlen(query_string - 8) >= 0 &&
                                                        query_string[ strlen(query_string) - 8 ] != '*') {
                                                        to_shift_query = 8;
                                                        char_to_check =
                                                                test_query & anchor_check[ query_len - 7 ];
                                                        char_mask = 0;
                                                        char_mask_1 = ((char_to_check << 8) |
                                                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                        char_mask_2 = ((char_mask_1 << 16) |
                                                                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                        char_mask = ((char_mask_2 << 32) |
                                                                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                        tester |= anchor_check[ strlen(search_term) - 7 ];
                                                        expected |= (char_mask &
                                                                     anchor_check[ strlen(search_term) - 7 ]);
                                                    } else {
                                                        char_to_check =
                                                                test_query & anchor_check[ query_len - 8 ];
                                                        char_mask = 0;
                                                        char_mask_1 = ((char_to_check << 8) |
                                                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                        char_mask_2 = ((char_mask_1 << 16) |
                                                                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                        char_mask = ((char_mask_2 << 32) |
                                                                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                        tester |= anchor_check[ strlen(search_term) - 8 ];
                                                        expected |= (char_mask &
                                                                     anchor_check[ strlen(search_term) - 8 ]);
                                                    }
                                                    char_to_check = test_query & anchor_check[ query_len - 6 ];
                                                    char_mask = 0;
                                                    char_mask_1 = ((char_to_check << 8) |
                                                                   char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                    char_mask_2 = ((char_mask_1 << 16) |
                                                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                    char_mask = ((char_mask_2 << 32) |
                                                                 char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                    tester |= anchor_check[ strlen(search_term) - 6 ];
                                                    expected |= (char_mask &
                                                                 anchor_check[ strlen(search_term) - 6 ]);
                                                }
                                                char_to_check = test_query & anchor_check[ query_len - 5 ];
                                                char_mask = 0;
                                                char_mask_1 = ((char_to_check << 8) |
                                                               char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                                char_mask_2 = ((char_mask_1 << 16) |
                                                               char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                                char_mask = ((char_mask_2 << 32) |
                                                             char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                                tester |= anchor_check[ strlen(search_term) - 5 ];
                                                expected |= (char_mask &
                                                             anchor_check[ strlen(search_term) - 5 ]);
                                            }
                                            char_to_check = test_query & anchor_check[ query_len - 4 ];
                                            char_mask = 0;
                                            char_mask_1 = ((char_to_check << 8) |
                                                           char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                            char_mask_2 = ((char_mask_1 << 16) |
                                                           char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                            char_mask = ((char_mask_2 << 32) |
                                                         char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                            tester |= anchor_check[ strlen(search_term) - 4 ];
                                            expected |= (char_mask & anchor_check[ strlen(search_term) - 4 ]);
                                        }
                                        char_to_check = test_query & anchor_check[ query_len - 3 ];
                                        char_mask = 0;
                                        char_mask_1 = ((char_to_check << 8) |
                                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                        char_mask_2 = ((char_mask_1 << 16) |
                                                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                        char_mask = ((char_mask_2 << 32) |
                                                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                        tester |= anchor_check[ strlen(search_term) - 3 ];
                                        expected |= (char_mask & anchor_check[ strlen(search_term) - 3 ]);
                                    }
                                    char_to_check = test_query & anchor_check[ query_len - 3 ];
                                    char_mask = 0;
                                    char_mask_1 = ((char_to_check << 8) |
                                                   char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                    char_mask_2 = ((char_mask_1 << 16) |
                                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                    char_mask = ((char_mask_2 << 32) |
                                                 char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                    tester |= anchor_check[ strlen(search_term) - 3 ];
                                    expected |= (char_mask & anchor_check[ strlen(search_term) - 3 ]);
                                }
                                char_to_check = test_query & anchor_check[ query_len - 2 ];
                                char_mask = 0;
                                char_mask_1 = ((char_to_check << 8) |
                                               char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                char_mask_2 = ((char_mask_1 << 16) |
                                               char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                char_mask = ((char_mask_2 << 32) |
                                             char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                tester |= anchor_check[ strlen(search_term) - 2 ];
                                expected |= (char_mask & anchor_check[ strlen(search_term) - 2 ]);
                            }
                            char_to_check = test_query & anchor_check[ query_len - 1 ];
                            char_mask = 0;
                            char_mask_1 = ((char_to_check << 8) |
                                           char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                            char_mask_2 = ((char_mask_1 << 16) |
                                           char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                            char_mask = ((char_mask_2 << 32) |
                                         char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                            tester |= anchor_check[ strlen(search_term) - 1 ];
                            expected |= (char_mask & anchor_check[ strlen(search_term) - 1 ]);
                            anchored_end = true;

                            //3, *m*k* -> m*k* -> m*k, still need 2 sections so only decrease if both anchors
                        }
                    }

                    // check if anchored positions appear where expected
                    if (anchored_end || anchored_beginning) {
                        uint64_t t = term;
                        uint64_t and_check = term & tester;
                        if ((term & tester) != expected) {
                            criteria_are_met = false;
                        } else {
                            //Remove anchored chars
                            term &= ~(expected);

                            //move altered term into place
                            //does this take care of full subqueries?
                            if (anchored_beginning) {
                                term >>= 8;
                                query_string[ 0 ] = '*';
                            }
                            if (anchored_end) {
                                test_query >>= 8 * to_shift_query;
                                if (test_query == 0) {
                                    term = 0;
                                } else {
                                    query_string[ strlen(query_string) - 1 ] = '*';
                                }
                            }
                        }
                    }
                    if (term > 0 && term != 48) {

                        // Make copy of query_string so strtok can mutate it without a seg fault
                        query = (char *) malloc(sizeof(char));
                        //strcpy(query, query_string);


                        // A lack of delimiter in first or last position of the query signifies the
                        // beginning and end of the term, respectively.
                        anchored_beginning = (query_string[ 0 ] != DELIMITER[ 0 ]);
                        anchored_end = (query_string[ strlen(query_string) - 1 ] != DELIMITER[ 0 ]);



                        // Split up query into subqueries. Every group of contiguous characters is a
                        // single subquery. This also filters out all blank subqueries to account
                        // for multiple delimiters in a row and beginning/end delimiters

                        // get the location of the first subquery
                        if (wild_count <= 0) {
                            wild_count = 1;
                        }
                        wild_count--;

                        *subquery = strtok(query_string, DELIMITER);
                        //temp_subquery[0] = subquery;
                        //iterate through chars in subquery and add their full masks to hashtable

                        // This section is obsolete, full masks are generated in the main function

                        if (*subquery != NULL) {
                            char *check = (char *) malloc(sizeof(char));
                            strcpy(check, *subquery);
                            /*int char_count = 0;
                            while (*check != 0) {
                                // if(char_count == 0) {
                                //struct DataItem* search_check = search(*check);
                                if (search(*check) == NULL) {

                                    //set the first char in check as teh key
                                    char uniquery = *check;
                                    int key = (int) uniquery;

                                    // generate uint64 representations of subqueries before processing
                                    uint64_t data;
                                    char_to_check = check[ 0 ];
                                    char_mask_1 = ((char_to_check << 8) |
                                                   char_to_check);                                   // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                    char_mask_2 = ((char_mask_1 << 16) |
                                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                    data = ((char_mask_2 << 32) |
                                            char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                    insert(*check, data);


                                }
                                check++;
                            }*/

                            int sub_count = 0;
                            if (subquery[ sub_count ] && wild_count > 1) {
                                while (sub_count <= wild_count) {
                                    sub_count++;
                                    char *check = strtok(NULL, DELIMITER);
                                    *(subquery + sub_count) = check;//strtok(NULL, DELIMITER);
                                    //temp_subquery[ sub_count ] = subquery;

                                    /*//iterate through chars in subquery and add to hashtable
                                    while (check != NULL && *check != 0) {
                                        //if (search(*check) == NULL) {

                                        //set the first char in check as teh key
                                        int key = (int) *check;

                                        // generate uint64 representations of subqueries before processing
                                        uint64_t data;
                                        char_to_check = check[ 0 ];
                                        char_mask_1 = ((char_to_check << 8) |
                                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                                        char_mask_2 = ((char_mask_1 << 16) |
                                                       char_mask_1);                                    // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                                        data = ((char_mask_2 << 32) |
                                                char_mask_2);                                           // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                                        insert(key, data);
                                        //}
                                        check++;
                                    }*/


                                }
                            }
                        }


                        // Used for testing of anchored queries
                        first_subquery = (subquery != NULL);
                        last_subquery = (subquery == NULL);

                        //if(subquery == NULL){
                        //    criteria_are_met = false;
                        //}
                    } else {
                        *subquery = NULL;
                    }
                }
            } else{
                criteria_are_met = false;
            }
        }
        else{
            same_string = true;
        }
    } else {
        wild_count = strlen(temp_query);
        remove_all_chars(temp_query, '*');
        size_t query_len = strlen(temp_query);
        size_t term_len = strlen(search_term);
        // check if cleaned query and cleaned term are a match
        if( strcmp(search_term, temp_query) != 0) {

            if(query_len != term_len) {
                wild_count = wild_count - query_len;
                bool same = query_len >= term_len;
                //Make sure that the term is long enough to satisfy the query
                if (query_len > term_len) {
                    criteria_are_met = false;
                }
                //The query can only be shorter than the term if the query has a wildcard
                else if (query_len < term_len && wild_count == 0) {
                    criteria_are_met = false;
                }
                //The query can only be of length 0 if the query is a wildcard
                else if (query_len == 0 && wild_count > 0) {
                    criteria_are_met = true;
                }
            }
        } else if(strcmp(search_term,temp_query) == 0){
            same_string = true;
        } else{
            criteria_are_met = false;
        }
        // add prerocessing for text and queries longer than 8 characters.
        // Split up query into subqueries. Every group of contiguous characters is a
        // single subquery. This also filters out all blank subqueries to account
        // for multiple delimiters in a row and beginning/end delimiters

        // get the location of the first subquery
        if (wild_count <= 0) {
            wild_count = 1;
        }
        //wild_count--;

        *subquery = strtok(query_string, DELIMITER);
        //temp_subquery[0] = subquery;
        //iterate through chars in subquery and add their full masks to hashtable

        // This section is obsolete, full masks are generated in the main function

        if (*subquery != NULL) {
            char *check = (char *) malloc(sizeof(char));
            strcpy(check, *subquery);
            /*int char_count = 0;
            while (*check != 0) {
                // if(char_count == 0) {
                //struct DataItem* search_check = search(*check);
                if (search(*check) == NULL) {

                    //set the first char in check as teh key
                    char uniquery = *check;
                    int key = (int) uniquery;

                    // generate uint64 representations of subqueries before processing
                    uint64_t data;
                    char_to_check = check[ 0 ];
                    char_mask_1 = ((char_to_check << 8) |
                                   char_to_check);                                   // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                    char_mask_2 = ((char_mask_1 << 16) |
                                   char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                    data = ((char_mask_2 << 32) |
                            char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                    insert(*check, data);


                }
                check++;
            }*/

            int sub_count = 0;
            if (subquery[ sub_count ] && wild_count >= 1) {
                while (sub_count <= wild_count) {
                    sub_count++;
                    char *check = strtok(NULL, DELIMITER);
                    *(subquery + sub_count) = check;//strtok(NULL, DELIMITER);
                    //temp_subquery[ sub_count ] = subquery;

                    /*//iterate through chars in subquery and add to hashtable
                    while (check != NULL && *check != 0) {
                        //if (search(*check) == NULL) {

                        //set the first char in check as teh key
                        int key = (int) *check;

                        // generate uint64 representations of subqueries before processing
                        uint64_t data;
                        char_to_check = check[ 0 ];
                        char_mask_1 = ((char_to_check << 8) |
                                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                        char_mask_2 = ((char_mask_1 << 16) |
                                       char_mask_1);                                    // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                        data = ((char_mask_2 << 32) |
                                char_mask_2);                                           // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111

                        insert(key, data);
                        //}
                        check++;
                    }*/


                }
            }
        }
    }
    return query_64;
}

void expect(char* init_term, char* init_query, bool expectation, char* message)
{
    if( test == 0 || test == 1) {
        //This setup will need to be changed for the new version. Or the krauss algo will need to be modified to take the
        //same parameters. No need to use them though
        t_init_term = (char *) malloc(sizeof(init_term));
        t_init_query = (char *) malloc(sizeof(init_query));
        query_64 = (struct DataItem *) malloc(sizeof(struct DataItem));

        char *temp_query = (char *) malloc(sizeof(char));

        strncpy(t_init_term, init_term, strlen(init_term));
        strcpy(t_init_query, init_query);


        query_64 = preprocessing(t_init_term, t_init_query, temp_query);
    }


    bool check = criteria_are_met;
    bool result = false;
    double cpu_time_used = 0;
    double* test_time = my_time;
    if(test == 0) {
        if (criteria_are_met) {
            if (((*subquery == NULL || subquery == NULL) && init_query != "") || same_string) {
                my_time[my_count] = 0;
                my_count++;
                result = true;
                same_string = false;
            } else {
                clock_t start;
                clock_t end;
                //clock_t loop_time = 0;
                //clock_t main_time = 0;
                //clock_t init_time = 0;
                //clock_t all_time = 0;
                start = clock();

                int count;
                for (count = 0; count < 1000000; count++) {
                    //  result = wildcard(init_term,
                    //                    init_query)
                    //result = Experimental_wildcard_a(term,subquery, full_mask);
                    result = Experimental_wildcard_arbitrary_length(init_term, subquery, full_mask);
                   // printf("%n \n", count);
                    //  loop_time = loop_time + loop_clock;
                    //main_time = main_time + main_loop_clock;
                    //init_time = init_time + init_clock;
                    //all_time = all_time + all_clock;
                }
                end = clock();
                //end = end - loop_time;
                cpu_time_used = ((double) (end - start)) * CLOCKS_PER_SEC;

                my_time[my_count] = cpu_time_used;
                my_count++;
            }
        } else {
            my_time[my_count] = 0;
            my_count++;
            result = false;
        }

    }
    else {

        clock_t start, end;
        start = clock();

        int count;
        for (count = 0; count < 1000000; count++) {
            //result = isMatch(init_term, init_query);
            //result = rksearch(init_term, init_query, 101);
            /*if(Search(init_term, init_query)){
                result = true;
            }else{
                result = false;
            }
             */
            //result = GeneralTextCompare(init_term, init_query);
            result = kraussListingTwo(init_term, init_query);
            /*if(wildcmp(init_term, init_query)){
                result = true;
            }
            else{
                result = false;
            }*/
        }
        end = clock();
        cpu_time_used = ((double) (end - start)) * CLOCKS_PER_SEC;
        krauss_time[krauss_count] = cpu_time_used;
        krauss_count++;
        /*
        if (criteria_are_met) {
            if (((*subquery == NULL || subquery == NULL) && init_query != "") || same_string) {
                krauss_time[ krauss_count ] = 0;
                krauss_count++;
                result = true;
                same_string = false;
            } else {
                clock_t start;
                clock_t end;
                //clock_t loop_time = 0;
                //clock_t main_time = 0;
                //clock_t init_time = 0;
                //clock_t all_time = 0;
                start = clock();

                int count;
                for (count = 0; count < 10000000; count++) {
                    //  result = wildcard(init_term,
                    //                    init_query)
                    result = Experimental_wildcard(term,
                                                   subquery);
                    // printf("%n \n", count);
                    //  loop_time = loop_time + loop_clock;
                    //main_time = main_time + main_loop_clock;
                    //init_time = init_time + init_clock;
                    //all_time = all_time + all_clock;
                }
                end = clock();
                //end = end - loop_time;
                cpu_time_used = ((double) (end - start)) * CLOCKS_PER_SEC;

                krauss_time[ krauss_count ] = cpu_time_used;
                krauss_count++;
            }
        } else {
            krauss_time[krauss_count] = 0;
            krauss_count++;
            result = false;
        }
*/
    }




    char *expected_return = (char*) malloc(sizeof(char));
    if (expectation == true) expected_return = "true";
    else expected_return = "false";

    char *success;
    if (result == expectation) success = "PASSED";
    else success = "FAILED";

    if(success == "FAILED"){
        my_time[my_count - 1] = 1000000000000;
    }

    char *actual_return;
    if (result == true) actual_return = "true";
    else actual_return = "false";

    char output[10000];
    if (strlen(message) == 0)
    {
        sprintf(output, "TEST %s: Returns %s on Query: %s, Term: %s. %s is expected. CPU time elapsed: %lf\n", success, actual_return, query, term, expected_return, cpu_time_used);
    } else {
        sprintf(output, "TEST %s: Returns %s on %s. %s is expected. CPU time elapsed: %lf\n" , success, actual_return, message, expected_return, cpu_time_used);
    }

    if (result == expectation)
    {
        printf(COLOR_GREEN "%s" COLOR_RESET, output);
        tests_passed++;
    } else {
        printf(COLOR_RED "%s" COLOR_RESET, output);
        tests_failed++;
    }

    tests_run++;
    criteria_are_met = true;

}

int main() {
    /*
    char* start = (char*) malloc(sizeof(char));
    printf("Enter start\n");
    scanf("%s", start);
    printf("you entered %s\n", start);
*/

    if(1 ){//strcmp(start, "start") == 0) {
        for (test; test < 2; test++) {
            //test = 1;

            int count;
            time_t start_sub = clock();
            char *test_query = "term";
            int shift = 2;
            int shift_length = 0;
            int location = 0;

            uint64_t subquery_matches = 256;

            bool one_left = false;

            int char_count;
            uint64_t temp_query;
            for (count = 0; count < 100000000; count++) {

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
            }
            time_t end_sub = clock();
            double sub_time = ((double) (end_sub - start_sub)) * CLOCKS_PER_SEC;

            time_t start_shift = clock();


            for (count = 0; count < 100000000; count++) {
                if (__builtin_popcount(subquery_matches)) {
                    __builtin_ffs(subquery_matches);
                    //  location = shift_length / 8;
                    // one_left = true;
                }
            }
            time_t end_shift = clock();
            double shift_time = ((double) (end_shift - start_shift)) * CLOCKS_PER_SEC;


            double start, end, cpu_time_used;
            start = clock();

            if (test == 1) {
                printf("\nKrauss Results\n");
                printf("---------------------------------------------\n");
            }


    /* Generate all full masks */

    char letters[62] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','1','2','3','4','5','6','7','8','9',' '};
    int i;
    for(i = 0; i < 62; i++) {
        uint64_t char_mask_1;
        uint64_t char_mask_2;
        uint64_t char_mask;
        char char_to_check = letters[i];
        char_mask_1 = ((char_to_check << 8) |
                       char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
        char_mask_2 = ((char_mask_1 << 16) |
                       char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
        char_mask = ((char_mask_2 << 32) |
                     char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
        full_mask[char_to_check] = char_mask;
    }
            printf("Testing undefined behavior returning false\n");
            // TODO: Define these undefined behaviors. Right now they return false
            expect("123456789", "query", false, "too long of a term");
            expect("", "query", false, "empty term");
            expect("term", "", false, "empty query");
            expect("", "*", true, "empty term and lone wildcard");
            printf("\n");

            printf("Testing non-wildcard cases\n");
            expect("term", "term", true, "matching term and query");
            expect("t", "t", true, "matching single letter term and query");
            expect("term", "aterm", false, "leading missing query character");
            expect("term", "termz", false, "trailing missing query character");
            expect("term", "tenrm", false, "middle missing query character");
            printf("\n");

            printf("Testing lone wildcards\n");
            expect("term", "*", true, "single lone wildcard");
            expect("term", "**", true, "dual lone wildcards");
            printf("\n");

            printf("Testing anchored queries\n");
            expect("term", "*term*", true, "matching term and unanchored query");
            expect("term", "*term", true, "matching term and right-anchored query");
            expect("term", "term*", true, "matching term and left-anchored query");
            expect("term", "**term**", true, "dual unanchored wildcards");
            expect("term", "t", false, "dual-anchored single leading letter query");

            expect("term", "t*", true, "left-anchored single leading letter query");

            expect("term", "m", false, "dual-anchored single trailing letter query");
            expect("term", "*m", true, "right-anchored single trailing letter query");
            expect("term", "e", false, "dual-anchored single middle letter query");
            expect("term", "*e*", true, "non-anchored single middle letter query");

            printf("\n");

            printf("Testing complex queries\n");
            expect("term", "*t*rm*", true,
                   "missing letter in query"); // search for first then check rest in subquery all at once
            expect("term", "*te*a*rm*", false, "additional letter in query");

            expect("term", "*rm*te*", false, "reversed subquery order");
            expect("term", "*ter*rm*", false, "minimally overlapping queries");
            expect("term", "*ter*erm*", false, "moderately overlapping queries");
            expect("term", "*term*term*", false, "maximally overlapping queries");
            printf("\n");

            // Start of Krauss Tests from http://www.drdobbs.com/architecture-and-design/matching-wildcards-an-empirical-way-to-t/240169123#ListingOne
            printf("Starting Krauss Tests\n");

            // Cases with repeating character sequences.
            expect("abcccd", "*ccd", true,
                   "Term: abcccd Query: *ccd ");  //add functionality that checks for a whole anchored subquery
            if (ARBITRARY_TERM_LENGTH) {
                expect("mississipissippi", "*issip*ss*", true, "Term: mississipissippi Query: *issip*ss*");
                if (WILDCARD_IN_TERM) {
                    expect("xxxx*zzzzzzzzy*f", "xxxx*zzy*fffff", false, "Term: xxxx*zzzzzzzzy*f Query: xxxx*zzy*fffff");
                    expect("xxxx*zzzzzzzzy*f", "xxx*zzy*f", true, "Term: xxxx*zzzzzzzzy*f Query: xxx*zzy*f");
                }
                expect("xxxxzzzzzzzzyf", "xxxx*zzy*fffff", false, "Term: xxxxzzzzzzzzyf Query: xxxx*zzy*fffff");
                expect("xxxxzzzzzzzzyf", "xxxx*zzy*f", true, "Term: xxxxzzzzzzzzyf Query: xxxx*zzy*f");
                expect("xyxyxyzyxyz", "xy*z*xyz", true, "Term: xyxyxyzyxyz Query: xy*z*xyz");
                expect("mississippi", "*sip*", true, "Term: mississippi Query: *sip*");
                expect("xyxyxyxyz", "xy*xyz", true, "Term: xyxyxyxyz Query: xy*xyz");
                expect("mississippi", "mi*sip*", true, "Term: mississippi Query: mi*sip*");
            }
            expect("ababac", "*abac*", true,
                   "Term: ababac Query: *abac* "); // search for first then check rest in subquery all at once
            expect("aaazz", "a*zz*", true, "Term: aaazz Query: a*zz* ");
            expect("a12b12", "*12*23", false, "Term: a12b12 Query: *12*23 ");
            expect("a12b12", "a12b", false, "Term: a12b12 Query: a12b ");
            expect("a12b12", "*12*12*", true, "Term: a12b12 Query: *12*12* "); // reuse searches

            // Additional cases where the '*' char appears in the tame string.
            if (WILDCARD_IN_TERM) {
                expect("*", "*", true, " ");
                expect("a*abab", "a*b", true, " ");
                expect("a*r", "a*", true, " ");
                expect("a*ar", "a*aar", false, " ");
            }

            // More double wildcard scenarios.
            if (ARBITRARY_TERM_LENGTH) {
                expect("XYXYXYZYXYz", "XY*Z*XYz", true, "Term: XYXYXYZYXYz Query: XY*Z*XYz");
                expect("missisSIPpi", "*SIP*", true, "Term: missisSIPpi Query: *SIP*");
                expect("mississipPI", "*issip*PI", true, "Term: mississipPI Query: *issip*PI");
                expect("xyxyxyxyz", "xy*xyz", true, "Term: xyxyxyxyz Query: xy*xyz");
                expect("miSsissippi", "mi*sip*", true, "Term: miSsissippi Query: mi*sip*");
                expect("miSsissippi", "mi*Sip*", false, "Term: miSsissippi Query: mi*Sip*");
                expect("abAbac", "*Abac*", true, "Term: abAbac Query: *Abac*");
                expect("abAbac", "*Abac*", true, "Term: abAbac Query: *Abac*");
                expect("aAazz", "a*zz*", true, "Term: aAazz Query: a*zz*");
                expect("A12b12", "*12*23", false, "Term: A12b12 Query: *12*23");
                expect("a12B12", "*12*12*", true, "Term: a12B12 Query: *12*12*");
                expect("oWn", "*oWn*", true, "Term: oWn Query: *oWn*");
            }

            // Completely tame (no wildcards) cases.
            expect("bLah", "bLah", true, "Term: bLah Query: bLah");
            expect("bLah", "bLaH", false, "Term: bLah Query: bLaH");

            // Simple mixed wildcard tests suggested by IBMer Marlin Deckert.
            if (SINGLE_CHARACATER_WILDCARDS) {
                expect("a", "*?", true, "");
                expect("ab", "*?", true, "");
                expect("abc", "*?", true, "");
            }

            // More mixed wildcard tests including coverage for false positives.
            if (SINGLE_CHARACATER_WILDCARDS) {
                expect("a", "??", false, " ");
                expect("ab", "?*?", true, " ");
                expect("ab", "*?*?*", true, " ");
                expect("abc", "?**?*?", true, " ");
                expect("abc", "?**?*&?", false, " ");
                expect("abcd", "?b*??", true, " ");
                expect("abcd", "?a*??", false, " ");
                expect("abcd", "?**?c?", true, " ");
                expect("abcd", "?**?d?", false, " ");
                expect("abcde", "?*b*?*d*?", true, " ");
            }

            // Single-character-match cases.
            if (SINGLE_CHARACATER_WILDCARDS) {
                expect("bLah", "bL?h", true, " ");
                expect("bLaaa", "bLa?", false, " ");
                expect("bLah", "bLa?", true, " ");
                expect("bLaH", "?Lah", false, " ");
                expect("bLaH", "?LaH", true, " ");
            }

            // Many-wildcard scenarios.
            if (ARBITRARY_TERM_LENGTH) {

                expect(
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab",
                        "a*a*a*a*a*a*aa*aaa*a*a*b", true, "Term: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab  Query: a*a*a*a*a*a*aa*aaa*a*a*b");
                expect(
                        "abababababababababababababababababababaacacacacacacaca"
                        "daeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                        "*a*b*ba*ca*a*aa*aaa*fa*ga*b*", true, "Term: abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab Query: *a*b*ba*ca*a*aa*aaa*fa*ga*b*");
                expect(
                        "abababababababababababababababababababaacacacacacacaca"
                        "daeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                        "*a*b*ba*ca*a*x*aaa*fa*ga*b*", false, "Term: abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab Query: *a*b*ba*ca*a*x*aaa*fa*ga*b*");

                 expect(
                        "abababababababababababababababababababaacacacacacacaca"
                        "daeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                        "*a*b*ba*ca*aaaa*fa*ga*gggg*b*", false, "Term: abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab Query: *a*b*ba*ca*aaaa*fa*ga*gggg*b*");

                 expect(
                        "abababababababababababababababababababaacacacacacacacadae"
                        "afagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                        "*a*b*ba*ca*aaaa*fa*ga*ggg*b*", true, "Term: abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab Query: *a*b*ba*ca*aaaa*fa*ga*ggg*b*");

                 expect(
                        "aaabbaabbaab",
                        "*aabbaa*a*", true, "Term: aaabbaabbaab Query: *aabbaa*a*");
                expect(
                        "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*",
                        "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", true, "Term: a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a* Query: a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*");
                expect(
                        "aaaaaaaaaaaaaaaaa",
                        "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", true, "Term: aaaaaaaaaaaaaaaaa Query: *a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*");
                expect(
                        "aaaaaaaaaaaaaaaa",
                        "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", false, "Term: aaaaaaaaaaaaaaaa Query: *a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*");
                expect(
                        "abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefg"
                        "hij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn",
                        "abc*abc*abc*abc*abc*abc*abc*abc*abc*"
                        "abc*abc*abc*abc*abc*abc*abc*abc*", false, "Term: abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn Query: abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*");

                expect(
                        "abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghi"
                        "j*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn",
                        "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*", true, "Term: abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn Query: abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*");
                expect(
                        "abc*abcd*abcd*abc*abcd",
                        "abc*abc*abc*abc*abc", false, "Term: abc*abcd*abcd*abc*abcd Query: abc*abc*abc*abc*abc");
                expect(
                        "abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd",
                        "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd", true, "Term: abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd Query: abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd");
                expect(
                        "********a********b********c********",
                        "abc", false, "Term: ********a********b********c******** Query: abc");

                 expect(
                        "Have you gazed on naked grandeur where there's nothing else to gaze on, set pieces and drop-curtain scenes galore, big mountians heaved to heaven, which the blinding sunsets blazon, black canyons where the rapids rip and roar? Have you swept the visioned valley with the green stream streaking through it, searched the vastness for a something you have lost? Have you strung your soul to silence? Then for God's sake go and do it; Hear the challenge, learn the lesson, pay the cost.",
                        "*Have*ther*bl*rip*vast*strung your soul*go*lesson*", true, "Term: Have you gazed on naked grandeur where there's nothing else to gaze on, set pieces and drop-curtain scenes galore, big mountians heaved to heaven, which the blinding sunsets blazon, black canyons where the rapids rip and roar? Have you swept the visioned valley with the green stream streaking through it, searched the vastness for a something you have lost? Have you strung your soul to silence? Then for God's sake go and do it; Hear the challenge, learn the lesson, pay the cost. Query: *Have*ther*bl*rip*vast*strung your soul*go*lesson*");

                expect(
                        "Have you gazed on naked grandeur where there's nothing else to gaze on, set pieces and drop-curtain scenes galore, big mountians heaved to heaven, which the blinding sunsets blazon, black canyons where the rapids rip and roar? Have you swept the visioned valley with the green stream streaking through it, searched the vastness for a something you have lost? Have you strung your soul to silence? Then for God's sake go and do it; Hear the challenge, learn the lesson, pay the cost.",
                        "*cost*", true, "Term: Have you gazed on naked grandeur where there's nothing else to gaze on, set pieces and drop-curtain scenes galore, big mountians heaved to heaven, which the blinding sunsets blazon, black canyons where the rapids rip and roar? Have you swept the visioned valley with the green stream streaking through it, searched the vastness for a something you have lost? Have you strung your soul to silence? Then for God's sake go and do it; Hear the challenge, learn the lesson, pay the cost. Query: *cost*");
                expect(
                        "Have you gazed on naked grandeur where there's nothing else to gaze on, set pieces and drop-curtain scenes galore, big mountians heaved to heaven, which the blinding sunsets blazon, black canyons where the rapids rip and roar? Have you swept the visioned valley with the green stream streaking through it, searched the vastness for a something you have lost? Have you strung your soul to silence? Then for God's sake go and do it; Hear the challenge, learn the lesson, pay the cost. Have you wandered in teh wilderness, the sagebrush desolation, the bunch-grass levels where the cattle graze? Have you whistled bits of rag-time at the end of all creation, and learned to know the desert's little ways? Have you camped upon the foothills, have you galloped o'er the ranges, Have you roamed the arid sun-lands thorugh and through? Have you chummed up with the mesa? Do you know its moods and changes? Then listen to the wild-it's calling you. Have you known the great while silence, not a snow-gemmed twig aquiver? (eternal truths that shame our soothing lies.) Have you broken trail on snowshoes? mushed your huskies up the river, dared the unkown, led the way, and clutched the prize? Have you marked the map's void spaces, mingled with the mongrel races, felt the savage strength of brute in every thew? And though grim as hell the worst is, can you round it off with curses? Then harken to the wild-it's wanting you.",
                        "*wanting*", true, "Term: Have you gazed on naked grandeur where there's nothing else to gaze on, set pieces and drop-curtain scenes galore, big mountians heaved to heaven, which the blinding sunsets blazon, black canyons where the rapids rip and roar? Have you swept the visioned valley with the green stream streaking through it, searched the vastness for a something you have lost? Have you strung your soul to silence? Then for God's sake go and do it; Hear the challenge, learn the lesson, pay the cost. Query: *cost*");

            }
            expect("abc", "********a********b********c********", true,
                   "Term: abc Query: ********a********b********c********");
            expect("abc", "********a********b********b********", false,
                   "Term: abc  Query: ********a********b********b********");
            expect("*abc*", "***a*b*c***", true, "Term: *abc*  Query: ***a*b*c***");
                  expect("monkeys*", "m*key*s*", true, " ");
                  expect("monkeys*", "*m*o*n*k*e*y*s*", true, " ");
            printf("\n");
            //free(query_64);

            printf("Ran %d tests with %d passes and %d failures\n", tests_run, tests_passed, tests_failed);

            end = clock();
            cpu_time_used = ((double) (end - start)) * CLOCKS_PER_SEC;
            printf("Total time used: %f \n", cpu_time_used);

            if (test == 1) {
                int i = 0;
                int loss_count = 0;
                int win_count = 0;
                double my_total = 0;
                double krauss_total = 0;
                printf("   My Time      |   Krauss Time      |   Result\n");
                for (i = 0; i < (tests_run / 2); i++) {
                    if (my_time[ i ] > krauss_time[ i ]) {
                        printf("   %f      |   %f               |   Loss\n", my_time[ i ], krauss_time[ i ]);
                        loss_count++;
                        if (my_time[ i ] == 1000000000000) {
                            my_total += 0;
                        } else {
                            my_total += my_time[ i ];
                        }
                        krauss_total += krauss_time[ i ];
                    } else {
                        printf("   %f      |   %f               |   Win\n", my_time[ i ], krauss_time[ i ]);
                        win_count++;
                        if (my_time[ i ] == 1000000000000) {
                            my_total += 0;
                        } else {
                            my_total += my_time[ i ];
                        }
                        krauss_total += krauss_time[ i ];
                    }
                }
                printf("total: %f      |   %f               |   Win\n", my_total, krauss_total);
                printf("Difference: %f \n", (my_total - krauss_total));
                printf("Win: %i.  Loss: %i \n", win_count, loss_count);

            }


        }


        return 0;
    }
}



