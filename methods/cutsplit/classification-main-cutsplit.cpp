#include "classification-main-cutsplit.h"

using namespace std;


void PerformClassificationCutSplit(CommandStruct command, ProgramState *program_state, 
								   pc_rule* rules, int rules_num, uint32_t** traces, int traces_num,
								   vector<int> &ans) {
	// printf("PerformClassificationCutSplit\n");

    timeval timeval_start, timeval_end;

	CutSplit cutsplit;
    gettimeofday(&timeval_start,NULL);
	cutsplit.Create(rules, rules_num);
	gettimeofday(&timeval_end,NULL);
    program_state->build_time = GetRunTimeUs(timeval_start, timeval_end) / 1000000.0;

    // lookup
    //printf("lookup\n");
    int lookup_round = command.lookup_round;
    // printf("lookup_round %d\n", lookup_round);
    // classifier.Reconstruct();

    vector<uint64_t> lookup_times;
    for (int k = 0; k < lookup_round; ++k) {
        gettimeofday(&timeval_start,NULL);
        for (int i = 0; i < traces_num; ++i)
            cutsplit.Lookup(traces[i]);
        gettimeofday(&timeval_end,NULL);
        lookup_times.push_back(GetRunTimeUs(timeval_start, timeval_end));
    }
    uint64_t lookup_time = GetAvgTime(lookup_times);
    program_state->lookup_speed = traces_num / (lookup_time / 1.0);

	int hit_num = 0;
    for (int i = 0; i < traces_num; ++i) {
        //int priority = classifier.Lookup(traces[i], 0);
        // printf("verify %d\n", i);
        int priority = cutsplit.LookupAccess(traces[i], program_state);
        priority = rules_num - priority;
        //int priority = (classifier.*LookupAccess)(traces[i], 0, &rules[i], program_state);
        if (command.force_test > 0) {
            if (ans[i] != priority) {
                printf("May be wrong : %d ans %d lookup %d\n", i, ans[i], priority);
                exit(1);
            }
            if (priority != 0)
                ++hit_num;
            //printf("%d\n", priority);
            //break;
        }
        // printf("verify end \n");
    }
    program_state->data_memory_size = sizeof(pc_rule) * rules_num / 1024.0 / 1024.0;
    program_state->index_memory_size = cutsplit.MemorySize() / 1024.0 / 1024.0;

    cutsplit.CalculateState(program_state);

    cutsplit.Free();
}

int ClassificationMainCutSplit(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans) {
    pc_rule* pc_rules = GenerateCutSplitRules(rules);
    // PrintByteCutsRules(bc_rules);
    uint32_t** packets = GenerateCutSplitPackets(traces);
    
    PerformClassificationCutSplit(command, program_state, pc_rules, rules.size(), packets, traces.size(), ans);
    
    FreeCutSplitRules(pc_rules);
    FreeCutSplitPackets(packets, traces.size());
    return 0;
}