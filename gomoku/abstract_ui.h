#ifndef ABSTRACT_UI_H
#define ABSTRACT_UI_H

#include "simple_board.h"


/* ABSTRACT USER INTERFACE
 * 
 * Abstract class for interaction with the player.
 * 
 */

class abstract_ui {
public:
	// tries to read next move until the move is correct
	virtual coords read_next_move(const simple_board& board, const std::string& message) = 0;

	// renders the board; when $last_move == INCORRECT_POSITION, don't highlight the last move
	virtual void render(const simple_board& board, coords last_move = coords::INCORRECT_POSITION, bool show_winning_line = false) = 0;
	
	// shows game over screen after one of the players wins
	virtual void show_winning_screen(const simple_board& board, bool human_win) = 0;

	virtual ~abstract_ui() {}
};

#endif