#include "db_node.h"

uint_least32_t db_node::counter_ = 0;


// places both attack and defence positions of the node on the board

void db_node::place_on_board(bit_board& board, figure attacker, figure defender) const {
	for(auto&& position : positions) {
		board.place_move(position.attack, attacker);
		for(uint_fast8_t i = 0; i < position.defence.size(); ++i) {
			board.place_move(position.defence[i], defender);
		}
	}
}


// removes both attack and defence positions of the node from the board

void db_node::remove_from_board(bit_board& board) const {
	for (auto&& position : positions) {
		board.delete_move(position.attack);
		for (uint_fast8_t i = 0; i < position.defence.size(); ++i) {
			board.delete_move(position.defence[i]);
		}
	}
}

bool db_node::is_on_board(const bit_board& board) const {
	for (auto&& position : positions) {
		if (!board.is_empty(position.attack)) 
			return true;

		for (uint_fast8_t i = 0; i < position.defence.size(); ++i) {
			if (!board.is_empty(position.defence[i]))
				return true;
		}
	}

	return false;
}
