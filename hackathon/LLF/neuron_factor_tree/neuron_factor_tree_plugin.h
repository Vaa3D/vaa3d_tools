/************************************************************************/
/*	neuron_factor_tree_plugin.h
	a header file for vaa3d plugin:NeuronFactorTree.
	by Longfei li, 2019-01-17.                                           */
/************************************************************************/

#ifndef __NEURON_FACTOR_TREE_PLUGIN_H__
#define __NEURON_FACTOR_TREE_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include "neuron_factor_tree.h"
#include <math.h>

using namespace std;

#define TESTDIR "C:/Users/Administrator/Desktop/neuron_factor_tree_test/test_1"

//A struct handle x,y,z coordinates;
struct Coordxyz
{
	double x, y, z;

	Coordxyz() { x = y = z = 0; }
	Coordxyz(double a, double b, double c) :x(a), y(b), z(c) {}
	Coordxyz(float a, float b, float c) :x(a), y(b), z(c) {}
	Coordxyz operator-(Coordxyz coord) const { return Coordxyz(x - coord.x, y - coord.y, z - coord.z); }
	float operator*(Coordxyz coord) const { return (x*coord.x + y * coord.y + z * coord.z); }
	Coordxyz operator*(float d) const { return Coordxyz(x*d, y*d, z*d); }
	friend Coordxyz operator*(float d, const Coordxyz coord) { return Coordxyz(coord.x*d, coord.y*d, coord.z*d);  }
	friend ostream & operator<<(ostream &os, const Coordxyz coord) { os << coord.x << "," << coord.y << "," << coord.z; return os; }
	Coordxyz operator/(float d) const { return Coordxyz(x / d, y / d, z / d); }
	Coordxyz cross_product(Coordxyz c)
	{
		return Coordxyz(y*c.z - c.y*z, c.x*z - x*c.z, x*c.y - c.x*y);
	}
	float length()
	{
		return sqrt(x*x + y * y + z * z);
	}
};

class NeuronFactorTreePlugin : public QObject, public V3DPluginInterface2_1
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

/************************************************************************/
/* Domenu fucs                                                          */
/************************************************************************/
void generate_neuron_factor_tree();
//create a neuronFactorTree from a selected neuronTree by domenu fuc.
void serializate_neuron_factor_tree();//from a .nft file get a .nfss neuronFactor sequences file.
void collect_nfss_in_folder();//collect all .nfss files under a path;
//combine files by their classes under a base dir : base_dir/classes/class_n/*.sqss -> base_dir/collected_features.txt.

void construct_neuron_factor_tree(const NeuronTree nt, NeuronFactorTree &nft);//from neuronTree create a level 1 neuronFactorTree. TODO
void construct_neuron_factor_tree(NeuronFactorTree nft_input, NeuronFactorTree &nft_output, int level);//generate a coarser level neuronFactorTree. TODO

/************************************************************************/
/* Dofunc fucs                                                          */
/************************************************************************/
bool nfss_product_pipeline(const V3DPluginArgList & input, V3DPluginArgList & output);//pipeline to trans a *.swc to *.nfss.
bool nfss_collect_pipeline(const V3DPluginArgList & input, V3DPluginArgList & output);//pipeline to collect all the *.nfss into a folder.
void print_help();//print help infos.

/************************************************************************/
/* neuronFactor tool fucs                                               */
/************************************************************************/

//NeuronFactor Tree construct tools
void calculate_branches(const NeuronTree &neuron, vector<V3DLONG> &branches);
void get_leaf_branch_nodes(const NeuronTree &neuron, vector<V3DLONG> branches, set<V3DLONG> &leaf_node_set, set<V3DLONG> &branch_node_set);
void get_nft_node_relation_map(const NeuronTree &neuron, vector<V3DLONG> branches, set<V3DLONG> nft_node_set, map<V3DLONG,V3DLONG> &nft_parent_map, map<V3DLONG,V3DLONG> &nft_child_map, map<V3DLONG, V3DLONG> &nft_sibling_map);
NeuronFactorTree create_neuron_factor_tree(const NeuronTree &neuron);

//Use relation maps calculate features at each node
void calculate_features_at_node(const NeuronTree neuron, const V3DLONG current, const map<V3DLONG,V3DLONG> nft_pa_map, const map<V3DLONG,V3DLONG> nft_chi_map, const map<V3DLONG,V3DLONG> nft_sib_map,unordered_map<string,double> &feature, vector<double> &feature_vector);

//NeuronFactor Tree w/r tools
void save_neuron_factor_tree(const NeuronFactorTree nft, const QString path);
NeuronFactorTree read_neuron_factor_tree(const QString path);

//Serialization tools
NeuronFactorSequences serialization(const NeuronFactorTree nft);
void save_neuron_factor_sequences(const NeuronFactorSequences &nfseqs, const QString path);
void collect_nfss(const QString file_path, FILE * fp, int &sequence_total_count, const QString class_name);//collect a .nfss file under file_path marked by classname;
//don't need read sequences;

//Classifer need training:class??
vector<int> classifer(NeuronFactors nfs);//product labels for neuronFactors.
int classifer(NeuronFactor nf);//product a label for a neuronFactor.


#endif
