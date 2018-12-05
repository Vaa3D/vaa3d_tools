//calculate_cover_scores.cpp
// by Hanchuan Peng
// 2008-2012

#include "v3dneuron_gd_tracing.h"


double calculate_overlapping_ratio_n1(const V_NeuronSWC_unit & n1, const V_NeuronSWC_unit & n2, unsigned char ***imap,  V3DLONG sz[3], double trace_z_thickness)
{
	//in this function I intentionally do not check if n2 has been marked as to be deleted (.nchild<0), as this function should be general
	
	V3DLONG i,j,k;
	V3DLONG R = ceil(n1.r);
	double r12 = n1.r*n1.r, r22 = n2.r*n2.r;
	double curr_k, curr_j, curr_i;
	double n1x = n1.x, n1y = n1.y, n1z = n1.z;
	V3DLONG cx, cy, cz;
	double n1_totalsample = 0, n2_includedsample = 0, n1_totalpixel=0;
	for (k=-R/trace_z_thickness;k<=R/trace_z_thickness;k++)
	{
		cz = V3DLONG(n1z+k+0.5); if (cz<0 || cz>=sz[2]) continue;
		curr_k = double(k)*k;
		for (j=-R;j<=R;j++)
		{
			cy = V3DLONG(n1y+j+0.5); if (cy<0 || cy>=sz[1]) continue;
			if ((curr_j = double(j)*j+curr_k) > r12)
				continue;
			
			for (i=-R;i<=R;i++)
			{
				cx = V3DLONG(n1x+i+0.5); if (cx<0 || cx>=sz[0]) continue;
				if ((curr_i = double(i)*i+curr_j) > r12)
					continue;
				
				n1_totalpixel += 1;
				n1_totalsample += imap[cz][cy][cx];
				if ((cz-n2.z)*(cz-n2.z)+(cy-n2.y)*(cy-n2.y)+(cx-n2.x)*(cx-n2.x) <= r22)
					n2_includedsample += imap[cz][cy][cx];
			}
		}
	}
	
	if (n1_totalpixel==0)
	{
		v3d_msg("total # sample is 0. This means radius is wrong. Should never see this!!! Check data and program. \n", 0);
		printf("total pixel=%ld, R=%ld n1.r=%5.3f n1x=%5.3f n1y=%5.3f n1z=%5.3f sz[0]=%ld sz[1]=%ld sz[2]=%ld ", V3DLONG(n1_totalpixel), R, n1.r, n1x, n1y, n1z, sz[0], sz[1], sz[2]);
		return 1; //return 1 because this means this n1 node should be eleminated anyway
	}
	
	return (n1_totalsample<=0) ? 1.0 : (n2_includedsample/n1_totalsample);
}

double calculate_overlapping_ratio_n1(const V_NeuronSWC_unit & n1, vector<V_NeuronSWC_unit> & mUnit, map<double,V3DLONG> & index_map, list<V3DLONG> & mylist_n, unsigned char ***imap,  V3DLONG sz[3], double trace_z_thickness)
{
	if (mylist_n.size()<=0) //return 0 if the covering list is empty
		return 0;
	if (mylist_n.size()==1 && mUnit[index_map[*(mylist_n.begin())]].n==n1.n) //return 0 if there is no other node covering this target node
		return 0;
	
	V3DLONG i,j,k;
	V3DLONG R = ceil(n1.r);
	double r12 = n1.r*n1.r;
	double curr_k, curr_j, curr_i;
	double n1x = n1.x, n1y = n1.y, n1z = n1.z;
	V3DLONG cx, cy, cz;
	double n1_totalsample = 0, n2_includedsample = 0, n1_totalpixel=0;
	
	//for (k=-R/trace_z_thickness;k<=R/trace_z_thickness;k++)
	for (k=-R;k<=R;k++)
	{
		cz = V3DLONG(n1z+k+0.5); if (cz<0 || cz>=sz[2]) continue;
		curr_k = double(k)*k;
		for (j=-R;j<=R;j++)
		{
			cy = V3DLONG(n1y+j+0.5); if (cy<0 || cy>=sz[1]) continue;
			if ((curr_j = double(j)*j+curr_k) > r12)
				continue;
			
			for (i=-R;i<=R;i++)
			{
				cx = V3DLONG(n1x+i+0.5); if (cx<0 || cx>=sz[0]) continue;
				if ((curr_i = double(i)*i+curr_j) > r12)
					continue;
				
				n1_totalpixel += 1;
				n1_totalsample += imap[cz][cy][cx];
				
				list<V3DLONG>::iterator it = mylist_n.begin();
				for ( it=mylist_n.begin() ; it != mylist_n.end(); it++ )
				{
					V_NeuronSWC_unit & n2 = mUnit[index_map[*it]];
					if (n2.n == n1.n) //the same node, then skip
						continue;
					if (n2.nchild<0) //then n2 has already be labeled to be removed. then it should not participate computation
						continue;
					
					if ((cz-n2.z)*(cz-n2.z)+(cy-n2.y)*(cy-n2.y)+(cx-n2.x)*(cx-n2.x) <= n2.r*n2.r)
					{
						n2_includedsample += imap[cz][cy][cx];
						break;
					}
				}
				
			}
		}
	}
	
	if (n1_totalpixel==0)
	{
		v3d_msg("total # sample is 0. This means radius is wrong. Should never see this!!! Check data and program. \n", 0);
		printf("total pixel=%ld, R=%ld n1.r=%5.3f n1x=%5.3f n1y=%5.3f n1z=%5.3f sz[0]=%ld sz[1]=%ld sz[2]=%ld ", V3DLONG(n1_totalpixel), R, n1.r, n1x, n1y, n1z, sz[0], sz[1], sz[2]);
		return 1; //return 1 because this means this n1 node should be eleminated anyway
	}
	
	return (n1_totalsample<=0) ? 1.0 : (n2_includedsample/n1_totalsample);
}


double computeEdgeMeanVal(const V_NeuronSWC_unit & n1, const V_NeuronSWC_unit & n2, unsigned char ***imap,  V3DLONG sz[3])
{
    double s = 0;
    
	V3DLONG i,j,k;
	double n1x = n1.x, n1y = n1.y, n1z = n1.z;
	double n2x = n2.x, n2y = n2.y, n2z = n2.z;
    double len = sqrt((n1x-n2x)*(n1x-n2x) + (n1y-n2y)*(n1y-n2y) + (n1z-n2z)*(n1z-n2z));
    double dx=0, dy=0, dz=0;
    if (len<=0)
    {
        return s = (n1x>=0 && n1x<=sz[0]-1 && n1y>=0 && n1y<=sz[1]-1 && n1z>=0 && n1z<=sz[2]-1) ? imap[int(n1z+0.5)][int(n1y+0.5)][int(n1x+0.5)] : 0; //use int() to force to use floor()
    }
    else
    {
        dx = (n2x-n1x)/len;
        dy = (n2y-n1y)/len;
        dz = (n2z-n1z)/len;
    }
    
    V3DLONG cnt;
    for (int i=0, cnt=0;i<len;i++)
    {
        double cx = n1x + dx*i, cy = n1y + dy*i, cz = n1z + dz*i;
        if (cx>=0 && cx<=sz[0]-1 && cy>=0 && cy<=sz[1]-1 && cz>=0 && cz<=sz[2]-1)
        {
            s += imap[int(cz+0.5)][int(cy+0.5)][int(cx+0.5)]; //use int() to force to use floor()
            cnt++;
        }
    }
    
    return s = (cnt>0) ? s/cnt : s;    
}

