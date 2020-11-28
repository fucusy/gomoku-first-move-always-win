#include "threat_positions.h"

bool threat_positions::operator==(const threat_positions& other) const {
	if (attack != other.attack || defence.size() != other.defence.size())
		return false;

	for (uint_fast8_t i = 0; i < defence.size(); ++i) {
		if (defence[i] != other.defence[i])
			return false;
	}

	return true;
}