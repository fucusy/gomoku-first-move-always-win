#ifndef ABSTRACT_DB_SEARCH
#define ABSTRACT_DB_SEARCH


#include "threat_positions.h"
#include "db_node.h"
#include "threat_sequence.h"

#include <vector>
#include <unordered_set>


/* ABSTRACT DEPENDENCY-BASED SEARCH
 * 
 * Abstract ancestor of classes db_search and db_refute_search implementing methods they have in common.
 *
 */

class abstract_db_search {
protected:

	explicit abstract_db_search(bit_board* board) : board_(board), attacker_(figure::NONE), defender_(figure::NONE), root_(nullptr), level_(1), tree_size_increased_(true) {}


	// computes the combination stage of the search (as explained in the paper)
	void combination_stage(db_node& node, uint_least8_t max_threat_level);
	void combine(db_node& base_node, db_node& node, uint_least8_t max_threat_level);


	bit_board* board_;

	figure attacker_;
	figure defender_;
	db_node_unique root_;
	uint_least8_t level_;
	bool tree_size_increased_;

	std::unordered_set<uint_least32_t> parent_ids_;
	std::vector<threat_positions> accumulated_positions;
};

#endif