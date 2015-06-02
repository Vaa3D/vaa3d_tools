#include "neuron_retrieve.h"
#include "pre_process.h"
#include <QtGlobal>
#include "compute_morph.h"
#include "compute_gmi.h"
#include "sim_measure.h"
#include <math.h>


bool neuron_retrieve(NeuronTree query, QList<double*> & feature_list, vector<V3DLONG>  & result, V3DLONG cand, int method_code, int norm_code)
{
	double step_size = 2;
	V3DLONG VSIZE = 0;
	switch (method_code)
	{
		case 1:
			VSIZE = 21;
			break;
		case 2:
			VSIZE = 14;
			break;
		default:
			printf("ERROR: feature extraction method code unsupported.\n");
			return false;
	}

	V3DLONG neuronNum = feature_list.size();

	//pre-process the query neuron
	NeuronTree query_preprocessed = pre_process(query, step_size);
	
	double *qf;
	int *sbj;
	double* score;
	try 
	{
		qf = new double[VSIZE];
		sbj = new int[cand];
		score = new double[cand];
	}
	catch (...)
	{
		fprintf(stderr,"fail to allocate memory");
		if (qf) {delete []qf; qf=NULL;}
		if (sbj) {delete []sbj; sbj=NULL;}
		if (score) {delete []score; score=NULL;}
		return false;
	}

	if (method_code==1)
	{
		computeFeature(query_preprocessed, qf);
	}
	else if (method_code==2)
	{
		computeGMI(query_preprocessed, qf);
	}

	printf("(2) normalization.\n");
	switch (norm_code)
	{
		case 1:
			if (!retrieve_dist(qf, feature_list, cand, sbj, VSIZE, score))
			{
				printf("Error in retrieve_dist.\n");
				if (qf) {delete []qf; qf=NULL;}
				for (V3DLONG i=0;i<neuronNum;i++)
					if (feature_list[i]) {delete []feature_list[i]; feature_list[i]=NULL;}
				if (sbj) {delete []sbj; sbj=NULL;}
				if (score) {delete[]score; score=NULL;}
				return false;
			}
			break;
		case 2:
			if (!rankScore(qf, feature_list, cand, sbj, VSIZE, score))
			{
				printf("Error in rankscore.\n");
				if (qf) {delete []qf; qf=NULL;}
				for (V3DLONG i=0;i<neuronNum;i++)
					if (feature_list[i]) {delete []feature_list[i]; feature_list[i]=NULL;}
				if (sbj) {delete []sbj; sbj=NULL;}
				if (score) {delete []score; score=NULL;}
				return false;
			}
			break;
		default:
			printf("ERROR: normalization code not supported.\n");
			if (qf) {delete []qf; qf=NULL;}
			for (V3DLONG i=0;i<neuronNum;i++)
				if (feature_list[i]) {delete []feature_list[i]; feature_list[i]=NULL;}
			if (sbj) {delete []sbj; sbj=NULL;}
			if (score) {delete[]score; score=NULL;}
			return false;
	}

	printf("(3) store candidates.\n");
	for (V3DLONG i=0;i<cand;i++)
		result.push_back(sbj[i]);


	if (qf) {delete []qf; qf=NULL;}
	if (sbj) {delete []sbj; sbj=NULL;}
	if (score) {delete[]score; score=NULL;}
	
	return true;
}

bool loadFeatureFile(QString file_name, QList<double*>&  morph_list, QList<double*> & gmi_list, QStringList & nameList)
{
	nameList.clear();
	morph_list.clear();
	gmi_list.clear();
	QFile database_file(file_name);
	if (!database_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		fprintf(stderr, "fail to open file %s\n", qPrintable(file_name));
		return false;
	}
	QTextStream in(&database_file);
	QString line;
	QStringList string_fea;
	line = in.readLine();
	while (!line.isNull())
	{
		nameList.append(line);
		line = in.readLine();
		string_fea = line.split("\t");
		double * new_feature1 = new double[string_fea.size()];
		for (int i=0;i<string_fea.size();i++)
			new_feature1[i] = string_fea[i].toDouble();
		morph_list.append(new_feature1);
		line = in.readLine();
		string_fea = line.split("\t");
		double * new_feature2 = new double[string_fea.size()];
		for (int i=0;i<string_fea.size();i++)
			new_feature2[i] = string_fea[i].toDouble();
		gmi_list.append(new_feature2);
		line = in.readLine();
	}
	
	return true;
}

bool read_matrix(const char* file_name, QList<double*>&  matrix, V3DLONG line, V3DLONG width)
{
	FILE * fp;
	fp = fopen(file_name, "r");
	if (fp==NULL)
	{
		fprintf(stderr, "ERROR: cannot read file %s.\n", file_name);
		return false;
	}
	for (V3DLONG i=0;i<line;i++)
	{
		double * new_feature = new double[width];
		for (V3DLONG j=0;j<width;j++)
		{
			float tmp;
			fscanf(fp, "%f", &tmp);
			new_feature[j] = tmp;
		}
		matrix.append(new_feature);
	}
	return true;
}

bool print_result(vector<V3DLONG> result, const char* file_name, QStringList name_list, const char* database_name, const char* query_name)
{
	FILE * fp;
	fp = fopen(file_name, "w");
	if (fp==NULL)
	{
		fprintf(stderr,"ERROR: %s: failed to open file to write!\n",file_name);
		return false;
	}
	fprintf(fp,"#database:          %s\n",database_name);
	fprintf(fp,"#query neuron:      %s\n",query_name);
	fprintf(fp,"#num of candidates: %d\n",result.size());
	for (int i=0;i<result.size();i++)
	{
		fprintf(fp,qPrintable(name_list[result[i]]));
		fprintf(fp,"\n");
	}
	printf("Linker file %s has been successfully generated.\n", file_name);
	fclose(fp);
	return true;
}

bool compute_intersect(vector< vector<V3DLONG> > & list, vector<V3DLONG> & result, int number, int rej_thres)
{
	result.clear();
	V3DLONG stratNum = list.size();
	V3DLONG neuronNum = list[0].size();
	double * rank_sum = new double[neuronNum];
	for (V3DLONG i=0;i<neuronNum;i++)
		rank_sum[i] = 0;
	for (V3DLONG i=0;i<neuronNum;i++)
		for (V3DLONG j=0;j<stratNum;j++)
			rank_sum[list[j][i]] += i;
	
	int * idx = new int[number];
	double * score = new double[number];
	for (V3DLONG i=0;i<neuronNum;i++)
		rank_sum[i] = -rank_sum[i];
	pick_max_n(idx, rank_sum, number, neuronNum, score);
/*	for (V3DLONG siz=1;siz<=l1.size();siz++)
	{
		result.clear();
		for (V3DLONG i=0;i<siz;i++)
		{
			for (V3DLONG j=0;j<siz;j++)
			{
				if (l1[i]==l2[j])
				{
					result.push_back(l1[i]);
					break;
				}
				if (result.size()>=number) return true;
			}
		}
	}*/
	for (int i=0;i<number;i++)
		result.push_back(idx[i]);
	if (rank_sum) {delete []rank_sum; rank_sum=NULL;}
	if (idx) {delete []idx; idx=NULL;}
	if (score) {delete []score; score=NULL;}
	return true;
}
bool compute_intersect(vector<V3DLONG> l1, vector<V3DLONG> l2, vector<V3DLONG> & result, double thres)
{
	if (l1.size()!=l2.size()) return false;
	int siz = (int) (l1.size() * thres);
	result.clear();
	for (V3DLONG i=0;i<siz;i++)
	{
		for (V3DLONG j=0;j<siz;j++)
		{
			if (l1[i]==l2[j])
			{
				result.push_back(l1[i]);
				break;
			}
		}
	}
	return true;
}
