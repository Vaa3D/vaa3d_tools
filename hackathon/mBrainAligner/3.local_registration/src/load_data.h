#ifndef __LOAD_DATA_H__
#define __LOAD_DATA_H__

//#include "jba_mainfunc.h"

#include <vector>

#include "until.h"
using namespace std;

#define WANT_STREAM

bool load_density_map(QString qs_filename_img_sub_seg,  map <int, float *> & density_map_sub);

bool loadImageData(Parameter input_Parameter, QString data_file, QString qs_filename_img_sub, unsigned char *&p_img_sub, float *&p_img32f_tar, float *&p_img32f_sub_bk,
	float *& p_img32_sub_label, long long * &sz_img);

bool LoadLandmarksData(vector<point3D64F> &vec_corners, vector<point3D64F> &fine_sub_corner, vector<point3D64F> &aver_corner, vector<int> &label, QString data_file,
	QString fine_filename, long long *sz_img, Parameter &input_Parameter, float  **** p_img_label_4d,
	QString qs_filename_img_sub_seg, map <int, float *> & density_map_sub, float  *& fmost_label_edge, float  ****&fmost_label_edge_4d);

bool outline_detec(float *& p_img_input, long long *& sz_img_input, float *& img_edge);

bool update_sub_corner(QString qs_filename_img_sub_seg, float * &fmost_label_edge, float  **** &fmost_label_edge_4d, vector<point3D64F> & fine_sub_corner);

#endif
