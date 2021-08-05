//main_warp_from_DF.cpp
//affine+TPS transform image based on given control pointsets and displace field
//
// by Lei Qu
//2019-03-17


#include <stdio.h>
#include "getopt.h"
#include <vector>
#include <math.h>
#include <io.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
using namespace std;

#define GOOGLE_GLOG_DLL_DECL           
#define GLOG_NO_ABBREVIATED_SEVERITIES // 没这个编译会出错,传说因为和Windows.h冲突
#include "glog/logging.h"
//将printf的内容同时输出到console和glog日志文件
//注释掉printf(format, ##__VA_ARGS__)可仅打印到log
//全部注释掉可仅打印到console
//将宏函数体放在“do-while(0)”中可避免宏替换时末尾多余；的问题
#define printf(format, ...) do { \
	printf(format, ##__VA_ARGS__); \
	char buf[200]; \
	sprintf(buf, format, ##__VA_ARGS__); \
	LOG(INFO) << buf; \
} while (0)

#include <math.h>
#define WANT_STREAM
#include "../../v3d_main/jba/newmat11/newmatap.h"
#include "../../v3d_main/jba/newmat11/newmatio.h"

#include"q_warp_common.h"
#include"q_TPSInv.h"
#include "stackutil.h"
#include "basic_surf_objs.h"
#include "basic_memory.cpp"
#include "q_imresize.cpp"
#include "q_warp_affine.h"
#include "q_warp_stps.h"
#include "q_imgwarp_tps_quicksmallmemory.h"
#include "q_medianfilter.cpp"
#include "q_convolve_kernel.h"
#include "q_convolve.cpp"
//#include <Eigen/Core>
//#include <iostream>
//#include <LBFGS.h>
//using Eigen::VectorXd;
//using namespace LBFGSpp;

void printHelp();
bool q_img_axis_swap(unsigned char *&p_img_inplace, long long *sz_img_inplace);
bool q_swc_axis_swap(NeuronTree &nt, long long *sz_img_input);

int main(int argc, char *argv[])
{
	//开启日志
	FLAGS_max_log_size = 500;		//设置日志文件大小上限为500M
	google::InitGoogleLogging("");	//初始化glog模块，参数用""即可
	google::SetLogDestination(google::GLOG_INFO, "./log/info_");//设置日志输出目录和前缀
	//google::SetLogDestination(google::GLOG_ERROR, "./log/error_");
	printf("Initialize Glog done!\n");

	if (argc <= 1)
	{
		printHelp();
		return 0;
	}

	QString qs_filename_img_tar = NULL;//only for get target size
	QString qs_filename_img_sub = NULL;//subject image

	QString qs_filename_marker_tar = NULL;
	QString qs_filename_marker_sub = NULL;
	QString qs_filename_swc_sub = NULL;

	QString qs_filename_affinegrid = NULL;
	int forward_stps = 0;
	//output
	QString qs_filename_img_output = NULL;
	QString qs_filename_swc_output = NULL;

	int c;
	static char optstring[] = "ht:s:g:T:S:w:o:f:m:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring)) != -1)
	{
		switch (c)
		{
		case 'h':
			printHelp();
			return 0;
			break;
		case 't':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			qs_filename_img_tar.append(optarg);
			break;
		case 's':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			qs_filename_img_sub.append(optarg);
			break;
		case 'g':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			qs_filename_affinegrid.append(optarg);
			break;
		case 'T':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -S.\n");
				return 1;
			}
			qs_filename_marker_tar.append(optarg);
			break;
		case 'S':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -a.\n");
				return 1;
			}
			qs_filename_marker_sub.append(optarg);
			break;
		case 'w':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -o.\n");
				return 1;
			}
			qs_filename_swc_sub.append(optarg);
			break;
		case 'm':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -x.\n");
				return 1;
			}
			qs_filename_img_output.append(optarg);
			break;
		case 'o':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -x.\n");
				return 1;
			}
			qs_filename_swc_output.append(optarg);
			break;
		case 'f':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -x.\n");
				return -1;
			}
			forward_stps = atoi(optarg);
			break;
		case '?':
			fprintf(stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
			return 1;
			break;
		}
	}
	//------------------------------------------------------------------------------------------------------------------------------------
	//global inplace varaibles
	unsigned char *p_img_sub = 0;
	long long *sz_img_sub = 0;
	long long *sz_img_tar = 0;
	NeuronTree nt_sub;
	vector<Coord3D_PCM> vec_ctlpt_affine_tar(8, Coord3D_PCM()), vec_ctlpt_affine_sub(8, Coord3D_PCM());
	QList<ImageMarker> ql_marker_tar, ql_marker_sub;

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("0. Read input target image. \n");
	if (!qs_filename_img_tar.isNull())
	{
		unsigned char *p_img_tar = 0;
		int datatype_tar = 0;
		if (!loadImage((char *)qPrintable(qs_filename_img_tar), p_img_tar, sz_img_tar, datatype_tar))
		{
			printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
			return false;
		}
		printf("\t>>read image file [%s] complete.\n", qPrintable(qs_filename_img_tar));
		printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n", sz_img_tar[0], sz_img_tar[1], sz_img_tar[2], sz_img_tar[3]);
		printf("\t\tdatatype: %d\n", datatype_tar);
		if (datatype_tar != 1)
		{
			printf("ERROR: Input image datatype is not UINT8.\n");
			return false;
		}
		if (p_img_tar) 				{ delete[]p_img_tar;			p_img_tar = 0; }
	}
	else
	{
		printf("\t>>ERROR: No target image provided!\n");
		return false;
	}

	printf("1. Read input subject files. \n");
	printf("1-1. Read subject image file. \n");
	if (!qs_filename_img_sub.isNull())
	{
		int datatype_sub = 0;
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
	else
	{
		printf("\t>>ERROR: No subject image provided!\n");
		return false;
	}

	printf("1-2. Read subject swc files. \n");
	{
		nt_sub = readSWC_file(qs_filename_swc_sub);
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2.  Read marker files. \n");
	if (qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
	{
		ql_marker_tar = readMarker_file(qs_filename_marker_tar);
		ql_marker_sub = readMarker_file(qs_filename_marker_sub);
		//printf("\t>>Target: read %d markers from [%s]\n", ql_marker_tar.size(), qPrintable(qs_filename_marker_tar));
		//printf("\t>>Subject:read %d markers from [%s]\n", ql_marker_sub.size(), qPrintable(qs_filename_marker_sub));
	}
	else
	{
		printf("ERROR: at least one marker file is invalid.\n");
		return false;
	}
	//re-formate to vector
	//since marker coordinate starts from 1 instead of 0, need -1 !!!
	vector<Coord3D_PCM> vec_tar, vec_sub;
	long l_minlength = min(ql_marker_tar.size(), ql_marker_sub.size());
	for (long i = 0; i < l_minlength; i++)
	{
		vec_tar.push_back(Coord3D_PCM(ql_marker_tar[i].x, ql_marker_tar[i].y, ql_marker_tar[i].z));
		vec_sub.push_back(Coord3D_PCM(ql_marker_sub[i].x, ql_marker_sub[i].y, ql_marker_sub[i].z));
	}

	//---------------------------------------------------------------------------------
	printf("2-1. Read affine grid swc file. \n");//swcread function fail to read!
	if (!qs_filename_affinegrid.isNull())
	{
		QFile qf(qs_filename_affinegrid);
		if (!qf.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			printf("ERROR: failed in loading affine grid swc files [%s].\n", qPrintable(qs_filename_affinegrid));
			return false;
		}
		vector<QString> vec_lines;
		while (!qf.atEnd())
		{
			char curline[2000];
			qf.readLine(curline, sizeof(curline));

			if (curline[0] == '#' || curline[0] == 'x' || curline[0] == 'X' || curline[0] == '\0') continue;
			//printf("%s", curline);
			vec_lines.push_back(QString(curline));
		}
		if (vec_lines.size() != 16)
		{
			printf("ERROR: some thing wrong in affine grid swc file, should be 16 coordinates.\n");
			return false;
		}
		//parse target and subject points
		QStringList qsl;
		for (unsigned int i = 0; i < 8; i++)
		{
			qsl = vec_lines[i * 2].trimmed().split(" ");
			//for (int i = 0; i < qsl.size(); i++) 	printf("%s\n", qPrintable(qsl[i]));
			vec_ctlpt_affine_tar[i].x = qsl[2].toFloat();
			vec_ctlpt_affine_tar[i].y = qsl[3].toFloat();
			vec_ctlpt_affine_tar[i].z = qsl[4].toFloat();
			qsl = vec_lines[i * 2 + 1].trimmed().split(" ");
			//for (int i = 0; i < qsl.size(); i++) 	printf("%s\n", qPrintable(qsl[i]));
			vec_ctlpt_affine_sub[i].x = qsl[2].toFloat();
			vec_ctlpt_affine_sub[i].y = qsl[3].toFloat();
			vec_ctlpt_affine_sub[i].z = qsl[4].toFloat();
			printf("tar[%.2f,%.2f,%.2f]->sub[%.2f,%.2f,%.2f]\n",
				vec_ctlpt_affine_tar[i].x, vec_ctlpt_affine_tar[i].y, vec_ctlpt_affine_tar[i].z,
				vec_ctlpt_affine_sub[i].x, vec_ctlpt_affine_sub[i].y, vec_ctlpt_affine_sub[i].z);
		}
	}
	//else
	//{
	//	printf("\t>>ERROR: No affine grid swc provided!\n");
	//	return false;
	//}
	
	//------------------------------------------------------------------------------------------
	if (!qs_filename_affinegrid.isNull())
	{
		printf("3. warp image based on affine grid. \n");
		long sz_img_tar[4] = { vec_ctlpt_affine_tar[4].x + 1, vec_ctlpt_affine_tar[2].y + 1, vec_ctlpt_affine_tar[1].z + 1, 1 };
		unsigned char *p_img_sub_affine = 0;
		long long *sz_img_sub_affine = 0;
		NeuronTree nt_sub_affine;
		nt_sub_affine.copy(nt_sub);

		printf("3-1. resize subject image/swc to the same size as target image. \n");
		if (!qs_filename_swc_output.isNull())
		{
			//printf("\t>>resize subject swc from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n", sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_tar[0], sz_img_tar[1], sz_img_tar[2]);
			double d_resizeratio[3];
			for (int i = 0; i < 3; i++)	d_resizeratio[i] = double(sz_img_tar[i]) / double(sz_img_sub[i]);

			for (long long i = 0; i < nt_sub.listNeuron.size(); i++)
			{
				nt_sub_affine.listNeuron[i].x *= d_resizeratio[0];
				nt_sub_affine.listNeuron[i].y *= d_resizeratio[1];
				nt_sub_affine.listNeuron[i].z *= d_resizeratio[2];
			}
			//writeSWC_file("resize.swc", nt_sub_affine);
		}
		if (!qs_filename_img_output.isNull())
		{
			//printf("\t>>resize subject image from size [%ld,%ld,%ld] to [%ld,%ld,%ld]. \n", sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], sz_img_tar[0], sz_img_tar[1], sz_img_tar[2]);
			unsigned char *p_img_tmp = 0;
			long sz_img_old[4] = { sz_img_sub[0], sz_img_sub[1], sz_img_sub[2], 1 };
			if (!q_imresize_3D(p_img_sub, sz_img_old, 1, sz_img_tar, p_img_tmp))
			{
				printf("ERROR: q_imresize_3D() return false!\n");
				return 0;
			}
			if (p_img_sub) 			{ delete[]p_img_sub;			p_img_sub = 0; }
			p_img_sub = p_img_tmp;	p_img_tmp = 0;
			for (unsigned i = 0; i < 4; i++) sz_img_sub[i] = sz_img_tar[i];
		}

		//affine warp subject swc
		if (!qs_filename_swc_output.isNull())
		{
			printf("3-2. affine warp subject swc. \n");
			Matrix x4x4_transmatrix(4, 4);
			if (!q_affine_compute_affinmatrix_3D(vec_ctlpt_affine_sub, vec_ctlpt_affine_tar, x4x4_transmatrix))//	B=T*A
			{
				printf("ERROR: q_affine_compute_affinmatrix_3D() return false!\n");
				return 0;
			}
			Matrix x_ori(4, 1), x_trans(4, 1);
			for (long long i = 0; i < nt_sub_affine.listNeuron.size(); i++)
			{
				x_ori(1, 1) = nt_sub_affine.listNeuron[i].x;
				x_ori(2, 1) = nt_sub_affine.listNeuron[i].y;
				x_ori(3, 1) = nt_sub_affine.listNeuron[i].z;
				x_ori(4, 1) = 1.0;
				x_trans = x4x4_transmatrix*x_ori;
				nt_sub_affine.listNeuron[i].x = x_trans(1, 1) / x_trans(4, 1);
				nt_sub_affine.listNeuron[i].y = x_trans(2, 1) / x_trans(4, 1);
				nt_sub_affine.listNeuron[i].z = x_trans(3, 1) / x_trans(4, 1);
			}
			nt_sub.copy(nt_sub_affine);
			//writeSWC_file("./data/ssd_affine.swc", nt_sub);
		}
		//for (long long i = 0; i < nt_sub.listNeuron.size(); i++)
		//{
		//	printf("[%d]:x=%.3f,y=%.3f,z=%.3f, n=%ld, pn=%ld\n", i,
		//		nt_sub.listNeuron[i].x, nt_sub.listNeuron[i].y, nt_sub.listNeuron[i].z,
		//		nt_sub.listNeuron[i].n, nt_sub.listNeuron[i].pn);
		//}

	}

	//------------------------------------------------------------------------------------------------------------------------------------
	if (!forward_stps)
	{
		//生成初始值抖动搜索场
		//1. 计算图像中所有点到中心点的xyz偏移和距离（棋盘格距离）
		//2. 按照距离排序组成数组xyz偏移数组
		//3. 以iter_redonewton次数为索引从该数组中取值进行偏移
		printf("Generate newton initial point offset vector\n");
		vector<Coord3D_PCM> vec_offset;
		{
			long long l_centerpos_x = long long((sz_img_tar[0] + 1) / 2.0);
			long long l_centerpos_y = long long((sz_img_tar[1] + 1) / 2.0);
			long long l_centerpos_z = long long((sz_img_tar[2] + 1) / 2.0);
			long long l_centerpos_max = max(max(l_centerpos_x, l_centerpos_y), l_centerpos_z) + 1;
			vector<vector<Coord3D_PCM>> vec2d_offset(l_centerpos_max, vector<Coord3D_PCM>());
			Coord3D_PCM offset;
			for (long long x = 0; x < sz_img_tar[0]; x++)
			{
				//printf("%ld, ", x);
				for (long long y = 0; y < sz_img_tar[1]; y++)
					for (long long z = 0; z < sz_img_tar[2]; z++)
					{
						offset.x = x - l_centerpos_x;
						offset.y = y - l_centerpos_y;
						offset.z = z - l_centerpos_z;
						long long dis = max(max(fabs(offset.x), fabs(offset.y)), fabs(offset.z));//棋盘格距离
						vec2d_offset[dis].push_back(offset);
					}
			}
			//printf("\n");
			for (long long i = 0; i < vec2d_offset.size(); i++)
			{
				//printf("dis=%d, npoint=%ld\n", i, vec2d_offset[i].size());
				for (long long j = 0; j < vec2d_offset[i].size(); j++)
					vec_offset.push_back(vec2d_offset[i][j]);
			}
			printf("\t[%ld] offset point genterated\n", vec_offset.size());
		}

		//------------------------------------------------------------------------------------------------------------------------------------
		printf("4. sTPS transform the subject image and swc to target. \n");
		{
			Matrix x4x4_affine, xnx4_c, xnxn_K;
			printf("\t4-1: stps warp subject image...\n");
			{
				unsigned char *p_img_stps = 0;
				/*	if(!q_imagewarp_stps(vec_tar, vec_sub, p_img_sub, sz_img_sub, sz_img_tar, x4x4_affine, xnx4_c, xnxn_K, p_img_stps))
					{
					printf("ERROR: q_imagewarp_stps() return false.\n");
					return false;
					}*/

				V3DLONG szBlock_x, szBlock_y, szBlock_z;
				szBlock_x = szBlock_y = szBlock_z = 4;
				int		i_interpmethod_df = 1;		//default B-spline
				int		i_interpmethod_img = 0;		//default trilinear
				if (!imgwarp_smallmemory(p_img_sub, sz_img_sub, ql_marker_sub, ql_marker_tar, szBlock_x, szBlock_y, szBlock_z, i_interpmethod_df, i_interpmethod_img, p_img_stps, x4x4_affine, xnx4_c, xnxn_K))
				{
					printf("ERROR: imgwarp_smallmemory() return false.\n");
					return false;
				}

				if (!qs_filename_img_output.isNull())
					saveImage(qPrintable(qs_filename_img_output), p_img_stps, sz_img_tar, 1);

				if (p_img_stps) 		{ delete[]p_img_stps;			p_img_stps = 0; }
			}

			printf("\t4-2: stps warp subject swc (Newton's optimiazation)...\n");
			{
				printf("\t\tcompute the forward mapped position for each target pixel...\n");
				//compute the forward mapped position for each target pixel (tar-->sub)
				//for finding the nearest mapped pos to current subject pos 
				//and then use its corresponding target pos as initial pos for Newton's optimization
				vector<vector<vector<Coord3D_PCM>>> vec3d_tar2sub_pos;
				vec3d_tar2sub_pos.assign(sz_img_tar[0], vector<vector<Coord3D_PCM>>(sz_img_tar[1], vector<Coord3D_PCM>(sz_img_tar[2], Coord3D_PCM())));
#pragma omp parallel for
				for (long long x = 0; x < sz_img_tar[0]; x++)
				{
					//printf("\tSTPS: [%d/%d]\n", sz_img_tar[0], x);
					for (long long y = 0; y < sz_img_tar[1]; y++)
						for (long long z = 0; z < sz_img_tar[2]; z++)
						{
							Coord3D_PCM pt_tar, pt_tar2sub;
							pt_tar.x = x;	pt_tar.y = y;	pt_tar.z = z;

							if (!q_compute_ptwarped_from_stps_3D(pt_tar, vec_tar, x4x4_affine, xnx4_c, pt_tar2sub))
							{
								printf("ERROR: q_compute_ptwarped_from_stps_3D() return false.\n");
								//return false;
							}

							vec3d_tar2sub_pos[x][y][z].x = pt_tar2sub.x;
							vec3d_tar2sub_pos[x][y][z].y = pt_tar2sub.y;
							vec3d_tar2sub_pos[x][y][z].z = pt_tar2sub.z;
						}
				}


				printf("\t\tfind target pos that can best approach to given subject pos by using Newton's optimization...\n");
				//setup Newton's optimizaer
				const int n = 3;
				// Set up paramete
				LBFGSParam<double> param;
				param.epsilon = 1e-5;
				param.max_iterations = 100;//100
				// Create solver and function object
				LBFGSSolver<double> solver(param);
				TPSInv fun(n);
				fun.setTPSPara(x4x4_affine, xnx4_c, vec_tar);

				//warp swc
				for (long long i = 0; i < nt_sub.listNeuron.size(); i++)
				{
					//待优化的节点坐标
					Coord3D_PCM pt_sub;
					pt_sub.x = nt_sub.listNeuron[i].x;	pt_sub.y = nt_sub.listNeuron[i].y;	pt_sub.z = nt_sub.listNeuron[i].z;

					//找到距离当前neuron节点最近的tar2sub映射点(为缩小搜索范围，此时tar和sub应已经仿射配准)
					//如果mindis较大的话则增加搜索半径1次
					int radius_search = 10;
					float mindis = radius_search * 3;
					int mindis_x = 0, mindis_y = 0, mindis_z = 0;
					for (int j = 0; j < 2; j++)
					{
						if (j == 1) radius_search = 100;

						for (long long x = pt_sub.x - radius_search; x < pt_sub.x + radius_search; x++)
							for (long long y = pt_sub.y - radius_search; y < pt_sub.y + radius_search; y++)
								for (long long z = pt_sub.z - radius_search; z < pt_sub.z + radius_search; z++)
								{
									if (x <= 0 || x > sz_img_tar[0] - 1 ||
										y <= 0 || y > sz_img_tar[1] - 1 ||
										z <= 0 || z > sz_img_tar[2] - 1)
										continue;

									double x_dif = pt_sub.x - vec3d_tar2sub_pos[x][y][z].x;
									double y_dif = pt_sub.y - vec3d_tar2sub_pos[x][y][z].y;
									double z_dif = pt_sub.z - vec3d_tar2sub_pos[x][y][z].z;
									double dis = sqrt(x_dif*x_dif + y_dif*y_dif + z_dif*z_dif);
									if (dis < mindis)
									{
										mindis = dis;
										mindis_x = x; mindis_y = y, mindis_z = z;
									}
								}

						//printf("[%d]: mindis=%f\n", i, mindis);
						if (mindis < 1) break;
					}

					//find target pos that can best approach to given subject pos by using Newton's optimization
					double loss_min = 1e+6;		Coord3D_PCM pt_inv_min;//记录loss最小的位置(以便优化失败时使用)
					long long iter_redonewton = 0;//优化失败的次数
					bool b_find = false;
					while (!b_find)
					{
						Coord3D_PCM pt_inv;

						// set subject position need to be approximated
						fun.setTargetPos(Coord3D_PCM(pt_sub.x, pt_sub.y, pt_sub.z));//neuron node position
						// set initial guess position in target
						VectorXd x = VectorXd::Zero(n);
						x[0] = mindis_x; x[1] = mindis_y; x[2] = mindis_z;
						//if failed, perturb initial guess position (to avoid trapped in local minimal)
						Coord3D_PCM offset;
						////Method1:
						//if (iter_redonewton >= 1)
						//{
						//	srand((int)time(0) + iter_redonewton);
						//	offset.x = (rand() % 1000 / 1000.0);	//[0~1]
						//	offset.y = (rand() % 1000 / 1000.0);	//[0~1]
						//	offset.z = (rand() % 1000 / 1000.0);	//[0~1]
						//	if (iter_redonewton <= 500)
						//	{
						//		offset.x *= 1; offset.y *= 1; offset.z *= 1;
						//	}
						//	else if (iter_redonewton <= 1000)
						//	{
						//		offset.x *= 2; offset.y *= 2; offset.z *= 2;
						//	}
						//	else
						//	{
						//		offset.x *= 5; offset.y *= 5; offset.z *= 5;
						//	}
						//	x[0] = mindis_x + offset.x; x[1] = mindis_y + offset.y; x[2] = mindis_z + offset.z;
						//}
						//Method2:
						offset.x = vec_offset[iter_redonewton].x;
						offset.y = vec_offset[iter_redonewton].y;
						offset.z = vec_offset[iter_redonewton].z;
						x[0] = mindis_x + offset.x; x[1] = mindis_y + offset.y; x[2] = mindis_z + offset.z;

						// x will be overwritten to be the best point found
						double fx = 0.0;
						int niter = solver.minimize(fun, x, fx);
						pt_inv.x = x[0]; pt_inv.y = x[1]; pt_inv.z = x[2];
						if (fx < loss_min)
						{
							loss_min = fx;
							pt_inv_min.x = pt_inv.x;	pt_inv_min.y = pt_inv.y;	pt_inv_min.z = pt_inv.z;
						}

						if (i % 100 == 0 && iter_redonewton == 0)
							printf("[%ld/%ld]: sub[%.3f,%.3f,%.3f]<--inv2tar[%.3f,%.3f,%.3f], loss=%e\n", i, nt_sub.listNeuron.size(),
							pt_sub.x, pt_sub.y, pt_sub.z, pt_inv.x, pt_inv.y, pt_inv.z, fx);

						//for validation: compute distance to parent node 
						//(should be very small, but in area with big skrew, it can also be big!)
						double d_dis2parent = 0;
						{
							long long l_pn = nt_sub.listNeuron[i].pn;
							if (l_pn >= 0)
							{
								double x_dif = pt_inv.x - nt_sub.listNeuron[l_pn - 1].x;//since node number start from 1 instead of 0, so pn-1
								double y_dif = pt_inv.y - nt_sub.listNeuron[l_pn - 1].y;
								double z_dif = pt_inv.z - nt_sub.listNeuron[l_pn - 1].z;
								d_dis2parent = sqrt(x_dif*x_dif + y_dif*y_dif + z_dif*z_dif);
							}
						}

						//success in finding minimal
						if (!isnan(fx) && fx < 1.0 /*&& d_dis2parent < 5*/)
						{
							b_find = true;
							nt_sub.listNeuron[i].x = pt_inv.x;
							nt_sub.listNeuron[i].y = pt_inv.y;
							nt_sub.listNeuron[i].z = pt_inv.z;
							if (iter_redonewton > 0)//error to success
								printf("\tSuccess: iter[%d]: loss=%e, dis2parent=%.3f, mindis=%.3f, radius=%d, offset[%.3f,%.3f,%.3f]\n",
								iter_redonewton, fx, d_dis2parent, mindis, radius_search, offset.x, offset.y, offset.z);
						}
						//failed (modify the initial position and redo optimization)
						else
						{
							if (iter_redonewton == 0)
								printf("WARNNING:[%ld/%ld]:n=%ld,mindis=%.3f,radius=%d,loss=%e\n",
								i, nt_sub.listNeuron.size(), nt_sub.listNeuron[i].n, mindis, radius_search, fx);
							if (isnan(fx))
								printf("\titer[%d]: loss=NaN, dis2parent=%.3f, offset[%.3f,%.3f,%.3f]\n",
								iter_redonewton, d_dis2parent, offset.x, offset.y, offset.z);
							else if (fx > 1.0 /*|| d_dis2parent > 5*/)
								printf("\titer[%d]: loss=%.3f, dis2parent=%.3f, offset[%.3f,%.3f,%.3f]\n",
								iter_redonewton, fx, d_dis2parent, offset.x, offset.y, offset.z);

							//如果超过指定迭代阈值，则取loss最小的点
							long long iter_thresh = 10000;
							if (iter_redonewton > iter_thresh)
							{
								printf("Try %d times, still cannot find minima, set to pos with min loss!\n", iter_thresh);
								nt_sub.listNeuron[i].x = pt_inv_min.x;
								nt_sub.listNeuron[i].y = pt_inv_min.y;
								nt_sub.listNeuron[i].z = pt_inv_min.z;
								b_find = true;//避免死循环
							}
							iter_redonewton++;
						}
					}//while (!b_find)
				}//for (long long i = 0; i < nt_sub.listNeuron.size(); i++)

				QString outputname = qs_filename_swc_output;
				writeSWC_file(outputname, nt_sub);
			}//printf("\t4-2:)
		}//printf("4. sTPS transform the subject image and swc to target. \n");
	}
	else
	{
		printf("4.3  STPS swc warp.   \n");
		Matrix x4x4_affine, xnx4_c, xnxn_K;
		if (!q_stps_cd(vec_sub, vec_tar, 0.2, x4x4_affine, xnx4_c, xnxn_K))
		{
			printf("ERROR: q_stps_cd() return false.\n");
		}
		printf("4.3  STPS swc warp.   \n");
		for (long long i = 0; i < nt_sub.listNeuron.size(); i++)
		{
			printf("STPS swc: [%d/%d]\n", nt_sub.listNeuron.size(), i);
			Coord3D_PCM spt_sub, spt_sub2tar;
			spt_sub.x = nt_sub.listNeuron[i].x;
			spt_sub.y = nt_sub.listNeuron[i].y;
			spt_sub.z = nt_sub.listNeuron[i].z;
			
			if (!q_compute_ptwarped_from_stps_3D(spt_sub, vec_sub, x4x4_affine, xnx4_c, spt_sub2tar))
			{
				printf("ERROR: q_compute_ptwarped_from_tpspara_3D() return false.\n");
				return false;
			}
			nt_sub.listNeuron[i].x = spt_sub2tar.x;
			nt_sub.listNeuron[i].y = spt_sub2tar.y;
			nt_sub.listNeuron[i].z = spt_sub2tar.z;
		}
		nt_sub.copy(nt_sub);
		writeSWC_file(qPrintable(qs_filename_swc_output), nt_sub);
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. free memory. \n");
	if (sz_img_tar) 			{ delete[]sz_img_tar;			sz_img_tar = 0; }
	if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
	if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }

	//停止GLog日志,与InitGoogleLogging()成对使用
	printf("OnDestroy() exit success!\n");
	google::ShutdownGoogleLogging();

	printf("Program exit success.\n");
	return true;
}

//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_warp_affine_tps\n");
	printf("Input paras:\n");
	printf("\t  -t   <imageFilename_target>	input target image file full name.\n");
	printf("\t  -s   <imageFilename_subject>	input subject image file full name.\n");
	printf("\t  -T   <filename_tar_marker>	            input target (marker) file full name.\n");
	printf("\t	-S  <filename_sub_marker>  input subject (marker) file full name.\n");
	printf("\t	-w  <filename_swc>     input swc file full name.\n");
	printf("Output paras:\n");
	printf("\t [-m]  <filename_out_img>     output img file full name.\n");
	printf("\t [-o]  <filename_out_swc>     output swc file full name.\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}

