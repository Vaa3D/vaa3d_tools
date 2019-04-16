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
float E_distance(NeuronSWC &Node1, NeuronSWC &Node2);
void calculate_nodes(NeuronTree ntree, vector<V3DLONG> &nodes);
void min_tree_prim(QList <NeuronSWC> &new_listNeuron);
void save_import_nodes(NeuronTree &ntree, vector<V3DLONG> &nodes, QList <NeuronSWC> &new_listNeuron_denr, QList <NeuronSWC> &new_listNeuron_axon);
void prim(QList <NeuronSWC> &new_listNeuron);
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
