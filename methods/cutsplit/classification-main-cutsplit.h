#ifndef  CLASSIFICATIONMAINCUTSPLIT_H
#define  CLASSIFICATIONMAINCUTSPLIT_H


#include "../../elementary.h"

#include "cutsplit/CutSplit.h"
#include "cutsplit-io.h"

using namespace std;

int ClassificationMainCutSplit(CommandStruct command, ProgramState *program_state, vector<Rule*> &rules, 
                          vector<Trace*> &traces, vector<int> &ans);

#endif