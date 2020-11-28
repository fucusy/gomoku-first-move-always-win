#include "console_ui.h"

#include <iostream>
#include <stdio.h>
#include <string>

#ifndef _COLORS_
#define _COLORS_

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

#endif  /* _COLORS_ */


using namespace std;


// tries to read next move until the move is correct

// if the next move command is incorrect, the same function is called recursively
// with a warning message

coords console_ui::read_next_move(const simple_board& board, const string& message) {
	//SetConsoleCursorPosition(hConsole, { 0, 35 });
	//SetConsoleTextAttribute(hConsole, 7); // light gray

	clear_console(last_answer_length);
	cout << endl << "     " << message << endl << endl;
	cout << endl << "     next move: "; 
	clear_console(last_answer_length);

	//SetConsoleTextAttribute(hConsole, 15); // default white colour
	string answer;
	cin >> answer;

	last_answer_length = answer.size();

	coords next_move;
	if (!coords::try_parse(answer, next_move)) return read_next_move(board, "\"" + answer + "\"" + " is not a correct input");
	if(!board.is_empty(next_move)) return read_next_move(board, "position \"" + answer + "\"" + " is already used");
	else return next_move;
}


// shows game over screen after one of the players wins

void console_ui::show_winning_screen(const simple_board& board, bool human_win) {
	render(board, coords::INCORRECT_POSITION, true);

	if (human_win) cout << endl << "     congratulation, you have defeated the undefeatable AI" << endl;
	else cout << endl << "     you have lost :(" << endl;

	cout << endl << "     press ENTER to start a new game";
	getchar(); getchar();
}


// deletes $number_of_chars characters in front of the cursor

void console_ui::clear_console(size_t number_of_chars) {
	for (int i = 0; i < number_of_chars; ++i)
		cout << " ";
	for (int i = 0; i < number_of_chars; ++i)
		cout << "\b";
}


// renders the board; when $last_move == INCORRECT_POSITION, don't highlight the last move

void console_ui::render(const simple_board& board, coords last_move, bool show_winning_line) {

	//SetConsoleTextAttribute(hConsole, 7); // light gray
	cout << endl << "     a b c d e f g h i j k l m n o" << endl;
	//SetConsoleTextAttribute(hConsole, 8); // dark grey

	// first row: ┌─┬─┬─ ...
    const char *left_up_corner  = u8"\u250c";
    const char *horizontal  = u8"\u2500";
    const char *t  = u8"\u252c";
    const char *vertical  = u8"\u2502";
    const char *right_up_corner = u8"\u2510";
    const char *left_most = u8"\u251c";
    const char *cross = u8"\u253c";
    const char *left_down_corner = u8"\u2514";
    const char *downside_T  = u8"\u2534";
    const char *right_down_corner  = u8"\u2518";



	cout << "    " << left_up_corner << horizontal;
	for (int x = 0; x < 14; ++x)
		cout << t << horizontal;
	cout << right_up_corner << endl;
	
	for (int y = 0; y < constants::BOARD_SIZE; y++) {

		// even row: 12 │ │X│O│ │O ...

		//SetConsoleTextAttribute(hConsole, 7); // light gray
		if (y <= 8) cout << "  " << y + 1 << " ";
		else cout << " " << y + 1 << " ";
		//SetConsoleTextAttribute(hConsole, 8); // dark grey

		for (int x = 0; x < constants::BOARD_SIZE; ++x) {
			cout << vertical;

			bool highlight = false;
			// highlight the piece, if it was placed as a last move or if it is part of the winning line
			if (last_move == coords(x, y) || (show_winning_line && board.is_winning(coords(x, y))))
				highlight = true;

			switch (board.get_move(x, y)) {
				case NONE: cout << " "; break;
                case BLACK: if(!highlight) cout << FBLU("X"); else cout << FYEL("X"); break;
                case WHITE: if(!highlight) cout << FRED("O"); else cout << FYEL("O"); break;
				//case BLACK: SetConsoleTextAttribute(hConsole, highlight ? 0xC0 : 0x0C); cout << "X"; SetConsoleTextAttribute(hConsole, 8); break;
				//case WHITE: SetConsoleTextAttribute(hConsole, highlight ? 0xB0 : 0x0B); cout << "O"; SetConsoleTextAttribute(hConsole, 8); break;
			}
		}
		cout << vertical << endl;
		if (y == 14) break;

		// odd row:  ├─┼─┼─┼ ...

		cout << "    " << left_most << horizontal;
		for (int x = 0; x < 14; ++x)
			cout << cross << horizontal;
		cout << cross << endl;
	}

	// last row └─┴─┴─ ...

	cout << "    " << left_down_corner << horizontal;
	for (int x = 0; x < 14; ++x)
		cout << downside_T << horizontal;
	cout << right_down_corner << endl;

	//SetConsoleTextAttribute(hConsole, 15); // default white colour
}
#include "game.h"

#include <ctime>


using namespace std;


/**
int main(int argc, char** argv) { // the optional console arguments is a path to a logged game file for debug purposes
    console_ui *ui = new console_ui();
    simple_board board = simple_board();
    auto position = ui->read_next_move(board, "Input");
    board.place_move(position, figure::BLACK);
    ui->render(board, position, false);
    auto position2 = ui->read_next_move(board, "Input");
    board.place_move(position2, figure::WHITE);
    ui->render(board, position, false);
    cout << position.to_string() << endl;
}
 **/