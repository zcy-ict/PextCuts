#include "pextcuts-ranges.h"

using namespace std;


int pext_check_tuple_cost = 5;
int pext_check_rule_cost = 3;
uint32_t INF = 1e9;

int Log2(int num) {
    int ans = 0;
    while (num) {
        num /= 2;
        ++ans;
    }
    return ans;
}

void PcDtInfo::PextCalculateXY(int x1, int y1) {
    int rules_num = rules.size();
    if (GetIpNumSum(x1, y1, x1, 32) == 0 || GetIpNumSum(x1, y1, 32, y1) == 0) {
        for (int x2 = x1; x2 <= 32; ++x2)
            for (int y2 = y1; y2 <= 32; ++y2) {
                if (GetIpNumSum(x1, y1, x2, y2) == 0) {
                    tuple_cost[x1][y1][x2][y2].cost = 0;
                    tuple_info[x1][y1][x2][y2].Init();
                }
                else
                    tuple_cost[x1][y1][x2][y2].cost = INF;
                tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SELF;
            }
        return;
    }
    
    // reduce rule ip
    for (int i = 0; i < rules_num; ++i)
        rules[i]->reduced_src_dst_ip = rules[i]->src_dst_ip & ip_mask_pair[x1][y1];
    sort(rules.begin(), rules.end(), CmpDtRulesIpPriority);
    
    // tuple
    check_tuple_num[x1][y1] = max_priority[x1][y1];
    for (int x = x1 + 1; x <= 32; ++x)
        check_tuple_num[x][y1] = max(check_tuple_num[x - 1][y1], max_priority[x][y1]);
    for (int y = y1 + 1; y <= 32; ++y)
        check_tuple_num[x1][y] = max(check_tuple_num[x1][y - 1], max_priority[x1][y]);
    for (int x = x1 + 1; x <= 32; ++x)
        for (int y = y1 + 1; y <= 32; ++y)
            check_tuple_num[x][y] = max(max(check_tuple_num[x - 1][y], check_tuple_num[x][y - 1]), max_priority[x][y]);

    for (int x2 = x1; x2 <= 32; ++x2)
        for (int y2 = y1; y2 <= 32; ++y2)
            tuple_info[x1][y1][x2][y2].check_tuple_num = check_tuple_num[x2][y2];

    // group && rule
    rules[rules_num - 1]->check_num = 1;
    for (int i = rules_num - 2; i >= 0; --i)
        if (rules[i]->reduced_src_dst_ip == rules[i + 1]->reduced_src_dst_ip) {
            rules[i + 1]->first = false;
            rules[i]->check_num = rules[i + 1]->check_num + 1;
        } else {
            rules[i + 1]->first = true;
            rules[i]->check_num = 1;
        }
    rules[0]->first = true;

    int group_num = 0;
    memset(group_vis, 0, sizeof(group_vis));
    memset(contain_group_num, 0, sizeof(contain_group_num));
    memset(match_group_num, 0, sizeof(match_group_num));
    memset(collide_group_num, 0, sizeof(collide_group_num));
    memset(check_rule_num, 0, sizeof(check_rule_num));

    for (int i = 0; i < rules_num; ++i) {
        int px = rules[i]->src_prefix_len;
        int py = rules[i]->dst_prefix_len;
        check_rule_num[px][py] += Log2(rules[i]->check_num);
    }
    
    // check_rule_num
    for (int x = x1 + 1; x <= 32; ++x)
        check_rule_num[x][y1] += check_rule_num[x - 1][y1];
    for (int y = y1 + 1; y <= 32; ++y)
        check_rule_num[x1][y] += check_rule_num[x1][y - 1];
    for (int x = x1 + 1; x <= 32; ++x)
        for (int y = y1 + 1; y <= 32; ++y)
            check_rule_num[x][y] += check_rule_num[x][y - 1] + check_rule_num[x - 1][y] - check_rule_num[x - 1][y - 1];
    
    for (int x2 = x1; x2 <= 32; ++x2)
        for (int y2 = y1; y2 <= 32; ++y2) {
            tuple_cost[x1][y1][x2][y2].cost = check_tuple_num[x2][y2] * pext_check_tuple_cost +
                                              check_rule_num[x2][y2] * pext_check_rule_cost;
            tuple_cost[x1][y1][x2][y2].split_type = SPLIT_SELF;
        }
}

void PcDtInfo::PextCalculate() {
    for (int x1 = 32; x1 >= 0; --x1)
        for (int y1 = 32; y1 >= 0; --y1)
            PextCalculateXY(x1, y1);
}

vector<TupleRange> tuple_range_null;
vector<TupleRange> PextTupleRanges(vector<Rule*> &rules, double &cal_time) {
    if (rules.size() == 0)
        return TupleRangeStep(8);
    
    timeval timeval_start, timeval_end;
	gettimeofday(&timeval_start,NULL);

    PcDtInfo *info = new PcDtInfo();
    info->GetDtRules(rules);
    info->Init(tuple_range_null);
    info->PextCalculate();
    info->DynamicProgramming();
    vector<TupleRange> tuple_ranges = info->GetTupleRanges(0, 0, 32, 32);
    info->Free();

    gettimeofday(&timeval_end,NULL);
    cal_time = GetRunTimeUs(timeval_start, timeval_end) / 1000000.0;

    return tuple_ranges;

}