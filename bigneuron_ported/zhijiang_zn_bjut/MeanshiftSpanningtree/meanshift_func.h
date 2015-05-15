/* example_func.h
 * This is an example plugin, the header file of plugin functions.
 * 2012-02-10 : by Yinan Wan
 */
 
#ifndef __EXAMPLE_FUNC_H__
#define __EXAMPLE_FUNC_H__

#include <v3d_interface.h>
#include "Gradient.h"
#include "node.h"


int meanshift_plugin_vn4(V3DPluginCallback2 &callback, QWidget *parent);
void meanshift_vn5(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration);
Node getnode(Node *node);
QList<Node*> found_path_vn2( QMap<V3DLONG,Node*> path_map, Node* temp,Node* temp1,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);

double distance_calculate(unsigned char * &img1d,QList<Node*> path,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
double** bf(QMap<int,Node* > roots,unsigned char * &img1d,double average_dis,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
bool contain(QList<Node> *queue,V3DLONG x,V3DLONG y,V3DLONG z);
int meanshift_plugin(V3DPluginCallback2 &callback, QWidget *parent);
int meanshift_plugin_vn2(V3DPluginCallback2 &callback, QWidget *parent);
QList<Node*> meanshift_vn2(unsigned char * &img1d,int *flag,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration);
QList<Node*> meanshift_vn3(unsigned char * &img1d,int *flag,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration);
void enlarge_radiusof_root_xy(unsigned char * &img1d,QList<Node*> &class_List,Node * &root,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
bool exist(V3DLONG x,V3DLONG y,V3DLONG z,QList<Node*> List);
void meanshift_vn4(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r,int iteration);
void construct_tree(QMap<int,QList<Node*> > finalclass_node,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
int meanshift_plugin_vn4(V3DPluginCallback2 &callback, QWidget *parent,unsigned char* img1d,V3DLONG *in_sz, QString &image_name,bool bmenu);
int meanshift_plugin(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp();
void merge_spanning_tree(QList<QList <NeuronSWC> > &tree_part);
void bubble_sort(QMap<int,double> &root_r,int n);
void spanning_tree1(QList<Node*> &seeds);
void prepare_write(QList<NeuronSWC> marker_MST_sorted);

//void enlarge_radiusof_allnodes_xy(unsigned char * &img1d,QList<Node*> &class_List,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
QList<Node*> enlarge_radiusof_allnodes_xy(unsigned char * &img1d,QList<Node*> &class_List,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
QList<Node*> trim_nodes(QList<Node*> seed,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
NeuronTree  post_process(NeuronTree nt);
void mark_nodeList(int mark,QList<Node*> &List,int s);
int found_finalnode_mark(V3DLONG x,V3DLONG y,V3DLONG z,QList<Node*> final_List);
bool found_final(unsigned char * &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,V3DLONG r);
int ClassMark(QList<Node *> final_List);
void merge_rootnode(QMap<int,Node*> &rootnodes,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
void printSWCByMap_List(QMap<int,QList<Node*> >  List,char * filename);
QMap<V3DLONG,Node*> searchAndConnectByWs(unsigned char* &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,unsigned int ws);
V3DLONG meanshift(unsigned char* &img1d,V3DLONG x,V3DLONG y,V3DLONG z,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,unsigned int ws);
void printSwcByList(QList<Node*> nodeList,char* path);
void printSwcByMap(QMap<int,Node*> nodeMap, char *path);
void createTree(unsigned char* &img1d,Node* curNode,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z,unsigned int ws);
bool checkConnect(unsigned char* &img1d,Node* begin,Node* end,unsigned int ws,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
void prundUnConnectNode(QMap<V3DLONG,Node*> nodeMap);
QList<Gradient*> gradient(QList<Node*> nodes,unsigned char * &img1d,V3DLONG sz_x,V3DLONG sz_y,V3DLONG sz_z);
#endif
