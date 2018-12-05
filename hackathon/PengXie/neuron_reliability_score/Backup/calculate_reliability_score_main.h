#ifndef CALCULATE_RELIABILITY_SCORE_MAIN_H
#define CALCULATE_RELIABILITY_SCORE_MAIN_H

#include "QtGui"
#include "v3d_interface.h"
#include "v3d_message.h"
#include "vector"
//#include "src/topology_analysis.h"
#include "src/my_surf_objs.h"
#include "fstream"
#include "neuron_format_converter.h"

void doCalculateScore(V3DPluginCallback2 &callback, QString fname_img, QString fname_swc, QString fname_output, int score_type, float radius_factor, bool is_gui);
NeuronTree calculateScoreTerafly(V3DPluginCallback2 &callback,QString fname_img, NeuronTree nt, int score_type=1,
                                 float radius_factor=2, float output_thres=255, QString prefix=QString(""));

#endif // CALCULATE_RELIABILITY_SCORE_MAIN_H
