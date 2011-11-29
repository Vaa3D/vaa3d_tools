//main_atlasguided_stranno.cpp
// by Lei Qu
//2010-07-30

#include <QtGui>
#include <stdio.h>
#include <unistd.h>
extern char *optarg;
extern int optind, opterr;
#include <vector>
using namespace std;
#define WANT_STREAM
#include "newmatap.h"
#include "newmatio.h"

#include "stackutil.h"
#include "basic_surf_objs.h"
#include "../../v3d_main/jba/c++/convert_type2uint8.h"
#include "../celegans_straighten/q_celegans_straighten.h"
#include "q_atlasguided_seganno.h"

void printHelp();
bool readCelloi_file(const QString &qs_filename,QList<QString> &ql_celloi);
bool writeInfo_file(const QString &qs_filename,const COutputInfo &outputinfo);

CControlPanel* CControlPanel::panel=0;

int main(int argc, char *argv[])
{
	if(argc<=1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//initialize parameters
	CSParas paras_str;
	//image and marker
	paras_str.b_imgfromV3D=0;
	paras_str.b_markerfromV3D=0;
	QString qs_filename_img				=NULL;
	QString qs_filename_marker_input	=NULL;
	QString qs_filename_strimg_output	=NULL;

	CParas paras_anno;
	//image and atlas
	QString qs_filename_atals_input		=NULL;
	QString qs_filename_celloi_input	=NULL;
	QString qs_filename_atals_output	=NULL;
	QString qs_filename_seglabel_output	=NULL;
	QString qs_filename_info_output		=NULL;
	//visualization
	paras_anno.b_showatlas=0;
	paras_anno.b_showsegmentation=0;
	paras_anno.b_stepwise=0;
	//initial align
	paras_anno.d_fgthresh_factor=3;
	paras_anno.l_refchannel=0;
	paras_anno.d_downsampleratio=3;
	paras_anno.d_T=20;
	paras_anno.d_T_min=0.2;
	paras_anno.d_annealingrate=0.95;
	paras_anno.l_niter_pertemp=2;
	//refine align
	paras_anno.b_ref_simplealign=1;
	paras_anno.d_ref_T=0.1;
	paras_anno.l_ref_cellradius=8;
	paras_anno.l_ref_maxiter=100;
	paras_anno.d_ref_minposchange=1.0;
	//mode
	paras_anno.l_mode=-1;

	//------------------------------------------------------------------------------------------------------------------------------------
	int c;
	static char optstring[]="ht:a:i:m:c:f:d:o:s:I:";
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
				qs_filename_img.append(optarg);
				break;
			case 'a':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -a.\n");
					return 1;
				}
				qs_filename_atals_input.append(optarg);
				break;
			case 'i':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				qs_filename_celloi_input.append(optarg);
				break;
			case 'm':
				if (strcmp (optarg, "(null)") == 0)
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				paras_anno.l_mode=atol(optarg);
				break;
			case 'c':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				paras_anno.l_refchannel=atol(optarg);
				break;
			case 'f':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -f.\n");
					return 1;
				}
				paras_anno.d_fgthresh_factor=atof(optarg);
				break;
			case 'd':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				paras_anno.d_downsampleratio=atof(optarg);
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				qs_filename_atals_output.append(optarg);
				break;
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				qs_filename_seglabel_output.append(optarg);
				break;
			case 'I':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -I.\n");
					return 1;
				}
				qs_filename_info_output.append(optarg);
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
    }

	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>atlas guided celegans cell annotation:\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>  input image file:                  %s\n",qPrintable(qs_filename_img));
	printf(">>  input atlas file:                  %s\n",qPrintable(qs_filename_atals_input));
	printf(">>  input interesting cell:            %s\n",qPrintable(qs_filename_celloi_input));
	printf(">>  mode of annotation:                %ld\n",paras_anno.l_mode);
	printf(">>  reference channel:                 %ld\n",paras_anno.l_refchannel);
	printf(">>  foreground threshold factor:       %f\n",paras_anno.d_fgthresh_factor);
	printf(">>  downsample ratio:                  %f\n",paras_anno.d_downsampleratio);
	printf(">>-------------------------\n");
	printf(">>output parameters:\n");
	printf(">>  output atlas file:                 %s\n",qPrintable(qs_filename_atals_output));
	printf(">>  output seglabel file:              %s\n",qPrintable(qs_filename_seglabel_output));
	printf(">>  output additional info file:       %s\n",qPrintable(qs_filename_info_output));
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");


	void *callback;
	bool b_strimg=1;
	if(qs_filename_marker_input.isEmpty()) b_strimg=0;

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1. Read image. \n");
	unsigned char *p_img_input=0;
	long *sz_img_input=0;
	int datatype_input=0;
	if(qs_filename_img.isEmpty())
	{
		v3d_msg(QString("invalid image path!"));
		return false;
	}
	if(!loadImage((char *)qPrintable(qs_filename_img),p_img_input,sz_img_input,datatype_input))
	{
		v3d_msg(QString("open file [%1] failed!").arg(qs_filename_img));
		return false;
	}
	printf("\t\timage size: [w=%ld, h=%ld, z=%ld, c=%ld]\n",sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);
	printf("\t\tdatatype: %d\n",datatype_input);

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("1.Convert image datatype to uint8. \n");
	unsigned char * p_img_8u=0;
	{
	long l_npixels=sz_img_input[0]*sz_img_input[1]*sz_img_input[2]*sz_img_input[3];
	p_img_8u=new unsigned char[l_npixels];
	if(!p_img_8u)
	{
		printf("ERROR: Fail to allocate memory. Do nothing. \n");
		if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
		return false;
	}
	if(datatype_input==1)
	{
		printf("\t>>convert image data from uint8 to uint8. \n");
		for(long i=0;i<l_npixels;i++)
			p_img_8u[i]=p_img_input[i];
	}
	else if(datatype_input==2)
	{
		printf("\t>>convert image data from uint16 to uint8. \n");
		double min,max;
		if(!rescale_to_0_255_and_copy((unsigned short int *)p_img_input,l_npixels,min,max,p_img_8u))
		{
			printf("ERROR: rescale_to_0_255_and_copy() return false.\n");
			if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return false;
		}
	}
	else if(datatype_input==3)
	{
		printf("\t>>convert image data from float to uint8. \n");
		double min,max;
		if(!rescale_to_0_255_and_copy((float *)p_img_input,l_npixels,min,max,p_img_8u))
		{
			printf("ERROR: rescale_to_0_255_and_copy() return false.\n");
			if(p_img_8u)								{delete []p_img_8u;			p_img_8u=0;}
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return false;
		}
	}
	else
	{
		v3d_msg(QString("Unknown datatype!\n"));
		if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
		if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
		return false;
	}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("2. Import markers for straighten. \n");
	vector< vector<double> > vec2d_markers;
	if(b_strimg)
	{
		QList<ImageMarker> ql_markers=readMarker_file(qs_filename_marker_input);
		printf("\t>>read %d markers from file: %s.\n",ql_markers.size(),qPrintable(qs_filename_marker_input));

		vector<double> vec_marker(3,0);
		for(long i=0;i<ql_markers.size();i++)
		{
			vec_marker[0]=ql_markers[i].x;
			vec_marker[1]=ql_markers[i].y;
			vec_marker[2]=ql_markers[i].z;
			vec2d_markers.push_back(vec_marker);
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("3. Read atlas apo file. \n");
	QList<CellAPO> ql_atlasapo;
	ql_atlasapo=readAPO_file(qs_filename_atals_input);
	printf("\t>>read %d points from [%s]\n",ql_atlasapo.size(),qPrintable(qs_filename_atals_input));
	if(ql_atlasapo.size()<=0)
	{
		v3d_msg(QString("Given atlas file is empty or invalid!"));
		if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
		if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input) 							{delete []sz_img_input;		sz_img_input=0;}
		return false;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("4. Read interesting cell file. \n");
	QList<QString> ql_celloi;
	if(!readCelloi_file(qs_filename_celloi_input,ql_celloi))
	{
		printf("ERROR: readCelloi_file() return false! \n");
		if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
		if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
		if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
		return false;
	}

	printf("\t>>interesting cell:\n");
	for(long i=0;i<ql_celloi.size();i++)
		printf("\t\t%s\n",qPrintable(ql_celloi[i]));

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("5. Do Straightening. \n");
	unsigned char *p_strimg=0;
	long *sz_strimg=0;
	vector< vector< vector< vector<long> > > > vec4d_mappingfield_str2ori;

	if(vec2d_markers.size()<2)
	{
		printf("\t>>marker num < 2, skip straightening.\n");
	}
	else
	{
		printf("\t>>marker num >= 2, do straightening.\n");

		long l_width=paras_str.l_radius_cuttingplane*2+1;
		QList<ImageMarker> ql_marker;
		for(long i=0;i<vec2d_markers.size();i++)
		{
			ImageMarker tmp;
			tmp.x=vec2d_markers[i][0];
			tmp.y=vec2d_markers[i][1];
			tmp.z=vec2d_markers[i][2];
			ql_marker.push_back(tmp);
		}

		if(!q_celegans_restacking_xy(
				p_img_8u,sz_img_input,
				ql_marker,l_width,
				p_strimg,sz_strimg,
				vec4d_mappingfield_str2ori))
		{
			printf("ERROR: q_celegans_restacking_xy() return false! \n");
			if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input)						 	{delete []sz_img_input;		sz_img_input=0;}
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("6. Do Annotation. \n");
	QList<CellAPO> ql_musclecell_output;
	unsigned char *p_img8u_seglabel=0;
	COutputInfo outputinfo;

	{
	unsigned char *p_img_anno=0;
	long *sz_img_anno=0;
	if(vec2d_markers.size()<2)	//on non-straightened image
	{
		p_img_anno=p_img_input;
		sz_img_anno=sz_img_input;
	}
	else						//on straightened image
	{
		p_img_anno=p_strimg;
		sz_img_anno=sz_strimg;
	}

	if(paras_anno.l_mode==-1)	//non-partial annotation
	{
		if(!q_atlas2image(paras_anno,(V3DPluginCallback&)callback,
				p_img_anno,sz_img_anno,ql_atlasapo,ql_celloi,
				ql_musclecell_output,p_img8u_seglabel,outputinfo))
		{
			printf("ERROR: q_atlas2image() return false!\n");
			if(p_strimg)								{delete []p_strimg; 		p_strimg=0;}
			if(sz_strimg) 								{delete []sz_strimg;		sz_strimg=0;}
			if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input) 							{delete []sz_img_input;		sz_img_input=0;}
			return false;
		}

	}
	else			//partial annotation
	{
		if(!q_atlas2image_partial(paras_anno,(V3DPluginCallback&)callback,
				p_img_anno,sz_img_anno,ql_atlasapo,ql_celloi,
				ql_musclecell_output,outputinfo))
		{
			printf("ERROR: q_atlas2image_partial() return false!\n");
			if(p_strimg)								{delete []p_strimg; 		p_strimg=0;}
			if(sz_strimg) 								{delete []sz_strimg;		sz_strimg=0;}
			if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
			if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
			if(sz_img_input) 							{delete []sz_img_input;		sz_img_input=0;}
			return false;
		}
	}
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf("7. Map the annotated cell back to non-straightened image and save. \n");
	QList<CellAPO> ql_musclecell_output_ori(ql_musclecell_output);

	//map back to non-straightened image
	if(vec2d_markers.size()>=2)
	{
		for(long i=0;i<ql_musclecell_output.size();i++)
		{
			long x=ql_musclecell_output[i].x;
			long y=ql_musclecell_output[i].y;
			long z=ql_musclecell_output[i].z;
			ql_musclecell_output_ori[i].x=vec4d_mappingfield_str2ori[y][x][z][0];
			ql_musclecell_output_ori[i].y=vec4d_mappingfield_str2ori[y][x][z][1];
			ql_musclecell_output_ori[i].z=vec4d_mappingfield_str2ori[y][x][z][2];
		}
	}

	//save deformed point cloud to apo file
	if(!qs_filename_atals_output.isEmpty())
		writeAPO_file(qPrintable(qs_filename_atals_output),ql_musclecell_output_ori);

	//save segmentation label mask image to raw file
	if(!qs_filename_seglabel_output.isEmpty())
	{
		long sz_seglabelimg[4]={sz_img_input[0],sz_img_input[1],sz_img_input[2],1};
		saveImage(qPrintable(qs_filename_seglabel_output),p_img8u_seglabel,sz_seglabelimg,1);
	}

	//write additional info file
	if(!qs_filename_info_output.isEmpty())
		writeInfo_file(qs_filename_info_output,outputinfo);

	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	printf(">>Free memory\n");
	if(p_img8u_seglabel)						{delete []p_img8u_seglabel; p_img8u_seglabel=0;}
	if(p_strimg)								{delete []p_strimg; 		p_strimg=0;}
	if(sz_strimg) 								{delete []sz_strimg;		sz_strimg=0;}
	if(p_img_8u) 								{delete []p_img_8u;			p_img_8u=0;}
	if(p_img_input && !paras_str.b_imgfromV3D) 	{delete []p_img_input;		p_img_input=0;}
	if(sz_img_input) 							{delete []sz_img_input;		sz_img_input=0;}

	//------------------------------------------------------------------------------------------------------------------------------------
	printf(">>Program exit successful!\n");
	return 0;
}


void printHelp()
{
	//pirnt help messages
	printf("\nUsage: main_atlasguided_stranno\n");
	printf("Input paras:\n");
	printf("\t  -t   input image filename.\n");
	printf("\t  -a   input atlas filename - apo.\n");
	printf("\t  -i   input interesting cell name file - txt.\n");
	printf("\t [-m]  annotation mode (default -1)\n");
	printf("\t       -1: atlasguided_stranno \n");
	printf("\t        0: atlasguided_stranno_partial\n");
	printf("\t        1: grid2image\n");
	printf("\t        2: atlas2image_affine_beforecrop\n");
	printf("\t        3: atlas2image_affine_aftercrop\n");
	printf("\t [-c]  interesting channel (default 1)\n");
        printf("\t       (1:red, 2:green, 3:blue)\n");
	printf("\t [-f]  foreground threshold factor (default 2)\n");
	printf("\t [-d]  downsample ratio (default 3)\n");
	printf("Output paras:\n");
	printf("\t [-o]  output deformed cells filename - apo.\n");
	printf("\t [-s]  output segmentation labeled mask image filename.\n");
	printf("\t [-I]  output additional info filename - txt.\n");
	printf("\n");
	printf("\t [-h]	print this message.\n");	//read arguments
	return;
}

bool readCelloi_file(const QString &qs_filename,QList<QString> &ql_celloi)
{
	//check paras
	if(qs_filename.isEmpty())
	{
		printf("ERROR: Invalid input file name.\n");
		return false;
	}
	if(!ql_celloi.isEmpty())
	{
		printf("WARNING: ql_celloi is not empty, original data will be deleted.\n");
		ql_celloi.clear();
	}

	QFile qf_file(qs_filename);
	if(!qf_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		printf("ERROR: open file error!\n");
		return false;
	}

    while(!qf_file.atEnd())
    {
		char curline[2000];
        qf_file.readLine(curline, sizeof(curline));

		QString qs_cellname=QString(curline).simplified().toUpper();
		if(qs_cellname.isEmpty())   	continue;
		if(qs_cellname.contains("#")) 	continue;

		ql_celloi.push_back(qs_cellname);
    }

	return true;
}

bool writeInfo_file(const QString &qs_filename,const COutputInfo &outputinfo)
{
	//check paras
	if(qs_filename.isEmpty())
	{
		printf("ERROR: Invalid input file name.\n");
		return false;
	}

	FILE *p_file=fopen(qs_filename.toAscii(), "wt");
	if(!p_file)
	{
		printf("ERROR: Fail to save info to file!\n");
		return false;
	}

	fprintf(p_file,"b_rotate90=%d\n",outputinfo.b_rotate90);
	fprintf(p_file,"b_flip=%d\n",outputinfo.b_flip);

	fclose(p_file);

	return true;
}
