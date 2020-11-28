#ifndef DB_NODE_H
#define DB_NODE_H

#include "threat_positions.h"

#include <cstdint>
#include <memory>
#include <vector>
#include "threat_finder.h"
#include "bit_board.h"
#include <algorithm>

class db_node;

enum class db_node_type {
	ROOT,
	DEPENDENCY,
	COMBINATION
};

using db_node_unique = std::unique_ptr<db_node>;


/* DEPENDENCY-BASED SEARCH NODE
 *
 * A node in the db_search containg helping methods for easier manipulation.
 * 
 * The node contains the attack position, all responses to that attack and all threats that are 
 * dependent on that attack.
 *
 */

class db_node {
public:
	db_node(db_node_type type, possible_positions&& dependent_threats, uint_least8_t level) 
		: type(type), level(level), parent(nullptr), dependent_threats(dependent_threats), are_dependent_threats_sorted(false), ID_(counter_++) { }

	db_node(db_node_type type, const db_node* parent, threat_positions&& positions, uint_least8_t level)
		: type(type), level(level), parent(parent), are_dependent_threats_sorted(false), ID_(counter_++) { this->positions.push_back(positions);	}

	db_node(db_node_type type, const db_node* parent, possible_positions&& dependent_threats, std::vector<threat_positions>&& positions, uint_least8_t level) 
		: type(type), level(level), parent(parent), positions(positions), dependent_threats(dependent_threats), are_dependent_threats_sorted(false), ID_(counter_++) {
	}

	db_node(db_node_type type, const db_node* parent, possible_positions&& dependent_threats, const std::vector<threat_positions>& positions, uint_least8_t level)
		: type(type), level(level), parent(parent), positions(positions), dependent_threats(dependent_threats), are_dependent_threats_sorted(false), ID_(counter_++) {
	}

	db_node(const db_node&) = delete;
	db_node& operator = (const db_node&) = delete;

	void place_on_board(bit_board& board, figure attacker, figure defender) const;
	void remove_from_board(bit_board& board) const;
	bool is_on_board(const bit_board& board) const;

	void sort_dependent_threats() {
		if (!are_dependent_threats_sorted) {
			std::sort(dependent_threats.begin(), dependent_threats.end(), threat_finder_package::ordinal_comparer);
			are_dependent_threats_sorted = true;
		}
	}

	uint_least32_t id() const { return ID_; }

	bool operator ==(const db_node& other) const { return ID_ == other.ID_; }
	bool operator !=(const db_node& other) const { return ID_ != other.ID_; }

	// allows db_node to be used as a key in hash tables
	struct hasher {
		std::size_t operator()(const db_node& node) const {
			return std::hash<uint_least32_t>()(node.ID_);
		}
	};


	/* fields */

	db_node_type type;
	uint_least8_t level;
	const db_node* parent;
	std::vector<db_node_unique> children;
	std::vector<threat_positions> positions; // older positions first
	possible_positions dependent_threats;
	bool are_dependent_threats_sorted;

private:
	static uint_least32_t counter_; // counter that yields unique IDs
	uint_least32_t ID_;
};

#endif