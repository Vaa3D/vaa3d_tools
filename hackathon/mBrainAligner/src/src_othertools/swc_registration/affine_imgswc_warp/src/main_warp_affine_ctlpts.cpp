//main_warp_from_DF.cpp
//affine transform image based on given control pointsets and displace field
//
// by Lei Qu
//2019-03-17

#include <stdio.h>
#include "getopt.h"
#include <vector>
using namespace std;

#include <math.h>
#define WANT_STREAM
#include "../../v3d_main/jba/newmat11/newmatap.h"
#include "../../v3d_main/jba/newmat11/newmatio.h"

#include"q_warp_common.h"
#include "stackutil.h"
#include "basic_surf_objs.h"
#include "basic_memory.cpp"
#include "q_warp_affine.h"


void printHelp();

int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp();
		return 0;
	}

	//input
	QString qs_filename_img_tar = NULL;//only for get target size
	QString qs_filename_img_sub = NULL;//subject image

	QString qs_filename_marker_tar = NULL;
	QString qs_filename_marker_sub = NULL;
	QString qs_filename_swc_sub = NULL;

	//output
	QString qs_filename_img_output = NULL;
	QString qs_filename_swc_output = NULL;

	int c;
	static char optstring[] = "ht:s:T:S:w:o:m:";
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
		case '?':
			fprintf(stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
			return 1;
			break;
		}
	}

	bool b_warpimg = 0, b_warpswc = 0;
	if (!qs_filename_img_output.isEmpty()) 		b_warpimg = 1;
	if (!qs_filename_swc_sub.isEmpty()) 		b_warpswc = 1;
	//------------------------------------------------------------------------------------------------------------------------------------
	//global inplace varaibles
	unsigned char *p_img_sub = 0;
	long long *sz_img_sub = 0;
	long long *sz_img_tar = 0;
	NeuronTree nt_sub;
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

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2.  Read marker files. \n");
	if (qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
	{
		ql_marker_tar = readMarker_file(qs_filename_marker_tar);
		ql_marker_sub = readMarker_file(qs_filename_marker_sub);
		printf("\t>>Target: read %d markers from [%s]\n", ql_marker_tar.size(), qPrintable(qs_filename_marker_tar));
		printf("\t>>Subject:read %d markers from [%s]\n", ql_marker_sub.size(), qPrintable(qs_filename_marker_sub));
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

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. Affine transform the subject image and swc to target. \n");
	{
		Matrix x4x4_affinematrix;
		printf("\t3-1: affine warp subject image...\n");
		{
			unsigned char *p_img_warp_affine = 0;
			if (!q_imagewarp_affine(vec_tar, vec_sub,
				p_img_sub, sz_img_sub, sz_img_tar,
				x4x4_affinematrix, p_img_warp_affine))
			{
				printf("ERROR: q_imagewarp_affine() return false!\n");
				return 0;
			}
			if (p_img_sub) 			{ delete[]p_img_sub;			p_img_sub = 0; }
			p_img_sub = p_img_warp_affine;	p_img_warp_affine = 0;
			for (int i = 0; i < 3; i++) sz_img_sub[i] = sz_img_tar[i];

			if (b_warpimg)
			{
				QString outputname = qs_filename_img_output;
				saveImage(qPrintable(outputname), p_img_sub, sz_img_tar, 1);
			}
		}
		if (b_warpswc)
		{

			printf("\t3-1. Read subject swc files. \n");
			{
				nt_sub = readSWC_file(qs_filename_swc_sub);
			}

			printf("\t3-2: affine warp swc...\n");
			{
				//warping direction is inverse to image
				Matrix x_ori(4, 1), x_trans(4, 1);
				for (long long i = 0; i < nt_sub.listNeuron.size(); i++)
				{
					x_ori(1, 1) = nt_sub.listNeuron[i].x;
					x_ori(2, 1) = nt_sub.listNeuron[i].y;
					x_ori(3, 1) = nt_sub.listNeuron[i].z;
					x_ori(4, 1) = 1.0;
					x_trans = x4x4_affinematrix.i()*x_ori;
					nt_sub.listNeuron[i].x = x_trans(1, 1) / x_trans(4, 1);
					nt_sub.listNeuron[i].y = x_trans(2, 1) / x_trans(4, 1);
					nt_sub.listNeuron[i].z = x_trans(3, 1) / x_trans(4, 1);
				}
				//QString outputname = "./data/output/sub_affine.swc";
				QString outputname = qs_filename_swc_output;
				writeSWC_file(outputname, nt_sub);
			}
		}
	}
	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. free memory. \n");
	if (sz_img_tar) 			{ delete[]sz_img_tar;			sz_img_tar = 0; }
	if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
	if (sz_img_sub) 			{ delete[]sz_img_sub;			sz_img_sub = 0; }

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
	printf("\t	-S   <filename_sub_marker>  input subject (marker) file full name.\n");
	printf("\t	-w   <filename_swc>     input swc file full name.\n");
	printf("Output paras:\n");
	printf("\t  -m   <filename_out_img>     out img file full name.\n");
	printf("\t  -o  <filename_out_swc>     output swc file full name.\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}

