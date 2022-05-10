#include<stdio.h>
#include<stdlib.h> 
#include<unistd.h>
#include<math.h>
#include<list>
#include"CutSplit.h"  
#include"hs.h"  
#include"trie.h" 
#include "common.h"
#include <sys/time.h>
#include "stdinc.h"

using namespace std;

int cs_bucketSize = 8;   // leaf threashold
int cs_threshold = 24;   // Assume T_SA=T_DA=threshold

hs_node_t* big_node;
trie T_sa;
trie T_da;
int num_subset_3[3]={0,0,0}; 
pc_rule *pc_rules; 


void count_length(int number_rule,pc_rule *rule,field_length *field_length_ruleset)
{
   unsigned temp_size=0;
   unsigned temp_value=0;
   //unsigned temp0=0;

   for(int i=0;i<number_rule;i++)
   {
       for(int j=0;j<5;j++)  //record field length in field_length_ruleset[i]
       {
          field_length_ruleset[i].length[j]=rule[i].field[j].high-rule[i].field[j].low;    
          if(field_length_ruleset[i].length[j]==0xffffffff)
             field_length_ruleset[i].size[j]=32; //for address *
          else 
          {
             temp_size=0;
             temp_value=field_length_ruleset[i].length[j]+1;   
             while((temp_value=temp_value/2)!=0)
                temp_size++;  
             //for port number 
             if((field_length_ruleset[i].length[j]+1 - pow(2,temp_size))!=0) 
               temp_size++; 

             field_length_ruleset[i].size[j]=temp_size;
          }
       }
   }
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  partition_v1 (version1)
 *  Description:  partition ruleset into subsets based on address field(2 dim.)
 * =====================================================================================
 */
void partition_v1(pc_rule *rule,pc_rule* subset[3],int num_subset[3],int number_rule,field_length *field_length_ruleset,int threshold_value[2])
{
  int num_small_tmp[number_rule];
  for(int i=0;i<number_rule;i++){ 
      num_small_tmp[i]=0;
      for(int k=0;k<2;k++)
         if(field_length_ruleset[i].size[k] <= threshold_value[k]) 
            num_small_tmp[i]++; 
  }

  int count_big=0;  
  for(int i=0;i<number_rule;i++)
     if(num_small_tmp[i]==0) {
     	// printf("%d %d %d\n", i, field_length_ruleset[i].size[0], field_length_ruleset[i].size[1]);
        subset[0][count_big++]=rule[i];

     }
  num_subset[0]=count_big;

  int count_sa=0;
  int count_da=0;
  for(int i=0;i<number_rule;i++){
      if((num_small_tmp[i]==1)&&(field_length_ruleset[i].size[0]<=threshold_value[0])) 
         subset[1][count_sa++]=rule[i];      
      if((num_small_tmp[i]==1)&&(field_length_ruleset[i].size[1]<=threshold_value[1])) 
         subset[2][count_da++]=rule[i];

      if(num_small_tmp[i]==2)
      {
         if(field_length_ruleset[i].size[0]<field_length_ruleset[i].size[1]) 
            subset[1][count_sa++]=rule[i];
         else if(field_length_ruleset[i].size[0]>field_length_ruleset[i].size[1])
            subset[2][count_da++]=rule[i];     
         else if(count_sa<=count_da)     
            subset[1][count_sa++]=rule[i];
         else
            subset[2][count_da++]=rule[i];
      }
  }

   num_subset[1]=count_sa; 
   num_subset[2]=count_da;
   // printf("Big_subset:%d\tSa_subset:%d\tDa_subset:%d\n\n",count_big,count_sa,count_da);

/*
   printf("***********************big_ruleset*******************************************\n");
        if(num_subset[0]!=0)
           dump_ruleset(subset[0],num_subset[0]);
        else
           printf("empty!\n");
   printf("***********************SA_ruleset********************************************\n");
        dump_ruleset(subset[1],num_subset[1]);
   printf("***********************DA_ruleset********************************************\n");
        dump_ruleset(subset[2],num_subset[2]);
*/

}

int CutSplit::Create(pc_rule* rules, int rules_num) {
	// printf("CutSplit Create\n");

	pc_rules = rules;

	field_length *field_length_ruleset = (field_length*)malloc(sizeof(field_length) * rules_num);
	count_length(rules_num, rules, field_length_ruleset);
	pc_rule* subset_3[3];  
	for(int n=0;n<3;n++)
	  subset_3[n]=(pc_rule *)malloc(rules_num*sizeof(pc_rule)); 
	int threshold_value_3[2]={cs_threshold,cs_threshold};   
	partition_v1(rules,subset_3,num_subset_3,rules_num,field_length_ruleset,threshold_value_3); 

  // exit(1);
	T_sa = trie(num_subset_3[1],cs_bucketSize,rules,subset_3[1],cs_threshold,0); 
	T_da = trie(num_subset_3[2],cs_bucketSize,rules,subset_3[2],cs_threshold,1); 

  big_node = (hs_node_t *) malloc(sizeof(hs_node_t));	
	if(num_subset_3[0] > 0){  
    if (num_subset_3[0] > 1500)
      cs_bucketSize = 16;
		hstrie T(num_subset_3[0],subset_3[0], cs_bucketSize, big_node);
    // printf("***Big rules(using HyperSlit):***\n");
    // printf(">>RESULTS:");
    // printf("\n>>number of rules: %d", T.result.num_rules);
    // printf("\n>>number of children: %d", T.result.num_childnode);
    // printf("\n>>worst case tree depth: %d", T.result.wst_depth);
    // printf("\n>>average tree depth: %f", T.result.avg_depth);
    // printf("\n>>number of tree nodes:%d", T.result.num_tree_node);
    // printf("\n>>number of leaf nodes:%d", T.result.num_leaf_node);
    // printf("\n>>total memory: %f(KB)", T.result.total_mem_kb);
    // printf("\n***SUCCESS in building HyperSplit sub-tree for big rules***\n\n");
	} else {
		big_node = NULL;
	}

  return 0;
}

int CutSplit::Lookup(uint32_t *header) {
	int matchid = -1;
	int match_sa = -1;
	int match_da = -1;
	int match_big = -1;

	// printf("lookup1\n");
  match_sa = T_sa.trieLookup((int*)header);
  // printf("lookup2\n");
  match_da = T_da.trieLookup((int*)header);
  // printf("lookup3\n");
  if(num_subset_3[0] > 0) match_big = LookupHSTree(pc_rules,big_node,(int*)header);
  // printf("\nmatch_sa = %d   match_da = %d   match_big = %d\n", match_sa, match_da, match_big);

  if(match_sa != -1) matchid = match_sa;
  if((matchid == -1) || (match_da != -1 && match_da < matchid)) matchid = match_da;
  if((matchid == -1) || (match_big != -1 && match_big < matchid)) matchid = match_big;
  return matchid;
}

int CutSplit::LookupAccess(uint32_t *header, ProgramState *program_state) {
    program_state->AccessClear();

    int matchid = -1;
    int match_sa = -1;
    int match_da = -1;
    int match_big = -1;

    // printf("lookup1\n");
    program_state->access_tuples.AddNum();
    match_sa = T_sa.trieLookupAccess((int*)header, program_state);
    // printf("lookup2\n");
    program_state->access_tuples.AddNum();
    match_da = T_da.trieLookupAccess((int*)header, program_state);
    // printf("lookup3\n");
    if(num_subset_3[0] > 0) {
        program_state->access_tuples.AddNum();
        match_big = LookupHSTreeAccess(pc_rules,big_node,(int*)header, program_state);
    }
    // printf("\nmatch_sa = %d   match_da = %d   match_big = %d\n", match_sa, match_da, match_big);

    if(match_sa != -1) matchid = match_sa;
    if((matchid == -1) || (match_da != -1 && match_da < matchid)) matchid = match_da;
    if((matchid == -1) || (match_big != -1 && match_big < matchid)) matchid = match_big;


    program_state->AccessCal();


  return matchid;
}

void CutSplit::Free() {
  FreeHSTree(big_node, true);
  T_sa.Freetrie();
  T_da.Freetrie();
}

uint64_t CutSplit::MemorySize() {
  uint64_t memory_size = 0;
  memory_size += HSMemorySize(big_node);
  // printf("%.2f KB\n", memory_size / 1024.0);

  uint64_t sa = T_sa.MemorySize();
  memory_size += sa;
  // printf("%.2f KB\n", sa / 1024.0);

  uint64_t da = T_da.MemorySize();
  memory_size += da;
  // printf("%.2f KB\n", da / 1024.0);

  return memory_size;
}

void CutSplit::CalculateState(ProgramState *program_state) {
  int tuples_num = 2;
  if (big_node)
     ++tuples_num;

  program_state->tuples_num = tuples_num;
  program_state->tuples_sum = tuples_num;

  program_state->tree_height_num.push_back(T_sa.Height());
  program_state->tree_real_height_num.push_back(T_sa.RealHeight());
  T_sa.CalNum(program_state);

  program_state->tree_height_num.push_back(T_da.Height());
  program_state->tree_real_height_num.push_back(T_da.RealHeight());
  T_da.CalNum(program_state);

  if (big_node) {
      program_state->tree_height_num.push_back(HSHeight(big_node));
      program_state->tree_real_height_num.push_back(HSRealHeight(big_node));
      HSCalNum(big_node, program_state);
  }


}