#include "multipextcuts.h"

#include <immintrin.h>

using namespace std;

int MultiPextCuts::Create(vector<Rule*> &rules, bool insert) {
    int rules_num = rules.size();
    memset(protocol_num, 0, sizeof(protocol_num));
    for (int i = 0; i < rules_num; ++i)
            ++protocol_num[rules[i]->range[4][0]];
    for (int i = 0; i < 256; ++i)
        if (i == 0 || protocol_num[i] > 0) {
            pextcuts[i] = new PextCuts();
        } else {
            pextcuts[i] = pextcuts[0];
        }
    for (int i = 0; i < 256; ++i)
        if (i == 0 || protocol_num[i] > 0) {
            vector<Rule*> protocol_rules;
            protocol_rules.clear();
            for (int j = 0; j < rules_num; ++j)
                if (rules[j]->range[4][0] <= i && i <= rules[j]->range[4][1])
                    protocol_rules.push_back(rules[j]);
            // printf("%d %ld\n", i, protocol_rules.size());
            protocol_rules = UniqueRulesIgnoreProtocol(protocol_rules);
            // printf("%d %ld\n", i, protocol_rules.size());
            pextcuts[i]->Create(protocol_rules, true);
        }
    return 0;
}

int MultiPextCuts::InsertRule(Rule *rule) {
	return 0;
}

int MultiPextCuts::DeleteRule(Rule *rule) {
	return 0;
}

int MultiPextCuts::Lookup(Trace *trace, int priority) {
    // return pextcuts[trace->key[4]]->Lookup(trace, priority);
    PextNode *pext_node;
    for (int i = 0; i < pextcuts[trace->key[4]]->trees_num; ++i) {
        pext_node = &pextcuts[trace->key[4]]->trees[i];
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
                        pext_node->rules_arr[j].dst_port_begin <= trace->key[3] && trace->key[3] <= pext_node->rules_arr[j].dst_port_end
                        // && pext_node->rules_arr[j].protocol_begin <= trace->key[4] && trace->key[4] <= pext_node->rules_arr[j].protocol_end
                        ) {
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


int MultiPextCuts::LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state) {
    return pextcuts[trace->key[4]]->LookupAccess(trace, priority, ans_rule, program_state);
    // return Lookup(trace, priority);
}

uint64_t MultiPextCuts::MemorySize() {
    uint64_t memory_size = sizeof(MultiPextCuts);
    for (int i = 0; i < 256; ++i)
        if (i == 0 || protocol_num[i] > 0)
            memory_size += pextcuts[i]->MemorySize();
    return memory_size;
}

int MultiPextCuts::CalculateState(ProgramState *program_state) {
    for (int i = 0; i < 256; ++i)
        if (i == 0 || protocol_num[i] > 0)
            pextcuts[i]->CalculateState(program_state);
    return 0;
}

int MultiPextCuts::Free(bool free_self) {
    for (int i = 0; i < 256; ++i)
        if (i == 0 || protocol_num[i] > 0)
            pextcuts[i]->Free(true);
    if (free_self)
        free(this);
    return 0;
}

int MultiPextCuts::Test(void *ptr) {
    return 0;
}