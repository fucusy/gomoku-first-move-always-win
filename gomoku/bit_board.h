#ifndef BIT_BOARD_H
#define BIT_BOARD_H

#include "simple_board.h"
#include <stdexcept>
#include <cstdint>
#include <array>
#include <string>
#include "parameters.h"
#include <set>
#include <vector>
#include "helper.h"

using namespace std;


/**
 * For example, the position is H8
 *
 * 0, horizontal_,      F8, G8, H8, I8, J8
 * 1, vertical_,        H6, H7, H8, H9, H10
 * 2, diagonal_,        F6, G7, H8, I9, J10
 * 3, anti_diagonal_,  F10, G9, H8, I7, J6
 *
 */
using bit_lines = std::array<uint32_t, 4>;


/* BIT BOARD
 *
 * https://spin.atomicobject.com/2017/07/08/game-playing-ai-bitboards/
 *
 * Data structure representing the 15x15 board, with functions for adding and retrieving pieces from board.
 * 
 * This data-structure isn't implemented in the most obvious way as a 2D array of figures, but it is implemented
 * in the following way to ensure fast retrieval of a line of pieces to be used in the static evaluator.
 * 
 * The structure consists of 4 arrays, each representing one of the 4 directions. Each array consists of lines of pieces, 
 * where each line is represented as a 4-byte uint with each piece as a 2-bit value.
 * We can thus get an arbitrary line in any direction by shifting and masking one of the lines from this array.
 * 
 * This comes for the price of nontrivial and ugly bit manipulations that are needed to insert and delete a figure into/from
 * the lines that are represented as unsigned integers.
 * 
 */

class bit_board {
public:

	bit_board();
	void set_board_str(std::string board_str);
	void set_steps_str(std::string steps_str);
	std::string to_string() const{
        std::string res = "";
        for (int y = 0; y < constants::BOARD_SIZE; ++y){
            res = res + "," + std::to_string(horizontal_[y]);
        }
        return res.substr(1);
	};

	void apply_all_transformation(vector<bit_board>& trans_boards, vector<vector<bool>>& all_trans);


	void place_move(coords pos, figure fig);
	void delete_move(coords pos);

	// black heuristic evaluation, return the evaluation result for given last_move
    uint16_t black_evaluation(coords last_move);

	// counter opponent, last move is always white
	std::set<std::string> counter_opponent(coords last_move);

	// return invalid coords if not found
	coords get_move_to_win(figure fig);

	figure get_move(coords pos) const { return get_move(pos.x, pos.y); }
	bool is_empty(coords pos) const { return get_move(pos.x, pos.y) == figure::NONE; }

	figure get_move(coord x, coord y) const {
		#ifdef _DEBUG
			if (coords(x, y).is_out_of_board()) throw std::runtime_error("position " + coords(x, y).to_string() + " is out of board");
		#endif

		// shift the line to get the right x coord and then mask it to get only one figure 
		return figure((horizontal_[y] >> (x * 2)) & 3);
	}

	// returns a line of figures which are represented as 2-bit values (according to figures enumeration)
	// the middle of the line is at *pos* and its length is 2*Radius + 1
	template<uint8_t Radius>
	uint32_t get_line(coords pos, direction dir) const;

	// returns a line of figures which are represented as 2-bit values (according to figures enumeration)
	// the middle of the line is at *pos* and its length is 2*Radius + 1
	template<uint8_t Radius>
	uint32_t get_line(coords pos, figure middle_figure, direction dir) const {
		return get_line<Radius>(pos, dir) & masks_.figure_mask[middle_figure][Radius];
	}

	// returns a lines of figures which are represented as 2-bit values (according to figures enumeration)
	// returns four lines in each direction, the middle of the lines is at *pos* and their length is 2*Radius + 1
	template<uint8_t Radius>
	bit_lines get_lines(coords pos) const;

	// returns a line of figures which are represented as 2-bit values with *middle_figure* in the middle
	template<uint8_t Radius>
	bit_lines get_lines(coords pos, figure middle_figure) const;

	// converts an array of figures into a comprimed representation of line with figures as 2-bit values
	template<size_t K>
	static uint32_t line_to_uint(std::array<figure, K> line);

	// converts a comprimed representation of line with figures as 2-bit values into an array of figures
	template<size_t K>
	static std::array<figure, K> uint_to_line(uint32_t line);

	// shrinks the lines by I-O figures from both sides
	template<uint8_t I, uint8_t O>
	static void shrink_lines(bit_lines& lines) {
		for(int i = 0; i < 4; ++i)
			lines[i] = (lines[i] >> (2 * (I - O))) & masks_.line_mask[O];
	}
    simple_board debug_board_;

private:

	static const int SIZE = constants::BOARD_SIZE;

	// horizontal, vertical, ... lines of board, each line is an uint32_t, in which is each piece encoded in 2 bits
	uint32_t horizontal_[SIZE];
	uint32_t vertical_[SIZE];
	uint32_t diagonal_[2*SIZE - 1];
	uint32_t anti_diagonal_[2* SIZE - 1];

	// bit masks for fast manipulation with lines
	struct bit_board_masks {
		uint32_t delete_mask[SIZE]; // ORing this mask causes erasing the piece
		uint32_t figure_mask[4][SIZE]; // ANDing this mask on a "NONE" piece puts the figure (represented as the first dimension) on board
		uint32_t line_mask[8]; // ANDing this mask erases redundant pieces from gathered line

		bit_board_masks();
	};

	static bit_board_masks masks_;

};


// returns a line of figures which are represented as 2-bit values (according to figures enumeration)
// the middle of the line is at *pos* and its length is 2*Radius + 1

template <uint8_t Radius>
uint32_t bit_board::get_line(coords pos, direction dir) const {

	// first select the right array according to the requested direction

	// when the right array is selected, we have to choose the right line, then shift it
	// so that the position $pos is in the middle of the returned line and finally unmask the excessive
	// figures by masking them

	switch (dir) {
	case direction::HORIZONTAL:
		if (pos.x >= Radius)
			return (horizontal_[pos.y] >> (2 * (pos.x - Radius))) & masks_.line_mask[Radius];
		return (horizontal_[pos.y] << (2 * (Radius - pos.x))) & masks_.line_mask[Radius];
	case direction::VERTICAL:
		if (pos.y >= Radius)
			return (vertical_[pos.x] >> (2 * (pos.y - Radius))) & masks_.line_mask[Radius];
		return (vertical_[pos.x] << (2 * (Radius - pos.y))) & masks_.line_mask[Radius];
	case direction::DIAGONAL:
		if (pos.x >= Radius)
			return (diagonal_[pos.y - pos.x + SIZE - 1] >> (2 * (pos.x - Radius))) & masks_.line_mask[Radius];
		return (diagonal_[pos.y - pos.x + SIZE - 1] << (2 * (Radius - pos.x))) & masks_.line_mask[Radius];
	default:
		if (pos.y >= Radius)
			return (anti_diagonal_[pos.x + pos.y] >> (2 * (pos.y - Radius))) & masks_.line_mask[Radius];
		return (anti_diagonal_[pos.x + pos.y] << (2 * (Radius - pos.y))) & masks_.line_mask[Radius];
	}
}


// returns a line of figures which are represented as 2-bit values (according to figures enumeration)
// the middle of the line is at *pos* and its length is 2*Radius + 1

template <uint8_t Radius>
bit_lines bit_board::get_lines(coords pos) const {
	bit_lines output;

	// for each direction, we have to choose the right line, then shift it so that the position $pos is in 
	// the middle of the returned line and finally unmask the excessive figures by masking them

	if (pos.x >= Radius) {
		output[0] = (horizontal_[pos.y] >> (2 * (pos.x - Radius))) & masks_.line_mask[Radius];
		output[2] = (diagonal_[pos.y - pos.x + SIZE - 1] >> (2 * (pos.x - Radius))) & masks_.line_mask[Radius];
	}
	else {
		output[0] = (horizontal_[pos.y] << (2 * (Radius - pos.x))) & masks_.line_mask[Radius];
		output[2] = (diagonal_[pos.y - pos.x + SIZE - 1] << (2 * (Radius - pos.x))) & masks_.line_mask[Radius];
	}

	if (pos.y >= Radius) {
		output[1] = (vertical_[pos.x] >> (2 * (pos.y - Radius))) & masks_.line_mask[Radius];
		output[3] = (anti_diagonal_[pos.x + pos.y] >> (2 * (pos.y - Radius))) & masks_.line_mask[Radius];
	}
	else {
		output[1] = (vertical_[pos.x] << (2 * (Radius - pos.y))) & masks_.line_mask[Radius];
		output[3] = (anti_diagonal_[pos.x + pos.y] << (2 * (Radius - pos.y))) & masks_.line_mask[Radius];
	}

	return output;
}


// returns a line of figures which are represented as 2-bit values with *middle_figure* in the middle

template <uint8_t Radius>
bit_lines bit_board::get_lines(coords pos, figure middle_figure) const {
	#ifdef _DEBUG
		if (!is_empty(pos)) throw std::runtime_error("the middle position should be empty when calling this function");
	#endif

	auto output = get_lines<Radius>(pos);

	for (int i = 0; i < 4; ++i)
		output[i] &= masks_.figure_mask[middle_figure][Radius]; // puts the required figure in the middle of the line

	return output;
}


// converts an array of figures into a comprimed representation of line with figures as 2-bit values

template<size_t K>
uint32_t bit_board::line_to_uint(std::array<figure, K> line) {
	uint32_t output = 0;

	for(size_t i = 0; i < K; ++i) {
		output |= line[i] << 2 * i;
	}

	return output;
}


// converts a comprimed representation of line with figures as 2-bit values into an array of figures

template <size_t K>
std::array<figure, K> bit_board::uint_to_line(uint32_t line) {
	std::array<figure, K> output;

	for(int i = 0; i < K; ++i) {
		output[i] = figure((line >> (2 * i)) & 3);
	}

	return output;
}

#endif