#ifndef THREAT_SEQUENCE_H
#define THREAT_SEQUENCE_H

#include "coords.h"
#include "threat_positions.h"

#include <vector>


class db_node;


struct threat_sequence_package {
	threat_positions positions;
	uint_fast8_t depth;

	threat_sequence_package(threat_positions&& positions, uint_fast8_t depth) : positions(positions), depth(depth) {}
	threat_sequence_package(const threat_positions& positions, uint_fast8_t depth) : positions(positions), depth(depth) {}
};


/* THREAT SEQUENCE
 * 
 * Class representing the winning sequence of moves.
 * 
 * The moves are placed into $list_, for fast search using coordinates,
 * an array (game board) is used. The memory overhead isn't an issue,
 * because only one instance of this class is used in one AI. 
 * 
 */

class threat_sequence {
public:
	threat_sequence() : depth_(0) { init_array(); }
	void restart() { list_.clear(); depth_ = 0; init_array(); }
	void init_array() { for (auto x = 0; x < constants::BOARD_SIZE; ++x) for (auto y = 0; y < constants::BOARD_SIZE; ++y) board_[x][y] = 0; }

	uint_fast8_t contains(coords position) const { return board_[position.x][position.y]; }
	coords first_move() const { return list_[0].positions.attack; }
	size_t size() const { return list_.size(); }
	uint_fast8_t depth() const { return depth_; }
	void push(const db_node& node);
	void pop(const db_node& node);
	std::string to_string() const;
	const threat_sequence_package& get(size_t index) const { return list_[index]; }


private:
	uint_fast8_t board_[constants::BOARD_SIZE][constants::BOARD_SIZE]; // indices into $list_
	std::vector<threat_sequence_package> list_;
	uint_fast8_t depth_;

	void place_on_board(const threat_positions& positions, uint_fast8_t count);
	void remove_from_board(const threat_positions& positions);
};

#endif