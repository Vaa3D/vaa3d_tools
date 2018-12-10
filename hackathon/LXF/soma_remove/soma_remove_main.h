#ifndef SOMA_REMOVE_MAIN_H
#define SOMA_REMOVE_MAIN_H

#include "v3d_message.h"
#include "../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h"
#include "../../heyishan/blastneuron_bjut/overlap/overlap_gold.h"
#include "data_training.h"
#include <stack>
#include <algorithm>
#include <stdlib.h>
#include <sstream>
#include <utility>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/config.hpp>
#include <boost/graph/connected_components.hpp>

#include  <vector>
struct Coordinate
{
    int x;
    int y;
    int z;
    int bri;
    string inf1;
    int inf2;
    string name;
    bool operator == (const Coordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};
struct Max_level
{
    int level1;
    int level2;
};
struct Soma
{
  double x_b;
  double x_e;
  double y_b;
  double y_e;
  double z_b;
  double z_e;
};
vector<double> classify_glio_Y(Feature &f,Each_line &E,vector<vector<double> > &R,double &sum_log);
bool soma_remove_main(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
bool soma_remove_main_2(unsigned char* data1d,V3DLONG in_sz[4],V3DPluginCallback2 &callback);
bool if_is_connect(Coordinate &curr,Coordinate &b,vector<vector<vector<V3DLONG> > > &mark3D);
void find_relation(QHash<vector<Coordinate>,vector<Coordinate> > &relation,vector<Coordinate> &curr,vector<Coordinate> &tmp,vector<V3DLONG> &reminder2,QHash<Coordinate,int> &map_index);
bool feature_calculate(vector<inf> &inf_v,vector<double> &y_n,vector<double> &overlap_level,vector<double> &ratio_v,vector<double> &count_v,vector<double> &D,vector<double> &grey,vector<double> &grey_std,QList<ImageMarker> &center,vector<vector<Coordinate> >&connected_region_final,vector<double> &overlap_all);
vector<vector<Coordinate> > build_rela2(vector<vector<Coordinate> >&connected_region,QHash<int,vector<int> > &rela2);
#endif // SOMA_REMOVE_MAIN_H
