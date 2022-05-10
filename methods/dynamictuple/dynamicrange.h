// #ifndef  DYNAMICRANGE_H
// #define  DYNAMICRANGE_H

// #include "../../elementary.h"
// #include "../../io/io.h"

// #include <cmath>

// #define SPLIT_SELF 1
// #define SPLIT_SRC 2
// #define SPLIT_DST 3
// #define SPLIT_IGNORE 4

// using namespace std;

// struct TupleRange {
//     int x1, y1, x2, y2;
//     TupleRange(int _x1, int _y1, int _x2, int _y2) {
//         x1 = _x1;
//         y1 = _y1;
//         x2 = _x2;
//         y2 = _y2;
//     }
//     TupleRange() {}
// };

// struct DrsRule {
//     uint64_t src_dst_ip;
//     uint64_t reduce_src_dst_ip;
//     int priority;
//     int check_num;
//     char src_prefix_len, dst_prefix_len;
//     bool first;
// };

// struct TupleCost {
//     uint64_t cost;
//     char split_type;
//     char split_prefix_len;
// };

// struct TupleInfo {
//     uint64_t check_tuple_num;
//     uint64_t contain_group_num;
//     uint64_t match_group_num;
//     uint64_t collide_group_num;
//     uint64_t check_rule_num;

//     void Init() {
//         check_tuple_num = 0;
//         contain_group_num = 0;
//         match_group_num = 0;
//         collide_group_num = 0;
//         check_rule_num = 0;
//     }
// };

// vector<TupleRange> TupleRangeStep(int step);
// vector<TupleRange> DynamicTupleRange(vector<Rule*> &rules, double &cal_time, vector<TupleRange> pre_tuple_ranges);
// vector<TupleRange> PextTupleRange(vector<Rule*> &rules, double &cal_time, vector<TupleRange> pre_tuple_ranges);
// void PrintTupleRanges(vector<TupleRange> &tuple_ranges);

// #endif