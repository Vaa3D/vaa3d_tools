/* nblast_demo_plugin.h
 * Try to complete a plugin in nblast way
 * 2018-10-25 : by Longfei Li
 */

#ifndef __PERSISTENCE_DIAGRAM_PLUGIN_H__
#define __PERSISTENCE_DIAGRAM_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include <math.h>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <set>

using namespace std;

#define VALUETYPE double

#define VOID -1000000000

#define DIST(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))

struct LifeTime 
{
	VALUETYPE birth;
	VALUETYPE death;
};

typedef vector<LifeTime> BarCode;

class PersistenceDiagramPlugin : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const { return 1.1f; }

	QStringList menulist() const;
	void domenu(const QString &menu_name,
		V3DPluginCallback2 &callback,
		QWidget *parent);

	QStringList funclist() const;
	bool dofunc(const QString &func_name,
		const V3DPluginArgList &input,
		V3DPluginArgList &output,
		V3DPluginCallback2 &callback,
		QWidget *parent);
};
//generate a PD
void generate_persistence_diagram();
//find leaf nodes
void find_leaf_nodes(NeuronTree neuron, set<V3DLONG> &leaf_node_set);
//calculate branches
void calculate_branches(NeuronTree neuron, vector<V3DLONG> &branches);
//calculate feature at one node
double calculate_feature_at_node(NeuronTree neuron, V3DLONG node_id);
//add value interval to a barcode
void barcode_add(BarCode &barcode,VALUETYPE value_begin,VALUETYPE value_end);
//save a barcode
void barcode_save(QString output_path,QString neuron_path, BarCode barcode);
//from a barcode generate a PD
void construct_PD_from_barcode(BarCode barcode);
//help
void printHelp();

#endif