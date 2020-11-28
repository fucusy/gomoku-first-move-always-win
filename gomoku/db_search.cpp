#include "db_search.h"
#include "parameters.h"
#include <iostream>
#include "console_ui.h"


using namespace std;


// returns a move that leads to a certain win

optional<coords> db_search::get_winning_move(figure attacker, uint_least8_t max_threat_level) {
	attacker_ = attacker;
	defender_ = attacker == figure::BLACK ? figure::WHITE : figure::BLACK;

	threat_sequence_.restart();
	set_root(max_threat_level);
	level_ = 1;

	do {
		tree_size_increased_ = false;

		if (dependency_stage(*root_, max_threat_level)) // returns true if a win was found
			return threat_sequence_.first_move();
		
		level_ += 1;
		combination_stage(*root_, max_threat_level);
		level_ += 2;

	} while (tree_size_increased_ && refuted_moves_ < parameters::DB_SEARCH_MAX_REFUTE_WINS);

	return {};
}


// computes the dependency stage of the search (as explained in the paper)
// returns true if a win was found
// recursively finds the unexpanded nodes and calls the overriden dependency_stage on them to expand them

bool db_search::dependency_stage(db_node& node, uint_least8_t max_threat_level) {
	node.place_on_board(*board_, attacker_, defender_);
	threat_sequence_.push(node);

	if (node.level < level_ - 1) { // recursively move to the children and do nothing
		for (auto&& child : node.children) {
			if (dependency_stage(*child, max_threat_level)) {
				node.remove_from_board(*board_);
				return true;
			}
		}
	}

	// else recursively expand the node via calling the overriden dependency_stage
	// the expansion is done only with the threats that depend on the threat represented by this node

	else { 
		if (node.type == db_node_type::DEPENDENCY && node.level == level_) {
			node.dependent_threats = threat_finder_.find_dependent_threats(*board_, node.positions[0].attack, attacker_, max_threat_level);
		}
		for (auto&& dependent_threat : node.dependent_threats) {
			uint_fast8_t returnNumber = 0;
			if (dependency_stage(node, move(dependent_threat), max_threat_level, returnNumber)) {
				node.remove_from_board(*board_);
				return true;
			}
		}
	}

	node.remove_from_board(*board_);
	threat_sequence_.pop(node);
	return false;
}


// expands the $parent node

bool db_search::dependency_stage(db_node& parent, threat_finder_package&& package, uint_least8_t max_threat_level, uint_fast8_t& return_number) {
	unique_ptr<db_node> node = make_unique<db_node>(db_node_type::DEPENDENCY, &parent, move(package.positions), level_);
	threat_sequence_.push(*node);

	// if the goal state was reached, check for a possible refutation
	// if a refutation was found, then it also says how many moves backwards are refuted, so return that
	// many recursive calls from the stack (that's what the parameter $return_number is used for)

	if (is_goal_state(package.threat_type, package.positions.attack, max_threat_level)) {
		return_number = refute_search_.number_of_refutable_moves(defender_);
		if (return_number > 0) {
			++refuted_moves_;
			--return_number;
			threat_sequence_.pop(*node);
			return false;
		}
		return true;
	}

	node->place_on_board(*board_, attacker_, defender_);
	node->dependent_threats = threat_finder_.find_dependent_threats(*board_, node->positions[0].attack, attacker_, max_threat_level);

	// expand the node and possibly return, if a refutation was found in a child
	for (auto&& dependent_threat : node->dependent_threats) {
		if (dependency_stage(*node, move(dependent_threat), max_threat_level, return_number)) {
			node->remove_from_board(*board_);
			return true;
		}
		if (return_number > 0) {
			--return_number;
			node->remove_from_board(*board_);
			threat_sequence_.pop(*node);
			return false;
		}
	}

	// if the node doesn't have any direct children, check it for a refutation and remove it from the tree if a
	// refutation is found
	// this is an effective heuristic that help to reduce the number of nodes 

	if (node->dependent_threats.empty()) {
		return_number = refute_search_.number_of_refutable_moves(defender_);
		if (return_number > 0) {
			++refuted_moves_;
			--return_number;
			node->remove_from_board(*board_);
			threat_sequence_.pop(*node);
			return false;
		}
	}

	node->remove_from_board(*board_);
	threat_sequence_.pop(*node);
	parent.children.push_back(move(node));
	return false;
}