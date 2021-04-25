//main_srsreg.cpp

// by Lei Qu@20190425

#include<fstream>
//# include "outline_detec.h"
#include"load_data.h"
#include"until.h"
#include"mBrainAligner.h"
#include "Config.h" 


void printHelp();
void printHelp()
{
	printf("\n\n===================================================================================================\n");
	printf("\n");
	printf("\t [-p <input_parameter_file>]         This is .txt file which containing all the parameter Settings .\n");
	printf("\t [-s <input_subject_image>]           .raw/.tif/.lsm file containing 3D subject which will deform/warp to best appriximate the geometry of the template/target. The .raw file is in V3D's RAW format.\n");
	printf("\t [-m <input_segmentation_image>]          This is segmentation file path which containing the segmentation results of the 3D subject image.\n");
	printf("\t [-g <input_dta_file>]          This is  .raw/.tif/.lsm file path containing several 3D images,\
		                                            eg: average_template_25_u8_xpad.v3draw: target image \
													 landmarker_tar.marker,\
													 region_attention.v3draw,\
													 average_sub.marker,average_tar.marker ,edge.v3draw.\n");
	printf("\t [-f <finetune data>]          If you interrupt during iteration registration, you can continue running based on the previous results.\n");
	printf("\t [-S <save result file>]          This file path can save all result .\n");

	printf("\n===================================================================================================\n\n");
	return;
}


int main(int argc, char *argv[])
{
	//-----------------------------------------------------------------------------------------
	
	QString qs_filename_img_sub, qs_filename_img_sub_seg;
	QString fine_filename;
	QString  data_file, resample, save_path;
	QString parameter_path;
	
	if (argc <= 1)
	{
		printHelp();
		return 0;
	}

	int c;
	static char optstring[] = "p:s:m:g:f:S:";
	opterr = 0;
	while ((c = getopt(argc, argv, optstring)) != -1) // can be replaced by getopt_long() in the future
	{
		switch (c)
		{
		case 'p':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			parameter_path = optarg;
			break;

			//for subject image
		case 's':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			qs_filename_img_sub = optarg;
			break;

		case 'm':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			qs_filename_img_sub_seg = optarg;
			break;

	
		case 'g':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -t.\n");
				return 1;
			}
			data_file = optarg;
			break;

		case 'f':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			fine_filename = optarg;
			break;

		case 'S':
			if (strcmp(optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf(stderr, "Found illegal or NULL parameter for the option -s.\n");
				return 1;
			}
			save_path = optarg;
			break;

		case 'h':
			printHelp();
			return 0;
			break;

		case '?':
			fprintf(stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
			return 0;

		default:
			fprintf(stderr, "Illeagal arguments, do nothing.\n", optopt);
			return 0;
		}
	}

	QByteArray parameter_path_ba = parameter_path.toLatin1();
	Config configSettings(parameter_path_ba.data());
	Parameter input_Parameter;
	input_Parameter.Select_modal = configSettings.Read("Select_modal", input_Parameter.Select_modal);
	input_Parameter.iterations_number = configSettings.Read("iterations_number", input_Parameter.iterations_number);
	input_Parameter.star_lamda_outline = configSettings.Read("star_lamda_outline", input_Parameter.star_lamda_outline);
	input_Parameter.kernel_radius = configSettings.Read("kernel_radius", input_Parameter.kernel_radius);
	input_Parameter.search_radius = configSettings.Read("search_radius", input_Parameter.search_radius);
	input_Parameter.fre_save = configSettings.Read("fre_save", input_Parameter.fre_save);
	input_Parameter.fre_region_constraint = configSettings.Read("fre_region_constraint", input_Parameter.fre_region_constraint);
	input_Parameter.fre_global_constraint = configSettings.Read("fre_global_constraint", input_Parameter.fre_global_constraint);
	input_Parameter.star_iter = configSettings.Read("star_iter", input_Parameter.star_iter);
	input_Parameter.lam_end_out = configSettings.Read("lam_end_out", input_Parameter.lam_end_out);
	input_Parameter.lam_end_inner = configSettings.Read("lam_end_inner", input_Parameter.lam_end_inner);
	input_Parameter.resample = configSettings.Read("resample", input_Parameter.resample);
	input_Parameter.star_lamda_inner = configSettings.Read("star_lamda_inner", input_Parameter.star_lamda_inner);
	input_Parameter.save_path = save_path;
	
	//-----------------------------------------------------------------------------------------
	// loda image data 
	//-----------------------------------------------------------------------------------------
	printf("\n 1. load image data. \n");
	unsigned char   *p_img_sub = 0;
	float *p_img32f_tar = 0, *p_img32f_sub_bk = 0, *p_img32_sub_label=0;
	long long * sz_img;

	if (!loadImageData(input_Parameter,data_file, qs_filename_img_sub, p_img_sub, p_img32f_tar, p_img32f_sub_bk, p_img32_sub_label, sz_img))
	{
		printf("ERROR:loadImageData().\n");
		return false;
	}
	
	float  ****p_img32f_tar_4d = 0, ****p_img32f_sub_4d = 0, ****p_img_label_4d = 0;
	new4dpointer(p_img32f_tar_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img32f_tar);
	new4dpointer(p_img32f_sub_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img32f_sub_bk);
	if (input_Parameter.Select_modal < 2)
	{
		new4dpointer(p_img_label_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3], p_img32_sub_label);
	}
	

	//-----------------------------------------------------------------------------------------
	// load landmarks
	//-----------------------------------------------------------------------------------------
	printf("\n 2. load landmarks data and deep feature. \n");
	vector<point3D64F> vec_corners, fine_sub_corner, aver_corner;
	vector<int> region_label;
	map <int, float *> density_map_sub;

	float *fmost_label_edge = 0;
	float  ****fmost_label_edge_4d = 0;

	if(!LoadLandmarksData(vec_corners, fine_sub_corner, aver_corner, region_label, data_file, fine_filename,sz_img, input_Parameter, p_img_label_4d,
		qs_filename_img_sub_seg, density_map_sub, fmost_label_edge,fmost_label_edge_4d))
	{
		printf("ERROR: LoadLandmarksData().\n");
		return false;
	}
	if (fmost_label_edge) 		{ delete[]fmost_label_edge;		fmost_label_edge = 0; }
	if (fmost_label_edge_4d)		{ delete4dpointer(fmost_label_edge_4d, sz_img[0], sz_img[1], sz_img[2], sz_img[3]); }



	printf("============================================================================================ \n");	
	if (input_Parameter.Select_modal==0)
	    printf(" Registration modal: fMost mouse to CCF \n");
	else if(input_Parameter.Select_modal == 1)
		printf(" Registration modal: other modal mouse to CCF \n");
	else if (input_Parameter.Select_modal == 2)
		printf(" Registration modal: zebra fish registration \n");
	printf("Load %d target landmarks. \n", vec_corners.size());
	printf("Load %d subject landmarks. \n", fine_sub_corner.size());
	printf("The iterations_number: %d. \n", input_Parameter.iterations_number);
	printf("mul_scale_mBrainAligner: %d. \n", input_Parameter.resample);
	printf("save_file_path:%s \n",qPrintable(save_path));
	printf("============================================================================================ \n");


	//-----------------------------------------------------------------------------------------
	printf("\n 3. Do Local registration. \n");
	//-----------------------------------------------------------------------------------------
		
	if (!mul_scale_mBrainAligner(input_Parameter, vec_corners, fine_sub_corner, aver_corner, region_label, sz_img, p_img32f_tar, p_img32f_sub_bk,
		p_img32_sub_label, p_img_sub, density_map_sub))
	{
		printf("ERROR: mul_scale_mBrainAligner().\n");
		return false;
	}

	//-----------------------------------------------------------------------------------------
	printf("*. free memory. \n");
	
	if (p_img32f_sub_bk) 		{ delete[]p_img32f_sub_bk;		p_img32f_sub_bk = 0; }
	if (p_img32f_tar) 			{ delete[]p_img32f_tar;			p_img32f_tar = 0; }
	if (p_img32f_sub_bk) 		{ delete[]p_img32f_sub_bk;		p_img32f_sub_bk = 0; }
	if (p_img32_sub_label) 		{ delete[]p_img32_sub_label;		p_img32_sub_label = 0; }
	if (p_img_sub) 				{ delete[]p_img_sub;			p_img_sub = 0; }
	
	printf("Program exit success.\n");
	return true;
}
