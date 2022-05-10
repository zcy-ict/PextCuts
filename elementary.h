#ifndef  ELEMENTARY_H
#define  ELEMENTARY_H

#include <getopt.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <map>

using namespace std;

struct CommandStruct {
    string run_mode;
    string method_name;

	string rules_file;
	string traces_file;
	string ans_file;
	string output_file;

	int rules_shuffle;
    int lookup_round;
    int force_test;  // 0表示不验证，1表示使用全部规则，2表示使用%4！=0的规则
	int print_mode;

	int prefix_dims_num;

	int lookup_thread_time;
	int update_thread_speed;
	int reconstruct_thread_time;

	int next_layer_rules_num;

	void Init();
};

CommandStruct ParseCommandLine(int argc, char *argv[]);

struct AccessNum {
	int sum;
	int minn;
	int maxn;
	int num;

	void Update() {
		sum += num;
		minn = min(minn, num);
		maxn = max(maxn, num);
	}

	void ClearNum() {
		num = 0;
	}

	void AddNum() {
		++num;
	}
};

struct DecisionTreeInfo {
	int node_num;
	int children_num;
	int diff_node_num;
	int diff_children_num;

	void Init() {
		node_num = 0;
		children_num = 0;
		diff_node_num = 0;
		diff_children_num = 0;
	}

	void Add(DecisionTreeInfo info) {
		node_num += info.node_num;
		children_num += info.children_num;
		diff_node_num += info.diff_node_num;
		diff_children_num += info.diff_children_num;
	}
};

struct DecisionTreeInfoLayer {
	DecisionTreeInfo info[7]; // 0 cut ip, 1 cut port, 2 cut protocol, 3 split ip, 4 split port, 5 split protocol, 6 leaf 
};

struct ProgramState {
	int rules_num;
	int traces_num;

	double data_memory_size;  // MB
	double index_memory_size;  // MB

	double build_time;  // S
	double lookup_speed;  // Mlps
	double insert_speed;  // Mups
	double delete_speed;  // Mups
	double update_speed;  // Mups


    int tuples_num;
    int tuples_sum;
    int hash_node_num;
    int bucket_sum;
    int bucket_use;
	int next_layer_num;

	AccessNum access_tuples;
	AccessNum access_tables;
	AccessNum access_nodes;
	AccessNum access_rules;

	int max_access_all;
	int max_access_tuple_node_rule;
	int max_access_tuple_rule;
	int max_access_node_rule;
	int max_access_num;

	void AccessClear();
	void AccessCal();

	int low_priority_matching_access;
	int high_priority_matching_access;
	
	int low_priority_collision_access; 
	int high_priority_collision_access;

	int low_priority_rule_access; 
	int high_priority_rule_access;


	int check_hash_before_rule_sum;
	int check_hash_after_rule_sum;

	int check_hash_large;
	int check_hash_small;


	int match_hash_sum;  // 匹配进hash_node的次数
	int match_hash_num;

	int match_hash_after_rule_sum;  // 在匹配到规则后匹配进hash_node的次数

	int match_hash_before_rule_sum; 
	int match_hash_large;
	int match_hash_small;
	int before_large;
	int before_small;

	int match_real_rules_sum;

	int check_rules_large;
	int check_rules_small;
	int check_rule_break;



	int match_rules_sum;

	int check_hash_node_sum;

	int tc_cls_num;

	double cal_time;  // S

	DecisionTreeInfoLayer layers[30];
	vector<int> tree_height_num;
	int tree_height_sum;
	vector<int> tree_real_height_num;
	int tree_real_height_sum;
	int tree_rules_num;
	int tree_real_rules_num;
	int tree_child_num;
	int tree_real_child_num;
};

struct Rule {
	uint32_t range[5][2];
	char prefix_len[5];
	int priority;

	bool operator<(const Rule& rule)const{
		for (int i = 0; i < 5; ++i)
			for (int j = 0; j < 2; ++j)
				if (range[i][j] != rule.range[i][j])
					return range[i][j] < rule.range[i][j];
		for (int i = 0; i < 5; ++i)
			if (prefix_len[i] != rule.prefix_len[i])
				return prefix_len[i] < rule.prefix_len[i];
		return priority < rule.priority;
    }

    bool operator==(const Rule& rule)const {
		for (int i = 0; i < 5; ++i)
			for (int j = 0; j < 2; ++j)
				if (range[i][j] != rule.range[i][j])
					return false;
		for (int i = 0; i < 5; ++i)
			if (prefix_len[i] != rule.prefix_len[i])
				return false;
		return priority == rule.priority;
    }
    void Print() {
    	printf("priority %d\n", priority);
    	for (int i = 0; i < 5; ++i)
    		printf("%d : %08x %08x / %d\n", i, range[i][0], range[i][1], prefix_len[i]);
    }

};

struct RangeRule {
	uint32_t range[5][2];
	int priority;
	Rule *rule;

	bool operator<(const RangeRule& rule)const{
		for (int i = 0; i < 5; ++i)
			for (int j = 0; j < 2; ++j)
				if (range[i][j] != rule.range[i][j])
					return range[i][j] < rule.range[i][j];
		return 0;
    }
};

struct Trace{
	union {
		uint32_t key[5];
		uint64_t dst_src_ip;
	};
	uint64_t key_u64[2];
};

class Classifier {
public:
    virtual int Create(vector<Rule*> &rules, bool insert) = 0;

    virtual int InsertRule(Rule *rule) = 0;
    virtual int DeleteRule(Rule *rule) = 0;
    virtual int Lookup(Trace *trace, int priority) = 0;
    virtual int LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state) = 0;

    virtual int Reconstruct() = 0;
    virtual uint64_t MemorySize() = 0;
    virtual int CalculateState(ProgramState *program_state) = 0;
    virtual int GetRules(vector<Rule*> &rules) = 0;
    virtual int Free(bool free_self) = 0;
    virtual int Test(void *ptr) = 0;

    int test_num;
    pthread_mutex_t lookup_mutex;
    pthread_mutex_t update_mutex;
};

bool SameRule(Rule *rule1, Rule *rule2);
bool CmpRulePriority(Rule *rule1, Rule *rule2);
bool CmpPtrRulePriority(Rule *rule1, Rule *rule2);
bool MatchRuleTrace(Rule *rule, Trace *trace);
uint64_t GetRunTimeUs(timeval timeval_start, timeval timeval_end);
uint64_t GetAvgTime(vector<uint64_t> &lookup_times);
int Popcnt(uint64_t num);
Rule** RuleArr(vector<Rule*> &rules);
vector<Rule*> RuleVector(Rule** rules, int rules_num);
vector<RangeRule> Rule2RangeRule(vector<Rule*> &rules);
Rule** RangeRuleArr(vector<RangeRule> &rules);
bool CmpRangeRulePriority(RangeRule rule1, RangeRule rule2);

#endif