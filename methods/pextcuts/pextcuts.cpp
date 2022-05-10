#include "pextcuts.h"

#include <immintrin.h>

using namespace std;

int pext_rules_sum = 0;
int log_2[1025];
uint32_t bit_head[35];
uint32_t bit_tail[35];
uint32_t mask_head[35];
uint32_t mask_tail[35];
uint32_t bit16_head[20];

int *bits_child_num = NULL;
int bits_child_size = 1024;

void ExtendBitsChildNum() {
	bits_child_size *= 2;
	free(bits_child_num);
	bits_child_num = (int*)malloc(sizeof(int) * bits_child_size);
}

int GetLog(int num) {
	int ans = 0;
	while (true) {
		if (num > 1024) {
			ans += 10;
			num >>= 10;
		} else {
			ans += log_2[num];
			break;
		}
	}
	return ans;
}

bool CmpRangeRules(RangeRules* range_rules1, RangeRules* range_rules2) {
	return range_rules1->max_priority > range_rules2->max_priority;
}

bool GetIpPextBits(int last_bit, int bit_num, uint32_t pext_bits, uint32_t &select_bits) {
	select_bits = 0;
	if (last_bit < 0 || bit_num < 0)
		return false;
	if (last_bit == 0) {
		if (bit_num > 0)
			return false;
		else
			return true;
	}
	if (last_bit > 32)
		return false;
	for (int i = last_bit; i >= 1 && bit_num > 0; --i) {
		if (pext_bits & bit_head[i])
			continue;
		--bit_num;
		select_bits |= bit_head[i];
	}
	if (bit_num > 0)
		return false;
	return true;
}

int FirstBit(int num) {
	for (int i = 1; i <= 32; ++i)
		if (num & bit_head[i])
			return i;
	return 0;
}

int LastBit(int num) {
	for (int i = 32; i >= 1; --i)
		if (num & bit_head[i])
			return i;
	return 0;
}

PextRule::PextRule(Rule *_rule) {
    rule = _rule;
    weight = 1;
    if (rule != NULL) {
    	port_prefix = new PextRulePortPrefix();
	    port_prefix->ports[0] = GetPortMask(rule->range[2][0], rule->range[2][1]);
	    port_prefix->ports[1] = GetPortMask(rule->range[3][0], rule->range[3][1]);
    } else {
    	port_prefix = NULL;
    }
}

// src and dst
double CutIpCostBits(vector<PextRule> &rules, uint32_t* bits, int layer) {
	int max_num = 1;
	for (int i = 0; i < 2; ++i)
		max_num <<= Popcnt(bits[i]);
	while (max_num + 1> bits_child_size)
		ExtendBitsChildNum();
	for (int i = 0; i < max_num; ++i)
		bits_child_num[i] = 0;
	double cost = 0;
	int src_bits_num = Popcnt(bits[0]);

	uint32_t rules_num = rules.size();
	uint32_t rules_sum = 0;
	uint32_t range[2][2];
	uint32_t range_num[2];
	uint32_t range_sum;
	double max_rules_rate = 3;
	// double max_rules_rate = 2 + layer * 0.2;
	for (int i = 0; i < rules_num; ++i) {
		for (int j = 0; j < 2; ++j) {
			for (int k = 0; k < 2; ++k)
				range[j][k] = _pext_u32(rules[i].rule->range[j][k], bits[j]);
			range_num[j] = range[j][1] - range[j][0] + 1;
		}
		range_sum = range_num[0] * range_num[1];
		rules_sum += range_sum;
		if (rules_sum > rules_num * max_rules_rate) {
			cost = 1e9;
			break;
		}
		for (int j = range[0][0]; j <= range[0][1]; ++j)
			for (int k = range[1][0]; k <= range[1][1]; ++k) {
				int index = k << src_bits_num | j;
				++bits_child_num[index];
				cost += bits_child_num[index] * rules[i].weight / range_sum;
			}
	}
	int max_child_num = 0;
	for (int i = 0; i < max_num; ++i)
		max_child_num = max(max_child_num, bits_child_num[i]);
	if (max_child_num >= rules_num - rules_num / 20)
		cost = 1e9;
	// printf("CutIpCostBits %08x %08x ", bits[0], bits[1]);
	// printf("cost %.2f ", cost);
	// printf("rules_sum %d\n", rules_sum);
	return cost;
}

double CutIpCost(vector<PextRule> &rules, uint32_t* bits, PextBits pext_bits, int layer) {
	int rules_num = rules.size();
	char min_prefix_len[2] = {32, 32};
	for (int i = 0; i < rules_num; ++i)
		for (int j = 0; j < 2; ++j)
			min_prefix_len[j] = min(min_prefix_len[j], rules[i].rule->prefix_len[j]);
	int bits_num = GetLog(rules_num);

	double cost = 1e9;
	bits[0] = 0;
	bits[1] = 0;

	int init_bits_num = 0;
	for (int i = 0; i < 2; ++i)
		init_bits_num += min_prefix_len[i] - Popcnt(pext_bits.ip_bits[i] >> (32 - min_prefix_len[i]));
	init_bits_num = min(init_bits_num, bits_num);

	uint32_t test_bits[2];
	bool flag[2];
	if (init_bits_num > 0) {
		for (int i = 0; i <= init_bits_num; ++i) {
			flag[0] = GetIpPextBits(min_prefix_len[0], i, pext_bits.ip_bits[0], test_bits[0]);
			flag[1] = GetIpPextBits(min_prefix_len[1], init_bits_num - i, pext_bits.ip_bits[1], test_bits[1]);
			if (!flag[0] || !flag[1])
				continue;
			double test_cost = CutIpCostBits(rules, test_bits, layer);
			if (test_cost < cost) {
				cost = test_cost;
				bits[0] = test_bits[0];
				bits[1] = test_bits[1];
			}
		}
	}

	int last_bit[2];
	uint32_t pre_bits[2];
	int test_last_bit[2];
	int test_bits_num[2];
	for (int i = 0; i < 2; ++i)
		last_bit[i] = max(LastBit(bits[i]), (int)min_prefix_len[i]);

	for (int i = 1; i <= 32; ++i) {
		for (int j = 0; j < 2; ++j) {
			if (i <= last_bit[j] || (pext_bits.ip_bits[j] & bit_head[i]))
				continue;
			pre_bits[0] = bits[0];
			pre_bits[1] = bits[1];
			for (int k = 0; k < 5; ++k) {
				test_bits[0] = pre_bits[0];
				test_bits[1] = pre_bits[1];
				if (k == 0 || k == 1) {
					int index = FirstBit(pre_bits[k & 1]);
					if (index == 0)
						continue;
					test_bits[k & 1] ^= bit_head[index];
				} else if (k == 2 || k == 3) {
					int index = LastBit(pre_bits[k & 1]);
					if (index == 0)
						continue;
					test_bits[k & 1] ^= bit_head[index];
				}
				test_bits[j] |= bit_head[i];
				if (Popcnt(test_bits[0]) + Popcnt(test_bits[1]) > bits_num)
					continue;
				double test_cost = CutIpCostBits(rules, test_bits, layer);
				
				if (test_cost < cost) {
					cost = test_cost;
					bits[0] = test_bits[0];
					bits[1] = test_bits[1];
				}
			}
		}
	}
	// printf("select %08x %08x cost %.2f\n", bits[0], bits[1], cost);

	return cost;
}

uint64_t HashRulePriority(vector<PextRule> &rules) {
	uint64_t hash = 0;
	for (int i = 0; i < rules.size(); ++i) {
		hash = hash * (1e9 + 7) + rules[i].rule->priority;
	}
	return hash;
}

bool SameRules(vector<PextRule> &rules1, vector<PextRule> &rules2) {
	if (rules1.size() != rules2.size())
		return false;
	for (int i = 0; i < rules1.size(); ++i)
		if (rules1[i].rule->priority != rules2[i].rule->priority)
			return false;
	return true;
}

void PextNode::CreateIpCut(vector<PextRule> &rules, uint32_t *bits, PextBits pext_bits) {
	type = PextCutIp;
	dim = 0;
	max_priority = rules[0].rule->priority;
	cut_ip_bits = (uint64_t)bits[1] << 32 | bits[0];
	int rules_num = rules.size();

	int max_num = 1;
	for (int i = 0; i < 2; ++i)
		max_num <<= Popcnt(bits[i]);
	vector<PextRule> child_rules[max_num];
	for (int i = 0; i < max_num; ++i)
		child_rules[i].clear();

	uint32_t range[2][2];
	uint32_t range_num[2];
	uint32_t range_sum;
	int src_bits_num = Popcnt(bits[0]);
	PextRule rule(NULL);
	for (int i = 0; i < rules_num; ++i) {
		for (int j = 0; j < 2; ++j) {
			for (int k = 0; k < 2; ++k)
				range[j][k] = _pext_u32(rules[i].rule->range[j][k], bits[j]);
			range_num[j] = range[j][1] - range[j][0] + 1;
		}
		range_sum = range_num[0] * range_num[1];
		rule = rules[i];
		rule.weight /= range_sum;

		for (int j = range[0][0]; j <= range[0][1]; ++j)
			for (int k = range[1][0]; k <= range[1][1]; ++k) {
				int index = k << src_bits_num | j;
				child_rules[index].push_back(rule);
			}
	}
	pext_bits.ip_bits[0] |= bits[0];
	pext_bits.ip_bits[1] |= bits[1];

	children = (PextNode*)malloc(sizeof(PextNode) * max_num);
	map<uint64_t, vector<int>> children_map;
	for (int i = 0; i < max_num; ++i) {
		uint64_t hash = HashRulePriority(child_rules[i]);
		if (children_map.find(hash) != children_map.end()) {
			vector<int> vc = children_map[hash];
			bool duplicate_flag = false;
			for (int j = 0; j < vc.size(); ++j) {
				int index = vc[j];
				if (SameRules(child_rules[index], child_rules[i])) {
					children[i] = children[index];
					children[i].duplicate = true;
					duplicate_flag = true;
					break;
				}
			}
			if (duplicate_flag)
				continue;
		}
		children[i].Create(child_rules[i], pext_bits, layer + 1);
		children_map[hash].push_back(i);
	}
}

vector<PrefixRange> GetPortRrangeBits(PextRule rule, int dim, uint32_t bits) {
	vector<PrefixRange> ranges;
	for (int i = 0; i < rule.port_prefix->ports[dim - 2].size(); ++i) {
		PrefixRange range = rule.port_prefix->ports[dim - 2][i];
		range.low = _pext_u32(range.low, bits);
		range.high = _pext_u32(range.high, bits);
		ranges.push_back(range);
	}
	for (int i = ranges.size() - 1; i > 0; --i) {
		for (int j = 0; j < i; ++j)
			if ((ranges[i].low <= ranges[j].low + 1 && ranges[j].low <= ranges[i].high + 1) || 
				(ranges[j].low <= ranges[i].low + 1 && ranges[i].low <= ranges[j].high + 1)){
				ranges[j].low = min(ranges[j].low, ranges[i].low);
				ranges[j].high = max(ranges[j].high, ranges[i].high);
				ranges.erase(ranges.begin() + i);
				break;
			}
	}
	return ranges;
}

double CutPortCostBits(vector<PextRule> &rules, int dim, uint32_t bits, int layer) {
	int max_num = 1 << Popcnt(bits);
	while (max_num + 1> bits_child_size)
		ExtendBitsChildNum();
	for (int i = 0; i < max_num; ++i)
		bits_child_num[i] = 0;
	double cost = 0;

	int rules_num = rules.size();
	int rules_sum = 0;
	uint32_t start;
	uint32_t end;
	uint32_t index;
	uint32_t cut_num;
	double max_rules_rate = 2.5;
	// double max_rules_rate = 2 + layer * 0.2;
	for (int i = 0; i < rules_num; ++i) {
		if (rules_sum > rules_num * max_rules_rate) {
			cost = 1e9;
			break;
		}
		start = rules[i].rule->range[dim][0];
		end = rules[i].rule->range[dim][1];
		if (start == end) {
			index = _pext_u32(start, bits);
			++bits_child_num[index];
			cost += bits_child_num[index] * rules[i].weight;
			rules_sum += 1; 
		} else if (end - start + 1 == 65536) {
			cut_num = max_num;
			for (int j = 0; j < max_num; ++j) {
				++bits_child_num[j];
				cost += bits_child_num[j] * rules[i].weight / cut_num;
			}
			rules_sum += max_num;
		} else {
			cut_num = 0;
			vector<PrefixRange> ranges = GetPortRrangeBits(rules[i], dim, bits);
			// if (ranges.size() > 2) printf("%ld\n", ranges.size());
			for (int k = 0; k < ranges.size(); ++k)
				cut_num += ranges[k].high - ranges[k].low + 1;
			for (int k = 0; k < ranges.size(); ++k) {
				for (int j = ranges[k].low; j <= ranges[k].high; ++j) {
					++bits_child_num[j];
					cost += bits_child_num[j] * rules[i].weight / cut_num;
				}
			}
			rules_sum += cut_num;
		}
		// printf("rules %d cost %.2f\n", i, cost);
	}
	if (rules_sum > rules_num * max_rules_rate)
		cost = 1e9;
	int max_child_num = 0;
	for (int i = 0; i < max_num; ++i)
		max_child_num = max(max_child_num, bits_child_num[i]);
	if (max_child_num >= rules_num - rules_num / 20)
		cost = 1e9;
	return cost;

}

// dim 2 or 3
double CutPortCost(vector<PextRule> &rules, int dim, uint32_t &bits, PextBits pext_bits, int layer) {
	// printf("CutPortCost dim %d\n", dim);
	int rules_num = rules.size();
	int bits_num = GetLog(rules_num);

	double cost = 1e9;
	bits = 0;
	int last_bit = 0;
	int pre_bits;
	while (last_bit < 16) {
		pre_bits = bits;
		while (last_bit <= 15 && (pext_bits.port_bits[dim - 2] & bit16_head[last_bit + 1]))
			++last_bit;
		if (last_bit == 16)
			break;
		for (int i = 0; i <= last_bit; ++i) {
			uint32_t test_bits = pre_bits;
			test_bits |= bit16_head[last_bit + 1];
			if (i > 0 && !(test_bits & bit16_head[i]))
				continue;
			test_bits ^= bit16_head[i];

			if (Popcnt(test_bits) > bits_num)
				continue;

			double test_cost = CutPortCostBits(rules, dim, test_bits, layer);
			if (test_cost < cost) {
				cost = test_cost;
				bits = test_bits;
			}
		}
		++last_bit;
	}
	// printf("CutPortCost bits %08x cost %.2f\n", bits, cost);
	return cost;
}

// dim = 2 or 3
void PextNode::CreatePortCut(vector<PextRule> &rules, uint32_t bits, int _dim, PextBits pext_bits) {
	type = PextCutPort;
	dim = _dim;
	max_priority = rules[0].rule->priority;
	cut_port_bits = bits;
	int rules_num = rules.size();

	int max_num = 1 << Popcnt(bits);
	vector<PextRule> child_rules[max_num];
	for (int i = 0; i < max_num; ++i)
		child_rules[i].clear();

	uint32_t start;
	uint32_t end;
	uint32_t index;
	uint32_t cut_num;
	PextRule rule(NULL);
	for (int i = 0; i < rules_num; ++i) {
		start = rules[i].rule->range[dim][0];
		end = rules[i].rule->range[dim][1];
		if (start == end) {
			index = _pext_u32(start, bits);
			child_rules[index].push_back(rules[i]);
		} else if (end - start + 1 == 65536) {
			cut_num = max_num;
			rule = rules[i];
			rule.weight /= cut_num;
			for (int j = 0; j < max_num; ++j)
				child_rules[j].push_back(rule);
		} else {
			cut_num = 0;
			vector<PrefixRange> ranges = GetPortRrangeBits(rules[i], dim, bits);
			for (int k = 0; k < ranges.size(); ++k)
				cut_num += ranges[k].high - ranges[k].low + 1;
			rule = rules[i];
			rule.weight /= cut_num;
			for (int k = 0; k < ranges.size(); ++k)
				for (int j = ranges[k].low; j <= ranges[k].high; ++j)
					child_rules[j].push_back(rule);
		}
	}
	pext_bits.port_bits[dim - 2] |= bits;

	children = (PextNode*)malloc(sizeof(PextNode) * max_num);
	map<uint64_t, vector<int>> children_map;
	for (int i = 0; i < max_num; ++i) {
		uint64_t hash = HashRulePriority(child_rules[i]);
		if (children_map.find(hash) != children_map.end()) {
			vector<int> vc = children_map[hash];
			bool duplicate_flag = false;
			for (int j = 0; j < vc.size(); ++j) {
				int index = vc[j];
				if (SameRules(child_rules[index], child_rules[i])) {
					children[i] = children[index];
					children[i].duplicate = true;
					duplicate_flag = true;
					break;
				}
			}
			if (duplicate_flag)
				continue;
		}
		children[i].Create(child_rules[i], pext_bits, layer + 1);
		children_map[hash].push_back(i);
	}
}

void PextNode::CreateLeaf(vector<PextRule> &rules) {
	type = PextLeaf;
	dim = 0;
	max_priority = rules[0].rule->priority;
	rules_arr_num = rules.size();
	rules_arr = (PextRuleNode*)malloc(sizeof(PextRuleNode) * rules_arr_num);
	for (int i = 0; i < rules_arr_num; ++i)
		rules_arr[i].Init(rules[i].rule);
}

int node_id;

void PextNode::Create(vector<PextRule> &rules, PextBits pext_bits, char _layer) {
	layer = _layer;
	duplicate = false;
	int rules_num = rules.size();
	if (rules_num == 0) {
		type = PextLeaf;
		rules_arr_num = 0;
		duplicate = true;
		return;
	}
	// printf("PextNode Create %d layer %d\n", rules_num, layer);
	int max_bits = GetLog(rules_num);
	double cost = 0;
	int select_type = PextLeaf;
	int select_dim = 0;
	uint32_t bits[2] = {0, 0};

	for (int i = 0; i < rules_num; ++i)
		cost += i * rules[i].weight;
		// cost += (i + 1) * rules[i].weight;
		// cost += (i + 0.5) * rules[i].weight;

	++node_id;
	if (rules_num <= 3) {
		CreateLeaf(rules);
		// for (int i = 0; i < layer; ++i) printf("    ");
		// printf("layer %d rules %d : select_type %d cost %.2f id %d\n", layer, rules_num, select_type, cost, node_id);
		return;
	}

	uint32_t test_bits[2];
	double test_cost = CutIpCost(rules, test_bits, pext_bits, layer);
	if (test_cost + 1e-9 < cost) {
		select_type = PextCutIp;
		select_dim = 0;
		cost = test_cost;
		bits[0] = test_bits[0];
		bits[1] = test_bits[1];
	}

	for (int i = 2; i <= 3; ++i) {
		test_cost = CutPortCost(rules, i, test_bits[0], pext_bits, layer);
		if (test_cost + 1e-9 < cost) {
			select_type = PextCutPort;
			select_dim = i;
			cost = test_cost;
			bits[0] = test_bits[0];
			bits[1] = 0;
		}
	}
	// for (int i = 0; i < layer; ++i) printf("    ");
	// printf("layer %d rules %d : select_type %d select_dim %d bits %08x %08x cost %.2f id %d\n", 
	// 		layer, rules_num, select_type, select_dim, bits[0], bits[1], cost, node_id);

	if (select_type == PextCutIp) {
		CreateIpCut(rules, bits, pext_bits);
	} else if (select_type == PextCutPort) {
		CreatePortCut(rules, bits[0], select_dim, pext_bits);
	} else if (select_type == PextLeaf) {
		CreateLeaf(rules);
	}
}

void PextCuts::Init() {
	trees_num = 0;
	trees = NULL;
	memset(log_2, 0, sizeof(log_2));
	memset(bit_head, 0, sizeof(bit_head));
	memset(bit_tail, 0, sizeof(bit_tail));
	memset(mask_head, 0, sizeof(mask_head));
	memset(mask_tail, 0, sizeof(mask_tail));
	memset(bit16_head, 0, sizeof(bit16_head));
	for (int i = 1; i < 1025; i *= 2)
		log_2[i] = 1;
	for (int i = 1; i < 1025; ++i)
		log_2[i] += log_2[i - 1];

	for (int i = 1; i <= 32; ++i) {
		bit_head[i] = 1U << (32 - i);
		mask_head[i] = mask_head[i - 1] + bit_head[i];
		bit_tail[i] = 1U << (i - 1);
		mask_tail[i] = mask_tail[i - 1] + bit_tail[i];
	}

	for (int i = 1; i <= 16; ++i) 
		bit16_head[i] = 1U << (16 - i);
	bits_child_num = (int*)malloc(sizeof(int) * bits_child_size);
}

int PextCuts::Create(vector<Rule*> &_rules, bool insert) {
	Init();
	if (_rules.size() == 0)
		return 0;
	pext_rules_sum = _rules.size();
	vector<Rule*> rules = UniqueRules(_rules);

	int rules_num = rules.size();

	vector<RangeRules*> ranges_rules;

	vector<TupleRange> tuple_ranges = PextTupleRanges(rules, cal_time);

	int tuple_ranges_num = tuple_ranges.size();
	for (int i = 0; i < tuple_ranges_num; ++i) {
		TupleRange tuple_range = tuple_ranges[i];
		vector<Rule*> rules2;
		for (int j = 0; j < rules_num; ++j)
			if (tuple_range.x1 <= rules[j]->prefix_len[0] && rules[j]->prefix_len[0] <= tuple_range.x2 && 
				tuple_range.y1 <= rules[j]->prefix_len[1] && rules[j]->prefix_len[1] <= tuple_range.y2)
					rules2.push_back(rules[j]);
		if (rules2.size() > 0) {
			RangeRules *range_rules = new RangeRules();
			sort(rules2.begin(), rules2.end(), CmpRulePriority);
			range_rules->rules = rules2;
			range_rules->max_priority = rules2[0]->priority;
			range_rules->tuple_range = tuple_range;
			ranges_rules.push_back(range_rules);
		}
	}

	sort(ranges_rules.begin(), ranges_rules.end(), CmpRangeRules);
	trees_num = ranges_rules.size();
	trees = (PextNode*)malloc(sizeof(PextNode) * trees_num);
	for (int i = 0; i < trees_num; ++i) {
		// printf("\n\n");
		// printf("%d %d %d %d: rules %ld priority %d\n", ranges_rules[i]->tuple_range.x1, ranges_rules[i]->tuple_range.y1, 
			// ranges_rules[i]->tuple_range.x2, ranges_rules[i]->tuple_range.y2, ranges_rules[i]->rules.size(), ranges_rules[i]->max_priority);
		vector<PextRule> pext_rules;
		int pext_rules_num = ranges_rules[i]->rules.size();
		for (int j = 0; j < pext_rules_num; ++j) {
			pext_rules.push_back(PextRule(ranges_rules[i]->rules[j]));
		}
		PextBits pext_bits;
		pext_bits.Init();
		trees[i].Create(pext_rules, pext_bits, 0);
		for (int j = 0; j < pext_rules_num; ++j)
			free(pext_rules[j].port_prefix);
	}
	free(bits_child_num);
	return 0;
}

int PextCuts::Lookup(Trace *trace, int priority) {
	PextNode *pext_node;
	for (int i = 0; i < trees_num; ++i) {
		pext_node = &trees[i];
		if (priority >= pext_node->max_priority)
			break;
		while (true) {
			if (pext_node->type == PextCutIp) {
				pext_node = &pext_node->children[_pext_u64(trace->dst_src_ip, pext_node->cut_ip_bits)];
			} else if (pext_node->type == PextCutPort) {
				pext_node = &pext_node->children[_pext_u32(trace->key[pext_node->dim], pext_node->cut_port_bits)];
			} else {
				for (int j = 0; j < pext_node->rules_arr_num; ++j) {
					if (priority >= pext_node->rules_arr[j].priority)
						break;
					if (pext_node->rules_arr[j].src_ip_begin   <= trace->key[0] && trace->key[0] <= pext_node->rules_arr[j].src_ip_end &&
                        pext_node->rules_arr[j].dst_ip_begin   <= trace->key[1] && trace->key[1] <= pext_node->rules_arr[j].dst_ip_end &&
                        pext_node->rules_arr[j].src_port_begin <= trace->key[2] && trace->key[2] <= pext_node->rules_arr[j].src_port_end &&
                        pext_node->rules_arr[j].dst_port_begin <= trace->key[3] && trace->key[3] <= pext_node->rules_arr[j].dst_port_end &&
                        pext_node->rules_arr[j].protocol_begin <= trace->key[4] && trace->key[4] <= pext_node->rules_arr[j].protocol_end) {
						priority = pext_node->rules_arr[j].priority;
						break;
					}
				}
				break;
			}
			if (priority >= pext_node->max_priority)
				break;
		}
	}
	return priority;
}

int PextCuts::LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state) {
	program_state->AccessClear();
	PextNode *pext_node;
	for (int i = 0; i < trees_num; ++i) {
		pext_node = &trees[i];
		if (priority >= pext_node->max_priority)
			break;
		program_state->access_tuples.AddNum();
		while (true) {
			program_state->access_nodes.AddNum();
			if (pext_node->type == PextCutIp) {
				pext_node = &pext_node->children[_pext_u64(trace->dst_src_ip, pext_node->cut_ip_bits)];
			} else if (pext_node->type == PextCutPort) {
				pext_node = &pext_node->children[_pext_u32(trace->key[pext_node->dim], pext_node->cut_port_bits)];
			} else {
				for (int j = 0; j < pext_node->rules_arr_num; ++j) {
					if (priority >= pext_node->rules_arr[j].priority)
						break;
					program_state->access_rules.AddNum();
					if (pext_node->rules_arr[j].src_ip_begin   <= trace->key[0] && trace->key[0] <= pext_node->rules_arr[j].src_ip_end &&
                        pext_node->rules_arr[j].dst_ip_begin   <= trace->key[1] && trace->key[1] <= pext_node->rules_arr[j].dst_ip_end &&
                        pext_node->rules_arr[j].src_port_begin <= trace->key[2] && trace->key[2] <= pext_node->rules_arr[j].src_port_end &&
                        pext_node->rules_arr[j].dst_port_begin <= trace->key[3] && trace->key[3] <= pext_node->rules_arr[j].dst_port_end &&
                        pext_node->rules_arr[j].protocol_begin <= trace->key[4] && trace->key[4] <= pext_node->rules_arr[j].protocol_end) {
						priority = pext_node->rules_arr[j].priority;
						break;
					}
				}
				break;
			}
			if (priority >= pext_node->max_priority)
				break;
		}
	}
    program_state->AccessCal();
	return priority;
}

int PextNode::Height() {
	int max_height = 0;
	if (type == PextCutIp) {
		int bits_num = Popcnt(cut_ip_bits);
		int children_num = 1 << bits_num;
		for (int i = 0; i < children_num; ++i)
			max_height = max(max_height, children[i].Height());
	} else if (type == PextCutPort) {
		int bits_num = Popcnt(cut_port_bits);
		int children_num = 1 << bits_num;
		for (int i = 0; i < children_num; ++i)
			max_height = max(max_height, children[i].Height());
	} else {
		max_height = 1;
	}
	return max_height + 1;
}

int PextNode::RealHeight() {
	int max_height = 0;
	if (type == PextCutIp) {
		int bits_num = Popcnt(cut_ip_bits);
		int children_num = 1 << bits_num;
		for (int i = 0; i < children_num; ++i)
			max_height = max(max_height, children[i].RealHeight());
	} else if (type == PextCutPort) {
		int bits_num = Popcnt(cut_port_bits);
		int children_num = 1 << bits_num;
		for (int i = 0; i < children_num; ++i)
			max_height = max(max_height, children[i].RealHeight());
	} else {
		max_height = rules_arr_num;
	}
	return max_height + 1;
}

int PextNode::CalculateState(ProgramState *program_state, bool _duplicate) {
	DecisionTreeInfo *info = NULL;
	if (duplicate)
		_duplicate = true;
	if (type == PextCutIp) {
		info = &program_state->layers[layer].info[0];
		int bits_num = Popcnt(cut_ip_bits);
		int children_num = 1 << bits_num;
		info->node_num += 1;
		info->children_num += children_num;
		program_state->tree_child_num += children_num;
		if (!_duplicate) {
			program_state->tree_real_child_num += children_num;
			info->diff_node_num += 1;
			info->diff_children_num += children_num;
		}
		for (int i = 0; i < children_num; ++i)
			children[i].CalculateState(program_state, _duplicate);
	} else if (type == PextCutPort) {
		info = &program_state->layers[layer].info[1];
		int bits_num = Popcnt(cut_port_bits);
		int children_num = 1 << bits_num;
		info->node_num += 1;
		info->children_num += children_num;
		program_state->tree_child_num += children_num;
		if (!_duplicate) {
			info->diff_node_num += 1;
			info->diff_children_num += children_num;
			program_state->tree_real_child_num += children_num;
		}
		for (int i = 0; i < children_num; ++i)
			children[i].CalculateState(program_state, _duplicate);
	} else {
		info = &program_state->layers[layer].info[6];
		info->node_num += 1;
		info->children_num += rules_arr_num;
		program_state->tree_rules_num += rules_arr_num;
		if (!_duplicate) {
			info->diff_node_num += 1;
			info->diff_children_num += rules_arr_num;
			program_state->tree_real_rules_num += rules_arr_num;
		}
	}
	return 0;
}

uint64_t PextNode::MemorySize() {
	uint64_t memory_size = sizeof(PextNode);
	if (duplicate)
		return memory_size;
	if (type == PextCutIp) {
		int bits_num = Popcnt(cut_ip_bits);
		int children_num = 1 << bits_num;
		for (int i = 0; i < children_num; ++i)
			memory_size += children[i].MemorySize();
	} else if (type == PextCutPort) {
		int bits_num = Popcnt(cut_port_bits);
		int children_num = 1 << bits_num;
		for (int i = 0; i < children_num; ++i)
			memory_size += children[i].MemorySize();
	} else {
		memory_size += sizeof(PextRuleNode) * rules_arr_num;
	}
	return memory_size;
}

int PextNode::Free(bool free_self) {
	if (!duplicate) {
		if (type == PextCutIp) {
			int bits_num = Popcnt(cut_ip_bits);
			int children_num = 1 << bits_num;
			for (int i = 0; i < children_num; ++i)
				children[i].Free(false);
			free(children);
		} else if (type == PextCutPort) {
			int bits_num = Popcnt(cut_port_bits);
			int children_num = 1 << bits_num;
			for (int i = 0; i < children_num; ++i)
				children[i].Free(false);
			free(children);
		} else {
			free(rules_arr);
		}
	}
	if (free_self)
		free(this);
	return 0;
}

int PextCuts::CalculateState(ProgramState *program_state) {
	program_state->tuples_num = max(program_state->tuples_num, trees_num);
	program_state->tuples_sum = max(program_state->tuples_sum, trees_num);
	int tree_height_sum = 0;
	int tree_real_height_sum = 0;
	for (int i = 0; i < trees_num; ++i) {
		trees[i].CalculateState(program_state, false);
		tree_height_sum += trees[i].Height();
		tree_real_height_sum += trees[i].RealHeight();
	}

	int current_tree_real_height_sum = 0;
	for (int i = 0; i < program_state->tree_real_height_num.size(); ++i)
		current_tree_real_height_sum += program_state->tree_real_height_num[i];

	if (current_tree_real_height_sum < tree_real_height_sum) {
		program_state->tree_height_num.clear();
		program_state->tree_real_height_num.clear();

		for (int i = 0; i < trees_num; ++i) {
			program_state->tree_height_num.push_back(trees[i].Height());
			program_state->tree_real_height_num.push_back(trees[i].RealHeight());
		}
	}
	return 0;
}

int PextCuts::GetRules(vector<Rule*> &rules) {
	return 0;
}
uint64_t PextCuts::MemorySize() {
	uint64_t memory_size = sizeof(PextCuts);
	// printf("PextNode %ld PextRuleNode %ld\n", sizeof(PextNode), sizeof(PextRuleNode)); // 24 32
	for (int i = 0; i < trees_num; ++i)
		memory_size += trees[i].MemorySize();
	return memory_size;
}

int PextCuts::Free(bool free_self) {
	for (int i = 0; i < trees_num; ++i)
		trees[i].Free(false);
	if (trees)
		free(trees);
	if (free_self)
		free(this);
	return 0;
}

int PextCuts::Test(void *ptr) {
	return 0;
}
