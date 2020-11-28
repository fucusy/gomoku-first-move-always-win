#include "db_refute_search.h"
#include <iostream>
#include "console_ui.h"


using namespace std;



// tries to find a refutation of a winning sequence, if the refutation exists, it
// return how many moves in the sequence can be refuted

uint_fast8_t db_refute_search::number_of_refutable_moves(figure attacker) {
	attacker_ = attacker;
	defender_ = attacker == figure::BLACK ? figure::WHITE : figure::BLACK;

	restart_board();

	for (uint_fast8_t i = 0; i < threat_sequence_->size(); ++i) {
		if (i == 0) push_board(threat_sequence_->get(i).positions.attack);
		else push_board(threat_sequence_->get(i - 1).positions.defence, threat_sequence_->get(i).positions.attack);

		if (threat_sequence_->get(i).positions.threat_level == 0) {
			pop_back(i);
			return 0;
		}
		auto max_threat_level = threat_sequence_->get(i).positions.threat_level - 1;
		if (exists_winning_move(attacker_, max_threat_level)) {
			pop_back(i);
			return threat_sequence_->depth() - (number_of_returns_ > 0 ? number_of_returns_ : threat_sequence_->get(i).depth);
		}
	}

	pop_back(uint_fast8_t(threat_sequence_->size() - 1));
	return 0;
}


// pops back $limit moves from the threat_finder and removes the last $limit moves from board, 
// it is called to return the threat_finder into its initial state

void db_refute_search::pop_back(uint_fast8_t limit) {
	for (int i = 0; i <= limit; ++i) {
		threat_finder_->pop();
	}
	ressurrect_board(limit);
}


// standard db_search method, which doesn't do additional checking of refutations

bool db_refute_search::exists_winning_move(figure attacker, uint_least8_t max_threat_level) {	
	set_root(max_threat_level);
	level_ = 1;

	do {
		tree_size_increased_ = false;

		if (dependency_stage(*root_, max_threat_level))
			return true;

		level_ += 1;
		combination_stage(*root_, max_threat_level);
		level_ += 2;

	} while (tree_size_increased_);

	return false;
}


// standard db_search method, which doesn't do additional checking of refutation

bool db_refute_search::dependency_stage(db_node& node, uint_least8_t max_threat_level) {
	node.place_on_board(*board_, attacker_, defender_);

	if (node.level < level_ - 1) {
		for (auto&& child : node.children) {
			if (dependency_stage(*child, max_threat_level)) {
				node.remove_from_board(*board_);
				return true;
			}
		}
	}

	else {
		if (node.type == db_node_type::DEPENDENCY && node.level == level_) {
			node.dependent_threats = threat_finder_->find_dependent_threats(*board_, node.positions[0].attack, attacker_, max_threat_level);
		}
		for (auto&& dependent_threat : node.dependent_threats) {
			uint_fast8_t return_number = 0;
			if (dependency_stage(node, move(dependent_threat), max_threat_level, return_number)) {
				node.remove_from_board(*board_);
				return true;
			}
		}
	}

	node.remove_from_board(*board_);
	return false;
}


// standard db_search method, which doesn't do additional checking of refutation

bool db_refute_search::dependency_stage(db_node& parent, threat_finder_package&& package, uint_least8_t max_threat_level, uint_fast8_t& return_number) {
	unique_ptr<db_node> node = make_unique<db_node>(db_node_type::DEPENDENCY, &parent, move(package.positions), level_);

	if (is_goal_state(package.threat_type, package.positions.attack, max_threat_level)) {
		return true;
	}

	node->place_on_board(*board_, attacker_, defender_);
	node->dependent_threats = threat_finder_->find_dependent_threats(*board_, node->positions[0].attack, attacker_, max_threat_level);

	for (auto&& dependent_threat : node->dependent_threats) {
		if (dependency_stage(*node, move(dependent_threat), max_threat_level, return_number)) {
			node->remove_from_board(*board_);
			return true;
		}
		if (return_number > 0) {
			--return_number;
			node->remove_from_board(*board_);
			return false;
		}
	}

	node->remove_from_board(*board_);
	parent.children.push_back(move(node));
	return false;
}


// pushes the position into the threat_finder

void db_refute_search::push_board(const positions_bundle& defence_positions, coords attack_position) {
	for (uint_fast8_t i = 0; i < defence_positions.size(); ++i)
		board_->place_move(defence_positions[i], attacker_);

	board_->place_move(attack_position, defender_);

	if(attacker_ == figure::BLACK)
		threat_finder_->update_positions(defence_positions, attack_position, figure::BLACK);
	else 
		threat_finder_->update_positions(attack_position, defence_positions, figure::WHITE);
}

void db_refute_search::push_board(coords attack_position) {
	board_->place_move(attack_position, defender_);	
	threat_finder_->update_position(attack_position, attacker_);
}


// removes moves in the winning sequence from the board

void db_refute_search::restart_board() {
	for (size_t i = 0; i < threat_sequence_->size(); ++i) {
		board_->delete_move(threat_sequence_->get(i).positions.attack);

		auto& defence_positions = threat_sequence_->get(i).positions.defence;
		for (uint_fast8_t j = 0; j < defence_positions.size(); ++j) {
			board_->delete_move(defence_positions[j]);
		}
	}
}


// places moves from the winning sequence back to the board

void db_refute_search::ressurrect_board(uint_fast8_t start_position_index) {
	if (start_position_index == threat_sequence_->size() - 1) {
		board_->delete_move(threat_sequence_->get(start_position_index).positions.attack);
		return;
	}

	auto& defence_positions = threat_sequence_->get(start_position_index).positions.defence;
	for (uint_fast8_t i = 0; i < defence_positions.size(); ++i) {
		board_->place_move(defence_positions[i], attacker_);
	}

	for (uint_fast8_t i = start_position_index + 1; i < threat_sequence_->size() - 1; ++i) {
		board_->place_move(threat_sequence_->get(i).positions.attack, defender_);

		auto& positions = threat_sequence_->get(i).positions.defence;
		for (uint_fast8_t j = 0; j < positions.size(); ++j) {
			board_->place_move(positions[j], attacker_);
		}
	}
}
