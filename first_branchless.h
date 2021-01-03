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

//---------//
// GLOBALS //
//---------//

#define LAST_BITS_ON 72340172838076673     // 00000001 repeated 8 times
#define frshift(x,n) (x & (x>>n))
#define haszero(v) (((v) - 0x0101010101010101UL) & ~(v) & 0x8080808080808080UL)
#define hasvalue(x,n) ((x) ^ (LAST_BITS_ON * (n)))

union Window {
    uint64_t* i;
    char* c;
}Window;

union BoolInt {
    uint64_t i;
    bool b[8];
};

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

bool Experimental_wildcard_arbitrary_length_moving_union_save (char* st,
                                                               char* query_array) {

    char* char_ptr;
    char* last;
    char* copy_array = (char*) malloc(sizeof(char));
    char** string_check = (char**) malloc(sizeof(char*));

    strcpy(copy_array, query_array);


    int text_len = strlen(st);
    int query_len = strlen(query_array);
    int query_len_less = query_len -1;
    int text_modifier = query_len - 1;
    int word_size = sizeof(size_t);
    int shift_count;

    union Window t_w;
    t_w.c = &st[text_modifier];

    union BoolInt query_matches;
    query_matches.i = 0;

    bool check = false;
    bool result = false;


    last = &query_array[text_modifier];

    char_ptr = last;

    uint64_t a = 'abc' * 0X101010101010101;

    a = a /'c';

    print_bits_t(a);

    while(text_modifier <= text_len && result != true)  {



        //get query_matches
        if(haszero(hasvalue(*t_w.i,*char_ptr))) {
            query_matches.i = hasvalue(*t_w.i, *char_ptr);
            //query_matches.i = frshift(frshift(frshift(hasvalue(*t_w.i, *char_ptr), 4)&0xF0F0F0F0F0F0F0FUL, 2), 1);
        }

        //if query_matches
        check = query_matches.b > 0;

        char_ptr -= check;

        string_check[0] = last;
        string_check[1] = char_ptr;
        char_ptr = string_check[check];

        text_modifier -= check;

        text_modifier += (8 + shift_count) * !check;

        t_w.c = &st[text_modifier];

        shift_count = ((shift_count+1) * check) + (shift_count * check);

        result = (*char_ptr == '\000');
    }
    return result;
}
