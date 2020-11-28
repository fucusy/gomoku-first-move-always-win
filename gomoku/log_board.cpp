#include "log_board.h"

#include <iomanip>
#include <ctime>
#include <sstream>


using namespace std;


// creates a new file in directory \log\, that is uniquely named according to current date and time

log_board::log_board() {
	auto t = time(nullptr);
	tm time;
	ostringstream oss;
	oss << put_time(&time, "%d-%m-%Y-%H-%M-%S");
	auto date_string = oss.str();

	ofs = ofstream("log/" + date_string + ".log");
}
