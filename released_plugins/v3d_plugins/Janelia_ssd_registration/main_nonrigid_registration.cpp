//main_nonrigid_registration.cpp
// by Lei Qu
//2010-02-16

#ifndef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif
#ifndef SHOWTIME
	#define SHOWTIME 1
#endif

#include <QtGui>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
extern char *optarg;
extern int optind, opterr;
#include "../../basic_c_fun/stackutil.h"
#include "../../basic_c_fun/basic_surf_objs.h"

#include "../histogram_matching/q_histogram_matching.h"
#include "q_convolve.h"
#include "q_imresize.h"
#include "q_nonrigid_registration.h"

void printHelp();
void releasememory(long*&,long*&,unsigned char*&,unsigned char*&,double*&,double*&,double*&,double*&,double*&,double*&);

int main(int argc, char *argv[])
{
	if(argc<=1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//read arguments
	QString qs_filename_img_tar			=NULL;
	QString qs_filename_img_sub			=NULL;
	QString qs_filename_swc_inigrid		=NULL;
	QString qs_filename_img_sub2tar		=NULL;
	QString qs_filename_swc_grid		=NULL;
	long l_refchannel=0;
	long l_hierarchlevel=3;
	double d_downsample_ratio=2.0;
	long l_gridwndsz=10;
	bool b_dohismatching=1;

	int c;
	static char optstring[]="ht:s:i:o:r:l:d:w:g:n:";
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
				qs_filename_img_tar.append(optarg);
				break;
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				qs_filename_img_sub.append(optarg);
				break;
			case 'i':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				qs_filename_swc_inigrid.append(optarg);
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				qs_filename_img_sub2tar.append(optarg);
				break;
			case 'r':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -r.\n");
					return 1;
				}
				l_refchannel=atoi(optarg);
				break;
			case 'l':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -h.\n");
					return 1;
				}
				l_hierarchlevel=atoi(optarg);
				break;
			case 'd':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				d_downsample_ratio=atof(optarg);
				break;
			case 'w':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -w.\n");
					return 1;
				}
				l_gridwndsz=atoi(optarg);
				break;
			case 'g':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -g.\n");
					return 1;
				}
				qs_filename_swc_grid.append(optarg);
				break;
			case 'n':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -n.\n");
					return 1;
				}
				b_dohismatching=atoi(optarg);
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
    }

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>FFD based non-rigid registration\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input target  image:          %s\n",qPrintable(qs_filename_img_tar));
	printf(">>  input subject image:          %s\n",qPrintable(qs_filename_img_sub));
	printf(">>  input initial grid:           %s\n",qPrintable(qs_filename_swc_inigrid));
	printf(">>  input reference channel:      %ld\n",l_refchannel);
	printf(">>  input hierarchical level:     %ld\n",l_hierarchlevel);
	printf(">>  input downsample ratio:       %.2f\n",d_downsample_ratio);
	printf(">>  input meshgrid window size:   %ld\n",l_gridwndsz);
	printf(">>  do histogram matching:        %d\n",b_dohismatching);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output sub2tar image:         %s\n",qPrintable(qs_filename_img_sub2tar));
	printf(">>  output meshgrid apo:          %s\n",qPrintable(qs_filename_swc_grid));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	clock_t start;

	//------------------------------------------------------------------------------------------------------------------------------------
	//pointers definition (needed to be released)
	long *sz_img_tar=0,*sz_img_sub=0;						//input image size. [0]:width, [1]:height, [2]:z, [3]:nchannel
	unsigned char *p_img_tar_input=0,*p_img_sub_input=0;	//input images pointer
	double *p_img64f_tar=0,*p_img64f_sub=0;					//input images pointer (double type)
	double *p_img64f_sub_bk=0;								//backed nonsmoothed subject image pointer
	double *p_img64f_tar_1c=0,*p_img64f_sub_1c=0;			//image pointers with only reference channel
	double *p_img64f_output_sub=0;							//output warped subject image pointer

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target, subject image and inigrid data. \n");
	//read target image
	if(SHOWTIME) start=clock();
	int datatype_tar=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_tar),p_img_tar_input,sz_img_tar,datatype_tar))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_tar));
		return false;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img_tar));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_tar[0],sz_img_tar[1],sz_img_tar[2],sz_img_tar[3]);
	printf("\t\tdatatype: %d\n",datatype_tar);
	if(SHOWTIME) printf("\t>>time consume: %.2f s\n",(clock()-start)/100.0);
	
	//read subject image
	if(SHOWTIME) start=clock();
	int datatype_sub=0;
	if(!loadImage((char *)qPrintable(qs_filename_img_sub),p_img_sub_input,sz_img_sub,datatype_sub))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_img_sub));
		return false;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_img_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
	printf("\t\tdatatype: %d\n",datatype_sub);
	if(SHOWTIME) printf("\t>>time consume: %.2f s\n",(clock()-start)/100.0);

	//read intial grid data
	bool b_hasinigrid=0;
	vector< vector< vector< vector<double> > > > vec4D_grid_ori,vec4D_grid_ini;
	if(qs_filename_swc_inigrid!=NULL)
	{
		b_hasinigrid=1;

		NeuronTree nt;
		nt=readSWC_file(qs_filename_swc_inigrid);

		//get grid size
		long sz_grid[3]={0}; //w,h,z
		double d_biggestcoord[3]={-1e+10,-1e+10,-1e+10};	//x,y,z
		for(long i=0;i<nt.listNeuron.size();i++)
			if(nt.listNeuron[i].pn!=-1)
			{
				if(nt.listNeuron[i].x>d_biggestcoord[0])	d_biggestcoord[0]=nt.listNeuron[i].x;
				if(nt.listNeuron[i].y>d_biggestcoord[1])	d_biggestcoord[1]=nt.listNeuron[i].y;
				if(nt.listNeuron[i].z>d_biggestcoord[2])	d_biggestcoord[2]=nt.listNeuron[i].z;
			}
		sz_grid[0]=d_biggestcoord[1]/l_gridwndsz+2+0.5;
		sz_grid[1]=d_biggestcoord[0]/l_gridwndsz+2+0.5;
		sz_grid[2]=d_biggestcoord[2]/l_gridwndsz+2+0.5;

		//read grid data
		vec4D_grid_ori.assign(sz_grid[0],vector< vector< vector<double> > >(sz_grid[1],vector< vector<double> >(sz_grid[2],vector<double>(3,0))));
		vec4D_grid_ini=vec4D_grid_ori;
		for(long i=0;i<nt.listNeuron.size();i++)
		{
			long x,y,z;
			if(nt.listNeuron[i].pn!=-1)
			{
				x=nt.listNeuron[i].x/l_gridwndsz+1;
				y=nt.listNeuron[i].y/l_gridwndsz+1;
				z=nt.listNeuron[i].z/l_gridwndsz+1;

				vec4D_grid_ori[y][x][z][0]=nt.listNeuron[i].x;
				vec4D_grid_ori[y][x][z][1]=nt.listNeuron[i].y;
				vec4D_grid_ori[y][x][z][2]=nt.listNeuron[i].z;

				i++;

				vec4D_grid_ini[y][x][z][0]=nt.listNeuron[i].x;
				vec4D_grid_ini[y][x][z][1]=nt.listNeuron[i].y;
				vec4D_grid_ini[y][x][z][2]=nt.listNeuron[i].z;
			}
		}

		printf("\t>>read initial grid file [%s] complete.\n",qPrintable(qs_filename_swc_inigrid));
		printf("\t\tsz_inigrid: %ld,%ld,%ld\n",sz_grid[0],sz_grid[1],sz_grid[2]);

	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//check image size
	if(sz_img_tar[0]!=sz_img_sub[0] || sz_img_tar[1]!=sz_img_sub[1] || sz_img_tar[2]!=sz_img_sub[2] || sz_img_tar[3]!=sz_img_sub[3])
	{
		printf("ERROR: input target and subject image have different size!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}

	long l_npixels=sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2]*sz_img_tar[3];

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Convert image data from uint16 to double and scale to [0~1]. \n");
	if(SHOWTIME) start=clock();
	{
	p_img64f_tar=new double[l_npixels]();
	p_img64f_sub=new double[l_npixels]();
	if(!p_img64f_tar || !p_img64f_sub)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}

	//scale to [0~1]
	long l_maxintensity_tar=0,l_maxintensity_sub=0;
	if(datatype_tar==1)
	{
		l_maxintensity_tar=l_maxintensity_sub=255;
		for(long i=0;i<l_npixels;i++)
		{
			p_img64f_tar[i]=p_img_tar_input[i]/(double)l_maxintensity_tar;
			p_img64f_sub[i]=p_img_sub_input[i]/(double)l_maxintensity_sub;
		}
		printf("\t>>convert from 8-bit to double and scale to [0~1]\n");
	}
	else if(datatype_tar==2)
	{
		unsigned short int * p_data16u_tar=(unsigned short int *)p_img_tar_input;
		unsigned short int * p_data16u_sub=(unsigned short int *)p_img_sub_input;

		//find the maiximal intensity value
		for(long i=0;i<l_npixels;i++)
		{
			if(p_data16u_tar[i]>l_maxintensity_tar)
				l_maxintensity_tar=p_data16u_tar[i];
			if(p_data16u_sub[i]>l_maxintensity_sub)
				l_maxintensity_sub=p_data16u_sub[i];
		}
		//rescale to [0~1]
		for(long i=0;i<l_npixels;i++)
		{
			p_img64f_tar[i]=p_data16u_tar[i]/(double)l_maxintensity_tar;
			p_img64f_sub[i]=p_data16u_sub[i]/(double)l_maxintensity_sub;
		}
		printf("\t>>convert from 16-bit to double and scale to [0~1]\n");
	}
	if(p_img_tar_input) 	{delete []p_img_tar_input;		p_img_tar_input=0;}
	if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}
	printf("\t\t>>l_maxintensity_tar=%ld; l_maxintensity_sub=%ld\n",l_maxintensity_tar,l_maxintensity_sub);
	}
	if(SHOWTIME) printf("\t>>time consume %.2f s\n",(clock()-start)/100.0);

//	q_save64f01_image(p_img64f_tar,sz_img_tar,"tar.tif");
//	q_save64f01_image(p_img64f_sub,sz_img_sub,"sub.tif");

	//------------------------------------------------------------------------------------------------------------------------------------
	//backup the subject image before smoothing
	printf("3. Backup the subject image before smoothing.\n");
	if(SHOWTIME) start=clock();
	//allocate memory
	p_img64f_sub_bk=new double[l_npixels]();
	if(!p_img64f_sub_bk)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_tar or p_img64f_sub!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}
	//backup
	for(long i=0;i<l_npixels;i++)
		p_img64f_sub_bk[i]=p_img64f_sub[i];
	if(SHOWTIME) printf("\t>>time consume %.2f s\n",(clock()-start)/100.0);

	//------------------------------------------------------------------------------------------------------------------------------------
	//extract or generate the reference channel
	printf("4. Extract or generate reference channel:[%ld].\n",l_refchannel);
	if(SHOWTIME) start=clock();
	//allocate memeory
	long sz_img_1c[4];
	sz_img_1c[0]=sz_img_tar[0];	sz_img_1c[1]=sz_img_tar[1];	sz_img_1c[2]=sz_img_tar[2];	sz_img_1c[3]=1;
	long l_npixels_1c=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];
	p_img64f_tar_1c=new double[l_npixels_1c]();
	p_img64f_sub_1c=new double[l_npixels_1c]();
	if(!p_img64f_tar_1c || !p_img64f_sub_1c)
	{
		printf("ERROR: Fail to allocate memory for p_img64f_tar_1c or p_img64f_sub_1c!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}
	//extract referece channel (l_refchannel<3) or do some special operation (e.g. l_refchannel=9, for Rex's 2 channels data, we average all channels)
	long pgsz1=sz_img_tar[0];
	long pgsz2=sz_img_tar[0]*sz_img_tar[1];
	long pgsz3=sz_img_tar[0]*sz_img_tar[1]*sz_img_tar[2];
	if(l_refchannel<3 && l_refchannel<sz_img_tar[3])
	{
		printf("\t>>extract channel:[%ld]\n",l_refchannel);
		for(long x=0;x<sz_img_tar[0];x++)
			for(long y=0;y<sz_img_tar[1];y++)
				for(long z=0;z<sz_img_tar[2];z++)
				{
					long ind_1c=pgsz2*z+pgsz1*y+x;
					long ind_ref=pgsz3*l_refchannel+ind_1c;
					p_img64f_tar_1c[ind_1c]=p_img64f_tar[ind_ref];
					p_img64f_sub_1c[ind_1c]=p_img64f_sub[ind_ref];
				}
	}
	else if(l_refchannel==9 && sz_img_tar[3]>1)//for Rex's 2 channels data, we average all channels
	{
		printf("\t>>average all channels --> for Rex's AQ1336 and XL138 data\n");
		for(long x=0;x<sz_img_tar[0];x++)
			for(long y=0;y<sz_img_tar[1];y++)
				for(long z=0;z<sz_img_tar[2];z++)
				{
					long ind_1c=pgsz2*z+pgsz1*y+x;
					for(long c=0;c<sz_img_tar[3];c++)
					{
						long ind_ref=pgsz3*c+ind_1c;
						//initialize
						if(c==0)
						{
							p_img64f_tar_1c[ind_1c]=0;
							p_img64f_sub_1c[ind_1c]=0;
						}
						//average
						p_img64f_tar_1c[ind_1c]+=p_img64f_tar[ind_ref];
						p_img64f_sub_1c[ind_1c]+=p_img64f_sub[ind_ref];
					}
					p_img64f_tar_1c[ind_1c]/=sz_img_tar[3];
					p_img64f_sub_1c[ind_1c]/=sz_img_tar[3];
				}
	}
	else
	{
		printf("ERROR: l_refchannel invalid! \n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}
	if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
	if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}
	if(SHOWTIME) printf("\t>>time consume %.2f s\n",(clock()-start)/100.0);

//	q_save64f01_image(p_img64f_tar_1c,sz_img_1c,"tar_1c.tif");
//	q_save64f01_image(p_img64f_sub_1c,sz_img_1c,"sub_1c.tif");

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. Do histogram matching. \n");
	if(b_dohismatching)
	{
		//convert images from 64f to 8u
		unsigned char *p_img8u1c_tar=new unsigned char[l_npixels_1c]();
		unsigned char *p_img8u1c_sub=new unsigned char[l_npixels_1c]();
		if(!p_img8u1c_tar || !p_img8u1c_sub)
		{
			printf("ERROR: Fail to allocate memory for p_img8u1c_tar or p_img8u1c_sub!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return false;
		}
		for(long i=0;i<l_npixels_1c;i++)
		{
			p_img8u1c_tar[i]=p_img64f_tar_1c[i]*255+0.5;
			p_img8u1c_sub[i]=p_img64f_sub_1c[i]*255+0.5;
		}
		//do histogram matching
		unsigned char *p_img8u1c_sub2tar=0;
		if(!q_histogram_matching_1c(p_img8u1c_tar,sz_img_1c,
									p_img8u1c_sub,sz_img_1c,
									p_img8u1c_sub2tar))
		{
			printf("ERROR: q_histogram_matching_1c() return false!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return false;
		}
		//write matched subject image to 64f image
		for(long i=0;i<l_npixels_1c;i++)
			p_img64f_sub_1c[i]=p_img8u1c_sub2tar[i]/255.0;
		//free memory
		if(p_img8u1c_tar) 		{delete []p_img8u1c_tar;		p_img8u1c_tar=0;}
		if(p_img8u1c_sub) 		{delete []p_img8u1c_sub;		p_img8u1c_sub=0;}
		if(p_img8u1c_sub2tar) 	{delete []p_img8u1c_sub2tar;	p_img8u1c_sub2tar=0;}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. Pre-warpping subject image based on the initial grid (if provided):\n");
	if(vec4D_grid_ini.size()!=0)
	{
		printf("\t>>use input initial grid to pre-warp subject image\n");
		double *p_img64f_prewarped_sub=0;

		//warp image based on deformed grid
		if(!q_warpimage_baseongrid(p_img64f_sub_1c,sz_img_1c,vec4D_grid_ini,l_gridwndsz,p_img64f_prewarped_sub))
		{
			printf("ERROR: q_warpimage_baseongrid() return false!\n");
			return false;
		}
		if(p_img64f_sub_1c) 	{delete []p_img64f_sub_1c;		p_img64f_sub_1c=0;}
		p_img64f_sub_1c=p_img64f_prewarped_sub;
		p_img64f_prewarped_sub=0;

	//	q_save64f01_image(p_img64f_sub_1c,sz_img_1c,"sub_prewarped.tif");
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//downsample target and subject image
	printf("6. downsample target and subject image. \n");
//	if(fabs(d_downsample_ratio-1.0)>1e-7)
//	{
		long sz_img_s[4]={1,1,1,1};//w,h,z,c
		for(long i=0;i<3;i++)
			sz_img_s[i]=sz_img_1c[i]/d_downsample_ratio;

		double *p_img64f_tar_1c_s=0,*p_img64f_sub_1c_s=0;
		if(!q_imresize64f_3D(p_img64f_tar_1c,sz_img_1c,sz_img_s,p_img64f_tar_1c_s) ||
		   !q_imresize64f_3D(p_img64f_sub_1c,sz_img_1c,sz_img_s,p_img64f_sub_1c_s))
		{
			printf("ERROR: q_imresize64f_3D() return false!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return false;
		}
		printf("\t>>sz_img_s:[%ld,%ld,%ld,%ld]\n",sz_img_s[0],sz_img_s[1],sz_img_s[2],sz_img_s[3]);

		if(p_img64f_tar_1c) 	{delete []p_img64f_tar_1c;		p_img64f_tar_1c=0;}
		if(p_img64f_sub_1c) 	{delete []p_img64f_sub_1c;		p_img64f_sub_1c=0;}
		p_img64f_tar_1c=p_img64f_tar_1c_s;		p_img64f_tar_1c_s=0;
		p_img64f_sub_1c=p_img64f_sub_1c_s;		p_img64f_sub_1c_s=0;
		for(long i=0;i<3;i++)	sz_img_1c[i]=sz_img_s[i];
		l_npixels_1c=sz_img_1c[0]*sz_img_1c[1]*sz_img_1c[2];
//	}
//	q_save64f01_image(p_img64f_tar_1c,sz_img_1c,"tar_resize.tif");
//	q_save64f01_image(p_img64f_sub_1c,sz_img_1c,"sub_resize.tif");
	
	//------------------------------------------------------------------------------------------------------------------------------------
	//make the 0 edge result from the affine registration invalid
	for(long i=0;i<l_npixels_1c;i++)
		if(p_img64f_sub_1c[i]==0) p_img64f_sub_1c[i]=-1e+10;

	//------------------------------------------------------------------------------------------------------------------------------------
	//Gaussian smooth (optional)
	printf("4. Gaussian smooth input images. \n");
	long l_radius_x=3,l_radius_y=3,l_radius_z=3;
	double d_sigma=1.0;
	//fast Gaussian (realized by applying 1D convolve on each dimention)
	vector<double> vec1D_kernel;
	if(!q_kernel_gaussian_1D(l_radius_x,d_sigma,vec1D_kernel))
	{
			printf("ERROR: q_kernel_gaussian_1D() return false!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return false;
	}
	printf("\tsmoothing target image.\n");
	if(SHOWTIME) start=clock();
	if(!q_convolve_img64f_3D_fast(p_img64f_tar_1c,sz_img_1c,vec1D_kernel))
	{
			printf("ERROR: q_convolve64f_3D_fast() return false!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return false;
	}
	if(SHOWTIME) printf("\t\t>>time consume %.2f s\n",(clock()-start)/100.0);
	printf("\tsmoothing subject image.\n");
	if(SHOWTIME) start=clock();
	if(!q_convolve_img64f_3D_fast(p_img64f_sub_1c,sz_img_1c,vec1D_kernel))
	{
			printf("ERROR: q_convolve64f_3D_fast() return false!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return false;
	}
	if(SHOWTIME) printf("\t\t>>time consume %.2f s\n",(clock()-start)/100.0);

//	q_save64f01_image(p_img64f_tar_1c,sz_img_1c,"tar_smooth_nonrigid.tif");
//	q_save64f01_image(p_img64f_sub_1c,sz_img_1c,"sub_smooth_nonrigid.tif");


	//------------------------------------------------------------------------------------------------------------------------------------
	//FFD based non-rigid registration
	printf("############################################################################################################################\n");
	printf("5. Enter FFD based non-rigid registration. \n");
	printf("############################################################################################################################\n");
	start=clock();
	vector< vector< vector< vector<double> > > > vec4D_grid;
	if(!q_nonrigid_registration_FFD(p_img64f_tar_1c,p_img64f_sub_1c,sz_img_1c,l_hierarchlevel,l_gridwndsz/d_downsample_ratio,vec4D_grid))
	{
		printf("ERROR: q_nonrigid_registration_FFD() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}
	if(SHOWTIME) printf("\t>>q_nonrigid_registration_FFD() time consume: %.2f s\n",(clock()-start)/100.0);

	//------------------------------------------------------------------------------------------------------------------------------------
	//prepare the final output (warp and trim the input subject image base on the deformed meshgrid)
	printf("6. prepare the final output (warp and trim the subject image base on the deformed meshgrid). \n");
	if(SHOWTIME) start=clock();
	//rescale the grid according to the real image downsample ratio
	double d_realdownsample_ratio[3];//x,y,z
	for(long i=0;i<3;i++)	d_realdownsample_ratio[i]=sz_img_tar[i]/sz_img_1c[i];
	for(long x=0;x<vec4D_grid[0].size();x++)
		for(long y=0;y<vec4D_grid.size();y++)
			for(long z=0;z<vec4D_grid[0][0].size();z++)
			{
				vec4D_grid[y][x][z][0]=vec4D_grid[y][x][z][0]*d_realdownsample_ratio[0];
				vec4D_grid[y][x][z][1]=vec4D_grid[y][x][z][1]*d_realdownsample_ratio[1];
				vec4D_grid[y][x][z][2]=vec4D_grid[y][x][z][2]*d_realdownsample_ratio[2];
			}
	//combine current deformed grid with the initial grid (if provided)
	if(b_hasinigrid)
		for(long x=0;x<vec4D_grid[0].size();x++)
			for(long y=0;y<vec4D_grid.size();y++)
				for(long z=0;z<vec4D_grid[0][0].size();z++)
				{
					vec4D_grid[y][x][z][0]+=vec4D_grid_ini[y][x][z][0]-vec4D_grid_ori[y][x][z][0];
					vec4D_grid[y][x][z][1]+=vec4D_grid_ini[y][x][z][1]-vec4D_grid_ori[y][x][z][1];
					vec4D_grid[y][x][z][2]+=vec4D_grid_ini[y][x][z][2]-vec4D_grid_ori[y][x][z][2];
				}
	//warp image based on deformed grid
	long l_realgridwndsz=floor(l_gridwndsz/d_downsample_ratio)*d_downsample_ratio;
	if(!q_warpimage_baseongrid(p_img64f_sub_bk,sz_img_sub,vec4D_grid,l_realgridwndsz,p_img64f_output_sub))
	{
		printf("ERROR: q_warpimage_baseongrid() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}
	if(SHOWTIME) printf("\t>>q_warpimage_baseongrid() time consume: %.2f s\n",(clock()-start)/100.0);

	//------------------------------------------------------------------------------------------------------------------------------------
	//save warped image
	printf("7. save warped subject image to file:[%s]. \n",qPrintable(qs_filename_img_sub2tar));
	if(!q_save64f01_image(p_img64f_output_sub,sz_img_sub,qPrintable(qs_filename_img_sub2tar)))
	{
		printf("ERROR: q_save64f01_image() return false!\n");
		releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
		return false;
	}
	if(p_img64f_output_sub) {delete []p_img64f_output_sub;		p_img64f_output_sub=0;}

	//------------------------------------------------------------------------------------------------------------------------------------
	//save deformed meshgrid to swc file
	if(qs_filename_swc_grid!=NULL)
	{
		printf("8. save warped subject image to swc file:[%s]. \n",qPrintable(qs_filename_swc_grid));
		if(!q_savegrid_swc(vec4D_grid,l_realgridwndsz,qPrintable(qs_filename_swc_grid)))
		{
			printf("ERROR: q_savegrid_swc() return false!\n");
			releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	printf(">>Free memory\n");
	releasememory(sz_img_tar,sz_img_sub,p_img_tar_input,p_img_sub_input,p_img64f_tar,p_img64f_sub,p_img64f_sub_bk,p_img64f_tar_1c,p_img64f_sub_1c,p_img64f_output_sub);

	printf(">>Program exit success!\n");
	return 0;
}

//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: q_nonrigid_registration -t <imageFilename_target> -s <imageFilename_subject> -o <imageFilename_sub2tar>\n");
	printf("\t -t <imageFilename_target>	input target image file full name.\n");
	printf("\t -s <imageFilename_subject>	input subject image file full name.\n");
	printf("\t -o <imageFilename_sub2tar>	output warped subject image file full name.\n");
	printf("\n");
	printf("\t [-r] <reference_channel>     reference channel used to do registration (default 0).\n");
	printf("\t             0~2: first, second and third channel\n");
	printf("\t             9:   average all channels --> for Rex's AQ1336 and XL138 data\n");
	printf("\t [-l] <hierarchicial_level>	hierarchicial level (default 3).\n");
	printf("\t [-d] <downsample_ratio>	    image downsample ratio (default 2).\n");
	printf("\t [-w] <meshgrid_window_size>	meshgrid window size (default 10).\n");
	printf("\t [-n] <do_histogram_matching> indicate whether do histogram matching before registration (default 1).\n");
	printf("\t [-i] <initial_grid>          used to pre-warp subject image.\n");
	printf("\t [-g] <gridFilename_swc>      output deformed meshgrid file name (default 0).\n");
	printf("\t [-h]	print this message.\n");
	return;
}

//release all allocated memory
void releasememory(long *&sz_img_tar,long *&sz_img_sub,
		unsigned char *&p_img_tar_input,unsigned char *&p_img_sub_input,
		double *&p_img64f_tar,double *&p_img64f_sub,
		double *&p_img64f_sub_bk,
		double *&p_img64f_tar_1c,double *&p_img64f_sub_1c,
		double *&p_img64f_output_sub)
{
	if(p_img_tar_input) 	{delete []p_img_tar_input;		p_img_tar_input=0;}
	if(p_img_sub_input) 	{delete []p_img_sub_input;		p_img_sub_input=0;}
	if(p_img64f_tar) 		{delete []p_img64f_tar;			p_img64f_tar=0;}
	if(p_img64f_sub) 		{delete []p_img64f_sub;			p_img64f_sub=0;}
	if(p_img64f_sub_bk)		{delete []p_img64f_sub_bk;		p_img64f_sub_bk=0;}
	if(p_img64f_tar_1c)		{delete []p_img64f_tar_1c;		p_img64f_tar_1c=0;}
	if(p_img64f_sub_1c)		{delete []p_img64f_sub_1c;		p_img64f_sub_1c=0;}
	if(p_img64f_output_sub) {delete []p_img64f_output_sub;	p_img64f_output_sub=0;}
	if(sz_img_tar) 			{delete []sz_img_tar;			sz_img_tar=0;}
	if(sz_img_sub) 			{delete []sz_img_sub;			sz_img_sub=0;}
}
