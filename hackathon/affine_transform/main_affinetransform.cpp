//main_affinetransform.cpp
//affine transfer the image based on the given point sets
// by Lei Qu
//2010-03-17

#include <QtGui>
#include <stdio.h>
#include <unistd.h>
extern char *optarg;
extern int optind, opterr;
#include <vector>
using namespace std;
#define WANT_STREAM
#include "../../v3d_main/jba/newmat11/newmatap.h"
#include "../../v3d_main/jba/newmat11/newmatio.h"
#include "../../v3d_main/basic_c_fun/stackutil.h"
#include "../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include "../../v3d_main/basic_c_fun/basic_memory.cpp"//note: should not include .h file, since they are template functions
#include "../../v3d_main/jba/c++/convert_type2uint8.h"


class Coord3D_PCM
{
public:
	double x,y,z;
	Coord3D_PCM(double x0,double y0,double z0) {x=x0;y=y0;z=z0;}
	Coord3D_PCM() {x=y=z=0;}
};

void printHelp();
//convert input image from 16bit to 8bit and rescale to [0~255]
bool q_imgconvertscale_16bit_2_8bit(unsigned char *&p_img,const long *sz_img);
//centrilize and scale the point set
bool q_normalize_points_2D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x3x3_normalize);
bool q_normalize_points_3D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x4x4_normalize);
//compute the affine matraix
//B=T*A
bool q_affine_compute_affinmatrix_2D(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,Matrix &x3x3_affinematrix);
bool q_affine_compute_affinmatrix_3D(const vector<Coord3D_PCM> &arr_A,const vector<Coord3D_PCM> &arr_B,Matrix &x4x4_affinematrix);


int main(int argc, char *argv[])
{
	if(argc<=1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//read arguments
//	QString qs_filename_marker_tar	="/Volumes/MyBook/Wayne/clones/brain_003.tif.marker";
//	QString qs_filename_marker_sub	="/Volumes/MyBook/Wayne/clones/brain_001.tif.marker";
//	QString qs_filename_img_sub		="/Volumes/MyBook/Wayne/clones/brain_001.tif";
//	QString qs_filename_img_sub2tar	="sub2tar.tif";
//	int 	n_how2affine			=0;
//	long 	sz_img_output[4]		={957,957,149,3};//whzc
	QString qs_filename_marker_tar	=NULL;
	QString qs_filename_marker_sub	=NULL;
	QString qs_filename_img_sub		=NULL;
	QString qs_filename_img_sub2tar	=NULL;
	int 	n_how2affine			=0;//0:estimate affine only based on the XY information of pointset, 1: based on the XYZ information
	long 	sz_img_output[4]		={0,0,0,0};//whzc

	int c;
	static char optstring[]="ht:s:S:o:f:W:H:Z:";
	opterr=0;
	while((c=getopt(argc,argv,optstring))!=-1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;
			case 't':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				qs_filename_marker_tar.append(optarg);
				break;
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				qs_filename_marker_sub.append(optarg);
				break;
			case 'S':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -S.\n");
					return 1;
				}
				qs_filename_img_sub.append(optarg);
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				qs_filename_img_sub2tar.append(optarg);
				break;
			case 'f':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -f.\n");
					return 1;
				}
				n_how2affine=atoi(optarg);
				break;
			case 'W':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -W.\n");
					return 1;
				}
				sz_img_output[0]=atoi(optarg);
				break;
			case 'H':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -H.\n");
					return 1;
				}
				sz_img_output[1]=atoi(optarg);
				break;
			case 'Z':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -Z.\n");
					return 1;
				}
				sz_img_output[2]=atoi(optarg);
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
    }

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>Image affine transformation:\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
    printf(">>  input target  marker/swc:          %s\n",qPrintable(qs_filename_marker_tar));
    printf(">>  input subject markerswc:          %s\n",qPrintable(qs_filename_marker_sub));
	printf(">>  input subject image:           %s\n",qPrintable(qs_filename_img_sub));
	printf(">>  input how2affine:              %d\n",n_how2affine);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output sub2tar image:          %s\n",qPrintable(qs_filename_img_sub2tar));
	printf(">>  output image size:             [%d,%d,%d]\n",sz_img_output[0],sz_img_output[1],sz_img_output[2]);
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	//------------------------------------------------------------------------------------------------------------------------------------
    printf("1. Read target and subject marker/swc files. \n");
	QList<ImageMarker> ql_marker_tar,ql_marker_sub;
    vector<Coord3D_PCM> vec_tar,vec_sub;
    NeuronTree ql_swc_tar,ql_swc_sub;
	if(qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
    {
		ql_marker_tar=readMarker_file(qs_filename_marker_tar);
		ql_marker_sub=readMarker_file(qs_filename_marker_sub);
    	printf("\t>>read %d markers from [%s]\n",ql_marker_tar.size(),qPrintable(qs_filename_marker_tar));
    	printf("\t>>read %d markers from [%s]\n",ql_marker_sub.size(),qPrintable(qs_filename_marker_sub));
        for(long i=0;i<ql_marker_tar.size();i++)
        {
            vec_tar.push_back(Coord3D_PCM(ql_marker_tar[i].x,ql_marker_tar[i].y,ql_marker_tar[i].z));
        }
        for(long i=0;i<ql_marker_sub.size();i++)
        {
            vec_sub.push_back(Coord3D_PCM(ql_marker_sub[i].x,ql_marker_sub[i].y,ql_marker_sub[i].z));
        }
    }
    else if(qs_filename_marker_tar.endsWith(".swc") && qs_filename_marker_sub.endsWith(".swc"))
    {
        ql_swc_tar=readSWC_file(qs_filename_marker_tar);
        ql_swc_sub=readSWC_file(qs_filename_marker_sub);
        printf("\t>>read %d markers from [%s]\n",ql_swc_tar.listNeuron.size(),qPrintable(qs_filename_marker_tar));
        printf("\t>>read %d markers from [%s]\n",ql_swc_sub.listNeuron.size(),qPrintable(qs_filename_marker_sub));
        for(long i=0;i<ql_swc_tar.listNeuron.size();i++)
        {
            vec_tar.push_back(Coord3D_PCM(ql_swc_tar.listNeuron.at(i).x,ql_swc_tar.listNeuron.at(i).y,ql_swc_tar.listNeuron.at(i).z));
        }
        for(long i=0;i<ql_swc_sub.listNeuron.size();i++)
        {
            vec_sub.push_back(Coord3D_PCM(ql_swc_sub.listNeuron.at(i).x,ql_swc_sub.listNeuron.at(i).y,ql_swc_sub.listNeuron.at(i).z));
        }
    }
    else
    {
    	printf("ERROR: at least one marker file is invalid.\n");
    	return false;
	}
	//reorganize the markers to the format needed by q_affine_compute_affinmatrix_2D()


	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. estimate the affine matrix. \n");
	//2D: estimate the affine matrix only based on the xy coordinate of the pointset
	Matrix x3x3_affinematrix;
	Matrix x4x4_affinematrix;
	if(n_how2affine==0)
	{
		if(!q_affine_compute_affinmatrix_2D(vec_tar,vec_sub,x3x3_affinematrix))
		{
			printf("ERROR: q_affine_compute_affinmatrix_2D() return false.\n");
			return false;
		}
	}
	//3D: estimate the affine matrix only based on the xyz coordinate of the pointset
	else if(n_how2affine==1)
	{
		if(!q_affine_compute_affinmatrix_3D(vec_tar,vec_sub,x4x4_affinematrix))
		{
			printf("ERROR: q_affine_compute_affinmatrix_2D() return false.\n");
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. Read subject image. \n");
	unsigned char *p_img_sub=0;
	long *sz_img_sub=0;
	int datatype_sub=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub,sz_img_sub,datatype_sub))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
		return false;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
	printf("\t\tdatatype: %d\n",datatype_sub);

	//if image is 16bit, convert to 8bit
	if(datatype_sub==2)
	{
		printf("\t>>convert image from 16bits to 8bits\n");
		if(!q_imgconvertscale_16bit_2_8bit(p_img_sub,sz_img_sub))
		{
			printf("ERROR: q_imgconvertscale_16bit_2_8bit() return false.\n");
			if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
			if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
			return false;
		}
	}
	else if(datatype_sub>2)
	{
		printf("ERROR: datatype>2 !.\n");
		if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
		if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. Affine transform the subject image to target. \n");
	//assign output/warp image size
	if(sz_img_output[0]==0)		sz_img_output[0]=sz_img_sub[0];
	if(sz_img_output[1]==0)		sz_img_output[1]=sz_img_sub[1];
	if(sz_img_output[2]==0)		sz_img_output[2]=sz_img_sub[2];
								sz_img_output[3]=sz_img_sub[3];

	//allocate memory
	unsigned char *p_img_sub2tar=0;
	p_img_sub2tar=new unsigned char[sz_img_output[0]*sz_img_output[1]*sz_img_output[2]*sz_img_output[3]];
	if(!p_img_sub2tar)
	{
		printf("ERROR: Fail to allocate memory for p_img_sub2tar.\n");
		if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
		if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
		return false;
	}
	unsigned char ****p_img_sub_4d=0,****p_img_sub2tar_4d=0;
	if(!new4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3],p_img_sub) ||
	   !new4dpointer(p_img_sub2tar_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3],p_img_sub2tar))
	{
		printf("ERROR: Fail to allocate memory for the 4d pointer of image.\n");
		if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
		if(p_img_sub2tar) 		{delete []p_img_sub2tar;		p_img_sub2tar=0;}
		if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
		if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3]);}
		if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
		return false;
	}
	//affine transform the subject image to target (inverse projection + linear interpolation)
	if(n_how2affine==0)	//2D
	{
		Matrix x_pt_sub2tar_homo(3,1),x_pt_sub_homo(3,1);
		for(long x=0;x<sz_img_output[0];x++)
			for(long y=0;y<sz_img_output[1];y++)
			{
				//compute the inverse projected coordinate in subject image
				x_pt_sub2tar_homo(1,1)=x;
				x_pt_sub2tar_homo(2,1)=y;
				x_pt_sub2tar_homo(3,1)=1.0;
				x_pt_sub_homo=x3x3_affinematrix*x_pt_sub2tar_homo;

				//------------------------------------------------------------------
				//linear interpolate
				for(long z=0;z<sz_img_output[2];z++)
				{
					//coordinate in subject image
					double cur_pos[3];//x,y,z
					cur_pos[0]=x_pt_sub_homo(1,1);
					cur_pos[1]=x_pt_sub_homo(2,1);
					cur_pos[2]=z;

					//if interpolate pixel is out of subject image region, set to 0
					if(cur_pos[0]<0 || cur_pos[0]>sz_img_sub[0]-1 ||
					   cur_pos[1]<0 || cur_pos[1]>sz_img_sub[1]-1 ||
					   cur_pos[2]<0 || cur_pos[2]>sz_img_sub[2]-1)
					{
						p_img_sub2tar_4d[0][z][y][x]=0.0;
						continue;
					}

					//find 8 neighor pixels boundary
					long x_s,x_b,y_s,y_b,z_s,z_b;
					x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
					y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
					z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

					//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
					double l_w,r_w,t_w,b_w;
					l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
					t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
					//compute weight for higer slice and lower slice
					double u_w,d_w;
					u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

					//linear interpolate each channel
					for(long c=0;c<sz_img_output[3];c++)
					{
						//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double higher_slice;
						higher_slice=t_w*(l_w*p_img_sub_4d[c][z_s][y_s][x_s]+r_w*p_img_sub_4d[c][z_s][y_s][x_b])+
									 b_w*(l_w*p_img_sub_4d[c][z_s][y_b][x_s]+r_w*p_img_sub_4d[c][z_s][y_b][x_b]);
						//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double lower_slice;
						lower_slice =t_w*(l_w*p_img_sub_4d[c][z_b][y_s][x_s]+r_w*p_img_sub_4d[c][z_b][y_s][x_b])+
									 b_w*(l_w*p_img_sub_4d[c][z_b][y_b][x_s]+r_w*p_img_sub_4d[c][z_b][y_b][x_b]);
						//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
						p_img_sub2tar_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice;
					}
				}

			}
	}
	else if(n_how2affine==1)	//3D
	{
		Matrix x_pt_sub2tar_homo(4,1),x_pt_sub_homo(4,1);
		for(long x=0;x<sz_img_output[0];x++)
			for(long y=0;y<sz_img_output[1];y++)
				for(long z=0;z<sz_img_output[2];z++)
				{
					//compute the inverse projected coordinate in subject image
					x_pt_sub2tar_homo(1,1)=x;
					x_pt_sub2tar_homo(2,1)=y;
					x_pt_sub2tar_homo(3,1)=z;
					x_pt_sub2tar_homo(4,1)=1.0;
					x_pt_sub_homo=x4x4_affinematrix*x_pt_sub2tar_homo;

					//------------------------------------------------------------------
					//linear interpolate
					//coordinate in subject image
					double cur_pos[3];//x,y,z
					cur_pos[0]=x_pt_sub_homo(1,1);
					cur_pos[1]=x_pt_sub_homo(2,1);
					cur_pos[2]=x_pt_sub_homo(3,1);

					//if interpolate pixel is out of subject image region, set to -inf
					if(cur_pos[0]<0 || cur_pos[0]>sz_img_sub[0]-1 ||
					   cur_pos[1]<0 || cur_pos[1]>sz_img_sub[1]-1 ||
					   cur_pos[2]<0 || cur_pos[2]>sz_img_sub[2]-1)
					{
						p_img_sub2tar_4d[0][z][y][x]=0.0;
						continue;
					}

					//find 8 neighor pixels boundary
					long x_s,x_b,y_s,y_b,z_s,z_b;
					x_s=floor(cur_pos[0]);		x_b=ceil(cur_pos[0]);
					y_s=floor(cur_pos[1]);		y_b=ceil(cur_pos[1]);
					z_s=floor(cur_pos[2]);		z_b=ceil(cur_pos[2]);

					//compute weight for left and right, top and bottom -- 4 neighbor pixel's weight in a slice
					double l_w,r_w,t_w,b_w;
					l_w=1.0-(cur_pos[0]-x_s);	r_w=1.0-l_w;
					t_w=1.0-(cur_pos[1]-y_s);	b_w=1.0-t_w;
					//compute weight for higer slice and lower slice
					double u_w,d_w;
					u_w=1.0-(cur_pos[2]-z_s);	d_w=1.0-u_w;

					//linear interpolate each channel
					for(long c=0;c<sz_img_output[3];c++)
					{
						//linear interpolate in higher slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double higher_slice;
						higher_slice=t_w*(l_w*p_img_sub_4d[c][z_s][y_s][x_s]+r_w*p_img_sub_4d[c][z_s][y_s][x_b])+
									 b_w*(l_w*p_img_sub_4d[c][z_s][y_b][x_s]+r_w*p_img_sub_4d[c][z_s][y_b][x_b]);
						//linear interpolate in lower slice [t_w*(l_w*lt+r_w*rt)+b_w*(l_w*lb+r_w*rb)]
						double lower_slice;
						lower_slice =t_w*(l_w*p_img_sub_4d[c][z_b][y_s][x_s]+r_w*p_img_sub_4d[c][z_b][y_s][x_b])+
									 b_w*(l_w*p_img_sub_4d[c][z_b][y_b][x_s]+r_w*p_img_sub_4d[c][z_b][y_b][x_b]);
						//linear interpolate the current position [u_w*higher_slice+d_w*lower_slice]
						p_img_sub2tar_4d[c][z][y][x]=u_w*higher_slice+d_w*lower_slice;
					}
				}

	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. Save results. \n");
	saveImage(qPrintable(qs_filename_img_sub2tar),p_img_sub2tar,sz_img_output,1);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. free memory. \n");
	if(p_img_sub_4d) 		{delete4dpointer(p_img_sub_4d,sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);}
	if(p_img_sub2tar_4d) 	{delete4dpointer(p_img_sub2tar_4d,sz_img_output[0],sz_img_output[1],sz_img_output[2],sz_img_output[3]);}
	if(p_img_sub) 			{delete []p_img_sub;			p_img_sub=0;}
	if(p_img_sub2tar) 		{delete []p_img_sub2tar;		p_img_sub2tar=0;}
	if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
	
	return true;
}

//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_affinetranform -t <markerFilename_target> -s <markerFilename_subject> -S <imageFilename_sub> -o <imageFilename_sub2tar>\n");
	printf("\t -t <markerFilename_target>	input target marker file full name.\n");
	printf("\t -s <markerFilename_subject>	input subject marker file full name.\n");
	printf("\t -S <imageFilename_sub>	    input subject image file full name.\n");
	printf("\t -o <imageFilename_sub2tar>	output sub2tar warped image file full name.\n");
	printf("\t -f <how2estimate_affine>	    assign what information is used to estimate affine transform\n");
	printf("\t               0: only use XY information of pointset to estimate the affine transform\n");
	printf("\t                  all Z planes share the same transformation\n");
	printf("\t               1: use XYZ information of pointset to estimate the affine transform\n");
	printf("\t [-W] <outputimg_width>	    assign the width of output image (default same as subject image size)\n");
	printf("\t [-H] <outputimg_height>      assign the height of output image (default same as subject image size)\n");
	printf("\t [-Z] <outputimg_z>	        assign the z of output image (default same as subject image size)\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}

//convert input image from 16bit to 8bit and rescale to [0~255]
bool q_imgconvertscale_16bit_2_8bit(unsigned char *&p_img,const long *sz_img)
{
	long l_npixels=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
	unsigned char * img_subject_tmp=0;
	img_subject_tmp=new unsigned char[l_npixels];
	if(!img_subject_tmp)
	{
		printf("ERROR: Fail to allocate memory for convert 16bit data to 8 bit. Do nothing. \n");
		return false;
	}

	double min,max;
	if(!rescale_to_0_255_and_copy((unsigned short int *)p_img, l_npixels, min, max, img_subject_tmp))
	{
		printf("ERROR: rescale_to_0_255_and_copy() return false.\n");
		if(img_subject_tmp) 	{delete []img_subject_tmp;		img_subject_tmp=0;}
		return false;
	}

	delete []p_img;
	p_img=img_subject_tmp;

	return true;
}

//centrilize and scale the point set
//	xn = T*x;
//	x: every column represent a point [2/3*N]
bool q_normalize_points_2D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x3x3_normalize)
{
	//check parameters
	if(vec_input.size()<=0)
	{
		fprintf(stderr,"ERROR: Input array is null! \n");
		return false;
	}
	if(!vec_output.empty())
		vec_output.clear();
	vec_output=vec_input;
	if(x3x3_normalize.nrows()!=3 || x3x3_normalize.ncols()!=3)
	{
		x3x3_normalize.ReSize(3,3);
	}

	//compute the centriod of input point set
	Coord3D_PCM cord_centroid;
	int n_point=vec_input.size();
	for(int i=0;i<n_point;i++)
	{
		cord_centroid.x+=vec_input[i].x;
		cord_centroid.y+=vec_input[i].y;
	}
	cord_centroid.x/=n_point;
	cord_centroid.y/=n_point;
	//center the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x-=cord_centroid.x;
		vec_output[i].y-=cord_centroid.y;
	}

	//compute the average distance of every point to the origin
	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(vec_output[i].x*vec_output[i].x+vec_output[i].y*vec_output[i].y);
		d_point2o_avg+=d_point2o;
	}
	d_point2o_avg/=n_point;
	//compute the scale factor
	double d_scale_factor=1.0/d_point2o_avg;
	//scale the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x*=d_scale_factor;
		vec_output[i].y*=d_scale_factor;
	}

	//compute the transformation matrix
	// 1 row
	x3x3_normalize(1,1)=d_scale_factor;
	x3x3_normalize(1,2)=0;
	x3x3_normalize(1,3)=-d_scale_factor*cord_centroid.x;
	// 2 row
	x3x3_normalize(2,1)=0;
	x3x3_normalize(2,2)=d_scale_factor;
	x3x3_normalize(2,3)=-d_scale_factor*cord_centroid.y;
	// 3 row
	x3x3_normalize(3,1)=0;
	x3x3_normalize(3,2)=0;
	x3x3_normalize(3,3)=1;

	return true;
}

bool q_normalize_points_3D(const vector<Coord3D_PCM> vec_input,vector<Coord3D_PCM> &vec_output,Matrix &x4x4_normalize)
{
	//check parameters
	if(vec_input.size()<=0)
	{
		fprintf(stderr,"ERROR: Input array is null! \n");
		return false;
	}
	if(!vec_output.empty())
		vec_output.clear();
	vec_output=vec_input;
	if(x4x4_normalize.nrows()!=4 || x4x4_normalize.ncols()!=4)
	{
		x4x4_normalize.ReSize(4,4);
	}

	//compute the centriod of input point set
	Coord3D_PCM cord_centroid;
	int n_point=vec_input.size();
	for(int i=0;i<n_point;i++)
	{
		cord_centroid.x+=vec_input[i].x;
		cord_centroid.y+=vec_input[i].y;
		cord_centroid.z+=vec_input[i].z;
	}
	cord_centroid.x/=n_point;
	cord_centroid.y/=n_point;
	cord_centroid.z/=n_point;
	//center the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x-=cord_centroid.x;
		vec_output[i].y-=cord_centroid.y;
		vec_output[i].z-=cord_centroid.z;
	}

	//compute the average distance of every point to the origin
	double d_point2o=0,d_point2o_avg=0;
	for(int i=0;i<n_point;i++)
	{
		d_point2o=sqrt(vec_output[i].x*vec_output[i].x+vec_output[i].y*vec_output[i].y+vec_output[i].z*vec_output[i].z);
		d_point2o_avg+=d_point2o;
	}
	d_point2o_avg/=n_point;
	//compute the scale factor
	double d_scale_factor=1.0/d_point2o_avg;
	//scale the point set
	for(int i=0;i<n_point;i++)
	{
		vec_output[i].x*=d_scale_factor;
		vec_output[i].y*=d_scale_factor;
		vec_output[i].z*=d_scale_factor;
	}

	//compute the transformation matrix
	// 1 row
	x4x4_normalize(1,1)=d_scale_factor;
	x4x4_normalize(1,2)=0;
	x4x4_normalize(1,3)=0;
	x4x4_normalize(1,4)=-d_scale_factor*cord_centroid.x;
	// 2 row
	x4x4_normalize(2,1)=0;
	x4x4_normalize(2,2)=d_scale_factor;
	x4x4_normalize(2,3)=0;
	x4x4_normalize(2,4)=-d_scale_factor*cord_centroid.y;
	// 3 row
	x4x4_normalize(3,1)=0;
	x4x4_normalize(3,2)=0;
	x4x4_normalize(3,3)=d_scale_factor;
	x4x4_normalize(3,4)=-d_scale_factor*cord_centroid.z;
	// 4 row
	x4x4_normalize(4,1)=0;
	x4x4_normalize(4,2)=0;
	x4x4_normalize(4,3)=0;
	x4x4_normalize(4,4)=1;

	return true;
}

//compute the affine matraix
//	B=T*A
bool q_affine_compute_affinmatrix_2D(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,Matrix &x3x3_affinematrix)
{
	//check parameters
	if(vec_A.size()<3 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x3x3_affinematrix.nrows()!=3 || x3x3_affinematrix.ncols()!=3)
	{
		x3x3_affinematrix.ReSize(3,3);
	}

	//normalize point set
	vector<Coord3D_PCM> vec_A_norm,vec_B_norm;
	Matrix x3x3_normalize_A(4,4),x3x3_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_2D(vec_A,vec_A_norm,x3x3_normalize_A);
	q_normalize_points_2D(vec_B,vec_B_norm,x3x3_normalize_B);

	//	 fill matrix A
	//
	//	  | h1, h2, h3 |    |x1| |x2|
	//	  | h4, h5, h6 | *  |y1|=|y2| <=>
	//	  | 0 ,  0,  1 |    |1 | |1 |
	//
	//	  |x1, y1, 1,  0,  0,  0, -x2 |
	//	  | 0,  0,  0, x1, y1, 1, -y2 | * |h1,h2,h3,h4,h5,h6,h7|=0
	//
	int n_point=vec_A.size();
	Matrix A(2*n_point,7);
	int row=1;
	for(int i=0;i<n_point;i++)
	{
		A(row,1)=vec_A_norm[i].x;	A(row,2)=vec_A_norm[i].y;	A(row,3)=1.0;
		A(row,4)=0.0;				A(row,5)=0.0;				A(row,6)=0.0;
		A(row,7)=-vec_B_norm[i].x;

		A(row+1,1)=0.0;				A(row+1,2)=0.0;				A(row+1,3)=0.0;
		A(row+1,4)=vec_A_norm[i].x;	A(row+1,5)=vec_A_norm[i].y;	A(row+1,6)=1.0;
		A(row+1,7)=-vec_B_norm[i].y;

		row+=2;
	}

	//compute T  --> bug? SVD in newmat need row>=col?
	DiagonalMatrix D;
	Matrix U,V;
	SVD(A,D,U,V);	//A = U * D * V.t()

	Matrix h=V.column(7);	//A*h=0
	if(D(6,6)==0)			//degenerate case
	{
		x3x3_affinematrix=0.0;	//check with A.is_zero()
		printf("Degenerate singular values in SVD! \n");
		//		return false;
	}

	//de-homo
	for(int i=1;i<=7;i++)
	{
		h(i,1) /= h(7,1);
	}

	//reshape h:7*1 to 3*3 matrix
	x3x3_affinematrix(1,1)=h(1,1);	x3x3_affinematrix(1,2)=h(2,1);	x3x3_affinematrix(1,3)=h(3,1);
	x3x3_affinematrix(2,1)=h(4,1);	x3x3_affinematrix(2,2)=h(5,1);	x3x3_affinematrix(2,3)=h(6,1);
	x3x3_affinematrix(3,1)=0.0;		x3x3_affinematrix(3,2)=0.0;		x3x3_affinematrix(3,3)=1.0;

	//denormalize
	x3x3_affinematrix=x3x3_normalize_B.i()*x3x3_affinematrix*x3x3_normalize_A;

	return true;
}

bool q_affine_compute_affinmatrix_3D(const vector<Coord3D_PCM> &vec_A,const vector<Coord3D_PCM> &vec_B,Matrix &x4x4_affinematrix)
{
	//check parameters
	if(vec_A.size()<4 || vec_A.size()!=vec_B.size())
	{
		fprintf(stderr,"ERROR: Invalid input parameters! \n");
		return false;
	}
	if(x4x4_affinematrix.nrows()!=4 || x4x4_affinematrix.ncols()!=4)
	{
		x4x4_affinematrix.ReSize(4,4);
	}

	//normalize point set
	vector<Coord3D_PCM> vec_A_norm,vec_B_norm;
	Matrix x4x4_normalize_A(4,4),x4x4_normalize_B(4,4);
	vec_A_norm=vec_A;	vec_B_norm=vec_B;
	q_normalize_points_3D(vec_A,vec_A_norm,x4x4_normalize_A);
	q_normalize_points_3D(vec_B,vec_B_norm,x4x4_normalize_B);

	//fill matrix A
	//
	//	  | h1, h2, h3, h4 |    |x1| |x2|
	//	  | h5, h6, h7, h8 | *  |y1|=|y2| <=>
	//	  | h9, h10,h11,h12|    |z1| |z2|
	//	  | 0 ,  0,  0,  1 |    |1 | |1 |
	//
	//	  |x1, y1, z1, 1,  0,  0,  0,  0,  0,  0,  0,  0, -x2 |
	//	  | 0,  0,  0, 0, x1, y1, z1,  1,  0,  0,  0,  0, -y2 | * |h1,h2,h3,h4,h5,h6,h7,h8,h9,h10,h11,h12,h13|=0
	//	  | 0,  0,  0, 0, 0, 0, 0, 0,  0, x1, y1, z1,  1, -z2 |
	int n_point=vec_A.size();
	Matrix A(3*n_point,13);
	int row=1;
	for(int i=0;i<n_point;i++)
	{
		A(row,1)=vec_A_norm[i].x;	A(row,2)=vec_A_norm[i].y;	A(row,3)=vec_A_norm[i].z;	A(row,4)=1.0;
		A(row,5)=0.0;				A(row,6)=0.0;				A(row,7)=0.0;				A(row,8)=0.0;
		A(row,9)=0.0;				A(row,10)=0.0;				A(row,11)=0.0;				A(row,12)=0.0;
		A(row,13)=-vec_B_norm[i].x;

		A(row+1,1)=0.0;				A(row+1,2)=0.0;				A(row+1,3)=0.0;				A(row+1,4)=0.0;
		A(row+1,5)=vec_A_norm[i].x;	A(row+1,6)=vec_A_norm[i].y;	A(row+1,7)=vec_A_norm[i].z;	A(row+1,8)=1.0;
		A(row+1,9)=0.0;				A(row+1,10)=0.0;			A(row+1,11)=0.0;			A(row+1,12)=0.0;
		A(row+1,13)=-vec_B_norm[i].y;

		A(row+2,1)=0.0;				A(row+2,2)=0.0;				A(row+2,3)=0.0;				A(row+2,4)=0.0;
		A(row+2,5)=0.0;				A(row+2,6)=0.0;				A(row+2,7)=0.0;				A(row+2,8)=0.0;
		A(row+2,9)=vec_A_norm[i].x;	A(row+2,10)=vec_A_norm[i].y;A(row+2,11)=vec_A_norm[i].z;A(row+2,12)=1.0;
		A(row+2,13)=-vec_B_norm[i].z;

		row+=3;
	}

	//compute T  --> bug? SVD in newmat need row>=col?
	DiagonalMatrix D;
	Matrix U,V;
	SVD(A,D,U,V);	//A = U * D * V.t()

	Matrix h=V.column(13);	//A*h=0
	if(D(12,12)==0)			//degenerate case
	{
		x4x4_affinematrix=0.0;	//check with A.is_zero()
		printf("Degenerate singular values in SVD! \n");
		//		return false;
	}

	//de-homo
	for(int i=1;i<=13;i++)
	{
		h(i,1) /= h(13,1);
	}

	//reshape h:13*1 to 4*4 matrix
	x4x4_affinematrix(1,1)=h(1,1);	x4x4_affinematrix(1,2)=h(2,1);	x4x4_affinematrix(1,3)=h(3,1);	x4x4_affinematrix(1,4)=h(4,1);
	x4x4_affinematrix(2,1)=h(5,1);	x4x4_affinematrix(2,2)=h(6,1);	x4x4_affinematrix(2,3)=h(7,1);	x4x4_affinematrix(2,4)=h(8,1);
	x4x4_affinematrix(3,1)=h(9,1);	x4x4_affinematrix(3,2)=h(10,1);	x4x4_affinematrix(3,3)=h(11,1);	x4x4_affinematrix(3,4)=h(12,1);
	x4x4_affinematrix(4,1)=0.0;		x4x4_affinematrix(4,2)=0.0;		x4x4_affinematrix(4,3)=0.0;		x4x4_affinematrix(4,4)=1.0;

	//denormalize
	x4x4_affinematrix=x4x4_normalize_B.i()*x4x4_affinematrix*x4x4_normalize_A;

	return true;
}


//	//project tar2sub use affine matrix
//	vector<Coord3D_PCM> vec_tar_invp;
//	Matrix x_vec_tar_invp(3,vec_tar.size()), x_vec_tar_homo(3,vec_tar.size());
//	vec_tar_invp.assign(vec_tar.size(),Coord3D_PCM());
//	for(long i=0;i<vec_tar.size();i++)
//	{
//		x_vec_tar_homo(1,i+1)=vec_tar[i].x;
//		x_vec_tar_homo(2,i+1)=vec_tar[i].y;
//		x_vec_tar_homo(3,i+1)=1.0;
//	}
//	x_vec_tar_invp=x3x3_affinematrix*x_vec_tar_homo;
//	for(long i=0;i<vec_tar.size();i++)
//	{
//		vec_tar_invp[i].x=x_vec_tar_invp(1,i+1);
//		vec_tar_invp[i].y=x_vec_tar_invp(2,i+1);
//	}
//	//rereorganize matched marker pairs back to LandmarkList
//	QList<ImageMarker> ql_marker_invp(ql_marker_tar);
//	for(long i=0;i<vec_tar_invp.size();i++)
//	{
//		ql_marker_invp[i].x=vec_tar_invp[i].x;
//		ql_marker_invp[i].y=vec_tar_invp[i].y;
//	}
//	wirteMarker_file("tar.marker",ql_marker_tar);
//	wirteMarker_file("sub.marker",ql_marker_sub);
//	wirteMarker_file("tar2sub.marker",ql_marker_invp);
//	//inverse project sub2tar use affine matrix
//	vector<Coord3D_PCM> vec_sub2tar;
//	Matrix x_vec_sub2tar_homo(4,vec_sub.size()), x_vec_sub_homo(4,vec_sub.size());
//	vec_sub2tar.assign(vec_sub.size(),Coord3D_PCM());
//	for(long i=0;i<vec_sub.size();i++)
//	{
//		x_vec_sub_homo(1,i+1)=vec_sub[i].x;
//		x_vec_sub_homo(2,i+1)=vec_sub[i].y;
////		x_vec_sub_homo(3,i+1)=vec_sub[i].z;
//		x_vec_sub_homo(3,i+1)=0;
//		x_vec_sub_homo(4,i+1)=1.0;
//	}
//	x_vec_sub2tar_homo=x4x4_affinematrix.i()*x_vec_sub_homo;
//	for(long i=0;i<vec_sub.size();i++)
//	{
//		vec_sub2tar[i].x=x_vec_sub2tar_homo(1,i+1);
//		vec_sub2tar[i].y=x_vec_sub2tar_homo(2,i+1);
//		vec_sub2tar[i].z=x_vec_sub2tar_homo(3,i+1);
//	}
//	//rereorganize matched marker pairs back to LandmarkList
//	QList<ImageMarker> ql_marker_sub2tar(ql_marker_tar);
//	for(long i=0;i<vec_sub2tar.size();i++)
//	{
//		ql_marker_sub2tar[i].x=vec_sub2tar[i].x;
//		ql_marker_sub2tar[i].y=vec_sub2tar[i].y;
//		ql_marker_sub2tar[i].z=vec_sub2tar[i].z;
//		printf("%.2f,%.2f,%.2f\n",ql_marker_sub2tar[i].x,ql_marker_sub2tar[i].y,ql_marker_sub2tar[i].z);
//	}
//	wirteMarker_file("sub2tar.marker",ql_marker_sub2tar);
