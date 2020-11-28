#ifndef LOG_BOARD_H
#define LOG_BOARD_H

#include "simple_board.h"

#include <ostream>
#include <memory>
#include <string>
#include <fstream>


/* LOG BOARD
 *
 * Extension of simple_board that logs moves after each insert into a file placed in directory \log\.
 *
 */

class log_board : public simple_board {
public:
	log_board();
	void place_move(coords pos, figure fig) { simple_board::place_move(pos, fig); ofs << pos.to_string() << ' ' << std::flush; }

private:
	std::ofstream ofs;
};

#endif