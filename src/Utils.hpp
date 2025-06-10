#include <random>

static inline int getRandomNumber(int lower, int upper) {
	if (lower > upper) {
		std::swap(lower, upper);
	}

	static std::random_device rd; 
	static std::mt19937 gen(rd()); 

	std::uniform_int_distribution<> dist(lower, upper);
	return dist(gen);
}