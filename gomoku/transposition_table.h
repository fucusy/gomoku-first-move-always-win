#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "pn_node.h"

#include <unordered_map>


/* TRANSPOSITION TABLE
 *
 * Hash table for handling equivalent states of the board.
 * 
 * It is used in the proof-number search to ensure, that two equivalent states aren't
 * expanded twice.
 * 
 * To hash a state of the board (3^225 possible state), zobrist_hash is used.
 *
 */

class transposition_table {
public:
	// returns equivalent pn_node, if $hash is already in the table
	// otherwise updates the table with $new_node
	const pn_node* update(uint64_t hash, const pn_node* new_node) {
		auto found = table_.find(hash);
		if(found == table_.end()) {
			table_.insert(std::make_pair(hash, new_node));
			return nullptr;
		}

#ifdef _DEBUG // check if the collision wasn't erroneous
		check_collision(new_node, found->second);
#endif

		return found->second;
	}

private:
	std::unordered_map<uint64_t, const pn_node*> table_;

#ifdef _DEBUG
	static void check_collision(const pn_node* a, const pn_node* b);
#endif
};

#endif