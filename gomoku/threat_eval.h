#ifndef THREAT_EVAL_H
#define THREAT_EVAL_H

#include "coords.h"
#include <set>


/* THREAT EVALUATION
 *
 * A class for evaluation of moves that works very similar to overlap evaluation. The difference
 * is that this doesn't approximate the strength of a move, but decides whether certain move is
 * a threat (i.e. open three, four, ... ) or a defence to a threat.
 * 
 * It is used to find necessary moves in the search tree and to decide what moves are playable at 
 * a certain situation. 
 *
 */

class threat_eval {
public:
	enum advantage {
		FOUR_ATTACK = 8,
		THREE_ATTACK = 4,
		FOUR_DEFENCE = 2,
		THREE_DEFENCE = 1,
		TOTAL_THREE_DEFENCE = FOUR_ATTACK | FOUR_DEFENCE | THREE_DEFENCE
	};

	static uint8_t evaluate(const std::array<uint32_t, 4>& lines);
    static std::set<uint8_t> find_counter(std::array<figure, 11> line);

	// returns TRUE if player_to_evaluate has a winning five - eg: ...OOOOO...
	static bool free_5_att(std::array<figure, 11>& line, figure player_to_evaluate);
	static bool free_5_att_9(std::array<figure, 9>& line, figure player_to_evaluate);

private:
	// reference method for initializing the lookup table
	static uint8_t helper_evaluate_line(std::array<figure, 11> line);

	// returns TRUE if player_to_evaluate is defending a threatening three - eg: ...XOXX....
	static bool threat_3_def(std::array<figure, 11>& line, figure player_to_evaluate);

	// returns TRUE if player_to_evaluate is defending a threatening four - eg: ...OXXXX....
	static bool threat_4_def(std::array<figure, 11>& line, figure player_to_evaluate);

	// returns TRUE if player_to_evaluate is threatening with a three - eg: ...O.OO...
	static bool threat_3_att(std::array<figure, 11>& line, figure player_to_evaluate);

	// returns TRUE if player_to_evaluate is threatening with a four - eg: ...O.OOO...
	static bool threat_4_att(std::array<figure, 11>& line, figure player_to_evaluate);

	// returns TRUE if player_to_evaluate has a free four - eg: ...OOOO...
	static bool free_4_att(std::array<figure, 11>& line, figure player_to_evaluate);


	// look-up table for fast evaluation
	const static struct table {
		// the look-up table has to be as large as the number of combinations of 11 moves == 4^11 == 2^(2*11)
		const static size_t SIZE = 1 << (2 * 11);
		uint8_t value[SIZE];
		table();
	} cache;
};

#endif