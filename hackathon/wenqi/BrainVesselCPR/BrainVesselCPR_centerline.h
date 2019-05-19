#ifndef BRAINVESSELCPR_CENTERLINE_H
#define BRAINVESSELCPR_CENTERLINE_H

#include <v3d_interface.h>
#include <vector>
#include <queue>
#include <iostream>
using namespace std;

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef ABS
#define ABS(x) ((x) > 0 ? (x) : (-(x)))
#endif

struct Node
{
    V3DLONG node_id;
    double priority;
    Node(V3DLONG id, double p): node_id(id), priority(p)
    {
    }
};

struct Coor3D
{
    double x;
    double y;
    double z;
};

bool operator > (const Node &n1, const Node &n2);

double edgeCost(int a_intensity, int b_intensity);

double heuristic(V3DLONG next, V3DLONG goal, int x_length, int y_length);

NeuronTree construcSwc(vector<Coor3D> path_point);

bool smooth_curve(std::vector<Coor3D> & mCoord, int winsize);

void findPath(V3DLONG start, V3DLONG goal, unsigned short int * image1d, int x_length, int y_length, int z_length, V3DPluginCallback2 &callback, QWidget *parent);

vector<Coor3D> meanshift(vector<Coor3D> path, unsigned short int * data1d, V3DLONG x_len, V3DLONG y_len, V3DLONG z_len, int windowradius);


#endif
