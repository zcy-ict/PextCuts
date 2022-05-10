#include "classification-main.h"

using namespace std;

void PrintTree(ProgramState *program_state) {
    int type_num = 7;
    DecisionTreeInfo info_sum;
    DecisionTreeInfo info_type[7];
    DecisionTreeInfo info_layer[30];
    info_sum.Init();
    for (int i = 0; i < type_num; ++i)
        info_type[i].Init();

    for (int i = 0; i < 30; ++i) {
        info_layer[i].Init();
        for (int j = 0; j < 7; ++j) {
            info_layer[i].Add(program_state->layers[i].info[j]);
            info_type[j].Add(program_state->layers[i].info[j]);
            info_sum.Add(program_state->layers[i].info[j]);
        }
        if (info_layer[i].node_num == 0)
            break;
    }
    if (info_sum.node_num == 0)
        return;
    printf("sum %d %d-%d  %d %d-%d\n\n", info_sum.node_num, info_sum.children_num, info_sum.children_num - info_type[6].children_num, 
                                        info_sum.diff_node_num, info_sum.diff_children_num, info_sum.diff_children_num - info_type[6].diff_children_num);
    for (int i = 0; i < type_num; ++i)
        printf("type%d %d %d  %d %d\n", i, info_type[i].node_num, info_type[i].children_num, info_type[i].diff_node_num, info_type[i].diff_children_num);
    for (int i = 0; i < 30; ++i) {
        if (info_layer[i].node_num == 0)
            break;
        printf("\nLayer %d\n", i);
        printf("%d %d  %d %d\n", info_layer[i].node_num, info_layer[i].children_num, info_layer[i].diff_node_num, info_layer[i].diff_children_num);
        for (int j = 0; j < 7; ++j) {
            DecisionTreeInfo info2 = program_state->layers[i].info[j];
            printf("  type%d %d %d  %d %d\n", j, info2.node_num, info2.children_num, info2.diff_node_num, info2.diff_children_num);
        }
    }
}

void PrintProgramState(CommandStruct command, ProgramState *program_state) {
    for (int i = 0; i < program_state->tree_height_num.size(); ++i)
        program_state->tree_height_sum += program_state->tree_height_num[i]; 
    for (int i = 0; i < program_state->tree_real_height_num.size(); ++i)
        program_state->tree_real_height_sum += program_state->tree_real_height_num[i];

    if (command.method_name == "DynamicTuple_Basic" || command.method_name == "DynamicTuple" ||
        command.method_name == "DynamicTuple_Dims" || command.method_name == "MultilayerTuple" ||
        command.method_name == "PSTSS" || command.method_name == "PartitionSort" ||
        command.method_name == "TupleMerge")
        program_state->max_access_num = program_state->max_access_tuple_rule;
    
    if (command.method_name == "PextCuts_Basic" || command.method_name == "PextCuts" ||
        command.method_name == "ByteCuts" || command.method_name == "CutSplit" )
        program_state->max_access_num =  program_state->max_access_node_rule;

    if (command.method_name == "MultilayerTuple" && command.prefix_dims_num ==5) {
        command.method_name = "MultilayerTuple(5)"; 
    }

    if (command.print_mode == 0) {
        printf("\n");
        // PrintTree(program_state);
        printf("rules %s\n", command.rules_file.c_str());
        if (command.prefix_dims_num > 2)
            printf("prefix_dims_num: %d\n", command.prefix_dims_num);
        printf("method_name : %s\n", command.method_name.c_str());
        printf("rules_num: %d\t", program_state->rules_num);
        printf("traces_num: %d\n\n", program_state->traces_num);

        printf("data_memory_size: %.3f MB\n", program_state->data_memory_size);
        printf("index_memory_size: %.3f MB\n\n", program_state->index_memory_size);
        double memory_size = program_state->data_memory_size + program_state->index_memory_size;
        printf("memory_size: %.3f MB\n\n", memory_size);

        printf("build_time: %.3f S\n", program_state->build_time);
        printf("lookup_speed: %.3f MLPS\n", program_state->lookup_speed);
        // printf("insert_speed: %.2f MUPS\n", program_state->insert_speed);
        // printf("delete_speed: %.2f MUPS\n", program_state->delete_speed);
        if (program_state->update_speed < 0 || program_state->update_speed > 100)
            program_state->update_speed = 0;
        printf("update_speed: %.2f MUPS\n\n", program_state->update_speed);

        printf("tuples/trees num: %d\n", program_state->tuples_num);
        printf("tuples_sum: %d\n", program_state->tuples_sum);
        printf("access_tuples_sum: %d\taccess_tuples_avg: %.2f\taccess_tuples_max: %d\n", program_state->access_tuples.sum, 1.0 * program_state->access_tuples.sum / program_state->traces_num, program_state->access_tuples.maxn);
        // // printf("查找哈希表总数: %d\t平均查找哈希表数目: %.2f\t最大查找哈希表数目: %d\n", program_state->access_tables.sum, 1.0 * program_state->access_tables.sum / program_state->traces_num, program_state->access_tables.maxn);
        printf("access_node_sum:   %d\taccess_node_sum:   %.2f\taccess_node_sum:   %d\n", program_state->access_nodes.sum, 1.0 * program_state->access_nodes.sum / program_state->traces_num, program_state->access_nodes.maxn);
        printf("access_rules_sum:  %d\taccess_rules_avg:  %.2f\taccess_rules_max:  %d\n", program_state->access_rules.sum, 1.0 * program_state->access_rules.sum / program_state->traces_num, program_state->access_rules.maxn);

        printf("max_access_num:  %d\n", program_state->max_access_num);
        
        printf("next_layer_num:  %d\n", program_state->next_layer_num);

        // printf("tree_real_height %d\n", program_state->tree_real_height_sum);
        printf("\n\n\n\n");
    } else if (command.print_mode == 1) {
        printf("|%18s | ", command.method_name.c_str());
        printf("%20s | ", command.rules_file.c_str());
        double memory_size = program_state->data_memory_size + program_state->index_memory_size;
        printf("%6.2f MB |", memory_size);
        printf("%6.2f Seconds |", program_state->build_time);
        printf("\033[1;32;31m%8.0f\033[0m KLPS |", program_state->lookup_speed * 1000);
        if (program_state->update_speed < 0 || program_state->update_speed > 100)
            program_state->update_speed = 0;
        printf("%8.0f KUPS |", program_state->update_speed * 1000);
        printf("\n");
    } else if (command.print_mode == 2) {
        // printf("%s\t", command.method_name.c_str());
        printf("%d\t", program_state->rules_num);
        printf("%d\t", program_state->traces_num);

        printf("%.3f\t", program_state->data_memory_size);
        printf("%.3f\t", program_state->index_memory_size);

        printf("%.4f\t", program_state->build_time);
        printf("%.4f\t", program_state->lookup_speed);
        // printf("%.2f\t", program_state->insert_speed);
        // printf("%.2f\t", program_state->delete_speed);
        printf("%.2f\t", program_state->update_speed);

        printf("%d\t", program_state->tuples_num);
        printf("%d\t", program_state->tuples_sum);
        printf("%d\t%.2f\t%d\t", program_state->access_tuples.sum, 1.0 * program_state->access_tuples.sum / program_state->traces_num, program_state->access_tuples.maxn);
        // printf("%d\t%.2f\t%d\t", program_state->access_tables.sum, 1.0 * program_state->access_tables.sum / program_state->traces_num, program_state->access_tables.maxn);
        printf("%d\t%.2f\t%d\t", program_state->access_nodes.sum, 1.0 * program_state->access_nodes.sum / program_state->traces_num, program_state->access_nodes.maxn);
        printf("%d\t%.2f\t%d\t", program_state->access_rules.sum, 1.0 * program_state->access_rules.sum / program_state->traces_num, program_state->access_rules.maxn);
        
        printf("%d\t", program_state->max_access_num);
        printf("%d\t", program_state->next_layer_num);
        printf("\n");
    }
}

int ClassificationMain(CommandStruct command) {

    vector<Rule*> rules = ReadRules(command.rules_file, command.rules_shuffle);
    vector<Trace*> traces = ReadTraces(command.traces_file);

    if (command.prefix_dims_num == 5)
        rules = RulesPortPrefix(rules, true);
    
    vector<int> ans = GenerateAns(rules, traces, command);
    
    ProgramState *program_state = new ProgramState();
    program_state->rules_num = rules.size();
    program_state->traces_num = traces.size();

    if (command.method_name == "DynamicTuple_Basic" || command.method_name == "DynamicTuple" ||
        command.method_name == "DynamicTuple_Dims" || command.method_name == "MultilayerTuple" ||
        command.method_name == "PextCuts_Basic" || command.method_name == "PextCuts") {
        ClassificationMainZcy(command, program_state, rules, traces, ans);
    } else if (command.method_name == "ByteCuts") {
        ClassificationMainByteCuts(command, program_state, rules, traces, ans);
    } else if (command.method_name == "CutSplit") {
        ClassificationMainCutSplit(command, program_state, rules, traces, ans);
    } else if (command.method_name == "PSTSS" || command.method_name == "PartitionSort" ||
        command.method_name == "TupleMerge") {
        ClassificationMainPS(command, program_state, rules, traces, ans);
    } else {
        printf("No such method %s\n", command.method_name.c_str());
    }

    PrintProgramState(command, program_state);
    
    FreeRules(rules);
    FreeTraces(traces);
    return 0;
}