#ifndef BOARD_LIST_SELECTOR_H
#define BOARD_LIST_SELECTOR_H

#include "coords.h"
#include "bit_board.h"

#include <array>


struct board_list_item;
using offset = uint_fast8_t; // index in a board_list

// double-linked list inside a 15x15 board
using board_list = std::array<board_list_item, constants::BOARD_SIZE*constants::BOARD_SIZE + 1>; // 15x15 board + head; 


/* BOARD LIST ITEM
 *
 * A node in a double-linked list representing a possible move.
 * 
 */

struct board_list_item {
	coords position;
	uint16_t score[2]; // overlap evaluation for BLACK and WHITE; impossible value { UINT16_MAX, UINT16_MAX } indicates, 
	                   // that the score wasn't yet evaluated
	uint8_t threats[2]; // threat evaluation for BLACK and WHITE

	bool is_empty;
	offset next;
	offset previous;

	const static offset NULL_OFFSET = constants::BOARD_SIZE*constants::BOARD_SIZE;

	board_list_item() : is_empty(true), next(NULL_OFFSET), previous(NULL_OFFSET) { reset(); }
	offset get_offset() const { return constants::BOARD_SIZE * position.y + position.x; }

	// reset the evaluated scores
	void reset() { score[0] = score[1] = UINT16_MAX; }

	// update the evaluated scores
	void update(uint16_t black_score, uint16_t white_score, uint8_t black_threat, uint8_t white_threat) {
		score[0] = black_score; score[1] = white_score;	
		threats[0] = black_threat; threats[1] = white_threat;
	}

	// delete the node from the double-linked list
	void erase(board_list& board) {
		board[next].previous = previous;
		board[previous].next = next; 
		is_empty = true; 
	}

	// add the node to the double-linked list (without evaluation!)
	void add(board_list& board) {
		previous = NULL_OFFSET;
		next = board[NULL_OFFSET].next;
		board[NULL_OFFSET].next = board[next].previous = get_offset();
		is_empty = false;
	}

	// add the node to the double-linked list
	void add(board_list& board, uint16_t black_score, uint16_t white_score, uint8_t black_threat, uint8_t white_threat) {
		previous = NULL_OFFSET;
		next = board[NULL_OFFSET].next;
		board[NULL_OFFSET].next = board[next].previous = get_offset();

		score[0] = black_score; score[1] = white_score;
		threats[0] = black_threat; threats[1] = white_threat;
		is_empty = false;
	}
};


/* BOARD LIST SELECTOR
 * 
 * Selects the best answers to a certain position on board and incrementally changes the set of possible 
 * answers as new moves are introduced.
 * 
 * It keeps the moves in a double-link list, that is statically allocated in a array to prevent expensive
 * allocations on the heap.
 * 
 */

class board_list_selector {
public:
	board_list_selector();

	// updates and evaluates the possible moves 
	void full_update(const bit_board& board, coords position);

	// updates the possible moves without evaluating them
	void shallow_update(const bit_board& board, coords position);

	// evaluate the still unevaluated moves and calculate the best reachable score
	void assign_scores(const bit_board& board, figure colour);

	// get the first move (really the first valid move, not invalid head of the list)
	const board_list_item& get_first() { return board_list_[board_list_[board_list_item::NULL_OFFSET].previous]; }

	// get a move with certain offset (used to access the next/previous node)
	const board_list_item& get_item(offset index) { return board_list_[index]; }

	// get the best evaluation of all of the possible moves (should be called after calling function assign_scores() )
	uint16_t best_score() const { return best_score_; }

	int size() {
	    int size = 0;
        auto move = get_first();
        while (move.get_offset() != board_list_item::NULL_OFFSET) { // loop over all possible moves
            size ++;
            move = get_item(move.previous);
        }
        return size;
	};

private:

	static offset index(offset x, offset y) { return y * constants::BOARD_SIZE + x; }
	static offset index(coords pos) { return pos.y * constants::BOARD_SIZE + pos.x; }

	board_list board_list_;
	uint16_t best_score_;
};

#endif
