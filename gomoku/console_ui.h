#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H

#include "bit_board.h"
#include "abstract_ui.h"


/* CONSOLE USER INTERFACE
 * 
 * Implementation of abstract_ui that uses the console window for I/O. 
 * 
 * It uses some functions of the console that are Windows-only and are not portable.
 * 
 */

class console_ui : public abstract_ui {
public:
	// tries to read next move until the move is correct
	coords read_next_move(const simple_board& board, const std::string& message) override;

	// renders the board; when $last_move == INCORRECT_POSITION, don't highlight the last move
	void render(const simple_board& board, coords last_move = coords::INCORRECT_POSITION, bool show_winning_line = false) override;

	// shows game over screen after one of the players wins
	void show_winning_screen(const simple_board& board, bool human_win) override;

private:

	// deletes $number_of_chars characters in front of the cursor
	void clear_console(size_t number_of_chars);

	size_t last_answer_length = 0;
};

#endif