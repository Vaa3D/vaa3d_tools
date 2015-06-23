/* Opencv_example_func.h
 * This plugin will load image and swc in domenu and dofunc
 * 2012-02-21 : by Hang Xiao
 */

#ifndef __Opencv_example_FUNC_H__
#define __Opencv_example_FUNC_H__

#include <v3d_interface.h>


#include <v3d_interface.h>
#include "v3d_message.h"





#include "basic_surf_objs.h"
#include "stackutil.h"


#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/ml.hpp"
#include "opencv2/highgui.hpp"

#include "vn_imgpreprocess.h"
#include "fastmarching_dt.h"
#include "fastmarching_tree.h"
//#include "hierarchy_prune.h"
#include "marker_radius.h"
#include "basic_surf_objs.h"

#include "smooth_curve.h"


#include <iostream>
#include <stdio.h>

#include <fstream>

#include <string>

#include <algorithm>

#include <numeric>

#include <cmath>

#include <set>

#include <list>

#include <limits> // for numeric_limits

#include <utility> // for pair

#include <iterator>

#include <vector>

#include <time.h>



typedef int vertex_t;
typedef double weight_t;

const weight_t max_weight = std::numeric_limits<double>::infinity();


struct neighbor {
    vertex_t target;
    weight_t weight;
    neighbor(vertex_t arg_target, weight_t arg_weight)
        : target(arg_target), weight(arg_weight) { }
};

struct HierarchySegment
{
	HierarchySegment * parent;
	MyMarker * leaf_marker;
	MyMarker * root_marker;      // its parent marker is in current segment's parent segment
	double length;               // the length from leaf to root
	int level;                   // the segments number from leaf to root

	HierarchySegment()
	{
		leaf_marker = 0;
		root_marker = 0;
		length = 0;
		level = 1;
		parent = 0;
	}
	HierarchySegment(MyMarker * _leaf, MyMarker * _root, double _len, int _level)
	{
		leaf_marker = _leaf;
		root_marker = _root;
		length = _len;
		level = _level;
		parent = 0;
	}

	void get_markers(vector<MyMarker*> & outswc)
	{
		if(!leaf_marker || !root_marker) return;
		MyMarker * p = leaf_marker;
		while(p != root_marker)
		{
			outswc.push_back(p);
			p = p->parent;
		}
		outswc.push_back(root_marker);
	}
};



struct trip_idx
{
    int x;
    int y;
    int z;
    trip_idx(int arg_x, int arg_y, int arg_z)
        : x(arg_x), y(arg_y), z(arg_z) { }
};


typedef std::vector<std::vector<neighbor> > adjacency_list_t;


void DijkstraComputePaths(vertex_t source,
						  vertex_t target,
                          const adjacency_list_t &adjacency_list,
                          std::vector<weight_t> &min_distance,
                          std::vector<vertex_t> &previous);


std::list<vertex_t> DijkstraGetShortestPathTo(
    vertex_t vertex, const std::vector<vertex_t> &previous);

QString getAppPath();

string file_type1(string para);

bool Batch_Process(V3DPluginCallback2 & callback, const V3DPluginArgList & input, V3DPluginArgList & output);

int multiscaleEhance(V3DPluginCallback2 & callback, char * infile, cv::Mat & conf_img);

int loadMat(cv::Mat & image, char * fileName_string);

int saveMat(cv::Mat image, char * fileName_string);

int saveImg(float *img_ptr, int img_sz[], char * fileName_string);

int find_t2_fragments(cv::Mat image, cv::Mat &seg_img, int t2, float dt);

int merge_base(cv::Mat image, cv::Mat &seg_img, vector<int> & comp_list);

int merge_base1(cv::Mat image, cv::Mat &seg_img, vector<int> & comp_list, vector<trip_idx> & base_idx, int large_flag);

int merge_base_img(cv::Mat image, cv::Mat &seg_img, vector<int> & comp_list);

int count_merged_base(cv::Mat seg_img,cv::Mat label_img, int * base_merge_list);

int retrieve_low_confidence(cv::Mat input_img, cv::Mat image, cv::Mat & seg_img, char * dataset);

int collect_hog1(cv::Mat label_img, cv::Mat & hist1, int * base_list,
vector<vector<int> > PixelIdxListx,vector<vector<int> > PixelIdxListy,vector<vector<int> > PixelIdxListz);


int check_connect(cv::Mat &seg_img);

int grow_base(cv::Mat label_img, cv::Mat &seg_img, int source, int *target_tip, float min_dist);

int grow_base1(cv::Mat label_img, cv::Mat &seg_img, int source, int *target_tip, float min_dist);

int grow_base_img(cv::Mat image, cv::Mat label_img, cv::Mat &seg_img, int source, int *target_tip, float min_dist1);

float bwdist_img1(cv::Mat image, cv::Mat label_image, float * &dist_img, float * dist_img1, int source, int target, int target_tip[], float md);

int grow_base_adv(cv::Mat label_img, cv::Mat &seg_img, int source, int *target_tip);

int expand_tip(cv::Mat &seg_img,int target_tip[]);

int collect_idx_c(cv::Mat seg_img, vector<trip_idx> &Pseg, uchar tc);

int collect_idx_int(cv::Mat seg_img, vector<trip_idx> &Pseg, int tc);

int LCM_boost(cv::Mat input_img, cv::Mat image, cv::Mat & seg_img, int t1, int t2,char * dataset);

int bwconnmp_img(cv::Mat input_img, cv::Mat &label_img);

int get_main_branch(cv::Mat image, cv::Mat &seg_img, int t1, int & base_t);

int isconnect(cv::Mat label_img, int nt, int *tx,int *ty,int *tz, int source);

int max_component(cv:: Mat label_img);

//int bwconnmp_img1(int * input_img, cv::Mat &label_img, int img_sz[]);

int bwconnmp_img2(cv::Mat input_img, int * &c_label, uchar sv);

int bwconnmp_img3(cv::Mat input_img, cv::Mat &label_img, uchar sv);

int bwconnmp_img4(cv::Mat input_img, cv::Mat &label_img, uchar sv);

int input_boundary(double * xv, int sz, double rng[], int bnd[], int bf);

float bwdist_img(cv::Mat image,cv::Mat label_img, float * &dist_img, int source, int target, int target_tip[], float md);


int count_labels_img(cv::Mat label_img);

int count_x_labels_img(cv::Mat label_img, int target, int * &tx, int * &ty, int * &tz);

int bwconnmp(cv::Mat label_img,std::vector<std::vector<int> > &PixelIdxListx,std::vector<std::vector<int> > &PixelIdxListy,std::vector<std::vector<int> > &PixelIdxListz);

int bwconnmp1(int * label_img,std::vector<std::vector<int> > &PixelIdxListx,std::vector<std::vector<int> > &PixelIdxListy,std::vector<std::vector<int> > &PixelIdxListz,V3DLONG sz[]);

int determine_bounding_box(int *bnd,std::vector<std::vector<int> > PixelIdxListx,std::vector<std::vector<int> > PixelIdxListy,std::vector<std::vector<int> > PixelIdxListz);

int collect_hog(cv::Mat label_img,cv::Mat & hist1,int base_thrs,std::vector<std::vector<int> > PixelIdxListx,
		std::vector<std::vector<int> > PixelIdxListy,std::vector<std::vector<int> > PixelIdxListz);


int force_merge(cv::Mat image, cv::Mat &seg_image, vector<int> base_list,vector<trip_idx> base_idx);


//int collect_hog1(int * label_img,cv::Mat & hist1,int base_thrs,std::vector<std::vector<int> > PixelIdxListx,
	//make
		//std::vector<std::vector<int> > PixelIdxListy,std::vector<std::vector<int> > PixelIdxListz);

int filter_small_comp(cv::Mat &seg_img, int thres);

int count_n_labels_img(cv::Mat label_img, int * &np );


int imdilate_v(int v3[], int * dil_v,int img_sz[],int sz[]);

int local_ftrs(cv::Mat image,cv::Mat & centre_cc1,cv::Mat & cub1,std::vector<std::vector<int> >  PixelIdxListx,
		std::vector<std::vector<int> >  PixelIdxListy,std::vector<std::vector<int> >  PixelIdxListz);


int link_nearest(int *min_dist1, std::vector <int> frag_list, std::vector <int> base_list, cv::Mat &frag_p1,
		cv::Mat &base_p1, std::vector<std::vector<int> >  PixelIdxListx,std::vector<std::vector<int> >  PixelIdxListy,
		std::vector<std::vector<int> >  PixelIdxListz);


int link_nearest1(int *min_dist1, std::vector <int> frag_list, std::vector <int> base_list, cv::Mat &frag_p1,
		cv::Mat &base_p1, std::vector<std::vector<int> >  PixelIdxListx,std::vector<std::vector<int> >  PixelIdxListy,
		std::vector<std::vector<int> >  PixelIdxListz);


int curvature_link(double *curvature_l, int *bnd, std::vector <int> frag_list,cv::Mat frag_p, cv::Mat base_p);

int link_ftrs( int *min_dist1, double * curvature_l, int base_thrs, int *bnd, cv::Mat & frag_p1,
		cv::Mat & base_p1, std::vector<std::vector<int> >  PixelIdxListx,std::vector<std::vector<int> >  PixelIdxListy,
		std::vector<std::vector<int> >  PixelIdxListz);

int link_ftrs1( int *min_dist1, double * curvature_l, int * base_pl, int *bnd, cv::Mat & frag_p1,
		cv::Mat & base_p1, std::vector<std::vector<int> >  PixelIdxListx,std::vector<std::vector<int> >  PixelIdxListy,
		std::vector<std::vector<int> >  PixelIdxListz);


int fill_features(cv::Mat & feature_cc,std::vector<std::vector<int> > PixelIdxListx, int * bnd,int *min_dist1,
		double *curvature_l, cv::Mat hist1, cv::Mat cub1);

int rt_test(cv::Mat feature_cc, double * resp_tst);

int LCM_classify(cv::Mat feature_cc, double * resp_tst,char * dataset);


int recover_component(cv::Mat image,cv::Mat label_img, int * fila_frag, std::vector<std::vector<trip_idx> > &Pathxyz,
		std::vector<std::vector<int> > PixelIdxListx,std::vector<std::vector<int> > PixelIdxListy,
		std::vector<std::vector<int> > PixelIdxListz);






int is_breach(int v3[], int im_sz[]);

int bwdist(cv::Mat seg_img,float * &dist_img,uchar lb1, float md);

int bwdist1(cv::Mat image,float * &dist_img, int source, float md);

float bwdist2(cv::Mat image,float * &dist_img, int source, int target_tip[], float md);

/*
struct neighbor {
    vertex_t target;
    weight_t weight;
    neighbor(vertex_t arg_target, weight_t arg_weight)
        : target(arg_target), weight(arg_weight) { }
};
*/

int assign_adj_neigh(cv::Mat sub_img,cv::Mat sub_label, int v3s[], int v3t[],std::vector<std::vector<neighbor> > &adj_list);


int ReadNumbers( const std::string & s, std::vector <double> & v );

void import_matrix_from_txt_file(const char* filename_X, std::vector <double>& v, int& rows, int& cols);

int trace_img(cv::Mat seg_img, cv::Mat image, char * outfile_swc);

bool Opencv_example(V3DPluginCallback2 &callback, QWidget *parent);
bool Opencv_example(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

int file_column_count ( std::string filename );
int file_row_count ( std::string input_filename );
int i4block_components ( int l, int m, int n, int a[], int c[] );
int i4_min ( int i1, int i2 );
int i4mat_components ( int m, int n, int a[], int c[] );
int *i4mat_data_read ( std::string input_filename, int m, int n );
void i4mat_header_read ( std::string input_filename, int *m, int *n );
int i4vec_components ( int n, int a[], int c[] );
int s_len_trim ( std::string s );
int s_to_i4 ( std::string s, int *last, bool *error );
bool s_to_i4vec ( std::string s, int n, int ivec[] );
int s_word_count ( std::string s );
void timestamp ( );

int fastMarch( char * fileName_string,cv::Mat & image);

bool topo_segs2swc1(std::vector<HierarchySegment*> & topo_segs, std::vector<MyMarker*> & outmarkers, int swc_type);

template<class T> bool swc2topo_segs(std::vector<MyMarker*> & inswc, std::vector<HierarchySegment*> & topo_segs,
 int length_method = 0, T * inimg1d = 0, long sz0 = 0, long sz1 = 0, long sz2 = 0);


template<class T> bool hierarchy_prune(std::vector<MyMarker*> &inswc, std::vector<MyMarker*> & outswc, T * inimg1d,
 long sz0, long sz1, long sz2, double length_thresh = 10.0);


template<class T> bool happ(std::vector<MyMarker*> &inswc, std::vector<MyMarker*> & outswc, T * inimg1d, long sz0, long sz1, long sz2,
double bkg_thresh = 10.0, double length_thresh = 2.0, double SR_ratio = 1.0/9.0,
 bool is_leaf_prune = true, bool is_smooth = true);


#endif

