

#ifndef __LANDMARKER_DEAL_H__
#define __LANDMARKER_DEAL_H__

#include "until.h"

#include "mBrainAligner.h"
using namespace std;

#define WANT_STREAM

bool auto_warp_marker_sp(float & lam, vector<point3D64F> &ccf_all_marker, vector<point3D64F> &fmost_all_marker, vector<point3D64F>&sub_marker, bool outline);

bool auto_warp_marker(float & lam, vector<point3D64F> &ccf_all_marker, vector<point3D64F> &fmost_all_marker, vector<point3D64F>&sub_marker);

bool landmark_region(vector<point3D64F>& vec_corners, vector<point3D64F>& fine_sub_corner, vector<point3D64F> & aver_corner, float **** & p_img_label,
	vector<int> &label, long long *sz_img_tar);

bool load_fine_marker(QString fine_filename_landmark_sub, vector<point3D64F> &vec_corners, vector<point3D64F> &fine_sub_corner, vector<point3D64F> &aver_corner, double &star_iter);

bool find_nearst_landmarks(vector< vector<neighborinfo> > &vec2d_neighborinfo, vector<point3D64F>& vec_corners, int &nneighbors);

bool update_average_landmarker(vector<point3D64F> vec_corners, vector<point3D64F> fine_sub_corner, vector<point3D64F> & aver_corner);

bool load_fine_marker(QString fine_filename, vector<point3D64F> &vec_corners, vector<point3D64F> &fine_sub_corner, vector<point3D64F> &aver_corner, Parameter &input_Parameter);

bool cmp(int x, int y); ///cmp函数传参的类型不是vector<int>型，是vector中元素类型,即int型

bool compare_neighborinfo(const neighborinfo& first, const neighborinfo& second);

bool compare_outline(const point3D64F& first, const point3D64F& second);

bool compare_label(const point3D64F& first, const point3D64F& second);

bool auto_warp_average_marker(vector<double> label, vector<point3D64F> &ccf_all_marker, vector<point3D64F> &fmost_all_marker, vector<point3D64F>&sub_marker);

#endif
