#ifndef __PRE_PROCESSING_MAIN_H__
#define __PRE_PROCESSING_MAIN_H__

//#define _YUN_FINAL_RELEASE_

#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"

#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"
#include "neuron_connector_func.h"
using namespace std;

#include "QtGui"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "neurite_analysis_main.h"
#include "connnect_swc_redefined.h"
#include "utilities.h"
#include "axon_func.h"
#include "auto_soma_correction.h"

#if !defined(Q_OS_WIN32)
#include "unistd.h"
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include "io.h"
#endif

#ifndef XSCALE
#define XSCALE 1
#endif
#ifndef YSCALE
#define YSCALE 1
#endif
#ifndef ZSCALE
#define ZSCALE 1
#endif

#ifdef _YUN_FINAL_RELEASE_
#include "..\\..\\MK\\v3d_imgManagerMK\\integratedDataStructures.h"
#endif

bool pre_processing_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool pre_processing_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output);
bool pre_processing(QString qs_input, QString qs_output, double prune_size = 2, double thres = 0.5, double thres_long = 10,
                    double step_size = 0, double connect_soma_dist = 20, bool rotation = false,
                    bool colorful = false, bool return_maintree = false, bool return_temp=false);
bool split_neuron_domenu(V3DPluginCallback2 &callback, QWidget *parent);
bool split_neuron_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output);
bool neurite_analysis_main(const V3DPluginArgList & input, V3DPluginArgList & output);

void printHelp_pre_processing();


/********* Soma Process Admendment, MK, Oct, 2020 *********/
bool getSomaFromProcessedSWC(const NeuronTree& inputTree, CellAPO& somaMarker);
bool getSomaFromProcessedAPO(const QList<CellAPO>& inputAPOs, CellAPO& somaMarker);
/**********************************************************/
#endif
