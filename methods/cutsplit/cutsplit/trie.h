#ifndef  _TRIE_H
#define  _TRIE_H

#include<stdio.h>
#include<stdlib.h>
#include<queue>
#include "CutSplit.h"
using namespace std;

class trie {
	struct nodeItem {
                bool	isleaf;   
                int	nrules; 
                range	field[MAXDIMENSIONS];  
                int	*ruleid;    
  	        unsigned int	ncuts;
		hs_node_t* rootnode;
                int*	child;
                int	layNo;
                int	flag;     //Cutting or Splitting
                };
public:

  	int	binth;                 
	int	pass;			// max trie level
        int	k;                      //dim. of small      
        int	freelist;		// first nodeItem on free list
        unsigned int	threshold;

	int	Total_Rule_Size;	// number of rules stored
	int	Total_Array_Size;
	int	Leaf_Node_Count;
	int	NonLeaf_Node_Count;
	float	total_ficuts_memory_in_KB;       
	float	total_hs_memory_in_KB;
	float	total_memory_in_KB;


	int	max_depth;
	int	numrules;
	pc_rule	*rule;  
	int	root;		// root of trie
	nodeItem *nodeSet;	// base of array of NodeItems

public:
       queue<int> qNode;	//queue for node
       trie(int, int, pc_rule*, pc_rule*,int, int);
       trie() {};
	   ~trie();
       int  count_np_ficut(nodeItem*);
       void createtrie();
	   int trieLookup(int*);
     int trieLookupAccess(int* header, ProgramState *program_state);

     void Freetrie();
     void FreeNode(int cnode);
     uint64_t MemorySize();

     int Height();
     int RealHeight();
     void CalNum(ProgramState *program_state);
};

#endif 
