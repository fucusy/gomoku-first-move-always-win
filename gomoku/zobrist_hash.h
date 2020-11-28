#ifndef ZOBRIST_HASH_H
#define ZOBRIST_HASH_H

#include "coords.h"

#include <cstdint>
#include <random>


/* POSITION HASH
 * 
 * Each possible position (coordinates x figure) is given a random uint64_t value, that
 * is then used in zobrist hash to hash the whole board
 * 
 */

struct position_hash {
public:
	uint64_t get[2][constants::BOARD_SIZE][constants::BOARD_SIZE];

	position_hash() {
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < constants::BOARD_SIZE; j++)
				for (int k = 0; k < constants::BOARD_SIZE; k++)
					get[i][j][k] = get_random_uint64();
	}

private:
	static uint64_t get_random_uint64() {
		static std::random_device rd;
		static std::mt19937_64 gen(rd());
		static std::uniform_int_distribution<uint64_t> dis;

		return dis(gen);
	}
};


/* ZOBRIST HASH
 *
 * Hashes from all possible board states (3^225) into uint_64 (2^64).
 * 
 * The algorithm is best described here - https://chessprogramming.wikispaces.com/Zobrist+Hashing
 *
 */

class zobrist_hash {
public:
	zobrist_hash() : actual_(0) {}

	uint64_t actual() const { return actual_; }
	void update(coords position, uint_fast8_t colour) {
		actual_ ^= position_hash_.get[colour][position.x][position.y];
	}
	void restart() { actual_ = 0; }

private:
	uint64_t actual_;
	static position_hash position_hash_;
};

#endif