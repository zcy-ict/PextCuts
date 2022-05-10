#ifndef  BYTECUTSIO_H
#define  BYTECUTSIO_H

#include "../../elementary.h"
#include "Common.h"

using namespace std;

vector<BCRule> GenerateByteCutsRules(vector<Rule*> &rules);
vector<BCPacket> GenerateByteCutsPackets(vector<Trace*> &traces);
#endif