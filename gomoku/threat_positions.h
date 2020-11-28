#ifndef THREAT_POSITIONS_H
#define THREAT_POSITIONS_H

#include "coords.h"
#include "static_vector.h"
#include "parameters.h"


using positions_bundle = static_vector<coords, 4>;

/* THREAT POSITIONS
 * 
 * Structure containing an attacking move, defence moves responding to that move
 * and the level of the threat of the attacking move.
 *
 */

struct threat_positions {
	coords attack;
	positions_bundle defence; // there are max 4 possible defence responses to a threat
	uint_least8_t threat_level;

	threat_positions(coords attack, positions_bundle&& defence, uint_least8_t threat_level) : attack(attack), defence(defence), threat_level(threat_level) {}

	bool operator ==(const threat_positions& other) const ;
	bool operator !=(const threat_positions& other) const { return !(*this == other); }

	// simple ordering that can be used for a fast artificial sort of this structure
	static bool ordinal_comparer(const threat_positions& a, const threat_positions& b) {
		return a.attack.y * constants::BOARD_SIZE + a.attack.x > b.attack.y * constants::BOARD_SIZE + b.attack.x;
	}
};

#endif