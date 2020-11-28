#include "pn_node.h"


// update proof and disprove numbers according to node's children


void pn_node::set_proof_and_disproof() {
	if (children_.size() == 0) { // treat draw as a loss
		switch (type_) {
			case pn_type::_AND: proof_ = 0; disproof_ = UINT32_MAX; break;
			case pn_type::_OR:  proof_ = UINT32_MAX; disproof_ = 0; break;
		}
		return;
	};

	subtree_size_ = sum_subtree_sizes();
	switch (type_) {
		case pn_type::_AND:
			proof_ = sum_children_proof();
			disproof_ = min_children_disproof();
			break;
		case pn_type::_OR:
			proof_ = min_children_proof();
			disproof_ = sum_children_disproof();
			break;
	}
}


// recursively updates all ancestors

void pn_node::update_ancestors() {
	set_proof_and_disproof();

	for (auto&& parent : parents_) {
		parent->update_ancestors();
	}
}

size_t pn_node::sum_subtree_sizes() {
	size_t sum = 0;
	for(auto&& child : children_)
		sum += child->subtree_size_;

	return sum;
}

uint32_t pn_node::sum_children_proof() {
	uint32_t sum = 0;
	for (auto&& child : children_) {
		if (child->proof_ == UINT32_MAX)
			return UINT32_MAX;
		sum += child->proof_;
	}

	return sum;
}

uint32_t pn_node::sum_children_disproof() {
	uint32_t sum = 0;
	for (auto&& child : children_) {
		if (child->disproof_ == UINT32_MAX)
			return UINT32_MAX;
		sum += child->disproof_;
	}

	return sum;
}

uint32_t pn_node::min_children_proof() {
	uint32_t min = UINT32_MAX;
	for (auto&& child : children_) {
		if (min > child->proof_)
			min = child->proof_;
	}

	return min;
}

uint32_t pn_node::min_children_disproof() {
	uint32_t min = UINT32_MAX;
	for (auto&& child : children_) {
		if (min > child->disproof_)
			min = child->disproof_;
	}

	return min;
}
