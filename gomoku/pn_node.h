#ifndef PN_NODE_H
#define PN_NODE_H

#include "coords.h"

#include <cstdint>
#include <memory>
#include <vector>


enum class pn_value : uint_fast8_t { _TRUE, _FALSE, _UNKNOWN, _UNEVALUATED };
enum class pn_type : uint_fast8_t { _OR, _AND };


/* PROOF-NUMBER NODE
 * 
 * Class representing a node in the proof-number search DAG
 * 
 */

class pn_node {
public:
	using ptr_pn_node = std::shared_ptr<pn_node>;

	pn_node(): proof_(1), disproof_(1), threat_(0), type_(), subtree_size_(1) {}
	pn_node(coords pos, pn_type type, uint32_t proof, uint32_t disproof, uint8_t threat) : proof_(proof), disproof_(disproof), threat_(threat), position_(pos), type_(type), subtree_size_(1) {}
	pn_node(coords pos, pn_type type, uint32_t proof, uint32_t disproof, uint8_t threat, pn_node* parent) : pn_node(pos, type, proof, disproof, threat) {
		parents_.push_back(parent);
	}

	// update proof and disprove numbers according to node's children
	void set_proof_and_disproof();

	// recursively updates all ancestors
	void update_ancestors();

	void add_child(coords pos, pn_type type, uint32_t proof, uint32_t disproof, uint8_t threat) { children_.push_back(std::make_shared<pn_node>(pos, type, proof, disproof, threat, this)); }
	void add_child(ptr_pn_node child) { child->add_parent(this); children_.push_back(child); }
	void add_parent(pn_node* parent) { parents_.push_back(parent); }
	uint8_t parent_threat() {
	    uint8_t output = 0;
	    for(auto n: parents_){
	        output |= threat_;
	    }
	    return output;
	}



    uint32_t proof() const { return proof_; }
	uint32_t disproof() const { return disproof_; }
	uint8_t threat() const { return threat_; }
	size_t subtree_size() const { return subtree_size_; }
	pn_type type() const { return type_; }
	coords position() const { return position_; }
	const pn_node* child(size_t index) const { return children_[index].get(); }
	pn_node* child(size_t index) { return children_[index].get(); }
	const std::vector<pn_node*>& parents() const { return parents_; }
	const std::vector<ptr_pn_node>& children() const { return children_; }
	size_t children_size() const { return children_.size(); }
	
private:
	size_t sum_subtree_sizes();
	uint32_t sum_children_proof();
	uint32_t sum_children_disproof();
	uint32_t min_children_proof();
	uint32_t min_children_disproof();


	uint32_t proof_;
	uint32_t disproof_;
	uint8_t threat_;

	coords position_;
	pn_type type_;
	size_t subtree_size_;

	std::vector<ptr_pn_node> children_;
	std::vector<pn_node*> parents_;
};

#endif
