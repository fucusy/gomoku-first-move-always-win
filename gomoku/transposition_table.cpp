#include "transposition_table.h"
#include "simple_board.h"
#include <stdexcept>

#ifdef _DEBUG

// noneffective method just for debug purposes
// checks if the collision in the hash table is valid
void transposition_table::check_collision(const pn_node* a, const pn_node* b) {
	simple_board board;
	auto node = a;
	while(true) {
		board.place_move(node->position(), BLACK);
		if (node->parents().size() == 0) break;
		node = node->parents()[0];
	}

	node = b;
	while (true) {
		if (board.is_empty(node->position())) throw std::runtime_error("erroneous collision detected");
		board.delete_move(node->position());
		if (node->parents().size() == 0) break;
		node = node->parents()[0];
	}

	for(int y = 0; y < constants::BOARD_SIZE; ++y) for(int x = 0; x < constants::BOARD_SIZE; ++x) {
		if(!board.is_empty(coords(x,y))) throw std::runtime_error("erroneous collision detected");
	}
}

#endif
