#ifndef  MULTIPEXTCUTS_H
#define  MULTIPEXTCUTS_H

#include "../../elementary.h"
#include "pextcuts.h"

using namespace std;

class MultiPextCuts : public Classifier {
public:
    
    int Create(vector<Rule*> &rules, bool insert);

    int InsertRule(Rule *rule);
    int DeleteRule(Rule *rule);
    int Lookup(Trace *trace, int priority);
    int LookupAccess(Trace *trace, int priority, Rule *ans_rule, ProgramState *program_state);

    int Reconstruct() {return 0;};
    uint64_t MemorySize();
    int CalculateState(ProgramState *program_state);
    int GetRules(vector<Rule*> &rules) {return 0;};
    int Free(bool free_self);
    int Test(void *ptr);

    int protocol_num[256];
    PextCuts *pextcuts[256];
    int rules_num;

};

#endif