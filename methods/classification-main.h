#ifndef  CLASSIFICATIONMAIN_H
#define  CLASSIFICATIONMAIN_H

#include "../elementary.h"
#include "../io/io.h"
#include "classification-main-zcy.h"
#include "partitionsort/classification-main-ps.h"
#include "bytecuts/classification-main-bytecuts.h"
#include "cutsplit/classification-main-cutsplit.h"

using namespace std;

int ClassificationMain(CommandStruct command);

#endif