#include "simple_board.h"


// checks if position $pos is winning in the current board

// there is no need for efficiency (there is better equivalent in the static evaluator), 
// so the check is done trivially

bool simple_board::is_winning(coords pos) const {

	figure base_figure = get_move(pos);
	if (base_figure == figure::NONE) return false; // return false if there is no figure in the checked position

	// checks for >=5 pieces in line in each of the four directions
	for (int i = 0; i < 4; ++i) {
		int counter = 1; // counts the number of figures in line

		for (int off = 1; off < 5; ++off) {
			if (get_move(pos + coords_offset::DIRECTIONS[i] * off) != base_figure) break;
			++counter;
		}
		for (int off = 1; off < 5; ++off) {
			if (get_move(pos - coords_offset::DIRECTIONS[i] * off) != base_figure) break;
			++counter;
		}

		if (counter >= 5) return true;
	}

	return false;
}

