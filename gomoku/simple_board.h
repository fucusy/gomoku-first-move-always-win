#ifndef SIMPLE_BOARD_H
#define SIMPLE_BOARD_H

#include "parameters.h"
#include "coords.h"



/* SIMPLE BOARD
 * 
 * Trivial representation of the gaming board as a 2D array of figures.
 * 
 * It is used in places where the more complicated bit_board doesn't have any use,
 * it is also used to debug the bit_board
 * 
 */

class simple_board {
public:
	simple_board() { for (int y = 0; y < constants::BOARD_SIZE; ++y) for (int x = 0; x < constants::BOARD_SIZE; ++x) board_[x][y] = NONE; }
	void place_move(coords pos, figure fig) { board_[pos.x][pos.y] = fig; }
	void delete_move(coords pos) { board_[pos.x][pos.y] = NONE; }
	figure get_move(coords pos) const { if (pos.is_out_of_board()) return OUTSIDE; else return board_[pos.x][pos.y]; }
	figure get_move(coord x, coord y) const { if (coords(x, y).is_out_of_board()) return OUTSIDE; else return board_[x][y]; }
	bool is_empty(coords pos) const { return board_[pos.x][pos.y] == NONE; }
	bool is_winning(coords pos) const;

private:
	figure board_[constants::BOARD_SIZE][constants::BOARD_SIZE];
};

#endif