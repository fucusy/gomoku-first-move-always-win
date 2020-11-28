#ifndef OVERLAP_EVAL_H
#define OVERLAP_EVAL_H

#include "coords.h"

#include <cstdint>
#include <array>


/* OVERLAP EVALUATION
 *
 * A static evaluation used for a fast calculation of the strength of a move.
 * 
 * The strength of a move is calculated using the move itself and moves that are 
 * in line with the move and are less or equal than 4 moves far away.
 * 
 * This creates four lines (for each direction) consisting of 9 moves, that are 
 * then put into this evaluator.
 * 
 * The overlap evaluation works simply by counting how many consecutive friend/enemy 
 * moves are in contact with the requested move. Then a heuristic function is used
 * to determine how good/bad the move is based on this number.
 * 
 * To make the evaluation as fast as possible a look-up table is used to speed up the
 * process of counting the number of consecutive moves.
 * 
 * It is used to reduce the branching of the search tree.
 */

class overlap_eval {	
public:
	static uint16_t evaluate(const std::array<uint32_t, 4>& lines);
	
private:
	// heuristic function: number of consecutive moves -> number repressenting the approximate impact on the game
	const static uint16_t attack_eval[6];
	const static uint16_t defence_eval[6];

	// helper method for initializing the lookup table
	static uint16_t helper_evaluate_line(const std::array<figure, 9>& line);

	// look-up table for fast evaluation
	const static struct table {
		// the look-up table has to be as large as the number of combinations of 9 moves == 4^9 == 2^(2*9)
		const static size_t SIZE = 1 << (2 * 9);

		uint16_t value[SIZE];
		table();
	} cache;
};

#endif