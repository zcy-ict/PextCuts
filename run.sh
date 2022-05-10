# data_path="../../ClassBench/data/"
data_path="./data/"
output="output"
loop_times=1
rules_shuffle=1
lookup_round=10
print_mode=1
force_test=0

RunProgram() {
    program_name="$1"
    method_name="$2"
    data="$3"

    rules="${data_path}${data}_rules"
    traces="${data_path}${data}_traces"

    i=0
    while [ $i -lt $loop_times ]
    do
        if [ $loop_times -gt 1 ]; then
            echo "  \c" >> ${output}
        else
            echo "${program_name}\t${data}\t\c" >> ${output}
        fi
        ./main --run_mode classification --method_name ${method_name} \
               --rules_file ${rules} --traces_file ${traces} \
               --rules_shuffle ${rules_shuffle} --lookup_round ${lookup_round} \
               --print_mode ${print_mode} --force_test ${force_test} ${4} ${5} ${6} ${7} ${8} ${9} ${10} ${11} ${12} ${13} ${14} ${15} ${16} ${17} ${18} ${19} ${20} ${21} ${22} ${24} ${25} ${26} ${27} ${28} ${29} ${30} \
               # >> ${output}
        i=`expr $i + 1`
    done
    
    if [ $loop_times -gt 1 ]; then
        echo "${program_name} ${data}" >> ${output}
    fi
}

RunMethod() {
    for data_size in 10K #50K 100K #
    do
        for data_type in acl1 #acl2 acl3 acl4 acl5  fw1 fw2 fw3 fw4 fw5 ipc1 ipc2 
        do
            data=${data_size}_${data_type}
            echo "${data}" >> ${output}

            RunProgram    PSTSS                   PSTSS                   ${data}
            RunProgram    PartitionSort           PartitionSort           ${data}
            RunProgram    TupleMerge              TupleMerge              ${data}
            RunProgram    MultilayerTuple-2         MultilayerTuple         ${data}
            RunProgram    MultilayerTuple-5         MultilayerTuple         ${data} --prefix_dims_num 5
            RunProgram    DynamicTuple_Basic      DynamicTuple_Basic      ${data}
            RunProgram    DynamicTuple            DynamicTuple            ${data}
            RunProgram    DynamicTuple_Dims       DynamicTuple_Dims       ${data} --prefix_dims_num 5

            RunProgram    CutSplit                  CutSplit                  ${data}
            RunProgram    ByteCuts                ByteCuts                ${data} 
            RunProgram    PextCuts_Basic          PextCuts_Basic          ${data} 
            RunProgram    PextCuts                PextCuts                ${data} 
        done
    done
}

make

rm -rf ${output}
date >> ${output}

echo "method_name data_set \
rules_num traces_num \
data_memory_size(MB) index_memory_size(MB) \
build_time(S) \
lookup_speed(MPPS) \
update_speed(MPPS) \
tuples_num tuples_sum \
access_tuples_sum access_tuples_avg access_tuples_max \
access_nodes_sum access_nodes_avg access_nodes_max \
access_rules_sum access_rules_avg access_rules_max \
max_access_num next_layer_num" >> ${output}


RunMethod
echo "\n\n change print_mode=0 in run.sh to see more details"

date >> ${output}
