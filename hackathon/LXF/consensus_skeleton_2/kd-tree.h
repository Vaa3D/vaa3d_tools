#ifndef CONVERGE_TREES_H
#define CONVERGE_TREES_H

#include <iostream>
#include <vector>

#include "ANN/ANN.h"
#include "basic_surf_objs.h"
#include "point3d_util.h"

double dist_pt_to_line_seg(const XYZ p0, const XYZ p1, const XYZ p2, XYZ & closestPt);
ANNkd_tree *nt_to_kdt(NeuronTree nt, int nPts);
double kd_correspondingPointFromNeuron( XYZ pt, NeuronTree * p_nt, ANNkd_tree *kd, XYZ & closest_p);
double kd_correspondingNodeFromNeuron(XYZ pt, QList<NeuronSWC> listNodes, int &closestNodeIdx, ANNkd_tree * kd, int TYPE_MERGED);

#endif // CONVERGE_TREES_H

