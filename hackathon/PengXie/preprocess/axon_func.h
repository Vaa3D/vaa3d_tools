#ifndef AXON_FUNC_H
#define AXON_FUNC_H

#include "QtGlobal"
#include "vector"
#include "v3d_message.h"
#include "v3d_interface.h"
#include "prune_short_branch.h"
#include "resampling.h"
#include "align_axis.h"
#include "neuron_connector_func.h"
#include "utilities.h"
#include "basic_surf_objs.h"
#include "sort_swc_redefined.h"
#include "math.h"
using namespace std;

bool axon_retype(QString whole_axon_swc, QString lpa_swc, QString output_swc, bool proximal_distal=true);
QList<double> branch_distribution(QString whole_axon_swc, QString lpa_swc);
bool export_branch_distribution(QList<double> lpa_density, QString output_fileName);
#endif // AXON_FUNC_H
