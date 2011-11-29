#ifndef __UTILS_H__
#define __UTILS_H__

#include "basic_types.h"
#include <limits>

typedef struct StatisticFeature
{
	int ndims;
	int nindis;
	vector<double> sum_feat;
	vector<double> mean_feat;
	vector<double> min_feat;
	vector<double> max_feat;
	StatisticFeature(){ndims=0;}
	StatisticFeature(int _ndims)
	{
		setDims(_ndims);
	}
	void setDims(int _ndims)
	{
		ndims = _ndims;
		sum_feat.resize(ndims);
		mean_feat.resize(ndims);
		min_feat.resize(ndims);
		max_feat.resize(ndims);
	}
	void init()
	{
		if(ndims > 0)
		for(int j = 0; j < ndims; j++)
		{
			min_feat[j] = numeric_limits<double>::max() / 2.0 - 1.0;
			max_feat[j] = 0.0;
			sum_feat[j] = 0.0;
			mean_feat[j] = 0.0;
		}
	}
	void setIndiNum(int _nindis) {nindis = _nindis;}
} StatFeat;

struct CompareResult
{
	int id1;
	int id2;
	double min_dst;
};
/*
struct CompareResult_Rank
{
	int id1;
	vector<vector<int> > allranks;
	int getId2(){
		if(allranks.empty()) return -1;
		int ndims = allranks.size();
		map<int, int> scores;
		for(int i = 0; i < ndims; i++)
		{
			vector<int> rank = allranks[i];
			for(int j = 0; j < rank.size(); j++)
			{
				if(scores.find(rank[j])!=scores.end())
				{
					scores[rank[j]] += j;
				}
				else scores[rank[j]] = j;
			}
		}
		return (*scores.begin()).first;
	}
};
*/
bool detect_marker(vector<MarkerType> & vecMarker, unsigned char* inimg1d, V3DLONG sz[3]);

bool marker_to_feature(vector<FeatureType> & vecFeature, vector<MarkerType> vecMarker, unsigned char* inimg1d, V3DLONG sz[3]);

vector<MarkerType> feature_to_marker(vector<FeatureType> & vecFeature);

bool uniform_features(vector<FeatureType> &vecFeature1, vector<FeatureType> &vecFeature2);

bool compare_features(vector<CompareResult> &crs, vector<FeatureType> & vecFeature1, vector<FeatureType> & vecFeature2);

vector<MarkerType> extract_id1_markers(vector<CompareResult>&crs, vector<MarkerType> & vecMarker);

vector<MarkerType> extract_id2_markers(vector<CompareResult>&crs, vector<MarkerType> & vecMarker);
#endif
