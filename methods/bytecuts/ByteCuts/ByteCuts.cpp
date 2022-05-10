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
#include "ByteCuts.h"

#include "../Utilities/MapExtensions.h"

#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#define MaxDelta 16

ByteCutsClassifier::ByteCutsClassifier(const vector<BCRule>& rules, const vector<ByteCutsNode*>& trees, const vector<int>& priorities, const vector<size_t>& sizes) : 
		rules(rules), trees(trees), priorities(priorities), sizes(sizes) {
}

ByteCutsClassifier::ByteCutsClassifier(const unordered_map<string, string>& args) 
	: dredgeFraction(GetDoubleOrElse(args, "BC.BadFraction", 0.02)),
	turningPoint(GetDoubleOrElse(args, "BC.TurningPoint", 0.01)),
	minFrac(GetDoubleOrElse(args, "BC.MinFraction", 0.75)) {
}

ByteCutsClassifier::~ByteCutsClassifier() { 
	for (ByteCutsNode* n : trees) {
		delete n;
	}
}

vector<BCRule> ByteCutsClassifier::Separate(const vector<BCRule>& rules, vector<BCRule>& remain) {
	int bestDim = -1;
	uint8_t bestLen = 0;
	size_t bestCost = numeric_limits<size_t>::max();
	size_t bestPart = numeric_limits<size_t>::max();
	size_t bestRemain = numeric_limits<size_t>::max();
	
	
	vector<int> dims = {FieldSA, FieldDA};
	for (int d : dims) {
		for (uint8_t len = BitsPerNybble; len <= BitsPerField; len += BitsPerNybble) {
			unordered_map<Point, size_t> counts;
			size_t dropped = 0;
			
			for (const BCRule& r : rules) {
				if (r.prefix_length[d] >= len) {
					Point x = r.range[d].low & (0xFFFFFFFF << (BitsPerField - len));
					counts[x]++;
				} else {
					dropped++;
				}
			}
			size_t maxPart = 0;
			for (auto pair : counts) {
				maxPart = max(maxPart, pair.second);
			}
			
			size_t cost = dropped + maxPart;;
			size_t kept = rules.size() - dropped;
			double ratioIn = maxPart * 1.0 / kept;
			double ratioOut = dropped * 1.0 / rules.size();
			bool betterPartition = maxPart < bestPart;
			bool betterRemain = dropped < bestRemain;
			bool goodPartition = maxPart <= bestPart;
			bool goodRemain = dropped <= bestRemain;
			
			bool better;
			if (goodPartition && goodRemain) {
				better = true;
			} else if (!goodPartition && !goodRemain) {
				better = false;
			} else {
				if (ratioIn < turningPoint) {
					better = betterRemain || (goodRemain && betterPartition);
				} else if (ratioOut < (1 - minFrac)) {
					better = betterPartition || (goodPartition && betterRemain);;
				} else {
					better = cost < bestCost;
				}
			}

			if (better) {
				bestDim = d;
				bestLen = len;
				bestCost = cost;
				bestPart = maxPart;
				bestRemain = dropped;
			}
		}
	}
	vector<BCRule> results;
	for (const BCRule& r : rules) {
		if (r.prefix_length[bestDim] >= bestLen) {
			results.push_back(r);
		} else {
			remain.push_back(r);
		}
	}
	
	return results;
}

void ByteCutsClassifier::ConstructClassifier(const std::vector<BCRule>& rules) {
	this->rules = rules;
	SortRules(this->rules);
	
	vector<BCRule> rl = this->rules;
	vector<vector<BCRule>> parts;
	
	while (rl.size() > rules.size() * dredgeFraction) {
		vector<BCRule> remain;
		parts.push_back(Separate(rl, remain));
		if (remain.size() == rl.size()) break;
		rl = remain;
	}
	
	for (vector<BCRule>& part : parts) {
		BuildTree(part);
	}
	BuildBadTree(rl);
}

void ByteCutsClassifier::BuildTree(const vector<BCRule>& rules) {
	vector<BCRule> rl = rules;
	while (!rl.empty()) {
		goodTrees++;
		TreeBuilder bc(8);
		vector<BCRule> remain;
		trees.push_back(bc.BuildPrimaryRoot(rl, remain));
		int priority = max_element(rl.begin(), rl.end(), [](auto rx, auto ry) { return rx.priority < ry.priority; })->priority;
		priorities.push_back(priority);
		sizes.push_back(rl.size());
		rl = remain;
		
	}
}

void ByteCutsClassifier::BuildBadTree(const vector<BCRule>& rules) {
	vector<BCRule> rl = rules;
	while (!rl.empty()) {
		badTrees++;
		TreeBuilder bc(8);
		vector<BCRule> remain;
		trees.push_back(bc.BuildSecondaryRoot(rl, remain));
		int priority = max_element(rl.begin(), rl.end(), [](auto rx, auto ry) { return rx.priority < ry.priority; })->priority;
		priorities.push_back(priority);
		sizes.push_back(rl.size());
		rl = remain;
	}
}

int ByteCutsClassifier::Lookup(const BCPacket& packet) {
	int result = -1;
	for (size_t i = 0; i < trees.size(); i++) {
		if (priorities[i] > result) {
			ByteCutsNode* tree = trees[i];
			result = max(result, tree->Lookup(packet));
		}
	}
	return result;
}

int ByteCutsClassifier::LookupAccess(const BCPacket& packet, ProgramState *program_state) {
    program_state->AccessClear();
	int result = -1;
	for (size_t i = 0; i < trees.size(); i++) {
		if (priorities[i] > result) {
			program_state->access_tuples.AddNum();
			ByteCutsNode* tree = trees[i];
			result = max(result, tree->LookupAccess(packet, program_state));
		}
	}
    program_state->AccessCal();
	return result;
}

bool ByteCutsClassifier::IsWideAddress(BCInterval s) const {
	return (s.low + 0xFFFF) < s.high;
}

int ByteCutsClassifier::CalculateState(ProgramState *program_state) {
	int tuples_num = trees.size();
	program_state->tuples_num = tuples_num;
	program_state->tuples_sum = tuples_num;
	for (int i = 0; i < tuples_num; ++i) {
		trees[i]->CalculateState(program_state, 0, true);
		program_state->tree_height_num.push_back(trees[i]->Height());
		program_state->tree_real_height_num.push_back(trees[i]->Cost());
	}
	return 0;
}

uint64_t ByteCutsClassifier::MemorySize() {
	uint64_t memory_size = sizeof(ByteCutsClassifier);
	// memory_size += sizeof(BCRule) * rules.size();
	memory_size += sizeof(ByteCutsNode*) * trees.size();
	memory_size += sizeof(int) * priorities.size();
	memory_size += sizeof(size_t) * sizes.size();
	// printf("rules %ld trees %ld priorities %ld sizes %ld\n", rules.size(), trees.size(), priorities.size(), sizes.size());
	// printf("ByteCutsNode %ld BCRule %ld\n", sizeof(ByteCutsNode), sizeof(BCRule)); // 16 68
	for (ByteCutsNode* t : trees) {
		memory_size += t->MemorySize();
	}
	return memory_size;
}

int ByteCutsClassifier::Free(bool free_self) {
	int trees_num = trees.size();
	for (int i = 0; i < trees_num; ++i) {
		trees[i]->Free(true);
		trees[i] = NULL;
	}
	if (free_self)
		free(this);
	return 0;
}