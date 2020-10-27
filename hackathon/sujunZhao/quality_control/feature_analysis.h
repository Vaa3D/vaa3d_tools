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
using namespace std;
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

struct TreeNode
{
    int val;
    int left;
    int right;
};

bool export_list2file(QList<NeuronSWC> & lN, QString fileSaveName, QString fileOpenName);
NeuronTree my_SortSWC(NeuronTree nt, V3DLONG newrootid, double thres);
bool SortSWC(QList<NeuronSWC> & neurons, QList<NeuronSWC> & result, V3DLONG newrootid, double thres);
void computeFeature(const NeuronTree & nt, double * features);
void computeLinear(const NeuronTree & nt);
void computeTree(const NeuronTree & nt);
double computeHausdorff(const NeuronTree & nt);
bool split_neuron(QString qs_input, QString qs_outdir);
void Analysis(QString indir,vector<char*> outfiles, bool hasOutput);
void Process(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 & callback);
QList<int> get_subtree(NeuronTree nt, int id);
QList<int> loop_detection(const NeuronTree & nt);
//QList<TreeNode> tree(NeuronTree nt, int n);
void tree_structure(const V3DPluginArgList & input, V3DPluginArgList & output,V3DPluginCallback2 & callback);
void arbor_main(V3DPluginCallback2 &callback, QWidget *parent);
void arbor_qc(QString input1, QString input2);
void arbor_analysis(QString swc,QString folderpath, QString outgf, QHash <QString, QString> swc_celltype,
                    QString out, QHash <QString, QString> swc_region);
#endif // FEATURE_ANALYSIS_H
