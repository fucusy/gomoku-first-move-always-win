#include "parameters.h"

const uint_fast8_t parameters::DB_SEARCH_MAX_REFUTE_WINS = 10;

const float parameters::PN_SEARCH_SELECTOR_CUT_BLACK = 0.0f;
// influences branching of the tree, the higher value
// the faster the search, but the search may become unreliable

const float parameters::PN_SEARCH_SELECTOR_CUT_WHITE = 0.5f; // influences branching of the tree, the higher value
														// the faster the search, but the search may become unreliable

const size_t parameters::PN_SEARCH_SIZE_LIMIT_ = 1000L * 1000 * 1000 * 1000; // 10B, limits of the size of the tree
