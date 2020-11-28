#ifndef ABSTRACT_ENGINE_H
#define ABSTRACT_ENGINE_H

#include "coords.h"
#include "bit_board.h"
#include <map>

/* ABSTRACT ENGINE
 * 
 * abstract ancestor of gomoku AIs
 * 
 */

class abstract_engine {
public:

    virtual bool search_winning_test() = 0;
	// informs the engine that a new move was placed on board
	virtual void set_next_move(coords pos) = 0;

	// engine returns his best response to actual board, doesn't update current state
	virtual coords get_response() = 0;

	// changes figure from BLACK to WHITE and vice versa
	static void change_player(figure& player) { player = (player == BLACK ? WHITE : BLACK); }

    std::string init_filename;
	std::map<std::string, std::string> solved_boardstr2action; // key is encoded by bit_board.to_string(), value is coords.to_string() 
	virtual ~abstract_engine() {}

protected:
	bit_board board_;
	figure current_player_ = BLACK;
};

#endif