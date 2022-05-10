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
#pragma once
#ifndef TreeBuilder_H
#define TreeBuilder_H

#include "ByteCutsNode.h"

SpanRange GetSpan(const BCRule& rule, uint8_t dim, uint8_t left, uint8_t right);
void CleanRules(std::vector<BCRule>& rules);

class TreeBuilder {
public:
	typedef std::function<bool(const BCRule&, uint8_t dim, uint8_t nl, uint8_t nr)> Allower;
	typedef std::function<ByteCutsNode*(const std::vector<BCRule>&, std::vector<BCRule>&, int, int)> Builder;

	TreeBuilder(size_t leafSize) : leafSize(leafSize) {
		allowableDims = {0, 1, 4};
		splitDims = {2, 3};
	}

	std::tuple<uint8_t, uint8_t, uint8_t, size_t> BestSpan(const std::vector<BCRule>& rules, Allower isAllowed, int penaltyRate);
	std::tuple<uint8_t, uint8_t, uint8_t, size_t> BestSpanMinPart(const std::vector<BCRule>& rules, Allower isAllowed, int penaltyRate);
	std::tuple<uint8_t, uint8_t, uint8_t, size_t> BestSpanMinPenalty(const std::vector<BCRule>& rules, Allower isAllowed, int penaltyRate);
	std::tuple<uint8_t, uint16_t, size_t> BestSplit(const std::vector<BCRule>& rules);
	
	ByteCutsNode* BuildNode(const std::vector<BCRule>& rules, std::vector<BCRule>& remain, int depth, int penaltyRate);
	ByteCutsNode* BuildPrimaryRoot(const std::vector<BCRule>& rules, std::vector<BCRule>& remain);
	ByteCutsNode* BuildSecondaryRoot(const std::vector<BCRule>& rules, std::vector<BCRule>& remain);
	
	bool BuiltSplit() const { return madeHyperSplit; }
private:
	ByteCutsNode* BuildNodeHelper(
			const std::vector<BCRule>& rules, 
			std::vector<BCRule>& remain, 
			int depth,
			Allower isAllowed, 
			Builder builder,
			int penaltyRate);
	ByteCutsNode* BuildRootHelper(
			const std::vector<BCRule>& rules, 
			std::vector<BCRule>& remain, 
			int depth,
			Allower isAllowed, 
			Builder builder,
			int penaltyRate);
	ByteCutsNode* BuildCutNode(
			const std::vector<BCRule>& rules, 
			std::vector<BCRule>& remain, 
			int depth,
			Allower isAllowed, 
			Builder builder,
			int penaltyRate,
			uint8_t d, uint8_t nl, uint8_t nr);

	std::vector<uint8_t> allowableDims;
	std::vector<uint8_t> splitDims;
	size_t leafSize;
	size_t numNodes = 0;
	
	bool madeHyperSplit = false;
};

#endif
