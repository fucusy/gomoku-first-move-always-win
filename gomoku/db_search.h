#ifndef DB_SEARCH_H
#define DB_SEARCH_H

#include "bit_board.h"
#include "threat_sequence.h"
#include "threat_finder.h"
#include "db_node.h"
#include "db_refute_search.h"

#include <memory>
#include <optional>
#include "abstract_db_search.h"


/* DEPENDENCY-BASED SEARCH
 *
 * Class implementing dependency-based search as described in Searching for Solutions in Games and Artificial Intelligence
 * (http://digitalarchive.maastrichtuniversity.nl/fedora/get/guid:36b5cf0a-cf06-4602-afdb-1af04d65c23b/ASSET1)
 * 
 * It is used to search through the threat-space, i.e. the space consisting only of sequences of moves that are threatening
 * to win against the opposite player (like for example an open three: __X_XX__). To further simplify the search and reduce
 * the branching, the threatening move and the defence moves of the opposite player are joined together and played as one 
 * move. This makes the search really fast without producing false positives (but not all wins are found, though).
 * 
 * After a sequence of threatening moves leading to a win is found, it must be tested, if the win can't be refuted (by occupying
 * the opponent's place a by a counter-win). This is checked by db_refute_search, which works similarly to this search.
 * 
 * In order to search a win fast, a structure that produces all responses to an arbitrary threat is needed. This is what class
 * threat_finder is used for.
 *
 */
	
class db_search: public abstract_db_search {
public:
	explicit db_search(bit_board* board) : abstract_db_search(board), threat_finder_(board), refute_search_(board, &threat_sequence_, &threat_finder_), refuted_moves_(0) {}

	// returns a move that leads to a certain win
	std::optional<coords> get_winning_move(figure attacker, uint_least8_t max_threat_level);

	// debug method for printing a sequence of moves leading to a threat
	std::string winning_sequence_to_string() const { return threat_sequence_.to_string(); }

private:

	// sets the last played move as the root
	void set_root(uint_least8_t max_threat_level) {
		root_ = std::make_unique<db_node>(db_node_type::ROOT, threat_finder_.get_possible_moves(attacker_, max_threat_level), 0);
	}

	// computes the dependency stage of the search (as explained in the paper)
	// returns true if a win was found
	bool dependency_stage(db_node& node, uint_least8_t max_threat_level);
	bool dependency_stage(db_node& parent, threat_finder_package&& package, uint_least8_t max_threat_level, uint_fast8_t& return_number);

	// returns true if the goal state (win) is reached
	static bool is_goal_state(dependency_eval_type type, coords attack, uint_least8_t max_threat_level) {
		return type == dependency_eval_type::FIVE || (max_threat_level >= 1 && type == dependency_eval_type::OPEN_FOUR);
	}


	threat_finder threat_finder_;
	db_refute_search refute_search_;
	threat_sequence threat_sequence_;
	uint_fast16_t refuted_moves_;
};

#endif