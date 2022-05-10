#ifndef  PEXTCUTS_H
#define  PEXTCUTS_H

#include "../../elementary.h"
#include "../dynamictuple/hash.h"
#include "pextcuts-ranges.h"
#include "../../io/io.h"

#define PextCutIp 0
#define PextCutPort 1
#define PextLeaf 2

using namespace std;

struct RangeRules {
    vector<Rule*> rules;
    int max_priority;

    TupleRange tuple_range;
    int x;
    int y;
};

bool CmpRangeRules(RangeRules* range_rules1, RangeRules* range_rules2);

struct PextBits {
    uint32_t ip_bits[2];
    uint32_t port_bits[2];
    uint32_t protocol_bits;

    void Init() {
        for (int i = 0; i < 2; ++i) {
            ip_bits[i] = 0;
            port_bits[i] = 0;
        }
        protocol_bits = 0;
    }
};

struct PextRulePortPrefix {
    vector<PrefixRange> ports[2];
};

struct PextRule {
    Rule *rule;
    double weight;
    PextRulePortPrefix *port_prefix;

    PextRule(Rule *_rule);
};

struct PextRuleNode {
    uint32_t src_ip_begin, src_ip_end;
    uint32_t dst_ip_begin, dst_ip_end;
    uint16_t src_port_begin, src_port_end;
    uint16_t dst_port_begin, dst_port_end;
    uint8_t protocol_begin, protocol_end;
    int priority;
    void Init(Rule *rule) {
        src_ip_begin   = rule->range[0][0];
        src_ip_end     = rule->range[0][1];
        dst_ip_begin   = rule->range[1][0];
        dst_ip_end     = rule->range[1][1];
        src_port_begin = rule->range[2][0];
        src_port_end   = rule->range[2][1];
        dst_port_begin = rule->range[3][0];
        dst_port_end   = rule->range[3][1];
        protocol_begin = rule->range[4][0];
        protocol_end   = rule->range[4][1];
        priority       = rule->priority;
    }
    uint64_t MemorySize() {
        return sizeof(PextRuleNode);
    }
    
    int Free(bool free_self) {
        if (free_self)
            free(this);
        return 0;
    }
};

struct PextNode {
    char type;  // 0  cut ip, 1 cut port, 2 leaf
    char dim;
    char layer;
    bool duplicate;
    int max_priority;

    union {
        uint64_t cut_ip_bits; // cut, split, leaf_num
        uint32_t cut_port_bits;
        uint32_t rules_arr_num;
    };

    union {
        PextNode *children;
        PextRuleNode *rules_arr;
    };

    void Create(vector<PextRule> &rules, PextBits pext_bits, char _layer);
    void CreateLeaf(vector<PextRule> &_rules);
    void CreateIpCut(vector<PextRule> &rules, uint32_t *bits, PextBits pext_bits);
    void CreatePortCut(vector<PextRule> &rules, uint32_t bits, int _dim, PextBits pext_bits);
    int Height();
    int RealHeight();
    int CalculateState(ProgramState *program_state, bool _duplicate);
    uint64_t MemorySize();
    int Free(bool free_self);
};

class PextCuts : public Classifier {
public:
    
    void Init();
    int Create(vector<Rule*> &_rules, bool insert);

    int InsertRule(Rule *rule) {return 0;}
    int DeleteRule(Rule *rule) {return 0;}
    int Lookup(Trace *trace, int priority);
    int LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state);

    int Reconstruct() {return 0;}
    uint64_t MemorySize();
    int CalculateState(ProgramState *program_state);
    int GetRules(vector<Rule*> &rules);
    int Free(bool free_self);
    int Test(void *ptr);

    int trees_num;
    PextNode *trees;

    double cal_time;
};


#endif