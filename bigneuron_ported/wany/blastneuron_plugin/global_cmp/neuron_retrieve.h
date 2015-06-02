#ifndef __NEURON_RETRIEVE_H__
#define __NEURON_RETRIEVE_H__

#include "basic_surf_objs.h"
#include <math.h>
#include <unistd.h>
#include <vector>
using namespace std;

bool neuron_retrieve(NeuronTree query, QList<double*> & feature_list, vector<V3DLONG>  & result, V3DLONG cand, int method_code, int norm_code);
bool print_result(vector<V3DLONG> result, const char* file_name, QStringList name_list, const char* database_name, const char* query_name);
bool compute_intersect(vector<vector<V3DLONG> > & l1,  vector<V3DLONG> & result, int thres, int rej_thres);
bool compute_intersect(vector<V3DLONG> l1, vector<V3DLONG> l2, vector<V3DLONG> & result, double thres);
bool read_matrix(const char* file_name, QList<double*> & matrix, V3DLONG line, V3DLONG width);
bool loadFeatureFile(QString file_name, QList<double*>&  morph_list, QList<double*> & gmi_list, QStringList & nameList);
#endif
