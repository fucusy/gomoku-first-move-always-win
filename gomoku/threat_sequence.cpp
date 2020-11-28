#include "threat_sequence.h"
#include "db_node.h"

#include <sstream>


void threat_sequence::push(const db_node& node) {
	for(auto&& position : node.positions) {
		list_.push_back(threat_sequence_package(position, depth_));
		place_on_board(position, depth_);
	}
	++depth_;
}

void threat_sequence::pop(const db_node& node) {
	for (auto&& position : node.positions) {
		list_.pop_back();
		remove_from_board(position);
	}
	--depth_;
}

std::string threat_sequence::to_string() const {
	std::stringstream ss;

	for(auto&& package : list_) {
		ss << package.positions.attack.to_string() << "  -  ";
		for(uint_fast8_t i = 0; i < package.positions.defence.size(); ++i) {
			ss << package.positions.defence[i].to_string() << " ";
		}
		ss << std::endl;
	}

	return ss.str();
}

void threat_sequence::place_on_board(const threat_positions& positions, uint_fast8_t count) {
	board_[positions.attack.x][positions.attack.y] = count;
	for (uint_fast8_t i = 0; i < positions.defence.size(); ++i) {
		board_[positions.defence[i].x][positions.defence[i].y] = count;
	}
}

void threat_sequence::remove_from_board(const threat_positions& positions) {
	board_[positions.attack.x][positions.attack.y] = 0;
	for (uint_fast8_t i = 0; i < positions.defence.size(); ++i) {
		board_[positions.defence[i].x][positions.defence[i].y] = 0;
	}
}