#ifndef THREAT_FINDER_H
#define THREAT_FINDER_H

#include "threat_positions.h"
#include "dependency_eval.h"
#include "bit_board.h"

#include <stack>


struct threat_finder_package;
using possible_positions = std::vector<threat_finder_package>;

// output from the thread_finder
struct threat_finder_package {
	threat_positions positions;
	direction dir;
	dependency_eval_type threat_type;

	threat_finder_package(threat_positions&& positions, direction dir, dependency_eval_type threat_type) 
		: positions(positions), dir(dir), threat_type(threat_type) { }

	threat_finder_package(coords attack_positions, positions_bundle&& defence_positions, direction dir, dependency_eval_type threat_type)
		: positions(threat_positions(attack_positions, std::move(defence_positions), threat_to_level(threat_type))), dir(dir), threat_type(threat_type) { }

	threat_finder_package(coords attack_positions, positions_bundle&& defence_positions, direction dir, dependency_eval_type threat_type, uint_least8_t level)
		: positions(threat_positions(attack_positions, std::move(defence_positions), level)), dir(dir), threat_type(threat_type) { }

	// substracts vector b from vector a
	static possible_positions substract_threats(const possible_positions& a, const possible_positions& b);

	bool operator ==(const threat_finder_package& other) const { return positions == other.positions; }
	bool operator !=(const threat_finder_package& other) const { return positions != other.positions; }
	
	static uint_least8_t threat_to_level(dependency_eval_type type);

	// simple ordering that can be used for a fast artificial sort of this structure
	static bool ordinal_comparer(const threat_finder_package& a, const threat_finder_package& b) {
		return threat_positions::ordinal_comparer(a.positions, b.positions);
	}
};



/* THREAT FINDER
 * 
 * Class mainly created for a fast search of all threats that are on a certain board. It is optimized for repeated
 * insertion and deletion of moves (for example in a DFS search).
 * 
 * The additional functions (like find_dependent_threats() ) can be considered as a wrapper around the threat
 * evaluation.
 * 
 * When adding a move to the board, this structure is updated via update_positions call and when a move is deleted
 * the function pop is used.
 * 
 * The main reason for usage of this overly complicated structure is the need for repeated starts of dependency-based
 * search in the db_refute_search algorithm (in the start of this search, we need to find all threats on the board).
 * 
 * It is algorithmicaly and functionaly very close to the simpler board_list_selector class.
 *  
 */

class threat_finder {
private:
	// ~ uint_least8_t[15][15][4][2]
	using indices_array = std::array<std::array<std::array<std::array<uint_least8_t, constants::BOARD_SIZE>, constants::BOARD_SIZE>, 4>, 2>;
	using possible_positions_pair = std::array<possible_positions, 2>;

	bit_board* board_;
	std::stack<possible_positions_pair> output_positions_;
	std::stack<indices_array> indices_;

	// lookup table for fast evaluation of is_potential_threat() method
	const static struct table {
		bool get[2 * constants::BOARD_SIZE - 1][2 * constants::BOARD_SIZE - 1];
		table();

	} cache_;

public:	
	explicit threat_finder(bit_board* board) : board_(board) { build(); }

	possible_positions get_possible_moves(figure attack_figure, uint_fast8_t max_level) {
		uint_fast8_t index = attack_figure == figure::BLACK ? 0 : 1;

		if (max_level == 2) return output_positions_.top()[index];
		else return filter_output_positions(output_positions_.top()[index], max_level);
	}

	// updates the threat_finder with these new positions
	// more specialized functions are implemented for faster computation
	void update_positions(const positions_bundle& black_positions, const positions_bundle& white_positions);
	void update_positions(const positions_bundle& black_positions, const positions_bundle& white_positions, figure placed_figure);
	void update_positions(coords black_position, const positions_bundle& white_positions, figure placed_figure);
	void update_positions(const positions_bundle& black_positions, coords white_position, figure placed_figure);
	void update_position(coords position, figure placed_figure);

	// pops the last state of the board
	void pop() { output_positions_.pop(); indices_.pop(); }
	void pop_to_base() { while (output_positions_.size() > 1) pop(); }

	// returns threats of $attack_figure's colour that are in the provided bit_board and are depending on $position
	static possible_positions find_dependent_threats(const bit_board& board, coords position, figure attack_figure, uint_fast8_t max_threat_level);

	// returns true if move a and move b can make a potential threat, which effectively means that
	// they are on the same line
	static bool is_potential_threat(coords a, coords b);

private:

	// initialization of the inner structures
	void build();
	void initialize_position(coords pos, figure attack_figure, indices_array& actual_indices, possible_positions_pair& actual_output_positions);
	
	// submethod of update_positions()
	void update_by_attack_position(coords pos, indices_array& actual_indices, possible_positions& actual_output_positions, int_fast8_t figure_index, figure placed_figure);
	void update_by_defence_position(coords pos, indices_array& actual_indices, possible_positions& actual_output_positions, int_fast8_t figure_index, figure placed_figure);

	// outputs only positions that heve the threat level <= than $maximalLevel
	// it is used when for example the only acceptable thrats are open fours
	static possible_positions filter_output_positions(const possible_positions& input_list, uint_fast8_t maximalLevel);

	// deletes the positions from the threat_finder
	// more specialized functions are implemented for faster computation
	static void delete_position(indices_array& indexes, possible_positions_pair& output_positions, coords deleted_position);
	static void delete_position(indices_array& indexes, possible_positions& output_positions, coords deleted_position, uint_fast8_t deleted_index);
	static void delete_position(indices_array& indices, possible_positions& output_positions, coords deleted_position, direction deleted_direction, uint_fast8_t deleted_index);
};

#endif