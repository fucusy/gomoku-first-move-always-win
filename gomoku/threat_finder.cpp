#include "threat_finder.h"
#include "board_list_selector.h"
#include <algorithm>
#include <iterator>
#include <iostream>

using namespace std;


// substracts vector b from vector a
possible_positions threat_finder_package::substract_threats(const possible_positions &a, const possible_positions &b) {
    vector<threat_finder_package> output;
    set_difference(a.begin(), a.end(), b.begin(), b.end(), inserter(output, output.begin()),
                   threat_finder_package::ordinal_comparer);
    return output;
}

uint_least8_t threat_finder_package::threat_to_level(dependency_eval_type type) {
    switch (type) {
        case dependency_eval_type::FIVE:
            return 0;
        case dependency_eval_type::FOUR:
        case dependency_eval_type::OPEN_FOUR:
            return 1;
        case dependency_eval_type::THREE:
            return 2;
        default:
            return UINT_LEAST8_MAX;
    }
}


// updates the threat_finder with these new positions
// more specialized functions are implemented for faster computation

void threat_finder::update_positions(const positions_bundle &black_positions, const positions_bundle &white_positions) {
    auto actual_indices = indices_.top(); // intentional copy
    auto actual_output_positions = output_positions_.top();

    for (uint_fast8_t i = 0; i < black_positions.size(); ++i) {
        update_by_attack_position(black_positions[i], actual_indices, actual_output_positions[0], 0, figure::BLACK);
        update_by_defence_position(black_positions[i], actual_indices, actual_output_positions[1], 1, figure::WHITE);
    }

    for (uint_fast8_t i = 0; i < white_positions.size(); ++i) {
        update_by_attack_position(white_positions[i], actual_indices, actual_output_positions[1], 1, figure::WHITE);
        update_by_defence_position(white_positions[i], actual_indices, actual_output_positions[0], 0, figure::BLACK);
    }

    indices_.push(move(actual_indices));
    output_positions_.push(move(actual_output_positions));
}

void threat_finder::update_positions(const positions_bundle &black_positions, const positions_bundle &white_positions,
                                     figure placed_figure) {
    auto actual_indices = indices_.top(); // intentional copy
    auto actual_output_positions = output_positions_.top();

    for (uint_fast8_t i = 0; i < black_positions.size(); ++i) {
        if (placed_figure == figure::BLACK)
            update_by_attack_position(black_positions[i], actual_indices, actual_output_positions[0], 0, figure::BLACK);
        else
            update_by_defence_position(black_positions[i], actual_indices, actual_output_positions[1], 1,
                                       figure::WHITE);
    }

    for (uint_fast8_t i = 0; i < white_positions.size(); ++i) {
        if (placed_figure == figure::WHITE)
            update_by_attack_position(white_positions[i], actual_indices, actual_output_positions[1], 1, figure::WHITE);
        else
            update_by_defence_position(white_positions[i], actual_indices, actual_output_positions[0], 0,
                                       figure::BLACK);
    }

    indices_.push(move(actual_indices));
    output_positions_.push(move(actual_output_positions));
}

void
threat_finder::update_positions(coords black_position, const positions_bundle &white_positions, figure placed_figure) {
    auto actual_indices = indices_.top(); // intentional copy
    auto actual_output_positions = output_positions_.top();

    if (placed_figure == figure::BLACK)
        update_by_attack_position(black_position, actual_indices, actual_output_positions[0], 0, figure::BLACK);
    else
        update_by_defence_position(black_position, actual_indices, actual_output_positions[1], 1, figure::WHITE);

    for (uint_fast8_t i = 0; i < white_positions.size(); ++i) {
        if (placed_figure == figure::WHITE)
            update_by_attack_position(white_positions[i], actual_indices, actual_output_positions[1], 1, figure::WHITE);
        else
            update_by_defence_position(white_positions[i], actual_indices, actual_output_positions[0], 0,
                                       figure::BLACK);
    }

    indices_.push(move(actual_indices));
    output_positions_.push(move(actual_output_positions));
}

void
threat_finder::update_positions(const positions_bundle &black_positions, coords white_position, figure placed_figure) {
    auto actual_indices = indices_.top(); // intentional copy
    auto actual_output_positions = output_positions_.top();

    for (uint_fast8_t i = 0; i < black_positions.size(); ++i) {
        if (placed_figure == figure::BLACK)
            update_by_attack_position(black_positions[i], actual_indices, actual_output_positions[0], 0, figure::BLACK);
        else
            update_by_defence_position(black_positions[i], actual_indices, actual_output_positions[1], 1,
                                       figure::WHITE);
    }

    if (placed_figure == figure::WHITE)
        update_by_attack_position(white_position, actual_indices, actual_output_positions[1], 1, figure::WHITE);
    else
        update_by_defence_position(white_position, actual_indices, actual_output_positions[0], 0, figure::BLACK);

    indices_.push(move(actual_indices));
    output_positions_.push(move(actual_output_positions));
}

void threat_finder::update_position(coords position, figure placed_figure) {
    auto actual_indices = indices_.top(); // intentional copy
    auto actual_output_positions = output_positions_.top();

    if (placed_figure == figure::BLACK)
        update_by_defence_position(position, actual_indices, actual_output_positions[0], 0, figure::BLACK);
    else
        update_by_defence_position(position, actual_indices, actual_output_positions[1], 1, figure::WHITE);

    indices_.push(std::move(actual_indices));
    output_positions_.push(std::move(actual_output_positions));
}


// returns threats of $attack_figure's colour that are in the provided bit_board and are depending on $position

possible_positions threat_finder::find_dependent_threats(const bit_board &board, coords position, figure attack_figure,
                                                         uint_fast8_t max_threat_level) {
    possible_positions output;

    for (uint_fast8_t dir = 0; dir < 4; ++dir) {
        dependency_eval_package package = dependency_eval::evaluate(board.get_line<4>(position, direction(dir)));

        if (package.type != dependency_eval_type::NOTHING) {
            for (auto &&offset : package.friendly_response) {
                coords new_position = position + coords::DIRECTIONS[dir] * offset;
                dependency_eval_package new_package = dependency_eval::evaluate(
                        board.get_line<4>(new_position, attack_figure, direction(dir)));
                auto defence_positions = dependency_eval::offsets_to_positions(new_package.enemy_response, new_position,
                                                                               direction(dir));
                auto level = threat_finder_package::threat_to_level(new_package.type);
                auto best_threat_type = new_package.type;
                threat_finder_package best(new_position, move(defence_positions), direction(dir), new_package.type,
                                           level);


                // select the best threat type for this new position
                for (uint_fast8_t new_dir = 0; new_dir < 4; ++new_dir) {
                    dependency_eval_package package_in_other_dir = dependency_eval::evaluate(
                            board.get_line<4>(new_position, attack_figure, direction(new_dir)));
                    auto defence_positions_in_other_dir =
                            dependency_eval::offsets_to_positions(package_in_other_dir.enemy_response, new_position,
                                                                  direction(new_dir));
                    auto level_in_other_dir = threat_finder_package::threat_to_level(package_in_other_dir.type);
                    if (package_in_other_dir.type < best_threat_type) {
                        // use the better one
                        best_threat_type = package_in_other_dir.type;
                        best = threat_finder_package(new_position, move(defence_positions_in_other_dir),
                                                     direction(new_dir), package_in_other_dir.type,
                                                     level_in_other_dir);
                    }
                }
                output.push_back(best);
            }
        }
    }

    return output;
}

bool threat_finder::is_potential_threat(coords a, coords b) {
    return cache_.get[constants::BOARD_SIZE + a.x - b.x - 1][constants::BOARD_SIZE + a.y - b.y - 1];
}


// initialization of the inner structures

void threat_finder::build() {
    output_positions_.push(possible_positions_pair());
    indices_.push(indices_array());

    for (uint_fast8_t y = 0; y < constants::BOARD_SIZE; ++y) {
        for (uint_fast8_t x = 0; x < constants::BOARD_SIZE; ++x) {
            for (int dir = 0; dir < 4; ++dir) {
                indices_.top()[0][dir][x][y] = UINT_LEAST8_MAX;
                indices_.top()[1][dir][x][y] = UINT_LEAST8_MAX;
            }
        }
    }

    for (uint_fast8_t y = 0; y < constants::BOARD_SIZE; ++y) {
        for (uint_fast8_t x = 0; x < constants::BOARD_SIZE; ++x) {
            coords occupied_position(x, y);
            figure occupied_figure = board_->get_move(occupied_position);
            if (occupied_figure != figure::NONE) {
                initialize_position(occupied_position, occupied_figure, indices_.top(), output_positions_.top());
            }
        }
    }
}

void threat_finder::initialize_position(coords pos, figure attack_figure, indices_array &actual_indices,
                                        possible_positions_pair &actual_output_positions) {
    uint_fast8_t figure_index = attack_figure == figure::BLACK ? 0 : 1;
    for (uint_fast8_t dir = 0; dir < 4; ++dir) {
        dependency_eval_package package = dependency_eval::evaluate(board_->get_line<4>(pos, direction(dir)));
        if (package.type != dependency_eval_type::NOTHING) {
            for (auto &&offset : package.friendly_response) {
                coords new_position = pos + coords::DIRECTIONS[dir] * offset;
                if (actual_indices[figure_index][dir][new_position.x][new_position.y] == UINT_LEAST8_MAX) {
                    auto new_package = dependency_eval::evaluate(
                            board_->get_line<4>(new_position, attack_figure, direction(dir)));
                    auto defence_positions =
                            dependency_eval::offsets_to_positions(
                                    new_package.enemy_response,
                                    new_position,
                                    direction(dir));


                    actual_output_positions[figure_index].push_back(
                            threat_finder_package(new_position, move(defence_positions), direction(dir),
                                                  new_package.type));
                    actual_indices[figure_index][dir][new_position.x][new_position.y] = uint_least8_t(
                            actual_output_positions[figure_index].size() - 1);
                } else {
                    uint_least8_t idx = actual_indices[figure_index][dir][new_position.x][new_position.y];
                    threat_finder_package existing_data = actual_output_positions[figure_index][idx];

                    auto new_package = dependency_eval::evaluate(
                            board_->get_line<4>(new_position, attack_figure, direction(dir)));
                    auto defence_positions =
                            dependency_eval::offsets_to_positions(
                                    new_package.enemy_response,
                                    new_position,
                                    direction(dir));
                    //if  found a better threaten in another direction, use the better one instead

                    if (new_package.type < existing_data.threat_type) {
                        actual_output_positions[figure_index].push_back(
                                threat_finder_package(new_position, move(defence_positions), direction(dir),
                                                      new_package.type));
                        actual_indices[figure_index][dir][new_position.x][new_position.y] = uint_least8_t(
                                actual_output_positions[figure_index].size() - 1);
                    }
                }
            }
        }
    }
}

void threat_finder::update_by_attack_position(coords pos, indices_array &actual_indices,
                                              possible_positions &actual_output_positions, int_fast8_t figure_index,
                                              figure placed_figure) {
    delete_position(actual_indices, actual_output_positions, pos, figure_index);

    for (uint_fast8_t dir = 0; dir < 4; ++dir) {
        auto package = dependency_eval::evaluate(board_->get_line<4>(pos, direction(dir)));

        if (package.type != dependency_eval_type::NOTHING) {
            for (auto &&offset : package.friendly_response) {
                coords new_position = pos + coords::DIRECTIONS[dir] * offset;
                dependency_eval_package new_package = dependency_eval::evaluate(
                        board_->get_line<4>(new_position, placed_figure, direction(dir)));
                auto new_defence_positions = dependency_eval::offsets_to_positions(new_package.enemy_response,
                                                                                   new_position, direction(dir));

                auto level = threat_finder_package::threat_to_level(new_package.type);

                if (actual_indices[figure_index][dir][new_position.x][new_position.y] == UINT_LEAST8_MAX) {
                    actual_output_positions.push_back(
                            threat_finder_package(new_position, move(new_defence_positions), direction(dir),
                                                  new_package.type, level));
                    actual_indices[figure_index][dir][new_position.x][new_position.y] = uint_least8_t(
                            actual_output_positions.size() - 1);
                } else if (level !=
                           actual_output_positions[actual_indices[figure_index][dir][new_position.x][new_position.y]].positions.threat_level) {
                    actual_output_positions[actual_indices[figure_index][dir][new_position.x][new_position.y]] = threat_finder_package(
                            new_position, move(new_defence_positions), direction(dir), new_package.type, level);
                }
            }
        }
    }
}

void threat_finder::update_by_defence_position(coords pos, indices_array &actual_indices,
                                               possible_positions &actual_output_positions, int_fast8_t figure_index,
                                               figure placed_figure) {
    delete_position(actual_indices, actual_output_positions, pos, figure_index);

    for (uint_fast8_t dir = 0; dir < 4; ++dir) {
        for (int_fast8_t offset = -4; offset <= 4; ++offset) {
            coords new_position = pos + coords::DIRECTIONS[dir] * offset;

            if (!new_position.is_out_of_board() &&
                actual_indices[figure_index][dir][new_position.x][new_position.y] != UINT_LEAST8_MAX) {
                dependency_eval_package package = dependency_eval::evaluate(
                        board_->get_line<4>(new_position, placed_figure, direction(dir)));
                auto level = threat_finder_package::threat_to_level(package.type);

                if (level == UINT_LEAST8_MAX) {
                    delete_position(actual_indices, actual_output_positions, new_position, direction(dir),
                                    figure_index);
                } else if (level !=
                           actual_output_positions[actual_indices[figure_index][dir][new_position.x][new_position.y]].positions.threat_level) {
                    auto new_defence_positions = dependency_eval::offsets_to_positions(package.enemy_response,
                                                                                       new_position, direction(dir));
                    actual_output_positions[actual_indices[figure_index][dir][new_position.x][new_position.y]] = threat_finder_package(
                            new_position, move(new_defence_positions), direction(dir), package.type, level);
                }
            }
        }
    }
}

possible_positions
threat_finder::filter_output_positions(const possible_positions &input_list, uint_fast8_t max_level) {
    possible_positions output_list;
    std::copy_if(input_list.begin(), input_list.end(), std::back_inserter(output_list),
                 [=](threat_finder_package p) { return p.positions.threat_level <= max_level; });
    return output_list;
}

void threat_finder::delete_position(indices_array &indexes, possible_positions_pair &output_positions,
                                    coords deleted_position) {
    for (uint_fast8_t dir = 0; dir < 4; ++dir) {
        delete_position(indexes, output_positions[0], deleted_position, direction(dir), 0);
        delete_position(indexes, output_positions[1], deleted_position, direction(dir), 1);
    }
}

void
threat_finder::delete_position(indices_array &indexes, possible_positions &output_positions, coords deleted_position,
                               uint_fast8_t deleted_index) {
    for (uint_fast8_t dir = 0; dir < 4; ++dir) {
        delete_position(indexes, output_positions, deleted_position, direction(dir), deleted_index);
    }
}

void
threat_finder::delete_position(indices_array &indices, possible_positions &output_positions, coords deleted_position,
                               direction deleted_direction, uint_fast8_t deleted_index) {
    if (indices[deleted_index][deleted_direction][deleted_position.x][deleted_position.y] != UINT_LEAST8_MAX) {
        if (indices[deleted_index][deleted_direction][deleted_position.x][deleted_position.y] ==
            output_positions.size() - 1) {
            indices[deleted_index][deleted_direction][deleted_position.x][deleted_position.y] = UINT_LEAST8_MAX;
            output_positions.pop_back();
        } else {
            auto index = indices[deleted_index][deleted_direction][deleted_position.x][deleted_position.y];
            output_positions[index] = move(output_positions.back());
            output_positions.pop_back();

            indices[deleted_index][output_positions[index].dir][output_positions[index].positions.attack.x][output_positions[index].positions.attack.y] = index;
            indices[deleted_index][deleted_direction][deleted_position.x][deleted_position.y] = UINT_LEAST8_MAX;
        }
    }
}


const threat_finder::table threat_finder::cache_;

threat_finder::table::table() {
    for (size_t x = 0; x < 29; ++x) {
        for (size_t y = 0; y < 29; ++y) {
            get[x][y] = false;
        }
    }

    for (int_fast8_t offset = -4; offset <= 4; ++offset) {
        for (uint_fast8_t dir = 0; dir < 4; ++dir) {
            get[14 + coords::DIRECTIONS[dir].x * offset][14 + coords::DIRECTIONS[dir].y * offset] = true;
        }
    }
}