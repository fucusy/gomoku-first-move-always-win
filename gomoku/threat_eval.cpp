#include "threat_eval.h"
#include "bit_board.h"

#include <array>
#include <vector>
#include <iostream>


using namespace std;


const threat_eval::table threat_eval::cache;


// initialization of the look-up table with the help of helper_evaluate_line()

threat_eval::table::table() {
	for (uint32_t line = 0; line < SIZE; ++line) { // creates any possible combination of 11 consecutive moves (each move is a 2-bit value)
		value[line] = helper_evaluate_line(bit_board::uint_to_line<11>(line));
	}
}


// evaluate each line by looking into the table

uint8_t threat_eval::evaluate(const std::array<uint32_t, 4>& lines) {
	uint8_t output = 0;
	for (int i = 0; i < 4; ++i) {
		output |= cache.value[lines[i]];
	}

	return output;
}

// helping method for initializing the lookup table
// does the calculation of threats "manually"
// uses the following methods to decide what threats are present

uint8_t threat_eval::helper_evaluate_line(array<figure, 11> line) {
	figure player_to_evaluate = line[5];
	
	if (player_to_evaluate == figure::NONE || player_to_evaluate == figure::OUTSIDE)
		return 0;

	uint8_t baseValue = 0;
	if (free_5_att(line, player_to_evaluate)) // if the line is winning, return UINT8_MAX
		return UINT8_MAX;
	if (free_4_att(line, player_to_evaluate) || threat_4_att(line, player_to_evaluate))
		baseValue |= FOUR_ATTACK;
	else if (threat_3_att(line, player_to_evaluate))
		baseValue |= THREE_ATTACK;
	if (threat_4_def(line, player_to_evaluate))
		baseValue |= FOUR_DEFENCE;
	else if (threat_3_def(line, player_to_evaluate))
		baseValue |= THREE_DEFENCE;

	return baseValue;
}


// returns TRUE if player_to_evaluate is defending a threatening three - eg: ...XOXX....
// assumes that figure player_to_evaluate is on line[5]

bool threat_eval::threat_3_def(array<figure, 11>& line, figure player_to_evaluate) {
	figure opposite_player = player_to_evaluate == figure::BLACK ? figure::WHITE : figure::BLACK;

	// checks if the opposite player would have threatening three if player_to_evaluate wasn't at line[5]

	if (line[4] == opposite_player) {
		array<figure, 11> shifted_line;
		for (int i = 0; i < 10; ++i)
			shifted_line[i + 1] = line[i];
		shifted_line[0] = figure::OUTSIDE;
		shifted_line[6] = figure::NONE;

		if (threat_3_att(shifted_line, opposite_player))
			return true;
	}

	if (line[6] == opposite_player) {
		array<figure, 11> shifted_line;
		for (int i = 0; i < 10; ++i)
			shifted_line[i] = line[i + 1];
		shifted_line[10] = figure::OUTSIDE;
		shifted_line[4] = figure::NONE;
		
		if (threat_3_att(shifted_line, opposite_player))
			return true;
	}

	return false;
}


// returns TRUE if player_to_evaluate is defending a thretening four - eg: ...OXXXX....
// assumes that figure player_to_evaluate is on line[5]

bool threat_eval::threat_4_def(std::array<figure, 11>& line, figure player_to_evaluate) {
	figure opposite_player = player_to_evaluate == figure::BLACK ? figure::WHITE : figure::BLACK;

	// checks if the opposite player would have winning five if player_to_evaluate wasn't at line[5]

	line[5] = opposite_player;
	bool returnValue = free_5_att(line, opposite_player);
	line[5] = player_to_evaluate;

	return returnValue;
}


// returns TRUE if player_to_evaluate is threating with a three - eg: ...O.OO....
// assumes that figure player_to_evaluate is on line[5]

bool threat_eval::threat_3_att(std::array<figure, 11>& line, figure player_to_evaluate) {

	// observes if there is way for player_to_evaluate to have a free four in the next move (if there is, then he must have theatening three)

	for (int i = 1; i < 10; ++i) {
		if (line[i] == figure::NONE) {
			line[i] = player_to_evaluate;
			if (free_4_att(line, player_to_evaluate)) {
				line[i] = figure::NONE;
				return true;
			}
			line[i] = figure::NONE;
		}
	}

	return false;
}


// returns TRUE if player_to_evaluate is threatening with a four - eg: ...O.OOO...
// assumes that figure player_to_evaluate is on line[5]

bool threat_eval::threat_4_att(std::array<figure, 11>& line, figure player_to_evaluate) {

	// observes if there is way for player_to_evaluate to have a winning five in the next move (if there is, then he must have theatening four)

	for (int i = 1; i < 10; ++i) {
		if (line[i] == figure::NONE) {
			line[i] = player_to_evaluate;
			if (free_5_att(line, player_to_evaluate)) {
				line[i] = figure::NONE;
				return true;
			}
			line[i] = figure::NONE;
		}
	}

	return false;
}


// returns TRUE if player_to_evaluate has a free four - eg: ...OOOO...
// assumes that figure player_to_evaluate is on line[5]

bool threat_eval::free_4_att(std::array<figure, 11>& line, figure player_to_evaluate) {
	for (int i = 1; i < 5; ++i) {
		if (line[0 + i] == figure::NONE &&
			line[1 + i] == player_to_evaluate &&
			line[2 + i] == player_to_evaluate &&
			line[3 + i] == player_to_evaluate &&
			line[4 + i] == player_to_evaluate &&
			line[5 + i] == figure::NONE) {

			return true;
		}
	}

	return false;
}


// returns TRUE if player_to_evaluate has a winning five - eg: ...OOOOO...
// assumes that figure player_to_evaluate is on line[5]

bool threat_eval::free_5_att(std::array<figure, 11>& line, figure player_to_evaluate) {
	for (int i = 1; i < 6; ++i) {
		if (line[0 + i] == player_to_evaluate &&
			line[1 + i] == player_to_evaluate &&
			line[2 + i] == player_to_evaluate &&
			line[3 + i] == player_to_evaluate &&
			line[4 + i] == player_to_evaluate) {

			return true;
		}
	}

	return false;
}

bool threat_eval::free_5_att_9(std::array<figure, 9>& line, figure player_to_evaluate) {
	for (int i = 1; i < 4; ++i) {
		if (line[0 + i] == player_to_evaluate &&
			line[1 + i] == player_to_evaluate &&
			line[2 + i] == player_to_evaluate &&
			line[3 + i] == player_to_evaluate &&
			line[4 + i] == player_to_evaluate) {

			return true;
		}
	}
	return false;
}

/**
 * this may be improved more to reduce the calculation
 *
 * @param line
 * @return
 */
std::set<uint8_t> threat_eval::find_counter(std::array<figure, 11> line) {
    std::set<uint8_t> s;
    std::vector<uint8_t> empty_point;
    for(int i = 0; i < 11; i ++){
        if(line[i] == figure::NONE){
            empty_point.push_back(i);
        }
    }
    for(int i =0; i < empty_point.size(); i++){
        // j start with i, in case there is only one side empty point available
        for(int j = i; j < empty_point.size(); j++){
            line[empty_point[i]] = line[empty_point[j]] = line[5];
            if(free_5_att(line, line[5])){
                s.insert(empty_point[i]);
                s.insert(empty_point[j]);
            }
            line[empty_point[i]] = line[empty_point[j]] = figure::NONE;
        }
    }
    return s;
}