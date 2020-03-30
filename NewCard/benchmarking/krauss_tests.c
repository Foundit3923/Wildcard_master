#include "krauss.h"

// For printing with colors
#define COLOR_RED   "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"

// Feature set to test for
#define ARBITRARY_TERM_LENGTH false
#define SINGLE_CHARACATER_WILDCARDS false
#define WILDCARD_IN_TERM false

int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

void expect(char term[], char query[], bool expectation, char message[])
{
    printf("Krauss Tests: \n");
    printf("___________________________________________________________________");
    bool result = kraussListingTwo(term, query);

    char *expected_return;
    if (expectation == true) expected_return = "true";
    else expected_return = "false";

    char *success;
    if (result == expectation) success = "PASSED";
    else success = "FAILED";

    char *actual_return;
    if (result == true) actual_return = "true";
    else actual_return = "false";

    char output[100];
    if (strlen(message) == 0)
    {
        sprintf(output, "TEST %s: Returns %s on Query: %s, Term: %s. %s is expected\n", success, actual_return, query, term, expected_return);
    } else {
        sprintf(output, "TEST %s: Returns %s on %s. %s is expected\n" , success, actual_return, message, expected_return);
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
}

int main()
{
    printf("Testing undefined behavior returning false\n");
    // TODO: Define these undefined behaviors. Right now they return false
    expect("123456789", "query", false, "too long of a term");
    expect("", "query", false, "empty term");
    expect("term", "", false, "empty query");
    expect("", "*", false, "empty term and lone wildcard");
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
    expect("abcccd", "*ccd", true, "");
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
    expect("ababac", "*abac*", true, "");
    expect("aaazz", "a*zz*", true, "");
    expect("a12b12", "*12*23", false, "");
    expect("a12b12", "a12b", false, "");
    expect("a12b12", "*12*12*", true, "");

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
    expect("bLah", "bLah", true, "");
    expect("bLah", "bLaH", false, "");

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
    expect("abc", "********a********b********c********", true, "");
    expect("abc", "********a********b********b********", false, "");
    expect("*abc*", "***a*b*c***", true, "");
    printf("\n");

    printf("Ran %d tests with %d passes and %d failures", tests_run, tests_passed, tests_failed);

}
