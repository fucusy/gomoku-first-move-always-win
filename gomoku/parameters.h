#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <stddef.h>
#include <cstdint>


class parameters {
public:
	static const uint_fast8_t DB_SEARCH_MAX_REFUTE_WINS; // max number of refuted wins before the search is cancelled
	static const size_t PN_SEARCH_SIZE_LIMIT_; // limits of the size of the tree
	static const float PN_SEARCH_SELECTOR_CUT_BLACK; // influences branching of the tree, the higher value
												// the faster the search, but the search may become unreliable

    static const float PN_SEARCH_SELECTOR_CUT_WHITE; // influences branching of the tree, the higher value
    // the faster the search, but the search may become unreliable
};


class constants {
public:
	static const int BOARD_SIZE = 15;
};

#endif