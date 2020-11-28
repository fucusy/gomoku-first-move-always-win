#ifndef DB_REFUTE_SEARCH_H
#define DB_REFUTE_SEARCH_H

#include "bit_board.h"
#include "threat_sequence.h"
#include "threat_finder.h"
#include "db_node.h"

#include "abstract_db_search.h"


/* DEPENDENCY-BASED REFUTE SEARCH
 * 
 * A very similar class to db_search, which doesn't search for a win, but searches for
 * a refutation of a sequence of moves leading to a win. For more details see the description
 * of db_search.
 * 
 * For speed purposes, it doesn't recursively check the correctness of the refutation (this can 
 * lead to incorrect refutation of an existing win, but never to an incorrect win).
 * 
 */

class db_refute_search: public abstract_db_search {

public:
	db_refute_search(bit_board* board, threat_sequence* threat_seq, threat_finder* finder) : abstract_db_search(board), threat_finder_(finder), threat_sequence_(threat_seq), number_of_returns_(0) {}

	// tries to find a refutation of a winning sequence, if the refutation exists, it
	// return how many moves in the sequence can be refuted
	uint_fast8_t number_of_refutable_moves(figure attacker);

	// pops back $limit moves from the threat_finder and removes the last $limit moves from board, 
	// it is called to return the threat_finder into its initial state
	void pop_back(uint_fast8_t limit);


private:

	// sets the last played move as the root
	void set_root(uint_least8_t max_threat_level) {
		root_ = std::make_unique<db_node>(db_node_type::ROOT, threat_finder_->get_possible_moves(attacker_, max_threat_level), 0);
	}

	// standard db_search method, which doesn't do additional checking of refutation
	bool exists_winning_move(figure attacker, uint_least8_t max_threat_level);
	bool dependency_stage(db_node& node, uint_least8_t max_threat_level);
	bool dependency_stage(db_node& parent, threat_finder_package&& package, uint_least8_t max_threat_level, uint_fast8_t& return_number);

	// removes moves in the winning sequence from the board
	void restart_board();

	// places moves from the winning sequence back to the board
	void ressurrect_board(uint_fast8_t start_position_index);

	bool is_goal_state(dependency_eval_type type, coords attack, uint_least8_t max_threat_level) {
		number_of_returns_ = threat_sequence_->contains(attack);
		return number_of_returns_ > 0 || type == dependency_eval_type::FIVE || (max_threat_level >= 1 && type == dependency_eval_type::OPEN_FOUR);;
	}

	// pushes the position into the threat_finder
	void push_board(const positions_bundle& defence_positions, coords attack_position);
	void push_board(coords attack_position);


	threat_finder* threat_finder_;
	threat_sequence* threat_sequence_;
	uint_fast8_t number_of_returns_;
};

#endif