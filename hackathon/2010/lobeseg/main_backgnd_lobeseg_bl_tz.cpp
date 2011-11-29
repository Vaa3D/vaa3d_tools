//main_worm_straightener.cpp
//by Hanchuan Peng
//2008-03-05
//2008-07-12
//Last update by BL 2008-08-22
//2008-08-22: a major revision by Hanchuan Peng
//Last update by BL 2008-08-26 separate mask generating function from do_lobeseg_bdbminus(), add gen_lobeseg_mask()
//Last update Aug 28, 2008 add Ting's shortest path function do_shortestpath()
//Last update by BL Aug 18, 2008 add do_shortestpath_xz and do_lobeseg_bdbminus3()
//Last update by BLAug 19, 2008 add do_shortestpath_cm() 
//last update: by Hanchuan Peng, change the do_shortestpath_cm() function


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "../basic_c_fun/basic_memory.h"
#include "../basic_c_fun/stackutil.h"
#include "../basic_c_fun/volimg_proc.h"
#include "../worm_straighten_c/bdb_minus.h"

#include "lobeseg2.h"
#include "../neurolabi/c/tz_stack_graph.h"

bool convert_type2uint8_3dimg_1dpt(void * &img, V3DLONG * sz, int datatype);

void printHelp ();
void printHelp()
{
	printf("\nUsage: <main prog name> -i <input_image_file> -p <prior control pt file> -o <output_image_file> -c <channalNo_reference> -A <alpha: image force> -B <beta: length force> -G <gamma: smoothness force> -n <nloop> -w <local win radius>\n");
	printf("\t -i <input_image_file>              input 3D image (tif, or Hanchuan's RAW or LSM). \n");
	//printf("\t -p <prior control pt file>         input prior location of control points (note: the order will matter!). If unspecified, then randomly initialized. \n");
	printf("\t -o <output_image_file>             output image where the third channel is a mask indicating the regions. \n");
	printf("\t -c <channalNo_reference>           the ID of channel for processing (starting from 0). If unspecified, then initialized as 0.\n");
	printf("\t -A <alpha>                         the alpha coefficient for the image force. Default = 1.0.\n");
	printf("\t -B <beta>                          the beta coefficient for the length force. Default = 0.5.\n");
	printf("\t -G <gamma>                         the gamma coefficient for the smoothness force Default = 0.5.\n");
	printf("\t -n <nloop>                         the number of maximum loops of optimization. If unspecified, then initialized as 500.\n");
	printf("\t -w <local win radius>              the radius of local window for center of mass estimation. The real win size is 2*radius+1. The default radius = 20. \n");
	printf("\t \n");
	printf("\t [-v]                               verbose printing enabled. \n");
	printf("\t [-h]                               print this message.\n");
	return;
}

#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

int main (int argc, char *argv[])
{
	int b_error = 0;
	
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}

	BDB_Minus_ConfigParameter mypara;
	mypara.f_image = 1;
	mypara.f_smooth = 0.3;
	mypara.f_length = 0.3;
	mypara.radius_x = 20;
	mypara.radius_y = 10;
	mypara.nloops = 500;
	mypara.TH = 0.1;
	
	// Read arguments 

	char *dfile_input = NULL;
	char *dfile_output = NULL;
	int b_verbose_print = 0;
	
	int in_channel_no = 0; //assume the reference channel is 0 
	int out_channel_no = 2; //save the seg mask to the third channel 
	
	int c;
	static char optstring[] = "hvi:o:c:A:B:G:n:";
	opterr = 0;
	while ((c = getopt (argc, argv, optstring)) != -1)
	{
		switch (c)
		{
		case 'h':
			printHelp ();
			return 0;
			break;

		case 'v':
			b_verbose_print = 1;
			break;

		case 'i':
			if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
				b_error=1; return b_error;
			}
			dfile_input = optarg;
			break;

		case 'o':
			if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
				b_error=1; return b_error;
			}
			dfile_output = optarg;
			break;

		case 'c':
			if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
				b_error=1; return b_error;
			}
			in_channel_no = atoi (optarg);
			if (in_channel_no < 0)
			{
				fprintf (stderr, "Illegal channelNo found! It must be >=0.\n");
				b_error=1; return b_error;
			}
			break;

		case 'A':
			if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf (stderr, "Found illegal or NULL parameter for the option -A.\n");
				b_error=1; return b_error;
			}
			mypara.f_image = atof (optarg);
			if (mypara.f_image<0)
			{
				fprintf (stderr, "alpha must not be less than than 0.\n");
				b_error=1; return b_error;
			}
			break;

		case 'B':
			if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf (stderr, "Found illegal or NULL parameter for the option -B.\n");
				b_error=1; return b_error;
			}
			mypara.f_length = atof (optarg);
			if (mypara.f_length<0)
			{
				fprintf (stderr, "beta must not be less than than 0.\n");
				b_error=1; return b_error;
			}
			break;

		case 'G':
			if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf (stderr, "Found illegal or NULL parameter for the option -G.\n");
				b_error=1; return b_error;
			}
			mypara.f_smooth = atof (optarg);
			if (mypara.f_smooth<0)
			{
				fprintf (stderr, "gamma must not be less than than 0.\n");
				b_error=1; return b_error;
			}
			break;

		case 'n':
			if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
			{
				fprintf (stderr, "Found illegal or NULL parameter for the option -n.\n");
				b_error=1; return b_error;
			}
			mypara.nloops = atoi (optarg);
			if (mypara.nloops < 1)
			{
				fprintf (stderr, "The number of loop must be >= 1.\n");
				b_error=1; return b_error;
			}
			break;

			/*	case 'w':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -w.\n");
					b_error=1; return b_error;
				}
				radius = atoi (optarg);
				if (radius < 0)
				{
					fprintf (stderr, "Illeagal radius ! It must be >=0.\n");
					b_error=1; return b_error;
				}
				break;
			 */
		case '?':
			fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
			return 0;

			// default:        abort ();
		}
	}

	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);


	// display the parameter info
	printf("\n-------------------------------------------------\n");
	printf("*** Parameters ***:\n");
	printf("Input  file: [%s]\n", dfile_input);
	printf("Output file: [%s]\n", dfile_output);
	printf("Channel  no: [%d]\n", in_channel_no);
	printf("alpha      : [%7.4f]\n", mypara.f_image);
	printf("beta       : [%7.4f]\n", mypara.f_length);
	printf("gamma      : [%7.4f]\n", mypara.f_smooth);
	//printf("radius     : [%d]\n", radius);
	printf("nloops     : [%d]\n", mypara.nloops);
	printf("Verbose    : [%d]\n", (int)b_verbose_print);
	printf("---------------------------------------------------\n\n");

	// Declare some necessary variables.
	
	unsigned char * img_input = 0; // note that this variable must be initialized as NULL.
	V3DLONG * sz_input = 0; // note that this variable must be initialized as NULL.
	//V3DLONG sz_output[2];
	int datatype_input = 0;

	unsigned char * img_output = 0;
	int * left_bound1d = 0;
	int * right_bound1d = 0;

	vector<vector<Coord2D> > incpt_left;
	vector<vector<Coord2D> > incpt_right;
	
	vector<vector<Coord2D> > incpt_left_high;
	vector<vector<Coord2D> > incpt_right_high;
	vector<vector<Coord2D> > incpt_left_low;
	vector<vector<Coord2D> > incpt_right_low;
	
	vector<int> xz_left;
	vector<int> xz_right;
	vector<int> zz_left;
	vector<int> zz_right;	
	
	if (!dfile_input || !dfile_output)
	{
		fprintf (stderr, "You have not specified any input and output file!. Exit. \n");
		goto Label_exit;
	}
	else
	{
		FILE *tmp_fp = fopen(dfile_input, "r");
		if (!tmp_fp)
		{
			fprintf (stderr, "You have specified an input that does not exist!. Exit. \n");
			goto Label_exit;
		}
		else
		{
			fclose(tmp_fp);
		}
	}

	if (loadImage(dfile_input, img_input, sz_input, datatype_input)!=true)
	{
		fprintf (stderr, "Error happens in reading the input file [%s]. Exit. \n", dfile_input);
		goto Label_exit;
	}

	//check if any one need to be converted as uint8, if so, then do the conversion.
	if (datatype_input!=1)
	{
		if (datatype_input==2 || datatype_input==4)
		{
			fprintf (stdout, "Now try to convert target image datatype from [%d]bytes per pixel to 1byte per pixel.\n", datatype_input);
			convert_type2uint8_3dimg_1dpt((void * &)img_input, sz_input, datatype_input);
			datatype_input=1;
		}
		else
		{
			fprintf (stderr, "The datatype of the target image cannot be converted to UINT8. Exit. \n");
			goto Label_exit;
		}
	}

	printf("Size of input_image = [%ld %ld %ld %ld].\n", sz_input[0], sz_input[1], sz_input[2], sz_input[3]);

	if (in_channel_no >= sz_input[3])
	{
		fprintf (stderr, "The reference channelNo of is invalid (bigger than the number of channels the images have). Exit. \n");
		goto Label_exit;
	}
	
	
	
	//prepare the data space for output
	{
		// 4d output
		
		img_output=0;
		img_output = new unsigned char [(V3DLONG)sz_input[0]*sz_input[1]*sz_input[2]*3];
		
		unsigned char **** img_input_4d=0;
		unsigned char **** img_output_4d=0;
		
		new4dpointer(img_input_4d, sz_input[0], sz_input[1], sz_input[2], sz_input[3], img_input);
		new4dpointer(img_output_4d, sz_input[0], sz_input[1], sz_input[2], sz_input[3], img_output);
		
		
		for (V3DLONG z=0; z<sz_input[2]; z++)
		{
			for (V3DLONG j=0; j<sz_input[1]; j++)
			{
				for (V3DLONG i=0; i<sz_input[0]; i++)
				{
					for (V3DLONG cc=0;cc<2; cc++)
						img_output_4d[cc][z][j][i] = img_input_4d[cc][z][j][i];
				}
			}
		}
		/*
		// 2d xz output
		img_output=0;
		img_output = new unsigned char [(V3DLONG)sz_input[0]*sz_input[2]];
		
		unsigned char ** img_output_2d_xz=0;
		
		new2dpointer(img_output_2d_xz, sz_input[0], sz_input[2], img_output);
				
		for (V3DLONG z=0; z<sz_input[2]; z++)
		{
			for (V3DLONG i=0; i<sz_input[0]; i++);
				//img_output_2d_xz[z][i] = img_input_4d[0][z][100][i];
		}
		*/
		
		if (img_input_4d) {delete4dpointer(img_input_4d, sz_input[0], sz_input[1], sz_input[2], sz_input[3]); img_input_4d=0;}
		if (img_output_4d) {delete4dpointer(img_output_4d, sz_input[0], sz_input[1], sz_input[2], sz_input[3]); img_output_4d=0;}
		//if (img_output_2d_xz) {delete2dpointer(img_output_2d_xz, sz_input[0], sz_input[1]); img_output_2d_xz=0;}
			
		
		int **left_bound=0;
		int **right_bound=0;
		
		left_bound1d=0;
		right_bound1d=0;
		
		try
		{
			left_bound1d = new int [sz_input[2]*sz_input[1]];
			new2dpointer(left_bound, sz_input[1], sz_input[2], left_bound1d);
			right_bound1d = new int [sz_input[2]*sz_input[1]];
			new2dpointer(right_bound, sz_input[1], sz_input[2], right_bound1d);
		}
		catch (...)
		{
			if (left_bound)	delete2dpointer(left_bound, sz_input[1], sz_input[2]);
			if (left_bound1d) {delete []left_bound1d; left_bound1d=0;}
			if (right_bound) delete2dpointer(right_bound, sz_input[1], sz_input[2]);
			if (right_bound1d) {delete []right_bound1d; right_bound1d=0;}
			
			printf("Fail to allocate momery.\n");
			return false;
		}				
	}
	
	//now call the computation program
	in_channel_no = 0, out_channel_no = 2;
	
	// choose the alogirthm
	{
	int lobeseg_bdbminus=0;
	int shortestpath=0;
	int shortestpath_xz=0;
	int shortestpath_cm=1;
		
	if (lobeseg_bdbminus)
	{
		if (!do_lobeseg_bdbminus(img_input, sz_input, in_channel_no, mypara, incpt_left, incpt_right))
		{
			printf("Fail to do the lobe seg correctly.\n");
			b_error = 1;
			goto Label_exit;
		}
		
		gen_lobeseg_mask(sz_input, img_output, out_channel_no, incpt_left, incpt_right, left_bound1d, right_bound1d);
	}
	
	if (shortestpath_cm)
	{
		do_shortestpath_cm(img_output, sz_input, in_channel_no, out_channel_no);
	}
	
	if (shortestpath)
	{
		do_shortestpath(img_output, sz_input, in_channel_no, out_channel_no);
	}
	
	
	vector<int> xz_left(sz_input[2], -1);
	vector<int> xz_right(sz_input[2], -1);
	
	if (shortestpath_xz)
	{
		do_shortestpath_xz(img_input, sz_input, img_output, in_channel_no, out_channel_no, xz_left, xz_right, zz_left, zz_right);
						
		do_lobeseg_bdbminus3(img_input, sz_input, in_channel_no, mypara, incpt_left, incpt_right, xz_left, xz_right);
		
		gen_lobeseg_mask(sz_input, img_output, out_channel_no, incpt_left, incpt_right, left_bound1d, right_bound1d);
		
		//do_lobeseg_bdbminus2(img_input, sz_input, in_channel_no, mypara, incpt_left_high, incpt_right_high, incpt_left_low, incpt_right_low, xz_left, xz_right);
		//gen_lobeseg_mask2(sz_input, img_output, out_channel_no, incpt_left_high, incpt_right_high, incpt_left_low, incpt_right_low, left_bound1d, right_bound1d);
	}
	
	// save to output file
	
	if (img_output && sz_input)
	{
		if (saveImage(dfile_output, (const unsigned char *)img_output, sz_input, sizeof(unsigned char))!=true)
		{
			fprintf(stderr, "Error happens in file writing. Exit. \n");
		}
	}	
	printf("The warped image has been saved to the file [%s].\n", dfile_output);
	}
//	if (incpt_left) {delete [] incpt_left; incpt_left=0;}
//	if (incpt_right) {delete [] incpt_right; incpt_right=0;}
		

	// clean all workspace variables
Label_exit:
	if (img_output) {delete [] img_output; img_output=0;}
	//if (img_input) {delete [] img_input; img_input=0;}
	//if (sz_input) {delete [] sz_input; sz_input=0;}
	
	return b_error;
}




bool convert_type2uint8_3dimg_1dpt(void * &img, V3DLONG * sz, int datatype)
{
	if (!img || !sz)
	{
		fprintf(stderr, "The input to convert_type2uint8_3dimg_1dpt() are invalid [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (datatype!=2 && datatype!=4)
	{
		fprintf(stderr, "This function convert_type2uint8_3dimg_1dpt() is designed to convert 16 bit and single-precision-float only. [%s][%d] \n", __FILE__, __LINE__);
		return false;
	}

	if (sz[0]<1 || sz[1]<1 || sz[2]<1 || sz[3]<1 || sz[0]>2048 || sz[1]>2048 || sz[2]>300 || sz[3]>3)
	{
		fprintf(stderr, "Input image size is not valid. [%s][%d]\n", __FILE__, __LINE__);
		return false;
	}

	V3DLONG totalunits = sz[0] * sz[1] * sz[2] * sz[3];
	unsigned char * outimg = new unsigned char [totalunits];
	if (!outimg)
	{
		fprintf(stderr, "Fail to allocate memory. [%s][%d].\n", __FILE__, __LINE__);
		return false;
	}

	if (datatype==2)
	{
		unsigned short int * tmpimg = (unsigned short int *)img;
		for (V3DLONG i=0;i<totalunits;i++)
		{
			outimg[i] = (unsigned char)(tmpimg[i]>>4); //as I knew it is 12-bit instead of 16 bit
			//note: 071120: seem because I did not read 12-bit data from LSM, the pointer operation tmpimg[i] will crash!!.
			//Thus as of today (071120 I temperarily pause to deal with 12-bit data)
			//080302: change /16 to >>4
		}
	}
	else
	{
		float * tmpimg = (float *)img;
		for (V3DLONG i=0;i<totalunits;i++)
		{
			outimg[i] = (unsigned char)(tmpimg[i]*255); //as I knew it is float between 0 and 1
		}
	}

	//copy to output data

	delete [] ((unsigned char *)img); //as I know img was originally allocated as (unsigned char *)
	img = outimg;

	return true;
}


vector<Coord2D> readControlPointFile2d(string posFile)
{
	vector<Coord2D> coordPos;
	Coord2D c2d;

	char curline[2000];
	ifstream file_op;
	file_op.open(posFile.c_str());
	if (!file_op)
	{
		fprintf(stderr, "Fail to open the pos file [%s]\n", posFile.c_str());
		return coordPos;
	}

	V3DLONG xpos, ypos;  xpos=ypos=-1;//set as default
	V3DLONG k=0;
	while(!file_op.eof())
	{
		file_op.getline(curline, 2000);
		//cout<<curline<<endl;
		k++;
		if (k>0) //ignore the first line
		{
			sscanf(curline, "%ld,%ld", &xpos, &ypos);
			if (xpos==-1 || ypos==-1)
			{
				continue;
			}
			else
			{
				c2d.x = xpos;
				c2d.y = ypos;
				coordPos.push_back(c2d);
			}
			xpos=ypos=-1; //reset to default
		}
	}
	file_op.close();

	return coordPos;
}

vector<Coord3D> readControlPointFile3d(string posFile)
{
	vector<Coord3D> coordPos;
	Coord3D c3d;

	char curline[2000];
	ifstream file_op;
	file_op.open(posFile.c_str());
	if (!file_op)
	{
		fprintf(stderr, "Fail to open the pos file [%s]\n", posFile.c_str());
		return coordPos;
	}

	V3DLONG xpos, ypos, zpos;  xpos=ypos=zpos=-1;//set as default
	V3DLONG k=0;
	while(!file_op.eof())
	{
		file_op.getline(curline, 2000);
		//cout<<curline<<endl;
		k++;
		if (k>0) //ignore the first line
		{
			sscanf(curline, "%ld,%ld,%ld", &xpos, &ypos, &zpos);
			if (xpos==-1 || ypos==-1 || zpos==-1)
			{
				continue;
			}
			else
			{
				c3d.x = xpos;
				c3d.y = ypos;
				c3d.z = zpos;
				coordPos.push_back(c3d);
			}
			xpos=ypos=zpos=-1; //reset to default
		}
	}
	file_op.close();

	return coordPos;
}

