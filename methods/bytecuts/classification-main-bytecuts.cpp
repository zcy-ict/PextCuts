#include "classification-main-bytecuts.h"

using namespace std;
using namespace std::chrono;



void PerformClassificationByteCuts(CommandStruct &command, ProgramState *program_state, 
                       vector<BCRule> &rules, vector<BCPacket> &packets, vector<int> ans,
                       int (ByteCutsClassifier::*ALookup)(const BCPacket& packet)) { 
	// printf("PerformClassificationByteCuts\n");
    int rules_num = rules.size();
    int traces_num = packets.size();
    timeval timeval_start, timeval_end;

    // build
    gettimeofday(&timeval_start,NULL);
	unordered_map<string, string> args;
	ByteCutsClassifier bc(args);
	bc.ConstructClassifier(rules);
    gettimeofday(&timeval_end,NULL);
    program_state->build_time = GetRunTimeUs(timeval_start, timeval_end) / 1000000.0;

    // lookup
    int lookup_round = command.lookup_round;
    vector<uint64_t> lookup_times;
    for (int k = 0; k < lookup_round; ++k) {
        gettimeofday(&timeval_start,NULL);
        for (int i = 0; i < traces_num; ++i)
        	(bc.*ALookup)(packets[i]);
        //     (ByteCutsClassifier.*Lookup)(packets[i]);
            // bc.Lookup(packets[i]);
        gettimeofday(&timeval_end,NULL);
        lookup_times.push_back(GetRunTimeUs(timeval_start, timeval_end));
    }
    uint64_t lookup_time = GetAvgTime(lookup_times);
    program_state->lookup_speed = traces_num / (lookup_time / 1.0);
    program_state->data_memory_size = sizeof(BCRule) * bc.rules.size() / 1024.0 / 1024.0;
    program_state->index_memory_size = bc.MemorySize() / 1024.0 / 1024.0;

    bc.CalculateState(program_state);
    // verify
    //printf("verify\n");
    int hit_num = 0;
    for (int i = 0; i < traces_num; ++i) {
        int priority = bc.LookupAccess(packets[i], program_state);
        int priority2 = (bc.*ALookup)(packets[i]);
        if (priority != priority2)
        	printf("Wrong Lookup or LookupAccess %d %d\n", priority2, priority);
        //int priority = (classifier.*LookupAccess)(traces[i], 0, &rules[i], program_state);
        if (command.force_test > 0) {
            if (ans[i] != priority) {
                printf("May be wrong : %d ans %d priority %d\n", i, ans[i], priority);
                exit(1);
            }
            if (priority != 0)
                ++hit_num;
            //printf("%d\n", priority);
            //break;
        }
        // printf("verify end \n");
    }
    bc.Free(false);
}

void PerformClassificationByteCuts2(CommandStruct &command, ProgramState *program_state, 
                       vector<BCRule> &rules, vector<BCPacket> &packets, vector<int> ans) {
	printf("rules %ld\n", rules.size());
	printf("packets %ld\n", packets.size());    
	printf("Hello, world.\n");
	
	unordered_map<string, string> args;
	
	time_point<steady_clock> start, end;
	duration<double> elapsedSeconds;
	duration<double,std::milli> elapsedMilliseconds;
	
	map<string, string> data;
	data["Name"] = "ByteCuts";
	
	
	printf("Constructing!\n");
	start = steady_clock::now();
	//ByteCutsClassifier bc(args);
	//SpanCutsClassifier bc(args);
	ByteCutsClassifier bc(args);
	bc.ConstructClassifier(rules);
	//StepCuts sc(8);
	//ByteCutsClassifier bc = sc.ConstructClassifier(rules);
	
	end = steady_clock::now();
	elapsedMilliseconds = end - start;
	elapsedSeconds = end - start;
	printf("\tConstruction time: %f ms\n", elapsedMilliseconds.count());
	data["Build"] = to_string(elapsedSeconds.count());
	
	printf("Testing!\n");
	int* results = new int[packets.size()];
	int i = 0;
	start = steady_clock::now();
	for (BCPacket p : packets) {
		results[i++] = bc.Lookup(p);
	}
	end = steady_clock::now();
	elapsedMilliseconds = end - start;
	elapsedSeconds = end - start;
	printf("\tClassification time: %f ms\n", elapsedMilliseconds.count());
	data["Classify"] = to_string(elapsedSeconds.count());
	
	Memory memBytes = bc.MemSizeBytes();
	printf("\tMemory: %d B\n", memBytes);
	printf("\tMemory: %.2f MiB\n", memBytes / (1024 * 1024.0));
	data["Memory"] = to_string(memBytes);
	
	printf("\tTrees: %lu\n", bc.NumTables());
	data["Trees"] = to_string(bc.NumTables());
	int height = 0;
	int maxHeight = 0;
	int cost = 0;
	int maxCost = 0;
	vector<int> heights;
	vector<int> costs;
	vector<int> priors;
	for (size_t i = 0; i < bc.NumTables(); i++) {
		int h = bc.HeightOfTree(i);
		int c = bc.CostOfTree(i);
		printf("Height:  %d / %d\n", h, c);
		height += h;
		maxHeight = max(h, maxHeight);
		cost += c;
		maxCost = max(c, maxCost);
		heights.push_back(h);
		costs.push_back(c);
		priors.push_back(bc.PriorityOfTable(i));
	}
	data["MaxHeight"] = to_string(maxHeight);
	data["SumHeight"] = to_string(height);
	data["Heights"] = Join("-", heights);
	data["MaxCost"] = to_string(maxCost);
	data["SumCost"] = to_string(cost);
	data["Costs"] = Join("-", costs);
	data["Priors"] = Join("-", priors);
	
	size_t firstSize = bc.RulesInTable(0);
	printf("\tRules In First Tree: %lu (%.2f%%)\n", firstSize, 100.0 * firstSize / rules.size());
	data["FirstSize"] = to_string(1.0 * firstSize / rules.size());
	
	size_t rulesFound = 0;
	size_t numTables = 0;
	size_t rules90 = 0.9 * rules.size();
	size_t rules95 = 0.95 * rules.size();
	size_t rules99 = 0.99 * rules.size();
	
	while (rulesFound < rules90) {
		rulesFound += bc.RulesInTable(numTables++);
	}
	data["Table90"] = to_string(numTables);
	while (rulesFound < rules95) {
		rulesFound += bc.RulesInTable(numTables++);
	}
	data["Table95"] = to_string(numTables);
	while (rulesFound < rules99) {
		rulesFound += bc.RulesInTable(numTables++);
	}
	data["Table99"] = to_string(numTables);
	
	data["GoodTrees"] = to_string(bc.NumGoodTrees());
	data["BadTrees"] = to_string(bc.NumBadTrees());
	
	printf("Done testing: %d.\n", i);
	
	// if (!resultsFile.empty()) {
	// 	OutputWriter::WriteResults(resultsFile, results, packets.size());
	// }
	
	delete [] results;
	for (BCPacket p : packets) {
		delete [] p;
	}
	
	
	
	printf("Writing statistics\n");
	vector<string> header = {"Name", "Build", "Classify", "Memory", "MaxHeight", "SumHeight", "MaxCost", "SumCost", "Trees", "FirstSize", "Table90", "Table95", "Table99", "Heights", "Costs", "Priors", "BadTrees", "GoodTrees"};
	vector<map<string, string>> multidata = {data};
	// OutputWriter::WriteCsvFile(statsFile, header, multidata);
	
	printf("Goodbye, world.\n");
}

int ClassificationMainByteCuts(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans) {
    vector<BCRule> bc_rules = GenerateByteCutsRules(rules);
    vector<BCPacket> packets = GenerateByteCutsPackets(traces);
    
    if (command.method_name == "ByteCuts") {
        PerformClassificationByteCuts(command, program_state, bc_rules, packets, ans, &ByteCutsClassifier::Lookup);
    }
    return 0;
}