//main_tpswarp_quicksmallmemory.cpp
//
// by Lei Qu
//2012-07-08

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
extern char *optarg;
extern int optind, opterr;
#define WANT_STREAM
#include "../newmat11/newmatap.h"
#include "../newmat11/newmatio.h"

#include "../../basic_c_fun/stackutil.h"
#include "../../basic_c_fun/basic_surf_objs.h"
#include "jba_mainfunc.h"
#include "img_definition.h"

#include "q_imgwarp_tps_quicksmallmemory.cpp"

void printHelp();


int main(int argc, char *argv[])
{
	V3DLONG time_total_start,time_warp_start;
	time_total_start=clock();

	if(argc<=1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//read arguments
	//input
	QString qs_filename_tar					=NULL;
	QString qs_filename_sub					=NULL;
	QString qs_filename_marker_tar			=NULL;
	QString qs_filename_marker_sub			=NULL;
	int		i_interpmethod_df				=1;		//default B-spline
	int		i_interpmethod_img				=0;		//default trilinear
	bool 	b_padding_img					=0;
	//output
	QString qs_filename_warp				=NULL;

	int c;
	static char optstring[]="hS:t:s:o:I:i:p:";
	opterr=0;
	while((c=getopt(argc,argv,optstring))!=-1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;
			case 'S':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -S.\n");
					return 1;
				}
				qs_filename_sub.append(optarg);
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
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				qs_filename_warp.append(optarg);
				break;
			case 'I':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -I.\n");
					return 1;
				}
				i_interpmethod_df=atoi(optarg);
				break;
			case 'i':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				i_interpmethod_img=atoi(optarg);
				break;
			case 'p':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -p.\n");
					return 1;
				}
				b_padding_img=atoi(optarg);
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
    }

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>Quick small image warping:\n");
	printf(">> 	quick <- TPS+linear\n");
	printf(">>	small <- warp block by block\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input subject image:           %s\n",qPrintable(qs_filename_sub));
	printf(">>  input target  marker:          %s\n",qPrintable(qs_filename_marker_tar));
	printf(">>  input subject marker:          %s\n",qPrintable(qs_filename_marker_sub));
	printf(">>  DF  interp method:             %d\n",i_interpmethod_df);
	printf(">>  img interp method:             %d\n",i_interpmethod_img);
	printf(">>  padding image?:                %d\n",b_padding_img);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output warped image:           %s\n",qPrintable(qs_filename_warp));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");


	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read target and subject marker files. \n");
	QList<ImageMarker> ql_marker_tar,ql_marker_sub;
	if(qs_filename_marker_tar.endsWith(".marker") && qs_filename_marker_sub.endsWith(".marker"))
    {
		ql_marker_tar=readMarker_file(qs_filename_marker_tar);
		ql_marker_sub=readMarker_file(qs_filename_marker_sub);
    	printf("\t>>Target: read %d markers from [%s]\n",ql_marker_tar.size(),qPrintable(qs_filename_marker_tar));
    	printf("\t>>Subject:read %d markers from [%s]\n",ql_marker_sub.size(),qPrintable(qs_filename_marker_sub));

    	if(ql_marker_tar.size()==0 || ql_marker_tar.size()!=ql_marker_sub.size())
        {
        	printf("ERROR: marker number not equal or has zero marker.\n");
        	return false;
    	}
    }
    else
    {
    	printf("ERROR: at least one marker file is invalid.\n");
    	return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Read subject image. \n");
	unsigned char *p_img_sub=0;
	V3DLONG *sz_img_sub=0;
	int datatype_sub=0;
	if(!loadImage((char *)qPrintable(qs_filename_sub),p_img_sub,sz_img_sub,datatype_sub))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_sub));
		return false;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_sub));
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_sub[0],sz_img_sub[1],sz_img_sub[2],sz_img_sub[3]);
	printf("\t\tdatatype: %d\n",datatype_sub);

	if(datatype_sub!=1 && datatype_sub!=2 && datatype_sub!=4)
	{
    	printf("ERROR: Input image datatype is not valid, return.\n");
    	if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
    	if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}
    	return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. warp the subject image block by block. \n");
	time_warp_start=clock();
	unsigned char *p_img_warp=0;
	V3DLONG szBlock_x,szBlock_y,szBlock_z;
	szBlock_x=szBlock_y=szBlock_z=4;
    
	bool b_status=false;
    if(datatype_sub==1)
    {
    	if(!b_padding_img)
    		b_status=imgwarp_smallmemory(p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_df,i_interpmethod_img,p_img_warp);
    	else
    		b_status=imgwarp_smallmemory_padding(p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,p_img_warp);
    }
    else if(datatype_sub==2)
    {
    	if(!b_padding_img)
    		b_status=imgwarp_smallmemory((unsigned short int *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_df,i_interpmethod_img,(unsigned short int *&)p_img_warp);
    	else
    		b_status=imgwarp_smallmemory_padding((unsigned short int *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,(unsigned short int *&)p_img_warp);
    }
    else if(datatype_sub==4)
    {
    	if(!b_padding_img)
    		b_status=imgwarp_smallmemory((float *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_df,i_interpmethod_img,(float *&)p_img_warp);
    	else
    		b_status=imgwarp_smallmemory_padding((float *)p_img_sub,sz_img_sub,ql_marker_tar,ql_marker_sub,szBlock_x,szBlock_y,szBlock_z,i_interpmethod_img,(float *&)p_img_warp);
    }
    if(!b_status)
	{
		printf("ERROR: imgwarp_smallmemory() return false.\n");
		if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
		if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}
		return false;
	}
	printf(">>>>The warping process took %f seconds\n",((float)(clock()-time_warp_start))/CLOCKS_PER_SEC);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. save warped image to file: [%s]\n",qPrintable(qs_filename_warp));
	if(!saveImage(qPrintable(qs_filename_warp),p_img_warp,sz_img_sub,datatype_sub))
	{
		printf("ERROR: saveImage() return false.\n");
		if(p_img_warp) 			{delete []p_img_warp;		p_img_warp=0;}
	   	if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
		if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. free memory. \n");
	if(p_img_warp) 			{delete []p_img_warp;		p_img_warp=0;}
	if(p_img_sub) 			{delete []p_img_sub;		p_img_sub=0;}
	if(sz_img_sub) 			{delete []sz_img_sub;		sz_img_sub=0;}

	printf(">>>>The whole process took %f seconds\n",((float)(clock()-time_total_start))/CLOCKS_PER_SEC);
	printf("Program exit success.\n");
	return true;
}

//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_imgwarp_tps_quicksmallmemory\n");
	printf("Input paras:\n");
	printf("\t  -S   <filename_sub>	            input subject (image) file full name.\n");
	printf("\t  -t   <markerFilename_target>	input target marker file full name.\n");
	printf("\t  -s   <markerFilename_subject>	input subject marker file full name.\n");
	printf("\t  -I   <DF_interp_method>	        subsampled displace field interpolation method (default 1).\n");
	printf("\t                       	        0: trilinear interpolation\n");
	printf("\t                       	        1: B-spline  interpolation\n");
	printf("\t  -i   <img_interp_method>	    image value interpolation method (default 1).\n");
	printf("\t                       	        0: trilinear interpolation\n");
	printf("\t                       	        1: nearest neighbor interpolation\n");
	printf("\t [-p]  <b_padding_img>			padding is just for generating the same result as jba (default 0).\n");
	printf("Output paras:\n");
	printf("\t [-o]  <filename_sub2tar>         output sub2tar tps warped (image) file full name.\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");
	return;
}


