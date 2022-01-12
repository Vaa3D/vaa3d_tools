//main_mbrainAligner.cpp
//2021-5-13

#include <QtGui>
#include <stdio.h>

#include <iostream>
//#include <io.h>
//#include <direct.h>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <numeric>

#include "getopt.h"

#include "stackutil.h"
#include "basic_surf_objs.h"
#include "q_warp_affine_tps.h"

#include "edge_detection.h"
#include "rpm.h"

#include "q_imresize.cpp"
#include "q_convolve.h"

#include "q_histogram_matching.h"
#include "q_rigidaffine_registration.h"

using namespace std;
#define absd(a) ((a)>(-a)?(a):(-a))

void printHelp();
//global
bool my_img_affine_warp(cv::Mat T, const unsigned char *p_img_sub, const long long *sz_img_sub,
	const long long *sz_img_affine, unsigned char *&p_img_affine)
{
	if (p_img_sub == 0 || sz_img_sub == 0)
	{
		printf("ERROR: p_img_sub or sz_img_sub is invalid.\n");
		return false;
	}
	if (p_img_affine)
	{
		printf("WARNNING: output image pointer is not null, original memory it point to will lost!\n");
		p_img_affine = 0;
	}
	//assign output/warp image size
	long long sz_img_output[4] = { 0 };
	if (sz_img_output[0] == 0)		sz_img_output[0] = sz_img_affine[0];
	if (sz_img_output[1] == 0)		sz_img_output[1] = sz_img_affine[1];
	if (sz_img_output[2] == 0)		sz_img_output[2] = sz_img_affine[2];
	sz_img_output[3] = sz_img_sub[3];

	//allocate memory
	p_img_affine = new unsigned char[sz_img_output[0] * sz_img_output[1] * sz_img_output[2] * sz_img_output[3]]();
	if (!p_img_affine)
	{
		printf("ERROR: Fail to allocate memory for p_img_sub2tar.\n");
		return false;
	}
	unsigned char ****p_img_sub_4d = 0, ****p_img_sub2tar_4d = 0;
	if (!new4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3], p_img_sub) ||
		!new4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3], p_img_affine))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if (p_img_affine) 		{ delete[]p_img_affine;		p_img_affine = 0; }
		if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
		if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }
		return false;
	}
	//estimate the affine matrix
	cv::Mat tmp = T.inv();
	tmp = tmp.t();
	Matrix x4x4_affinematrix(4, 4);
	x4x4_affinematrix(1, 1) = tmp.at<double>(0, 0);	x4x4_affinematrix(1, 2) = tmp.at<double>(0, 1);	x4x4_affinematrix(1, 3) = tmp.at<double>(0, 2);	x4x4_affinematrix(1, 4) = tmp.at<double>(0, 3);
	x4x4_affinematrix(2, 1) = tmp.at<double>(1, 0);	x4x4_affinematrix(2, 2) = tmp.at<double>(1, 1);	x4x4_affinematrix(2, 3) = tmp.at<double>(1, 2);	x4x4_affinematrix(2, 4) = tmp.at<double>(1, 3);
	x4x4_affinematrix(3, 1) = tmp.at<double>(2, 0);	x4x4_affinematrix(3, 2) = tmp.at<double>(2, 1); x4x4_affinematrix(3, 3) = tmp.at<double>(2, 2);	x4x4_affinematrix(3, 4) = tmp.at<double>(2, 3);
	x4x4_affinematrix(4, 1) = tmp.at<double>(3, 0);	x4x4_affinematrix(4, 2) = tmp.at<double>(3, 1);	x4x4_affinematrix(4, 3) = tmp.at<double>(3, 2);	x4x4_affinematrix(4, 4) = tmp.at<double>(3, 3);
	//affine image warping
#pragma omp parallel for
	for (long long x = 0; x < sz_img_output[0]; x++)
	{
		for (long long y = 0; y < sz_img_output[1]; y++)
			for (long long z = 0; z < sz_img_output[2]; z++)
			{
				Matrix x_pt_sub2tar_homo(4, 1), x_pt_sub_homo(4, 1);
				//compute the inverse affine projected coordinate in subject image
				x_pt_sub2tar_homo(1, 1) = x;
				x_pt_sub2tar_homo(2, 1) = y;
				x_pt_sub2tar_homo(3, 1) = z;
				x_pt_sub2tar_homo(4, 1) = 1.0;
				x_pt_sub_homo = x4x4_affinematrix*x_pt_sub2tar_homo;

				//linear interpolate
				//coordinate in subject image
				double cur_pos[3];//x,y,z
				cur_pos[0] = x_pt_sub_homo(1, 1);
				cur_pos[1] = x_pt_sub_homo(2, 1);
				cur_pos[2] = x_pt_sub_homo(3, 1);

				//if interpolate pixel is out of subject image region, set to -inf
				if (cur_pos[0]<0 || cur_pos[0]>sz_img_sub[0] - 1 ||
					cur_pos[1]<0 || cur_pos[1]>sz_img_sub[1] - 1 ||
					cur_pos[2]<0 || cur_pos[2]>sz_img_sub[2] - 1)
				{
					p_img_sub2tar_4d[0][z][y][x] = 0.0;
					continue;
				}

				//find 8 neighbor pixels boundary
				long long x_s, x_b, y_s, y_b, z_s, z_b;
				x_s = floor(cur_pos[0]);		x_b = ceil(cur_pos[0]);
				y_s = floor(cur_pos[1]);		y_b = ceil(cur_pos[1]);
				z_s = floor(cur_pos[2]);		z_b = ceil(cur_pos[2]);

				//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
				double l_w, r_w, t_w, b_w;
				l_w = 1.0 - (cur_pos[0] - x_s);	r_w = 1.0 - l_w;
				t_w = 1.0 - (cur_pos[1] - y_s);	b_w = 1.0 - t_w;
				//compute weight for higher slice and lower slice
				double u_w, d_w;
				u_w = 1.0 - (cur_pos[2] - z_s);	d_w = 1.0 - u_w;

				//linear interpolate each channel
				for (long long c = 0; c < sz_img_output[3]; c++)
				{
					//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double higher_slice;
					higher_slice = t_w*(l_w*p_img_sub_4d[c][z_s][y_s][x_s] + r_w*p_img_sub_4d[c][z_s][y_s][x_b]) +
						b_w*(l_w*p_img_sub_4d[c][z_s][y_b][x_s] + r_w*p_img_sub_4d[c][z_s][y_b][x_b]);
					//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
					double lower_slice;
					lower_slice = t_w*(l_w*p_img_sub_4d[c][z_b][y_s][x_s] + r_w*p_img_sub_4d[c][z_b][y_s][x_b]) +
						b_w*(l_w*p_img_sub_4d[c][z_b][y_b][x_s] + r_w*p_img_sub_4d[c][z_b][y_b][x_b]);
					//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
					p_img_sub2tar_4d[c][z][y][x] = u_w*higher_slice + d_w*lower_slice;
				}
			}
	}
	//------------------------------------------------------------------------------------------------------------------------------------
	printf("free memory. \n");
	if (p_img_sub_4d) 		{ delete4dpointer(p_img_sub_4d, sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]); }
	if (p_img_sub2tar_4d) 	{ delete4dpointer(p_img_sub2tar_4d, sz_img_output[0], sz_img_output[1], sz_img_output[2], sz_img_output[3]); }

	return true;
}


//ssd
void releasememory_rigidaffine(unsigned char *&, unsigned char *&, unsigned char *&, double *&, double *&, unsigned char *&);
void releasememory_nonrigid_FFD(unsigned char *&, unsigned char *&, unsigned char *&, unsigned char *&, double *&, double *&, unsigned char *&);

//norm
void releasememory_norm(unsigned char *&p_img_tar_input, unsigned char *&p_img_sub_input, double *&p_img64f_tar, double *&p_img64f_sub);
bool Split(const std::string &str, std::vector<std::string> &ret, std::string sep = ",");

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//read arguments
	//input
	QString qs_filename_img_sub = NULL;      //sub_img
	QString qs_filename_img_tar = NULL;      //tar_img
	QString qs_filename_img_rpm_tar = NULL;
	QString qs_algorithm_choose = NULL;
	int threshold = 20;
	//output
	QString qs_filename_output = NULL;
	

	//affine input
	QString qs_filename_marker_tar = NULL;
	QString qs_filename_marker_sub = NULL;

	long long sz_img_resize[4] = { 0, 0, 0, 0 };

	
	int i_regtype = 1;				   //0:rigid; 1:affine
	long l_refchannel = 0;
	double d_downsample_ratio = 8;
	int i_resizestyle = 1;				//0:linear, 1:average, 2:nn
	bool b_histmatch = 1;
	bool b_alignedges = 0;
	bool b_gausmooth = 1;
	long l_gauradius = 2;
	double d_gausigma = 0.5;

	int l_kenelradius = 20;


	bool affine = false;
	bool global = false;
	bool ssd = false;
	bool norm = false;


	int c;
	static char optstring[] = "hf:m:o:t:s:c:d:p:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring)) != -1)
	{
		switch (c)
		{
		case 'h':
			printHelp();
			return 0;
			break;
		case 'f':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -f.\n");
				return 1;
			}
			qs_filename_img_tar.append(optarg);
			break;
		case 'm':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -m.\n");
				return 1;
			}
			qs_filename_img_sub.append(optarg);
			break;
		case 'o':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
				return 1;
			}
			qs_filename_output.append(optarg);
			break;
		case 't':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_marker_tar.append(optarg);
			break;
		case 's':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			qs_filename_marker_sub.append(optarg);
			break;
		case 'c':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -c.\n");
				return 1;
			}
			qs_filename_img_rpm_tar.append(optarg);
			break;
		case 'd':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -th.\n");
				return 1;
			}
			threshold = atoi(optarg);
			break;
		case 'p':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -p.\n");
				return 1;
			}
			qs_algorithm_choose.append(optarg);
			break;
		case '?':
			fprintf(stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
			return 1;
			break;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	unsigned char * p_img_sub2tar_warped = 0;
	long long *sz_img_sub = 0;                //stripremove_size
	unsigned char *p_img_sub = 0;
	int datatype_sub = 0;
	//------------------------------------------------------------------------------------------------------------------------------------
	//Parse parameter
	string para = qs_algorithm_choose.toStdString();
	vector<string> res;
	Split(para, res, "+");
	for (int i = 0; i < res.size(); i++){
		if (res[i] == "a"){ affine = true; }
		else if (res[i] == "r"){ global = true; }
		else if (res[i] == "f"){ ssd = true; }
		else if (res[i] == "n"){ norm = true; }
	}
	
	printf("=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*\n");
	if (affine){
		printf("\tAffine");
	}
	if (global){
		printf("\tGlobal");
	}
	if (ssd){
		printf("+ssd");
	}
	if (norm){
		printf("+norm");
	}
	printf("\n=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*\n");
	if (affine && qs_filename_marker_tar == NULL && qs_filename_marker_sub == NULL){
		printf("No affine markers, Please input tar and sub markers! example:\n");
		printf("\t-t tar.marker\n");
		printf("\t-s sub.marker\n");
		return -1;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("Read target image. \n");
	unsigned char *p_img_tar_input = 0;
	long sz_img_tar_input[4];

	//read target image
	int datatype_tar_input = 0;
	long long *sz_img_tar_tmp = 0;
	if (!loadImage((char *)qPrintable(qs_filename_img_tar), p_img_tar_input, sz_img_tar_tmp, datatype_tar_input))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
		return 0;
	}

	for (int i = 0; i < 4; i++)	sz_img_tar_input[i] = sz_img_tar_tmp[i];
	//if (sz_img_tar_tmp) { delete[]sz_img_tar_tmp;	sz_img_tar_tmp = 0; }
	printf("\t>>read target image file [%s] complete.\n", qPrintable(qs_filename_img_tar));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_tar_input[0], sz_img_tar_input[1], sz_img_tar_input[2], sz_img_tar_input[3]);
	printf("\t\tdatatype: %d\n", datatype_tar_input);

	//----------------------------------------------------
	if (!qs_filename_img_sub.isNull())
	{
		if (!loadImage((char *)qPrintable(qs_filename_img_sub), p_img_sub, sz_img_sub, datatype_sub))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
			return false;
		}
		printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_img_sub));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]);
		printf("\t\tdatatype: %d\n", datatype_sub);
		if (datatype_sub != 1)
		{
			printf("ERROR: Input image datatype is not UINT8.\n");
			return false;
		}
	}

	for (long long x = 0; x < sz_img_sub[0]; x++){
		for (long long y = 0; y < sz_img_sub[1]; y++){
			for (long long z = 0; z < sz_img_sub[2]; z++){
				long index = z*sz_img_sub[1] * sz_img_sub[0] + y*sz_img_sub[0] + x;
				if (p_img_sub[index] <= threshold){
					p_img_sub[index] = 0;
				}
			}
		}
	}
	//-------------------------------------------------------

	//filename for output
	if (qs_filename_img_sub.contains("\\")){
		qs_filename_img_sub = qs_filename_img_sub.replace("\\", "/", Qt::CaseInsensitive);
	}
	string s;
	s = qs_filename_img_sub.toStdString();
	s = s.substr(s.find_last_of('/') + 1, s.find_last_of('.') - s.find_last_of('/') - 1);
	//string filename_out_global = qs_filename_output.toStdString() + '/' + s + "_global" + ".v3draw";
	string result_filename = qs_filename_output.toStdString() + '/' + "global" + ".v3draw";

	//------------------------------------------------------------------------------------------------------------------------------------
	if (global)
	{
		printf("=========================================Global Registration Start!===========================================\n");
		printf("1. Load RPM target subject image. \n");

		unsigned char *p_img_rpm_tar = 0;
		long long *sz_rpm_tar = 0;
		int datatype_rpm_tar = 0;

		if (!qs_filename_img_rpm_tar.isNull())
		{
			if (!loadImage((char *)qPrintable(qs_filename_img_rpm_tar), p_img_rpm_tar, sz_rpm_tar, datatype_rpm_tar))
			{
				printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_rpm_tar));
				return false;
			}
			printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_img_rpm_tar));
			printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_rpm_tar[0], sz_rpm_tar[1], sz_rpm_tar[2], sz_rpm_tar[3]);
			printf("\t\tdatatype: %d\n", datatype_rpm_tar);
			if (datatype_rpm_tar != 1)
			{
				printf("ERROR: Input image datatype is not UINT8.\n");
				return false;
			}
		}

		long long piex_size = sz_img_sub[0] * sz_img_sub[1] * sz_img_sub[2] * sz_img_sub[3];
		unsigned char *norm_sub = nullptr;
		norm_sub = new unsigned char[piex_size]();
		int max_val = INT_MIN, min_val = INT_MAX;
		for (long long i = 0; i < piex_size; i++)
		{
			if (p_img_sub[i] > max_val)
				max_val = p_img_sub[i];
			if (p_img_sub[i] < min_val)
				min_val = p_img_sub[i];
		}
		float k = (255 - 100) / (float)(max_val - min_val);
		for (long long i = 0; i < piex_size; i++)
		{
			if (p_img_sub[i] < 1)
				continue;
			norm_sub[i] = 100 + k*(p_img_sub[i] - min_val);
		}
		
		printf("2. Extract the external contour points. \n");
		vector<Coord3D_PCM> vec_fix_edge;
		vector<Coord3D_PCM> vec_mov_edge;
		if (!edgeContourExtract(p_img_rpm_tar, norm_sub, (long long*)sz_img_tar_tmp, sz_img_sub, vec_fix_edge, vec_mov_edge))
		{
			cout << "ERROR:edgeContourExtract return false!" << endl;
			return -1;
		}
		printf("\t[%d] fixed image edge point detected \n", vec_fix_edge.size());
		printf("\t[%d] moving image edge point detected \n", vec_mov_edge.size());

		Mat x, y;
		float *tmp_array = 0;
		tmp_array = new(std::nothrow) float[3 * vec_fix_edge.size()]();
		QList<ImageMarker> ql_marker_tar;
		for (unsigned long i = 0; i < vec_fix_edge.size(); i++)
		{
			ImageMarker marker;
			marker.radius = 3;
			marker.color.r = 255;
			marker.color.g = 255;
			marker.color.b = 255;
			marker.shape = 1;
			marker.x = vec_fix_edge[i].x; marker.y = vec_fix_edge[i].y; marker.z = vec_fix_edge[i].z;
			tmp_array[3 * i] = vec_fix_edge[i].x; tmp_array[3 * i + 1] = vec_fix_edge[i].y; tmp_array[3 * i + 2] = vec_fix_edge[i].z;
			ql_marker_tar.push_back(marker);
		}

		string tar_marker = qs_filename_output.toStdString() + '/' + s + "_RPM_tar" + ".marker";
		writeMarker_file(tar_marker.c_str(), ql_marker_tar);

		y = Mat::zeros(vec_fix_edge.size(), 3, CV_64F);
		for (int i = 0; i < vec_fix_edge.size(); i++)
		{
			double* yptr = y.ptr<double>(i);
			for (int j = 0; j < 3; j++)
			{
				yptr[j] = tmp_array[3 * i + j];
			}
		}
		
		float *tmp_array1 = 0;
		tmp_array1 = new(std::nothrow) float[3 * vec_mov_edge.size()]();
		for (unsigned long i = 0; i < vec_mov_edge.size(); i++)
		{
			tmp_array1[3 * i] = vec_mov_edge[i].x; tmp_array1[3 * i + 1] = vec_mov_edge[i].y; tmp_array1[3 * i + 2] = vec_mov_edge[i].z;
		}
		x = Mat::zeros(vec_mov_edge.size(), 3, CV_64F);
		for (int i = 0; i < vec_mov_edge.size(); i++)
		{
			double* xptr = x.ptr<double>(i);
			for (int j = 0; j < 3; j++)
			{
				xptr[j] = tmp_array1[3 * i + j];
			}
		}

		//if (p_img_tar) 			    { delete[]p_img_tar;			p_img_tar = 0; }
		if (tmp_array) 			    { delete[]tmp_array;			tmp_array = 0; }
		if (tmp_array1) 			{ delete[]tmp_array1;			tmp_array1 = 0; }

		//-------------------------registration--------------------------------------------
		Mat T, Tx, Ty, xp, xn, yn, cov, tmp, tmp1, near, Tpca, tar_vec;

		printf("3. Normalized point set. \n");
		points_norm(x, Tx, xn);
		points_norm(y, Ty, yn);

		printf("4. Initial registration. \n");
		pca(xn, yn, Tpca, xp, tar_vec);

		//iteration
		printf("5. iteration... \n");
		int flag_stop = 0;
		double anneal_rate = 0.96;
		double t_final = 0.01;//0.01
		double t = 0.3;//0.25
		Mat vx = xp.clone(), vy = yn.clone(), T_affine;
		while (flag_stop != 1)
		{
			int innermaxiter = 10;
			for (int i = 1; i < innermaxiter; i++)
			{
				//given vx, y, update transformation 
				vy = calc_vy(vx, yn, t);
				//update moving pointset vx
				T_affine = affine3d(xp, vy);
				tmp = am_acol(xp, 0)*T_affine.t();
				vx = am_acol(tmp, 1);
				if (t < t_final)
				{
					flag_stop = 1;
				}
			}
			t = t * anneal_rate;
		}
		printf("6. Calculate affine matrix parameters. \n");

		//suppress the reflection in the tar second big std dirction
		Mat suppress_reflect = (Mat_<double>(4, 4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
		if (determinant(Tpca)*determinant(T_affine) < 0)
		{
			Mat second_dirction = tar_vec.row(2);
			double max = 0;
			uint reflect_dirction_flag = 0;
			for (int i = 0; i < 3; i++)
			{
				if (second_dirction.at<double>(i)>max)
				{
					max = second_dirction.at<double>(i);
					reflect_dirction_flag = i;
				}
			}
			switch (reflect_dirction_flag)
			{
			case(0) :
				suppress_reflect = (Mat_<double>(4, 4) << -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
			case(1) :
				suppress_reflect = (Mat_<double>(4, 4) << 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
			case(2) :
				suppress_reflect = (Mat_<double>(4, 4) << 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1);
			default:break;
			}
		}

		tmp1 = Mat::zeros(4, 4, CV_64F);
		Tpca.copyTo(tmp1(Range(0, 3), Range(0, 3)));
		tmp1.at<double>(3, 3) = 1;
		//T record final trans mat for original sub points
		T = Tx*tmp1*T_affine.t()*suppress_reflect*Ty.inv();

		Mat mt = T.t();
		Matrix x4x4(4, 1), x_pt(4, 1);
		Matrix x4x4_affinematrix(4, 4);
		ImageMarker temp;
		QList <ImageMarker> vec_newmarker;
		x4x4_affinematrix(1, 1) = mt.at<double>(0, 0); x4x4_affinematrix(1, 2) = mt.at<double>(0, 1); x4x4_affinematrix(1, 3) = mt.at<double>(0, 2);
		x4x4_affinematrix(1, 4) = mt.at<double>(0, 3);
		x4x4_affinematrix(2, 1) = mt.at<double>(1, 0); x4x4_affinematrix(2, 2) = mt.at<double>(1, 1); x4x4_affinematrix(2, 3) = mt.at<double>(1, 2);
		x4x4_affinematrix(2, 4) = mt.at<double>(1, 3);
		x4x4_affinematrix(3, 1) = mt.at<double>(2, 0); x4x4_affinematrix(3, 2) = mt.at<double>(2, 1); x4x4_affinematrix(3, 3) = mt.at<double>(2, 2);
		x4x4_affinematrix(3, 4) = mt.at<double>(2, 3);
		x4x4_affinematrix(4, 1) = mt.at<double>(3, 0); x4x4_affinematrix(4, 2) = mt.at<double>(3, 1); x4x4_affinematrix(4, 3) = mt.at<double>(3, 2);
		x4x4_affinematrix(4, 4) = mt.at<double>(3, 3);

		for (long long row = 1; row <= x4x4_affinematrix.nrows(); row++)
		{
			printf("\t");
			for (long long col = 1; col <= x4x4_affinematrix.ncols(); col++)
				printf("%.3f\t", x4x4_affinematrix(row, col));
			printf("\n");
		}

		for (long i = 0; i < vec_fix_edge.size(); i++)
		{
			x4x4(1, 1) = vec_fix_edge[i].x;
			x4x4(2, 1) = vec_fix_edge[i].y;
			x4x4(3, 1) = vec_fix_edge[i].z;
			x4x4(4, 1) = 1.0;
			x_pt = x4x4_affinematrix.i()*x4x4;
			temp.x = x_pt(1, 1);
			temp.y = x_pt(2, 1);
			temp.z = x_pt(3, 1);
			vec_newmarker.push_back(temp);
			printf("%.3f,%.3f,%.3f\n", temp.x, temp.y, temp.z);
		}

		for (long long row = 1; row <= x4x4_affinematrix.nrows(); row++)
		{
			printf("\t");
			for (long long col = 1; col <= x4x4_affinematrix.ncols(); col++)
				printf("%.3f\t", x4x4_affinematrix(row, col));
			printf("\n");
		}

		string sub_marker = qs_filename_output.toStdString() + '/' + s + "_RPM_sub" + ".marker";
		writeMarker_file(sub_marker.c_str(), vec_newmarker);

		printf("7. Warping image. \n");
		
		if (!my_img_affine_warp(T, p_img_sub, sz_img_sub, (long long*)sz_img_tar_tmp, p_img_sub2tar_warped)){
			return false;
		}

		string filename_out_rpm = qs_filename_output.toStdString() + '/' + s + "_RPM" + ".v3draw";
		saveImage(filename_out_rpm.c_str(), (unsigned char*)p_img_sub2tar_warped, (long long*)sz_img_tar_tmp, 1);
		saveImage(result_filename.c_str(), (unsigned char*)p_img_sub2tar_warped, (long long*)sz_img_tar_tmp, 1);
		//if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }
		if (p_img_rpm_tar)          { delete[]p_img_rpm_tar;        p_img_rpm_tar = 0; }
		if (sz_rpm_tar) 			{ delete[]sz_rpm_tar;			sz_rpm_tar = 0; }
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	if (affine)
	{
		printf("=========================================Affine Registration Start!===========================================\n");
		string filename_out_affine = qs_filename_output.toStdString() + '/' + s + "_affine" + ".v3draw";

		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf(">>Image affine transformation:\n");
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf(">>input parameters:\n");
		printf(">>  input target  marker:          %s\n", qPrintable(qs_filename_marker_tar));
		printf(">>  input subject marker:          %s\n", qPrintable(qs_filename_marker_sub));
		printf(">>  input subject data:            %s\n", qPrintable(qs_filename_img_sub));
		//printf(">>  output image  size:            [%ld,%ld,%ld]\n", sz_img_resize[0], sz_img_resize[1], sz_img_resize[2]);
		printf(">>-------------------------\n");
		printf(">>output parameters:\n");
		printf(">>  output sub2tar_affine data:       %s\n", filename_out_affine.c_str());
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

		printf("1. Read target and subject marker files. \n");
		QList<ImageMarker> ql_marker_tar, ql_marker_sub;
		if (qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
		{
			ql_marker_tar = readMarker_file(qs_filename_marker_tar);
			ql_marker_sub = readMarker_file(qs_filename_marker_sub);
			printf("\t>>Target: read %d markers from [%s]\n", ql_marker_tar.size(), qPrintable(qs_filename_marker_tar));
			printf("\t>>Subject:read %d markers from [%s]\n", ql_marker_sub.size(), qPrintable(qs_filename_marker_sub));
		}
		//else
		//{
		//	printf("ERROR: at least one marker file is invalid.\n");
		//	return false;
		//}

		//re-formate to vector
		vector<Coord3D_PCM> vec_tar, vec_sub;
		long l_minlength = min(ql_marker_tar.size(), ql_marker_sub.size());
		for (long i = 0; i < l_minlength; i++)
		{
			vec_tar.push_back(Coord3D_PCM(ql_marker_tar[i].x, ql_marker_tar[i].y, ql_marker_tar[i].z));
			vec_sub.push_back(Coord3D_PCM(ql_marker_sub[i].x, ql_marker_sub[i].y, ql_marker_sub[i].z));
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		/*printf("2. Read subject image. \n");
		
		if (!loadImage((char *)qPrintable(qs_filename_img_sub), p_img_sub, sz_img_sub, datatype_sub))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
			return false;
		}
		printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_img_sub));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]);
		printf("\t\tdatatype: %d\n", datatype_sub);*/

		sz_img_resize[0] = sz_img_tar_input[0];
		sz_img_resize[1] = sz_img_tar_input[1];
		sz_img_resize[2] = sz_img_tar_input[2];
		//}

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("3. Affine transform the subject image to target. \n");
		sz_img_resize[3] = sz_img_sub[3];
		if (!q_imagewarp_affine(vec_tar, vec_sub, p_img_sub, sz_img_sub, sz_img_resize, p_img_sub2tar_warped))
		{
			printf("ERROR: q_imagewarp_affine return false!\n");
			if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
			if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }
			return false;
		}
		//save image

		saveImage(filename_out_affine.c_str(), p_img_sub2tar_warped, sz_img_resize, 1);
		saveImage(result_filename.c_str(), p_img_sub2tar_warped, sz_img_resize, 1);
		if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
		if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }
		//affine end
		//----------------------------------------------------------------------------------------------------------------------------------
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	if (ssd)
	{
		printf("=========================================SSD Registration Start!===========================================\n");
		
		CParas_reg paras;
		paras.i_regtype = i_regtype;
		paras.b_alignmasscenter = 1;
		paras.l_iter_max = 500;
		paras.d_step_inimultiplyfactor = 5;
		paras.d_step_annealingratio = 0.95;
		paras.d_step_min = 0.01;

		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		if (i_regtype == 0) printf(">>Rigid registration\n");
		if (i_regtype == 1) printf(">>Affine registration\n");
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		printf(">>input parameters:\n");
		printf(">>  input target  image:          %s\n", qPrintable(qs_filename_img_tar));
		printf(">>  input reference channel:      %ld\n", l_refchannel);
		printf(">>  input downsample ratio:       %.2f\n", d_downsample_ratio);
		printf(">>-------------------------\n");
		//printf(">>output parameters:\n");
		//printf(">>  output sub2tar image:         %s\n", qPrintable(qs_filename_sub2tar_ssd));
		//printf(">>  output meshgrid apo:          %s\n", qPrintable(qs_filename_swc_grid));
		printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		//------------------------------------------------------------------------------------------------------------------------------------
		//pointers definition (needed to be released)
		unsigned char *p_img8u_tar = 0, *p_img8u_sub = 0;			//intermediate images pointer (could be 1c, resize, smoothed image)
		double *p_img64f_tar = 0, *p_img64f_sub = 0;					//intermediate images pointer (used by registration function)
		unsigned char *p_img8u_sub_warp = 0;
		long sz_img_sub_input[4];

		unsigned char * p_img_sub_input = 0;
		p_img_sub_input = p_img_sub2tar_warped;

		if (affine){
			sz_img_sub_input[0] = sz_img_resize[0];
			sz_img_sub_input[1] = sz_img_resize[1];
			sz_img_sub_input[2] = sz_img_resize[2];
			sz_img_sub_input[3] = sz_img_resize[3];
		}
		if (global){
			sz_img_sub_input[0] = sz_img_tar_input[0];
			sz_img_sub_input[1] = sz_img_tar_input[1];
			sz_img_sub_input[2] = sz_img_tar_input[2];
			sz_img_sub_input[3] = sz_img_tar_input[3];
		}
		
		//------------------------------------------------------------------------------------------------------------------------------------
		printf("2. Extract reference channel. \n");	//do this at first to save memory
		{
			//target
			unsigned char *p_img_1c = 0;
			if (!q_extractchannel(p_img_tar_input, sz_img_tar_input, l_refchannel, p_img_1c))
			{
				printf("ERROR: q_extractchannel() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			if (p_img8u_tar) 			{ delete[]p_img8u_tar;			p_img8u_tar = 0; }
			p_img8u_tar = p_img_1c;		p_img_1c = 0;
			//if (p_img_tar_input) 		{ delete[]p_img_tar_input;		p_img_tar_input = 0; }
			//subject
			if (!q_extractchannel(p_img_sub_input, sz_img_sub_input, l_refchannel, p_img_1c))
			{
				printf("ERROR: q_extractchannel() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			if (p_img8u_sub) 			{ delete[]p_img8u_sub;			p_img8u_sub = 0; }
			p_img8u_sub = p_img_1c;		p_img_1c = 0;
			if (!ssd)
				if (p_img_sub_input) 	{ delete[]p_img_sub_input;		p_img_sub_input = 0; }
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("3. Downsample 1c images and resize them to the same size. \n");
		long sz_img[4] = { sz_img_tar_input[0], sz_img_tar_input[1], sz_img_tar_input[2], 1 };
		double d_downsample_ratio_tar[3], d_downsample_ratio_sub[3];
		{
			//compute the downsampled image size and the real downsample ratios (float)
			for (int i = 0; i < 3; i++)
			{
				sz_img[i] = sz_img_tar_input[i] / d_downsample_ratio + 0.5;
				d_downsample_ratio_tar[i] = sz_img_tar_input[i] / double(sz_img[i]);
				d_downsample_ratio_sub[i] = sz_img_sub_input[i] / double(sz_img[i]);
			}

			//resize target image if necessary
			printf("\t>>downsample target image from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n", sz_img_tar_input[0], sz_img_tar_input[1], sz_img_tar_input[2], sz_img[0], sz_img[1], sz_img[2]);
			if (sz_img_tar_input[0] != sz_img[0] || sz_img_tar_input[1] != sz_img[1] || sz_img_tar_input[2] != sz_img[2])
			{
				unsigned char *p_img_tmp = 0;
				long sz_img_old[4] = { sz_img_tar_input[0], sz_img_tar_input[1], sz_img_tar_input[2], 1 };
				if (!q_imresize_3D(p_img8u_tar, sz_img_old, i_resizestyle, sz_img, p_img_tmp))
				{
					printf("ERROR: q_imresize_3D() return false!\n");
					releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
					return 0;
				}
				if (p_img8u_tar) 			{ delete[]p_img8u_tar;			p_img8u_tar = 0; }
				p_img8u_tar = p_img_tmp;	p_img_tmp = 0;
			}

			//resize subject image if necessary
			printf("\t>>downsample subject image from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n", sz_img_sub_input[0], sz_img_sub_input[1], sz_img_sub_input[2], sz_img[0], sz_img[1], sz_img[2]);
			if (sz_img_sub_input[0] != sz_img[0] || sz_img_sub_input[1] != sz_img[1] || sz_img_sub_input[2] != sz_img[2])
			{
				unsigned char *p_img_tmp = 0;
				long sz_img_old[4] = { sz_img_sub_input[0], sz_img_sub_input[1], sz_img_sub_input[2], 1 };
				if (!q_imresize_3D(p_img8u_sub, sz_img_old, i_resizestyle, sz_img, p_img_tmp))
				{
					printf("ERROR: q_imresize_3D() return false!\n");
					releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
					return 0;
				}
				if (p_img8u_sub) 			{ delete[]p_img8u_sub;			p_img8u_sub = 0; }
				p_img8u_sub = p_img_tmp;	p_img_tmp = 0;
			}
		}
		long l_npixels = sz_img[0] * sz_img[1] * sz_img[2];


		//------------------------------------------------------------------------------------------------------------------------------------
		if (b_histmatch)
		{
			printf("[optional] Match the histogram of subject to that of target. \n");
			unsigned char *p_img_tmp = 0;

			if (!q_histogram_matching_1c(p_img8u_tar, sz_img,
				p_img8u_sub, sz_img,
				p_img_tmp))
			{
				printf("ERROR: q_histogram_matching_1c() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			if (p_img8u_sub) 			{ delete[]p_img8u_sub;			p_img8u_sub = 0; }
			p_img8u_sub = p_img_tmp;	p_img_tmp = 0;

			//V3DLONG sz_img_tmp[4];
			//for (int i = 0; i < 4; i++)	sz_img_tmp[i] = sz_img[i];
			//saveImage("sub_histmatch.tif", p_img8u_sub, sz_img_tmp, 1);
		}


		//------------------------------------------------------------------------------------------------------------------------------------
		printf("4. Convert image data from uint8 to double and scale to [0~1]. \n");
		{
			p_img64f_tar = new double[l_npixels]();
			p_img64f_sub = new double[l_npixels]();
			if (!p_img64f_tar || !p_img64f_sub)
			{
				printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}

			//scale to [0~1]
			long l_maxintensity_tar, l_maxintensity_sub;
			l_maxintensity_tar = l_maxintensity_sub = 255;
			for (long i = 0; i < l_npixels; i++)
			{
				p_img64f_tar[i] = p_img8u_tar[i] / (double)l_maxintensity_tar;
				p_img64f_sub[i] = p_img8u_sub[i] / (double)l_maxintensity_sub;
			}

			if (p_img8u_tar) 			{ delete[]p_img8u_tar;			p_img8u_tar = 0; }
			if (p_img8u_sub) 			{ delete[]p_img8u_sub;			p_img8u_sub = 0; }
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		//from now on the target and subject have the same size, one channel and double datatype
		//the image data pointer will not changed, all operations are inplace

		////------------------------------------------------------------------------------------------------------------------------------------
		//printf("[optional] Gaussian smooth input target and subject images. \n");
		//{
		//	long l_kenelradius = 2;
		//	double d_sigma = 0.5;
		//	vector<double> vec1D_kernel;
		//	if (!q_kernel_gaussian_1D(l_kenelradius, d_sigma, vec1D_kernel))
		//	{
		//		printf("ERROR: q_kernel_gaussian_1D() return false!\n");
		//		releasememory_rigidaffine(p_img_tar_input, p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
		//		return 0;
		//	}

		//	printf("\tsmoothing target image.\n");
		//	if (!q_convolve_img64f_3D_fast(p_img64f_tar, sz_img, vec1D_kernel))
		//	{
		//		printf("ERROR: q_convolve64f_3D_fast() return false!\n");
		//		releasememory_rigidaffine(p_img_tar_input, p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
		//		return 0;
		//	}
		//	printf("\tsmoothing subject image.\n");
		//	if (!q_convolve_img64f_3D_fast(p_img64f_sub, sz_img, vec1D_kernel))
		//	{
		//		printf("ERROR: q_convolve64f_3D_fast() return false!\n");
		//		releasememory_rigidaffine(p_img_tar_input, p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
		//		return 0;
		//	}
		//	//q_save64f01_image(p_img64f_tar, sz_img, "tar.tif");
		//	//q_save64f01_image(p_img64f_sub, sz_img, "sub.tif");
		//}

		//------------------------------------------------------------------------------------------------------------------------------------
		if (b_alignedges)
		{
			printf("[optional] Generate gradient images. \n");
			double *p_img64f_gradnorm = 0;
			//target
			if (!q_gradientnorm(p_img64f_tar, sz_img, 1, p_img64f_gradnorm))
			{
				printf("ERROR: q_gradientnorm() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			if (p_img64f_tar) 		{ delete[]p_img64f_tar;			p_img64f_tar = 0; }
			p_img64f_tar = p_img64f_gradnorm; p_img64f_gradnorm = 0;
			//subject
			if (!q_gradientnorm(p_img64f_sub, sz_img, 1, p_img64f_gradnorm))
			{
				printf("ERROR: q_gradientnorm() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			if (p_img64f_sub) 		{ delete[]p_img64f_sub;			p_img64f_sub = 0; }
			p_img64f_sub = p_img64f_gradnorm; p_img64f_gradnorm = 0;

			//q_save64f01_image(p_img64f_tar, sz_img, "tar.tif");
			//q_save64f01_image(p_img64f_sub, sz_img, "sub.tif");
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		if (b_gausmooth)
		{
			printf("[optional] Gaussian smooth input target and subject images. \n");
			long l_kenelradius = l_gauradius;
			double d_sigma = d_gausigma;
			vector<double> vec1D_kernel;
			if (!q_kernel_gaussian_1D(l_kenelradius, d_sigma, vec1D_kernel))
			{
				printf("ERROR: q_kernel_gaussian_1D() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}

			printf("\tsmoothing target image.\n");
			if (!q_convolve_img64f_3D_fast(p_img64f_tar, sz_img, vec1D_kernel))
			{
				printf("ERROR: q_convolve64f_3D_fast() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			printf("\tsmoothing subject image.\n");
			if (!q_convolve_img64f_3D_fast(p_img64f_sub, sz_img, vec1D_kernel))
			{
				printf("ERROR: q_convolve64f_3D_fast() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}

			//q_save64f01_image(p_img64f_tar, sz_img, "tar_smooth.tif");
			//q_save64f01_image(p_img64f_sub, sz_img, "sub_smooth.tif");
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		if (i_regtype == 0) printf("5. Enter rigid registration iteration: \n");
		if (i_regtype == 1) printf("5. Enter affine registration iteration: \n");

		vector< vector< vector< vector<double> > > > vec4D_grid;
		if (!q_rigidaffine_registration(paras, p_img64f_tar, p_img64f_sub, sz_img, vec4D_grid))
		{
			printf("ERROR: q_affine_registration_SSD() return false!\n");
			releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
			return 0;
		}
		if (p_img64f_tar) 		{ delete[]p_img64f_tar;			p_img64f_tar = 0; }
		if (p_img64f_sub) 		{ delete[]p_img64f_sub;			p_img64f_sub = 0; }

		//rescale the grid according to the output downsample ratio
		//since we want the output image have the same size as target image, so here we use d_downsample_ratio_tar instead of d_downsample_ratio_sub
		for (long x = 0; x < 2; x++)
			for (long y = 0; y < 2; y++)
				for (long z = 0; z < 2; z++)
				{
					vec4D_grid[y][x][z][0] = (vec4D_grid[y][x][z][0] + 1)*d_downsample_ratio_tar[0] - 1;
					vec4D_grid[y][x][z][1] = (vec4D_grid[y][x][z][1] + 1)*d_downsample_ratio_tar[1] - 1;
					vec4D_grid[y][x][z][2] = (vec4D_grid[y][x][z][2] + 1)*d_downsample_ratio_tar[2] - 1;
				}

		string swcGrid = qs_filename_output.toStdString() + '/' + s + "_FFD_grid" + ".swc";
		//if (qs_filename_swc_grid != NULL)
		//{
		printf(">> Save deformed grid to swc file:[%s] \n", swcGrid.c_str());
		if (!q_rigidaffine_savegrid_swc(vec4D_grid, (long *)sz_img_tar_input, swcGrid.c_str()))
		{
			printf("ERROR: q_savegrid_swc() return false!\n");
			releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
			return 0;
		}
		//}

		if (ssd)
		{
			printf(">> Warp subject image according to the grid and save. \n");
			printf("\t>> resize the input subject image to the same size as target. \n");
			unsigned char *p_img_sub_resize = 0;
			long sz_img_output[4] = { sz_img_tar_input[0], sz_img_tar_input[1], sz_img_tar_input[2], sz_img_sub_input[3] };
			if (!q_imresize_3D(p_img_sub_input, sz_img_sub_input, 0, sz_img_output, p_img_sub_resize))
			{
				printf("ERROR: q_imresize_3D() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			if (p_img_sub_input) 			{ delete[]p_img_sub_input;			p_img_sub_input = 0; }

			printf("\t>> warp subject image according to deformed grid. \n");
			if (!q_rigidaffine_warpimage_baseongrid(paras.i_regtype, p_img_sub_resize, sz_img_output, vec4D_grid, p_img8u_sub_warp))
			{
				printf("ERROR: q_warpimage_baseongrid() return false!\n");
				if (p_img_sub_resize) 			{ delete[]p_img_sub_resize;			p_img_sub_resize = 0; }
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			if (p_img_sub_resize) 			{ delete[]p_img_sub_resize;			p_img_sub_resize = 0; }

			string ssdImage = qs_filename_output.toStdString() + '/' + s + "_FFD" + ".v3draw";
			printf("\t>> Save warped subject image to file:[%s] \n", ssdImage);
			long long sz_img_tmp[4];
			for (int i = 0; i < 4; i++)	sz_img_tmp[i] = sz_img_output[i];
			if (!saveImage(ssdImage.c_str(), p_img8u_sub_warp, sz_img_tmp, 1))
			{
				printf("ERROR: q_save64f01_image() return false!\n");
				releasememory_rigidaffine(p_img_sub_input, p_img8u_tar, p_img8u_sub, p_img64f_tar, p_img64f_sub, p_img8u_sub_warp);
				return 0;
			}
			saveImage(result_filename.c_str(), p_img8u_sub_warp, sz_img_tmp, 1);
			//output_image ==> norm_input
			p_img_sub2tar_warped = p_img8u_sub_warp;
		}
		//free memory
		printf(">>Free memory ssd\n");
		if (p_img8u_tar) 		{ delete[]p_img8u_tar;			p_img8u_tar = 0; }
		if (p_img8u_sub) 		{ delete[]p_img8u_sub;			p_img8u_sub = 0; }
		if (p_img64f_tar) 		{ delete[]p_img64f_tar;			p_img64f_tar = 0; }
		if (p_img64f_sub) 		{ delete[]p_img64f_sub;			p_img64f_sub = 0; }
		//ssd_end
		//----------------------------------------------------------------------------------------------------------------------------------
	}

	if (norm){
		printf("=========================================Norm Start!===========================================\n");
		//pointers definition (needed to be released)
		//long sz_img_tar_input[4], sz_img_sub_input[4];			//input image size. [0]:width, [1]:height, [2]:z, [3]:nchannel
		//unsigned char *p_img_tar_input = 0, *p_img_sub_input = 0;
		double *p_img64f_tar = 0, *p_img64f_sub = 0;

		//------------------------------------------------------------------------------------------------------------------------------------
		long l_npixels = sz_img_tar_input[0] * sz_img_tar_input[1] * sz_img_tar_input[2];

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("1. Convert image data from uint8 to double and scale to [0~1]. \n");
		{
			p_img64f_tar = new double[l_npixels]();
			p_img64f_sub = new double[l_npixels]();
			if (!p_img64f_tar || !p_img64f_sub)
			{
				printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
				releasememory_norm(p_img_tar_input, p_img_sub2tar_warped, p_img64f_tar, p_img64f_sub);
				return 0;
			}

			for (long i = 0; i < l_npixels; i++)
			{
				p_img64f_tar[i] = p_img_tar_input[i];
				p_img64f_sub[i] = p_img_sub2tar_warped[i];
			}
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("2. Calculate the local mean of input target and subject images. \n");
		{
			long l_kernel_width = l_kenelradius * 2 + 1;
			//fill average kernel
			vector<double> vec1D_kernel(l_kernel_width, 1.0 / l_kernel_width);

			printf("\tsmoothing target image.\n");
			if (!q_convolve_img64f_3D_fast(p_img64f_tar, (long *)sz_img_tar_input, vec1D_kernel))
			{
				printf("ERROR: q_convolve64f_3D_fast() return false!\n");
				releasememory_norm(p_img_tar_input, p_img_sub2tar_warped, p_img64f_tar, p_img64f_sub);
				return 0;
			}

			printf("\tsmoothing subject image.\n");
			if (!q_convolve_img64f_3D_fast(p_img64f_sub, (long *)sz_img_tar_input, vec1D_kernel))
			{
				printf("ERROR: q_convolve64f_3D_fast() return false!\n");
				releasememory_norm(p_img_tar_input, p_img_sub2tar_warped, p_img64f_tar, p_img64f_sub);
				return 0;
			}

			for (long i = 0; i < l_npixels; i++)
				if (p_img_sub2tar_warped[i] > 5)
				{
					double tmp = p_img_sub2tar_warped[i] * (p_img64f_tar[i] / p_img64f_sub[i]);
					if (tmp > 255) tmp = 255; else if (tmp < 0) tmp = 0;
					p_img_sub2tar_warped[i] = (unsigned char)(tmp);
				}

			long long sz_img_tmp[4];
			for (int i = 0; i < 4; i++)	sz_img_tmp[i] = sz_img_tar_input[i];

			string normImage = qs_filename_output.toStdString() + '/' + s + "_NORM" + ".v3draw";
			saveImage(normImage.c_str(), p_img_sub2tar_warped, sz_img_tmp, 1);
			saveImage(result_filename.c_str(), p_img_sub2tar_warped, sz_img_tmp, 1);
		}
		// norm end
		printf("free memory. \n");
		releasememory_norm(p_img_tar_input, p_img_sub2tar_warped, p_img64f_tar, p_img64f_sub);
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	//printf("4. free memory. \n");
	printf("Program exit success.\n");
	return true;
}


//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_warp_affine_tps\n");
	printf("Input paras:\n");
	printf("\t  -T   <filename_tar>				input target (image) file full name.\n");
	printf("\t  -t   <markerFilename_target>	input target marker file full name.\n");
	printf("\t  -s   <markerFilename_subject>	input subject marker file full name.\n");
	printf("\t  -S   <filename_sub>	            input subject (image) file full name.\n");
	printf("\t [-r]  <kenelradius>				iuput localintnorm radius.\n");
	printf("Output paras:\n");
	printf("\t [-a]  <filename_sub2tar_affine>  output sub2tar affine warped (image) file full name.\n");
	printf("\t [-o]  <filename_sub2tar_global>  output sub2tar global warped (image) file full name.\n");
	printf("\t [-x]  <output size x>            output image size in x dim(affine).\n");
	printf("\t [-y]  <output size y>            output image size in y dim(affine).\n");
	printf("\t [-z]  <output size z>            output image size in z dim(affine).\n");
	printf("\t [-f]  <markerFilename_target>	output target marker file full name(global registration).\n");
	printf("\t [-m]  <markerFilename_subject>	output subject marker file full name(global registration).\n");
	printf("\t [-g]  <filename_grid>			output ssd grid file full name.\n");
	printf("\t [-d]  <filename_sub2tar_ssd>	    output sub2tar ssd warped (image) file full name.\n");
	printf("\t [-n]  <filename_sub2tar_norm>	output sub2tar norm (image) file full name.\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}

//************************************************************************************************************************************
//release all allocated memory
void releasememory_rigidaffine(
	 unsigned char *&p_img_sub_input,
	unsigned char *&p_img8u_tar, unsigned char *&p_img8u_sub,
	double *&p_img64f_tar, double *&p_img64f_sub,
	unsigned char *&p_img8u_output_sub)
{
	//if (p_img_tar_input) 	{ delete[]p_img_tar_input;		p_img_tar_input = 0; }
	if (p_img_sub_input) 	{ delete[]p_img_sub_input;		p_img_sub_input = 0; }
	if (p_img8u_tar) 		{ delete[]p_img8u_tar;			p_img8u_tar = 0; }
	if (p_img8u_sub) 		{ delete[]p_img8u_sub;			p_img8u_sub = 0; }
	if (p_img64f_tar) 		{ delete[]p_img64f_tar;			p_img64f_tar = 0; }
	if (p_img64f_sub) 		{ delete[]p_img64f_sub;			p_img64f_sub = 0; }
	if (p_img8u_output_sub)  { delete[]p_img8u_output_sub;	p_img8u_output_sub = 0; }
	printf("Release all memory done!\n");
}
void releasememory_nonrigid_FFD(
	unsigned char *&p_img_tar_input, unsigned char *&p_img_sub_input,
	unsigned char *&p_img8u_tar, unsigned char *&p_img8u_sub,
	double *&p_img64f_tar, double *&p_img64f_sub,
	unsigned char *&p_img8u_output_sub)
{
	if (p_img_tar_input) 	{ delete[]p_img_tar_input;		p_img_tar_input = 0; }
	if (p_img_sub_input) 	{ delete[]p_img_sub_input;		p_img_sub_input = 0; }
	if (p_img8u_tar) 		{ delete[]p_img8u_tar;			p_img8u_tar = 0; }
	if (p_img8u_sub) 		{ delete[]p_img8u_sub;			p_img8u_sub = 0; }
	if (p_img64f_tar) 		{ delete[]p_img64f_tar;			p_img64f_tar = 0; }
	if (p_img64f_sub) 		{ delete[]p_img64f_sub;			p_img64f_sub = 0; }
	if (p_img8u_output_sub)  { delete[]p_img8u_output_sub;	p_img8u_output_sub = 0; }
	printf("Release all memory done!\n");
}

void releasememory_norm(
	unsigned char *&p_img_tar_input, unsigned char *&p_img_sub_input,
	double *&p_img64f_tar, double *&p_img64f_sub)
{
	if (p_img_tar_input) 	{ delete[]p_img_tar_input;		p_img_tar_input = 0; }
	if (p_img_sub_input) 	{ delete[]p_img_sub_input;		p_img_sub_input = 0; }
	if (p_img64f_tar) 		{ delete[]p_img64f_tar;			p_img64f_tar = 0; }
	if (p_img64f_sub) 		{ delete[]p_img64f_sub;			p_img64f_sub = 0; }

	printf("Release all memory done!\n");
}

bool Split(const std::string &str, std::vector<std::string> &ret, std::string sep) {
	if (str.empty()) {
		return false;
	}

	std::string temp;
	std::string::size_type begin = str.find_first_not_of(sep);
	std::string::size_type pos = 0;

	while (begin != std::string::npos) {
		pos = str.find(sep, begin);
		if (pos != std::string::npos) {
			temp = str.substr(begin, pos - begin);
			begin = pos + sep.length();
		}
		else {
			temp = str.substr(begin);
			begin = pos;
		}

		if (!temp.empty()) {
			ret.push_back(temp);
			temp.clear();
		}
	}
	return true;
}


