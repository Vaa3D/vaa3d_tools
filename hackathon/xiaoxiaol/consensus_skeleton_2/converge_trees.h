#ifndef CONVERGE_TREES_H
#define CONVERGE_TREES_H

#include <iostream>
#include <vector>

#include "ANN/ANN.h"
#include "basic_surf_objs.h"
#include "point3d_util.h"

//XYZ mean_XYZ(vector<XYZ> points);
//double kd_dist_pt_to_line_seg(const XYZ p0, const XYZ p1, const XYZ p2, XYZ & closestPt);
//double kd_correspondingPointFromNeuron( XYZ pt, NeuronTree * p_nt, XYZ & closest_p);
//void  kd_resample_neurons(vector<NeuronTree> nt_list,  vector<NeuronTree> * nt_list_resampled);
void nt_to_kdt(NeuronTree nt, ANNpointArray *kd_pts, int nPts);






#endif // CONVERGE_TREES_H
