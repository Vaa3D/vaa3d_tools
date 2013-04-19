/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/




//by Hanchuan Peng
//090516
//090518: add ParaShortestPath struct
//100327: add find_shortest_path_graphpointset
//101212: add some block operation functions
//101219: update the deformable curve part
//101221: renamed as gd.h
//110115: add rearrange_and_remove_labeled_deletion_nodes_mmUnit()

#ifndef __NEURON_TRACING_H__
#define __NEURON_TRACING_H__

#include <math.h>

#include <iostream>
#include <vector>
#include <algorithm> //added 100417 for Ubuntu, PHC

#include "v_neuronswc.h"
#include "bdb_minus.h"

#if defined(__LP64__) || defined(__LLP64__)
typedef double     Weight;
typedef V3DLONG    Node;
#else
typedef float     Weight;
typedef int       Node;
#endif
typedef std::pair<Node, Node> Edge;


struct ParaShortestPath
{
	int node_step; //should be >=1 && odd.
	int outsample_step;
	int smooth_winsize;
	int edge_select;  //0 -- only use length 1 edge(optimal for small step), 1 -- plus diagonal edge
	int background_select; //0 -- no background, 1 -- compute background threshold
	double imgTH;
	double visible_thresh;

	ParaShortestPath()
	{
		node_step = 3; //should be >=1
		outsample_step = 2;
		smooth_winsize = 5;
		edge_select = 0;  //0 -- bgl_shortest_path(), 1 -- phc_shortest_path()
		background_select = 1;
		imgTH = 0; //do not tracing image background
        visible_thresh = 30;
	}
};

class BDB_Minus_Prior_Parameter
{
public:
  double f_prior, f_smooth, f_length; //the coefficients for the prior force, smoothness and length forces, respectively.
  double Kfactor; //the factor in "mean+Kfactor*sigma" of an image that would be used to find the backbone
  int nloops;

  BDB_Minus_Prior_Parameter()
  {
    f_prior=0.2;
	f_smooth=0.1;
	f_length=0.1;
	Kfactor=1.0;
	nloops=100;
  }
};

//#define VISIBLE_THRESHOLD 30 //change back to 30 from 10, to be consistent with APP1 code v1.70, which generate good result for Rubin_raw1 test data
//replace VISIBLE_THRESHOLD using the new "visible_thresh" field. 2013-02-10

struct VPoint 
{
	double *v;
	V3DLONG n;
	VPoint() {v=0; n=0;}
	VPoint(V3DLONG m) {if (m<=0) {v=0;n=0; return;} n=m; v=new double[n];}
	VPoint(V3DLONG m, double inival) {if (m<=0) {v=0;n=0; return;} n=m; v=new double[n]; for (V3DLONG i=0;i<n;i++) v[i]=inival;}
	~VPoint() { if (v) {delete []v; v=0;} n=0;}
	
	bool isValid() {return (n<=0 || !v)? false : true;}
	bool isComparable(VPoint &t) {if (!t.isValid() || !isValid()) return false; return (t.n==n)?true:false;}
	bool isComparable(VPoint *t) {if (!t->isValid() || !isValid()) return false; return (t->n==n)?true:false;}
	
	double abs() {double s=0;for (V3DLONG i=0;i<n; i++) s+=v[i]*v[i]; return sqrt(s);}
	bool simpleProduct(VPoint &t) {if (isComparable(t)==false) return false; for (V3DLONG i=0;i<n;i++) v[i]*=t.v[i]; return true;} 
	bool simpleProduct(VPoint *t) {if (isComparable(t)==false) return false; for (V3DLONG i=0;i<n;i++) v[i]*=t->v[i]; return true;} 
	double innerProduct(VPoint &t) { if (!simpleProduct(t)) return 0; else return sqrt(abs());}
	double innerProduct(VPoint *t) { if (!simpleProduct(t)) return 0; else return sqrt(abs());}
};


// return error message, NULL is no error
char* find_shortest_path_graphimg(unsigned char ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel, //image
								  float zthickness, // z-thickness for weighted edge
								  //const V3DLONG box[6],  //bounding box
								  V3DLONG bx0, V3DLONG by0, V3DLONG bz0, V3DLONG bx1, V3DLONG by1, V3DLONG bz1, //bounding box (ROI)
								  float x0, float y0, float z0,       // start node
								  int n_end_nodes,                    // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
								  float x1[], float y1[], float z1[],    // all end nodes
								  vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
								  const ParaShortestPath & para);

char* find_shortest_path_graphimg(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, //image
								  float zthickness, // z-thickness for weighted edge
								  //const V3DLONG box[6],  //bounding box
								  V3DLONG bx0, V3DLONG by0, V3DLONG bz0, V3DLONG bx1, V3DLONG by1, V3DLONG bz1, //bounding box (ROI)
								  float x0, float y0, float z0,       // start node
								  int n_end_nodes,                    // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
								  float x1[], float y1[], float z1[],    // all end nodes
								  vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
								  const ParaShortestPath & para);

char* find_shortest_path_graphpointset(V3DLONG n_all_nodes,
									   double xa[], double ya[], double za[], double va[], //the coordinates and values of all nodes
									   float zthickness, // z-thickness for weighted edge
									   std::vector<Edge> 	edge_array0,
									   V3DLONG ind_startnode,        // start node's index 
									   V3DLONG n_end_nodes0,          // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
									   V3DLONG ind_end_nodes0[],      // all end nodes' indexes
									   vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
									   const ParaShortestPath & para);

void rearrange_and_remove_labeled_deletion_nodes_mmUnit(vector< vector<V_NeuronSWC_unit> >& mmUnit); //remove those nchild < 0 and update indexes

// assume root node at tail of vector (result of back tracing)
char* merge_back_traced_paths(vector< vector<V_NeuronSWC_unit> >& mmUnit);

bool fit_radius_and_position(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
							vector <V_NeuronSWC_unit> & mUnit, bool b_move_position, float zthickness=1.0, bool b_est_in_xyplaneonly=false, double vis_threshold=30.0);
bool fit_radius_and_position(unsigned char ****img4d, V3DLONG sz[4],
							 vector <V_NeuronSWC_unit> & mCoord, bool b_move_position, float zthickness, bool b_est_in_xyplaneonly=false, double vis_threshold=30.0);

double getImageMaxValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2);
double getImageAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2);
double getImageStdValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2);
double fitRadiusPercent(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double bound_r,
							float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly);
void fitPosition(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double ir,
							float &x, float &y, float &z,  float* D=0, float zthickness=1.0);


//some block operation functions

double getBlockMaxValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
						V3DLONG x0, V3DLONG y0, V3DLONG z0,
						int xstep, int ystep, int zstep);
double getBlockAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
						V3DLONG x0, V3DLONG y0, V3DLONG z0,
						int xstep, int ystep, int zstep);

template <class T> VPoint *getBlockAveValue(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel, 
						V3DLONG x0, V3DLONG y0, V3DLONG z0,
						int xstep, int ystep, int zstep);

bool setBlockAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
					  V3DLONG x0, V3DLONG y0, V3DLONG z0,
					  int xstep, int ystep, int zstep, unsigned char target_val);
double getBlockStdValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
						V3DLONG x0, V3DLONG y0, V3DLONG z0,
						int xstep, int ystep, int zstep);


// template based functions /////////////////////////////////////////////////////////////////////

static
bool less_r(V_NeuronSWC_unit a, V_NeuronSWC_unit b) {return (a.r < b.r);}

template <class T> //should be a struct included members of (r), like V_NeuronSWC_unit
bool smooth_radius(vector <T> & mCoord, int winsize, bool median_filter)
{
	//std::cout<<" smooth_radius ";
	if (winsize<2) return true;

	std::vector<T> mC = mCoord; // a copy
	V3DLONG N = mCoord.size();
	int halfwin = winsize/2;

	for (int i=1; i<N-1; i++) // don't move start & end point
	{
		std::vector<T> winC;
		std::vector<double> winW;
		winC.clear();
		winW.clear();

		winC.push_back( mC[i] );
		winW.push_back( 1.+halfwin );
		for (int j=1; j<=halfwin; j++)
		{
			int k1 = i+j;	if(k1<0) k1=0;	if(k1>N-1) k1=N-1;
			int k2 = i-j;	if(k2<0) k2=0;	if(k2>N-1) k2=N-1;
			winC.push_back( mC[k1] );
			winC.push_back( mC[k2] );
			winW.push_back( 1.+halfwin-j );
			winW.push_back( 1.+halfwin-j );
		}
		//std::cout<<"winC.size = "<<winC.size()<<"\n";

		double r = 0;
		if (median_filter)
		{
			sort(winC.begin(), winC.end(), less_r);
			r = winC[halfwin].r;
		}
		else
		{
			double s = r = 0;
			for (int i=0; i<winC.size(); i++)
			{
				r += winW[i]* winC[i].r;
				s += winW[i];
			}
			if (s)	r /= s;
		}

		mCoord[i].r = r; // output
	}
	return true;
}

template <class T> //should be a struct included members of (x,y,z), like Coord3D
bool smooth_curve(std::vector<T> & mCoord, int winsize)
{
	//std::cout<<" smooth_curve ";
	if (winsize<2) return true;

	std::vector<T> mC = mCoord; // a copy
	V3DLONG N = mCoord.size();
	int halfwin = winsize/2;

	for (int i=1; i<N-1; i++) // don't move start & end point
	{
		std::vector<T> winC;
		std::vector<double> winW;
		winC.clear();
		winW.clear();

		winC.push_back( mC[i] );
		winW.push_back( 1.+halfwin );
		for (int j=1; j<=halfwin; j++)
		{
			int k1 = i+j;	if(k1<0) k1=0;	if(k1>N-1) k1=N-1;
			int k2 = i-j;	if(k2<0) k2=0;	if(k2>N-1) k2=N-1;
			winC.push_back( mC[k1] );
			winC.push_back( mC[k2] );
			winW.push_back( 1.+halfwin-j );
			winW.push_back( 1.+halfwin-j );
		}
		//std::cout<<"winC.size = "<<winC.size()<<"\n";

		double s, x,y,z;
		s = x = y = z = 0;
		for (int i=0; i<winC.size(); i++)
		{
			x += winW[i]* winC[i].x;
			y += winW[i]* winC[i].y;
			z += winW[i]* winC[i].z;
			s += winW[i];
		}
		if (s)
		{
			x /= s;
			y /= s;
			z /= s;
		}

		mCoord[i].x = x; // output
		mCoord[i].y = y; // output
		mCoord[i].z = z; // output
	}
	return true;
}

template <class T>
std::vector<T> downsample_curve(const std::vector<T> & mCoord, int step)
{
	//std::cout<<" downsample_curve ";
	if (step<1) return mCoord;

	std::vector<T> mC; // for out put
	mC.clear();

	V3DLONG N = mCoord.size();

	if (N>0)	mC.push_back( mCoord[0] ); // output
	for (int i=1; i<N-1; i+=step) // don't move start & end point
	{
		mC.push_back( mCoord[i] ); // output
	}
	if (N>1)	mC.push_back( mCoord[N-1] ); // output

	return mC;
}

//////////////////////////////////////////////////////////////////////////////////
// make the shift vector be orthogonal with curve -- a repulsion force opposite to attraction force
#define DIFF(diff, mCoord, i, xyz, HW) \
{ \
		diff = 0; \
		int kk; \
		V3DLONG N = mCoord.size(); \
		for (int k=1;k<=HW;k++) \
		{ \
			kk = i+k; if (kk<0) kk=0; if (kk>N-1) kk=N-1; \
			diff += mCoord[kk].xyz; \
			kk = i-k; if (kk<0) kk=0; if (kk>N-1) kk=N-1; \
			diff -= mCoord[kk].xyz; \
		} \
}
template <class T>
void curve_bending_vector(vector<T>& mCoord, V3DLONG i, T& Coord_new)
{
	float D[3];
	DIFF(D[0], mCoord, i, x, 5);
	DIFF(D[1], mCoord, i, y, 5);
	DIFF(D[2], mCoord, i, z, 5);
	//printf("[%g,%g,%g]	", D[0],D[1],D[2]);

	float x = mCoord[i].x;
	float y = mCoord[i].y;
	float z = mCoord[i].z;
	float cx = Coord_new.x;
	float cy = Coord_new.y;
	float cz = Coord_new.z;
	{
		// make normal vector
		double len = sqrt(D[0]*D[0] + D[1]*D[1] + D[2]*D[2]);
		if (len)
		{
			D[0] /= len;
			D[1] /= len;
			D[2] /= len;
			// displacement
			cx = cx-x;
			cy = cy-y;
			cz = cz-z;
			//printf("<%g,%g,%g>	", cx,cy,cz);
			double proj = cx*D[0] + cy*D[1] + cz*D[2];
			cx = cx - proj*D[0];
			cy = cy - proj*D[1];
			cz = cz - proj*D[2];
			//printf("<<%g,%g,%g>>	", cx,cy,cz);
			x += cx;
			y += cy;
			z += cz;
		}
	}
	Coord_new.x = x;
	Coord_new.y = y;
	Coord_new.z = z;
}

inline double get_intensity(unsigned char*** img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, int ix, int iy, int iz)
{
	if ((ix<0) || (ix>dim0-1)) return 0;
	if ((iy<0) || (iy>dim1-1)) return 0;
	if ((iz<0) || (iz>dim2-1)) return 0;
	return double(img3d[iz][iy][ix]);
}
#define I(ix, iy, iz) 	get_intensity(img3d, dim0, dim1, dim2, ix, iy, iz)


// 090520: create according to point_bdb_minus_3d_localwinmass()
// 090619: move from bdb_minus.h to here
// And combined with adaptive radius estimation & orthogonal shift
// 090621: add bending_code

template <class T> //should be a struct included members of (x,y,z), like V_NeuronSWC_unit
bool point_bdb_minus_3d_localwinmass_prior(unsigned char*** img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
		vector <T> & mCoord, const BDB_Minus_Prior_Parameter & bdb_para, bool b_fix_end,
		const vector <T> & mCoord_prior, int bending_code=1, float zthickness=1.0, bool b_est_in_xyplaneonly=false) // 0--no bending, 1--bending M_term, 2--bending mCoord
{
	bool b_use_M_term = 1; //for switch
	bool b_use_P_term = 1; //for switch
	bool b_use_G_term = 0; //for switch


	double f_image  = 1;
	double f_length = bdb_para.f_length;
	double f_smooth = bdb_para.f_smooth;
	double f_prior  = bdb_para.f_prior; //0.2;
	double f_gradient = 0;/// gradient is not stable
	int max_loops = bdb_para.nloops;

	double TH = 1; //pixel/node, the threshold to judge convergence

	double AR = 0;
	for (V3DLONG i=0; i<mCoord.size()-1; i++)
	{
		double x = mCoord[i].x;
		double y = mCoord[i].y;
		double z = mCoord[i].z;
		double x1 = mCoord[i+1].x;
		double y1 = mCoord[i+1].y;
		double z1 = mCoord[i+1].z;
		AR += sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1) + (z-z1)*(z-z1));
	}
	AR /= mCoord.size()-1; // average distance between nodes
	double radius = AR*2;
	double imgAve = getImageAveValue(img3d, dim0, dim1, dim2);
	double imgStd = getImageStdValue(img3d, dim0, dim1, dim2);
	double imgTH = imgAve + imgStd;


	V3DLONG M = mCoord.size(); // number of control points / centers of k_means
	if (M<=2) return true; //in this case no adjusting is needed. by PHC, 090119. also prevent a memory crash
	V3DLONG M_prior = mCoord_prior.size(); // number of prior control points

	V3DLONG i,j;
	T F_1_term, F_2_term, M_term, P_term, G_term;
	vector <T> mCoord_new = mCoord; // temporary out
	vector <T> mCoord_old = mCoord; // a constant copy
	double lastscore;


	for (V3DLONG nloop=0; nloop<max_loops; nloop++)
	{
		// for each control point
		for (j=0; j<M; j++)
		{
			//==================================================================
			// external force term initialization
			M_term = mCoord.at(j);
			P_term = mCoord.at(j);
			G_term = mCoord.at(j); // 090623 RZC

			//------------------------------------------------------------------
			//image force: M_term
			if (img3d && (b_use_M_term))
			{
				double xc = mCoord.at(j).x;
				double yc = mCoord.at(j).y;
				double zc = mCoord.at(j).z;

				//090621 RZC: dynamic radius estimation
				radius = 2* fitRadiusPercent(img3d, dim0, dim1, dim2, imgTH,  AR*2, xc, yc, zc, zthickness, b_est_in_xyplaneonly);

				V3DLONG x0 = xc - radius; x0 = (x0<0)?0:x0;
				V3DLONG x1 = xc + radius; x1 = (x1>dim0-1)?(dim0-1):x1;
				V3DLONG y0 = yc - radius; y0 = (y0<0)?0:y0;
				V3DLONG y1 = yc + radius; y1 = (y1>dim1-1)?(dim1-1):y1;
				V3DLONG z0 = zc - radius; z0 = (z0<0)?0:z0;
				V3DLONG z1 = zc + radius; z1 = (z1>dim2-1)?(dim2-1):z1;

				double sum_x=0, sum_y=0, sum_z=0, sum_px=0, sum_py=0, sum_pz=0;
				V3DLONG ix, iy, iz;
				//use a sphere region, as this is easiest to compute the unbiased center of mass
				double dx,dy,dz, r2=double(radius)*(radius);
				for (iz=z0;iz<=z1;iz++)
				{
					dz = fabs(iz-zc); dz*=dz;

					for (iy=y0;iy<=y1;iy++)
					{
						dy = fabs(iy-yc); dy*=dy;
						if (dy+dz>r2) continue;
						dy += dz;

						for (ix=x0;ix<x1;ix++)
						{
							dx = fabs(ix-xc); dx*=dx;
							if (dx+dy>r2) continue;

							register unsigned char tmpval = img3d[iz][iy][ix];
							if (tmpval)
							{
								sum_x += tmpval;
								sum_y += tmpval;
								sum_z += tmpval;
								sum_px += double(tmpval) * ix;
								sum_py += double(tmpval) * iy;
								sum_pz += double(tmpval) * iz;
							}
						}
					}
				}
				if (sum_x && sum_y && sum_z)
				{
					M_term.x = sum_px/sum_x;
					M_term.y = sum_py/sum_y;
					M_term.z = sum_pz/sum_z;
				}
				else
				{
					M_term.x = xc;
					M_term.y = yc;
					M_term.z = zc;
				}
				/////////////////////////////////////////////
				//090621 RZC
				if (bending_code==1)	curve_bending_vector(mCoord, j, M_term);
				/////////////////////////////////////////////
			}

			//----------------------------------------------------------------
			// image prior G_term (grident)
			if (img3d && b_use_G_term)
			{
				double xc = mCoord.at(j).x;
				double yc = mCoord.at(j).y;
				double zc = mCoord.at(j).z;

				V3DLONG ix = xc+.5;
				V3DLONG iy = yc+.5;
				V3DLONG iz = zc+.5;

				double gx = 0;
				for (V3DLONG j=-1; j<=1; j++) for (int k=-1; k<=1; k++)
				{
					gx += I(ix+1, iy+j, iz+k);
					gx -= I(ix-1, iy+j, iz+k);
				}
				double gy = 0;
				for (V3DLONG j=-1; j<=1; j++) for (int k=-1; k<=1; k++)
				{
					gy += I(ix+j, iy+1, iz+k);
					gy -= I(ix+j, iy-1, iz+k);
				}
				double gz = 0;
				for (V3DLONG j=-1; j<=1; j++) for (int k=-1; k<=1; k++)
				{
					gz += I(ix+k, iy+j, iz+1);
					gz -= I(ix+k, iy+j, iz-1);
				}
				double Im = I(ix,iy,iz);

				// factor to connect grayscle with pixel step for G_term
				double gradient_step = (imgStd)? 1/(imgStd*imgStd) : 0;
				Im = gradient_step*(255-Im);

				G_term.x += gx*Im;
				G_term.y += gy*Im;
				G_term.z += gz*Im;
			}

			//------------------------------------------------------------------
			// geometric prior P_term
			if (mCoord_prior.size()>0 && (b_use_P_term))
			{
				P_term = mCoord_prior.at(0);
				double dx,dy,dz;
				dx = mCoord.at(j).x - mCoord_prior.at(0).x;
				dy = mCoord.at(j).y - mCoord_prior.at(0).y;
				dz = mCoord.at(j).z - mCoord_prior.at(0).z;
				double d0 = sqrt(dx*dx+dy*dy+dz*dz);

				for (V3DLONG ip=1; ip < mCoord_prior.size(); ip++)
				{
					dx = mCoord.at(j).x - mCoord_prior.at(ip).x;
					dy = mCoord.at(j).y - mCoord_prior.at(ip).y;
					dz = mCoord.at(j).z - mCoord_prior.at(ip).z;
					double d1 = sqrt(dx*dx+dy*dy+dz*dz);

					if (d1<d0)
					{
						P_term = mCoord_prior.at(ip);
						d0 = d1;
					}
				}
			}

			//printf("M_term : [%5.3f, %5.3f, %5.3f], %d\n", M_term.x, M_term.y, M_term.z, j);
			//printf("P_term : [%5.3f, %5.3f, %5.3f], %d\n", P_term.x, P_term.y, P_term.z, j);
			//printf("G_term : [%5.3f, %5.3f, %5.3f], %d\n", G_term.x, G_term.y, G_term.z, j);


			//========================================================================================================================
			//           b{Ckm1 + Ckp1} + c{Ckm1 + Ckp1 - 0.25 Ckm2 - 0.25 Ckp2} + a{Mk} + d{Pk} + e{Ck + (1 - I[Ck] / Imax) I'[Ck]}
			// Ck_new = ---------------------------------------------------------------------------------------------------------------------
			//                                              (2 b + 1.5 c + a + d + e)
			//========================================================================================================================
			//internal force: F_1_term F_2_term for smoothing
			// new_coord = { f_length*F_1_term + f_smooth*F_2_term + f_image*M_term +
			//               f_prior*P_term + f_gradient*(G_term) } /(2*f_length + 1.5*f_smooth + f_image + f_prior + f_gradient)
			// boundary nodes have simple format
			double f;
			if (j==0 || j==M-1)
			{
				f = (f_image + f_prior + f_gradient);
				if (f==0) f=1;
				mCoord_new.at(j).x = (f_image*M_term.x + f_prior*P_term.x + f_gradient*G_term.x) /f;
				mCoord_new.at(j).y = (f_image*M_term.y + f_prior*P_term.y + f_gradient*G_term.y) /f;
				mCoord_new.at(j).z = (f_image*M_term.z + f_prior*P_term.z + f_gradient*G_term.z) /f;
			}
			else if (j==1 || j==M-2)
			{
				F_1_term.x = mCoord.at(j-1).x + mCoord.at(j+1).x;
				F_1_term.y = mCoord.at(j-1).y + mCoord.at(j+1).y;
				F_1_term.z = mCoord.at(j-1).z + mCoord.at(j+1).z;

				f =  (2*f_length + f_image + f_prior + f_gradient);
				if (f==0) f=1;
				mCoord_new.at(j).x = (f_length*F_1_term.x + f_image*M_term.x + f_prior*P_term.x + f_gradient*G_term.x) /f;
				mCoord_new.at(j).y = (f_length*F_1_term.y + f_image*M_term.y + f_prior*P_term.y + f_gradient*G_term.y) /f;
				mCoord_new.at(j).z = (f_length*F_1_term.z + f_image*M_term.z + f_prior*P_term.z + f_gradient*G_term.z) /f;
			}
			else // not boundary nodes
			{
				F_1_term.x = mCoord.at(j-1).x + mCoord.at(j+1).x;
				F_1_term.y = mCoord.at(j-1).y + mCoord.at(j+1).y;
				F_1_term.z = mCoord.at(j-1).z + mCoord.at(j+1).z;

				F_2_term.x = (mCoord.at(j-1).x + mCoord.at(j+1).x) - 0.25* (mCoord.at(j+2).x + mCoord.at(j-2).x);
				F_2_term.y = (mCoord.at(j-1).y + mCoord.at(j+1).y) - 0.25* (mCoord.at(j+2).y + mCoord.at(j-2).y);
				F_2_term.z = (mCoord.at(j-1).z + mCoord.at(j+1).z) - 0.25* (mCoord.at(j+2).z + mCoord.at(j-2).z);

				f =  (2*f_length + 1.5*f_smooth + f_image + f_prior + f_gradient);
				if (f==0) f=1;
				mCoord_new.at(j).x = (f_length*F_1_term.x + f_smooth*F_2_term.x + f_image*M_term.x + f_prior*P_term.x + f_gradient*G_term.x) /f;
				mCoord_new.at(j).y = (f_length*F_1_term.y + f_smooth*F_2_term.y + f_image*M_term.y + f_prior*P_term.y + f_gradient*G_term.y) /f;
				mCoord_new.at(j).z = (f_length*F_1_term.z + f_smooth*F_2_term.z + f_image*M_term.z + f_prior*P_term.z + f_gradient*G_term.z) /f;
			}

			//printf("[%5.3f, %5.3f, %5.3f], %d\n", mCoord_new.at(j).x, mCoord_new.at(j).y, mCoord_new.at(j).z, j);
		}

		///////////////////////////////////////////////////////
		//090621 RZC
		if (bending_code==2)
			for (j=0; j<M; j++)		curve_bending_vector(mCoord, j, mCoord_new[j]);

		/////////////////////////////////////////////////////
		// compute curve score
		double score = 0.0;
		for (j=0; j<M; j++)
			score += fabs(mCoord_new.at(j).x - mCoord.at(j).x) + fabs(mCoord_new.at(j).y - mCoord.at(j).y) + fabs(mCoord_new.at(j).z - mCoord.at(j).z);

		printf("score[%d]=%g ", nloop, score);

		// update the coordinates of the control points
		mCoord = mCoord_new;

		/////////////////////////////////////////////////////
		if (b_fix_end)
		{
			//without changing the start and end points
			mCoord[0].x = mCoord_old[0].x;
			mCoord[M-1].x = mCoord_old[M-1].x;
			mCoord[0].y = mCoord_old[0].y;
			mCoord[M-1].y = mCoord_old[M-1].y;
			mCoord[0].z = mCoord_old[0].z;
			mCoord[M-1].z = mCoord_old[M-1].z;
		}


		//////////////////////////////////////////////////////
		// Can the iteration be terminated ?
		if (score<TH || isnan(score))
			break;
		if (nloop > 0)
		{
			if (fabs(lastscore-score) < TH*0.1/M) // to prevent jumping around
				break;
		}

		lastscore = score;
	}

	return true;
}


//a special function to run the bdb- method but using the graph to find the neighbors (instead of assume a linear order)

template <class T> //should be a struct included members of (x,y,z), like V_NeuronSWC_unit
bool point_bdb_minus_3d_localwinmass_prior_withGraphOrder(unsigned char*** img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
										   vector <T> & mCoord, const BDB_Minus_Prior_Parameter & bdb_para, bool b_fix_end,
										   const vector <T> & mCoord_prior, int bending_code=1, float zthickness=1.0, bool b_est_in_xyplaneonly=false) // 0--no bending, 1--bending M_term, 2--bending mCoord
{
	V3DLONG tnodes = mCoord.size();
	V3DLONG i,j;
	
	//first propduce a LUT for easier access
	std::map<double,V3DLONG> index_map;	index_map.clear();
	V3DLONG root_id=-1;
	for (j=0; j<tnodes; j++)
	{
		double ndx   = mCoord[j].n;
		V3DLONG new_ndx = index_map.size(); //map the neuron node's id to row number
		index_map[ndx] = new_ndx;
		
		if (mCoord[j].parent<0)
		{
			if (root_id!=-1)
				printf("==================== detect a non-unique root!\n");
			root_id = V3DLONG(mCoord[j].n);
			printf("==================== nchild of root [%ld, id=%ld] = %ld\n", j, V3DLONG(mCoord[j].n), V3DLONG(mCoord[j].nchild));
		}
	}
	//also create a children list
	vector < vector < V3DLONG > > childrenList; childrenList.clear();
	for (j=0; j<tnodes; j++)
	{
		vector<V3DLONG> tmp; tmp.clear();
		childrenList.push_back(tmp);
	}
	for (j=0; j<tnodes; j++)
	{
		if (mCoord[j].parent>=0)
		{
			childrenList[index_map[mCoord[j].parent]].push_back(mCoord[j].n); 
		}
	}
	
	//now turn this children list as a neighbor list
	vector < vector < V3DLONG > > neighborList = childrenList; 
	for (j=0; j<tnodes; j++)
	{
		if (mCoord[j].parent>=0)
		{
			neighborList[index_map[mCoord[j].n]].push_back(mCoord[j].parent); 
		}
	}
	
	//
	
	bool b_use_M_term = 1; //for switch
	bool b_use_P_term = 1; //for switch
	bool b_use_G_term = 0; //for switch
	
	
	double f_image  = 1;
	double f_length = bdb_para.f_length;
	double f_smooth = bdb_para.f_smooth;
	double f_prior  = bdb_para.f_prior; //0.2;
	double f_gradient = 0;/// gradient is not stable
	int max_loops = bdb_para.nloops;
	
	double TH = 1; //pixel/node, the threshold to judge convergence
	
	double AR = 0;
	for (V3DLONG i=0; i<mCoord.size()-1; i++)
	{
		double x = mCoord[i].x;
		double y = mCoord[i].y;
		double z = mCoord[i].z;
		
		V3DLONG pi = mCoord[i].parent;
		if (pi<0 || pi==mCoord[i].n) //isolated nodes should not be counted
		{
			continue;
		}
		
		V3DLONG pi_rownum = index_map[ mCoord[i].parent ];
		
		double x1 = mCoord[pi_rownum].x;
		double y1 = mCoord[pi_rownum].y;
		double z1 = mCoord[pi_rownum].z;
		AR += sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1) + (z-z1)*(z-z1));
	}
	AR /= mCoord.size()-1; // average distance between nodes
	double radius = AR*2;
	double imgAve = getImageAveValue(img3d, dim0, dim1, dim2);
	double imgStd = getImageStdValue(img3d, dim0, dim1, dim2);
	double imgTH = imgAve + imgStd;
	
	
	V3DLONG M = mCoord.size(); // number of control points / centers of k_means
	if (M<=2) return true; //in this case no adjusting is needed. by PHC, 090119. also prevent a memory crash
	V3DLONG M_prior = mCoord_prior.size(); // number of prior control points
	
	T F_1_term, F_2_term, M_term, P_term, G_term;
	vector <T> mCoord_new = mCoord; // temporary out
	vector <T> mCoord_old = mCoord; // a constant copy
	double lastscore;
	
	for (V3DLONG nloop=0; nloop<max_loops; nloop++)
	{
		// for each control point
		for (j=0; j<M; j++)
		{
			//==================================================================
			// external force term initialization
			M_term = mCoord.at(j);
			P_term = mCoord.at(j);
			G_term = mCoord.at(j); 
			
			//------------------------------------------------------------------
			//image force: M_term
			if (img3d && (b_use_M_term))
			{
				double xc = mCoord.at(j).x;
				double yc = mCoord.at(j).y;
				double zc = mCoord.at(j).z;
				
				//dynamic radius estimation
				radius = 2* fitRadiusPercent(img3d, dim0, dim1, dim2, imgTH,  AR*2, xc, yc, zc, zthickness, b_est_in_xyplaneonly);
				
				V3DLONG x0 = xc - radius; x0 = (x0<0)?0:x0;
				V3DLONG x1 = xc + radius; x1 = (x1>dim0-1)?(dim0-1):x1;
				V3DLONG y0 = yc - radius; y0 = (y0<0)?0:y0;
				V3DLONG y1 = yc + radius; y1 = (y1>dim1-1)?(dim1-1):y1;
				V3DLONG z0 = zc - radius; z0 = (z0<0)?0:z0;
				V3DLONG z1 = zc + radius; z1 = (z1>dim2-1)?(dim2-1):z1;
				
				double sum_x=0, sum_y=0, sum_z=0, sum_px=0, sum_py=0, sum_pz=0;
				V3DLONG ix, iy, iz;
				//use a sphere region, as this is easiest to compute the unbiased center of mass
				double dx,dy,dz, r2=double(radius)*(radius);
				for (iz=z0;iz<=z1;iz++)
				{
					dz = fabs(iz-zc); dz*=dz;
					
					for (iy=y0;iy<=y1;iy++)
					{
						dy = fabs(iy-yc); dy*=dy;
						if (dy+dz>r2) continue;
						dy += dz;
						
						for (ix=x0;ix<x1;ix++)
						{
							dx = fabs(ix-xc); dx*=dx;
							if (dx+dy>r2) continue;
							
							register unsigned char tmpval = img3d[iz][iy][ix];
							if (tmpval)
							{
								sum_x += tmpval;
								sum_y += tmpval;
								sum_z += tmpval;
								sum_px += double(tmpval) * ix;
								sum_py += double(tmpval) * iy;
								sum_pz += double(tmpval) * iz;
							}
						}
					}
				}
				if (sum_x && sum_y && sum_z)
				{
					M_term.x = sum_px/sum_x;
					M_term.y = sum_py/sum_y;
					M_term.z = sum_pz/sum_z;
				}
				else
				{
					M_term.x = xc;
					M_term.y = yc;
					M_term.z = zc;
				}
			}
			
			//----------------------------------------------------------------
			// image prior G_term (gradient)
			if (img3d && b_use_G_term)
			{
				double xc = mCoord.at(j).x;
				double yc = mCoord.at(j).y;
				double zc = mCoord.at(j).z;
				
				V3DLONG ix = xc+.5;
				V3DLONG iy = yc+.5;
				V3DLONG iz = zc+.5;
				
				double gx = 0;
				for (V3DLONG j=-1; j<=1; j++) for (int k=-1; k<=1; k++)
				{
					gx += I(ix+1, iy+j, iz+k);
					gx -= I(ix-1, iy+j, iz+k);
				}
				double gy = 0;
				for (V3DLONG j=-1; j<=1; j++) for (int k=-1; k<=1; k++)
				{
					gy += I(ix+j, iy+1, iz+k);
					gy -= I(ix+j, iy-1, iz+k);
				}
				double gz = 0;
				for (V3DLONG j=-1; j<=1; j++) for (int k=-1; k<=1; k++)
				{
					gz += I(ix+k, iy+j, iz+1);
					gz -= I(ix+k, iy+j, iz-1);
				}
				double Im = I(ix,iy,iz);
				
				// factor to connect grayscle with pixel step for G_term
				double gradient_step = (imgStd)? 1/(imgStd*imgStd) : 0;
				Im = gradient_step*(255-Im);
				
				G_term.x += gx*Im;
				G_term.y += gy*Im;
				G_term.z += gz*Im;
			}
			
			//------------------------------------------------------------------
			// geometric prior P_term
			if (mCoord_prior.size()>0 && (b_use_P_term))
			{
				P_term = mCoord_prior.at(0);
				double dx,dy,dz;
				dx = mCoord.at(j).x - mCoord_prior.at(0).x;
				dy = mCoord.at(j).y - mCoord_prior.at(0).y;
				dz = mCoord.at(j).z - mCoord_prior.at(0).z;
				double d0 = sqrt(dx*dx+dy*dy+dz*dz);
				
				for (V3DLONG ip=1; ip < mCoord_prior.size(); ip++)
				{
					dx = mCoord.at(j).x - mCoord_prior.at(ip).x;
					dy = mCoord.at(j).y - mCoord_prior.at(ip).y;
					dz = mCoord.at(j).z - mCoord_prior.at(ip).z;
					double d1 = sqrt(dx*dx+dy*dy+dz*dz);
					
					if (d1<d0)
					{
						P_term = mCoord_prior.at(ip);
						d0 = d1;
					}
				}
			}
			
			//printf("M_term : [%5.3f, %5.3f, %5.3f], %d\n", M_term.x, M_term.y, M_term.z, j);
			//printf("P_term : [%5.3f, %5.3f, %5.3f], %d\n", P_term.x, P_term.y, P_term.z, j);
			//printf("G_term : [%5.3f, %5.3f, %5.3f], %d\n", G_term.x, G_term.y, G_term.z, j);
			
		
			// boundary nodes have simple format
			double f;
			
			if (neighborList[index_map[mCoord[j].n]].size()<1) //do not adjust isolated node (and this should never happen anyway?)
				continue;
			else if (neighborList[index_map[mCoord[j].n]].size()==1) 
			{
				f = (f_image + f_prior + f_gradient);
				if (f==0) f=1;
				mCoord_new.at(j).x = (f_image*M_term.x + f_prior*P_term.x + f_gradient*G_term.x) /f;
				mCoord_new.at(j).y = (f_image*M_term.y + f_prior*P_term.y + f_gradient*G_term.y) /f;
				mCoord_new.at(j).z = (f_image*M_term.z + f_prior*P_term.z + f_gradient*G_term.z) /f;
			}
			else  
			{
				V3DLONG cur_row = index_map[mCoord[j].n];
				
				F_1_term.x = 0;
				F_1_term.y = 0;
				F_1_term.z = 0;
								
				for (V3DLONG n=0;n<neighborList[cur_row].size();n++)
				{
					V3DLONG cur_neighbor = index_map[ neighborList[cur_row][n] ];
					
					F_1_term.x += mCoord.at(cur_neighbor).x;
					F_1_term.y += mCoord.at(cur_neighbor).y;
					F_1_term.z += mCoord.at(cur_neighbor).z;
				}
				
				f =  (neighborList[cur_row].size()*f_length + f_image + f_prior + f_gradient);
				if (f==0) f=1;
				mCoord_new.at(j).x = (f_length*F_1_term.x + f_image*M_term.x + f_prior*P_term.x + f_gradient*G_term.x) /f;
				mCoord_new.at(j).y = (f_length*F_1_term.y + f_image*M_term.y + f_prior*P_term.y + f_gradient*G_term.y) /f;
				mCoord_new.at(j).z = (f_length*F_1_term.z + f_image*M_term.z + f_prior*P_term.z + f_gradient*G_term.z) /f;
			}

			
			//printf("[%5.3f, %5.3f, %5.3f], %d\n", mCoord_new.at(j).x, mCoord_new.at(j).y, mCoord_new.at(j).z, j);
		}
		
		
		/////////////////////////////////////////////////////
		// compute curve score
		double score = 0.0;
		for (j=0; j<M; j++)
			score += fabs(mCoord_new.at(j).x - mCoord.at(j).x) + fabs(mCoord_new.at(j).y - mCoord.at(j).y) + fabs(mCoord_new.at(j).z - mCoord.at(j).z);
		
		printf("score[%d]=%g ", nloop, score);
		
		// update the coordinates of the control points
		mCoord = mCoord_new;
		
		/////////////////////////////////////////////////////
		if (b_fix_end)
		{
			//without changing the start and end points
			
			for (j=0;j<M;j++)
				if (neighborList[index_map[ mCoord[j].n ]].size()<=1) //then terminal node
				{
					mCoord[j].x = mCoord_old[j].x;
					mCoord[j].y = mCoord_old[j].y;
					mCoord[j].z = mCoord_old[j].z;
				}
		}
		
		
		//////////////////////////////////////////////////////
		// Can the iteration be terminated ?
		if (score<TH || isnan(score))
			break;
		if (nloop > 0)
		{
			if (fabs(lastscore-score) < TH*0.1/M) // to prevent jumping around
				break;
		}
		
		lastscore = score;
	}
	
	return true;
}

template <class T> VPoint *getBlockAveValueVPoint(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel, 
										   V3DLONG x0, V3DLONG y0, V3DLONG z0,
										   int xstep, int ystep, int zstep, int method)
{
	if (!img4d || !sz|| sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || 
		x0<0 || x0>=sz[0] || y0<0 || y0>=sz[1] || z0<0 || z0>=sz[2] || nChannel<=0 || !channelsToUse)
		return 0;

	V3DLONG i,j,k,c,n;
	for (c=0;c<nChannel;c++)
	{
		if (channelsToUse[c]<0 || channelsToUse[c]>=sz[3])
			return 0;
	}
	
	double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
	V3DLONG xs=x0-xsteph, xe=x0+xsteph,
	ys=y0-ysteph, ye=y0+ysteph,
	zs=z0-zsteph, ze=z0+zsteph;
	
	if (xs<0) xs=0; if (xe>=sz[0]) xe=sz[0]-1;
	if (ys<0) ys=0; if (ye>=sz[1]) ye=sz[1]-1;
	if (zs<0) zs=0; if (ze>=sz[2]) ze=sz[2]-1;

	if (method==0)
	{
		VPoint *v = new VPoint(nChannel, 0);
		n=0;
		for (k=zs;k<=ze; k++)
			for (j=ys;j<=ye; j++)
				for (i=xs;i<=xe; i++)
				{
					for (c=0;c<nChannel;c++)
					{
						v->v[c] += double(img4d[channelsToUse[c]][k][j][i]);
						n++;
					}
				}
		if (n>0)		
			for (c=0;c<nChannel;c++)
			{
				//printf("[%5.4f %5.4f]", v->v[c], (double(n)));
				v->v[c] /= n;		
				if (v->v[c]>255)
					printf("ha");
			}

		return v;
	}
	else 
	{
		VPoint *v = new VPoint(1, 0);
		n=0;
		for (k=zs;k<=ze; k++)
			for (j=ys;j<=ye; j++)
				for (i=xs;i<=xe; i++)
				{
					double m = double(img4d[channelsToUse[0]][k][j][i]);
					for (c=1;c<nChannel;c++)
					{
						if (m<double(img4d[channelsToUse[c]][k][j][i]))
							m = double(img4d[channelsToUse[c]][k][j][i]);
					}
					v->v[0] += m;
					n++;
				}
		if (n>0)		
			{
				v->v[0] /= n;		
				if (v->v[0]>255)
					printf("ha");
			}
		
		return v;
	}
}

template <class T> double getBlockAveValue(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel, 
											V3DLONG x0, V3DLONG y0, V3DLONG z0,
											int xstep, int ystep, int zstep, int method)
{
	VPoint * v = getBlockAveValueVPoint(img4d, sz, channelsToUse, nChannel, 
								  x0, y0, z0,
								  xstep, ystep, zstep, method);
	if (!v) return 0;
	double s = v->abs();
	if(v) {delete v; v=0;}
	return s;
}

template <class T> VPoint *getImageAveValueVPoint(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel, int method) 
{
	V3DLONG x0 = sz[0]/2;
	V3DLONG y0 = sz[1]/2;
	V3DLONG z0 = sz[2]/2;
	V3DLONG xstep = sz[0];
	V3DLONG ystep = sz[1];
	V3DLONG zstep = sz[2];
	return getBlockAveValueVPoint(img4d, sz, channelsToUse, nChannel, 
							x0, y0, z0,
							xstep, ystep, zstep, method);
}

template <class T> double getImage4DAveValue(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel, int method) 
{
	VPoint * v = getImageAveValueVPoint(img4d, sz, channelsToUse, nChannel, method);
	if (!v) return 0;
	double s = v->abs();
	if(v) {delete v; v=0;}
	return s;
}

template <class T> VPoint * getBlockStdValueVPoint(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel,
						V3DLONG x0, V3DLONG y0, V3DLONG z0,
						int xstep, int ystep, int zstep)
{
	if (!img4d || !sz|| sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || 
		x0<0 || x0>=sz[0] || y0<0 || y0>=sz[1] || z0<0 || z0>=sz[2] || nChannel<=0 || !channelsToUse)
	{
		printf("getBlockStdValueVPoint() return 0! Error.");
		return 0;
	}
	
	V3DLONG i,j,k,c,n;
	for (c=0;c<nChannel;c++)
	{
		if (channelsToUse[c]<0 || channelsToUse[c]>=sz[3])
			return 0;
	}
	
	int method=0;
	VPoint *va = getBlockAveValueVPoint(img4d, sz, channelsToUse, nChannel, 
								  x0, y0, z0,
								  xstep, ystep, zstep, method);
	
	double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
	V3DLONG xs=x0-xsteph, xe=x0+xsteph,
	ys=y0-ysteph, ye=y0+ysteph,
	zs=z0-zsteph, ze=z0+zsteph;
	
	if (xs<0) xs=0; if (xe>=sz[0]) xe=sz[0]-1;
	if (ys<0) ys=0; if (ye>=sz[1]) ye=sz[1]-1;
	if (zs<0) zs=0; if (ze>=sz[2]) ze=sz[2]-1;
	
	VPoint *vs = new VPoint(nChannel, 0);
	double v=0;
	n=0;
	for (k=zs;k<=ze; k++)
		for (j=ys;j<=ye; j++)
			for (i=xs;i<=xe; i++)
			{
				for(c=0;c<nChannel;c++)
				{
					double d = (double(img4d[channelsToUse[c]][k][j][i]) - va->v[c]);
					vs->v[c] += d*d;
				}
				n++;
			}
	
	if (n>0)
	{
		for(c=0;c<nChannel;c++)
		{
			vs->v[c] = sqrt(vs->v[c]/n);
			printf("[------------------------------------------------------------------------------%5.4f]\n" , vs->v[c]);
		}
	}
	
	if (va) {delete va; va=0;}
	return vs;
}

template <class T> double getBlockStdValue(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel,
											 V3DLONG x0, V3DLONG y0, V3DLONG z0,
											 int xstep, int ystep, int zstep)
{
	VPoint *v = getBlockStdValueVPoint(img4d, sz, channelsToUse, nChannel,
								x0, y0, z0,
								 xstep, ystep, zstep);
	if (!v) return 0;
	double s = v->abs();
	if(v) {delete v; v=0;}
	return s;
}

template <class T> VPoint *getImageStdValueVPoint(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel) 
{
	V3DLONG x0 = sz[0]/2;
	V3DLONG y0 = sz[1]/2;
	V3DLONG z0 = sz[2]/2;
	V3DLONG xstep = sz[0];
	V3DLONG ystep = sz[1];
	V3DLONG zstep = sz[2];
	return getBlockStdValueVPoint(img4d, sz, channelsToUse, nChannel, 
							x0, y0, z0,
							xstep, ystep, zstep);
}

template <class T> double getImage4DStdValue(T ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel) 
{
	VPoint * v = getImageStdValueVPoint(img4d, sz, channelsToUse, nChannel);
	if (!v) return 0;
	double s = v->abs();
	if(v) {delete v; v=0;}
	return s;
}


#endif

