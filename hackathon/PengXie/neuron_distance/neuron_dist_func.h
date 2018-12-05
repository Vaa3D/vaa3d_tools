/* neuron_dist_func.h
 * The plugin to calculate distance between two neurons. Distance is defined as the average distance among all nearest pairs in two neurons.
 * 2012-05-04 : by Yinan Wan
 */
 
#ifndef __NEURON_DIST_FUNC_H__
#define __NEURON_DIST_FUNC_H__
#define XSCALE 0.2
#define YSCALE 0.2
#define ZSCALE 1
#include <v3d_interface.h>

int neuron_dist_io(V3DPluginCallback2 &callback, QWidget *parent);
bool neuron_dist_io(const V3DPluginArgList & input, V3DPluginArgList & output);
bool neuron_dist_toolbox(const V3DPluginArgList & input, V3DPluginCallback2 & callback);
void printHelp();

bool neuron_overlap_io(const V3DPluginArgList & input, V3DPluginArgList & output);

int neuron_dist_mask(V3DPluginCallback2 &callback, QWidget *parent);
bool neuron_dist_mask(const V3DPluginArgList & input, V3DPluginArgList & output);

CellAPO get_marker(NeuronSWC node, double vol, double color_r, double color_g, double color_b);

double overlap_directional_swc_1_2(NeuronTree nt1, NeuronTree nt2, double overlap_thres, QString savemarkerfile);


// Structure for saving overlap information
struct NeuronOverlapSimple
{
    QString nt1;
    QString nt2;
    double nt1_length;
    double nt2_length;
    double overlap_length;
    NeuronOverlapSimple() {nt1_length = nt2_length = overlap_length = -1; }
};
NeuronOverlapSimple overlapping_structure(const NeuronTree *p1, const NeuronTree *p2, bool bmenu, double overlap_thres, QString savemarkerfile=QString());

// Structure for roughly check overlaps
double max(double x, double y);
double min(double x, double y);
struct cuboid{
    XYZ min_corner;
    XYZ max_corner;
    cuboid() {min_corner=max_corner=XYZ(-1,-1,-1);}
};
struct myline{
    double a;
    double b;
    myline() {a=b=-1;}
};
cuboid check_cuboid_order(cuboid cb);
myline check_line_order(myline ln);
bool line_intersect(myline l1, myline l2);
bool cuboid_intersect(cuboid cb1, cuboid cb2);

/* Overlaps of two neurons */
cuboid nt_to_cuboid(NeuronTree nt);
bool cuboid_overlap_nt(NeuronTree nt1, NeuronTree nt2);
#endif

