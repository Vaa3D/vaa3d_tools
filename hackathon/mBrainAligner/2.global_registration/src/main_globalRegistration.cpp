#include <stdio.h>
#include <iostream>
#include <io.h>
#include <direct.h>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <numeric>
#include "stackutil.h"
#include "basic_surf_objs.h"
#include "../../basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "edge_detection.h"
#include "rpm.h"
#include "getopt.h"
#include "../../jba/newmat11/newmatio.h"

using namespace std;

#define absd(a) ((a)>(-a)?(a):(-a))

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

int main(int argc, char *argv[])
{
	//-----------------------------------------------------------------------------------------
	QString qs_filename_img_sub = NULL;
	QString qs_filename_img_tar = NULL;
	QString qs_filename_output_path = NULL;
	QString qs_filename_sub_marker = NULL;
	QString qs_filename_tar_marker = NULL;

	/*-----------------------------------------getopt set--------------------------------*/
	int c;
	static char optstring[] = "f:m:o:t:s:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring)) != -1)
	{
		switch (c)
		{
		case 'f':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_img_tar = optarg;
			break;
		case 'm':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_img_sub = optarg;
			break;
		case 'o':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_output_path = optarg;
			break;
		case 't':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_tar_marker = optarg;
			break;
		case 's':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_sub_marker = optarg;
			break;
		case '?':
			fprintf(stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
			return 0;
		default:
			fprintf(stderr, "Illeagal arguments, do nothing.\n", optopt);
			return 0;
		}
	}

	printf("1. Load image. \n");
	unsigned char *p_img_sub = 0, *p_img_tar = 0;
	long long *sz_img_sub = 0, *sz_img_tar = 0;
	int datatype_input = 0;

	if (!qs_filename_img_sub.isNull())
	{
		if (!loadImage((char *)qPrintable(qs_filename_img_sub), p_img_sub, sz_img_sub, datatype_input))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
			return false;
		}
		printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_img_sub));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_sub[3]);
		printf("\t\tdatatype: %d\n", datatype_input);
		if (datatype_input != 1)
		{
			printf("ERROR: Input image datatype is not UINT8.\n");
			return false;
		}
	}
	if (!qs_filename_img_tar.isNull())
	{
		if (!loadImage((char *)qPrintable(qs_filename_img_tar), p_img_tar, sz_img_tar, datatype_input))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
			return false;
		}
		printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_img_tar));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3]);
		printf("\t\tdatatype: %d\n", datatype_input);
		if (datatype_input != 1)
		{
			printf("ERROR: Input image datatype is not UINT8.\n");
			return false;
		}
	}

	                  
	printf("2. Extract the external contour points. \n");
	vector<Coord3D_PCM> vec_fix_edge;
	vector<Coord3D_PCM> vec_mov_edge;
	if (!edgeContourExtract(p_img_tar, p_img_sub, sz_img_tar, sz_img_sub, vec_fix_edge, vec_mov_edge))
	{
		cout << "ERROR:edgeContourExtract return false!" << endl;
		return -1;
	}
	printf("\t[%d] fixed image edge point detected \n", vec_fix_edge.size());
	printf("\t[%d] moving image edge point detected \n", vec_mov_edge.size());

	Mat x, y;
	float *tmp_array = 0;
	tmp_array = new(std::nothrow) float[3 * vec_fix_edge.size()]();
	QList<ImageMarker> ql_marker_outline;
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
		ql_marker_outline.push_back(marker);
	}
	
	//writeMarker_file(qPrintable(qs_filename_tar_marker), ql_marker_outline);

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
	QList<ImageMarker> ql_marker_inner;
	for (unsigned long i = 0; i < vec_mov_edge.size(); i++)
	{
		ImageMarker marker;
		marker.radius = 3;
		marker.color.r = 255;
		marker.color.g = 255;
		marker.color.b = 255;
		marker.shape = 1;
		marker.x = vec_mov_edge[i].x; marker.y = vec_mov_edge[i].y; marker.z = vec_mov_edge[i].z;
		tmp_array1[3 * i] = vec_mov_edge[i].x; tmp_array1[3 * i + 1] = vec_mov_edge[i].y; tmp_array1[3 * i + 2] = vec_mov_edge[i].z;
		ql_marker_inner.push_back(marker);
	}
	writeMarker_file(qPrintable(qs_filename_sub_marker), ql_marker_inner);

	x = Mat::zeros(vec_mov_edge.size(), 3, CV_64F);
	for (int i = 0; i < vec_mov_edge.size(); i++)
	{
		double* xptr = x.ptr<double>(i);
		for (int j = 0; j < 3; j++)
		{
			xptr[j] = tmp_array1[3 * i + j];
		}
	}

	if (p_img_tar) 			    { delete[]p_img_tar;			p_img_tar = 0; }
	if (tmp_array) 			    { delete[]tmp_array;			tmp_array = 0; }
	if (tmp_array1) 			{ delete[]tmp_array1;			tmp_array1 = 0; }

	//-------------------------registration--------------------------------------------
	Mat T, Tx, Ty, xp, xn, yn, cov, tmp, tmp1, near, Tpca,tar_vec;

	printf("3. Normalized point set. \n");
	points_norm(x, Tx, xn);
	points_norm(y, Ty, yn);

	printf("4. Initial registration. \n");
	pca(xn, yn, Tpca, xp,tar_vec);

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
	Matrix x4x4(4, 1),x_pt(4, 1);
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
	
	for (long i = 0; i < vec_mov_edge.size(); i++)
	{
		x4x4(1, 1) = vec_mov_edge[i].x;
		x4x4(2, 1) = vec_mov_edge[i].y;
		x4x4(3, 1) = vec_mov_edge[i].z;
		x4x4(4, 1) = 1.0;
		x_pt = x4x4_affinematrix*x4x4; //不是逆是因为要和swc配准统一
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

	writeMarker_file(qPrintable(qs_filename_tar_marker), vec_newmarker);
	
	printf("7. Warping image. \n");
	unsigned char * p_img_affine = 0;
	if (!my_img_affine_warp(T, p_img_sub, sz_img_sub, sz_img_tar, p_img_affine)){
		return false;
	}
	
	saveImage(qPrintable(qs_filename_output_path), (unsigned char*)p_img_affine, sz_img_tar, 1);

	printf("8. free the memory. \n");
	if (sz_img_tar) 			{ delete[]sz_img_tar;			sz_img_tar = 0; }
	if (p_img_sub) 			{ delete[]p_img_sub;			p_img_sub = 0; }
	if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }

	return 0;
}