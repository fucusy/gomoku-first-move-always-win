#ifndef RESTRICT_BLACK_EVAL_H
#define RESTRICT_BLACK_EVAL_H

#include "coords.h"
#include <cstdint>
#include <array>


/* Restrict the black moves as described in Searching for Solutions in Games and Artificial Intelligence
 *
 */

class restrict_black_eval {
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