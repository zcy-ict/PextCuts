#include "elementary.h"

using namespace std;

void CommandStruct::Init() {
	run_mode = "";
	method_name = "";

	rules_file = "";
	traces_file = "";
	ans_file = "";
	output_file = "";

	rules_shuffle = 0;
    lookup_round = 1;
    force_test = 0;
	print_mode = 0;
}
CommandStruct command_empty;
CommandStruct ParseCommandLine(int argc, char *argv[]) {
	CommandStruct command = command_empty;
	command.Init();
	command.prefix_dims_num = 2;
    int i = 0, j, k;
    char short_opts[] = ""; //"a:"
	struct option long_opts[] = {
       {"run_mode", required_argument, NULL, 0},
       {"method_name", required_argument, NULL, 0},
       {"rules_file", required_argument, NULL, 0},
       {"traces_file", required_argument, NULL, 0},
       {"ans_file", required_argument, NULL, 0},
       {"output_file", required_argument, NULL, 0},
       {"rules_shuffle", required_argument, NULL, 0},
       {"lookup_round", required_argument, NULL, 0},
       {"force_test", required_argument, NULL, 0},
       {"print_mode", required_argument, NULL, 0},
       {"prefix_dims_num", required_argument, NULL, 0},
       {"lookup_thread_time", required_argument, NULL, 0},
       {"update_thread_speed", required_argument, NULL, 0},
       {"reconstruct_thread_time", required_argument, NULL, 0},
       {"next_layer_rules_num", required_argument, NULL, 0},
       {0, 0, 0, 0}
   };
    int opt, option_index;
	bool flag = true;
    while((opt = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1) {
        //if (opt == 'a') {
		//
		//} else 
		if (opt == 0) {
			if (strcmp(long_opts[option_index].name, "run_mode") == 0) {
            	command.run_mode = optarg;
			} else if (strcmp(long_opts[option_index].name, "method_name") == 0) {
            	command.method_name = optarg;
			} else if (strcmp(long_opts[option_index].name, "rules_file") == 0) {
            	command.rules_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "traces_file") == 0) {
            	command.traces_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "ans_file") == 0) {
            	command.ans_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "output_file") == 0) {
            	command.output_file = optarg;
			} else if (strcmp(long_opts[option_index].name, "rules_shuffle") == 0) {
            	command.rules_shuffle = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "lookup_round") == 0) {
            	command.lookup_round = strtoul(optarg, NULL, 0);
            	if (command.lookup_round <= 0) {
            		printf("lookup_round should > 0\n");
            		flag = false;
            	}
			} else if (strcmp(long_opts[option_index].name, "force_test") == 0) {
            	command.force_test = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "print_mode") == 0) {
            	command.print_mode = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "prefix_dims_num") == 0) {
            	command.prefix_dims_num = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "lookup_thread_time") == 0) {
            	command.lookup_thread_time = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "update_thread_speed") == 0) {
            	command.update_thread_speed = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "reconstruct_thread_time") == 0) {
            	command.reconstruct_thread_time = strtoul(optarg, NULL, 0);
			} else if (strcmp(long_opts[option_index].name, "next_layer_rules_num") == 0) {
            	command.next_layer_rules_num = strtoul(optarg, NULL, 0);
			} else {
				flag = false;
				printf("Wrong command %s\n", long_opts[option_index].name);
			}
		} else {
			flag = false;
			printf("Wrong command -%c\n", opt);
		}
	}
	if (command.prefix_dims_num != 2 && command.prefix_dims_num != 5) {
		printf("prefix_dims_num should be 2 or 5\n");
		exit(1);
	}
	if (!flag)
		exit(1);
    return command;
}


void ProgramState::AccessClear() {
    access_tuples.ClearNum();
    access_tables.ClearNum();
    access_nodes.ClearNum();
    access_rules.ClearNum();

}
void ProgramState::AccessCal() {
	max_access_all = max(max_access_all, access_tuples.num + access_tables.num + access_nodes.num + access_rules.num);
	max_access_tuple_node_rule = max(max_access_tuple_node_rule, access_tuples.num + access_nodes.num + access_rules.num);
	max_access_tuple_rule = max(max_access_tuple_rule, access_tuples.num + access_rules.num);
	max_access_node_rule = max(max_access_node_rule, access_nodes.num + access_rules.num);

    access_tuples.Update();
    access_tables.Update();
    access_nodes.Update();
    access_rules.Update();
}

bool SameRule(Rule *rule1, Rule *rule2) {
	for (int i = 0; i < 5; ++i)
		for (int j = 0; j < 2; ++j)
			if (rule1->range[i][j] != rule2->range[i][j])
				return false;
	for (int i = 0; i < 5; ++i)
		if (rule1->prefix_len[i] != rule2->prefix_len[i])
			return false;
	if (rule1->priority != rule2->priority)
		return false;
	return true;
}

bool CmpRulePriority(Rule *rule1, Rule *rule2) {
    return rule1->priority > rule2->priority;
}

bool CmpPtrRulePriority(Rule *rule1, Rule *rule2) {
    return rule1->priority > rule2->priority;
}

bool MatchRuleTrace(Rule *rule, Trace *trace) {
	for (int i = 0; i < 5; i++)
		if (trace->key[i] < rule->range[i][0] || trace->key[i] > rule->range[i][1])
			return false;
	return true;
}

uint64_t GetRunTimeUs(timeval timeval_start, timeval timeval_end) {  // us
	return 1000000 * (timeval_end.tv_sec - timeval_start.tv_sec)+ timeval_end.tv_usec - timeval_start.tv_usec;
}

uint64_t GetAvgTime(vector<uint64_t> &lookup_times) {
	int num = lookup_times.size();
	if (num == 0)
		return 0;
	sort(lookup_times.begin(), lookup_times.end());
	int l = num / 4;
	int r = num - l;
	//printf("GetAvgTime num %d l %d r %d\n", num, l, r);
	uint64_t sum = 0;
	for (int i = l; i < r; ++i)
		sum += lookup_times[i];
	return sum / (r - l);
}

int Popcnt(uint64_t num) {
	return __builtin_popcountll(num);
}

Rule** RuleArr(vector<Rule*> &rules) {
	int rules_num = rules.size();
	Rule **rules_arr = (Rule**)malloc(sizeof(Rule*) * rules_num);
	for (int i = 0; i < rules_num; ++i)
		rules_arr[i] = rules[i];
	return rules_arr;
}

vector<Rule*> RuleVector(Rule** rules, int rules_num) {
	vector<Rule*> rules_vc;
	for (int i = 0; i < rules_num; ++i)
		rules_vc.push_back(rules[i]);
	return rules_vc;
}

vector<RangeRule> Rule2RangeRule(vector<Rule*> &rules) {
	int rules_num = rules.size();
	vector<RangeRule> range_rules;
	RangeRule range_rule;
	for (int i = 0; i < rules_num; ++i) {
		for (int j = 0; j < 5; ++j)
			for (int k = 0; k < 2; ++k)
				range_rule.range[j][k] = rules[i]->range[j][k];
		range_rule.priority = rules[i]->priority;
		range_rule.rule = rules[i];
		range_rules.push_back(range_rule);
	}
	return range_rules;
}


Rule** RangeRuleArr(vector<RangeRule> &rules) {
	int rules_num = rules.size();
	Rule **rules_arr = (Rule**)malloc(sizeof(Rule*) * rules_num);
	for (int i = 0; i < rules_num; ++i)
		rules_arr[i] = rules[i].rule;
	return rules_arr;
}

bool CmpRangeRulePriority(RangeRule rule1, RangeRule rule2) {
    return rule1.priority > rule2.priority;
}
