#ifndef  DYNAMICTUPLE_H
#define  DYNAMICTUPLE_H

#include "../../elementary.h"
#include "tuple.h"
#include "dynamictuple-ranges.h"

using namespace std;

class DynamicTuple : public Classifier {
public:
    
    int Create(vector<Rule*> &rules, bool insert);

    int InsertRule(Rule *rule);
    int DeleteRule(Rule *rule);
    int Lookup(Trace *trace, int priority);
    int LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state);

    int Reconstruct();
    uint64_t MemorySize();
    int CalculateState(ProgramState *program_state);
    int GetRules(vector<Rule*> &rules);
    int Free(bool free_self);
    int Test(void *ptr);

    string method_name;
    vector<TupleRange> tuple_ranges;
	char prefix_down[33][33][2];  // 第一维是源IP，第二维是目的IP，第三维是修改后的源目的前缀长度
	int threshold;  // port_hashtable的阈值

	uint32_t prefix_mask[33];
    bool use_port_hash_table;

    Tuple **tuples_arr;
	map<uint32_t, Tuple*> tuples_map;
    int tuples_num;
    int max_tuples_num;
    int rules_num;
    int max_priority;

    double cal_time;


    void InsertTuple(Tuple *tuple);
    void SortTuples();
};

#endif