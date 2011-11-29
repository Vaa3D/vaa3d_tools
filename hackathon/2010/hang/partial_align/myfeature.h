#ifndef MYFEATURE_H_H
#define MYFEATURE_H_H
#include <iostream>
#include <vector>
#include <map>
#include "v3d_interface.h"
#include "v3d_basicdatatype.h"
#include "v3d_message.h"
#include "img_definition.h"

#include "compute_moments.h"

using namespace std;

typedef double REAL;

enum MyFeatureType{NONE_FEATURE, AVERAGE_FEATURE, STD_VAR_FEATURE, SIFT_FEATURE, INVARIANT_MOMENT_FEATURE};


template<class T> class MyFeature : public Image2DSimple<T>
{
	public :
		MyFeature(){m_featureType = NONE_FEATURE;}
		~MyFeature(){};

		V3DLONG size()
		{
			return (V3DLONG)sz1();
		}
		V3DLONG ndims()
		{
			return (V3DLONG)sz0();
		}
		MyFeatureType getFeatureType() const
		{
			return m_featureType;
		}

		template <class TT> int setFeatures(LandmarkList & landmarks, TT *inimg1d, V3DLONG sz[3], MyFeatureType type)
		{
			if(! inimg1d || sz[0] <= 0 || sz[1] <= 0 || sz[2] <= 0) return -1;
			TT *** data3d = 0;
			try
			{
				new3dpointer(data3d, sz[0], sz[1], sz[2], inimg1d);
			}
			catch(...)
			{
				if(data3d) delete3dpointer(data3d, sz[0], sz[1], sz[2]);
				return -1;
			}

			if(type == AVERAGE_FEATURE)
			{
				cout<<"set average intensity feature"<<endl;
				m_featureType = type;
				V3DLONG sz0 = 1;
				V3DLONG sz1 = landmarks.size();
				allocateMemory(sz0, sz1);
				T ** features = getData2dHandle();
				double radius = 5.0;
				for(V3DLONG i = 0; i < sz1; i++)
				{
					V3DLONG posx = landmarks[i].x;
					V3DLONG posy = landmarks[i].y;
					V3DLONG posz = landmarks[i].z;

					V3DLONG minx = posx - radius >= 0 ? posx - radius : 0;
					V3DLONG maxx = posx + radius < sz[0] ? posx + radius : sz[0];

					V3DLONG miny = posy - radius >= 0 ? posy - radius : 0;
					V3DLONG maxy = posy + radius < sz[1] ? posy + radius : sz[1];

					V3DLONG minz = posz - radius >= 0 ? posz - radius : 0;
					V3DLONG maxz = posz + radius < sz[2] ? posz + radius : sz[2];

					V3DLONG count = 0;
					double sum = 0;
					for(V3DLONG x = minx; x <= maxx; x++)
					{
						V3DLONG dx = x - posx;
						for(V3DLONG y = miny; y <= maxy; y++)
						{
							V3DLONG dy = y - posy;
							for(V3DLONG z = minz; z < maxz; z++)
							{
								V3DLONG dz = z - posz;
								if(dx*dx + dy*dy + dz*dz <= radius*radius)
								{
									sum += data3d[z][y][x]; 
									count++;
								}
							}
						}
					}
					features[i][0] = (T)(sum/count);		
				}
			}
			else if(type == STD_VAR_FEATURE)
			{
				cout<<"set stand variance feature"<<endl;
				m_featureType = type;
				V3DLONG sz0 = 2;
				V3DLONG sz1 = landmarks.size();
				allocateMemory(sz0, sz1);
				T ** features = getData2dHandle();
				double radius = 5.0;
				for(V3DLONG i = 0; i < sz1; i++)
				{
					V3DLONG posx = landmarks[i].x;
					V3DLONG posy = landmarks[i].y;
					V3DLONG posz = landmarks[i].z;

					V3DLONG minx = posx - radius >= 0 ? posx - radius : 0;
					V3DLONG maxx = posx + radius < sz[0] ? posx + radius : sz[0];

					V3DLONG miny = posy - radius >= 0 ? posy - radius : 0;
					V3DLONG maxy = posy + radius < sz[1] ? posy + radius : sz[1];

					V3DLONG minz = posz - radius >= 0 ? posz - radius : 0;
					V3DLONG maxz = posz + radius < sz[2] ? posz + radius : sz[2];

					V3DLONG count = 0;
					double sum_square = 0.0;
					double sum = 0.0;
					for(V3DLONG x = minx; x <= maxx; x++)
					{
						V3DLONG dx = x - posx;
						for(V3DLONG y = miny; y <= maxy; y++)
						{
							V3DLONG dy = y - posy;
							for(V3DLONG z = minz; z < maxz; z++)
							{
								V3DLONG dz = z - posz;
								if(dx*dx + dy*dy + dz*dz <= radius*radius)
								{
									V3DLONG intensity = data3d[z][y][x];
									sum += intensity;
									sum_square += intensity*intensity;
									count++;
								}
							}
						}
					}
					double avg = sum / count;
					double std_var = sqrt((sum_square - count * avg * avg)/(count - 1));
					features[i][0] = (T)avg;
					features[i][1] = (T)std_var;
				}
			}
			else if(type == SIFT_FEATURE)
			{
				cout<<"set scale invariant feature transform (SIFT) feature"<<endl;
				m_featureType = type;
			}
			else if(type == INVARIANT_MOMENT_FEATURE)
			{
				cout<<"set invariant methods feature"<<endl;
				m_featureType = type;
				V3DLONG sz0 = 5;
				V3DLONG sz1 = landmarks.size();
				allocateMemory(sz0, sz1);
				T ** features = getData2dHandle();

				Vector1DSimple<double> momentVec;
				momentVec.resize(sz0);
				V3DLONG r = 30;

				for(V3DLONG i = 0; i < sz1; i++)
				{
					V3DLONG x0 = landmarks[i].x;
					V3DLONG y0 = landmarks[i].y;
					V3DLONG z0 = landmarks[i].z;

					computeGMI(momentVec, inimg1d, sz, x0, y0, z0, r);

					double* data1d = momentVec.getData1dHandle();
					for(V3DLONG ii = 0; ii < sz0; ii++) features[i][ii] = (T)data1d[ii];
				}
			}
			else 
			{
				m_featureType = NONE_FEATURE;
				return -1;
			}
			return 1;
		}
		void printFeatures()
		{
			QString output;
			T ** features = getData2dHandle();
			for(V3DLONG j = 0; j < sz1(); j++)
			{
				for(V3DLONG i = 0; i < sz0(); i++)
				{
					//cout<<m_features[i][j]<<" ";
					output.append(QObject::tr("%1 ").arg(features[j][i]));
				}
				//cout<<endl;
				output.append("\n");
			}
			v3d_msg(output);
		}

	private :
		MyFeatureType m_featureType;
};

template <class T> int get_matched_feature_list(V3DLONG* &match_list, V3DLONG &match_sz, MyFeature<T>& sub_feat, MyFeature<T>& tar_feat)
{
	if(sub_feat.getFeatureType() != tar_feat.getFeatureType() || match_list != 0 || match_sz <= 0 || match_sz > tar_feat.size() || sub_feat.ndims() != tar_feat.ndims())
	{
		cerr<<"compare two features with different type or match_list != 0"<<endl;
		return -1;
	}
	V3DLONG ndims = sub_feat.ndims();
	V3DLONG sub_sz = sub_feat.size();
	V3DLONG tar_sz = tar_feat.size();
	
	try
	{
		match_list = new V3DLONG[tar_sz];
	}
	catch(...)
	{
		if(match_list) {delete[] match_list; match_list = 0;}
		cerr<<"error when alloc memory for match_list"<<endl;
		return -1;
	}

	T * min_feat = new T[ndims];
	T * max_feat = new T[ndims];
	double * dist_factor = new double[ndims];

	int i = 0;
	int j = 0;
	T ** sub_data2d = sub_feat.getData2dHandle();
	T ** tar_data2d = tar_feat.getData2dHandle();
	for(i = 0; i < sub_feat.size(); i++)
	{
		if(i == 0)
		{
			for(j = 0; j < ndims; j++) min_feat[j] = max_feat[j] = sub_data2d[i][j];
		}
		else
		{
			for(j = 0; j < ndims; j++)
			{
				min_feat[j] = sub_data2d[i][j] < min_feat[j] ? sub_data2d[i][j] : min_feat[j];
				max_feat[j] = sub_data2d[i][j] > max_feat[j] ? sub_data2d[i][j] : max_feat[j];
			}
		}
	}
	
	for(i = 0; i < tar_feat.size(); i++)
	{
		for(j = 0; j < ndims; j++)
		{
			min_feat[j] = tar_data2d[i][j] < min_feat[j] ? tar_data2d[i][j] : min_feat[j];
			max_feat[j] = tar_data2d[i][j] > max_feat[j] ? tar_data2d[i][j] : max_feat[j];
		}
	}
	for(j = 0; j < ndims; j++) 
	{
		dist_factor[j] = (max_feat[j] - min_feat[j]) * (max_feat[j] - min_feat[j]);
		if(dist_factor[j] > 0.0)  dist_factor[j] = 1.0 / dist_factor[j];
	}

	if(match_sz == tar_sz)
	{
		V3DLONG i,j,d;
		V3DLONG count = 0;

		for(i = 0; i < tar_sz; i++)
		{
			double min_dist = 0.0;
			V3DLONG min_id = -1;
			for(j = 0; j < sub_sz; j++)
			{
				double dist = 0.0;
				for(d = 0; d < ndims; d++) dist += (tar_data2d[i][d] - sub_data2d[j][d]) * (tar_data2d[i][d] - sub_data2d[j][d]) * dist_factor[d];
				if( min_id == -1){ min_id = j; min_dist = dist;}
				else
				{
					if(dist < min_dist)
					{
						min_id = j;
						min_dist = dist;
					}
				}
			}
			match_list[count++] = min_id;
		}
	}
	else
	{
		V3DLONG i,j,d;
		map<double, V3DLONG> dist_map;

		T ** sub_data2d = sub_feat.getData2dHandle();
		T ** tar_data2d = tar_feat.getData2dHandle();

		for(i = 0; i < tar_sz; i++)
		{
			double min_dist = 0.0;
			V3DLONG min_id = -1;
			for(j = 0; j < sub_sz; j++)
			{
				double dist = 0.0;
				for(d = 0; d < ndims; d++) dist += (tar_data2d[i][d] - sub_data2d[j][d]) * (tar_data2d[i][d] - sub_data2d[j][d]) * dist_factor[d];
				if( min_id == -1){ min_id = j; min_dist = dist;}
				else
				{
					if(dist < min_dist)
					{
						min_id = j;
						min_dist = dist;
					}
				}
			}
			dist_map[min_dist] = min_id;
		}
		V3DLONG count = 0;
		for(map<double, V3DLONG>::iterator it = dist_map.begin(); it != dist_map.end(); it++)
		{
			match_list[count++] = it->second;
		}
	}
	return 1;
}
#endif
