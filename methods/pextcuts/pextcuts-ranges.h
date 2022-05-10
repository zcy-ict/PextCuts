#ifndef  PEXTCUTSRANGES_H
#define  PEXTCUTSRANGES_H

#include "../../elementary.h"
#include "../../io/io.h"
#include "../dynamictuple/dynamictuple-ranges.h"

#include <cmath>

using namespace std;

struct PcDtInfo : public DtInfo {
	void PextCalculateXY(int x1, int y1);
	void PextCalculate();

};

vector<TupleRange> PextTupleRanges(vector<Rule*> &rules, double &cal_time);

#endif