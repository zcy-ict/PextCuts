/*
 * MIT License
 *
 * Copyright (c) 2017 by J. Daly at Michigan State University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "ByteCutsNode.h"

#include "../Utilities/MapExtensions.h"

#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#define MaxDelta 16

//************
// ByteCutsNode
//************

void ByteCutsNode::LeafNode(ByteCutsNode& self, const vector<BCRule>& rules) {
	self.mode = Leaf;
	self.numRules = rules.size();
	self.rules = new BCRule[rules.size()];
	for (size_t i = 0; i < rules.size(); i++) {
		self.rules[i] = rules[i];
	}
	
}
void ByteCutsNode::SplitNode(ByteCutsNode& self, uint8_t dim, uint16_t point, ByteCutsNode* left, ByteCutsNode* right) {
	self.mode = Split;
	self.dim = dim;
	self.splitPoint = point;
	self.children = new ByteCutsNode*[2];
	self.children[0] = left;
	self.children[1] = right;
}

void ByteCutsNode::CutNode(ByteCutsNode& self, uint8_t dim, uint8_t left, uint8_t right, ByteCutsNode** children) {
	self.mode = Cut;
	self.dim = dim;
	self.cutInfo.cutLow = right;
	self.cutInfo.cutTotal = left + right;
	self.children = children;
	self.pext = (0xFFFFFFFFu >> (self.cutInfo.cutTotal)) << right;
}

ByteCutsNode::ByteCutsNode() {
	// TODO
}

ByteCutsNode::~ByteCutsNode() {
	if (mode == Leaf) {
		delete [] rules;
	} else if (mode == Cut) {
		size_t numChildren = NumChildren();
		
		unordered_set<ByteCutsNode*> uniqueChildren;
		for (size_t i = 0; i < numChildren; i++) {
			uniqueChildren.insert(children[i]);
		}
		for (auto c : uniqueChildren) {
			delete c;
		}
		delete [] children;
	} else {
		delete children[0];
		delete children[1];
		delete [] children;
	}
}

int ByteCutsNode::Lookup(const BCPacket& p) const {
	switch (mode) {
		case Cut:
			return children[IndexPacket(p)]->Lookup(p);
		case Split:
			if (p[dim] <= splitPoint) {
				return children[0]->Lookup(p);
			} else {
				return children[1]->Lookup(p);
			}
		case Leaf:
			for (uint16_t i = 0; i < numRules; i++) {
				if (rules[i].MatchesPacket(p)) {
					return rules[i].priority;
				}
			}
			return -1;
			break;
		default:
			printf("no mode\n");
			exit(1);
	}
}

int ByteCutsNode::LookupAccess(const BCPacket& p, ProgramState *program_state) const {
	program_state->access_nodes.AddNum();
	switch (mode) {
		case Cut:
			return children[IndexPacket(p)]->LookupAccess(p, program_state);
		case Split:
			if (p[dim] <= splitPoint) {
				return children[0]->LookupAccess(p, program_state);
			} else {
				return children[1]->LookupAccess(p, program_state);
			}
		case Leaf:
			// if (numRules > 0) {
			// 	printf("numRules %d\n", numRules);
			// 	exit(1);
			// }
			for (uint16_t i = 0; i < numRules; i++) {
				program_state->access_rules.AddNum();
				if (rules[i].MatchesPacket(p)) {
					return rules[i].priority;
				}
			}
			return -1;
			break;
	}
	return 0;
}

int ByteCutsNode::Size(int ruleSize) const {
	int result = NodeSize;
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					result += child->Size(ruleSize);
				}
			}
			break;
		case Split:
			result += children[0]->Size(ruleSize);
			result += children[1]->Size(ruleSize);
			break;
		case Leaf:
			result += numRules * ruleSize;
			break;
	}
	return result;
}

int ByteCutsNode::Height() const {
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				int maxHeight = 0;
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					maxHeight = max(maxHeight, child->Height());
				}
				return maxHeight + 1;
			}
			break;
		case Split:
			return max(children[0]->Height(), children[1]->Height()) + 1;
			break;
		case Leaf:
			return 1;
			break;
	}
	return 0;
}

int ByteCutsNode::Cost() const {
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				int maxHeight = 0;
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					maxHeight = max(maxHeight, child->Cost());
				}
				return maxHeight + 1;
			}
			break;
		case Split:
			return max(children[0]->Height(), children[1]->Cost()) + 1;
			break;
		case Leaf:
			// printf("Leaf\n");
			return numRules;
			break;
	}
	return 0;
}

size_t ByteCutsNode::IndexPacket(const BCPacket& p) const {
	Point pt = p[dim];
	Point mask = 0xFFFFFFFFu >> (cutInfo.cutTotal);
	return (pt >> cutInfo.cutLow) & mask;
}

void ByteCutsNode::CalculateState(ProgramState *program_state, int layer, bool diff) {
	// if (mode == 2)
	// 	printf("numRules %d\n", numRules);
	DecisionTreeInfo *info = NULL;
	switch (mode) {
		case Cut: {
				if (dim == 0 || dim == 1)
					info = &program_state->layers[layer].info[0];
				else if (dim == 2 || dim == 3)
					info = &program_state->layers[layer].info[1];
				else if (dim == 4)
					info = &program_state->layers[layer].info[2];
				else 
					printf("ByteCutsNode CalculateState wrong Cut\n");

				size_t numChildren = NumChildren();
				info->node_num += 1;
				info->children_num += numChildren;
				program_state->tree_child_num += numChildren;
				if (diff) {
					info->diff_node_num += 1;
					info->diff_children_num += numChildren;
					program_state->tree_real_child_num += numChildren;
				}
				map<ByteCutsNode*, int> match;
				for (int i = 0; i < numChildren; ++i) {
					if (match.find(children[i]) == match.end()) {
						children[i]->CalculateState(program_state, layer + 1, diff);
						match[children[i]] = 1;
						// if (diff)
						// 	info->diff_children_num += 1;
					} else {
						children[i]->CalculateState(program_state, layer + 1, false);
						// info->children_num += 1;
					}
				}
			}
			break;
		case Split:{
				if (dim == 0 || dim == 1)
					info = &program_state->layers[layer].info[3];
				else if (dim == 2 || dim == 3)
					info = &program_state->layers[layer].info[4];
				else if (dim == 4)
					info = &program_state->layers[layer].info[5];
				else 
					printf("ByteCutsNode CalculateState wrong Split\n");

				info->node_num += 1;
				info->children_num += 2;
				program_state->tree_child_num += 2;
				if (diff) {
					info->diff_node_num += 1;
					info->diff_children_num += 2;
					program_state->tree_real_child_num += 2;
				}
				// printf("Split\n");
				children[0]->CalculateState(program_state, layer + 1, diff);
				children[1]->CalculateState(program_state, layer + 1, diff);
			}
			break;
		case Leaf: {
			info = &program_state->layers[layer].info[6];
				info->node_num += 1;
				info->children_num += numRules;
				program_state->tree_rules_num += numRules;
				if (diff && numRules > 0) {
					program_state->tree_real_rules_num += numRules;
					info->diff_node_num += 1;
					info->diff_children_num += numRules;
				}
			}
			break;
	}
}

uint64_t ByteCutsNode::MemorySize() {
	uint64_t memory_size = sizeof(ByteCutsNode);
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				memory_size += sizeof(ByteCutsNode*) * numChildren;
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					memory_size += child->MemorySize();
				}
			}
			break;
		case Split:
			{
				memory_size += sizeof(ByteCutsNode*) * 2;
				memory_size += children[0]->MemorySize();
				memory_size += children[1]->MemorySize();
			}
			break;
		case Leaf:
			{
				memory_size += numRules * sizeof(BCRule);
			}
			break;
	}
	return memory_size;
	
}



int ByteCutsNode::Free(bool free_self) {
	switch (mode) {
		case Cut:
			{
				size_t numChildren = NumChildren();
				unordered_set<ByteCutsNode*> uchildren(children, children + numChildren);
				for (ByteCutsNode* child : uchildren) {
					child->Free(true);
				}
				free(children);
			}
			break;
		case Split:
			{
				children[0]->Free(true);
				children[1]->Free(true);
				free(children);
			}
			break;
		case Leaf:
			{
				free(rules);
			}
			break;
	}
	// printf("%016lx\n", this);
	if (free_self)
		free(this);
	return 0;
	
}