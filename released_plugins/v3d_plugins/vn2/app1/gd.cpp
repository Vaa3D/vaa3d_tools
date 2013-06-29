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




//dij_bgl.cpp this file is created to trace neuron using shortest path method
//by Hanchuan Peng
//090208
//090511 RZC: extract typedef(Edge, Weight), and the key code to a function bgl_shortest_path()
//090512 PHC: add function phc_shortest_path(), and also add the double-edges
//090514 RZC: find a nice weight function. rearrange code layout
//090516: add header file by Hanchuan Peng
//090518: separate the ParaShortestPath struct
//100327: add find_shortest_path_graphpointset()
//100520: add msvc compiling support
//101230: fix a missing root index map bug 
//110716: add in the mst option

//the folowing conditional compilation is added by PHC, 2010-05-20
#if defined (_MSC_VER)
#include "../../v3d_main/basic_c_fun/vcdiff.h"
#else
#endif

#ifdef _WIN32
#define fabs(x) ((x<0)?-x:x)
#define round(x) (int(x))
#endif


#include "gd.h"

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>

#include <stack>

using namespace boost;

#include "../../v3d_main/graph/dijk.h"

#include "../../v3d_main/v3d/compute_win_pca.h"


// 090529: std::cout => printf => qDebug for non-delayed displaying debug text
//#include <QtCore>
//#define printf qDebug


char* bgl_shortest_path(Edge *edge_array, V3DLONG nedges, Weight *weights, V3DLONG num_nodes, //input graph
						Node start_nodeind,
						Node *plist); //output path
char* phc_shortest_path(Edge *edge_array, V3DLONG nedges, Weight *weights, V3DLONG num_nodes, //input graph
						Node start_nodeind,
						Node *plist); //output path
char * mst_shortest_path(Edge * edge_array, V3DLONG n_edges, Weight * weights, V3DLONG n_nodes, //input graph
						 Node start_nodeind, // input source
						 Node * plist); //add the MST-based shortest path, 110716



double getBlockMaxValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
							V3DLONG x0, V3DLONG y0, V3DLONG z0,
							int xstep, int ystep, int zstep)
{
	if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 ||
		x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2)
		return 0;

	double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
	V3DLONG xs=x0-xsteph, xe=x0+xsteph,
		ys=y0-ysteph, ye=y0+ysteph,
		zs=z0-zsteph, ze=z0+zsteph;

	if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
	if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
	if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;

	V3DLONG i,j,k,n;
	double v=0;
	double mv=0;
	for (k=zs;k<=ze; k++)
		for (j=ys;j<=ye; j++)
			for (i=xs;i<=xe; i++)
			{
				v = double(img3d[k][j][i]);
				if (v>mv) mv = v;
			}
	return mv;
}

double getBlockAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
							V3DLONG x0, V3DLONG y0, V3DLONG z0,
							int xstep, int ystep, int zstep)
{
	if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 ||
		x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2)
		return 0;

	double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
	V3DLONG xs=x0-xsteph, xe=x0+xsteph,
		ys=y0-ysteph, ye=y0+ysteph,
		zs=z0-zsteph, ze=z0+zsteph;

	if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
	if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
	if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;

	V3DLONG i,j,k,n;
	double v=0;
	n=0;
	for (k=zs;k<=ze; k++)
		for (j=ys;j<=ye; j++)
			for (i=xs;i<=xe; i++)
			{
				v += double(img3d[k][j][i]);
				n++;
			}
	return (n==0)?0:v/n;
}

//#define CHECK_DATA4D_VALID_AND_RETURN \
//	if (!img4d || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || \
//		x0<0 || x0>=sz[0] || y0<0 || y0>=sz[1] || z0<0 || z0>=sz[2]) \
//		return 0;
//
//#define CHECK_DATA3D_VALID_AND_RETURN \
//	if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 || \
//		x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2) \
//		return false;
//

//double getBlockAveValue(unsigned char ****img4d, V3DLONG sz[4],
//						V3DLONG x0, V3DLONG y0, V3DLONG z0,
//						int xstep, int ystep, int zstep)
//{
//	CHECK_DATA4D_VALID_AND_RETURN
//	
//	double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
//	V3DLONG xs=x0-xsteph, xe=x0+xsteph,
//	ys=y0-ysteph, ye=y0+ysteph,
//	zs=z0-zsteph, ze=z0+zsteph;
//	
//	if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
//	if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
//	if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;
//	
//	V3DLONG i,j,k,n;
//	double v=0;
//	n=0;
//	for (k=zs;k<=ze; k++)
//		for (j=ys;j<=ye; j++)
//			for (i=xs;i<=xe; i++)
//			{
//				v += double(img3d[k][j][i]);
//				n++;
//			}
//	return (n==0)?0:v/n;
//}




bool setBlockAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
						V3DLONG x0, V3DLONG y0, V3DLONG z0,
						int xstep, int ystep, int zstep, unsigned char target_val)
{
	if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 ||
		x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2)
		return false;
	
	double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
	V3DLONG xs=x0-xsteph, xe=x0+xsteph,
	ys=y0-ysteph, ye=y0+ysteph,
	zs=z0-zsteph, ze=z0+zsteph;
	
	if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
	if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
	if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;
	
	V3DLONG i,j,k;
	for (k=zs;k<=ze; k++)
		for (j=ys;j<=ye; j++)
			for (i=xs;i<=xe; i++)
			{
				img3d[k][j][i] = target_val;
			}
	return true;
}


double getBlockStdValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
							V3DLONG x0, V3DLONG y0, V3DLONG z0,
							int xstep, int ystep, int zstep)
{
	if (!img3d || dim0<=0 || dim1<=0 || dim2<=0 ||
		x0<0 || x0>=dim0 || y0<0 || y0>=dim1 || z0<0 || z0>=dim2)
		return 0;

	double blockAve = getBlockAveValue(img3d, dim0, dim1, dim2,
							x0, y0, z0,
							xstep, ystep, zstep);

	double xsteph=fabs(xstep)/2, ysteph=fabs(ystep)/2, zsteph=fabs(zstep)/2;
	V3DLONG xs=x0-xsteph, xe=x0+xsteph,
		ys=y0-ysteph, ye=y0+ysteph,
		zs=z0-zsteph, ze=z0+zsteph;

	if (xs<0) xs=0; if (xe>=dim0) xe=dim0-1;
	if (ys<0) ys=0; if (ye>=dim1) ye=dim1-1;
	if (zs<0) zs=0; if (ze>=dim2) ze=dim2-1;

	V3DLONG i,j,k,n;
	double v=0;
	n=0;
	for (k=zs;k<=ze; k++)
		for (j=ys;j<=ye; j++)
			for (i=xs;i<=xe; i++)
			{
				double d = (double(img3d[k][j][i]) - blockAve);
				v += d*d;
				n++;
			}
	return (n==0)?0: sqrt(v/n);
}

double getImageMaxValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2)
{
	V3DLONG x0 = dim0/2;
	V3DLONG y0 = dim1/2;
	V3DLONG z0 = dim2/2;
	V3DLONG xstep = dim0;
	V3DLONG ystep = dim1;
	V3DLONG zstep = dim2;
	return getBlockMaxValue(img3d, dim0, dim1, dim2, x0, y0, z0, xstep, ystep, zstep);
}
double getImageAveValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2)
{
	V3DLONG x0 = dim0/2;
	V3DLONG y0 = dim1/2;
	V3DLONG z0 = dim2/2;
	V3DLONG xstep = dim0;
	V3DLONG ystep = dim1;
	V3DLONG zstep = dim2;
	return getBlockAveValue(img3d, dim0, dim1, dim2, x0, y0, z0, xstep, ystep, zstep);
}


double getImageStdValue(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2)
{
	V3DLONG x0 = dim0/2;
	V3DLONG y0 = dim1/2;
	V3DLONG z0 = dim2/2;
	V3DLONG xstep = dim0;
	V3DLONG ystep = dim1;
	V3DLONG zstep = dim2;
	return getBlockStdValue(img3d, dim0, dim1, dim2, x0, y0, z0, xstep, ystep, zstep);
}

//=============================================================================================================
/* comments:
 * bgl_shortest_path is very faster, works well for small step. but seems use approximate solution for large step.
 * phc_shortest_path is slow, not testing the case of small step. but work well for large step.
 * float precision may be not enough for sum of high dynamic weight value,
 *  best solution is to use very V3DLONG precision sum variety within graph algorithm.
*/
//=============================================================================================================


//######################################################
static
struct edge_table_item {
	V3DLONG i0,j0,k0,  i1,j1,k1;  double dist;
} edge_table[13] = {           // in one cube
	{0,0,0, 1,0,0, 1},         // 3 dist=1
	{0,0,0, 0,1,0, 1},
	{0,0,0, 0,0,1, 1},
	{0,0,0, 1,1,0, sqrt(2.0)}, // 6 dist=sqrt(2)
	{0,0,0, 0,1,1, sqrt(2.0)},
	{0,0,0, 1,0,1, sqrt(2.0)},
	{1,0,0, 0,1,0, sqrt(2.0)},
	{0,1,0, 0,0,1, sqrt(2.0)},
	{0,0,1, 1,0,0, sqrt(2.0)},
	{0,0,0, 1,1,1, sqrt(3.0)}, // 4 dist=sqrt(3)
	{1,0,0, 0,1,1, sqrt(3.0)},
	{0,1,0, 1,0,1, sqrt(3.0)},
	{0,0,1, 1,1,0, sqrt(3.0)},
};

//extract code to function
static inline
double similarity_two_vpoints_v1(VPoint *a, VPoint *b) 
{ 
	if (!(a->isComparable(b)))
		return 0; 

	double sa = a->abs();
	double sb = b->abs();
	if (sa==0 || sb==0) return 0;
	
	double s=0;
	for (V3DLONG i=0;i<a->n;i++) s+=	a->v[i] * b->v[i];
	return ((s/(sa*sb)))*(sa+sb)/2.0;
}

static inline
double similarity_two_vpoints_v2(VPoint *a, VPoint *b) 
{ 
	if (!(a->isComparable(b)))
		return 0; 
	
	double st=0, sr=0, t, r;
	for (V3DLONG i=0;i<a->n;i++) 
	{
		t = (a->v[i] - b->v[i]); 
		r = (a->v[i] * b->v[i]);
		st += t*t;
		sr += r;
	}
	st = 255-sqrt(st)/sqrt(3.0); //1 - normalized colors' distance
	double sa = a->abs();
	double sb = b->abs();
	sr = (sr/(sa*sb));
	printf("[%5.3f, %5.3f] ", st, sr);

	//return s*(sa+sb)/2.0;
	return sr; //sr;
}

static inline
double similarity_two_vpoints(VPoint &a, VPoint &b) { return similarity_two_vpoints_v1(&a, &b);}
									   
static inline
double cosAngleBetween(VPoint *a, VPoint *b) 
{ 
	if (!(a->isComparable(b)))
		return 0; 

	double sa = a->abs();
	double sb = b->abs();
	if (sa==0 || sb==0) return 0;
	
	double s=0;
	for (V3DLONG i=0;i<a->n;i++) s+=	a->v[i] * b->v[i];
	return (s/(sa*sb));
}

static inline
double metric_func(double v, double max_v=255)
{
	double tmpv = 1-v/max_v;
	return	exp((tmpv*tmpv)*10); //float min-step:1e-6, min:1e-37, max:1e38
}

static inline
double metric_func(VPoint & p, double max_v=255)
{
	double v = p.abs();
	double tmpv = 1-v/max_v;
	return	exp((tmpv*tmpv)*10); //float min-step:1e-6, min:1e-37, max:1e38
}

static inline
double edge_weight_func(double dist, double va, double vb, double max_v=255)
{
	double m_ab = (metric_func(va, max_v) + metric_func(vb, max_v))*0.5;
	// (metric_func((va + vb, max_v)*0.5);
	
	const double min_weight_step = 1e-5;   //090915 more precise //float min-step:1e-6
	return (dist * m_ab) *min_weight_step;
}

static inline
double edge_weight_func(int it, double va, double vb, double max_v=255)
{
	double dist = edge_table[it].dist;
	double m_ab = (metric_func(va, max_v) + metric_func(vb, max_v))*0.5;
	//printf("[%5.3f] ", m_ab);
				// (metric_func((va + vb, max_v)*0.5);

	const double min_weight_step = 1e-5;   //090915 more precise //float min-step:1e-6
	return (dist * m_ab) *min_weight_step;
}

static inline
double edge_weight_func(int it, VPoint * va, VPoint * vb, double max_v=255)
{
	double dist = edge_table[it].dist;
	//double m_ab = (metric_func(va, max_v) + metric_func(vb, max_v))*0.5;
	//double m_ab = (metric_func(va->innerProduct(vb), max_v)); //20110913. need adjustment later
	double s_ab = similarity_two_vpoints_v1(va, vb);
	double m_ab = (metric_func(s_ab, max_v));
	//if (s_ab>0.5) 
	//	printf("[%5.3f] ", s_ab);
	
	const double min_weight_step = 1e-5;   //090915 more precise //float min-step:1e-6
	return (dist * m_ab) *min_weight_step;
	return s_ab;
	//return dist;
}

	//(sqrt(dist) * tmpv*tmpv);
	//(sqrt(dist) * tmpv*tmpv * exp(fabs(va-vb)/255.0));
	//(dist * exp((tmpv*tmpv-0.5)*10));  //090514 RZC: max float ~ 10^38 //090621 RZC: positive min float ~ 10^-6 //
	//(dist * exp((1-tmpv)*(1-tmpv)*-30));  // min float ~ 10^-37 // not good as above
    //(dist * exp(tmpv*tmpv*30));  // max float ~ 10^38 // the original working scheme, - but maybe not really correct. by PHC. 090622
	//(dist + exp(tmpv*tmpv*10)-1);  // try plus scheme, 090713
//########################################################


char* find_shortest_path_graphimg(unsigned char ****img4d, V3DLONG sz[4], V3DLONG channelsToUse[], V3DLONG nChannel, //image
													 float zthickness, // z-thickness for weighted edge
													 //const V3DLONG box[6],  //bounding box
													 V3DLONG bx0, V3DLONG by0, V3DLONG bz0, V3DLONG bx1, V3DLONG by1, V3DLONG bz1, //bounding box (ROI)
													 float x0, float y0, float z0,       // start node
													 int n_end_nodes,                    // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
													 float x1[], float y1[], float z1[],    // all end nodes
													 vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
													 const ParaShortestPath & para)
{
	//printf("start of find_shortest_path_graphimg \n");
	bool b_error = false;
	char* s_error = 0;
	const float dd = 0.5;
	
	V3DLONG i,j,k,n,m;

	printf("sizeof(Weight) = %d, sizeof(Node) = %d \n", sizeof(Weight), sizeof(Node));
	printf("bounding (%ld %ld %ld)--(%ld %ld %ld) in image (%ld x %ld x %ld)\n", bx0,by0,bz0, bx1,by1,bz1, sz[0],sz[1],sz[2]);
	if (!img4d || sz[0]<=0 || sz[1]<=0 || sz[2]<=0 || sz[3]<=0 || !channelsToUse || nChannel<=0)
	{
		printf(s_error="Error happens: no image data or invalid parameters!\n"); 
		return s_error;
	}
	for (k=0;k<nChannel;k++)
	{
		if (channelsToUse[k]<0 || channelsToUse[k]>=sz[3])
		{
			printf(s_error="Error happens: the channelToUse parameters is not correct!\n"); 
			return s_error;
		}
	}
	V3DLONG dim0 = sz[0], dim1 = sz[1], dim2 = sz[2];
	
	if ((bx0<0-dd || bx0>=dim0-dd || by0<0-dd || by0>=dim1-dd || bz0<0-dd || bz0>=dim2-dd)
		|| (bx1<0-dd || bx1>=dim0-dd || by1<0-dd || by1>=dim1-dd || bz1<0-dd || bz1>=dim2-dd))
	{
		printf(s_error="Error happens: bounding box out of image bound!\n"); 
		printf("inside z1=%ld\n", bz1);;
		return s_error;
	}
	
	//now set parameters
	int min_step       = para.node_step; //should be >=1
	int smooth_winsize = para.smooth_winsize;
	int edge_select    = para.edge_select;  //0 -- only use length 1 edge(optimal for small step), 1 -- plus diagonal edge
	double imgTH = para.imgTH; //anything <= imgTH will NOT be traced!
	
	if (min_step<1)       min_step =1;
	if (smooth_winsize<1) smooth_winsize =1;
	
	//bounding box volume
	V3DLONG xmin = bx0, xmax = bx1,
	ymin = by0, ymax = by1,
	zmin = bz0, zmax = bz1;
	
	V3DLONG nx=((xmax-xmin)/min_step)+1, 	xstep=min_step,
	ny=((ymax-ymin)/min_step)+1, 	ystep=min_step,
	nz=((zmax-zmin)/min_step)+1, 	zstep=min_step;
	
	V3DLONG num_edge_table = (edge_select==0)? 3:13; // exclude/include diagonal-edge
	
	printf("valid bounding (%ld %ld %ld)--(%ld %ld %ld) ......  ", xmin,ymin,zmin, xmax,ymax,zmax);
	printf("%ld x %ld x %ld nodes, step = %d, connect = %d \n", nx, ny, nz, min_step, num_edge_table*2);
	
	V3DLONG num_nodes = nx*ny*nz;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NODE_FROM_XYZ(x,y,z) 	(V3DLONG((z+.5)-zmin)/zstep*ny*nx + V3DLONG((y+.5)-ymin)/ystep*nx + V3DLONG((x+.5)-xmin)/xstep)
#define NODE_TO_XYZ(j, x,y,z) \
{ \
z = (j)/(nx*ny); 		y = ((j)-V3DLONG(z)*nx*ny)/nx; 	x = ((j)-V3DLONG(z)*nx*ny-V3DLONG(y)*nx); \
x = xmin+(x)*xstep; 	y = ymin+(y)*ystep; 			z = zmin+(z)*zstep; \
}
#define NODE_FROM_IJK(i,j,k) 	((k)*ny*nx+(j)*nx+(i))
#define X_I(i)				 	(xmin+(i)*xstep)
#define Y_I(i)				 	(ymin+(i)*ystep)
#define Z_I(i)				 	(zmin+(i)*zstep)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//out of bound handler
#define NODE_XYZ_OUT_OF_BOUND(x0,y0,z0)		(x0<xmin-dd || x0>xmax+dd || y0<ymin-dd || y0>ymax+dd || z0<zmin-dd || z0>zmax+dd)
#define NODE_INDEX_OUT_OF_BOUND(ind) 		(ind<0 || ind>=num_nodes)
	
	V3DLONG start_nodeind, *end_nodeind = 0;
	if (n_end_nodes>0) //101210 PHC
		end_nodeind = new V3DLONG [n_end_nodes]; //100520, PHC	
	else
		printf("**************** n_end_nodes is 0, and thus do not need to allocate memory. *********************\n");
	
	
	if (NODE_XYZ_OUT_OF_BOUND(x0,y0,z0))
	{
		printf(s_error="Error happens: start_node out of bound! \n");
		return s_error;
	}
	start_nodeind = NODE_FROM_XYZ(x0,y0,z0);
	if (NODE_INDEX_OUT_OF_BOUND(start_nodeind))
	{
		printf(s_error="Error happens: start_node index out of range! \n");
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}
	
	V3DLONG n_end_outbound = 0;
	for (i=0; i<n_end_nodes; i++)
	{
		if (NODE_XYZ_OUT_OF_BOUND(x1[i],y1[i],z1[i]))
		{
			end_nodeind[i] = -1;
			printf("Warning: end_node[%d] out of bound! \n", i);
			n_end_outbound ++;
			continue; //ignore this end_node out of ROI
		}
		end_nodeind[i]   = NODE_FROM_XYZ(x1[i],y1[i],z1[i]);
		if (NODE_INDEX_OUT_OF_BOUND(end_nodeind[i]))
		{
			end_nodeind[i] = -1;
			printf("Warning: end_node[%d] index out of range! \n", i);
			n_end_outbound ++;
			continue; //ignore this end_node out of ROI
		}
	}
	
	if (n_end_nodes>0 //for 1-to-N, not 1-to-image
		&& n_end_outbound>=n_end_nodes)
	{
		printf(s_error="Error happens: all end_nodes out of bound! At least one end_node must be in bound.\n");
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}
	
	
#define _creating_graph_
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//switch back to new[] from std::vector for *** glibc detected *** ??? on Linux
	std::vector<Node> 	plist(num_nodes);		for (i=0;i<num_nodes;i++) plist[i]=i;
	std::vector<Edge> 	edge_array;				edge_array.clear();
	std::vector<Weight>	weights;				weights.clear();
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	
#define _setting_weight_of_edges_
	printf("setting weight of edges ......  ");
	
	// z-thickness weighted edge
	for (V3DLONG it=0; it<num_edge_table; it++)
	{
		double di = (edge_table[it].i0 - edge_table[it].i1);
		double dj = (edge_table[it].j0 - edge_table[it].j1);
		double dk = (edge_table[it].k0 - edge_table[it].k1) * zthickness;
		edge_table[it].dist = sqrt(di*di + dj*dj + dk*dk);
	}
	
	int block_ave_method=0;
	VPoint *va=0, *vb=0;
	double maxw=0, minw=1e+6; //for debug info 
	n=0; m=0;
	for (k=0;k<nz;k++)
	{
		for (j=0;j<ny;j++)
		{
			for (i=0;i<nx;i++)
			{
				for (int it=0; it<num_edge_table; it++)
				{
					// take an edge
					V3DLONG ii = i+ edge_table[it].i0;
					V3DLONG jj = j+ edge_table[it].j0;
					V3DLONG kk = k+ edge_table[it].k0;
					V3DLONG ii1 = i+ edge_table[it].i1;
					V3DLONG jj1 = j+ edge_table[it].j1;
					V3DLONG kk1 = k+ edge_table[it].k1;
					
					if (ii>=nx || jj>=ny || kk>=nz || ii1>=nx || jj1>=ny || kk1>=nz) continue;//for boundary condition
					
					V3DLONG node_a = NODE_FROM_IJK(ii,jj,kk);
					V3DLONG node_b = NODE_FROM_IJK(ii1,jj1,kk1);
					
					m++;
					
					//=========================================================================================
					// edge link
					va = getBlockAveValueVPoint(img4d, sz, channelsToUse, nChannel, X_I(ii),Y_I(jj),Z_I(kk),
										  xstep, ystep, (zstep/zthickness), block_ave_method); //zthickness
					vb = getBlockAveValueVPoint(img4d, sz, channelsToUse, nChannel, X_I(ii1),Y_I(jj1),Z_I(kk1),
										  xstep, ystep, (zstep/zthickness), block_ave_method); //zthickness
					if (!va || !vb)
					{
						if (va) {delete va; va=0;}
						if (vb) {delete vb; vb=0;}
						continue;
					}
					
					if (va->abs()>255) printf("va-abs()=%5.3f RGB=[%5.3f,%5.3f,%5.3f]\n", va->abs(), (va->v[0]), (va->v[1]), (va->v[2]));
					
					if (va->abs()<=imgTH || vb->abs()<=imgTH || cosAngleBetween(va, va)<0.866) //larger than 45 degrees then there is no edge
						continue; //skip background node link
					
					Edge e = Edge(node_a, node_b);
					edge_array.push_back( e );
					
					Weight w =	edge_weight_func(it, va,vb, 442); //442=255*sqrt(3) 
					weights.push_back( w );
					//printf("w=%5.3f a_RGB=[%5.3f,%5.3f,%5.3f] b_RGB=[%5.3f,%5.3f,%5.3f]\n", w, (va->v[0]), (va->v[1]), (va->v[2]), (vb->v[0]), (vb->v[1]), (vb->v[2]));
					
					{delete va; va=0;}
					{delete vb; vb=0;}
					//=========================================================================================
					
					n++; // that is the correct position of n++
					
					if (w>maxw) maxw=w;	if (w<minw) minw=w;
				}
			}
		}
	}
	printf(" minw=%g maxw=%g ", minw,maxw);
	printf(" graph defined! \n");
	
	if (n != edge_array.size())
	{
		printf(s_error="The number of edges is not consistent \n");
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}
	V3DLONG num_edges = n; // back to undirectEdge for less memory consumption
	
	//printf("image average =%g, std =%g, max =%g.  select %ld out of %ld links \n", imgAve, imgStd, imgMax, n, m);
	printf("select %ld out of %ld links \n", n, m);
	printf("total %ld nodes, total %ld edges \n", num_nodes, num_edges);
	printf("start from #%ld to ", start_nodeind);
	for(V3DLONG i=0; i<n_end_nodes; i++) printf("#%ld ", end_nodeind[i]); printf("\n");
	printf("---------------------------------------------------------------\n");
	
	
	
#define _do_shortest_path_algorithm_
	//========================================================================================================
	
	int code_select = 0; // BGL has the best speed and correctness
	switch(code_select)
	{
		case 0:
			printf("bgl_shortest_path() \n");
			s_error = bgl_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes, start_nodeind, &plist[0]);
			break;
		case 1:
			printf("phc_shortest_path() \n");
			s_error = phc_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
			break;
		case 2:
			printf("mst_shortest_path() \n");
			s_error = phc_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
			break;
	}
	if (s_error)
	{
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}
	//=========================================================================================================
	//for (i=0;i<num_nodes;i++)	std::cout<<"p("<<i<<")="<<plist[i]<<";   ";  std::cout<<std::endl;
	
	
	// output node coordinates of the shortest path
	mmUnit.clear();
	V3DLONG nexist = 0;
	
	V_NeuronSWC_unit cc;
	vector<V_NeuronSWC_unit> mUnit;
	
	if (n_end_nodes==0) // trace from start-->each possible node
	{
#define _output_shortest_path_TREE_
		printf("trace from start-->each possible node \n");
		mUnit.clear();
		
		std::map<double,V3DLONG> index_map;	index_map.clear();
		// set nchild=0
		for (j=0; j<num_nodes; j++)
		{
			if (j==start_nodeind)
			{
				cc.x = x0;
				cc.y = y0;
				cc.z = z0;
				cc.n = 1+j;
				cc.parent = -1;
				cc.nchild = 0; // although root isn't leaf, but the different should be told by checking their parent nodes instead of number of children. changed from 1 to 0, by PHC, 20110908. I think root can also be a leaf, 
				mUnit.push_back(cc);
				index_map[cc.n] = mUnit.size()-1; //fix the missing line bug by PHC, 2010-12-30
				printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
			}
			else if ( (k=plist[j]) != j ) // has parent
				if (k>=0 && k<num_nodes)  // is valid
				{
					NODE_TO_XYZ(j, cc.x, cc.y, cc.z);
					cc.n = 1+j;
					cc.parent = 1+k; //k=plist[j]
					cc.nchild = 0;
					mUnit.push_back(cc);
					//printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
					
					index_map[cc.n] = mUnit.size()-1;
				}
		}
		
		printf("counting parent->nchild \n");
		// count parent->nchild
		for (j=0; j<mUnit.size(); j++)
		{
			double parent = mUnit[j].parent;
			V3DLONG i = index_map[parent]; // this is very fast
			
			mUnit[i].nchild++;
		}
		
		double myTH = imgTH; if (myTH<para.visible_thresh) myTH=para.visible_thresh;
		if (1)
		{
			printf("labeling to remove bark leaf child \n");
			printf("before dark-pruning there are %ld nodes in total, from %ld nodes in the initial graph. \n", mUnit.size(), num_nodes);
			//remove leaf node (nchild==0)
			
			int min_cut_level = 10/min_step;	//maybe can change to 1 ? 2011-01-13. by PHC
			if (min_cut_level<1) min_cut_level=1;
			VPoint *vroot = getBlockAveValueVPoint(img4d, sz, channelsToUse, nChannel, x0, y0, z0,
											 min_cut_level, min_cut_level, min_cut_level, 0);
			
			for (k=0; ; k++)
			{
				V3DLONG nprune=0;
				for (j=0; j<mUnit.size(); j++)
				{
					if (mUnit[j].nchild ==0)
					{
						double parent = mUnit[j].parent;
						V3DLONG i = index_map[parent];
						
						VPoint *va = getBlockAveValueVPoint(img4d, sz, channelsToUse, nChannel, mUnit.at(j).x,mUnit.at(j).y,mUnit.at(j).z,
													 min_cut_level, min_cut_level, min_cut_level, 0);
						//VPoint *vp = getBlockAveValue(img4d, sz, channelsToUse, nChannel, mUnit.at(i).x,mUnit.at(i).y,mUnit.at(i).z,
						//							  min_cut_level, min_cut_level, min_cut_level, 0);
						
						if (va 
							//&& vp
							)
						{
							if (va->abs() <= myTH  || //dark pruning
							//	cosAngleBetween(va,vp) < 0.866
								cosAngleBetween(va,vroot) < 0.866
								) //this gives a symmetric pruning, seems better than (k<5) criterion which leads to an asymmetric prunning
							{
								mUnit[i].nchild--;
								//label to remove
								mUnit[j].nchild = -1;	//mUnit[j].parent = mUnit[j].n; //seems no need for this
								nprune++;
							}
						}
						
						if (va) {delete va; va=0;}
						//if (vp) {delete vp; vp=0;}
					}
				}
				
				printf("dark prune loop %ld. remove %ld nodes.\n", k, nprune);
				if (nprune==0)
					break;
			}
			
			if (vroot) {delete vroot; vroot=0;}
		}
		
		//remove those nchild<0 and rearraneg indexes
		
		mmUnit.push_back(mUnit);
		rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
		
		printf("done with the SP step. \n");
	}
	
	else
		for (V3DLONG npath=0; npath<n_end_nodes; npath++) // n path of back tracing end-->start
		{
#define _output_shortest_path_N_
			printf("the #%d path of back tracing end-->start \n", npath+1);
			mUnit.clear();
			
			j = end_nodeind[npath]; //search from the last one
			cc.x = x1[npath];
			cc.y = y1[npath];
			cc.z = z1[npath];
			cc.n = nexist +1+mUnit.size();
			cc.parent = cc.n +1;
			printf("[end: x y z] %ld: %g %g %g ", j, cc.x, cc.y, cc.z);
			if (j<0 || j>=num_nodes) // for the end_node out of ROI
			{
				printf(" end_node is out of ROI, ignored.\n");
				continue;
			}
			printf("\n");
			
			mUnit.push_back(cc);
			
			for (k=0;k<n;k++) //at most n edge links
			{
				V3DLONG jj = j;	j = plist[j];
				
				if (j==jj)
				{
					mUnit.clear();
					printf(s_error="Error happens: this path is broken because a node has a self-link!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
					break;
				}
				else if (j>=num_nodes)
				{
					mUnit.clear();
					printf(s_error="Error happens: this node's parent has an index out of range!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
					break;
				}
				else if (j<0) // should not be reached, because stop back trace at his child node
				{
					mUnit.clear();
					printf(s_error="find the negative node, which should indicate the root has been over-reached."); printf(" [j->p(j)] %ld->%ld \n", jj, j);
					break;
				}
				
				if (j!=start_nodeind)
				{
					NODE_TO_XYZ(j, cc.x, cc.y, cc.z);
					cc.n = nexist +1+mUnit.size();
					cc.parent = cc.n +1;
					mUnit.push_back(cc);
					//printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
				}
				else //j==start_nodeind
				{
					cc.x = x0;
					cc.y = y0;
					cc.z = z0;
					cc.n = nexist +1+mUnit.size();
					cc.parent = -1;
					mUnit.push_back(cc);
					printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
					
					break; //STOP back tracing
				}
			}
			nexist += mUnit.size();
			
			if (mUnit.size()>=2)	mmUnit.push_back(mUnit);
		}
	
	//	//also can do smoothing outside in proj_trace_smooth_dwonsample_last_traced_neuron
	//	printf("smooth_curve + downsample_curve \n");
	//	smooth_curve(mCoord, smooth_winsize);
	//	mCoord = downsample_curve(mCoord, outsample_step);
	
	if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
	
	if (mmUnit.size())	return 0;
	return s_error;
}


// return error message, 0 is no error
// 
char* find_shortest_path_graphimg(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, //image
		float zthickness, // z-thickness for weighted edge
		//const V3DLONG box[6],  //bounding box
		V3DLONG bx0, V3DLONG by0, V3DLONG bz0, V3DLONG bx1, V3DLONG by1, V3DLONG bz1, //bounding box (ROI)
							float x0, float y0, float z0,       // start node
							int n_end_nodes,                    // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
							float x1[], float y1[], float z1[],    // all end nodes
							vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
							const ParaShortestPath & para)
{
	//printf("start of find_shortest_path_graphimg \n");
	bool b_error = false;
	char* s_error = 0;
	const float dd = 0.5;

	printf("sizeof(Weight) = %d, sizeof(Node) = %d \n", sizeof(Weight), sizeof(Node));
	printf("bounding (%ld %ld %ld)--(%ld %ld %ld) in image (%ld x %ld x %ld)\n", bx0,by0,bz0, bx1,by1,bz1, dim0,dim1,dim2);
	if (!img3d || dim0<=0 || dim1<=0 || dim2<=0)
	{
		printf(s_error="Error happens: no image data!\n"); 
		return s_error;
	}
	if ((bx0<0-dd || bx0>=dim0-dd || by0<0-dd || by0>=dim1-dd || bz0<0-dd || bz0>=dim2-dd)
	   || (bx1<0-dd || bx1>=dim0-dd || by1<0-dd || by1>=dim1-dd || bz1<0-dd || bz1>=dim2-dd))
	{
		printf(s_error="Error happens: bounding box out of image bound!\n"); 
		printf("inside z1=%ld\n", bz1);;
		return s_error;
	}

	//now set parameters
	int min_step       = para.node_step; //should be >=1
	int smooth_winsize = para.smooth_winsize;
	int edge_select    = para.edge_select;  //0 -- only use length 1 edge(optimal for small step), 1 -- plus diagonal edge
	double imgTH = para.imgTH; //anything <= imgTH will NOT be traced!

	if (min_step<1)       min_step =1;
	if (smooth_winsize<1) smooth_winsize =1;

	//bounding box volume
	V3DLONG xmin = bx0, xmax = bx1,
	     ymin = by0, ymax = by1,
		 zmin = bz0, zmax = bz1;

	V3DLONG nx=((xmax-xmin)/min_step)+1, 	xstep=min_step,
		 ny=((ymax-ymin)/min_step)+1, 	ystep=min_step,
		 nz=((zmax-zmin)/min_step)+1, 	zstep=min_step;

	V3DLONG num_edge_table = (edge_select==0)? 3:13; // exclude/include diagonal-edge

	printf("valid bounding (%ld %ld %ld)--(%ld %ld %ld) ......  ", xmin,ymin,zmin, xmax,ymax,zmax);
	printf("%ld x %ld x %ld nodes, step = %d, connect = %d \n", nx, ny, nz, min_step, num_edge_table*2);

	V3DLONG num_nodes = nx*ny*nz;
	V3DLONG i,j,k,n,m;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	#define NODE_FROM_XYZ(x,y,z) 	(V3DLONG((z+.5)-zmin)/zstep*ny*nx + V3DLONG((y+.5)-ymin)/ystep*nx + V3DLONG((x+.5)-xmin)/xstep)
	#define NODE_TO_XYZ(j, x,y,z) \
	{ \
		z = (j)/(nx*ny); 		y = ((j)-V3DLONG(z)*nx*ny)/nx; 	x = ((j)-V3DLONG(z)*nx*ny-V3DLONG(y)*nx); \
		x = xmin+(x)*xstep; 	y = ymin+(y)*ystep; 			z = zmin+(z)*zstep; \
	}
	#define NODE_FROM_IJK(i,j,k) 	((k)*ny*nx+(j)*nx+(i))
	#define X_I(i)				 	(xmin+(i)*xstep)
	#define Y_I(i)				 	(ymin+(i)*ystep)
	#define Z_I(i)				 	(zmin+(i)*zstep)
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//out of bound handler
	#define NODE_XYZ_OUT_OF_BOUND(x0,y0,z0)		(x0<xmin-dd || x0>xmax+dd || y0<ymin-dd || y0>ymax+dd || z0<zmin-dd || z0>zmax+dd)
	#define NODE_INDEX_OUT_OF_BOUND(ind) 		(ind<0 || ind>=num_nodes)

	V3DLONG start_nodeind, *end_nodeind = 0;
	if (n_end_nodes>0) //101210 PHC
		end_nodeind = new V3DLONG [n_end_nodes]; //100520, PHC	
	else
		printf("**************** n_end_nodes is 0, and thus do not need to allocate memory. *********************\n");
		
		
	if (NODE_XYZ_OUT_OF_BOUND(x0,y0,z0))
	{
		printf(s_error="Error happens: start_node out of bound! \n");
		return s_error;
	}
	start_nodeind = NODE_FROM_XYZ(x0,y0,z0);
	if (NODE_INDEX_OUT_OF_BOUND(start_nodeind))
	{
		printf(s_error="Error happens: start_node index out of range! \n");
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}

	V3DLONG n_end_outbound = 0;
	for (i=0; i<n_end_nodes; i++)
	{
		if (NODE_XYZ_OUT_OF_BOUND(x1[i],y1[i],z1[i]))
		{
			end_nodeind[i] = -1;
			printf("Warning: end_node[%d] out of bound! \n", i);
			n_end_outbound ++;
			continue; //ignore this end_node out of ROI
		}
		end_nodeind[i]   = NODE_FROM_XYZ(x1[i],y1[i],z1[i]);
		if (NODE_INDEX_OUT_OF_BOUND(end_nodeind[i]))
		{
			end_nodeind[i] = -1;
			printf("Warning: end_node[%d] index out of range! \n", i);
			n_end_outbound ++;
			continue; //ignore this end_node out of ROI
		}
	}

	if (n_end_nodes>0 //for 1-to-N, not 1-to-image
		&& n_end_outbound>=n_end_nodes)
	{
		printf(s_error="Error happens: all end_nodes out of bound! At least one end_node must be in bound.\n");
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}


#define _creating_graph_

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//switch back to new[] from std::vector for *** glibc detected *** ??? on Linux
	std::vector<Node> 	plist(num_nodes);		for (i=0;i<num_nodes;i++) plist[i]=i;
	std::vector<Edge> 	edge_array;				edge_array.clear();
	std::vector<Weight>	weights;				weights.clear();
	///////////////////////////////////////////////////////////////////////////////////////////////////////

#define _setting_weight_of_edges_
	printf("setting weight of edges ......  ");

	// z-thickness weighted edge
	for (V3DLONG it=0; it<num_edge_table; it++)
	{
		double di = (edge_table[it].i0 - edge_table[it].i1);
		double dj = (edge_table[it].j0 - edge_table[it].j1);
		double dk = (edge_table[it].k0 - edge_table[it].k1) * zthickness;
		edge_table[it].dist = sqrt(di*di + dj*dj + dk*dk);
	}

	double va, vb;
	double maxw=0, minw=1e+6; //for debug info
	n=0; m=0;
	for (k=0;k<nz;k++)
	{
		for (j=0;j<ny;j++)
		{
			for (i=0;i<nx;i++)
			{
				for (int it=0; it<num_edge_table; it++)
				{
					// take an edge
					V3DLONG ii = i+ edge_table[it].i0;
					V3DLONG jj = j+ edge_table[it].j0;
					V3DLONG kk = k+ edge_table[it].k0;
					V3DLONG ii1 = i+ edge_table[it].i1;
					V3DLONG jj1 = j+ edge_table[it].j1;
					V3DLONG kk1 = k+ edge_table[it].k1;

					if (ii>=nx || jj>=ny || kk>=nz || ii1>=nx || jj1>=ny || kk1>=nz) continue;//for boundary condition

					V3DLONG node_a = NODE_FROM_IJK(ii,jj,kk);
					V3DLONG node_b = NODE_FROM_IJK(ii1,jj1,kk1);

					m++;

					//=========================================================================================
					// edge link
					va = getBlockAveValue(img3d, dim0, dim1, dim2, X_I(ii),Y_I(jj),Z_I(kk),
							xstep, ystep, (zstep/zthickness)); //zthickness
					vb = getBlockAveValue(img3d, dim0, dim1, dim2, X_I(ii1),Y_I(jj1),Z_I(kk1),
							xstep, ystep, (zstep/zthickness)); //zthickness
					if (va<=imgTH || vb<=imgTH)
						continue; //skip background node link

					Edge e = Edge(node_a, node_b);
					edge_array.push_back( e );

					Weight w =	edge_weight_func(it, va,vb, 255); 
					weights.push_back( w );
					//=========================================================================================

					n++; // that is the correct position of n++

					if (w>maxw) maxw=w;	if (w<minw) minw=w;
				}
			}
		}
	}
	printf(" minw=%g maxw=%g ", minw,maxw);
	printf(" graph defined! \n");

	if (n != edge_array.size())
	{
		printf(s_error="The number of edges is not consistent \n");
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}
	V3DLONG num_edges = n; // back to undirectEdge for less memory consumption

	//printf("image average =%g, std =%g, max =%g.  select %ld out of %ld links \n", imgAve, imgStd, imgMax, n, m);
	printf("select %ld out of %ld links \n", n, m);
	printf("total %ld nodes, total %ld edges \n", num_nodes, num_edges);
	printf("start from #%ld to ", start_nodeind);
	for(V3DLONG i=0; i<n_end_nodes; i++) printf("#%ld ", end_nodeind[i]); printf("\n");
	printf("---------------------------------------------------------------\n");



#define _do_shortest_path_algorithm_
	//========================================================================================================

	int code_select = 0; // BGL has the best speed and correctness
	switch(code_select)
	{
		case 0:
			printf("bgl_shortest_path() \n");
			s_error = bgl_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes, start_nodeind, &plist[0]);
			break;
		case 1:
			printf("phc_shortest_path() \n");
			s_error = phc_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
			break;
		case 2:
			printf("mst_shortest_path() \n");
			s_error = phc_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
			break;
	}
	if (s_error)
	{
		if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC
		return s_error;
	}
	//=========================================================================================================
	//for (i=0;i<num_nodes;i++)	std::cout<<"p("<<i<<")="<<plist[i]<<";   ";  std::cout<<std::endl;


	// output node coordinates of the shortest path
	mmUnit.clear();
	V3DLONG nexist = 0;

	V_NeuronSWC_unit cc;
	vector<V_NeuronSWC_unit> mUnit;

	if (n_end_nodes==0) // trace from start-->each possible node
	{
#define _output_shortest_path_TREE_
		printf("trace from start-->each possible node \n");
		mUnit.clear();

		std::map<double,V3DLONG> index_map;	index_map.clear();
		// set nchild=0
		for (j=0; j<num_nodes; j++)
		{
			if (j==start_nodeind)
			{
				cc.x = x0;
				cc.y = y0;
				cc.z = z0;
				cc.n = 1+j;
				cc.parent = -1;
				cc.nchild = 0; // although root isn't leaf, but the different should be told by checking their parent nodes instead of number of children. changed from 1 to 0, by PHC, 20110908. I think root can also be a leaf, 
				mUnit.push_back(cc);
				index_map[cc.n] = mUnit.size()-1; //fix the missing line bug by PHC, 2010-12-30
				printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
			}
			else if ( (k=plist[j]) != j ) // has parent
				if (k>=0 && k<num_nodes)  // is valid
			{
				NODE_TO_XYZ(j, cc.x, cc.y, cc.z);
				cc.n = 1+j;
				cc.parent = 1+k; //k=plist[j]
				cc.nchild = 0;
				mUnit.push_back(cc);
				//printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);

				index_map[cc.n] = mUnit.size()-1;
			}
		}

		printf("counting parent->nchild \n");
		// count parent->nchild
		for (j=0; j<mUnit.size(); j++)
		{
			double parent = mUnit[j].parent;
			V3DLONG i = index_map[parent]; // this is very fast

			mUnit[i].nchild++;
		}

		double myTH = imgTH; if (myTH<para.visible_thresh) myTH=para.visible_thresh;
		if (1)
		{
			printf("labeling to remove bark leaf child \n");
			printf("before dark-pruning there are %ld nodes in total, from %ld nodes in the initial graph. \n", mUnit.size(), num_nodes);
			//remove leaf node (nchild==0)
			
			for (k=0; ; k++)
			{
				V3DLONG nprune=0;
				for (j=0; j<mUnit.size(); j++)
				{
					if (mUnit[j].nchild ==0)
					{
						double parent = mUnit[j].parent;
						V3DLONG i = index_map[parent];

						int min_cut_level = 10/min_step;	//maybe can change to 1 ? 2011-01-13. by PHC
						if (min_cut_level<1) min_cut_level=1;
						//double va = getBlockAveValue(img3d, dim0, dim1, dim2, mUnit.at(i).x,mUnit.at(i).y,mUnit.at(i).z,
						//		min_cut_level, min_cut_level, min_cut_level);
						double va = getBlockAveValue(img3d, dim0, dim1, dim2, mUnit.at(j).x,mUnit.at(j).y,mUnit.at(j).z,
													 min_cut_level, min_cut_level, min_cut_level);

						//if (k<min_cut_level || va <= imgAve+imgStd*min_cut_level)
					
						if (va <= myTH  //dark pruning
							//|| (k<5 
										   //&& mUnit[i].nchild >=2
							//			   )
							) //this gives a symmetric pruning, seems better than (k<5) criterion which leads to an asymmetric prunning
						{
							mUnit[i].nchild--;
							//label to remove
							mUnit[j].nchild = -1;	//mUnit[j].parent = mUnit[j].n; //seems no need for this
							nprune++;
						}

	// no need to do at this moment. by PHC, 101226
	//					else //091108. update this node's coordinate using its CoM
	//					{
	//						if (1) //091120. temporary block the code for the demo
	//						{
	//							float curx= mUnit.at(i).x, cury = mUnit.at(i).y, curz = mUnit.at(i).z;
	//							fitPosition(img3d, dim0, dim1, dim2, imgTH, 3*min_cut_level, curx, cury, curz);
	//							mUnit.at(i).x = curx, mUnit.at(i).y = cury, mUnit.at(i).z = curz;
	//						}
	//					}
					}
				}
				
				if (0)
				for (j=mUnit.size()-1; j>=0; j--)
				{
					if (mUnit[j].nchild ==0)
					{
						double parent = mUnit[j].parent;
						V3DLONG i = index_map[parent];
						
						int min_cut_level = 10/min_step;	if (min_cut_level<1) min_cut_level=1;
						double va = getBlockAveValue(img3d, dim0, dim1, dim2, mUnit.at(i).x,mUnit.at(i).y,mUnit.at(i).z,
													 min_cut_level, min_cut_level, min_cut_level);
						
						if (va <= myTH || (k<5 
										   //&& mUnit[i].nchild >=2
										   )) //this gives a symmetric pruning, seems better than (k<5) criterion which leads to an asymmetric prunning
						{
							mUnit[i].nchild--;
							mUnit[j].nchild = -1; 							//label to remove
						}
					}
				}
				
				printf("dark prune loop %ld. remove %ld nodes.\n", k, nprune);
				if (nprune==0)
					break;
			}
		}

		//remove those nchild<0 and rearraneg indexes
		
		mmUnit.push_back(mUnit);
		rearrange_and_remove_labeled_deletion_nodes_mmUnit(mmUnit);
		
		printf("done with the SP step. \n");
	}

	else
	for (V3DLONG npath=0; npath<n_end_nodes; npath++) // n path of back tracing end-->start
	{
#define _output_shortest_path_N_
		printf("the #%d path of back tracing end-->start \n", npath+1);
		mUnit.clear();

		j = end_nodeind[npath]; //search from the last one
		cc.x = x1[npath];
		cc.y = y1[npath];
		cc.z = z1[npath];
		cc.n = nexist +1+mUnit.size();
		cc.parent = cc.n +1;
		printf("[end: x y z] %ld: %g %g %g ", j, cc.x, cc.y, cc.z);
		if (j<0 || j>=num_nodes) // for the end_node out of ROI
		{
			printf(" end_node is out of ROI, ignored.\n");
			continue;
		}
		printf("\n");

		mUnit.push_back(cc);

		for (k=0;k<n;k++) //at most n edge links
		{
			V3DLONG jj = j;	j = plist[j];

			if (j==jj)
			{
				mUnit.clear();
				printf(s_error="Error happens: this path is broken because a node has a self-link!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
				break;
			}
			else if (j>=num_nodes)
			{
				mUnit.clear();
				printf(s_error="Error happens: this node's parent has an index out of range!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
				break;
			}
			else if (j<0) // should not be reached, because stop back trace at his child node
			{
				mUnit.clear();
				printf(s_error="find the negative node, which should indicate the root has been over-reached."); printf(" [j->p(j)] %ld->%ld \n", jj, j);
				break;
			}

			if (j!=start_nodeind)
			{
				NODE_TO_XYZ(j, cc.x, cc.y, cc.z);
				cc.n = nexist +1+mUnit.size();
				cc.parent = cc.n +1;
				mUnit.push_back(cc);
				//printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
			}
			else //j==start_nodeind
			{
				cc.x = x0;
				cc.y = y0;
				cc.z = z0;
				cc.n = nexist +1+mUnit.size();
				cc.parent = -1;
				mUnit.push_back(cc);
				printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);

				break; //STOP back tracing
			}
		}
		nexist += mUnit.size();

		if (mUnit.size()>=2)	mmUnit.push_back(mUnit);
	}

//	//also can do smoothing outside in proj_trace_smooth_dwonsample_last_traced_neuron
//	printf("smooth_curve + downsample_curve \n");
//	smooth_curve(mCoord, smooth_winsize);
//	mCoord = downsample_curve(mCoord, outsample_step);

	if (end_nodeind) {delete []end_nodeind; end_nodeind=0;} //100520, by PHC

	if (mmUnit.size())	return 0;
	return s_error;
}

// return error message, 0 is no error
// 
char* find_shortest_path_graphpointset(V3DLONG n_all_nodes,
								  double xa[], double ya[], double za[], double va[], //the coordinates and values of all nodes
								   float zthickness, // z-thickness for weighted edge
								   std::vector<Edge> 	edge_array0,
								  V3DLONG ind_startnode,        // start node's index 
								  V3DLONG n_end_nodes0,          // n_end_nodes == (0 for shortest path tree) (1 for shortest path) (n-1 for n pair path)
								  V3DLONG ind_end_nodes0[],      // all end nodes' indexes
								  vector< vector<V_NeuronSWC_unit> >& mmUnit, // change from Coord3D for shortest path tree
								  const ParaShortestPath & para)
{
	//printf("start of find_shortest_path_graphpointset \n");
	bool b_error = false;
	char* s_error = 0;
	V3DLONG i,j,k,n,m;

	printf("sizeof(Weight) = %d, sizeof(Node) = %d \n", sizeof(Weight), sizeof(Node));
	if (!xa || !ya || !za || !va || n_all_nodes<=0 || ind_startnode<0 || ind_startnode>=n_all_nodes)
	{
		printf(s_error="Error happens: no graph nodes's info was supplied in the input data!\n"); 
		return s_error;
	}
	V3DLONG n_end_outbound=0, n_end_nodes = 0;
	std::vector <V3DLONG> ind_end_nodes;
	for (i=0, n_end_outbound=0;i<n_end_nodes0;i++)
	{
		if (ind_end_nodes0[i]<0 || ind_end_nodes0[i]>=n_all_nodes)
		{
			n_end_outbound++;
			printf("Found an invalid end node index [%ld]\n", i);
		}
		else
		{
			n_end_nodes++;
			ind_end_nodes.push_back(ind_end_nodes0[i]);
		}
	}
	if (n_end_nodes0>0 && n_end_outbound>=n_end_nodes0) //n_end_nodes0>0 set the condition that this is not one-to-all-others
	{
		printf(s_error="Error happens: all end nodes' indexes are out the valid bound. Check your data. Do nothing!\n"); 
		return s_error;
	}
	
	//now set parameters
	V3DLONG num_nodes = n_all_nodes;
	
#define DIST_TWO_PTS(x1,y1,z1,x2,y2,z2,zthickness) 	(sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2)*zthickness*zthickness))

	V3DLONG start_nodeind = ind_startnode;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//switch back to new[] from std::vector for *** glibc detected *** ??? on Linux
	std::vector<Node> 	plist(num_nodes);		for (i=0;i<num_nodes;i++) plist[i]=i;
	std::vector<Edge> 	edge_array;				edge_array.clear();
	std::vector<Weight>	weights;				weights.clear();
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	
#define _setting_weight_of_edges_
	printf("setting weight of edges ......  ");
	
	// set up the graph. note that the edge is z-thickness weighted 
	double maxw=0, minw=1e+6; //for debug info
	double cur_dist;
	n=0; m=0;

	for (V3DLONG ie=0;ie<edge_array0.size();ie++)
	{
		// take an edge
		m=m+2;
		k = edge_array0.at(ie).first;
		j = edge_array0.at(ie).second;
			
		//=========================================================================================
		// edge link
		cur_dist = DIST_TWO_PTS(xa[k], ya[k], za[k], xa[j], ya[j], za[j], zthickness);
		//if (cur_dist > ETH)	continue; //skip this edge is it is too V3DLONG 

		Edge e = Edge(k, j);
		edge_array.push_back( e );
			
		Weight w =	edge_weight_func(cur_dist, va[k], va[j], 255); //note that edge_weight_func is an overloaded func. Don't get confused.
		weights.push_back( w );

		//=========================================================================================
		
		e = Edge(j, k);
		edge_array.push_back( e );
		weights.push_back( w );
		
		n=n+2; // that is the correct position of n++
			
		if (w>maxw) maxw=w;	if (w<minw) minw=w;
	}
	printf(" minw=%g maxw=%g ", minw,maxw);
	printf(" graph defined!\n");
	
	if (n != edge_array.size())
	{
		printf(s_error="The number of edges is not consistent \n");
		return s_error;
	}
	V3DLONG num_edges = n; // back to undirectEdge for less memory consumption
	
	printf("select %ld out of %ld links \n", n, m);
	printf("total %ld nodes, total %ld edges \n", num_nodes, num_edges);
	printf("start from #%ld to ", start_nodeind);
	for(i=0; i<n_end_nodes; i++) printf("#%ld ", ind_end_nodes[i]); printf("\n");
	printf("---------------------------------------------------------------\n");
	
#define _do_shortest_path_algorithm_
	//========================================================================================================
	// extract key code to function ???_shortest_path()	
	
	int code_select = 0; // BGL has the best speed and correctness
	switch(code_select)
	{
		case 0:
			printf("bgl_shortest_path() \n");
			s_error = bgl_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes, start_nodeind, &plist[0]);
			break;
		case 1:
			printf("phc_shortest_path() \n");
			s_error = phc_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
			break;
		case 2:
			printf("mst_shortest_path() \n");
			s_error = phc_shortest_path(&edge_array[0], num_edges, &weights[0], num_nodes,	start_nodeind, &plist[0]);
			break;
	}
	if (s_error)
		return s_error;
	
	
	// output node coordinates of the shortest path
	mmUnit.clear();
	V3DLONG nexist = 0;
	
	V_NeuronSWC_unit cc;
	vector<V_NeuronSWC_unit> mUnit;
	
	if (n_end_nodes==0) // trace from start-->each possible node
	{
#define _output_shortest_path_TREE_
		printf("trace from start-->each possible node \n");
		mUnit.clear();
		
		std::map<double,V3DLONG> index_map;	index_map.clear();
		// set nchild=0
		for (j=0; j<num_nodes; j++)
		{
			if (j==start_nodeind)
			{
				cc.x = xa[start_nodeind];
				cc.y = ya[start_nodeind];
				cc.z = za[start_nodeind];
				cc.n = 1+j;
				cc.parent = -1;
				cc.nchild = 1; // root isn't leaf
				mUnit.push_back(cc);
				printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
			}
			else if ( (k=plist[j]) != j ) // has parent
				if (k>=0 && k<num_nodes)  // is valid
				{
					cc.x = xa[j];
					cc.y = ya[j];
					cc.z = za[j];
					cc.n = 1+j;
					cc.parent = 1+k; //k=plist[j]
					cc.nchild = 0;
					mUnit.push_back(cc);
					//printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
					
					index_map[cc.n] = mUnit.size()-1;
				}
		}
		
		printf("counting parent->nchild \n");
		// count parent->nchild
		for (j=0; j<mUnit.size(); j++)
		{
			double parent = mUnit[j].parent;
			V3DLONG i = index_map[parent]; // this is very fast
			
			mUnit[i].nchild++;
		}
		
		printf("rearranging index number \n");
		index_map.clear();
		for (j=0; j<mUnit.size(); j++)
		{
			if (mUnit[j].nchild >=0)
			{
				double ndx   = mUnit[j].n;
				V3DLONG new_ndx = index_map.size()+1;
				index_map[ndx] = new_ndx;
			}
		}
		for (j=0; j<mUnit.size(); j++)
		{
			if (mUnit[j].nchild >=0)
			{
				double ndx    = mUnit[j].n;
				double parent = mUnit[j].parent;
				mUnit[j].n = index_map[ndx];
				if (parent>=1)	mUnit[j].parent = index_map[parent];
			}
		}
		
		mmUnit.push_back(mUnit);
	}
	
	else
		for (int npath=0; npath<n_end_nodes; npath++) // n path of back tracing end-->start
		{
#define _output_shortest_path_N_
			printf("the #%d path of back tracing end-->start \n", npath+1);
			mUnit.clear();
			
			j = ind_end_nodes.at(npath);//end_nodeind[npath]; //search from the last one
			if (j<0 || j>=num_nodes) // for the end_node out of ROI
			{
				printf(" end_node is out of range, ignored but should check error.\n");
				continue;
			}
			cc.x = xa[j];
			cc.y = ya[j];
			cc.z = za[j];
			cc.n = nexist +1+mUnit.size();
			cc.parent = cc.n +1;
			printf("[end: x y z] %ld: %g %g %g ", j, cc.x, cc.y, cc.z);
			printf("\n");
			
			mUnit.push_back(cc);
			
			for (k=0;k<n;k++) //at most n edge links
			{
				V3DLONG jj = j;	j = plist[j];
				
				if (j==jj)
				{
					mUnit.clear();
					printf(s_error="Error happens: this path is broken because a node has a self-link!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
					break;
				}
				
				if (j>=num_nodes)
				{
					mUnit.clear();
					printf(s_error="Error happens: this node's parent has an index out of range!"); printf(" [j->p(j)] %ld->%ld \n", jj, j);
					break;
				}
				
				if (j<0) // should not be reached, because stop back trace at his child node
				{
					mUnit.clear();
					printf(s_error="find the negative node, which should indicate the root has been reached over."); printf(" [j->p(j)] %ld->%ld \n", jj, j);
					break;
				}
				
				if (j!=start_nodeind)
				{
					cc.x = xa[j];
					cc.y = ya[j];
					cc.z = za[j];
					cc.n = nexist +1+mUnit.size();
					cc.parent = cc.n +1;
					mUnit.push_back(cc);
					//printf("[node: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
				}
				else //j==start_nodeind
				{
					cc.x = xa[j];
					cc.y = ya[j];
					cc.z = za[j];
					cc.n = nexist +1+mUnit.size();
					cc.parent = -1;
					mUnit.push_back(cc);
					printf("[start: x y z] %ld: %g %g %g \n", j, cc.x, cc.y, cc.z);
					
					break; //STOP back tracing
				}
			}
			nexist += mUnit.size();
			
			if (mUnit.size()>=2)	mmUnit.push_back(mUnit);
		}
	
	if (mmUnit.size())	return 0;
	return s_error;
}


char* bgl_shortest_path(Edge *edge_array, V3DLONG n_edges, Weight *weights, V3DLONG n_nodes, //input graph
		Node start_nodeind, //input source
		Node *plist) //output path
{
	char* s_error=0;

	//typedef adjacency_list < vecS, vecS, directedS, no_property, property < edge_weight_t, Weight > > graph_t;
	typedef adjacency_list < vecS, vecS, undirectedS, no_property, property < edge_weight_t, Weight > > graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
	typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;

    printf("n_nodes=%ld start_node=%ld \n", n_nodes, start_nodeind);

    //graph_t g(edge_array, edge_array + num_edges, weights, num_nodes);
    graph_t g(n_nodes);

    //printf("num_vertices(g)=%ld  num_edges(g)=%ld \n", num_vertices(g), num_edges(g));

    property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
	for (V3DLONG j=0; j<n_edges; ++j)
	{
		edge_descriptor e; bool inserted;
		tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);		weightmap[e] = weights[j];
	}

    printf("num_vertices(g)=%ld  num_edges(g)=%ld \n", num_vertices(g), num_edges(g));
    if (n_nodes != num_vertices(g))
    {
    	printf(s_error="ERROR to create graph: n_nodes != num_vertices(g) \n");
    	return s_error;
    }
//	//for debugging purpose
//	{
//		property_map<graph_t, edge_weight_t>::type   weightmap = get(edge_weight, g);
//	    std::cout << " vertices(g) = ";
//		vertex_iterator vi, vend;
//	    for (tie(vi, vend) = vertices(g); vi != vend; ++vi)
//			;//std::cout << index[*vi] <<  " ";
//	    std::cout <<"total "<<i<< std::endl;
//	    std::cout << " edges(g) = ";
//	    edge_iterator ei, ei_end;
//	    for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
//			;//std::cout << "(" << index[source(*ei, g)] << "," << index[target(*ei, g)] << ":" << weightmap[*ei] <<") ";
//	    std::cout <<"total "<<i<< std::endl;
//	}

	vertex_descriptor s = vertex(start_nodeind, g);
	std::vector<vertex_descriptor> p(num_vertices(g));
	std::vector<Weight> d(num_vertices(g));
	dijkstra_shortest_paths(g, s, predecessor_map(&p[0]).distance_map(&d[0]));


//	std::cout<<"===================================================================\n";
//	std::cout << "distances and parents:" << std::endl;
	graph_traits < graph_t >::vertex_iterator vi, vend;
	V3DLONG i=0;
	for (tie(vi, vend) = vertices(g), i=0;
		i<n_nodes && vi != vend;
		++vi, i++)
	{
		//std::cout << "distance(" << (*vi) << ") = " << d[*vi] << ", ";
		//std::cout << "parent(" << (*vi) << ") = " << (p[*vi]) << std::endl;
		plist[*vi] = p[*vi];
	}

	return s_error;
}


//090512 PHC: add function phc_shortest_path()
char* phc_shortest_path(Edge *edge_array, V3DLONG n_edges, Weight *weights, V3DLONG n_nodes, //input graph
					   Node start_nodeind, //input source
					   Node *plist) //output path
{
	char* s_error=0;

	//check data
	if (!edge_array || n_edges<=0 || !weights || n_nodes<=0 ||
		start_nodeind<0 || start_nodeind>=n_nodes ||
		!plist)
	{
		printf(s_error="Invalid parameters to phc_shortest_path(). do nothing\n");
		return s_error;
	}

	//copy data

	DijkstraClass * p = new DijkstraClass;
	if (!p)
    {
		printf(s_error="Fail to allocate memory for DijkstraClass().\n");
		return s_error;
	}
	p->nnode = n_nodes;
	p->allocatememory(p->nnode);
	if (!(p->adjMatrix))
	{
		printf(s_error="Fail to assign value to the internal edge matrix.\n");
		if (p) {delete p; p=0;}
		return s_error;
	}

	char* copyEdgeSparseData2Adj(Edge *edge_array, V3DLONG n_edges, Weight *weights, V3DLONG n_nodes,
							  vector <connectionVal> * adjMatrix,
							  float &minn,  float &maxx);

	float minlevel,maxlevel;
	s_error = copyEdgeSparseData2Adj(edge_array, n_edges, weights, n_nodes,
						  p->adjMatrix,
						  minlevel, maxlevel);
	if (s_error)
	{
		printf(s_error="Fail to assign value to the internal edge matrix.\n");
		if (p) {delete p; p=0;}
		return s_error;
	}

	p->dosearch(start_nodeind); //set root as the first node

	//copy the output plist
	for (V3DLONG i=0;i<n_nodes;i++)
	{
		plist[i] = p->nodeParent[i]-1; //-1 because I used the Matlab convention in dijk core function
	}

	//free memory and return

	if (p) {delete p; p=0;}
	return s_error;
}



//================    Function Start ================//
char * mst_shortest_path(Edge * edge_array, V3DLONG n_edges, Weight * weights, V3DLONG n_nodes, //input graph
						 Node start_nodeind, // input source
						 Node * plist)
{
	using namespace boost;
	
	char * s_error = 0;
	typedef adjacency_list < vecS, vecS, undirectedS, no_property, property < edge_weight_t, Weight> > graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
	typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;
	
	printf("n_nodes=%ld start_node=%ld \n", n_nodes, start_nodeind);
	
	graph_t g(edge_array, edge_array + n_edges, weights, n_nodes);
	//graph_t(n_nodes);
	
	//printf("num_vertices(g)=%ld  num_edges(g)=%ld \n", num_vertices(g), num_edges(g));
	
	property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
	
	std::vector < edge_descriptor > spanning_tree;
	kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));
	
#ifdef __MST_SHORTEST_PATH_DEBUG__
	std::cout<<"Print the edges in the MST:" << std::endl;
#endif
	std::vector< std::vector< Node > > adjacency_table(n_nodes, std::vector<Node>());
	for(V3DLONG i = 0; i < spanning_tree.size(); i++)
	{
#ifdef __MST_SHORTEST_PATH_DEBUG__
		std::cout<<source(edge_array[i], g) << " <--> " << target(edge_array[i], g) << "with weight of "<< weights[i] << std::endl;
#endif
		Node a = source(edge_array[i], g);
		Node b = target(edge_array[i],g);
		adjacency_table[a].push_back(b);
		adjacency_table[b].push_back(a);
	}
	
	Node top = -1;
	std::stack<Node> node_stack;
	node_stack.push(start_nodeind);
	plist[start_nodeind] = -1;
	while(!node_stack.empty())
	{
		top = node_stack.top();
		node_stack.pop();
		std::vector<Node>::iterator it = adjacency_table[top].begin();
		while(it != adjacency_table[top].end())
		{
			if(*it != plist[top]) 
			{
				plist[*it] = top;
				node_stack.push(*it);
			}
			it++;
		}
	}
	
	return s_error;
}


char* copyEdgeSparseData2Adj(Edge *edge_array, V3DLONG n_edges, Weight *weights, V3DLONG n_nodes,
						  vector <connectionVal> * adjMatrix,
						  float &minn,
						  float &maxx)
{
	char* s_error=0;

	if (!edge_array || n_edges<=0 || !weights || n_nodes<=0 || !adjMatrix)
		{printf(s_error="the adjMatrix pointer or nnodes of copyEdgeSparseData2Adj() is invalid. Do nothing.\n");return s_error;}

	for (V3DLONG i=0; i<n_edges; i++)
	{
		if (edge_array[i].first<0 || edge_array[i].first>=n_nodes || edge_array[i].second<0 || edge_array[i].second>=n_nodes)
		{
			printf("illegal edge parent and child node info found in copyEdgeSparseData2Adj(). [%ld %ld w=%5.3f] n_nodes=%ld. do nothing\n",
					edge_array[i].first, edge_array[i].second, weights[i], n_nodes);
			return s_error;
		}

		connectionVal tmpVal;

		#define ADD_EDGE(p, c, w) { tmpVal.pNode=p; tmpVal.cNode=c; tmpVal.aVal=w; adjMatrix[tmpVal.pNode].push_back(tmpVal); }

		ADD_EDGE( edge_array[i].first, edge_array[i].second, weights[i]);
		ADD_EDGE( edge_array[i].second, edge_array[i].first, weights[i]);

		if (i==0)
		{
			maxx = minn = weights[i];
		}
		else
		{
			if (weights[i]>maxx) maxx=weights[i];
			else if (weights[i]<minn) minn=weights[i];
		}
	}
	return s_error;
}


//template <class T> bool compute_sphere_win3d_pca(T ***img3d, V3DLONG sx, V3DLONG sy, V3DLONG sz,
//										  V3DLONG x0, V3DLONG y0, V3DLONG z0,
//										  V3DLONG rx, V3DLONG ry, V3DLONG rz,
//										  double &pc1, double &pc2, double &pc3, bool b_disp_CoM_etc=true) //b_disp_CoM_etc is the display option for center of mass )


double fitRadiusPCA(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double bound_r,
					float x, float y, float z, float zthickness)
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fitRadiusPCA() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset

	double pc1, pc2, pc3;
	compute_sphere_win3d_pca(img3d, dim0, dim1, dim2,  x, y, z,  bound_r,bound_r,bound_r,
			pc1, pc2, pc3,
			false);

	double r = sqrt(sqrt(pc3)*sqrt(pc2));
	if (r==0) r = 0.5;
	return r;
}

double fitRadiusPercent(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double bound_r,
							float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly)
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fitRadiusPercent() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset

	double max_r = dim0/2;
	if (max_r > dim1/2) max_r = dim1/2;
	if (!b_est_in_xyplaneonly)
	{
		if (max_r > (dim2*zthickness)/2) max_r = (dim2*zthickness)/2;
	}
	//max_r = bound_r; //unused as of now (comment added by PHC, 2010-Dec-21)

	double total_num, background_num;
	double ir;
	for (ir=1; ir<=max_r; ir++)
	{
		total_num = background_num = 0;

		double dz, dy, dx;
		double zlower = -ir/zthickness, zupper = +ir/zthickness;
		if (b_est_in_xyplaneonly)
			zlower = zupper = 0;
		for (dz= zlower; dz <= zupper; ++dz)
			for (dy= -ir; dy <= +ir; ++dy)
				for (dx= -ir; dx <= +ir; ++dx)
				{
					total_num++;

					double r = sqrt(dx*dx + dy*dy + dz*dz);
					if (r>ir-1 && r<=ir)
					{
						V3DLONG i = x+dx;	if (i<0 || i>=dim0) goto end;
						V3DLONG j = y+dy;	if (j<0 || j>=dim1) goto end;
						V3DLONG k = z+dz;	if (k<0 || k>=dim2) goto end;

						if (img3d[k][j][i] <= imgTH)
						{
							background_num++;

							if ((background_num/total_num) > 0.001)	goto end; //change 0.01 to 0.001 on 100104
						}
					}
				}
	}
end:
	return ir;
}

double fitRadiusPercent(unsigned char ****img4d, V3DLONG sz[4], double imgTH, double bound_r,
						float x, float y, float z, float zthickness, bool b_est_in_xyplaneonly)
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fitRadiusPercent() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset
	
	V3DLONG dim0 = sz[0], dim1 = sz[1], dim2 = sz[2];
	double max_r = dim0/2;
	if (max_r > dim1/2) max_r = dim1/2;
	if (!b_est_in_xyplaneonly)
	{
		if (max_r > (dim2*zthickness)/2) max_r = (dim2*zthickness)/2;
	}
	//max_r = bound_r; //unused as of now (comment added by PHC, 2010-Dec-21)
	
	VPoint v0(sz[3],0), v1(sz[3],0);
	V3DLONG c;
	for (c=0;c<sz[3];c++)
		v0.v[c] = img4d[c][(V3DLONG)round(z)][(V3DLONG)round(y)][(V3DLONG)round(x)];
	
	double total_num, background_num;
	double ir;
	for (ir=1; ir<=max_r; ir++)
	{
		total_num = background_num = 0;
		
		double dz, dy, dx;
		double zlower = -ir/zthickness, zupper = +ir/zthickness;
		if (b_est_in_xyplaneonly)
			zlower = zupper = 0;
		for (dz= zlower; dz <= zupper; ++dz)
			for (dy= -ir; dy <= +ir; ++dy)
				for (dx= -ir; dx <= +ir; ++dx)
				{
					total_num++;
					
					double r = sqrt(dx*dx + dy*dy + dz*dz);
					if (r>ir-1 && r<=ir)
					{
						V3DLONG i = x+dx;	if (i<0 || i>=dim0) goto end;
						V3DLONG j = y+dy;	if (j<0 || j>=dim1) goto end;
						V3DLONG k = z+dz;	if (k<0 || k>=dim2) goto end;
						
						for (c=0;c<sz[3];c++)
							v1.v[c] = img4d[c][k][j][i];
						
						if (v1.abs() <= imgTH ||
							cosAngleBetween(&v0, &v1) < 0.866
							)
						{
							background_num++;
							
							if ((background_num/total_num) > 0.001)	goto end; //change 0.01 to 0.001 on 100104
						}
					}
				}
	}
end:
	return ir;
}


void fitPosition(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2, double imgTH, double ir,
							float &x, float &y, float &z,  float* D, float zthickness) // 090602: add tangent D to remove movement of tangent direction
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fitPosition() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset
	
	double s, cx,cy,cz;
	s = cx = cy = cz = 0;

	double r2=double(ir)*ir;
	for (double dz= -ir/zthickness; dz <= +ir/zthickness; ++dz)
	{
		double rtmpz = dz*dz;
		for (double dy= -ir; dy <= +ir; ++dy)
		{
			double rtmpy = rtmpz+dy*dy;
			if (rtmpy>r2) 
				continue;
				
			for (double dx= -ir; dx <= +ir; ++dx)
			{
				double rtmpx = rtmpy+dx*dx;
				if (rtmpx>r2)
					continue;
					
				double r = sqrt(rtmpx);
				if (r<=ir)
				{
					V3DLONG i = x+dx;	if (i<0 || i>=dim0) continue;
					V3DLONG j = y+dy;	if (j<0 || j>=dim1) continue;
					V3DLONG k = z+dz;	if (k<0 || k>=dim2) continue;
					double f = (img3d[k][j][i]);

					if (f > imgTH)
					{
						s += f;
						cx += f*(x+dx);
						cy += f*(y+dy);
						cz += f*(z+dz);
					}
				}
			}
		}
	}
	if (s>0)
	{
		cx = cx/s;
		cy = cy/s;
		cz = cz/s;

		if (D)
		{
			// make unit vector
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
				double proj = cx*D[0] + cy*D[1] + cz*D[2];
				// remove movement of tangent direction
				cx = cx - proj*D[0];
				cy = cy - proj*D[1];
				cz = cz - proj*D[2];
				x += cx;
				y += cy;
				z += cz;
			}
		}
		else //by PHC, 2010-12-29
		{
			x = cx;
			y = cy;
			z = cz;
		}
	}
	return;
}

//#define fitRadius fitRadiusPCA
#define fitRadius fitRadiusPercent
//////////////////////////////////////////////////
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
//////////////////////////////////////////////////

#define ITER_POSITION 10
bool fit_radius_and_position(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
							vector <V_NeuronSWC_unit> & mCoord, bool b_move_position, float zthickness, bool b_est_in_xyplaneonly, double vis_threshold)
//template <class T>
//bool fit_radius_and_position(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
//							vector <T> & mCoord, bool b_move_position)
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fit_radius_and_position() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset

	if (mCoord.size()<2)
		return false;

	double AR = 0;
	for (int i=0; i<mCoord.size()-1; i++)
	{
		float x = mCoord[i].x;
		float y = mCoord[i].y;
		float z = mCoord[i].z;
		float x1 = mCoord[i+1].x;
		float y1 = mCoord[i+1].y;
		float z1 = mCoord[i+1].z;
		AR += sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1) + (z-z1)*(z-z1));
	}
	AR /= mCoord.size()-1; // average distance between nodes

	double imgAve = getImageAveValue(img3d, dim0, dim1, dim2);
	double imgStd = getImageStdValue(img3d, dim0, dim1, dim2);
	double imgTH = imgAve + imgStd; //change to VISIBLE_THRESHOLD 2011-01-21 but the result is not good
    //if (imgTH < vis_threshold) imgTH = vis_threshold; //added by PHC 20121016. consistent with APP2 improvement

	for (int i=0; i<mCoord.size(); i++)
	{
		float x = mCoord[i].x;
		float y = mCoord[i].y;
		float z = mCoord[i].z;

		double r;
		if (i==0 || i==mCoord.size()-1) // don't move start && end point
		{
			r = fitRadius(img3d, dim0, dim1, dim2, imgTH, AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
		}
		else
		{
			if (! b_move_position)
			{
				r = fitRadius(img3d, dim0, dim1, dim2, imgTH, AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
			}
			else
			{
				float axdir[3];
				DIFF(axdir[0], mCoord, i, x, 5);
				DIFF(axdir[1], mCoord, i, y, 5);
				DIFF(axdir[2], mCoord, i, z, 5);

				r = AR;
				for (int j=0; j<ITER_POSITION; j++)
				{
					fitPosition(img3d, dim0, dim1, dim2,   0,   r*2, x, y, z,  axdir, zthickness);
					r = fitRadius(img3d, dim0, dim1, dim2, imgTH,  AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
				}
			}
		}

		mCoord[i].r = r;
		mCoord[i].x = x;
		mCoord[i].y = y;
		mCoord[i].z = z;
	}
	return true;
}


bool fit_radius_and_position(unsigned char ****img4d, V3DLONG sz[4],
							 vector <V_NeuronSWC_unit> & mCoord, bool b_move_position, float zthickness, bool b_est_in_xyplaneonly, double vis_threshold)
//template <class T>
//bool fit_radius_and_position(unsigned char ***img3d, V3DLONG dim0, V3DLONG dim1, V3DLONG dim2,
//							vector <T> & mCoord, bool b_move_position)
{
	if (zthickness<=0) { zthickness=1.0; printf("Your zthickness value in fit_radius_and_position() is invalid. disable it (i.e. reset it to 1) in computation.\n"); }//if it an invalid value then reset
	
	if (mCoord.size()<2)
		return false;
	
	double AR = 0;
	for (int i=0; i<mCoord.size()-1; i++)
	{
		float x = mCoord[i].x;
		float y = mCoord[i].y;
		float z = mCoord[i].z;
		float x1 = mCoord[i+1].x;
		float y1 = mCoord[i+1].y;
		float z1 = mCoord[i+1].z;
		AR += sqrt((x-x1)*(x-x1) + (y-y1)*(y-y1) + (z-z1)*(z-z1));
	}
	AR /= mCoord.size()-1; // average distance between nodes
	
	V3DLONG i;
	V3DLONG *channelsToUse= new V3DLONG[sz[3]]; for (i=0;i<sz[3];i++) channelsToUse[i]=i; 

	int method=0;
	double imgAve = getImage4DAveValue(img4d, sz, channelsToUse, sz[3], method);
	double imgStd = getImage4DStdValue(img4d, sz, channelsToUse, sz[3]);
	double imgTH = imgAve + imgStd; //change to VISIBLE_THRESHOLD 2011-01-21 but the result is not good
    if (imgTH<vis_threshold) imgTH = vis_threshold; //added by PHC 20121016. consistent with APP2 improvement
	
	for (i=0; i<mCoord.size(); i++)
	{
		float x = mCoord[i].x;
		float y = mCoord[i].y;
		float z = mCoord[i].z;
		
		double r=0;
		if (i==0 || i==mCoord.size()-1) // don't move start && end point
		{
			r = fitRadius(img4d, sz, imgTH, AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
		}
		else
		{
			if (! b_move_position)
			{
				r = fitRadius(img4d, sz, imgTH, AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
			}
//			else
//			{
//				float axdir[3];
//				DIFF(axdir[0], mCoord, i, x, 5);
//				DIFF(axdir[1], mCoord, i, y, 5);
//				DIFF(axdir[2], mCoord, i, z, 5);
//				
//				r = AR;
//				for (int j=0; j<ITER_POSITION; j++)
//				{
//					fitPosition(img4d, sz,  0,   r*2, x, y, z,  axdir, zthickness);
//					r = fitRadius(img4d, sz, imgTH,  AR*2, x, y, z, zthickness, b_est_in_xyplaneonly);
//				}
//			}
		}
		
		mCoord[i].r = r;
		mCoord[i].x = x;
		mCoord[i].y = y;
		mCoord[i].z = z;
	}
	
	if (channelsToUse) {delete []channelsToUse; channelsToUse=0;}
	
	return true;
}


// assume root node at tail of vector (result of back tracing)
char* merge_back_traced_paths(vector< vector<V_NeuronSWC_unit> >& mmUnit)
{
	printf("merge_back_traced_paths \n");
	char* s_error=0;
	int npath = mmUnit.size();
	if (npath <2) return s_error; // no need to merge

	vector< vector<V_NeuronSWC_unit> > all_segment;
	vector<V_NeuronSWC_unit> same_segment;
	same_segment.clear();

	vector<V3DLONG> path_start(npath); // 0-based index
	vector<V3DLONG> path_try(npath);   // 0-based index
	for (int i=0; i<npath; i++)		path_try[i] = path_start[i] = mmUnit[i].size()-1;

	V3DLONG nexist = 0; // count output node index
	int lastn_same_node;

	int * flag_skipped = new int [npath];; //added by PHC, 2010-05-22. // 0--searching path, 1--connected branch, 2--separated branch, 3--over searched
	for (;;) //iteration for all segment
	{
		for (int i=0; i<npath; i++)		flag_skipped[i] = (path_start[i] <1)? 3:0;

		printf("all_segment.size=%d >> \n", all_segment.size());
//		printf("path_start:      "); 	for (int i=0; i<npath; i++)		printf("%d(%ld) ", i+1, path_start[i]);	printf("\n");
//		printf("path_try:        "); 	for (int i=0; i<npath; i++)		printf("%d(%ld) ", i+1, path_try[i]);	printf("\n");
//		printf("flag_skipped:    "); 	for (int i=0; i<npath; i++)		printf("%d(%ld) ", i+1, flag_skipped[i]);	printf("\n");

		/////////////////////////////////////////////////////////////////////////
		bool all_skipped = true;
		for (int i=0; i<npath; i++)		all_skipped = (all_skipped && flag_skipped[i]);

		if (all_skipped)
			break; /// STOP iteration when every path is over searched
		/////////////////////////////////////////////////////////////////////////

		V3DLONG jj;
		V_NeuronSWC_unit same_node;
		V3DLONG n_same_node;
		V3DLONG ipath = -1;
		lastn_same_node = 0;

		for (V3DLONG j=0; true; j++) //searching same_segment along 1 path with other paths
		{
			n_same_node = 0;

			for (int i=0; i<npath; i++) //find a same node with other paths
			{
				if (flag_skipped[i]) continue; // skip the path because of skipped branch/or over searched

				jj = path_try[i]-1; // ##################
				if (jj <0) // this path is over searched=======================
				{
					flag_skipped[i] = 3;
					if (lastn_same_node==1)		break; 		// no other connected path
					else						continue;	// try other connected path
				}
				V_NeuronSWC_unit & node = mmUnit[i][jj];

				if (n_same_node==0) // start a new segment by picking a node as a template ######################
				{
					same_node = node;
					n_same_node = 1;
					ipath = i;
					if (lastn_same_node==1)		break; 		// no other connected path
					else						continue;	// compare with other connected path
				}
				else //(n_same_node >0)
				{
					if (same_node.x == node.x && same_node.y == node.y && same_node.z == node.z)
					{
						n_same_node ++;
					}
					else // this path is a branch================================
					{
						path_start[i] = path_try[i]; // ###################
						switch (lastn_same_node)
						{
						case 0: // start a new iteration
							flag_skipped[i] = 2; // separated branch
							break;
						case 1: // only 1 connected branch, here is impossible
							flag_skipped[i] = 3; // over searched
							break;
						default: //(lastn_same_node >1)
							flag_skipped[i] = 1; // have other connected segment
							break;
						}
						continue; 				// try other connected path
					}
				}
			}//i -- compared the same_node with each start-node of other paths
			if (lastn_same_node==0) // initialize lastn_same_node when start a new iteration #####################
			{
				lastn_same_node = n_same_node;
			}
			//printf("   [%ld: ipath lastn_same_node n_same_node]         %d %d %d \n", j, ipath+1, lastn_same_node, n_same_node);
			//printf("	flag_skipped:    "); 	for (int i=0; i<npath; i++)		printf("%d(%ld) ", i+1, flag_skipped[i]);	printf("\n");
			//printf("	path_start:      "); 	for (int i=0; i<npath; i++)		printf("%d(%ld) ", i+1, path_start[i]);	printf("\n");
			//printf("	path_try:        "); 	for (int i=0; i<npath; i++)		printf("%d(%ld) ", i+1, path_try[i]);	printf("\n");


			if (n_same_node >0 && n_same_node ==lastn_same_node) // in a same segment to merge
			{
				//printf("add a node \n");
				for (int i=0; i<npath; i++)
				{
					switch (flag_skipped[i])
					{
					case 0: // searching path
						path_try[i] --; // ##################
						break;
					}
				}
			}
			else if (n_same_node <lastn_same_node ) // save the merged segment to output buffer
			{
				//printf("end of a segment \n");
				// push nodes from path_start to path_try
				same_segment.clear();
				if (ipath > -1)
				{
					printf("add a valid segment form path %d(%ld -- %ld)\n", ipath+1, path_start[ipath], path_try[ipath]);
					for (jj = path_start[ipath]; jj >= path_try[ipath]; jj--)
					{
						same_node = mmUnit[ipath][jj];
						nexist ++;
						same_node.n = nexist;
						same_node.parent = nexist+1;
						same_segment.push_back(same_node);
					}
					same_segment[same_segment.size()-1].parent = -1; //====================make segment have a root node
					all_segment.push_back(same_segment);
				}
				// adjust start node index of branch
				for (int i=0; i<npath; i++)
				{
					switch (flag_skipped[i])
					{
					case 0: // searching path
					case 3: // over searched
						path_start[i] = path_try[i]; // ################
						break;
					}
				}

				///////////////////////////////////////////////////////////////////////////
				if (n_same_node <1) // no need to continue merging node
					break; // STOP at end of 1 branch
				///////////////////////////////////////////////////////////////////////////
			}
			else
			{
				mmUnit = all_segment;
				printf(s_error="Error happens: n_same_node > lastn_same_node in merge_back_traced_paths() \n");
				return s_error;
			}

			lastn_same_node = n_same_node;
		
		}//j -- along 1 branch
	}//all branch & path

	if (flag_skipped) {delete []flag_skipped; flag_skipped=0;} //added by PHC, 2010-05-22
	mmUnit = all_segment;
	return s_error;
}


void rearrange_and_remove_labeled_deletion_nodes_mmUnit(vector< vector<V_NeuronSWC_unit> >& mmUnit) //by PHC, 2011-01-15
{
	printf("....... rearranging index number \n");
	std::map<double,V3DLONG> index_map;	
	
	V3DLONG i,j,k;
	
	for (k=0; k<mmUnit.size(); k++)
	{
		printf("....... removing the [%ld] segment children that have been labeled to delete (i.e. nchild < 0).\n", k);
		vector<V_NeuronSWC_unit> & mUnit = mmUnit.at(k);
		index_map.clear();
		
		for (j=0; j<mUnit.size(); j++)
		{
			if (mUnit[j].nchild >=0)
			{
				double ndx   = mUnit[j].n;
				V3DLONG new_ndx = index_map.size()+1;
				index_map[ndx] = new_ndx;
			}
		}
		for (j=0; j<mUnit.size(); j++)
		{
			if (mUnit[j].nchild >=0)
			{
				double ndx    = mUnit[j].n;
				double parent = mUnit[j].parent;
				mUnit[j].n = index_map[ndx];
				if (parent>=0)	mUnit[j].parent = index_map[parent];
			}
		}
		
		vector<V_NeuronSWC_unit> mUnit_new; 
		V3DLONG root_id=-1;
		for (j=0; j<mUnit.size(); j++)
		{
			if (mUnit[j].nchild >= 0)
			{
				mUnit_new.push_back(mUnit[j]);
			}
			
			if (mUnit[j].parent<0)
			{
				if (root_id!=-1)
					printf("== [segment %ld] ================== detect a non-unique root!\n", k);
				root_id = V3DLONG(mUnit[j].n);
				printf("== [segment %ld] ================== nchild of root [%ld, id=%ld] = %ld\n", k, j, V3DLONG(mUnit[j].n), V3DLONG(mUnit[j].nchild));
			}
			
		}
		
		mmUnit[k] = mUnit_new;
	}
}



