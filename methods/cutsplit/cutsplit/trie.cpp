/*-----------------------------------------------------------------------------
 *  
 *  Name:		trie.c for CutSplit
 *  Description:	trie construction for CutSplit: Pre-Cutting + Post-Splitting
 *  Version:		2.0 (release)
 *  Author:		Wenjun Li (Peking University, Email:wenjunli@pku.edu.cn)	 
 *  Date:		5/3/2019
 * 
 *-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "stdinc.h"
#include<queue>
#include<list>
#include"CutSplit.h"
#include"hs.h"  
#include "trie.h"

using namespace std;

trie::trie(int numrules1, int binth1, pc_rule* rule1, pc_rule* rule2, int threshold1, int k1)
{
   numrules = numrules1;  
   binth = binth1;
   rule = rule1;
   k=k1;      //0:SA, 1:DA 
   threshold = pow(2,threshold1); 
   nodeSet = new nodeItem[MAXNODES+1]; 
   root = 1;  

   pass=1;
   max_depth=0;
   Total_Rule_Size=0;
   Total_Array_Size=0;
   Leaf_Node_Count=0;
   NonLeaf_Node_Count=0;
   total_ficuts_memory_in_KB=0;
   total_hs_memory_in_KB=0;
   total_memory_in_KB=0;


   for(int i=1; i<=MAXNODES; i++) 
      nodeSet[i].child = (int*)malloc(sizeof(int)); 

    
   nodeSet[root].isleaf = 0;
   nodeSet[root].nrules = numrules;

   for (int i=0; i<MAXDIMENSIONS; i++){
        nodeSet[root].field[i].low = 0;
        if(i<2)
           nodeSet[root].field[i].high = 0xffffffff;
        else if(i==4) 
           nodeSet[root].field[i].high = 255;
        else 
           nodeSet[root].field[i].high = 65535; 
   }

  nodeSet[root].ruleid = (int*)calloc(numrules, sizeof(int));
  for(int i=0; i<numrules; i++)
      nodeSet[root].ruleid[i] = rule2[i].id;  


  nodeSet[root].ncuts = 0;
  nodeSet[root].layNo = 1; 
  nodeSet[root].flag = 1; //cut

  freelist = 2;
  for (int i = 2; i < MAXNODES; i++)
      nodeSet[i].child[0] = i+1;
  nodeSet[MAXNODES].child[0] = Null;

  createtrie();
}
trie::~trie() {
  // delete [] nodeSet;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  count_np_ficut
 *  Description:  count np_ficut in pre-cutting stage (using FiCut[from HybridCuts] for one small field)
 * =====================================================================================
 */
int trie::count_np_ficut(nodeItem *v)
{
   int done=0;
   int sm=0;
   int nump=0; 
   int *nr;   
   nr=(int *)malloc(sizeof(int));
   int lo,hi,r; 

   if(v->field[k].high == v->field[k].low)
      nump=1;
   else
      nump=2;
     
   while(!done){
        if(nump < MAXCUTS && (v->field[k].high - v->field[k].low) > threshold)
           nump=nump*2;
        else
           done=1; 
   }
   return nump; 
}

void trie::createtrie()
{
   int np=0; 
   int nr;
   int empty;
   unsigned int r1, lo1, hi1;
   int u,v;
   int s = 0;

   qNode.push(root);

   while(!qNode.empty()){
       v=qNode.front();
       qNode.pop();

       if(nodeSet[v].flag==1){
           np=count_np_ficut(&nodeSet[v]);
           if(np<MAXCUTS) 
               nodeSet[v].flag=2;
       }

       if(nodeSet[v].flag==1) //FiCuts stage
       {
           if(nodeSet[v].nrules <= binth || np == 1){ //leaf node
               nodeSet[v].isleaf = 1;
               Total_Rule_Size+= nodeSet[v].nrules;
               Leaf_Node_Count++;
               if(max_depth<(nodeSet[v].layNo+nodeSet[v].nrules))
                   max_depth=nodeSet[v].layNo+nodeSet[v].nrules;
           }
           else{
               NonLeaf_Node_Count++;
               nodeSet[v].ncuts = np;
               nodeSet[v].child = (int *)realloc(nodeSet[v].child, nodeSet[v].ncuts * sizeof(int));

               Total_Array_Size += nodeSet[v].ncuts;

               r1 = (nodeSet[v].field[k].high - nodeSet[v].field[k].low)/nodeSet[v].ncuts;
               lo1 = nodeSet[v].field[k].low;
               hi1 = lo1 + r1;

               for(int i = 0; i < nodeSet[v].ncuts; i++){
                   empty = 1;
                   nr = 0;
                   for(int j=0; j<nodeSet[v].nrules; j++){
                       if(rule[nodeSet[v].ruleid[j]].field[k].low >=lo1 && rule[nodeSet[v].ruleid[j]].field[k].low <=hi1 ||
                          rule[nodeSet[v].ruleid[j]].field[k].high>=lo1 && rule[nodeSet[v].ruleid[j]].field[k].high<=hi1 ||
                          rule[nodeSet[v].ruleid[j]].field[k].low <=lo1 && rule[nodeSet[v].ruleid[j]].field[k].high>=hi1){
                           empty = 0;
                           nr++;  
                       }
                   }

                   if(!empty){
                       nodeSet[v].child[i] = freelist;
                       u=freelist;
                       freelist = nodeSet[freelist].child[0]; 
                       nodeSet[u].nrules = nr;
                       nodeSet[u].layNo=nodeSet[v].layNo+1;
                       if(nr <= binth){ 
                           nodeSet[u].isleaf = 1;
                           Total_Rule_Size+= nr;
                           Leaf_Node_Count++;

                           if(max_depth<(nodeSet[u].layNo+nr))
                               max_depth=nodeSet[v].layNo+nr;

                       }
                       else{
                           nodeSet[u].isleaf = 0;

                           if(np<MAXCUTS)
                               nodeSet[u].flag=2;  //split
                           else
                               nodeSet[u].flag=1;  //cut

                           if(pass<nodeSet[u].layNo)
                               pass=nodeSet[u].layNo;

                           qNode.push(u); 
                       }

                       for (int t=0; t<MAXDIMENSIONS; t++){   
                           if(t != k){
                               nodeSet[u].field[t].low = nodeSet[v].field[t].low;
                               nodeSet[u].field[t].high= nodeSet[v].field[t].high;
                           }
                           else{
                               nodeSet[u].field[t].low = lo1;
                               nodeSet[u].field[t].high= hi1;
                           }
                       }

                       s = 0;
                       nodeSet[u].ruleid = (int *)calloc(nodeSet[v].nrules, sizeof(int));
                       for(int j=0; j<nodeSet[v].nrules; j++){  //update rules in node
                           if(rule[nodeSet[v].ruleid[j]].field[k].low >=lo1 && rule[nodeSet[v].ruleid[j]].field[k].low <=hi1 ||
                              rule[nodeSet[v].ruleid[j]].field[k].high>=lo1 && rule[nodeSet[v].ruleid[j]].field[k].high<=hi1 ||
                              rule[nodeSet[v].ruleid[j]].field[k].low <=lo1 && rule[nodeSet[v].ruleid[j]].field[k].high>=hi1)
                           {
                               nodeSet[u].ruleid[s] = nodeSet[v].ruleid[j];
                               s++;
                           }
                       }
                   }
                   else //empty
                       nodeSet[v].child[i] = Null;
				   
                   lo1 = hi1 + 1;
                   hi1 = lo1 + r1;
               }
           }
       } else{ //HyperSplit stage
           if(nodeSet[v].nrules <= binth){  //leaf node
               nodeSet[v].isleaf = 1;

               Total_Rule_Size+= nodeSet[v].nrules;
               Leaf_Node_Count++;
               if(max_depth<(nodeSet[v].layNo+nodeSet[v].nrules))
                   max_depth=nodeSet[v].layNo+nodeSet[v].nrules;
           }
           else{
               NonLeaf_Node_Count++;
               pc_rule* subset_node;
               subset_node = (pc_rule *)malloc(nodeSet[v].nrules*sizeof(pc_rule));
               for(int m=0;m<nodeSet[v].nrules;m++){
                   subset_node[m].id = nodeSet[v].ruleid[m];
                   for(int k = 0; k < MAXDIMENSIONS; k++){
                       subset_node[m].field[k].low = rule[nodeSet[v].ruleid[m]].field[k].low;
                       subset_node[m].field[k].high = rule[nodeSet[v].ruleid[m]].field[k].high;
                   }
               }

               nodeSet[v].rootnode = (hs_node_t *) malloc(sizeof(hs_node_t));
               hstrie Ttmp(nodeSet[v].nrules,subset_node, binth, nodeSet[v].rootnode);

               total_hs_memory_in_KB += Ttmp.result.total_mem_kb;

               nodeSet[v].layNo += Ttmp.result.wst_depth;

               if(max_depth<nodeSet[v].layNo)
                   max_depth=nodeSet[v].layNo;

           }

       }

   }


  total_ficuts_memory_in_KB=double(Total_Rule_Size*PTR_SIZE+Total_Array_Size*PTR_SIZE+LEAF_NODE_SIZE*Leaf_Node_Count+TREE_NODE_SIZE*NonLeaf_Node_Count)/1024;
  total_memory_in_KB = total_ficuts_memory_in_KB + total_hs_memory_in_KB;


   if(numrules>binth){
   // if(k==0)
   //    printf("***SA Subset Tree(using FiCuts + HyperSlit):***\n");
   // if(k==1)
   //    printf("***DA Subset Tree(using FiCuts + HyperSlit):***\n");
   // printf(">>RESULTS:");
   // printf("\n>>number of rules:%d",numrules);
   // printf("\n>>worst case tree level: %d",pass);
   // printf("\n>>worst case tree depth: %d",max_depth);
   // printf("\n>>total memory(Pre-Cutting): %f(KB)",total_ficuts_memory_in_KB);
   // printf("\n>>total memory(Post_Splitting): %f(KB)",total_hs_memory_in_KB);
   // printf("\n>>total memory (Pre-Cutting + Post_Splitting): %f(KB)",total_memory_in_KB);
   // printf("\n***SUCCESS in building %d-th CutSplit sub-tree(0_sa, 1_da)***\n\n",k);

  }

}


unsigned int get_nbits(unsigned int n)
{
    int		k = 0;

    while (n >>= 1)
        k++;
    return	k;
}
unsigned int get_pow(unsigned int n)
{
    int		k = 1;

    while (n--)
        k*=2;
    return	k;
}

int trie::trieLookup(int* header){
    int index[MAXDIMENSIONS];
    int cdim, cchild, cover, cuts;
    int cnode = root;
    int match = 0;
    int nbits;
    int i,j;
    int flag_hs = 0;
    int match_id = -1;

    for(i = 0; i< MAXDIMENSIONS; i++){ //sip:32 dip:32 sport:16 dport:16 protocol:8
        if(i == 4) index[i] = 8;
        else if(i >= 2) index[i] = 16;
        else index[i] = 32;
    }

    while(nodeSet[cnode].isleaf != 1){  

        nbits = 0;
        cuts = nodeSet[cnode].ncuts; 
        nbits = get_nbits(nodeSet[cnode].ncuts);
        cdim = k; 
        cchild = 0;
        for(i = index[cdim]; i > index[cdim] - nbits; i--){   
            if((header[cdim] & 1<<(i-1)) != 0){
                cchild += (int)get_pow(i-index[cdim]+nbits-1);
            }
        }
        cnode = nodeSet[cnode].child[cchild]; 

        if(cnode == Null) break;
        if(nodeSet[cnode].flag == 2 && nodeSet[cnode].isleaf != 1) {
            flag_hs = 1;
            break;
        }
        index[cdim] -= nbits;  

    }

    if(cnode != Null && flag_hs == 1){
        match_id = LookupHSTree(rule, nodeSet[cnode].rootnode, header);
    }
    if(cnode != Null && flag_hs == 0 && nodeSet[cnode].isleaf == 1){
        for(i = 0; i < nodeSet[cnode].nrules; i++){
            cover = 1;
            for(j = 0; j < MAXDIMENSIONS; j++){   
                if(rule[nodeSet[cnode].ruleid[i]].field[j].low > header[j] ||
                   rule[nodeSet[cnode].ruleid[i]].field[j].high < header[j]){
                    cover = 0;
                    break;
                }
            }
            if(cover == 1){
                match = 1;
                break;
            }
        }
    }
    if(match == 1) {
        match_id = nodeSet[cnode].ruleid[i]; 
    }

    return  match_id;

}

int trie::trieLookupAccess(int* header, ProgramState *program_state) {
    int index[MAXDIMENSIONS];
    int cdim, cchild, cover, cuts;
    int cnode = root;
    int match = 0;
    int nbits;
    int i,j;
    int flag_hs = 0;
    int match_id = -1;

    for(i = 0; i< MAXDIMENSIONS; i++){ //sip:32 dip:32 sport:16 dport:16 protocol:8
        if(i == 4) index[i] = 8;
        else if(i >= 2) index[i] = 16;
        else index[i] = 32;
    }

    program_state->access_nodes.AddNum();
    while(nodeSet[cnode].isleaf != 1){  

        nbits = 0;
        cuts = nodeSet[cnode].ncuts; 
        nbits = get_nbits(nodeSet[cnode].ncuts);
        cdim = k; 
        cchild = 0;
        for(i = index[cdim]; i > index[cdim] - nbits; i--){   
            if((header[cdim] & 1<<(i-1)) != 0){
                cchild += (int)get_pow(i-index[cdim]+nbits-1);
            }
        }
        cnode = nodeSet[cnode].child[cchild]; 
        program_state->access_nodes.AddNum();

        if(cnode == Null) break;
        if(nodeSet[cnode].flag == 2 && nodeSet[cnode].isleaf != 1) {
            flag_hs = 1;
            break;
        }
        index[cdim] -= nbits;  

    }

    if(cnode != Null && flag_hs == 1){
        // printf("find in hs %d \n", cnode);
        match_id = LookupHSTreeAccess(rule, nodeSet[cnode].rootnode, header, program_state);
    }
    if(cnode != Null && flag_hs == 0 && nodeSet[cnode].isleaf == 1){
      // if (k == 0)
      //   printf("cnode %d %d\n", cnode, nodeSet[cnode].nrules);
        for(i = 0; i < nodeSet[cnode].nrules; i++){
            cover = 1;
            program_state->access_rules.AddNum();
            for(j = 0; j < MAXDIMENSIONS; j++){   
                if(rule[nodeSet[cnode].ruleid[i]].field[j].low > header[j] ||
                   rule[nodeSet[cnode].ruleid[i]].field[j].high < header[j]){
                    cover = 0;
                    break;
                }
            }
            if(cover == 1){
                match = 1;
                break;
            }
        }
    }
    if(match == 1) {
        match_id = nodeSet[cnode].ruleid[i]; 
    }

    return  match_id;

}

void trie::Freetrie() {
  FreeNode(root);
  free(nodeSet);
}

void trie::FreeNode(int cnode) {
    if (cnode == Null)
      return;
    // printf("%d\n", cnode);
    if (nodeSet[cnode].flag == 2) {
        FreeHSTree(nodeSet[cnode].rootnode, true);
    } else if (nodeSet[cnode].isleaf == 1) {
        free(nodeSet[cnode].ruleid);
    } else {
        int nbits = get_nbits(nodeSet[cnode].ncuts);
        if (nbits == 0)
          return;
        // printf("nbits %d\n", nbits);
        for (int i = 0; i < (1 << nbits); ++i) {
            int child_cnode = nodeSet[cnode].child[i];
            if (child_cnode != Null)
                FreeNode(child_cnode);
        }
        free(nodeSet[cnode].child);
    } 
    // else {
    //     printf("check cutsplit trie flag %d\n", nodeSet[cnode].flag);
    //     printf("%d %d %d\n", nodeSet[cnode].child != NULL, nodeSet[cnode].ruleid != NULL, nodeSet[cnode].rootnode != NULL);
    //     exit(1);
    // }
}

uint64_t trie::MemorySize() {
  uint64_t memory_size = sizeof(trie);
  memory_size += sizeof(nodeItem) * freelist;
  for (int cnode = 0; cnode < freelist; ++cnode) {
      if (nodeSet[cnode].flag == 2) {
          memory_size += HSMemorySize(nodeSet[cnode].rootnode);
      } else if (nodeSet[cnode].isleaf == 1) {
          memory_size += sizeof(int) * nodeSet[cnode].nrules;
      } else {
          int nbits = get_nbits(nodeSet[cnode].ncuts);
          memory_size += sizeof(int) << nbits;
      } 
  }
  return memory_size;
}

int trie::Height() {
    int max_height = 0;
    for (int cnode = 0; cnode < freelist; ++cnode) { 
        int height = nodeSet[cnode].layNo;
        if (nodeSet[cnode].flag == 2) {
            height += HSHeight(nodeSet[cnode].rootnode);
        } else {

        }
        max_height = max(max_height, height);
    }
    return max_height;
}

int trie::RealHeight() {
    int max_height = 0;
    for (int cnode = 0; cnode < freelist; ++cnode) { 
        int height = nodeSet[cnode].layNo;
        if (nodeSet[cnode].flag == 2) {
            height += HSRealHeight(nodeSet[cnode].rootnode);
        } else if (nodeSet[cnode].isleaf == 1) {
            height += nodeSet[cnode].nrules;
        }
        max_height = max(max_height, height);
    }
    return max_height;
}

void trie::CalNum(ProgramState *program_state) {
  for (int cnode = 0; cnode < freelist; ++cnode) {
      if (nodeSet[cnode].flag == 2) {
        // printf("cnode 2 %d\n", cnode);
          HSCalNum(nodeSet[cnode].rootnode, program_state);
      } else if (nodeSet[cnode].isleaf == 1) {
        // printf("cnode 3 %d\n", cnode);
        // printf("nodeSet[cnode].nrules %d\n", nodeSet[cnode].nrules);
          program_state->tree_rules_num += nodeSet[cnode].nrules;
          program_state->tree_real_rules_num += nodeSet[cnode].nrules;
      } else {
          int nbits = get_nbits(nodeSet[cnode].ncuts);
          if (nbits == 0) {

          } else {
            program_state->tree_child_num += 1 << nbits;
            program_state->tree_real_child_num += 1 << nbits;

          }
      } 
  }
}