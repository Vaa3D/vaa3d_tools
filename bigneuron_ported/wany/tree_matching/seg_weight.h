#ifndef __SEG_WEIGHT_H__
#define __SEG_WEIGHT_H__

#include <math.h>

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#define MAX_DOUBLE 1.7976931348623158e+308

double edit_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, int s1, int e1, int s2, int e2);
double euc_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, int s1, int e1, int s2, int e2);

//min seg-distance
double seg_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2)
{
	double weight = MAX_DOUBLE;
	for (int i=0;i<seg1.size();i++)
		for (int j=0;j<seg2.size();j++)
			weight = MIN(dist(*seg1[i], *seg2[j]), weight);
	return weight;
}

//min seg-distance with alignment history stored in matching_res
double seg_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res)
{
	double weight = 0;
	for (int i=0;i<seg1.size();i++)
	{
		double weight_point = MAX_DOUBLE;
		int min_tgt_idx = -1;
		for (int j=0;j<seg2.size();j++)
		{
			if (weight_point >= dist(*seg1[i], *seg2[j]) )
			{
				weight_point = dist(*seg1[i], *seg2[j]);
				min_tgt_idx = j;
			}
		}
		matching_res.push_back(pair<int, int>(i, min_tgt_idx));
	}
	
	return weight;
}

//curve alignment method
//optional of euc_dist (local) and euc_dist (global) weight
//Reference: Sebastian et al, On Aligning Curves, 2003
double seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2)
{
	if (seg1.size()<=1 || seg2.size()<=1) //very short branch can map to any branch without constrains
		return 0;
	int k=0, l=0;
	vector<vector<double> > matrix(seg1.size()-1, vector<double>(seg2.size()-1, MAX_DOUBLE));
	matrix[0][0] = euc_dist(seg1, seg2, 0, 1, 0, 1);
	for (int i=0;i<seg1.size()-1;i++)
	{
		for (int j=0;j<seg2.size()-1;j++)
		{
			k = 1; l = 0;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 0; l = 1;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 1; l = 1;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 2; l = 1;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 3; l = 1;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 1; l = 2;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 1; l = 3;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 2; l = 3;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
			k = 3; l = 2;
			if (i-k>=0 && j-l>=0)
				matrix[i][j] = MIN(matrix[i][j], matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1));
		}
	}
	return matrix.back().back();
}

//curve alignment method with align history stored in matching_res
//optional of euc_dist (local) and euc_dist (global) weight
//Reference: Sebastian et al, On Aligning Curves, 2003
double seg_weight(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, vector<pair<int, int> > & matching_res)
{
	if (seg1.size()<=1 || seg2.size()<=1) //single-point branch can map to any branch without constrains
		return 0;
	int k=0, l=0;
	vector<vector<double> > matrix(seg1.size()-1, vector<double>(seg2.size()-1, MAX_DOUBLE));
	vector<vector<pair<int, int> > > last_point(seg1.size()-1, vector<pair<int, int> >(seg2.size()-1, pair<int, int>(-1, -1)));
	matrix[0][0] = euc_dist(seg1, seg2, 0, 1, 0, 1);
	for (int i=0;i<seg1.size()-1;i++)
	{
		for (int j=0;j<seg2.size()-1;j++)
		{
			k = 1; l = 0;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			k = 0; l = 1;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			k = 1; l = 1;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
	/*		k = 2; l = 1;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			k = 3; l = 1;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			k = 1; l = 2;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			k = 1; l = 3;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			k = 2; l = 3;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			k = 3; l = 2;
			if (i-k>=0 && j-l>=0)
			{
				if (matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1) <= matrix[i][j])
				{
					matrix[i][j]  = matrix[i-k][j-l] + euc_dist(seg1, seg2, i-k, i+1, j-l, j+1);
					last_point[i][j].first = i-k;
					last_point[i][j].second = j-l;
				}
			}
			*/
		}
	}
	matching_res.push_back(pair<int, int>(seg1.size()-1, seg2.size()-1));

	
	int lastp1 = seg1.size()-2;
	int lastp2 = seg2.size()-2;
	int p1, p2;
	do
	{
		p1 = last_point[lastp1][lastp2].first;
		p2 = last_point[lastp1][lastp2].second;
		if (p1<0 || p2<0)
			break;
		matching_res.push_back(pair<int,int>(p1, p2));
		lastp1 = p1;
		lastp2 = p2;
	}
	while (true);

	return matrix.back().back();
}

//edit distance between sub-segment [s1,e1] in seg1 and [s2,e2] in seg2
//currently not in use because the dismatch in local prperties
double edit_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, int s1, int e1, int s2, int e2)
{
	double R = 0.1;
	double d1 = 0, d2 = 0;
	for (int i=s1;i<e1;i++)
		d1 += dist(*seg1[i], *seg1[i+1]);
	for (int i=s2;i<e2;i++)
		d2 += dist(*seg2[i], *seg2[i+1]);
	double d_dist = fabs(d1-d2);

	double t1, t2;
	if (e1-s1<=1) t1 = 0;
	else
	{
		MyMarker t1s, t1e;
		t1s.x = seg1[s1+1]->x - seg1[s1]->x;
		t1s.y = seg1[s1+1]->y - seg1[s1]->y;
		t1s.z = seg1[s1+1]->z - seg1[s1]->z;
		t1e.x = seg1[e1]->x - seg1[e1-1]->x;
		t1e.y = seg1[e1]->y - seg1[e1-1]->y;
		t1e.z = seg1[e1]->z - seg1[e1-1]->z;
		t1 = acos((t1s.x*t1e.x + t1s.y*t1e.y + t1s.z*t1e.z) / (dist(*seg1[s1+1], *seg1[s1]) * dist(*seg1[e1], *seg1[e1-1])));
	}
	if (e2-s2<=1) t2 = 0;
	else
	{
		MyMarker t2s, t2e;
		t2s.x = seg2[s2+1]->x - seg2[s2]->x;
		t2s.y = seg2[s2+1]->y - seg2[s2]->y;
		t2s.z = seg2[s2+1]->z - seg2[s2]->z;
		t2e.x = seg2[e2]->x - seg2[e2-1]->x;
		t2e.y = seg2[e2]->y - seg2[e2-1]->y;
		t2e.z = seg2[e2]->z - seg2[e2-1]->z;
		t2 = acos((t2s.x*t2e.x + t2s.y*t2e.y + t2s.z*t2e.z) / (dist(*seg2[s2+1], *seg2[s2]) * dist(*seg2[e2], *seg2[e2-1])));
	}
	double t_dist = fabs(t1-t2);
	//	cout<<"s1 = "<<s1<<" e1 = "<<e1<<" s2 = "<<s2<<" e2 = "<<e2<<" t_dist = "<<t_dist<<endl;

	return d_dist+R*t_dist;
}

//average Euclidean distance between sub-segment [s1,e1] in seg1 and [s2,e2] in seg2
double euc_dist(vector<MyMarker*> & seg1, vector<MyMarker*> & seg2, int s1, int e1, int s2, int e2)
{
	double result = 0;
	for (int i=s1;i<=e1;i++)
		for (int j=s2;j<=e2;j++)
			result += dist(*seg1[i], *seg2[j]);
	result /= (e1-s1+1)*(e2-s2+1);

	return result;
}

#endif
