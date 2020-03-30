/*Goal is to create a siplified way of moving between two MAX SIZE values
 *This method should allow for seamless transition from one to the other
 *During the check phase of the search.*/

//if last position
//encoded_window = text encoded by character
//text_window = literal representation of text
//next_window = literal representation of text
//st[] = the text
if((encoded_window & location_mask[7]) == location_mask[7]){
    //make new window
    int sev_counter;
    for(sev_counter = 6; sev_counter >= 0; --sev_counter){
         next_window <<= 8;
         next_window |= (uint64_t) st[sev_counter + text_modifier];
    }

    //add the last value of text_window to the beginning of the next window
    next_window |= text_window & location_mask[0];

    // When we do the above we are "joining" them together in such a way that resuming the search process in next_window
    // should be seamless. The only downside is that the register effectievly looses 1 byte of space. This poses less of
    // a problem when dealing with two 64 bit registers, but is still less than ideal.
    // it might be possible to shift that transfered character out of next_window. However, it might result in problems
    // down the road where we have to repeat the process multiple times which would be even more costly.

    uint64_t char_to_check = character;

    uint64_t char_mask = f_m[ char_to_check ];
    n_encoded_window = ~(char_mask ^ next_window);
    n_subquery_matches = LAST_BITS_ON;
    int match_count;
    for( match_count = 0; match_count < 8; match_count++){
        n_subquery_matches &= n_encoded_window;
        n_encoded_window >>= 1;
    } // will always return non-0 n_encoded_window because we transplanted the character into it
}