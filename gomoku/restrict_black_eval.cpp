#include "math_extension.h"
#include "restrict_black_eval.h"
#include "threat_eval.h"
#include "bit_board.h"
#include <iostream>

#include <cstdint>


using namespace std;


// heuristically estimated constants that seems to be working well
// they represent the evaluation of a certain number of friend/enemy moves in a local area
const uint16_t restrict_black_eval::attack_eval[6] = {2, 30, 100, 500, UINT16_MAX, UINT16_MAX};
const uint16_t restrict_black_eval::defence_eval[6] = {1, 20, 90, 600, 5000, 0};

const restrict_black_eval::table restrict_black_eval::cache;


// initialization of the look-up table with the help of helper_evaluate_line()

restrict_black_eval::table::table() {
    for (uint32_t line = 0;
         line < SIZE; ++line) { // creates any possible combination of 9 consecutive moves (each move is a 2-bit value)

        figure middle_figure = figure((line >> 8) & 3); // gets the middle 2 bits representing the figurein the middle

        if (middle_figure == NONE || middle_figure == OUTSIDE)
            value[line] = 0;
        else
            value[line] = helper_evaluate_line(bit_board::uint_to_line<9>(line));
    }
}


// evaluate each line by looking into the table

uint16_t restrict_black_eval::evaluate(const std::array<uint32_t, 4> &lines) {
    uint16_t output = 0;
    for (int i = 0; i < 4; ++i) {
        output = max(output, cache.value[lines[i]]);
    }
    return output;
}


// helping method for initializing the lookup table
// does the calculation of consecutive moves "manually"

uint16_t restrict_black_eval::helper_evaluate_line(const array<figure, 9> &line) {
    int friend_in_five; // number of friend moves in a five-moves frame
    int opponent_in_five;  // number of friend moves in a five-moves frame
    uint16_t output = 0; // cumulated output
    const figure friend_colour = line[4]; // the middle figure
    const figure opponent_colour = friend_colour == BLACK ? WHITE : BLACK; // reverse colour


    // 4 points for each three with a two-stone reply   ..OOO..
    int start_idx_found = -1;
    for (int start_idx = 2; start_idx <= 4; start_idx++) {
        bool found = true;
        for (int off = 0; off < 3; ++off) {
            if (line[start_idx + off] != friend_colour) {
                found = false;
            }
        }
        if (found) {
            start_idx_found = start_idx;
            break;
        }
    }
    if (start_idx_found != -1) {
        // check other points
        if (line[start_idx_found - 1] == NONE
            && line[start_idx_found - 2] == NONE
            && line[start_idx_found + 3] == NONE
            && line[start_idx_found + 4] == NONE) {
            return 5;
        }
    }

    // 3 points for each three with a three-stone reply   ..OOO.X  X.OOO..
    start_idx_found = -1;
    for (int start_idx = 2; start_idx <= 4; start_idx++) {
        bool found = true;
        for (int off = 0; off < 3; ++off) {
            if (line[start_idx + off] != friend_colour) {
                found = false;
            }
        }
        if (found) {
            start_idx_found = start_idx;
            break;
        }
    }
    if (start_idx_found != -1) {
        // check other points
        if (line[start_idx_found - 1] == NONE
            && line[start_idx_found - 2] == NONE
            && line[start_idx_found + 3] == NONE
            && line[start_idx_found + 4] == opponent_colour) {
            return 4;
        }

        if (line[start_idx_found - 1] == NONE
            && line[start_idx_found - 2] == opponent_colour
            && line[start_idx_found + 3] == NONE
            && line[start_idx_found + 4] == NONE) {
            return 4;
        }
    }

    // 3 points for each broken three .OO.O.
    if (line[3] == NONE
        && line[4] == friend_colour
        && line[5] == friend_colour
        && line[6] == NONE
        && line[7] == friend_colour
        && line[8] == NONE) {
        return 3;
    }
    if (line[2] == NONE
        && line[3] == friend_colour
        && line[4] == friend_colour
        && line[5] == NONE
        && line[6] == friend_colour
        && line[7] == NONE) {
        return 3;
    }
    if (line[0] == NONE
        && line[1] == friend_colour
        && line[2] == friend_colour
        && line[3] == NONE
        && line[4] == friend_colour
        && line[5] == NONE) {
        return 3;
    }

    // 3 points for each broken three  .O.OO.

    if (line[3] == NONE
        && line[4] == friend_colour
        && line[5] == NONE
        && line[6] == friend_colour
        && line[7] == friend_colour
        && line[8] == NONE) {
        return 3;
    }
     if (line[1] == NONE
        && line[2] == friend_colour
        && line[3] == NONE
        && line[4] == friend_colour
        && line[5] == friend_colour
        && line[6] == NONE) {
        return 3;
    }
     if (line[0] == NONE
        && line[1] == friend_colour
        && line[2] == NONE
        && line[3] == friend_colour
        && line[4] == friend_colour
        && line[5] == NONE) {
        return 3;
    }

    // 3 points for four thread, add ability to search
    /**
     .OOOO
     O.OOO
     OO.OO
     OOO.O
     OOOO.
    **/
    bool search_form_four = true;
    //search_form_four = false;
    if(search_form_four){
        array<figure, 9> new_line;
        for(int i = 0; i < 9; i++){
            new_line[i] = line[i];
        }
        for (int i = 0; i < 9; ++i) {
            if (new_line[i] == NONE) {
                new_line[i] = friend_colour;
                if (threat_eval::free_5_att_9(new_line, friend_colour)) {
                    new_line[i] = NONE;
                    return 3;
                }
                new_line[i] = NONE;
            }
        }
    }


    // 2 points for each open two, which is defined as two black stones in the center ..OO..
    if (line[2] == NONE
        && line[3] == NONE
        && line[4] == friend_colour
        && line[5] == friend_colour
        && line[6] == NONE
        && line[7] == NONE) {
        return 2;
    }
    if (line[1] == NONE
        && line[2] == NONE
        && line[3] == friend_colour
        && line[4] == friend_colour
        && line[5] == NONE
        && line[6] == NONE) {
        return 2;
    }



    // 1 point for each broken two, which is defined as two black stones with one gap in the center  ..O.O..
    if (line[2] == NONE
        && line[3] == NONE
        && line[4] == friend_colour
        && line[5] == NONE
        && line[6] == friend_colour
        && line[7] == NONE
        && line[8] == NONE) {
        return 1;
    }

    if (line[0] == NONE
        && line[1] == NONE
        && line[2] == friend_colour
        && line[3] == NONE
        && line[4] == friend_colour
        && line[5] == NONE
        && line[6] == NONE) {
        return 1;
    }


    return 0;
}
