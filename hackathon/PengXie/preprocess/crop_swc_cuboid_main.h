#ifndef CROP_SWC_CUBOID_MAIN_H
#define CROP_SWC_CUBOID_MAIN_H


#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"

//#include "prune_short_branch.h"
//#include "resampling.h"
//#include "align_axis.h"
//#include "neuron_connector_func.h"
using namespace std;

#include "QtGui"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "neurite_analysis_main.h"
#include "connnect_swc_redefined.h"
#include "utilities.h"
//#include "axon_func.h"

#if !defined(Q_OS_WIN32)
#include "unistd.h"
#endif

#if defined(Q_OS_WIN32)
#include "getopt_tool.h"
#include "io.h"
#endif

bool crop_swc_cuboid(QString qs_input, QString qs_output,
              double xs=0, double ys=0, double zs=0,
              double xe=0, double ye=0, double ze=0,
              double xshift=0, double yshift=0, double zshift=0
              );

bool crop_swc_cuboid_dofunc(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp_crop_swc_cuboid();
#endif // CROP_SWC_CUBOID_H
