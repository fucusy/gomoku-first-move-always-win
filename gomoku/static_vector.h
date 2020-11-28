#ifndef STATIC_VECTOR_H
#define STATIC_VECTOR_H

#include <cstdint>
#include <array>
#include <stdexcept>

/* STATIC VECTOR
 * 
 * Implementation of a vector container for a small value type $T with small upper bound on number of elements $SIZE
 * 
 * Doesn't offer anything more than standard vector other than speedup for containers that are very small.
 * It is used in dependency-based search, because there is a need for an array of variable length, that maximally contains 
 * only 5 elements, each 2 bytes long.
 * 
 */

template<typename T, uint_fast8_t SIZE>
class static_vector {
public:
	static_vector() : size_(0) {}

	void push_back(T item) {
#ifdef _DEBUG
		if (size_ >= SIZE) throw std::runtime_error("Out of bounds");
#endif

		list_[size_] = item; ++size_;
	}

	T operator [] (uint_fast8_t index) const {
#ifdef _DEBUG
		if (index >= SIZE) throw std::runtime_error("Out of bounds");
#endif

		return list_[index];
	}

	uint_fast8_t size() const { return size_; }

private:
	uint_fast8_t size_;
	std::array<T, SIZE> list_;
};

#endif
