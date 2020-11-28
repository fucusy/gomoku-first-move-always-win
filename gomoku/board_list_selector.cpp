#include "board_list_selector.h"
#include "selector_helper.h"
#include "overlap_eval.h"
#include "restrict_black_eval.h"
#include "threat_eval.h"
#include <iostream>
#include <string>


using namespace std;


board_list_selector::board_list_selector(): best_score_(0) {
	// initializes the head of the double-linked list
	board_list_[board_list_item::NULL_OFFSET].is_empty = false;
	board_list_[board_list_item::NULL_OFFSET].position = coords(15, 14); // invalid possition corresponding to the NULL_OFFSET
	board_list_[board_list_item::NULL_OFFSET].score[0] = board_list_[board_list_item::NULL_OFFSET].score[1] = UINT16_MAX;
	board_list_[board_list_item::NULL_OFFSET].next = board_list_[board_list_item::NULL_OFFSET].previous = board_list_item::NULL_OFFSET;

	// sets the right position of other nodes
	for(offset y = 0; y < 15; ++y)
		for(offset x = 0; x < 15; ++x)
			board_list_[index(x, y)].position = coords(x, y);
}


// updates and evaluates the possible moves 

void board_list_selector::full_update(const bit_board& board, coords position) {
	// if $position is among the possible moves, remove it
	if (!board_list_[index(position)].is_empty)
		board_list_[index(position)].erase(this->board_list_);


	// update moves that are 1 or 2 pieces apart from $position
	// if the move is not among the possible moves, add it

	for (int i = 0; i < 16; ++i) { // loop over all offset that are 1 or 2 pieces apart from $position
		coords new_pos = position + inner_star[i];
		if (new_pos.is_out_of_board()) continue;
		if (!board.is_empty(new_pos)) continue;

		auto black_lines = board.get_lines<5>(new_pos, figure::BLACK); // get lines in order to evaluate the position
		auto white_lines = board.get_lines<5>(new_pos, figure::WHITE);

		auto black_threat = threat_eval::evaluate(black_lines);
		auto white_threat = threat_eval::evaluate(white_lines);

		bit_board::shrink_lines<5, 4>(black_lines); // shrink the lines, so that they can be put into the overlap evaluator
		bit_board::shrink_lines<5, 4>(white_lines);

		auto black_eval = restrict_black_eval::evaluate(black_lines);
		auto white_eval = overlap_eval::evaluate(white_lines);

		if (board_list_[index(new_pos)].is_empty)
			board_list_[index(new_pos)].add(this->board_list_, black_eval, white_eval, black_threat, white_threat);
		else
			board_list_[index(new_pos)].update(black_eval, white_eval, black_threat, white_threat);
	}


	// update moves that are 3 or 4 pieces apart from $position
	// if the move is not among the possible moves, ignore it (because moves that are farther
	// than 2 pieces aren't likely to be good moves)

	for (int i = 0; i < 16; ++i) {
		coords new_pos = position + middle_star[i];
		if (new_pos.is_out_of_board()) continue;
		if (!board.is_empty(new_pos)) continue;
		if (board_list_[index(new_pos)].is_empty) continue;

		auto black_lines = board.get_lines<5>(new_pos, figure::BLACK);
		auto white_lines = board.get_lines<5>(new_pos, figure::WHITE);

		auto black_threat = threat_eval::evaluate(black_lines);
		auto white_threat = threat_eval::evaluate(white_lines);

		bit_board::shrink_lines<5, 4>(black_lines);
		bit_board::shrink_lines<5, 4>(white_lines);

		auto black_eval = restrict_black_eval::evaluate(black_lines);
		auto white_eval = overlap_eval::evaluate(white_lines);

		board_list_[index(new_pos)].update(black_eval, white_eval, black_threat, white_threat);
	}
}


// updates the possible moves without evaluating them

void board_list_selector::shallow_update(const bit_board& board, coords position) {
	// if $position is among the possible moves, remove it
	if (!board_list_[index(position)].is_empty)
		board_list_[index(position)].erase(this->board_list_);


	// update moves that are 1 or 2 pieces apart from $position
	// if the move is not among the possible moves, add it

	for (int i = 0; i < 16; ++i) {
		coords new_pos = position + inner_star[i];
		if (new_pos.is_out_of_board()) continue;
		if (!board.is_empty(new_pos)) continue;

		if (board_list_[index(new_pos)].is_empty)
			board_list_[index(new_pos)].add(this->board_list_);
		else
			board_list_[index(new_pos)].reset();
	}


	// update moves that are 3 or 4 pieces apart from $position
	// if the move is not among the possible moves, ignore it (because moves that are farther
	// than 2 pieces aren't likely to be good moves)

	for (int i = 0; i < 16; ++i) {
		coords new_pos = position + middle_star[i];
		if (new_pos.is_out_of_board()) continue;
		if (!board.is_empty(new_pos)) continue;
		if (board_list_[index(new_pos)].is_empty) continue;

		board_list_[index(new_pos)].reset();
	}
}


// evaluate the still unevaluated moves and calculate the best reachable score

void board_list_selector::assign_scores(const bit_board& board, figure colour) {
	offset actual_offset = board_list_[board_list_item::NULL_OFFSET].previous;
	best_score_ = 0;

	while (actual_offset != board_list_item::NULL_OFFSET) {

		// if the value is still unevaluated, evaluate and update it
		if(board_list_[actual_offset].score[colour - 1] == UINT16_MAX && board_list_[actual_offset].score[(colour & 3) - 1] == UINT16_MAX) {
			auto lines = board.get_lines<5>(board_list_[actual_offset].position, colour);

			board_list_[actual_offset].threats[colour - 1] = threat_eval::evaluate(lines);

			bit_board::shrink_lines<5, 4>(lines);
			if(colour == figure::WHITE){
                board_list_[actual_offset].score[colour - 1] = overlap_eval::evaluate(lines);
			}else{
			    auto s = restrict_black_eval::evaluate(lines);
                board_list_[actual_offset].score[colour - 1] = s;
            }
		}

		// update the best score
		if (board_list_[actual_offset].score[colour - 1] > best_score_)
			best_score_ = board_list_[actual_offset].score[colour - 1];

		// move to the next possible move
		actual_offset = board_list_[actual_offset].previous;
	}
}
