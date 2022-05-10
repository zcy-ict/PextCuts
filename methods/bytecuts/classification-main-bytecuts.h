#ifndef  CLASSIFICATIONMAINBYTECUTS_H
#define  CLASSIFICATIONMAINBYTECUTS_H

#include "Common.h"
#include "bytecuts-io.h"
#include "ByteCuts/ByteCuts.h"
#include "Utilities/MapExtensions.h"
#include "Utilities/VectorExtensions.h"
#include "../../elementary.h"

#include <vector>
#include <queue>
#include <list>
#include <set>
#include <iostream>
#include <algorithm>
#include <random>
#include <numeric>
#include <memory>
#include <chrono> 
#include <array>

using namespace std;

int ClassificationMainByteCuts(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans);

#endif