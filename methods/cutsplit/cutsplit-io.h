#ifndef  CUTSPLITIO_H
#define  CUTSPLITIO_H

#include "../../elementary.h"
#include "cutsplit/CutSplit.h"

using namespace std;

pc_rule* GenerateCutSplitRules(vector<Rule*> &rules);
uint32_t** GenerateCutSplitPackets(vector<Trace*> &traces);

void FreeCutSplitRules(pc_rule* rules);
void FreeCutSplitPackets(uint32_t** packets, int packets_num);

#endif