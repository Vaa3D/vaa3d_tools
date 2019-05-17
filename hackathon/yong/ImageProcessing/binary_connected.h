#ifndef THRESHOLD_SELECT_H
#define THRESHOLD_SELECT_H

#include "ImageProcessing_plugin.h"
#include <stack>
#include <algorithm>
#include <stdlib.h>
#include <sstream>
#include <utility>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/config.hpp>
#include <boost/graph/connected_components.hpp>

using namespace std;
using namespace boost;
typedef subgraph< adjacency_list<vecS, vecS, directedS,property<vertex_color_t, int>, property<edge_index_t, int> > > Graph;
typedef graph_traits<Graph>::vertex_iterator vertex_iter;

struct ForefroundCoordinate
{
    int x;
    int y;
    int z;
    int signal;

    bool operator == (const ForefroundCoordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};

struct Coordinate
{
    int x;
    int y;
    int z;
    int bri;

    bool operator == (const Coordinate &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};

struct Average
{
    double x;
    double y;
    double z;
    double n;

    bool operator == (const Average &b) const
    {
        return (x==b.x && y==b.y && z == b.z);
    }
};

//typedef subgraph< adjacency_list<vecS, vecS, directedS,property<vertex_color_t, int>, property<edge_index_t, int> > > Graph;
//typedef graph_traits<Graph>::vertex_iterator vertex_iter;

bool binary_connected(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA);
//bool binary_connected(V3DPluginCallback2 &callback,unsigned char* data1d,V3DLONG in_sz[4],V3DPluginArgList &output);
bool binary_connected(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA);
template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev);
bool if_is_connect(Coordinate &curr,Coordinate &b,vector<vector<vector<V3DLONG> > > &mark3D,double connected_thres);
void choose_point(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,QList<NeuronSWC> &choose,QList<NeuronSWC> &choose1,QList<NeuronSWC> &choose2,double thre1,double thre2);
QList<NeuronSWC> choose_point2(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,double thre1,double thre2);
QList<NeuronSWC> choose_point3(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,double thres);
QList<NeuronSWC> choose_point4(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2,double thres);
int choose_distance(QList<NeuronSWC> &neuron1,QList<NeuronSWC> &neuron2);
Average get_average(QList<NeuronSWC> &swc_list,QString filename);
Average get_average1(QList<NeuronSWC> &swc_list,QString filename);
Average get_average2(QList<NeuronSWC> &swc_list,QString filename);
Average get_average3(QList<NeuronSWC> &swc_list,QString filename);
QList<NeuronSWC> marker_to_swc(QList<ImageMarker> & inmarkers);
vector<vector<Coordinate> > build_rela2(vector<vector<Coordinate> >&connected_region,QHash<int,vector<int> > &rela2);
int GetStack(vector<vector<int> > dist_matrix,int i,int n,stack<int> &st);
int GetChildGrape(vector<vector<int> > dist_matrix,int n);
bool export_TXT(vector<ForefroundCoordinate> &vec_coord,QString fileSaveName);
bool writeTXT_file(const QString & filename, const QList <ImageMarker> & listMarker);



#endif // BINARY_CONNECTED_H
