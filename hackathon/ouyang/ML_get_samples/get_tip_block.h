#ifndef GET_TIP_BLOCK_H
#define GET_TIP_BLOCK_H

#include <v3d_interface.h>
#include "../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h"
struct block{
    QString name;
    XYZ small;
    XYZ large;
};
struct node_and_id{
    vector<MyMarker> all_nodes;
    vector<int> all_id;
    int total_signal;

};

void get_deleted_tree(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
QList<int> get_tips(NeuronTree nt, bool include_root);
block offset_block(block input_block, XYZ offset);
void crop_img(QString image, block crop_block, QString outputdir_img, V3DPluginCallback2 & callback, QString output_format,QString input_swc,int tipnum,XYZ tip);
void crop_swc(QString input_swc, QString output_swc, block crop_block);
bool my_saveANO(QString ano_dir, QString fileNameHeader, QList<QString> suffix);
void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp1(const V3DPluginArgList & input, V3DPluginArgList & output);
void get2d_label_image(NeuronTree nt_crop_sorted,V3DLONG mysz,unsigned char * data1d_crop,V3DPluginCallback2 & callback,QString output_format,int tipnum,XYZ tip);
void get_2d_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
void get_tip_image(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);
int find_tip(NeuronTree nt, long sz0, long sz1, long sz2);
double get_circle_signal(vector<MyMarker> allmarkers, unsigned char * data1d,long sz0, long sz1, long sz2);
node_and_id return_signal_in_circle_nodes(int radius, double ave_signal,V3DLONG mysz[4],unsigned char * data1d,MyMarker center_marker);
vector<MyMarker> get_in_circle_nodes(MyMarker center_marker, double circle_radius);
QList<int> get_tips(NeuronTree nt, bool include_root);
MyMarker node_to_center(node_and_id all_max_nodes,QList<NeuronSWC> input_swc,long mysz[4],unsigned char * data1d);
node_and_id get_26_neib_id(MyMarker center_marker,long mysz[4],unsigned char * data1d);
QList<NeuronSWC> change_tip_xyz(QList<NeuronSWC>input_swc,int tip_node,MyMarker center);
NeuronTree neuronlist_2_neurontree(QList<NeuronSWC> neuronlist);
NeuronTree revise_radius(NeuronTree inputtree,float radius);
NeuronTree get_unfinished_sample(QList<int> tip_list,NeuronTree treeswc,int maximum);
bool export_list2file(const QList<NeuronSWC>& lN, QString fileSaveName);
void get_block(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback);

#endif // GET_TIP_BLOCK_H
