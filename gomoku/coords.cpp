#include "coords.h"

#include <algorithm>
#include  <stdexcept>

using namespace std;


// parses $input into $output; returns TRUE if parsing position into $output was successful, otherwise returns FALSE
bool coords::try_parse(const std::string& input, coords& output) {
	string s = input;
	transform(s.begin(), s.end(), s.begin(), tolower); // converts input string into lowercase
	if (s.size() < 2 || s.size() > 3 || s[0] < 'a' || s[0] > 'o') return false; // only strings in format such as "b3" are accepted

	coord x = s[0] - 'a';
	coord y;
	try {
		y = stoi(s.substr(1));
	} catch(invalid_argument&) {
		return false;
	}

	output = coords(x, y - 1);
	if (output.is_out_of_board()) return false;
	return true;
}

const coords_offset coords_offset::DIRECTIONS[4] = { coords_offset(1, 0), coords_offset(0, 1), coords_offset(1, 1), coords_offset(-1, 1) };
const coords coords::INCORRECT_POSITION = coords(-1, -1);