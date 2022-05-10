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
#include "TreeBuilder.h"

#include "../Utilities/MapExtensions.h"

#include <limits>
#include <unordered_map>
#include <unordered_set>

using namespace std;

#define MaxDelta 16

SpanRange GetSpan(const BCRule& rule, uint8_t dim, uint8_t left, uint8_t right) { 
	Point lp = rule.range[dim].low;
	Point rp = rule.range[dim].high;
	uint32_t mask = 0xFFFFFFFF >> (left + right);
	uint32_t l = (lp >> right) & mask;
	uint32_t r = (rp >> right) & mask;
	return SpanRange(l, r);
}

tuple<uint8_t, uint8_t, uint8_t, size_t> TreeBuilder::BestSpan(const vector<BCRule>& rules, Allower isAllowed, int penaltyRate) {
	
	size_t bestCost = std::numeric_limits<size_t>::max();
	uint8_t bestDim = 0;
	uint8_t bestNl = 0;
	uint8_t bestNr = 0;

	for (uint8_t delta = BitsPerNybble; delta <= MaxDelta; delta += BitsPerNybble) {
		for (uint8_t dim : allowableDims) {
			for (uint8_t nl = 0; nl + delta <= BitsPerField; nl += BitsPerNybble) {
				uint8_t nr = BitsPerField - nl - delta;
				unordered_map<Point, size_t> counts;
				size_t penalty = 0;
				for (const BCRule& r : rules) {
					if (isAllowed(r, dim, nl, nr)) {
						SpanRange span = GetSpan(r, dim, nl, nr);
						for (Point i = span.first; i <= span.second; i++) {
							counts[i]++;
						}
					} else {
						penalty += 1;
					}
				}
				size_t fitness = 0;
				for (auto pair : counts) {
					if (pair.second > fitness) fitness = pair.second;
				}
				fitness += penalty * penaltyRate;
		
				if (fitness < bestCost) {
					bestCost = fitness;
					bestDim = dim;
					bestNl = nl;
					bestNr = nr;
				}
			}
		}
	}
	
	return tuple<uint8_t, uint8_t, uint8_t, size_t>(bestDim, bestNl, bestNr, bestCost);
}

tuple<uint8_t, uint8_t, uint8_t, size_t> TreeBuilder::BestSpanMinPart(const vector<BCRule>& rules, Allower isAllowed, int penaltyRate) {
	
	size_t bestCost = std::numeric_limits<size_t>::max();
	size_t bestPart = std::numeric_limits<size_t>::max();
	uint8_t bestDim = 0;
	uint8_t bestNl = 0;
	uint8_t bestNr = 0;

	for (uint8_t delta = BitsPerNybble; delta <= MaxDelta; delta += BitsPerNybble) {
		for (uint8_t dim : allowableDims) {
			for (uint8_t nl = 0; nl + delta <= BitsPerField; nl += BitsPerNybble) {
				uint8_t nr = BitsPerField - nl - delta;
				unordered_map<Point, size_t> counts;
				size_t penalty = 0;
				for (const BCRule& r : rules) {
					if (isAllowed(r, dim, nl, nr)) {
						SpanRange span = GetSpan(r, dim, nl, nr);
						for (Point i = span.first; i <= span.second; i++) {
							counts[i]++;
						}
					} else {
						penalty += 1;
					}
				}
				size_t fitness = 0;
				for (auto pair : counts) {
					if (pair.second > fitness) fitness = pair.second;
				}
				size_t cost = fitness + penalty * penaltyRate;
		
				if ((fitness > 0 && fitness < bestPart) || (fitness == bestPart && cost < bestCost)) {
					bestCost = cost;
					bestPart = fitness;
					bestDim = dim;
					bestNl = nl;
					bestNr = nr;
				}
			}
		}
	}

	// printf("Chosen: %lu\n", bestPart);
	return tuple<uint8_t, uint8_t, uint8_t, size_t>(bestDim, bestNl, bestNr, bestCost);
}

tuple<uint8_t, uint8_t, uint8_t, size_t> TreeBuilder::BestSpanMinPenalty(const vector<BCRule>& rules, Allower isAllowed, int penaltyRate) {
	
	size_t bestCost = std::numeric_limits<size_t>::max();
	size_t bestPenalty = std::numeric_limits<size_t>::max();
	uint8_t bestDim = 0;
	uint8_t bestNl = 0;
	uint8_t bestNr = 0;

	for (uint8_t delta = BitsPerNybble; delta <= MaxDelta; delta += BitsPerNybble) {
		for (uint8_t dim : allowableDims) {
			for (uint8_t nl = 0; nl + delta <= BitsPerField; nl += BitsPerNybble) {
				uint8_t nr = BitsPerField - nl - delta;
				unordered_map<Point, size_t> counts;
				size_t penalty = 0;
				for (const BCRule& r : rules) {
					if (isAllowed(r, dim, nl, nr)) {
						SpanRange span = GetSpan(r, dim, nl, nr);
						for (Point i = span.first; i <= span.second; i++) {
							counts[i]++;
						}
					} else {
						penalty += 1;
					}
				}
				size_t fitness = 0;
				for (auto pair : counts) {
					if (pair.second > fitness) fitness = pair.second;
				}
				size_t cost = fitness + penalty * penaltyRate;
		
				if (penalty < bestPenalty || (penalty == bestPenalty && cost < bestCost)) {
					bestCost = cost;
					bestPenalty = penalty;
					bestDim = dim;
					bestNl = nl;
					bestNr = nr;
				}
			}
		}
	}
	
	return tuple<uint8_t, uint8_t, uint8_t, size_t>(bestDim, bestNl, bestNr, bestCost);
}

tuple<uint8_t, uint16_t, size_t> TreeBuilder::BestSplit(const vector<BCRule>& rules) {
	size_t bestCost = numeric_limits<size_t>::max();
	uint8_t bestDim = 0;
	uint16_t bestSplit = 0;
	
	for (uint8_t dim : splitDims) {
		set<Point> splits;
		for (const BCRule& r : rules) {
			splits.insert(r.range[dim].high);
		}
		for (Point s : splits) {
			size_t lc = 0, rc = 0;
			for (const BCRule& r : rules) {
				if (r.range[dim].low <= s) {
					lc++;
				}
				if (r.range[dim].high > s) {
					rc++;
				}
			}
			size_t cost = max(lc, rc);
			if (cost < bestCost) {
				bestCost = cost;
				bestDim = dim;
				bestSplit = s;
			}
		}
	}
	return tuple<uint8_t, uint16_t, size_t>(bestDim, bestSplit, bestCost);
}

bool AllowAll(const BCRule& r, uint8_t dim, uint8_t nl, uint8_t nr) {
	return true;
}

bool BlockSplit(const BCRule& r, uint8_t dim, uint8_t nl, uint8_t nr) {
	SpanRange s = GetSpan(r, dim, nl, nr);
	return s.first == s.second;
}

bool LimitedSplit(const BCRule& r, uint8_t dim, uint8_t nl, uint8_t nr) {
	SpanRange s = GetSpan(r, dim, nl, nr);
	return s.first + 16 >= s.second;
}

void CleanRules(vector<BCRule>& rules) {
	SortRules(rules);
	rules.erase(unique(rules.begin(), rules.end(), [](const BCRule& r1, const BCRule& r2) { return r1.priority == r2.priority;} ), rules.end());
}

ByteCutsNode* TreeBuilder::BuildNode(const vector<BCRule>& rules, vector<BCRule>& remain, int depth, int penaltyRate) {
	return BuildNodeHelper(rules, remain, depth, LimitedSplit, [&](const vector<BCRule>& rl, vector<BCRule>& rmn, int dep, int pr) { return BuildNode(rl, rmn, dep, pr); }, penaltyRate);
}

ByteCutsNode* TreeBuilder::BuildPrimaryRoot(const vector<BCRule>& rules, vector<BCRule>& remain) {
	numNodes = 0;
	return BuildNode(rules, remain, 0, 5);
}

ByteCutsNode* TreeBuilder::BuildSecondaryRoot(const vector<BCRule>& rules, vector<BCRule>& remain) {
	numNodes = 0;
	auto node = BuildRootHelper(rules, remain, 0, LimitedSplit, [&](const vector<BCRule>& rl, vector<BCRule>& rmn, int depth, int pr) { return BuildNode(rl, rmn, depth, pr); }, 1);

	CleanRules(remain);
	
	return node;
}

ByteCutsNode* TreeBuilder::BuildCutNode(const vector<BCRule>& rules, vector<BCRule>& remain, int depth, Allower isAllowed, Builder builder, int penaltyRate, uint8_t d, uint8_t nl, uint8_t nr) {
	vector<BCRule> inrules;
	for (const BCRule& rule : rules) {
		if (isAllowed(rule, d, nl, nr)) {
			inrules.push_back(rule);
		} else {
			remain.push_back(rule);
		}
	}
	
	if (inrules.empty()) {
		// Shouldn't happen
		ByteCutsNode* node = new ByteCutsNode();
		ByteCutsNode::LeafNode(*node, rules);
		remain.clear();
		PrintRules(rules);
		return node;
	}

	size_t numChildren = 0x1 << (BitsPerField - nl - nr);
	unordered_map<vector<bool>, ByteCutsNode*> composer;
	ByteCutsNode** children = new ByteCutsNode*[numChildren];
	for (size_t i = 0; i < numChildren; i++) {
		vector<bool> brl(inrules.size(), false);
		for (size_t j = 0; j < inrules.size(); j++) {
			const BCRule& rule = inrules[j];
			auto s = GetSpan(rule, d, nl, nr);
			if (s.first <= i && i <= s.second) {
				brl[j] = true;
			}
		}
		if (!composer.count(brl)) {
			vector<BCRule> rl;
			for (size_t j = 0; j < inrules.size(); j++) {
				if (brl[j]){
					rl.push_back(inrules[j]);
				}
			}
			auto node = builder(rl, remain, depth + 1, penaltyRate);
			composer[brl] = node;
		}
		children[i] = composer[brl];
	}
	ByteCutsNode* node = new ByteCutsNode();
	ByteCutsNode::CutNode(*node, d, nl, nr, children);
	return node;
}

ByteCutsNode* TreeBuilder::BuildRootHelper(const vector<BCRule>& rules, vector<BCRule>& remain, int depth, Allower isAllowed, Builder builder, int penaltyRate) {
	numNodes++;

	if (rules.size() <= leafSize) {
		ByteCutsNode* node = new ByteCutsNode();
		ByteCutsNode::LeafNode(*node, rules);
		return node;
	} else {
		vector<BCRule> remainCost, remainPart, remainPenalty;
		uint8_t d, nl, nr;
		size_t c;
		tie(d, nl, nr, c) = BestSpan(rules, isAllowed, penaltyRate);
		ByteCutsNode* costNode = BuildCutNode(rules, remainCost, depth, isAllowed, builder, penaltyRate, d, nl, nr);
		tie(d, nl, nr, c) = BestSpanMinPart(rules, isAllowed, penaltyRate);
		ByteCutsNode* partNode = BuildCutNode(rules, remainPart, depth, isAllowed, builder, penaltyRate, d, nl, nr);
		tie(d, nl, nr, c) = BestSpanMinPenalty(rules, isAllowed, penaltyRate);
		ByteCutsNode* penaltyNode = BuildCutNode(rules, remainPenalty, depth, isAllowed, builder, penaltyRate, d, nl, nr);
		
		size_t minRemain = min({remainCost.size(), remainPart.size(), remainPenalty.size()});
		if (remainCost.size() == minRemain) {
			remain = remainCost;
			delete partNode;
			delete penaltyNode;
			return costNode;
		} else if (remainPart.size() == minRemain) {
			remain = remainPart;
			delete costNode;
			delete penaltyNode;
			return partNode;
		} else {
			remain = remainPenalty;
			delete costNode;
			delete partNode;
			return penaltyNode;
		}
	}
}

ByteCutsNode* TreeBuilder::BuildNodeHelper(const vector<BCRule>& rules, vector<BCRule>& remain, int depth, Allower isAllowed, Builder builder, int penaltyRate) {
	numNodes++;

	if (rules.size() <= leafSize) {
		ByteCutsNode* node = new ByteCutsNode();
		ByteCutsNode::LeafNode(*node, rules);
		return node;
	} else {
		uint8_t d, nl, nr;
		size_t c;
		tie(d, nl, nr, c) = BestSpan(rules, isAllowed, penaltyRate);
		
		
		uint8_t ds;
		uint16_t ss;
		size_t cs;
		tie(ds, ss, cs) = BestSplit(rules);
		size_t cMin = min(c, cs);
		
		if (cMin == rules.size()) {
			// degenerate case: no improvement
			ByteCutsNode* node = new ByteCutsNode();
			ByteCutsNode::LeafNode(*node, rules);
			return node;
		} else if (c == cMin) {
			return BuildCutNode(rules, remain, depth, isAllowed, builder, penaltyRate, d, nl, nr);
		} else {
			madeHyperSplit = true;
			vector<BCRule> lefts;
			vector<BCRule> rights;
			for (const BCRule& r : rules) {
				if (r.range[ds].low <= ss) lefts.push_back(r);
				if (r.range[ds].high > ss) rights.push_back(r);
			}
			ByteCutsNode* lc = builder(lefts, remain, depth + 1, penaltyRate);
			ByteCutsNode* rc = builder(rights, remain, depth + 1, penaltyRate);
			ByteCutsNode* node = new ByteCutsNode();
			ByteCutsNode::SplitNode(*node, ds, ss, lc, rc);
			return node;
		}
	}
}
