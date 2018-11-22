/* nblast_demo_plugin.h
 * Try to complete a plugin in nblast way
 * 2018-10-25 : by Longfei Li
 */

#ifndef __NBLAST_DEMO_PLUGIN_H__
#define __NBLAST_DEMO_PLUGIN_H__

#include <QtGui>
#include <v3d_interface.h>
#include "v3d_message.h"
#include "basic_surf_objs.h"
#include <math.h>
#include <vector>
#include<iostream>

using namespace std;

//number of features
#define FEATURE_NUM 10

#define VOID -1000000000

//DIST for NeuronSWC;DISTP for struct Point 
#define DIST(a,b) sqrt(((a).x-(b).x)*((a).x-(b).x)+((a).y-(b).y)*((a).y-(b).y)+((a).z-(b).z)*((a).z-(b).z))
#define DISTP(a,b) sqrt(((a)->x-(b)->x)*((a)->x-(b)->x) + ((a)->y-(b)->y)*((a)->y-(b)->y) + ((a)->z-(b)->z)*((a)->z-(b)->z))

struct Point 
{
	double x,y,z,r;
	Point* p;
	int type;
	V3DLONG childNum;
};

typedef vector<Point*> Segment;
typedef vector<Point*> Tree;

class NblastDemoPlugin : public QObject, public V3DPluginInterface2_1
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

//pre_processing
void pre_processing();
//help
void printHelp();
//prune a neuro
bool prune_branch(NeuronTree nt, NeuronTree &nt_pruned);
//resample on a neuronTree by a resample_distance
void resample(NeuronTree &neuron_pruned, double resample_distance, NeuronTree &resampled);
//resample from a segment get one resample point every resample_distance
void resample_segment(Segment* seg,double resample_distance);
//save neuron tree to a file
void save_neuron_tree(QString outfileName, NeuronTree &neuron);

//global retrieve
void global_retrieve();
//load features from a database
void load_features(QString database_path,QList<double*> &features_list, QStringList &nameList);
//load features from a text
void load_features(QString feature_path,double* features);
//retrieve neurons that are similar with query one, decrease order stored in sorted_index_vec
void neuron_retrieve(QString query_neuron, QList<double*> &features_list, vector<V3DLONG> &sorted_index_vec);
//compute a neuron's feature
void compute_features(NeuronTree query_preprocessed, double* features);//get features from query neuron
//culculate similarity
void retrieve_similarity(double *features, QList<double*> &features_list,int *similarity_top_index,double *similarity_top_score,int top_similar_num);
//output result
void print_result(vector<V3DLONG> sorted_index_vec, const char* output_path, QStringList name_list, const char* database_path, const char* query_neuron_path);
//get the position of a value in the max list(decrease order)
int bin_search(double* list, double v, int start, int end);
//a tool for making swcs to ano file
void swcs2ano();
//for files in a dir
QFileInfoList GetFileInfoList(QString path);
//a tool count results' accuracy
void count_accuracy();
#endif