#include "bit_board.h"
#include "restrict_black_eval.h"
#include <vector>
#include <set>
#include "threat_eval.h"
#include <iostream>
#include "helper.h"


using namespace std;


// default initialization of lines to be empty - i.e. contain only figure::NONE pieces


bit_board::bit_board() {
    for (int i = 0; i < SIZE; ++i)
        horizontal_[i] = 0x3FFFFFFF; // initialize each line to contain 15 NONE pieces - i.e. 15x the number 0x3
    for (int i = 0; i < SIZE; ++i)
        vertical_[i] = 0x3FFFFFFF; // initialize each line to contain 15 NONE pieces - i.e. 15x the number 0x3
    for (int i = 0; i < SIZE; ++i) {

        /* the pattern, that is being filled here via ugly bit manipulations is the following: (for the $diagonal_ array, $anti_diagonal is horizontally flipped)
         *
         *     X
         *    XX
         *   XXX
         *  XXXX
         *  XXX
         *  XX
         *  X
         *
         *  this represents the diagonal cuts of the board (a1, b1-a2, c1-b2-a3, ...)
         */

        diagonal_[2 * SIZE - 2 - i] = anti_diagonal_[i] = (1 << (2 * (i + 1))) - 1;
        diagonal_[i] = anti_diagonal_[2 * SIZE - 2 - i] = ((1 << (2 * (i + 1))) - 1) << (2 * (SIZE - 1 - i));
    }
}

void bit_board::set_board_str(std::string board_str){
// init from board_str
    std::vector<std::string> horizontal_strs;
    tokenize(board_str, ',', horizontal_strs);
    for(int y = 0; y < 15; y++){
        int i_auto = std::stoi(horizontal_strs[y],nullptr,0);
        for(int x = 0; x < 15; x++){
            figure fig = figure((i_auto >> (x * 2)) & 3);
            if(fig == figure::WHITE || fig == figure::BLACK){
                place_move(coords(x, y), fig);
            }
        }
    }
}

void bit_board::set_steps_str(std::string steps_str){
     vector<std::string> steps;
     tokenize(steps_str, '_', steps);
     figure current_player_ = figure::BLACK;
     for (int i = 0; i < steps.size(); i++) {
         string word = steps[i];
         coords next_move;
         if (!coords::try_parse(word, next_move)) throw runtime_error(word + " is invalid, coordinates expected");
         place_move(next_move, current_player_);
         current_player_ = current_player_ == figure::BLACK ? figure::WHITE : figure::BLACK;
    }
}

void bit_board::apply_all_transformation(vector<bit_board>& trans_boards, vector<vector<bool>>& all_trans){
    for(int is_symmetry_horizontal = 0; is_symmetry_horizontal < 2; is_symmetry_horizontal ++){
        for(int is_symmetry_vertical = 0; is_symmetry_vertical < 2; is_symmetry_vertical ++){
            for(int is_symmetry_anti_diagonal = 0; is_symmetry_anti_diagonal < 2; is_symmetry_anti_diagonal ++){
                bit_board b;
                for(int x = 0; x < 15; x++){
                    for(int y = 0; y < 15; y++){
                        coords current = coords(x, y);
                        figure fig = get_move(current);
                        if(fig == figure::WHITE || fig == figure::BLACK){
                            coords new_coords = coords::apply_trans(current, is_symmetry_horizontal==1, is_symmetry_vertical==1, is_symmetry_anti_diagonal==1);
                            b.place_move(new_coords, fig);
                        }
                    }
                }
                trans_boards.push_back(b);
                all_trans.push_back({is_symmetry_horizontal==1, is_symmetry_vertical==1, is_symmetry_anti_diagonal==1});
            }
        }
    }
}


// initialization of masks that allow operations on the board
// ugly bit operations are done trivially by following the definitions (in header file) of each mask

bit_board::bit_board_masks bit_board::masks_;

bit_board::bit_board_masks::bit_board_masks() {
    for (uint32_t i = 0; i < SIZE; ++i) {
        delete_mask[i] = 3 << (i * 2); // shifting ..00000 11 00000..

        figure_mask[OUTSIDE][i] = ~((uint32_t(OUTSIDE) ^ 3) << (i * 2)); // shifting ...11111 fig 11111...
        figure_mask[WHITE][i] = ~((uint32_t(WHITE) ^ 3) << (i * 2));
        figure_mask[BLACK][i] = ~((uint32_t(BLACK) ^ 3) << (i * 2));
        figure_mask[NONE][i] = ~((~uint32_t(NONE) ^ 3) << (i * 2));
    }

    for (uint32_t radius = 0; radius < 8; ++radius) {
        // this makes the following pattern: 11, 11.11.11, 11.11.11.11.11, ...
        line_mask[radius] = (1 << (2 * (2 * radius + 1))) - 1;
    }
}


// places move on each of the arrays by applying figure masks

void bit_board::place_move(coords pos, figure fig) {
    if (pos.x < 0 || pos.x >= SIZE || pos.y < 0 || pos.y >= SIZE)
        throw runtime_error("position " + pos.to_string() + " is out of board");
    debug_board_.place_move(pos, fig);

    horizontal_[pos.y] &= masks_.figure_mask[fig][pos.x];
    vertical_[pos.x] &= masks_.figure_mask[fig][pos.y];
    diagonal_[pos.y - pos.x + SIZE - 1] &= masks_.figure_mask[fig][pos.x];
    anti_diagonal_[pos.x + pos.y] &= masks_.figure_mask[fig][pos.y];
}


// deletes move from each of the arrays by applying delete masks

void bit_board::delete_move(coords pos) {
    if (pos.x < 0 || pos.x >= SIZE || pos.y < 0 || pos.y >= SIZE)
        throw runtime_error("position " + pos.to_string() + " is out of board");
    debug_board_.delete_move(pos);

    horizontal_[pos.y] |= masks_.delete_mask[pos.x];
    vertical_[pos.x] |= masks_.delete_mask[pos.y];
    diagonal_[pos.y - pos.x + SIZE - 1] |= masks_.delete_mask[pos.x];
    anti_diagonal_[pos.x + pos.y] |= masks_.delete_mask[pos.y];
}

// black heuristic evaluation, return the evaluation result for given last_move
uint16_t bit_board::black_evaluation(coords last_move) {
    // black is the last move
    auto lines = get_lines<5>(last_move);
    bit_board::shrink_lines<5, 4>(lines);
    return restrict_black_eval::evaluate(lines);
}

// counter opponent, last move is always white
set<string> bit_board::counter_opponent(coords last_move) {
    set<string> counter_positions;
    auto lines = get_lines<5>(last_move);
    vector<uint32_t> found_direct;
    for (uint32_t i = 0; i < 4; ++i) {
        /**
         * 0, horizontal_,      F8, G8, H8, I8, J8
         * 1, vertical_,        H6, H7, H8, H9, H10
         * 2, diagonal_,        F6, G7, H8, I9, J10
         * 3, anti_diagonal_,  F10, G9, H8, I7, J6
         */
        auto line = bit_board::uint_to_line<11>(lines[i]);
        set<uint8_t> counter_point = threat_eval::find_counter(line);

        for (auto idx: counter_point) {
            auto diff = idx - 5;
            if (i == 0) {
                coords t = coords(last_move.x + diff, last_move.y);
                counter_positions.insert(t.to_string());
            } else if(i == 1){
                coords t = coords(last_move.x, last_move.y + diff);
                counter_positions.insert(t.to_string());
            } else if(i == 2){
                coords t = coords(last_move.x + diff, last_move.y + diff);
                counter_positions.insert(t.to_string());
            } else {
                coords t = coords(last_move.x - diff, last_move.y + diff);
                counter_positions.insert(t.to_string());
            }
        }
    }
    return counter_positions;
}


coords bit_board::get_move_to_win(figure fig) {
    for(int x = 0; x < 15; x++){
        for(int y = 0; y < 15; y++){
            coords move = coords(x, y);
            if(is_empty(move)){
                auto lines = get_lines<5>(move, fig);
                for (uint32_t i = 0; i < 4; ++i) {
                    /**
                     * 0, horizontal_,      F8, G8, H8, I8, J8
                     * 1, vertical_,        H6, H7, H8, H9, H10
                     * 2, diagonal_,        F6, G7, H8, I9, J10
                     * 3, anti_diagonal_,  F10, G9, H8, I7, J6
                     */
                    auto line = bit_board::uint_to_line<11>(lines[i]);
                    if(threat_eval::free_5_att(line, fig)){
                        return move;
                    }
                }
            }
        }
    }
    return coords::INCORRECT_POSITION;
}

