#ifndef BRANCH_DETECT_H
#define BRANCH_DETECT_H

#include "branch_crop_plugin.h"
#include "../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h"
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <set>
#include "my_surf_objs.h"
#include <prune_short_branch.h>
#include "v3d_message.h"
#include  "volimg_proc.h"
//#include "../../PengXie/tip_signal/utilities.h"
//#include "../../PengXie/neuron_reliability_score/src/my_surf_objs.h"

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

struct block{
    QString name;
    XYZ small;
    XYZ large;
};
double Angle(XYZ p1,XYZ p2);
block offset_block(block input_block, XYZ offset);
//double average_intensity(unsigned char *data1d_crop,NeuronTree nt,XYZ center, int size, XYZ diff, long mysz0,long mysz1);
bool getMarkersBetween(vector<MyMarker> &allmarkers, MyMarker m1, MyMarker m2);
double signal_at_markers(vector<MyMarker> allmarkers, unsigned char * data1d, long sz0, long sz1, long sz2);
bool export_list22file(const QList<NeuronSWC>& lN, QString fileSaveName);
double signal_between_markers(unsigned char * data1d, NeuronSWC n1, NeuronSWC n2, long sz0, long sz1, long sz2, long sz3, V3DPluginCallback2 & callback);
NeuronTree my_SortSWC(NeuronTree nt, V3DLONG newrootid, double thres);
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist);
bool in_cuboid(NeuronSWC node, XYZ small, XYZ large);
bool crop_swc_cuboid(NeuronTree nt, QString qs_output,block input_block);
double marker_dist(MyMarker a, MyMarker b, bool scale);
int find_tip(NeuronTree nt, long sz0, long sz1, long sz2);
void get_branches(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
XYZ offset_XYZ(XYZ input, XYZ offset);
vector< vector<int> > get_close_points(NeuronTree nt,vector<int> a);
//void crop_swc(QString input_swc, QString output_swc, block crop_block);
void get2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
double computeDist2(const NeuronSWC & s1, const NeuronSWC & s2);
void missing_branch_detection(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget *parent);
LandmarkList get_missing_branches_menu(unsigned char *data1d_crop, NeuronTree nt, long mysz0,long mysz1,long mysz2);
#endif // BRANCH_DETECT_H
