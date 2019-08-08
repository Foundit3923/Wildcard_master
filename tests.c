#include "Experimental_wildcard.h"
#include "benchmarking/krauss.h"


// For printing with colors
#define COLOR_RED   "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"

// Feature set to test for
#define ARBITRARY_TERM_LENGTH false
#define SINGLE_CHARACATER_WILDCARDS false
#define WILDCARD_IN_TERM false
#define ONE 255
#define TWO 65280
#define THREE 16711680


int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;
int test = 0;
int my_count = 0;
int krauss_count = 0;
int wild_count = 0;

char** subquery;
char* query;
double my_time[76];
double krauss_time[76];
uint64_t anchor_check[8] = {255,65280,16711680,4278190080,1095216660480,280375465082880,71776119061217280,18374686479671624000};
uint64_t term = 0;
char* t_init_term;
char* t_init_query;
bool criteria_are_met = true;
bool anchored_beginning = false;
bool anchored_end = false;
bool first_subquery;
bool last_subquery;

void remove_all_chars(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

void preprocessing( char search_term[],  char query_string[]){

    bool check = criteria_are_met;
    char *temp_query = (char *) malloc(sizeof(char));
    subquery = (char**) malloc(sizeof(char*));
    uint64_t expected = 0;
    uint64_t tester = 0;
    strcpy(temp_query, query_string);
    //Check if there is a query
    if (strlen(temp_query) > 0 && strlen(search_term) <= 8) {
        //Make sure that the term is long enough to satisfy the query
        wild_count = strlen(temp_query);
        remove_all_chars(temp_query, '*');
        size_t query_len = strlen(temp_query);
        size_t term_len = strlen(search_term);
        wild_count = wild_count - query_len;
        bool same = query_len >= term_len;
        if (query_len > term_len) {
            criteria_are_met = false;
        }

        if (*search_term == 0 && *query_string != 42) {
            criteria_are_met = false;
        } else {
            //Check for anchored chars
            anchored_beginning = false;
            anchored_end = false;
            if (query_string[ 0 ] != '*') {
                anchored_beginning = true;
                if (query_string[ strlen(query_string) - 1 ] != '*') {
                    anchored_end = true;

                    //3, *m*k* -> m*k* -> m*k, still need 2 sections so only decrease if both anchors
                }

            }

            if (query_string[ strlen(query_string) - 1 ] != '*') {
                anchored_end = true;
            }
            if (anchored_beginning == true || anchored_end == true) {
                uint64_t test_query;
                int j;
                unsigned char query_input[8] = {temp_query[ 0 ], temp_query[ 1 ], temp_query[ 2 ],
                                                temp_query[ 3 ],
                                                temp_query[ 4 ], temp_query[ 5 ], temp_query[ 6 ],
                                                temp_query[ 7 ]};
                test_query = 0;
                for (j = 7; j >= 0; --j) {
                    test_query <<= 8;
                    test_query |= (uint64_t) query_input[ j ];
                }

                // Make char_mask
                uint64_t char_to_check = 0;
                uint64_t char_mask_1 = 0;
                uint64_t char_mask_2 = 0;
                uint64_t char_mask = 0;



                //If anchored_end, get location

                if (anchored_end) {

                    char_to_check = test_query & anchor_check[query_len - 1];
                    char_mask = char_to_check;
                    //char_to_check >>= 8;
                    //char_to_check >>= 8;
                    //char_to_check >>= 8;
                    int shifter = (query_len - 1) * 8;
                    char_to_check >>= shifter;
                    //if( (query_len -1) > 0){
                        char_mask = 0;
                        char_mask_1 = ((char_to_check << 8) | char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                        char_mask_2 = ((char_mask_1 << 16) | char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                        char_mask = ((char_mask_2 << 32) | char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
                    //}
                    /*The tester is an 8 bit mask that reveals characters at particular locations.
                     * In this one, we are getting the location of the end of the term.
                     * In this way the tester represente
                    *  For a char at postion 4:
                    *       0000000000000000000000000000000011111111000000000000000000000000
                    */
                    tester = anchor_check[ strlen(search_term) - 1 ];
                    uint64_t check = term & tester;

                    //Expected shows what output we expect when we compare the term and the tester
                    /* For a 'm' at position 4 we do:
                     *      0000000000000000000000000000000011111111000000000000000000000000  tester
                     *      0111001101110011011100110111001101110011011100110111001101110011  char mask
                     *   &  ________________________________________________________________
                     *      0000000000000000000000000000000001110011000000000000000000000000  expected result
                     */
                    expected = char_mask & tester;

                    if (anchored_beginning) {
                        /*char_to_check = test_query & anchor_check[0];
                        char_mask = char_to_check;
                        if( (query_len -1) > 0){

                            char_mask_1 = ((char_to_check << 8) & char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                            char_mask_2 = ((char_mask_1 << 16) & char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                            char_mask = ((char_mask_2 << 32) & char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
                        }*/
                        expected |= (test_query & anchor_check[ 0 ]);
                        tester |= anchor_check[ 0 ];

                    }
                } else if (anchored_beginning) {

                    /*char_to_check = test_query & anchor_check[0];
                    char_mask = char_to_check;

                    if( (query_len -1) > 0){

                        char_mask_1 = ((char_to_check << 8) | char_to_check);                                  // 00000000 00000000 00000000 00000000 00000000 00000000 00000000 11111111 -> 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111
                        char_mask_2 = ((char_mask_1 << 16) | char_mask_1);                                     // 00000000 00000000 00000000 00000000 00000000 00000000 11111111 11111111 -> 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111
                        char_mask = ((char_mask_2 << 32) | char_mask_2);                                       // 00000000 00000000 00000000 00000000 11111111 11111111 11111111 11111111 -> 11111111 11111111 11111111 11111111 11111111 11111111 11111111 11111111
                    }*/

                    expected = test_query & anchor_check[ 0 ];
                    tester = anchor_check[ 0 ];
                }
            }

            // if it is, then convert the term into a uint64_t format
            int i;
            unsigned char input[8] = {search_term[ 0 ], search_term[ 1 ], search_term[ 2 ], search_term[ 3 ],
                                      search_term[ 4 ], search_term[ 5 ], search_term[ 6 ], search_term[ 7 ]};
            term = (uint64_t) malloc(sizeof(uint64_t));
            for (i = 7; i >= 0; --i) {
                term <<= 8;
                term |= (uint64_t) input[ i ];
            }

            if (anchored_end || anchored_beginning) {
                uint64_t t = term;
                uint64_t and_check = term & tester;
                if ((term & tester) != expected) {
                    criteria_are_met = false;
                } else {
                    //Remove anchored chars
                    term &= ~(expected);
                    //Don't forget to remove found chars from query
                    if (anchored_beginning) {
                        term >>= 8;
                        query_string[ 0 ] = '*';
                    }
                    if (anchored_end) {
                        //need to take care of term here too
                        query_string[ strlen(query_string) - 1 ] = '*';
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

                if(anchored_beginning && anchored_end){
                    wild_count--;
                }
                else{
                    wild_count--;
                }
                if(wild_count <= 0){
                    wild_count = 1;
                }
                //remove outer *?
                //char temp_subquery[wild_count];

                *subquery = strtok(query_string, DELIMITER);
                //temp_subquery[0] = subquery;
                int sub_count = 0;
                if(subquery[sub_count] && wild_count > 1) {
                    while (sub_count < wild_count) {
                        sub_count++;
                        subquery[sub_count] = strtok(NULL, DELIMITER);
                        //temp_subquery[ sub_count ] = subquery;
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
    } else {
        criteria_are_met = false;
    }
}

void expect(char init_term[], char init_query[], bool expectation, char message[])
{

    //This setup will need to be changed for the new version. Or the krauss algo will need to be modified to take the
    //same parameters. No need to use them though
    t_init_term = (char*) malloc(sizeof(char));
    t_init_query= (char*) malloc(sizeof(char));

    strncpy(t_init_query,init_query,strlen(init_query));
    strncpy(t_init_term,init_term,strlen(init_term));

    preprocessing(t_init_term, t_init_query);

    bool check = criteria_are_met;
    bool result = false;
    double cpu_time_used = 0;
    double* test_time = my_time;
    if(test == 0) {
        if (criteria_are_met) {
            if ((*subquery == NULL || subquery == NULL) && init_query != "") {
                my_time[my_count] = 0;
                my_count++;
                result = true;
            } else {
                clock_t start;
                clock_t end;
                start = clock();

                int count;
                for (count = 0; count < 1; count++) {
                    //  result = wildcard(init_term,
                    //                    init_query)
                    result = Experimental_wildcard(term,
                                                   subquery);
                }
                end = clock();
                cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

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
        for (count = 0; count < 1; count++) {
            result = kraussListingTwo(init_term, init_query);
        }
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        krauss_time[krauss_count] = cpu_time_used;
        krauss_count++;
    }


/*        bool new_wildcard(uint64_t search_term,
                          char *t,
                          char* subquery_array,
                          bool anchored_beginning,
                          bool anchored_end,
                          bool first_subquery,
                          bool last_subquery);
        bool kraussListingTwo(uint64_t search_term,
                              char *t,
                              char *q,
                              bool anchored_beginning,
                              bool anchored_end,
                              bool first_subquery,
                              bool last_subquery);
        clock_t start, end;
        double cpu_time_used;
        start = clock();

        bool (*algo[2])(uint64_t search_term,
                        char *t,
                        char* subquery_array,
                        bool anchored_beginning,
                        bool anchored_end,
                        bool first_subquery,
                        bool last_subquery);
        algo[ 0 ] = new_wildcard;
        algo[ 1 ] = kraussListingTwo;

        int count;

        for (count = 0; count < 1000000; count++) {
            result = algo[ test ](term, search_term, subquery,);
        }
        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

*/

    char *expected_return;
    if (expectation == true) expected_return = "true";
    else expected_return = "false";

    char *success;
    if (result == expectation) success = "PASSED";
    else success = "FAILED";

    if(success == "FAILED"){
        my_time[my_count - 1] = 1000;
    }

    char *actual_return;
    if (result == true) actual_return = "true";
    else actual_return = "false";

    char output[100];
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
    for (test; test < 2; test++) {
        double start, end, cpu_time_used;
        start = clock();

        if (test == 1) {
            printf("\nKrauss Results\n");
            printf("---------------------------------------------\n");
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
        expect("term", "*te*m*", true, "missing letter in query");
        expect("term", "*te*a*rm*", false, "additional letter in query");
        expect("term", "*rm*te*", false, "reversed subquery order");
        expect("term", "*ter*rm*", false, "minimally overlapping queries");
        expect("term", "*ter*erm*", false, "moderately overlapping queries");
        expect("term", "*term*term*", false, "maximally overlapping queries");
        printf("\n");

        // Start of Krauss Tests from http://www.drdobbs.com/architecture-and-design/matching-wildcards-an-empirical-way-to-t/240169123#ListingOne
        printf("Starting Krauss Tests\n");

        // Cases with repeating character sequences.
        expect("abcccd", "*ccd", true, " ");
        if (ARBITRARY_TERM_LENGTH) {
            expect("mississipissippi", "*issip*ss*", true, "");
            if (WILDCARD_IN_TERM) {
                expect("xxxx*zzzzzzzzy*f", "xxxx*zzy*fffff", false, "");
                expect("xxxx*zzzzzzzzy*f", "xxx*zzy*f", true, "");
            }
            expect("xxxxzzzzzzzzyf", "xxxx*zzy*fffff", false, "");
            expect("xxxxzzzzzzzzyf", "xxxx*zzy*f", true, "");
            expect("xyxyxyzyxyz", "xy*z*xyz", true, "");
            expect("mississippi", "*sip*", true, "");
            expect("xyxyxyxyz", "xy*xyz", true, "");
            expect("mississippi", "mi*sip*", true, "");
        }
        expect("ababac", "*abac*", true, " ");
        expect("aaazz", "a*zz*", true, " ");
        expect("a12b12", "*12*23", false, " ");
        expect("a12b12", "a12b", false, " ");
        expect("a12b12", "*12*12*", true, " ");

        // Additional cases where the '*' char appears in the tame string.
        if (WILDCARD_IN_TERM) {
            expect("*", "*", true, "");
            expect("a*abab", "a*b", true, "");
            expect("a*r", "a*", true, "");
            expect("a*ar", "a*aar", false, "");
        }

        // More double wildcard scenarios.
        if (ARBITRARY_TERM_LENGTH) {
            expect("XYXYXYZYXYz", "XY*Z*XYz", true, "");
            expect("missisSIPpi", "*SIP*", true, "");
            expect("mississipPI", "*issip*PI", true, "");
            expect("xyxyxyxyz", "xy*xyz", true, "");
            expect("miSsissippi", "mi*sip*", true, "");
            expect("miSsissippi", "mi*Sip*", false, "");
            expect("abAbac", "*Abac*", true, "");
            expect("abAbac", "*Abac*", true, "");
            expect("aAazz", "a*zz*", true, "");
            expect("A12b12", "*12*23", false, "");
            expect("a12B12", "*12*12*", true, "");
            expect("oWn", "*oWn*", true, "");
        }

        // Completely tame (no wildcards) cases.
        expect("bLah", "bLah", true, " ");
        expect("bLah", "bLaH", false, " ");

        // Simple mixed wildcard tests suggested by IBMer Marlin Deckert.
        if (SINGLE_CHARACATER_WILDCARDS) {
            expect("a", "*?", true, "");
            expect("ab", "*?", true, "");
            expect("abc", "*?", true, "");
        }

        // More mixed wildcard tests including coverage for false positives.
        if (SINGLE_CHARACATER_WILDCARDS) {
            expect("a", "??", false, "");
            expect("ab", "?*?", true, "");
            expect("ab", "*?*?*", true, "");
            expect("abc", "?**?*?", true, "");
            expect("abc", "?**?*&?", false, "");
            expect("abcd", "?b*??", true, "");
            expect("abcd", "?a*??", false, "");
            expect("abcd", "?**?c?", true, "");
            expect("abcd", "?**?d?", false, "");
            expect("abcde", "?*b*?*d*?", true, "");
        }

        // Single-character-match cases.
        if (SINGLE_CHARACATER_WILDCARDS) {
            expect("bLah", "bL?h", true, "");
            expect("bLaaa", "bLa?", false, "");
            expect("bLah", "bLa?", true, "");
            expect("bLaH", "?Lah", false, "");
            expect("bLaH", "?LaH", true, "");
        }

        // Many-wildcard scenarios.
        if (ARBITRARY_TERM_LENGTH) {
            expect(
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                            "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab",
                    "a*a*a*a*a*a*aa*aaa*a*a*b", true, "");
            expect(
                    "abababababababababababababababababababaacacacacacacaca"
                            "daeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                    "*a*b*ba*ca*a*aa*aaa*fa*ga*b*", true, "");
            expect(
                    "abababababababababababababababababababaacacacacacacaca"
                            "daeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                    "*a*b*ba*ca*a*x*aaa*fa*ga*b*", false, "");
            expect(
                    "abababababababababababababababababababaacacacacacacaca"
                            "daeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                    "*a*b*ba*ca*aaaa*fa*ga*gggg*b*", false, "");
            expect(
                    "abababababababababababababababababababaacacacacacacacadae"
                            "afagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab",
                    "*a*b*ba*ca*aaaa*fa*ga*ggg*b*", true, "");
            expect(
                    "aaabbaabbaab",
                    "*aabbaa*a*", true, "");
            expect(
                    "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*",
                    "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", true, "");
            expect(
                    "aaaaaaaaaaaaaaaaa",
                    "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", true, "");
            expect(
                    "aaaaaaaaaaaaaaaa",
                    "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", false, "");
            expect(
                    "abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefg"
                            "hij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn",
                    "abc*abc*abc*abc*abc*abc*abc*abc*abc*"
                            "abc*abc*abc*abc*abc*abc*abc*abc*", false, "");
            expect(
                    "abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghi"
                            "j*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn",
                    "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*", true, "");
            expect(
                    "abc*abcd*abcd*abc*abcd",
                    "abc*abc*abc*abc*abc", false, "");
            expect(
                    "abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd",
                    "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd", true, "");
            expect(
                    "********a********b********c********",
                    "abc", false, "");
        }
        expect("abc", "********a********b********c********", true, " ");
        expect("abc", "********a********b********b********", false, " ");
        expect("*abc*", "***a*b*c***", true, " ");
        printf("\n");

        printf("Ran %d tests with %d passes and %d failures\n", tests_run, tests_passed, tests_failed);

        end = clock();
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Total time used: %f \n", cpu_time_used);

        if( test == 1) {
            int i = 0;
            int loss_count = 0;
            int win_count = 0;
            double my_total = 0;
            double krauss_total = 0;
            printf("   My Time      |   Krauss Time      |   Result\n");
            for (i = 0; i < 38; i++) {
                if (my_time[ i ] > krauss_time[ i ]) {
                    printf("   %f      |   %f               |   Loss\n", my_time[ i ], krauss_time[ i ]);
                    loss_count++;
                    if( my_time[i] == 1000){
                        my_total += 0;
                    }
                    else {
                        my_total += my_time[ i ];
                    }
                    krauss_total += krauss_time[i];
                } else {
                    printf("   %f      |   %f               |   Win\n", my_time[ i ], krauss_time[ i ]);
                    win_count++;
                    if( my_time[i] == 1000){
                        my_total += 0;
                    }
                    else {
                        my_total += my_time[ i ];
                    }
                    krauss_total += krauss_time[i];
                }
            }
            printf("total: %f      |   %f               |   Win\n", my_total, krauss_total);
            printf("Win: %i.  Loss: %i \n", win_count, loss_count);
        }




    }

    return 0;
}

