#ifndef FEATURE_ANALYSIS_H
#define FEATURE_ANALYSIS_H
#include "quality_control_plugin.h"
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>
#include <vector>
#include "my_surf_objs.h"
#include "v3d_message.h"
#include "basic_surf_objs.h"

class OpenSWCDialog: public QDialog
{
    Q_OBJECT

public:
    V3DPluginCallback2 * callback;
    QListWidget * listWidget;
    QList<NeuronTree> _3DViewerTreeList;
    NeuronTree  nt;
    QString file_name;

    OpenSWCDialog(QWidget * parent, V3DPluginCallback2 * callback);
    void getAllNeuronTreeNames(V3DPluginCallback2 * callback);

public slots:
    bool run();
    bool setTree(const QString & file);
};

void computeFeature(const NeuronTree & nt, double * features);
void computeLinear(const NeuronTree & nt);
void computeTree(const NeuronTree & nt);
double computeHausdorff(const NeuronTree & nt);
void Process(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 & callback);


#endif // FEATURE_ANALYSIS_H
