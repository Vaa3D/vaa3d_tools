#ifndef FIND_FEATURE_H
#define FIND_FEATURE_H


#include "basic_surf_objs.h"

#include <math.h>
#include <iostream>
#include <complex>
#include <vector>
#include <QVector>
using namespace std;

#define VOID 1000000000
#define PI 3.141592653589793



void compute_neuron_GMI(double **b, int siz,  double* centerpos, double radius_thres, double * gmi);
double compute_moments_neuron(double ** a, int siz, double p, double q, double r, double radius_thres);
void computeGMI(const NeuronTree & nt, double * gmi);

void computeFeature(const NeuronTree & nt, double * features);
QVector<V3DLONG> getRemoteChild(int t);
void computeLinear(const NeuronTree & nt);
void computeTree(const NeuronTree & nt);


bool neuron_retrieve(NeuronTree query, QList<double*> & feature_list, vector<V3DLONG>  & result, V3DLONG cand, int method_code, int norm_code);
bool print_result(vector<V3DLONG> result,const char* database_name, const char* query_name,vector<NeuronTree> &result_out,vector<V3DLONG> &num_out);
bool compute_intersect(vector<vector<V3DLONG> > & l1,  vector<V3DLONG> & result, int thres, int rej_thres);
bool compute_intersect(vector<V3DLONG> l1, vector<V3DLONG> l2, vector<V3DLONG> & result, double thres);
bool read_matrix(const char* file_name, QList<double*> & matrix, V3DLONG line, V3DLONG width);
bool loadFeatureFile(QString file_name, QList<double*>&  morph_list, QList<double*> & gmi_list, QStringList & nameList);
bool match_little_pattern(vector<NeuronTree> v_nt,NeuronTree nt,vector<V3DLONG> &num_out,QList<double*> m_morph_list, QList<double*> m_gmi_list,QList<double* > v_morph_list,QList<double* > v_gmi_list);
bool get_feature(NeuronTree v_nt,NeuronTree nt,QList<double*> m_morph_list, QList<double*> m_gmi_list,QList<double*> v_morph_list,QList<double*> v_gmi_list);



#endif // FIND_FEATURE_H
