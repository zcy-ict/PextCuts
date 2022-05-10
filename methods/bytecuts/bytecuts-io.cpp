#include "bytecuts-io.h"

using namespace std;

vector<BCRule> GenerateByteCutsRules(vector<Rule*> &rules) {
	vector<BCRule> bc_rules;
	int rules_num = rules.size();
	for (int i = 0; i < rules_num; ++i) {
		BCRule rule;
		rule.priority = rules[i]->priority;
		rule.markedDelete = 0;
		for (int j = 0; j < 5; ++j) {
			rule.range[j].low = rules[i]->range[j][0];
			rule.range[j].high = rules[i]->range[j][1];
			if (j == 0 || j == 1)
				rule.prefix_length[j] = rules[i]->prefix_len[j];
			else if (j == 2 || j == 3)
				rule.prefix_length[j] = rule.range[j].low == rule.range[j].high ? 32 : 16;
			else
				rule.prefix_length[j] = rule.range[j].low == rule.range[j].high ? 32 : 24;
		}
		bc_rules.push_back(rule);
	}
	return bc_rules;
}

vector<BCPacket> GenerateByteCutsPackets(vector<Trace*> &traces) {
	vector<BCPacket> packets;
	int traces_num = traces.size();
	for (int i = 0; i < traces_num; ++i) {
		BCPacket packet = new Point[NumDims];
		for (int j = 0; j < 5; ++j)
			packet[j] = traces[i]->key[j];
		packets.push_back(packet);
	}
	return packets;
}