#include "math_extension.h"
#include "overlap_eval.h"
#include "bit_board.h"

#include <cstdint>


using namespace std;


// heuristically estimated constants that seems to be working well
// they represent the evaluation of a certain number of friend/enemy moves in a local area
const uint16_t overlap_eval::attack_eval[6] = {2, 30, 100, 500, UINT16_MAX, UINT16_MAX};
const uint16_t overlap_eval::defence_eval[6] = {1, 20, 90, 600, 5000, 0};

const overlap_eval::table overlap_eval::cache;


// initialization of the look-up table with the help of helper_evaluate_line()

overlap_eval::table::table() {
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

uint16_t overlap_eval::evaluate(const std::array<uint32_t, 4> &lines) {
    uint16_t output = 0;
    for (int i = 0; i < 4; ++i) {
        output = inf_plus(output, cache.value[lines[i]]);
    }

    return output;
}


// helping method for initializing the lookup table
// does the calculation of consecutive moves "manually"

uint16_t overlap_eval::helper_evaluate_line(const array<figure, 9> &line) {
    int friend_in_five; // number of friend moves in a five-moves frame
    int opponent_in_five;  // number of friend moves in a five-moves frame
    uint16_t output = 0; // cumulated output
    const figure friend_colour = line[4]; // the middle figure
    const figure opponent_colour = friend_colour == BLACK ? WHITE : BLACK; // reverse colour

    // loop over all possible frames of five moves and cumulate the output
    for (int base = 0; base < 5; ++base) {
        friend_in_five = opponent_in_five = 0;

        for (int off = 0; off < 5; ++off) { // loop over all moves in a frame
            if (base + off == 4) continue;
            else if (line[base + off] == friend_colour) ++friend_in_five;
            else if (line[base + off] == opponent_colour) ++opponent_in_five;
            else if (line[base + off] == OUTSIDE) {
                ++friend_in_five;
                ++opponent_in_five;
            }
        }

        // if the opposite player has a move in the frame, than this frame is not beneficial,
        // otherwise add the heuristic evaluation into the output

        if (opponent_in_five == 0) output = inf_plus(output, attack_eval[friend_in_five]);
        else if (friend_in_five == 0) output = inf_plus(output, defence_eval[opponent_in_five]);
    }

    return output;
}
