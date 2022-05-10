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
#ifndef ByteCutsNode_H
#define ByteCutsNode_H

#include "../../../elementary.h"
#include "../Common.h"

#include <functional>
#include <unordered_map>
#include <utility>

#define NybblesPerField 8
#define BitsPerNybble 4
#define NybbleMask 0xF
#define BytesPerField 4
#define BitsPerByte 8
#define ByteMask 0xFF
#define MaxCuts 16
#define MaxByteCuts 256
#define NybMin 0
#define NybMax 15
#define ByteMin 0
#define ByteMax 255

#define NodeSize 8

#define BitsPerField 32

typedef std::pair<uint32_t, uint32_t> SpanRange;

struct CutInfo {
	uint8_t cutLow;
	uint8_t cutTotal;
};

struct ByteCutsNodeLayerInfo {
	int cut_type[3];
	int rules_num;
	int children_num;
};

struct ByteCutsNodeInfo {
	ByteCutsNodeLayerInfo layers[30];
};

class ByteCutsNode {
public:
	enum BCMode : uint8_t {
		Cut,
		Split,
		Leaf
	};

	static void LeafNode(ByteCutsNode& self, const std::vector<BCRule>& rules);
	static void SplitNode(ByteCutsNode& self, uint8_t dim, uint16_t point, ByteCutsNode* left, ByteCutsNode* right);
	static void CutNode(ByteCutsNode& self, uint8_t dim, uint8_t left, uint8_t right, ByteCutsNode** children);
	
	ByteCutsNode();
	~ByteCutsNode();

	int Lookup(const BCPacket& p) const;
	int LookupAccess(const BCPacket& p, ProgramState *program_state) const;
	int Size(int ruleSize) const;
	bool IsEmpty() const { return false; }
	size_t NumChildren() const { return 0x1u << (BitsPerField - cutInfo.cutTotal); }

	size_t IndexPacket(const BCPacket& p) const;
	
	int Height() const;
	int Cost() const;

	void CalculateState(ProgramState *program_state, int layer, bool diff);
// private:
	uint8_t CutLow() const {
		return cutInfo.cutLow;
	}
	uint8_t CutHigh() const {
		return cutInfo.cutTotal - cutInfo.cutLow;
	}
	uint64_t MemorySize();
	int Free(bool free_self);

	BCMode mode;
	union {
		uint8_t dim;
		uint8_t numRules;
	};
	union {
		CutInfo cutInfo;
		uint16_t splitPoint;
	};
	uint32_t pext;
	union {
		ByteCutsNode** children;
		BCRule* rules;
	};
	
};
#endif
