#ifndef  COMMON_H
#define  COMMON_H
#include <vector>
#include <queue>
#include <list>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <numeric>
#include <memory>
#include <chrono> 
#include <array>

#define NumDims 5

#define FieldSA 0
#define FieldDA 1
#define FieldSP 2
#define FieldDP 3
#define FieldProto 4

#define LowDim 0
#define HighDim 1
 
#define POINT_SIZE_BITS 32

using namespace std;

typedef uint32_t Point;
typedef Point* BCPacket;

typedef uint32_t Memory;

struct BCInterval {
	Point low;
	Point high;
};

struct BCRule
{
	int	priority;

	//int id;
	//int tag;
	bool markedDelete = 0;

	unsigned prefix_length[NumDims];

	BCInterval range[NumDims];

	bool inline MatchesPacket(const BCPacket p) const {
		for (int i = 0; i < NumDims; i++) {
			if (p[i] < range[i].low || p[i] > range[i].high) return false;
		}
		return true;
	}
	
	bool inline IntersectsRule(const BCRule& r) const {
		for (int i = 0; i < NumDims; i++) {
			if (range[i].high < r.range[i].low || range[i].low > r.range[i].high) return false;
		}
		return true;
	}

	void Print() const {
		for (int i = 0; i < NumDims; i++) {
			printf("%u:%u ", range[i].low, range[i].high);
		}
		printf("\n");
	}
};

// class Random {
// public:
// 	// random number generator from Stroustrup: 
// 	// http://www.stroustrup.com/C++11FAQ.html#std-random
// 	// static: there is only one initialization (and therefore seed).
// 	static int random_int(int low, int high)
// 	{
// 		//static std::mt19937  generator;
// 		using Dist = std::uniform_int_distribution < int >;
// 		static Dist uid{};
// 		return uid(generator, Dist::param_type{ low, high });
// 	}

// 	// random number generator from Stroustrup: 
// 	// http://www.stroustrup.com/C++11FAQ.html#std-random
// 	// static: there is only one initialization (and therefore seed).
// 	static int random_unsigned_int()
// 	{
// 		//static std::mt19937  generator;
// 		using Dist = std::uniform_int_distribution < unsigned int >;
// 		static Dist uid{};
// 		return uid(generator, Dist::param_type{ 0, 4294967295 });
// 	}
// 	static double random_real_btw_0_1()
// 	{
// 		//static std::mt19937  generator;
// 		using Dist = std::uniform_real_distribution < double >;
// 		static Dist uid{};
// 		return uid(generator, Dist::param_type{ 0,1 });
// 	}

// 	template <class T>
// 	static std::vector<T> shuffle_vector(std::vector<T> vi) {
// 		//static std::mt19937  generator;
// 		std::shuffle(std::begin(vi), std::end(vi), generator);
// 		return vi;
// 	}
// private:
// 	static std::mt19937 generator;
// };

inline void SortRules(std::vector<BCRule>& rules) {
	sort(rules.begin(), rules.end(), [](const BCRule& rx, const BCRule& ry) { return rx.priority > ry.priority; });
}

inline void SortRules(std::vector<BCRule*>& rules) {
	sort(rules.begin(), rules.end(), [](const BCRule* rx, const BCRule* ry) { return rx->priority > ry->priority; });
}

inline void PrintRules(const std::vector<BCRule>& rules) {
	for (const BCRule& r : rules) {
		r.Print();
	}
}

#endif
