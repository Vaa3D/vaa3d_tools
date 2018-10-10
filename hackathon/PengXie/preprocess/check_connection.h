#ifndef CHECK_CONNECTION_H
#define CHECK_CONNECTION_H

// Get regions near connections and evaluate their scores.

#include "crop_swc_main.h"
#include "calculate_reliability_score_main.h"
using namespace std;

bool check_connection_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
void printHelp_check_connection();

#endif // CHECK_CONNECTION_H
