/* neurontree_structure_matching_plugin.h
 * neurontree structure matching
 * 2017-6-23 : by Xiaodong Yue
 */
 
#ifndef __NEURONTREE_STRUCTURE_MATCHING_PLUGIN_H__
#define __NEURONTREE_STRUCTURE_MATCHING_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>

#include <iostream>
#include <global_neuron_feature.h>
//
typedef struct 
{
    double x_min;
	double x_max;
	double y_min;
	double y_max;
	double z_min;
	double z_max;
}CubeBox;

class NeurontreeStructureMatching : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
	Q_INTERFACES(V3DPluginInterface2_1);

public:
	float getPluginVersion() const {return 1.1f;}

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

	QStringList funclist() const ;
	bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent);

	bool FindSimilarStructures(const NeuronTree & nt1, const NeuronTree & nt2);
	bool GetSubTreesSWC(const NeuronTree & nt, int window_size, int step, const QString & filepath, QList<NeuronTree> & nt_list);

};

//extern functions
bool SeperateNeuronTreesSWC(const QString & filepath, QList<NeuronTree> & nt_list);
bool GetNeuronTreeFromCubeBox(const QList<CubeBox> & list_box, const NeuronTree & nt_ori, QList<NeuronTree> & list_nt);



#endif

