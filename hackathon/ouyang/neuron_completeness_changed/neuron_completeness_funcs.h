#ifndef __NEURON_COMPLETENESS_FUNCS_H__
#define __NEURON_COMPLETENESS_FUNCS_H__


#include <QtGui>
#include <v3d_interface.h>
#include <vector>

#include <map>
#include <set>


using namespace std;
#define MAX_DOUBLE 1.79769e+308
#define GRID_LENGTH 50

struct NEURON_METRICS {
    int numTrees;
    int numTypes;
    int numSegs;
};

double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2);
void calComplete(NeuronTree nt, QList<NEURON_METRICS> & scores);
void detect_type_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QStringList importSWCFileList(const QString & curFilePath);
void exportComplete(NeuronTree nt,QList<NeuronSWC>& sorted_neuron, LandmarkList& markerlist,
                    QMultiMap<int, QList<NeuronSWC> >& multi_neurons,QHash<int,int>& map_type,QList<double>& dist);
void markerlist_before_sorting(QList<NeuronSWC>ori_tree1swc,LandmarkList& markerlist,int& wrongtype);
void markerlist_after_sorting(QList<NeuronSWC>sorted_neuron,LandmarkList& markerlist,QVector<QVector<V3DLONG> > childs,int& wrongplace);
bool export_file2record(string swc_name,int ct1,double num_wrong_type,QString fileSaveName);
bool export_file2record_tree(string swc_name,double dis,QString fileSaveName,QString tree);
vector<QString> GetFileList(QString dir);


#endif
