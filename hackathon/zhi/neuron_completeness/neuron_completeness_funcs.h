#ifndef __NEURON_COMPLETENESS_FUNCS_H__
#define __NEURON_COMPLETENESS_FUNCS_H__


#include <QtGui>
#include <v3d_interface.h>
#include <vector>

using namespace std;
#define MAX_DOUBLE 1.79769e+308

struct NEURON_METRICS {
    int numTrees;
    int numTypes;
    int numSegs;

};

double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2);
void calComplete(NeuronTree nt, QList<NEURON_METRICS> & scores);
QStringList importSWCFileList(const QString & curFilePath);
void exportComplete(NeuronTree nt,QList<NeuronSWC>& sorted_neuron, LandmarkList& markerlist,
                    QMultiMap<int, QList<NeuronSWC> >& multi_neurons,QHash<int,int>& map_type,QList<double>& dist);


#endif
