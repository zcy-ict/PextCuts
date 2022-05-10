// #ifndef  DIMRANGE_H
// #define  DIMRANGE_H

// #include "../../elementary.h"
// #include "../../io/io.h"

// #include <cmath>

// struct DimRange {
// 	int x1, x2;
// };

// struct DimsRange {
// 	vector<DimRange> dims[5];
// 	char reduced[5][33];
// };

// struct DimKey {
// 	uint32_t key[5];
// 	bool operator<(const DimKey& rule)const{
// 		for (int i = 0; i < 4; ++i)
// 			if (key[i] != rule.key[i])
// 				return key[i] < rule.key[i];
// 		return key[4] < rule.key[4];
//     }
// };

// struct DimRule {
// 	uint32_t key[5];
// 	uint32_t reduced_key[5];
// 	char prefix_len[5];
// 	int priority;
//     int check_num;
//     bool first;
// };

// DimsRange GetDimRange(vector<Rule*> &rules, int prefix_dims_num);
// #endif