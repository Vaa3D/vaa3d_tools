//v3dneuron_tracing_shortestpath.h
// by Hanchuan Peng
// 2008-2010
// separated from main V3D program on May 24, 2010 for command line program. by Hanchuan Peng
// renamed as v3dneuron_gd_tracing.h. Nov 30, 2010
// 2011-09-06. add function proj_trace_smooth_downsample_last_traced_neuron() for mmUnit

#ifndef __V3DNEURON_GD_TRACING_H__
#define __V3DNEURON_GD_TRACING_H__

#include <QtGui>
#include <vector>

#include "basic_landmark.h"
#include "basic_surf_objs.h"
#include "v3d_curvetracepara.h"
#include "v_neuronswc.h"

#include "gd.h"

//main interface

NeuronTree v3dneuron_GD_tracing(unsigned char ****p4d, V3DLONG sz[4], 
								LocationSimple & p0, vector<LocationSimple> & pp, 
								CurveTracePara & trace_para, double trace_z_thickness);

//enabling functions

V_NeuronSWC_list trace_one_pt_to_N_points_shortestdist(unsigned char ****p4d, V3DLONG sz[4], 
																   LocationSimple & p0, vector<LocationSimple> & pp, 
																   CurveTracePara & trace_para, double trace_z_thickness, vector< vector<V_NeuronSWC_unit> > & mmUnit); 


int mergeback_mmunits_to_neuron_path(int n_end_nodes, vector< vector<V_NeuronSWC_unit> > & mmUnit, V_NeuronSWC_list & tNeuron);

NeuronTree convertNeuronTreeFormat(V_NeuronSWC_list & tracedNeuron);

bool proj_trace_smooth_downsample_last_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, int seg_begin, int seg_end);
bool proj_trace_smooth_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], vector< vector<V_NeuronSWC_unit> > & mmUnit, CurveTracePara & trace_para, int seg_begin, int seg_end);

bool proj_trace_compute_radius_of_last_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness, bool b_smooth);
bool proj_trace_compute_radius_of_last_traced_neuron(unsigned char ****p4d, V3DLONG sz[4], vector< vector<V_NeuronSWC_unit> >& mmUnit, CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness, bool b_smooth);

bool proj_trace_mergeAllClosebyNeuronNodes(V_NeuronSWC_list & tracedNeuron);

bool filterThinSegmentsInImage(unsigned char ****p4d, V3DLONG sz[4], V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, int seg_begin, int seg_end, float myzthickness);

bool filterNodesByFindingBestCovers(unsigned char ****p4d, V3DLONG sz[4], 
									V3DLONG bx0, V3DLONG bx1,
									V3DLONG by0, V3DLONG by1,
									V3DLONG bz0, V3DLONG bz1,
									V_NeuronSWC_list & tracedNeuron, CurveTracePara & trace_para, float myzthickness);

bool est_pixel_radius(unsigned char *p1d, V3DLONG sz[3], float imgTH, float myzthickness, bool b_est_in_xyplaneonly, float *p1dr);//p1dr the detected radius
bool dt_v3dneuron(unsigned char *p1d, V3DLONG sz[3], float imgTH, bool b_to_background, bool b_3d_dt, float *p1dt); //imgTH specify the foreground

//

vector<LocationSimple> find_valid_tip_via_ignore_short_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char * imap, float *dmap, V3DLONG sz[3]);

bool condense_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness);

V3DLONG pruning_covered_leaf_single_cover(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness);
V3DLONG pruning_covered_leaf_multi_covers(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness);
V3DLONG pruning_covered_leaf_closebyfake_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness, double imgTH);
V3DLONG pruning_internodes(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness);
V3DLONG pruning_branch_nodes(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness);

double calculate_overlapping_ratio_n1(const V_NeuronSWC_unit & n1, const V_NeuronSWC_unit & n2, unsigned char ***imap,  V3DLONG sz[3], double trace_z_thickness);
double calculate_overlapping_ratio_n1(const V_NeuronSWC_unit & n1, vector<V_NeuronSWC_unit> & mUnit, map<double,V3DLONG> & index_map, 
											 list<V3DLONG> & mylist_n, unsigned char ***imap,  V3DLONG sz[3], double trace_z_thickness);
double computeEdgeMeanVal(const V_NeuronSWC_unit & n1, const V_NeuronSWC_unit & n2, unsigned char ***imap,  V3DLONG sz[3]);

V3DLONG pruning_artifacial_branches(vector< vector<V_NeuronSWC_unit> >& mmUnit, unsigned char ***imap,  float *dmap, V3DLONG sz[3], double trace_z_thickness, double vis_threshold);


template <class T> bool median_filter_3d(T ***p3d, V3DLONG sz[3], double r); //in-place median filter
template <class T> bool median_filter_3d(T ***p3d, V3DLONG sz[3], double r) 
{
	if (!p3d || !sz || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || r<0)
		return false;
	
	V3DLONG tlen = sz[0]*sz[1]*sz[2];
	V3DLONG sz2 = sz[2], sz1 = sz[1], sz0 = sz[0], pagesz=sz[0]*sz[1];
	T * res = 0;
	
	try {res = new T [tlen];}catch(...) {fprintf(stderr,"Unable to allocate memory in median_filter().\n"); return false;}	
	
	V3DLONG R = ceil(r);
	double r2 = r*r;
	
	V3DLONG i,j,k;
	double curr_k, curr_j, curr_i;
	V3DLONG cx, cy, cz;

	V3DLONG kk,jj,ii;
	for (kk=0; kk<sz2; kk++)
		for (jj=0;jj<sz1; jj++)
			for (ii=0;ii<sz0; ii++)
			{
				//for each voxel, find the neighborhood
				
				vector <T> myvector; myvector.clear();
				V3DLONG totalsample = 0;
				for (k=-R;k<=R;k++)
				{
					cz = V3DLONG(kk+k); if (cz<0 || cz>=sz2) continue;
					curr_k = double(k)*k;
					for (j=-R;j<=R;j++)
					{
						cy = V3DLONG(jj+j); if (cy<0 || cy>=sz1) continue;
						if ((curr_j = double(j)*j+curr_k) > r2)
							continue;
						
						for (i=-R;i<=R;i++)
						{
							cx = V3DLONG(ii+i); if (cx<0 || cx>=sz0) continue;
							if ((curr_i = double(i)*i+curr_j) > r2)
								continue;
							
							totalsample += 1;
							
							myvector.push_back(p3d[cz][cy][cx]);
						}
					}
				}
				
				// using default comparison (operator <):
				sort(myvector.begin(), myvector.end());
				res[kk*pagesz+jj*sz0+ii] = myvector[totalsample/2];
				
			}  // end of voxel loop
	
	for (kk=0; kk<sz2; kk++)
		for (jj=0;jj<sz1; jj++)
			for (ii=0;ii<sz0; ii++)
			{
				p3d[kk][jj][ii] = res[kk*pagesz+jj*sz0+ii];
			}
	
	if (res) {delete []res; res=0;}
	return true;
}

template <class T> double corrcoef(const vector<T> & a, const vector<T> & b)
{
	V3DLONG na = a.size();
	V3DLONG nb = b.size();
	if (na!=nb || na<=0)
	{
		printf("Invalid data in corrcoef()\n");
		return 0;
	}
	
	double ma=0, mb=0;
	V3DLONG i;
	
	for (i=0;i<na;i++)
	{
		ma += a[i];
		mb += b[i];
	}
	
	ma /= na;
	mb /= nb;
	
	double sa=0, sb=0, sab=0;
	for (i=0;i<na;i++)
	{
		double ta = a[i]-ma;
		double tb = b[i]-mb;
		sab += ta*tb;
		sa += ta*ta;
		sb += tb*tb;
	}
	
	return (sab/sqrt(sa*sb));
}

#endif
