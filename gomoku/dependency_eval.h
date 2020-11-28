#ifndef DEPENDENCY_EVAL_H
#define DEPENDENCY_EVAL_H

#include "coords.h"
#include "bit_board.h"
#include "static_vector.h"
#include "threat_positions.h"

#include <vector>
#include <array>


// types of threats
enum class dependency_eval_type {
	FIVE,
	OPEN_FOUR,
	FOUR,
	THREE,
	POSSIBLE_THREAT,
	NOTHING
};

using response = std::vector<int_least8_t>;

// structure returned from the dependency_eval
struct dependency_eval_package {
	dependency_eval_type type;
	response friendly_response;
	response enemy_response;

	dependency_eval_package(dependency_eval_type type, response&& friendly_response, response&& enemy_response) :
		type(type), friendly_response(friendly_response), enemy_response(enemy_response) {}

	bool operator ==(const dependency_eval_package& other) const;
	bool operator !=(const dependency_eval_package& other) const { return !(*this == other); }
};


/* DEPENDENCY EVALUATION
 * 
 * Static evaluation of moves, that has the neighbours of a move as an input and returns
 * the type of threat that the move is, defence responses to that threat and possible threat
 * that are dependent on that move.
 * 
 * The algorithm of this evaluation is similar to overlap_eval and is better described there.
 * 
 * This evaluation is used in dependency-based search for fast searching in the threat-space.
 * 
 * The evaluation first converts a line of moves into a number, that is then used as an index
 * of a lookup table. The output of this lookup table is then used as an index of a (smaller,
 * i.e. <= 255 items) vector containing the final results.
 * 
 */

class dependency_eval {
private:
	
	// lookup table for fast evaluation
	static struct table {
		// the look-up table has to be as large as the number of combinations of 9 moves == 4^9 == 2^(2*9)
		const static size_t SIZE = 1 << (2 * 9);

		uint_least8_t indices[SIZE]; // table of indices into $results
		std::vector<dependency_eval_package> results;

		table();

	} cache_;


public:
	static dependency_eval_package evaluate(uint32_t line) {
		return cache_.results[cache_.indices[line]];
	}

	static std::array<dependency_eval_package, 4> evaluate(const bit_lines& lines) {
		return std::array<dependency_eval_package, 4> { cache_.results[cache_.indices[lines[0]]], cache_.results[cache_.indices[lines[1]]],
												  cache_.results[cache_.indices[lines[2]]], cache_.results[cache_.indices[lines[3]]] };
	}

	// converts the relative offsets from $base in a $direction to an absolute position
	static positions_bundle offsets_to_positions(const response& offsets, coords base, direction dir);
	
private:

	// adds a line into the lookup table
	static uint_least8_t add_line(std::array<figure, 9> line);

	// updates the lookup table
	static uint_least8_t update_results(dependency_eval_type type, response&& friendly_response, response&& enemy_response);

	// all of the following methods "manually" searches the input line in order to cache the
	// result into the lookup table


	// if it is a possible threat, return friendly response as parameter $friendly_response
	// parameter $line is being changed inside, but is reverted to its initial state after return
	static bool is_possible_threat(std::array<figure, 9>& line, figure player_to_evaluate, response& friendly_response);

	// parameter $line is being changed inside, but is reverted to its initial state after return
	static bool is_three(std::array<figure, 9>& line, figure player_to_evaluate);

	// if it is a possible threat, return friendly response as parameter $friendly_response and enemy response as parameter $enemy_response
	// parameter $line is being changed inside, but is reverted to its initial state after return
	static bool is_three(std::array<figure, 9>& line, figure player_to_evaluate, response& friendly_response, response& enemy_response);

	// parameter $line is being changed inside, but is reverted to its initial state after return
	static bool is_four(std::array<figure, 9>& line, figure player_to_evaluate);

	// if it is a possible threat, return exact type of threat as $typy, friendly response as parameter $friendly_response and enemy response 
	// as parameter $enemy_response
	// parameter $line is being changed inside, but is reverted to its initial state after return
	static bool is_four(std::array<figure, 9>& line, figure player_to_evaluate, dependency_eval_type& type, response& friendly_response, response& enemy_response);

	// parameter $line is being changed inside, but is reverted to its initial state after return
	static bool is_five(std::array<figure, 9>& line, figure player_to_evaluate);
};

#endif