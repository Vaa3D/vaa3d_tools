#include <iostream>
#include <vector>
#include <algorithm>

#include "img_definition.h"
#include "basic_memory.h"
#include "utils.h"
#include "basic_types.h"

#include "../../v3d_convert/gaussian_blur.cpp"    // smooth
#include "../edge_detection.h"
#include "../compute_moments.h"

using namespace std;

bool detect_marker(vector<MarkerType> & markervector, unsigned char* inimg1d, V3DLONG sz[3])
{
#define SIGMA 1.0
#define THRESHOLD 20.0

	if(!markervector.empty() || !inimg1d || sz[0] <=0 || sz[1] <=0 || sz[2]<=0) return false;
	double * outimg1d = new double [sz[0] * sz[1] * sz[2]];
	double * eimg1d = 0;
	if(!smooth(outimg1d, inimg1d, sz, SIGMA)) return false;
	if(computeGradience(eimg1d, outimg1d, sz, THRESHOLD) == -1) return false;

	V3DLONG gsz[3] = {10,10,10};
	V3DLONG * grids[3];
	V3DLONG gridnum;
	if(computeEdgeGrid(grids, gridnum, gsz, inimg1d, eimg1d, sz, THRESHOLD) == -1) return false;
	cout<<"gridnum = "<<gridnum<<" , gsz[0] = "<<gsz[0]<<" gsz[1] = "<<gsz[1]<<" gsz[2] = "<<gsz[2]<<endl;
	for(V3DLONG i = 0; i < gridnum; i++)
	{
		//V3DLONG value = (V3DLONG)inimg1d[grids[2][i] * sz[1]* sz[0] + grids[1][i] * sz[0] + grids[0][i]];
		markervector.push_back(MarkerType(grids[0][i], grids[1][i], grids[2][i]));
	}

	if(outimg1d) {delete outimg1d; outimg1d = 0;}
	if(eimg1d) {delete eimg1d; eimg1d = 0;}
	return true;
}


bool marker_to_feature(vector<FeatureType> &vecFeature, vector<MarkerType> vecMarker, unsigned char* inimg1d, V3DLONG sz[3])
{
	if(!vecFeature.empty() || vecMarker.empty() || inimg1d ==0 || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return false;

	V3DLONG n = vecMarker.size();
	vecFeature.resize(n);

	Vector1DSimple<double> momentVec;
	momentVec.resize(5);
	V3DLONG r = 30;

	vector<MarkerType>::iterator it = vecMarker.begin();
	int i = 0;

	while(it != vecMarker.end())
	{
		MarkerType S = *it;
		vecFeature[i].x = S.x;
		vecFeature[i].y = S.y;
		vecFeature[i].z = S.z;
		
		computeGMI(momentVec, (unsigned char *)inimg1d, sz , S.x, S.y, S.z, r);
		double* data1d = momentVec.getData1dHandle();

		for(V3DLONG ii = 0; ii < momentVec.sz0(); ii++) vecFeature[i].descriptor.push_back(data1d[ii]);

		it++;
		i++;
	}
	return true;
}


vector<MarkerType> feature_to_marker(vector<FeatureType> & vecFeature)
{
	vector<MarkerType> vecMarker;
	if(vecFeature.empty()) return vecMarker;
	int nindis = vecFeature.size();
	for(int i = 0; i < nindis; i++)
	{
		V3DLONG x = vecFeature[i].x;
		V3DLONG y = vecFeature[i].y;
		V3DLONG z = vecFeature[i].z;
		vecMarker.push_back(MarkerType(x,y,z));
	}
}

static StatisticFeature statistic_features(vector<FeatureType> &vecFeature)
{
	StatisticFeature sf;
	if(vecFeature.empty()) return sf;	
	int ndims = vecFeature.front().descriptor.size();
	int nindis = vecFeature.size();
	sf.setDims(ndims);
	sf.setIndiNum(nindis);
	sf.init();
	for(int i = 0; i < nindis; i++)
	{
		vector<double> & des = vecFeature[i].descriptor;

		for(int j = 0; j < ndims; j++)
		{
			sf.sum_feat[j] += des[j];
			sf.min_feat[j] = (des[j] < sf.min_feat[j]) ? des[j] : sf.min_feat[j];
			sf.max_feat[j] = (des[j] > sf.max_feat[j]) ? des[j] : sf.max_feat[j];
		}
	}
	for(int j = 0; j < ndims; j++) sf.mean_feat[j] = sf.sum_feat[j] / nindis;
	return sf;
}

bool uniform_features(vector<FeatureType> &vecFeature1, vector<FeatureType> &vecFeature2)
{
	if(vecFeature1.empty() || vecFeature2.empty() || vecFeature1.front().descriptor.size() != vecFeature2.front().descriptor.size()) return false;	

	StatFeat sf1 = statistic_features(vecFeature1);
	StatFeat sf2 = statistic_features(vecFeature2);
	int ndims = sf1.ndims;
	int nindis1 = sf1.nindis;
	int nindis2 = sf2.nindis;
	StatFeat sf(ndims);
	sf.nindis = nindis1 + nindis2;
	cout<<"ndims = "<<ndims<<" nindis1 = "<<nindis1<<" nindis2 = "<<nindis2<<endl;
	cout<<"min max mean sum"<<endl;
	for(int j = 0; j < ndims; j++) 
	{
		sf.min_feat[j] = sf1.min_feat[j] < sf2.min_feat[j] ? sf1.min_feat[j] : sf2.min_feat[j];
		sf.max_feat[j] = sf1.max_feat[j] > sf2.max_feat[j] ? sf1.max_feat[j] : sf2.max_feat[j];
		sf.sum_feat[j] = sf1.sum_feat[j] + sf2.sum_feat[j];
		sf.mean_feat[j] = sf.sum_feat[j] / sf.nindis;
		cout<<j<<" : "<<sf.min_feat[j]<<" "<<sf.max_feat[j]<<" "<<sf.mean_feat[j]<<" "<<sf.sum_feat[j]<<endl;
		//cout<<" | "<<sf1.min_feat[j]<<" "<<sf1.max_feat[j]<<" "<<sf1.mean_feat[j]<<" "<<sf1.sum_feat[j];
		//cout<<" | "<<sf2.min_feat[j]<<" "<<sf2.max_feat[j]<<" "<<sf2.mean_feat[j]<<" "<<sf2.sum_feat[j]<<endl;
	}

	for(int i = 0; i <nindis1; i++)
	{
		for(int j = 0; j < ndims; j++)
		{
			vecFeature1[i].descriptor[j] = (vecFeature1[i].descriptor[j] - sf.min_feat[j])/(sf.max_feat[j] - sf.min_feat[j]);
		}
	}

	for(int i = 0; i < nindis2; i++)
	{
		for(int j = 0; j < ndims; j++)
		{
			vecFeature2[i].descriptor[j] = (vecFeature2[i].descriptor[j] - sf.min_feat[j])/(sf.max_feat[j] - sf.min_feat[j]);
		}
	}
	return true;
}

static bool operator< (const CompareResult &cr1, const CompareResult & cr2)
{
	return cr1.min_dst < cr2.min_dst;
}

bool compare_features(vector<CompareResult> &crs, vector<FeatureType> & vecFeature1, vector<FeatureType> & vecFeature2)
{
//#define USE_RANK_METHOD
	if(!crs.empty() || vecFeature1.empty() || vecFeature2.empty() || vecFeature1.front().descriptor.size() != vecFeature2.front().descriptor.size()) return false;	
	int nindis1 = vecFeature1.size();
	int nindis2 = vecFeature2.size();
	int ndims = vecFeature1.front().descriptor.size();

	vector<CompareResult> crs1; crs1.resize(nindis1);
	vector<CompareResult> crs2; crs2.resize(nindis2);

#ifndef USE_RANK_METHOD
	if(ndims != 5) {cerr<<"ndims != 5"<<endl;return false;}

#define DST(i,j) (pow(vecFeature1[i].descriptor[0]-vecFeature2[j].descriptor[0],2) + \
		pow(vecFeature1[i].descriptor[1]-vecFeature2[j].descriptor[1],2) + \
		pow(vecFeature1[i].descriptor[2]-vecFeature2[j].descriptor[2],2) + \
		pow(vecFeature1[i].descriptor[3]-vecFeature2[j].descriptor[3],2) + \
		pow(vecFeature1[i].descriptor[4]-vecFeature2[j].descriptor[4],2))
	for(int i = 0; i < nindis1; i++)
	{
		double min_dst = numeric_limits<double>::max();
		int min_id = -1;
		for(int j = 0; j < nindis2; j++)
		{
			double dst = DST(i,j);
			if(dst < min_dst) {min_dst = dst; min_id = j;}
		}
		crs1[i].id1 = i;
		crs1[i].id2 = min_id;
		crs1[i].min_dst = min_dst;
	}
	for(int j = 0; j < nindis2; j++)
	{
		double min_dst = numeric_limits<double>::max();
		int min_id = -1;
		for(int i = 0; i < nindis1; i++)
		{
			double dst = DST(i,j);
			if(dst < min_dst) {min_dst = dst; min_id = i;}
		}
		crs2[j].id1 = j;
		crs2[j].id2 = min_id;
		crs2[j].min_dst = min_dst;
	}
#else
	/*
	for(int i = 0; i < nindis1; i++)
	{
		vector<vector<int> > allranks;
		map<int, double> dist;
		for(int d = 0; d < ndims; d++)
		{
			vector<int> rank;
			for(int j=0; j < nindis2; j++) dist[j] = abs(vecFeature1[i].descriptor[d] - vecFeature2[j].descriptor[d]);
			int rankId = 1;
			
		}

	}
	for(int j = 0; j < nindis2; j++)
	{
	}*/
#endif
	for(int i = 0; i < nindis1; i++)
	{
		int j = crs1[i].id2;
		if(crs2[j].id2 == i) crs.push_back(crs1[i]);
	}
	sort(crs.begin(), crs.end());
	return true;
}

vector<MarkerType> extract_id1_markers(vector<CompareResult>&crs, vector<MarkerType> & vecMarker)
{
	vector<MarkerType> extMarker;
	if(crs.empty() || vecMarker.empty()) return extMarker;
	int nresults = crs.size();
	for(int i = 0; i < nresults; i++) extMarker.push_back(vecMarker[crs[i].id1]);
	return extMarker;
}

vector<MarkerType> extract_id2_markers(vector<CompareResult>&crs, vector<MarkerType> & vecMarker)
{
	vector<MarkerType> extMarker;
	if(crs.empty() || vecMarker.empty()) return extMarker;
	int nresults = crs.size();
	for(int i = 0; i < nresults; i++) extMarker.push_back(vecMarker[crs[i].id2]);
	return extMarker;
}
