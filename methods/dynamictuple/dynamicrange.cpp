// #include "dynamicrange.h"

// using namespace std;

// uint64_t check_hash_cost = 5;  // us
// uint64_t check_group_cost = 2;
// uint64_t check_rule_cost = 3;

// uint32_t INF = 1e9;

// bool drs_print = false;
// bool drs_print_num = false;

// // Init
// uint64_t ip_mask[33];
// uint64_t ip_mask_pair[33][33];
// uint32_t ip_num[33][33];
// uint32_t ip_num_sum[33][33];
// uint64_t max_priority[33][33];
// bool pre_tuples_prefix[33][33];

// // X Y
// uint64_t check_tuple_num[33][33];
// uint64_t group_vis[33][33];
// uint64_t contain_group_num[33][33];
// uint64_t match_group_num[33][33];
// uint64_t collide_group_num[33][33];
// uint64_t check_rule_num[33][33];

// // global
// TupleInfo tuple_info[33][33][33][33];
// TupleCost tuple_cost[33][33][33][33];

// extern TupleInfo main_tuple_info;

// //void GetDrsTupleRange
// vector<TupleRange> TupleRangeStep(int step) {
//     vector<TupleRange> tuple_ranges;
//     for (int x1 = 0; x1 <= 32; x1 += step)
//         for (int y1 = 0; y1 <= 32; y1 += step) {
//             int x2 = min(x1 + step - 1, 32);
//             int y2 = min(y1 + step - 1, 32);
//             TupleRange tuple_range(x1, y1, x2, y2);
//             tuple_ranges.push_back(tuple_range);
//         }
//     return tuple_ranges;
// }

// vector<DrsRule*> GetDrsRules(vector<Rule*> &rules) {
//     int rules_num = rules.size();
//     vector<DrsRule*> drs_rules;
//     //sort(rules, rules + rules_num, CmpRulePriority);
//     sort(rules.begin(), rules.end(), CmpRulePriority);
//     for (int i = 0; i < rules_num; ++i) {
//         DrsRule *drs_rule = (DrsRule*)malloc(sizeof(DrsRule));
//         drs_rule->src_dst_ip = (uint64_t)rules[i]->range[0][0] << 32 | rules[i]->range[1][0];
//         drs_rule->src_prefix_len = rules[i]->prefix_len[0];
//         drs_rule->dst_prefix_len = rules[i]->prefix_len[1];
//         drs_rule->priority = rules_num - i;
//         drs_rules.push_back(drs_rule);
//     }
//     return drs_rules;
// }

// int FreeDrsRules(vector<DrsRule*> &drs_rules) {
//     int rules_num = drs_rules.size();
//     for (int i = 0; i < rules_num; ++i)
//         free(drs_rules[i]);
//     drs_rules.clear();
//     return 0;
// }

// void DrsInit(vector<DrsRule*> &drs_rules, vector<TupleRange> pre_tuple_ranges) {
//     int rules_num = drs_rules.size();
//     // 计算mask
//     ip_mask[0] = 0;
//     for (int i = 1; i <= 32; ++i)
//         ip_mask[i] = ip_mask[i-1] | (1ULL << (32 - i));
//     for (int i = 0; i <= 32; ++i)
//         for (int j = 0; j <= 32; ++j)
//             ip_mask_pair[i][j] = ip_mask[i] << 32 | ip_mask[j];

//     // 计算ip_num和priority
//     memset(ip_num, 0, sizeof(ip_num));
//     memset(max_priority, 0, sizeof(max_priority));
//     for (int i = rules_num - 1; i >= 0; --i) {
//         int x = drs_rules[i]->src_prefix_len;
//         int y = drs_rules[i]->dst_prefix_len;
//         ++ip_num[x][y];
//         max_priority[x][y] = drs_rules[i]->priority;
//     }

//     for (int x = 0; x <= 32; ++x)
//         for (int y = 0; y <= 32; ++y) {
//             ip_num_sum[x][y] = ip_num[x][y];
//             if (x > 0)
//                 ip_num_sum[x][y] += ip_num_sum[x - 1][y];
//             if (y > 0)
//                 ip_num_sum[x][y] += ip_num_sum[x][y - 1];
//             if (x > 0 && y > 0)
//                 ip_num_sum[x][y] -= ip_num_sum[x - 1][y - 1];
//         }
//     memset(pre_tuples_prefix, 0, sizeof(pre_tuples_prefix));
//     for (int i = 0; i < pre_tuple_ranges.size(); ++i)
//         pre_tuples_prefix[pre_tuple_ranges[i].x1][pre_tuple_ranges[i].y1] = 1;

//     if (drs_print_num) {
//         for (int y = 32; y >= 0; --y) {
//             for (int x = 0; x <= 32; ++x)
//                 printf("%d ", ip_num[x][y]);
//             printf("\n");
//         }
//         printf("\n");
//     }
// }

// uint32_t GetIpNumSum(int x1, int y1, int x2 ,int y2) {
//     uint64_t ans = ip_num_sum[x2][y2];
//     if (x1 > 0)
//         ans -= ip_num_sum[x1 - 1][y2];
//     if (y1 > 0)
//         ans -= ip_num_sum[x2][y1 - 1];
//     if (x1 > 0 && y1 > 0)
//         ans += ip_num_sum[x1 - 1][y1 - 1];
//     return ans;
// }

// bool CmpDrsRulesIpPriority(DrsRule *drs_rule1, DrsRule *drs_rule2) {
//     if (drs_rule1->reduce_src_dst_ip != drs_rule2->reduce_src_dst_ip)
//         return drs_rule1->reduce_src_dst_ip < drs_rule2->reduce_src_dst_ip;
//     return drs_rule1->priority > drs_rule2->priority;
// }

// void DrsCalculateXY(vector<DrsRule*> &drs_rules, int x1, int y1, bool accelerate) {
//     int rules_num = drs_rules.size();
//     if (accelerate && (GetIpNumSum(x1, y1, x1, 32) == 0 || GetIpNumSum(x1, y1, 32, y1) == 0)) {
//         for (int x2 = x1; x2 <= 32; ++x2)
//             for (int y2 = y1; y2 <= 32; ++y2) {
//                 if (GetIpNumSum(x1, y1, x2, y2) == 0) {
//                     tuple_cost[x1][y1][x2][y2].cost = 0;
//                     tuple_info[x1][y1][x2][y2].Init();
//                 }
//                 else
//                     tuple_cost[x1][y1][x2][y2].cost = INF;
//                 tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SELF;
//             }
//         return;
//     }
    
//     // reduce rule ip
//     for (int i = 0; i < rules_num; ++i)
//         drs_rules[i]->reduce_src_dst_ip = drs_rules[i]->src_dst_ip & ip_mask_pair[x1][y1];
//     sort(drs_rules.begin(), drs_rules.end(), CmpDrsRulesIpPriority);
    
//     // tuple
//     check_tuple_num[x1][y1] = max_priority[x1][y1];
//     for (int x = x1 + 1; x <= 32; ++x)
//         check_tuple_num[x][y1] = max(check_tuple_num[x - 1][y1], max_priority[x][y1]);
//     for (int y = y1 + 1; y <= 32; ++y)
//         check_tuple_num[x1][y] = max(check_tuple_num[x1][y - 1], max_priority[x1][y]);
//     for (int x = x1 + 1; x <= 32; ++x)
//         for (int y = y1 + 1; y <= 32; ++y)
//             check_tuple_num[x][y] = max(max(check_tuple_num[x - 1][y], check_tuple_num[x][y - 1]), max_priority[x][y]);

//     for (int x2 = x1; x2 <= 32; ++x2)
//         for (int y2 = y1; y2 <= 32; ++y2)
//             tuple_info[x1][y1][x2][y2].check_tuple_num = check_tuple_num[x2][y2];

//     // group && rule
//     drs_rules[rules_num - 1]->check_num = 1;
//     for (int i = rules_num - 2; i >= 0; --i)
//         if (drs_rules[i]->reduce_src_dst_ip == drs_rules[i + 1]->reduce_src_dst_ip) {
//             drs_rules[i + 1]->first = false;
//             drs_rules[i]->check_num = drs_rules[i + 1]->check_num + 1;
//         } else {
//             drs_rules[i + 1]->first = true;
//             drs_rules[i]->check_num = 1;
//         }
//     drs_rules[0]->first = true;

//     int group_num = 0;
//     memset(group_vis, 0, sizeof(group_vis));
//     memset(contain_group_num, 0, sizeof(contain_group_num));
//     memset(match_group_num, 0, sizeof(match_group_num));
//     memset(collide_group_num, 0, sizeof(collide_group_num));
//     memset(check_rule_num, 0, sizeof(check_rule_num));
    
//     // match_group_num
//     for (int i = 0; i < rules_num; ++i) {
//         int px = drs_rules[i]->src_prefix_len;
//         int py = drs_rules[i]->dst_prefix_len;
//         if (drs_rules[i]->first)
//             ++group_num;
//         if (px < x1 || py < y1)
//             continue;
//         for (int x = px; x <= 32; ++x) {
//             if (group_vis[x][y1] == group_num)
//                 break;
//             for (int y = py; y <= 32; ++y) {
//                 if (group_vis[x][y] == group_num)
//                     break;
//                 group_vis[x][y] = group_num;
//                 contain_group_num[x][y] += 1;
//                 match_group_num[x][y] += drs_rules[i]->check_num;
//                 collide_group_num[x][y] += drs_rules[i]->priority - drs_rules[i]->check_num;
//             }
//         }
//         check_rule_num[px][py] += drs_rules[i]->check_num;
//     }
    
//     // collide_group_num
//     for (int x2 = x1; x2 <= 32; ++x2)
//         for (int y2 = y1; y2 <= 32; ++y2) {
//             int bucket_size = 32;
//             while (bucket_size * 0.85 <= contain_group_num[x2][y2])
//                 bucket_size <<= 1;
//             collide_group_num[x2][y2] = 1.0 * collide_group_num[x2][y2] / bucket_size;
//         }
    
//     // check_rule_num
//     for (int x = x1 + 1; x <= 32; ++x)
//         check_rule_num[x][y1] += check_rule_num[x - 1][y1];
//     for (int y = y1 + 1; y <= 32; ++y)
//         check_rule_num[x1][y] += check_rule_num[x1][y - 1];
//     for (int x = x1 + 1; x <= 32; ++x)
//         for (int y = y1 + 1; y <= 32; ++y)
//             check_rule_num[x][y] += check_rule_num[x][y - 1] + check_rule_num[x - 1][y] - check_rule_num[x - 1][y - 1];
    
//     // summary
//     for (int x2 = x1; x2 <= 32; ++x2)
//         for (int y2 = y1; y2 <= 32; ++y2) {
//             tuple_info[x1][y1][x2][y2].contain_group_num = contain_group_num[x2][y2];
//             tuple_info[x1][y1][x2][y2].match_group_num = match_group_num[x2][y2];
//             tuple_info[x1][y1][x2][y2].collide_group_num = collide_group_num[x2][y2];
//             tuple_info[x1][y1][x2][y2].check_rule_num = check_rule_num[x2][y2];
//         }

//     for (int x2 = x1; x2 <= 32; ++x2)
//         for (int y2 = y1; y2 <= 32; ++y2) {
//             tuple_cost[x1][y1][x2][y2].cost = check_tuple_num[x2][y2] * check_hash_cost +
//                                               (match_group_num[x2][y2] + collide_group_num[x2][y2]) * check_group_cost +
//                                               check_rule_num[x2][y2] * check_rule_cost;
//             tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SELF;
//         }
// }

// void DrsCalculate(vector<DrsRule*> &drs_rules, bool accelerate) {
//     for (int x1 = 32; x1 >= 0; --x1)
//         for (int y1 = 32; y1 >= 0; --y1)
//             DrsCalculateXY(drs_rules, x1, y1, accelerate && !pre_tuples_prefix[x1][y1]);
// }

// void ReducePreTupleRanges(vector<TupleRange> pre_tuple_ranges) {
//     for (int i = 0; i < pre_tuple_ranges.size(); ++i) {
//         TupleRange tuple_range = pre_tuple_ranges[i];
//         tuple_cost[tuple_range.x1][tuple_range.y1][tuple_range.x2][tuple_range.y2].cost *= 0.9;
//     }
// }

// void DynamicProgrammingXY(int x1, int y1) {
//     for (int x2 = x1; x2 <= 32; ++x2) {
//         for (int y2 = y1; y2 <= 32; ++y2) {
//             if (tuple_cost[x1][y1][x2][y2].cost == 0)
//                 continue;
//             if (tuple_cost[x2][y1][x2][y2].cost == 0) {
//                 tuple_cost[x1][y1][x2][y2]= tuple_cost[x1][y1][x2 - 1][y2];
//                 continue;
//             }
//             if (tuple_cost[x1][y2][x2][y2].cost == 0) {
//                 tuple_cost[x1][y1][x2][y2]= tuple_cost[x1][y1][x2][y2 - 1];
//                 continue;
//             }
//             if (tuple_cost[x1][y1][x1][y2].cost == 0) {
//                 if (tuple_cost[x1 + 1][y1][x1 + 1][y2].cost != 0) {
//                     tuple_cost[x1][y1][x2][y2].cost = tuple_cost[x1 + 1][y1][x2][y2].cost;
//                     tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SRC;
//                     tuple_cost[x1][y1][x2][y2].split_prefix_len = x1;
//                 } else {
//                     tuple_cost[x1][y1][x2][y2]= tuple_cost[x1 + 1][y1][x2][y2];
//                 }
//                 continue;
//             }
//             if (tuple_cost[x1][y1][x2][y1].cost == 0) {
//                 if (tuple_cost[x1][y1 + 1][x2][y1 + 1].cost != 0) {
//                     tuple_cost[x1][y1][x2][y2].cost = tuple_cost[x1][y1 + 1][x2][y2].cost;
//                     tuple_cost[x1][y1][x2][y2].split_type = SPLIT_DST;
//                     tuple_cost[x1][y1][x2][y2].split_prefix_len = y1;
//                 } else {
//                     tuple_cost[x1][y1][x2][y2]= tuple_cost[x1][y1 + 1][x2][y2];
//                 }
//                 continue;
//             }

//             for (int x = x1; x < x2; ++x) {
//                 uint32_t new_cost = tuple_cost[x1][y1][x][y2].cost + tuple_cost[x + 1][y1][x2][y2].cost;
//                 if (new_cost < tuple_cost[x1][y1][x2][y2].cost 
//                     //|| (new_cost == tuple_cost[x1][y1][x2][y2].cost && tuple_cost[x + 1][y1][x2][y2].cost > 0)
//                     ) {
//                     tuple_cost[x1][y1][x2][y2].cost = new_cost;
//                     tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SRC;
//                     tuple_cost[x1][y1][x2][y2].split_prefix_len = x;
//                 }
//             }

//             for (int y = y1; y < y2; ++y) {
//                 uint32_t new_cost = tuple_cost[x1][y1][x2][y].cost + tuple_cost[x1][y + 1][x2][y2].cost;
//                 if (new_cost < tuple_cost[x1][y1][x2][y2].cost 
//                     //|| (new_cost == tuple_cost[x1][y1][x2][y2].cost && tuple_cost[x1][y + 1][x2][y2].cost > 0)
//                     ) {
//                     tuple_cost[x1][y1][x2][y2].cost = new_cost;
//                     tuple_cost[x1][y1][x2][y2].split_type = SPLIT_DST;
//                     tuple_cost[x1][y1][x2][y2].split_prefix_len = y;
//                 }
//             }
//         }
//     }
// }


// void DynamicProgramming() {
//     for (int x1 = 32; x1 >= 0; --x1)
//         for (int y1 = 32; y1 >= 0; --y1)
//             DynamicProgrammingXY(x1, y1);
// }

// void TupleInfoSum(TupleInfo &tuple_info_sum, int x1, int y1, int x2, int y2) {
//     int check_tuple_num = tuple_info[x1][y1][x2][y2].check_tuple_num;
//     int contain_group_num = tuple_info[x1][y1][x2][y2].contain_group_num;
//     int match_group_num = tuple_info[x1][y1][x2][y2].match_group_num;
//     int collide_group_num = tuple_info[x1][y1][x2][y2].collide_group_num;
//     int check_rule_num = tuple_info[x1][y1][x2][y2].check_rule_num;
//     int ip_num = GetIpNumSum(x1, y1, x2, y2);
//     int tuple_cost_num = tuple_cost[x1][y1][x2][y2].cost;

//     tuple_info_sum.check_tuple_num += check_tuple_num;
//     tuple_info_sum.contain_group_num += contain_group_num;
//     tuple_info_sum.match_group_num += match_group_num;
//     tuple_info_sum.collide_group_num += collide_group_num;
//     tuple_info_sum.check_rule_num += check_rule_num;

//     if (drs_print) {
//         printf("%d, %d, %d, %d \\\\  ", x1, y1, x2, y2);
//         printf("check_tuple_num %d contain_group_num %d match_group_num %d collide_group_num %d check_rule_num %d  ", 
//                 check_tuple_num, contain_group_num, match_group_num, collide_group_num, check_rule_num);
//         printf("ip_num %d tuple_cost_num %d\n", ip_num, tuple_cost_num);
//     }
// }
// void AddTupleRanges(vector<TupleRange> &tuple_ranges, TupleInfo &tuple_info_sum,
//                     int x1, int y1,  int x2, int y2) {
//     if (tuple_cost[x1][y1][x2][y2].split_type == SPLIT_SELF) {
//         TupleRange tuple_range(x1, y1, x2, y2);
//         tuple_ranges.push_back(tuple_range);
//         TupleInfoSum(tuple_info_sum, x1, y1, x2, y2);
//     } else if (tuple_cost[x1][y1][x2][y2].split_type == SPLIT_SRC) {
//         int x = tuple_cost[x1][y1][x2][y2].split_prefix_len;
//         AddTupleRanges(tuple_ranges, tuple_info_sum, x1, y1, x, y2);
//         AddTupleRanges(tuple_ranges, tuple_info_sum, x + 1, y1, x2, y2);
//     } else if (tuple_cost[x1][y1][x2][y2].split_type == SPLIT_DST) {
//         int y = tuple_cost[x1][y1][x2][y2].split_prefix_len;
//         AddTupleRanges(tuple_ranges, tuple_info_sum, x1, y1, x2, y);
//         AddTupleRanges(tuple_ranges, tuple_info_sum, x1, y + 1, x2, y2);
//     }
// }

// vector<TupleRange> GetTupleRanges(int x1, int y1, int x2, int y2) {
//     vector<TupleRange> tuple_ranges;
//     TupleInfo tuple_info_sum;
//     tuple_info_sum.Init();
//     AddTupleRanges(tuple_ranges, tuple_info_sum, x1, y1, x2, y2);
//     main_tuple_info = tuple_info_sum;
//     if (drs_print) {
//         printf("check_tuple_sum %ld\n", tuple_info_sum.check_tuple_num);
//         printf("contain_group_sum %ld\n", tuple_info_sum.contain_group_num);
//         printf("match_group_sum %ld\n", tuple_info_sum.match_group_num);
//         printf("collide_group_sum %ld\n", tuple_info_sum.collide_group_num);
//         printf("check_rule_sum %ld\n", tuple_info_sum.check_rule_num);
//         printf("tuple_cost_sum %ld\n", tuple_cost[0][0][32][32].cost);
//     }
//     return tuple_ranges;
// }

// vector<TupleRange> DynamicTupleRange(vector<Rule*> &rules, double &cal_time,
//                                      vector<TupleRange> pre_tuple_ranges) {
//     int rules_num = rules.size();
//     if (rules_num == 0)
//         return TupleRangeStep(8);
    
//     timeval times[10];
// 	gettimeofday(&times[0],NULL);

//     // vector<Rule*> rules2 = rules;
//     // random_shuffle(rules2.begin(),rules2.end());
//     // rules2.resize(rules_num / 10);
//     // vector<DrsRule*> drs_rules = GetDrsRules(rules2);

//     vector<DrsRule*> drs_rules = GetDrsRules(rules);
// 	gettimeofday(&times[1],NULL);
//     uint32_t timer = GetRunTimeUs(times[0], times[1]);
//     //printf("GetDrsRules %d us\n", timer);

//     DrsInit(drs_rules, pre_tuple_ranges);
// 	gettimeofday(&times[2],NULL);
//     timer = GetRunTimeUs(times[1], times[2]);
//     //printf("DrsInit %d us\n", timer);


//     DrsCalculate(drs_rules, true);
// 	gettimeofday(&times[3],NULL);
//     timer = GetRunTimeUs(times[2], times[3]);
//     //printf("DrsCalculate %d us\n", timer);

//     ReducePreTupleRanges(pre_tuple_ranges);

//     DynamicProgramming();
// 	gettimeofday(&times[4],NULL);
//     timer = GetRunTimeUs(times[3], times[4]);
//     //printf("Dynamic %d us\n", timer);

//     FreeDrsRules(drs_rules);

//     cal_time = GetRunTimeUs(times[0], times[4]) / 1000000.0;

//     vector<TupleRange> tuple_ranges = GetTupleRanges(0, 0, 32, 32);
//     //exit(1);
//     return tuple_ranges;
// }

// void PrintTupleRanges(vector<TupleRange> &tuple_ranges) {
//     int tuple_ranges_num = tuple_ranges.size();
//     printf("%d\n", tuple_ranges_num);
//     for (int i = 0; i < tuple_ranges_num; ++i)
//        printf("%d %d %d %d\n", tuple_ranges[i].x1, tuple_ranges[i].y1, tuple_ranges[i].x2, tuple_ranges[i].y2);
// }


// int Log2(int num) {
//     int ans = 0;
//     while (num) {
//         num /= 2;
//         ++ans;
//     }
//     return ans;
// }

// int pext_check_tuple_cost = 5;
// int pext_check_rule_cost = 3;


// // PextCuts

// void PextCalculateXY(vector<DrsRule*> &drs_rules, int x1, int y1) {
//     int rules_num = drs_rules.size();
//     if (GetIpNumSum(x1, y1, x1, 32) == 0 || GetIpNumSum(x1, y1, 32, y1) == 0) {
//         for (int x2 = x1; x2 <= 32; ++x2)
//             for (int y2 = y1; y2 <= 32; ++y2) {
//                 if (GetIpNumSum(x1, y1, x2, y2) == 0) {
//                     tuple_cost[x1][y1][x2][y2].cost = 0;
//                     tuple_info[x1][y1][x2][y2].Init();
//                 }
//                 else
//                     tuple_cost[x1][y1][x2][y2].cost = INF;
//                 tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SELF;
//             }
//         return;
//     }
    
//     // reduce rule ip
//     for (int i = 0; i < rules_num; ++i)
//         drs_rules[i]->reduce_src_dst_ip = drs_rules[i]->src_dst_ip & ip_mask_pair[x1][y1];
//     sort(drs_rules.begin(), drs_rules.end(), CmpDrsRulesIpPriority);
    
//     // tuple
//     check_tuple_num[x1][y1] = max_priority[x1][y1];
//     for (int x = x1 + 1; x <= 32; ++x)
//         check_tuple_num[x][y1] = max(check_tuple_num[x - 1][y1], max_priority[x][y1]);
//     for (int y = y1 + 1; y <= 32; ++y)
//         check_tuple_num[x1][y] = max(check_tuple_num[x1][y - 1], max_priority[x1][y]);
//     for (int x = x1 + 1; x <= 32; ++x)
//         for (int y = y1 + 1; y <= 32; ++y)
//             check_tuple_num[x][y] = max(max(check_tuple_num[x - 1][y], check_tuple_num[x][y - 1]), max_priority[x][y]);

//     for (int x2 = x1; x2 <= 32; ++x2)
//         for (int y2 = y1; y2 <= 32; ++y2)
//             tuple_info[x1][y1][x2][y2].check_tuple_num = check_tuple_num[x2][y2];

//     // group && rule
//     drs_rules[rules_num - 1]->check_num = 1;
//     for (int i = rules_num - 2; i >= 0; --i)
//         if (drs_rules[i]->reduce_src_dst_ip == drs_rules[i + 1]->reduce_src_dst_ip) {
//             drs_rules[i + 1]->first = false;
//             drs_rules[i]->check_num = drs_rules[i + 1]->check_num + 1;
//         } else {
//             drs_rules[i + 1]->first = true;
//             drs_rules[i]->check_num = 1;
//         }
//     drs_rules[0]->first = true;

//     int group_num = 0;
//     memset(group_vis, 0, sizeof(group_vis));
//     memset(contain_group_num, 0, sizeof(contain_group_num));
//     memset(match_group_num, 0, sizeof(match_group_num));
//     memset(collide_group_num, 0, sizeof(collide_group_num));
//     memset(check_rule_num, 0, sizeof(check_rule_num));

//     for (int i = 0; i < rules_num; ++i) {
//         int px = drs_rules[i]->src_prefix_len;
//         int py = drs_rules[i]->dst_prefix_len;
//         check_rule_num[px][py] += Log2(drs_rules[i]->check_num);
//     }
    
//     // check_rule_num
//     for (int x = x1 + 1; x <= 32; ++x)
//         check_rule_num[x][y1] += check_rule_num[x - 1][y1];
//     for (int y = y1 + 1; y <= 32; ++y)
//         check_rule_num[x1][y] += check_rule_num[x1][y - 1];
//     for (int x = x1 + 1; x <= 32; ++x)
//         for (int y = y1 + 1; y <= 32; ++y)
//             check_rule_num[x][y] += check_rule_num[x][y - 1] + check_rule_num[x - 1][y] - check_rule_num[x - 1][y - 1];
    
//     for (int x2 = x1; x2 <= 32; ++x2)
//         for (int y2 = y1; y2 <= 32; ++y2) {
//             tuple_cost[x1][y1][x2][y2].cost = check_tuple_num[x2][y2] * pext_check_tuple_cost +
//                                               check_rule_num[x2][y2] * pext_check_rule_cost;
//             tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SELF;
//         }
// }

// void PextCalculate(vector<DrsRule*> &drs_rules) {
//     for (int x1 = 32; x1 >= 0; --x1)
//         for (int y1 = 32; y1 >= 0; --y1)
//             PextCalculateXY(drs_rules, x1, y1);
// }

// vector<TupleRange> PextTupleRange(vector<Rule*> &rules, double &cal_time,
//                                      vector<TupleRange> pre_tuple_ranges) {
//     int rules_num = rules.size();
//     if (rules_num == 0)
//         return TupleRangeStep(8);
    
//     timeval times[10];
//     gettimeofday(&times[0],NULL);

//     // vector<Rule*> rules2 = rules;
//     // random_shuffle(rules2.begin(),rules2.end());
//     // rules2.resize(rules_num / 10);
//     // vector<DrsRule*> drs_rules = GetDrsRules(rules2);

//     vector<DrsRule*> drs_rules = GetDrsRules(rules);
//     gettimeofday(&times[1],NULL);
//     uint32_t timer = GetRunTimeUs(times[0], times[1]);
//     //printf("GetDrsRules %d us\n", timer);

//     DrsInit(drs_rules, pre_tuple_ranges);
//     gettimeofday(&times[2],NULL);
//     timer = GetRunTimeUs(times[1], times[2]);
//     //printf("DrsInit %d us\n", timer);


//     // DrsCalculate(drs_rules, true);
//     PextCalculate(drs_rules);
//     gettimeofday(&times[3],NULL);
//     timer = GetRunTimeUs(times[2], times[3]);
//     //printf("PextCalculate %d us\n", timer);

//     DynamicProgramming();
//     gettimeofday(&times[4],NULL);
//     timer = GetRunTimeUs(times[3], times[4]);
//     //printf("Dynamic %d us\n", timer);

//     FreeDrsRules(drs_rules);

//     cal_time = GetRunTimeUs(times[0], times[4]) / 1000000.0;

//     vector<TupleRange> tuple_ranges = GetTupleRanges(0, 0, 32, 32);
//     //exit(1);
//     return tuple_ranges;
// }