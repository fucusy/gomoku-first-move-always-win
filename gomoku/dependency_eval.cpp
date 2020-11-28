#include "dependency_eval.h"
#include "bit_board.h"

#include <cassert>


using namespace std;


bool dependency_eval_package::operator==(const dependency_eval_package& other) const {
	if (type == other.type && friendly_response.size() == other.friendly_response.size() && enemy_response.size() == other.enemy_response.size()) {
		for (size_t i = 0; i < friendly_response.size(); ++i) {
			if (friendly_response[i] != other.friendly_response[i])
				return false;
		}
		for (size_t i = 0; i < enemy_response.size(); ++i) {
			if (enemy_response[i] != other.enemy_response[i])
				return false;
		}
		return true;
	}
	return false;
}

dependency_eval::table dependency_eval::cache_;


// lookup table for fast evaluation

dependency_eval::table::table() {
	results.push_back(dependency_eval_package(dependency_eval_type::NOTHING, response(), response()));

	for (uint_least32_t line = 0; line < SIZE; ++line) {
		figure middleFigure = figure((line >> 8) & 3);

		if (middleFigure == figure::NONE || middleFigure == figure::OUTSIDE)
			indices[line] = 0;
		else
			indices[line] = add_line(bit_board::uint_to_line<9>(line));
	}
}


// converts the relative offsets from $base in a $direction to an absolute position

positions_bundle dependency_eval::offsets_to_positions(const response& offsets, coords base, direction dir) {
	positions_bundle output;

	for (size_t i = 0; i < offsets.size(); ++i)
		output.push_back(base + coords::DIRECTIONS[dir] * offsets[i]);

	return output;
}


// adds a line into the lookup table

uint_least8_t dependency_eval::add_line(array<figure, 9> line) {
	figure middle_figure = line[4];

	dependency_eval_type type;
	response friendly_response;
	response enemy_response;

	if (is_five(line, middle_figure)) {
		return update_results(dependency_eval_type::FIVE, response(), response());
	}
	if (is_four(line, middle_figure, type, friendly_response, enemy_response)) {
		return update_results(type, move(friendly_response), move(enemy_response));
	}
	if (is_three(line, middle_figure, friendly_response, enemy_response)) {
		return update_results(dependency_eval_type::THREE, move(friendly_response), move(enemy_response));
	}
	if (is_possible_threat(line, middle_figure, friendly_response)) {
		return update_results(dependency_eval_type::POSSIBLE_THREAT, move(friendly_response), response());
	}
	return 0;
}


// updates the lookup table

uint_least8_t dependency_eval::update_results(dependency_eval_type type, response&& friendly_response, response&& enemy_response) {
	dependency_eval_package testing_package(type, move(friendly_response), move(enemy_response));

	for (uint_least8_t i = 0; i < cache_.results.size(); ++i) {
		if (cache_.results[i] == testing_package) return i;
	}

	cache_.results.push_back(testing_package);
	assert(cache_.results.size() <= 256);

	return uint_least8_t(cache_.results.size() - 1);
}


bool dependency_eval::is_possible_threat(array<figure, 9>& line, figure player_to_evaluate, response& friendly_response) {
	friendly_response.clear();

	for (int i = 0; i < 9; ++i) {
		if (line[i] == figure::NONE) {
			line[i] = player_to_evaluate;
			if (is_four(line, player_to_evaluate) || is_three(line, player_to_evaluate)) {
				friendly_response.push_back(i - 4);
			}
			line[i] = figure::NONE;
		}
	}

	return friendly_response.size() > 0;
}

bool dependency_eval::is_three(array<figure, 9>& line, figure player_to_evaluate) {
	for (int i = 0; i < 4; ++i) {
		int myCount = 0;
		int noneCount = 0;
		for (int j = 1; j <= 4; ++j) {
			if (line[i + j] == player_to_evaluate)
				myCount++;
			else if (line[i + j] == figure::NONE)
				noneCount++;
		}

		// general description of a threat by three
		if (line[0 + i] == figure::NONE && myCount == 3 && noneCount == 1 && line[5 + i] == figure::NONE) {
			return true;
		}
	}

	return false;
}

bool dependency_eval::is_three(std::array<figure, 9>& line, figure player_to_evaluate, response& friendly_response, response& enemy_response) {
	friendly_response.clear();
	enemy_response.clear();
	bool found = false;

	for (int i = 0; i < 3; ++i) {
		if (line[0 + i] == figure::NONE &&
			line[1 + i] == figure::NONE &&
			line[2 + i] == player_to_evaluate &&
			line[3 + i] == player_to_evaluate &&
			line[4 + i] == player_to_evaluate &&
			line[5 + i] == figure::NONE &&
			line[6 + i] == figure::NONE) {

			friendly_response.push_back(-3 + i); friendly_response.push_back(1 + i);
			enemy_response.push_back(-3 + i); enemy_response.push_back(1 + i);
			return true;
		}
	}

	for (int i = 0; i < 4; ++i) {
		if (line[0 + i] == figure::NONE &&
			line[1 + i] == player_to_evaluate &&
			line[2 + i] == figure::NONE &&
			line[3 + i] == player_to_evaluate &&
			line[4 + i] == player_to_evaluate &&
			line[5 + i] == figure::NONE) {

			if (find(friendly_response.begin(), friendly_response.end(), -2 + i) == friendly_response.end())
				friendly_response.push_back(-2 + i);

			if (find(enemy_response.begin(), enemy_response.end(), -4 + i) == enemy_response.end())
				enemy_response.push_back(-4 + i);
			if (find(enemy_response.begin(), enemy_response.end(), -2 + i) == enemy_response.end())
				enemy_response.push_back(-2 + i);
			if (find(enemy_response.begin(), enemy_response.end(), 1 + i) == enemy_response.end())
				enemy_response.push_back(1 + i);

			found = true;
		}

		else if (line[0 + i] == figure::NONE &&
			line[1 + i] == player_to_evaluate &&
			line[2 + i] == player_to_evaluate &&
			line[3 + i] == figure::NONE &&
			line[4 + i] == player_to_evaluate &&
			line[5 + i] == figure::NONE) {

			if (find(friendly_response.begin(), friendly_response.end(), -1 + i) == friendly_response.end())
				friendly_response.push_back(-1 + i);

			if (find(enemy_response.begin(), enemy_response.end(), -4 + i) == enemy_response.end())
				enemy_response.push_back(-4 + i);
			if (find(enemy_response.begin(), enemy_response.end(), -1 + i) == enemy_response.end())
				enemy_response.push_back(-1 + i);
			if (find(enemy_response.begin(), enemy_response.end(), 1 + i) == enemy_response.end())
				enemy_response.push_back(1 + i);

			found = true;
		}

		else if (line[0 + i] == figure::NONE &&
			line[1 + i] == player_to_evaluate &&
			line[2 + i] == player_to_evaluate &&
			line[3 + i] == player_to_evaluate &&
			line[4 + i] == figure::NONE &&
			line[5 + i] == figure::NONE) {

			if (find(friendly_response.begin(), friendly_response.end(), -4 + i) == friendly_response.end())
				friendly_response.push_back(-4 + i);
			if (find(friendly_response.begin(), friendly_response.end(), 0 + i) == friendly_response.end())
				friendly_response.push_back(0 + i);

			if (find(enemy_response.begin(), enemy_response.end(), -4 + i) == enemy_response.end())
				enemy_response.push_back(-4 + i);
			if (find(enemy_response.begin(), enemy_response.end(), 0 + i) == enemy_response.end())
				enemy_response.push_back(0 + i);
			if (find(enemy_response.begin(), enemy_response.end(), 1 + i) == enemy_response.end())
				enemy_response.push_back(1 + i);

			found = true;
		}

		else if (line[0 + i] == figure::NONE &&
			line[1 + i] == figure::NONE &&
			line[2 + i] == player_to_evaluate &&
			line[3 + i] == player_to_evaluate &&
			line[4 + i] == player_to_evaluate &&
			line[5 + i] == figure::NONE) {
			
			if (find(friendly_response.begin(), friendly_response.end(), -3 + i) == friendly_response.end())
				friendly_response.push_back(-3 + i);
			if (find(friendly_response.begin(), friendly_response.end(), 1 + i) == friendly_response.end())
				friendly_response.push_back(1 + i);

			if (find(enemy_response.begin(), enemy_response.end(), -4 + i) == enemy_response.end())
				enemy_response.push_back(-4 + i);
			if (find(enemy_response.begin(), enemy_response.end(), -3 + i) == enemy_response.end())
				enemy_response.push_back(-3 + i);
			if (find(enemy_response.begin(), enemy_response.end(), 1 + i) == enemy_response.end())
				enemy_response.push_back(1 + i);

			found = true;
		}
	}

	return found;
}

bool dependency_eval::is_four(std::array<figure, 9>& line, figure player_to_evaluate) {
	for (int i = 0; i < 9; ++i) {
		if (line[i] == figure::NONE) {
			line[i] = player_to_evaluate;
			if (is_five(line, player_to_evaluate)) {
				line[i] = figure::NONE;
				return true;
			}
			line[i] = figure::NONE;
		}
	}

	return false;
}

bool dependency_eval::is_four(std::array<figure, 9>& line, figure player_to_evaluate, dependency_eval_type& type, response& friendly_response, response& enemy_response) {
	friendly_response.clear();
	enemy_response.clear();

	for (int i = 0; i < 9; ++i) {
		if (line[i] == figure::NONE) {
			line[i] = player_to_evaluate;
			if (is_five(line, player_to_evaluate)) {
				friendly_response.push_back(i - 4);
				enemy_response.push_back(i - 4);
			}
			line[i] = figure::NONE;
		}
	}

	if (friendly_response.size() >= 2) {
		type = dependency_eval_type::OPEN_FOUR;
		return true;
	}
	if (friendly_response.size() == 1) {
		type = dependency_eval_type::FOUR;
		return true;
	}

	type = dependency_eval_type::NOTHING;
	return false;
}

bool dependency_eval::is_five(std::array<figure, 9>& line, figure player_to_evaluate) {
	for (int i = 0; i < 5; ++i) {
		int count = 0;
		for (int j = 0; j < 5; ++j) {
			if (line[i + j] == player_to_evaluate) count++;
		}

		if (count == 5) return true;
	}
	return false;
}