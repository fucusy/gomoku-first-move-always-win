#ifndef SELECTOR_HELPER_H
#define SELECTOR_HELPER_H

#include "coords.h"


/* helper arrays for offset values
 *
 * the values are offsets of positions in a star-like formation frim the middle figure, i.e:
 * 
 * X  X  X
 *  X X X
 *   XXX
 * XXX XXX
 *   XXX
 *  X X X
 * X  X  X
 * 
 */

const coords_offset inner_star[16] = {
	coords_offset( 0,  1), coords_offset( 0,  2),
	coords_offset( 1,  1), coords_offset( 2,  2),
	coords_offset( 1,  0), coords_offset( 2,  0),
	coords_offset( 1, -1), coords_offset( 2, -2),
	coords_offset( 0, -1), coords_offset( 0, -2),
	coords_offset(-1, -1), coords_offset(-2, -2),
	coords_offset(-1,  0), coords_offset(-2,  0),
	coords_offset(-1,  1), coords_offset(-2,  2)
};

const coords_offset middle_star[16] = {
	coords_offset( 0,  3), coords_offset( 0,  4),
	coords_offset( 3,  3), coords_offset( 4,  4),
	coords_offset( 3,  0), coords_offset( 4,  0),
	coords_offset( 3, -3), coords_offset( 4, -4),
	coords_offset( 0, -3), coords_offset( 0, -4),
	coords_offset(-3, -3), coords_offset(-4, -4),
	coords_offset(-3,  0), coords_offset(-4,  0),
	coords_offset(-3,  3), coords_offset(-4,  4)
};

const coords_offset outer_star[8] = {
	coords_offset( 0,  5),
	coords_offset( 5,  5),
	coords_offset( 5,  0),
	coords_offset( 5, -5), 
	coords_offset( 0, -5), 
	coords_offset(-5, -5),
	coords_offset(-5,  0),
	coords_offset(-5,  5),
};

#endif