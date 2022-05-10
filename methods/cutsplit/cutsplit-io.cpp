#include "cutsplit-io.h"

using namespace std;

pc_rule* GenerateCutSplitRules(vector<Rule*> &rules) {
	int rules_num = rules.size();
	pc_rule* pc_rules = (pc_rule*)malloc(sizeof(pc_rule) * rules_num);

	for (int i = 0; i < rules_num; ++i) {
		for (int j = 0; j < 5; ++j) {
			pc_rules[i].field[j].low = rules[i]->range[j][0];
			pc_rules[i].field[j].high = rules[i]->range[j][1];
		}
		pc_rules[i].id = i;
	}
	return pc_rules;
}

void FreeCutSplitRules(pc_rule* rules) {
	free(rules);
}

uint32_t** GenerateCutSplitPackets(vector<Trace*> &traces) {
	int traces_num = traces.size();
	uint32_t** packets = (uint32_t**)malloc(sizeof(uint32_t*) * traces_num);
	for (int i = 0; i < traces_num; ++i) {
		packets[i] = (uint32_t*)malloc(sizeof(uint32_t) * 5);
		for (int j = 0; j < 5; ++j)
			packets[i][j] = traces[i]->key[j];
	}
	return packets;
}

void FreeCutSplitPackets(uint32_t** packets, int packets_num) {
	for (int i = 0; i < packets_num; ++i)
		free(packets[i]);
	free(packets);
}