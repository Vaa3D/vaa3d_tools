/*
genVirture_plugin.h
Tue Mar 26 14:52:35 2019
*/

#ifndef __GENVIRTURE_PLUGIN_H__
#define __GENVIRTURE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include <vector>
using namespace std;
//任意两点之间的欧式距离
float E_distance(NeuronSWC &Node1, NeuronSWC &Node2);
//calculate the number of nodes children
void calculate_nodes(NeuronTree ntree, vector<V3DLONG> &nodes);
//
void save_point_nodes(NeuronTree &ntree, vector<V3DLONG> &nodes, QList <NeuronSWC> &new_listNeuron);
void min_tree_prim(QList <NeuronSWC> &new_listNeuron);
//save branchs and terminal and soma 
void save_import_nodes(NeuronTree &ntree, vector<V3DLONG> &nodes, QList <NeuronSWC> &new_listNeuron_denr, QList <NeuronSWC> &new_listNeuron_axon);
//prim alogrithm
void prim(QList <NeuronSWC> &new_listNeuron);
void init(QList <NeuronSWC> &listNeuron);
void gen_axon(QList <NeuronSWC> &listneuron);
class genVirturePlugin: public QObject, public V3DPluginInterface2_1
{
   Q_OBJECT
   Q_INTERFACES(V3DPluginInterface2_1);

public:
   float getPluginVersion() const{return 1.1f;}

   QStringList menulist() const;
   void domenu(const QString & menu_name, V3DPluginCallback2 & callback, QWidget * parent);

   QStringList funclist() const;
   bool dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent);
};
//not use 
struct NodeXYZ{
	float x;
	float y;
	float z;
};

struct edge{
	V3DLONG from;
	V3DLONG to;
	float cost;
	edge(V3DLONG f,V3DLONG t, float c) :from(f),to(t), cost(c){}
	edge(){}
	bool operator< (const edge &a) const{
		return a.cost < cost;
	}
};
#endif
