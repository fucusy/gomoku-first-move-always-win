#include "abstract_db_search.h"


// computes the combination stage of the search (as explained in the paper)

// it tries to combine any two nodes of the search and creates a new combination node
// if these nodes create a new threat dependent on them

// combining any two nodes is quite expensive, so this stage is called only when no new
// nodes can be created via the dependency stage and only nodes that weren't exemined previously
// are checked

void abstract_db_search::combination_stage(db_node& node, uint_least8_t max_threat_level) {
	node.place_on_board(*board_, attacker_, defender_);

	if (node.type == db_node_type::DEPENDENCY && node.level == level_ - 1) { // combine only new dependency nodes
		combine(node, *root_, max_threat_level);
		node.level = level_;
	}

	parent_ids_.insert(node.id());
	for (auto&& child : node.children) {
		combination_stage(*child, max_threat_level);
	}

	parent_ids_.erase(node.id());
	node.remove_from_board(*board_);
}


// tries to combine node $base_node with node $node and recursively combines $base_node with $node's children

void abstract_db_search::combine(db_node& base_node, db_node& node, uint_least8_t max_threat_level) {

	// don't try to combine two nodes that were already checked or two same nodes or node that is already placed on the board
	if (node.level == level_ || node == base_node || node.is_on_board(*board_)) {
		return;
	}

	// if $node is a parent of $base_node, don't combine them and just recursively check the children 
	if (parent_ids_.find(node.id()) != parent_ids_.end()) {
		for (auto&& child : node.children) {
			combine(base_node, *child, max_threat_level);
		}
		return;
	}

	node.place_on_board(*board_, attacker_, defender_);
	accumulated_positions.insert(accumulated_positions.end(), node.positions.begin(), node.positions.end()); // positions to be shared

	if (node.type == db_node_type::DEPENDENCY && threat_finder::is_potential_threat(base_node.positions[0].attack, node.positions[0].attack)) {

		auto dependent_threats = threat_finder::find_dependent_threats(*board_, base_node.positions[0].attack, attacker_, max_threat_level);
		std::sort(dependent_threats.begin(), dependent_threats.end(), threat_finder_package::ordinal_comparer);

		base_node.sort_dependent_threats();
		dependent_threats = threat_finder_package::substract_threats(dependent_threats, base_node.dependent_threats);

		node.sort_dependent_threats();
		dependent_threats = threat_finder_package::substract_threats(dependent_threats, node.dependent_threats);

		// if any new threats can be made using this combination, create a new combination node
		if (!dependent_threats.empty()) {
			std::unique_ptr<db_node> combination_node = std::make_unique<db_node>(db_node_type::COMBINATION, &base_node, move(dependent_threats), accumulated_positions, level_ + 1);
			combination_node->are_dependent_threats_sorted = true;

			base_node.children.push_back(move(combination_node));
			tree_size_increased_ = true;
		}
	}

	for (auto&& child : node.children) {
		combine(base_node, *child, max_threat_level);
	}
	node.remove_from_board(*board_);
	accumulated_positions.erase(accumulated_positions.begin() + accumulated_positions.size() - node.positions.size(), accumulated_positions.end());
}