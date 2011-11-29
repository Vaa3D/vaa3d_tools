//main_principalskeleton_detection_domain.cpp
//deform the principal skeleton based on given image(.tif) + control points position(.marker) + domain and corresponding weight definition (.domain)
//the shape prior of principal skeleton is defined in .marker file and .domain file
//by Lei Qu
//2009-12-2
 
#ifndef DISABLE_V3D_MSG
	#define DISABLE_V3D_MSG
#endif

#include <stdio.h>
#include <vector>
using namespace std;

#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

#include "../../basic_c_fun/stackutil.h"
#include "../../basic_c_fun/basic_surf_objs.h"
#include "../../worm_straighten_c/spline_cubic.h"
#include "q_principalskeleton_detection.h"

void printHelp();
bool readDomain_file(const QString &qs_filename,
		vector< vector<long> > &vecvec_domain_length_ind,vector<double> &vec_domain_length_weight,
		vector< vector<long> > &vecvec_domain_smooth_ind,vector<double> &vec_domain_smooth_weight);
//save cubic spline interpolated larva skeleton to swc file
bool q_saveSkeleton2swcFile(const QList<ImageMarker> &ql_cptpos,const vector< vector<long> > &vecvec_domain_cptind,const QString &qs_filename_swc_skeleton_output);
//save cubic spline interpolated larva skeleton to swc file
bool q_saveSkeleton2swcFile_cubicspline(const QList<ImageMarker> &ql_cptpos,const vector< vector<long> > &vecvec_domain_cptind,const QString &qs_filename_swc_cubicspline_skeleton_output);
//cubic spline interpolate given marker series
bool q_cubicSplineMarker(const QList<ImageMarker> &ql_marker,QList<ImageMarker> &ql_marker_cubicspline);

int main(int argc, char *argv[])
{
//	QList<ImageMarker> ql_cptpos;
//	vector< vector<long> > vecvec_domain_cptind;
//	ImageMarker im;
//	im.x=496.215;	im.y=983.090;	ql_cptpos.push_back(im);
//	im.x=478.375;	im.y=812.091;	ql_cptpos.push_back(im);
//	im.x=458.128;	im.y=648.109;	ql_cptpos.push_back(im);
//	im.x=438.501;	im.y=472.245;	ql_cptpos.push_back(im);
//	im.x=415.634;	im.y=269.149;	ql_cptpos.push_back(im);
//	im.x=496.215;	im.y=983.090;	ql_cptpos.push_back(im);
//
//	im.x=496.215;	im.y=983.090;	ql_cptpos.push_back(im);
//	im.x=478.375;	im.y=812.091;	ql_cptpos.push_back(im);
//	im.x=458.128;	im.y=648.109;	ql_cptpos.push_back(im);
//	im.x=438.501;	im.y=472.245;	ql_cptpos.push_back(im);
//	im.x=415.634;	im.y=269.149;	ql_cptpos.push_back(im);
//	vector<long> vec_domain_cptind;
//	vec_domain_cptind.push_back(6);
//	vec_domain_cptind.push_back(7);
//	vec_domain_cptind.push_back(8);
//	vec_domain_cptind.push_back(9);
//	vec_domain_cptind.push_back(10);
//	vecvec_domain_cptind.push_back(vec_domain_cptind);
//
//	q_saveSkeleton2swcFile_cubicspline(ql_cptpos,vecvec_domain_cptind,"/Users/qul/work/v3d_2.0/plugin_demo/principalskeleton_detection/data/test.swc");



	if (argc <= 1)
	{
		printHelp();
		return 0;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//read arguments
	QString qs_filename_input_img			=NULL;
	QString qs_filename_input_marker		=NULL;
	QString qs_filename_input_domain		=NULL;
	QString qs_filename_output_marker		=NULL;
	QString qs_filename_output_swc			=NULL;
	QString qs_filename_output_swc_cspline	=NULL;
	QString qs_flag_output;
	int 	n_index_channel=2;
	double 	d_inizoomfactor_skeleton=1.0;

	int c;
	static char optstring[]="hi:m:d:c:s:f:o:w:W:";
	opterr=0;
	while((c=getopt(argc,argv,optstring))!=-1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;
			case 'i':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				qs_filename_input_img.append(optarg);
				break;
			case 'm':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				qs_filename_input_marker.append(optarg);
				break;
			case 'd':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				qs_filename_input_domain.append(optarg);
				break;
			case 'c':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				n_index_channel=atoi(optarg);
				break;
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				d_inizoomfactor_skeleton=atof(optarg);
				break;
			case 'f':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -f.\n");
					return 1;
				}
				qs_flag_output.append(optarg);
				break;
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				qs_filename_output_marker.append(optarg);
				break;
			case 'w':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -w.\n");
					return 1;
				}
				qs_filename_output_swc.append(optarg);
				break;
			case 'W':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -W.\n");
					return 1;
				}
				qs_filename_output_swc_cspline.append(optarg);
				break;
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 1;
				break;
		}
    }

	//define default output directory (same as the image directory)
	if(qs_filename_input_img.length()!=0 && qs_filename_output_marker.length()==0)
	{
		QString qs_path=qs_filename_input_img.left(qs_filename_input_img.lastIndexOf('.'));
		qs_filename_output_marker=qs_path+QString("_skeleton.marker");
	}
	if(qs_filename_input_img.length()!=0 && qs_filename_output_swc.length()==0)
	{
		QString qs_path=qs_filename_input_img.left(qs_filename_input_img.lastIndexOf('.'));
		qs_filename_output_swc=qs_path+QString("_skeleton.swc");
	}
	if(qs_filename_input_img.length()!=0 && qs_filename_output_swc_cspline.length()==0)
	{
		QString qs_path=qs_filename_input_img.left(qs_filename_input_img.lastIndexOf('.'));
		qs_filename_output_swc_cspline=qs_path+QString("_skeleton_cspline.swc");
	}

	//fill ouput flat array
	bool b_flag_ouput[10]={0};
	for(int ind_flagoutput=0;ind_flagoutput<qs_flag_output.length();ind_flagoutput++)
	{
		if(qs_flag_output[ind_flagoutput]=='1')
			b_flag_ouput[1]=1;
		if(qs_flag_output[ind_flagoutput]=='2')
			b_flag_ouput[2]=1;
		if(qs_flag_output[ind_flagoutput]=='3')
			b_flag_ouput[3]=1;
		if(qs_flag_output[ind_flagoutput]=='9')
			b_flag_ouput[9]=1;
	}


	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>principal skeleton detection\n");
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	printf(">>input parameters:\n");
	printf(">>input image:             %s\n",qPrintable(qs_filename_input_img));
	printf(">>input cpt position:      %s\n",qPrintable(qs_filename_input_marker));
	printf(">>input domain definition: %s\n",qPrintable(qs_filename_input_domain));
	printf(">>initial skeleton zoom:   %f\n",d_inizoomfactor_skeleton);
	printf(">>-------------------------\n");
	printf(">>output flag:             %s\n",qPrintable(qs_flag_output));
	printf(">>output cpt position:     %s\n",qPrintable(qs_filename_output_marker));
	printf(">>output swc position:     %s\n",qPrintable(qs_filename_output_swc));
	printf(">>output swc_cspline position:%s\n",qPrintable(qs_filename_output_swc_cspline));

	//------------------------------------------------------------------------------------------------------------------------------------
	//read image
	unsigned char *p_img_input=0;
	long *sz_img_input=0;//[0]:width, [1]:height, [2]:z, [3]:nchannel
	int datatype_subject=0;
	if(!loadImage((char *)qPrintable(qs_filename_input_img),p_img_input,sz_img_input,datatype_subject))
	{
		printf("ERROR: loadImage() return false in loading [%s].\n", qPrintable(qs_filename_input_img));
		return false;
	}
	printf("\t>>read image file [%s] complete.\n",qPrintable(qs_filename_input_img));
	printf("\t\timage size: [w=%d, h=%d, z=%d, c=%d]\n",sz_img_input[0],sz_img_input[1],sz_img_input[2],sz_img_input[3]);

	//read marker file (initial skeleton control points position definition)
	QList<ImageMarker> ql_cptpos_input;
	ql_cptpos_input=readMarker_file(qs_filename_input_marker);
	printf("\t>>read marker file [%s] complete.\n",qPrintable(qs_filename_input_marker));
    if(ql_cptpos_input.isEmpty())
    {
    	printf("ERROR: read nothing from input skeleotn control points definition file.\n");
    	return false;
    }
    for(long i=0;i<ql_cptpos_input.size();i++)
    	printf("\t\tcpt[%d]=[%.2f,%.2f,%.2f]\n",i,ql_cptpos_input[i].x,ql_cptpos_input[i].y,ql_cptpos_input[i].z);

    //read domain file (include domain definition and corresponding weight definition)
    vector< vector<long> > vecvec_domain_length_ind,vecvec_domain_smooth_ind;	//the index of control point of each domain is refer to the corresponding marker file
    vector<double> vec_domain_length_weight,vec_domain_smooth_weight;
    if(!readDomain_file(qs_filename_input_domain,
    		vecvec_domain_length_ind,vec_domain_length_weight,
    		vecvec_domain_smooth_ind,vec_domain_smooth_weight))
    {
    	printf("ERROR: readDomain_file() return false!\n");
    	return false;
    }
    printf("\t>>read domain file [%s] complete.\n",qPrintable(qs_filename_input_domain));
    printf("\t\tdomain - length constraint:\n");
    for(long i=0;i<vecvec_domain_length_ind.size();i++)
    {
    	printf("\t\tweight=%.2f;\tcontol points index=[",vec_domain_length_weight[i]);
    	for(long j=0;j<vecvec_domain_length_ind[i].size();j++)
    		printf("%d,",vecvec_domain_length_ind[i][j]);
    	printf("]\n");
    }
    printf("\t\tdomain - smooth constraint:\n");
    for(long i=0;i<vecvec_domain_smooth_ind.size();i++)
    {
    	printf("\t\tweight=%.2f;\tcontol points index=[",vec_domain_smooth_weight[i]);
    	for(long j=0;j<vecvec_domain_smooth_ind[i].size();j++)
    		printf("%d,",vecvec_domain_smooth_ind[i][j]);
    	printf("]\n");
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    //generate MIP 2d image
    printf("\t>>generate MIP image ...\n");
    unsigned char *p_img_MIP=0;
    p_img_MIP=new unsigned char[sz_img_input[0]*sz_img_input[1]*sz_img_input[3]];
    if(!p_img_MIP)
    {
		printf("ERROR:Fail to allocate memory for the MIP image. \n");
    	if(p_img_input) 	{delete []p_img_input;		p_img_input=0;}
    	if(sz_img_input) 	{delete []sz_img_input;		sz_img_input=0;}
    	return false;
    }

	long pgsz1=sz_img_input[0];
	long pgsz2=sz_img_input[0]*sz_img_input[1];
	long pgsz3=sz_img_input[0]*sz_img_input[1]*sz_img_input[2];
	unsigned char u_MIP_rgb[3];
	for(long y=0;y<sz_img_input[1];y++)
		for(long x=0;x<sz_img_input[0];x++)
		{
			u_MIP_rgb[0]=u_MIP_rgb[1]=u_MIP_rgb[2]=0;
			for(long z=0;z<sz_img_input[2];z++)
				for(long c=0;c<sz_img_input[3];c++)
				{
					long index=pgsz3*c+pgsz2*z+pgsz1*y+x;
					if(p_img_input[index]>u_MIP_rgb[c])
						u_MIP_rgb[c]=p_img_input[index];
				}

			for(long c=0;c<sz_img_input[3];c++)
			{
				long index_MIP=pgsz2*c+pgsz1*y+x;
				p_img_MIP[index_MIP]=u_MIP_rgb[c];
			}
		}

	//------------------------------------------------------------------------------------------------------------------------------------
	//extract the reference channel
//	int n_index_channel=2;
    printf("\t>>extract the reference [%d]th channel from MIP image: ...\n",n_index_channel);

    unsigned char *p_img_MIP_ref=0;
    p_img_MIP_ref=new unsigned char[sz_img_input[0]*sz_img_input[1]];
    if(!p_img_MIP_ref)
    {
    	printf("ERROR: fail to allocate memory for reference image!\n");
    	if(p_img_input) 	{delete []p_img_input;		p_img_input=0;}
    	if(sz_img_input) 	{delete []sz_img_input;		sz_img_input=0;}
    	if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
    	return false;
    }

	for(long y=0;y<sz_img_input[1];y++)
		for(long x=0;x<sz_img_input[0];x++)
		{
			long index_MIP=pgsz2*n_index_channel+pgsz1*y+x;
			long index_MIP_1c=pgsz1*y+x;
			p_img_MIP_ref[index_MIP_1c]=p_img_MIP[index_MIP];
		}

	//------------------------------------------------------------------------------------------------------------------------------------
	//downsample 1 channel MIP image to given size (for speed) and modify the coordinate of skeleton accordingly
	double d_ratio_sample=2;//d_ratio_sample=sz_ori/sz_sample
//	double d_ratio_sample=1;//d_ratio_sample=sz_ori/sz_sample
	long sz_img_sample[2];	//[0]:width, [1]:height
	sz_img_sample[0]=sz_img_input[0]/d_ratio_sample+0.5;
	sz_img_sample[1]=sz_img_input[1]/d_ratio_sample+0.5;

	printf("\t>>resize image(to [w=%ld,h=%ld]) and skeleton ...\n",sz_img_sample[0],sz_img_sample[1]);
	//downsample 1 channel MIP image
    unsigned char *p_img_sample=0;
    p_img_sample=new unsigned char[sz_img_sample[0]*sz_img_sample[1]];
    if(!p_img_sample)
    {
    	printf("ERROR: fail to allocate memory for sample image!\n");
    	if(p_img_input) 	{delete []p_img_input;		p_img_input=0;}
    	if(sz_img_input) 	{delete []sz_img_input;		sz_img_input=0;}
    	if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
    	if(p_img_MIP_ref) 	{delete []p_img_MIP_ref;	p_img_MIP_ref=0;}
    	return false;
    }

	for(long y=0;y<sz_img_sample[1];y++)
		for(long x=0;x<sz_img_sample[0];x++)
		{
			long x_o=x*d_ratio_sample+0.5;
			long y_o=y*d_ratio_sample+0.5;
			x_o=x_o<0?0:x_o;	x_o=x_o>=sz_img_input[0]?sz_img_input[0]-1:x_o;
			y_o=y_o<0?0:y_o;	y_o=y_o>=sz_img_input[1]?sz_img_input[1]-1:y_o;

			long index_o=sz_img_input[0]*y_o+x_o;
			long index_s=sz_img_sample[0]*y+x;
			p_img_sample[index_s]=p_img_MIP_ref[index_o];
		}

	QList<ImageMarker> ql_cptpos_resize(ql_cptpos_input);
	for(long i=0;i<ql_cptpos_input.size();i++)
	{
		ql_cptpos_resize[i].x/=d_ratio_sample;
		ql_cptpos_resize[i].y/=d_ratio_sample;
		ql_cptpos_resize[i].z/=d_ratio_sample;
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	//principal skeleton detection
    //data structure convert
    vector<point3D64F> vec_cptpos_input,vec_cptpos_output;
    for(long i=0;i<ql_cptpos_input.size();i++)
    {
    	point3D64F temp;
    	temp.x=ql_cptpos_resize[i].x;
    	temp.y=ql_cptpos_resize[i].y;
    	temp.z=ql_cptpos_resize[i].z;
    	vec_cptpos_input.push_back(temp);
    }

    //principal skeleton detection parameters
    PSDParas paras_input;
    paras_input.d_inizoomfactor_skeleton=d_inizoomfactor_skeleton;
    paras_input.l_maxitertimes=500;
    paras_input.d_stopiter_threshold=0.01;
    paras_input.d_foreground_treshold=1;//larva:0.5, vnc:1
    paras_input.l_diskradius_openning=7;
    paras_input.l_diskradius_closing=7;
//    paras_input.l_diskradius_openning=5;
//    paras_input.l_diskradius_closing=5;

    //do principal skeleton detection
    if(!q_principalskeleton_detection(
    		p_img_sample,sz_img_sample,
    		vec_cptpos_input,
    		vecvec_domain_length_ind,vec_domain_length_weight,
    		vecvec_domain_smooth_ind,vec_domain_smooth_weight,
    		paras_input,
    		vec_cptpos_output))
    {
    	printf("ERROR:q_principalskeleton_detection() return false!\n");
    	if(p_img_input) 	{delete []p_img_input;		p_img_input=0;}
    	if(sz_img_input) 	{delete []sz_img_input;		sz_img_input=0;}
    	if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
    	if(p_img_MIP_ref) 	{delete []p_img_MIP_ref;	p_img_MIP_ref=0;}
    	if(p_img_sample) 	{delete []p_img_sample;		p_img_sample=0;}
    	return false;
    }

    //------------------------------------------------------------------------------------------------------------------------------------
    //output deform principal skeleton
	QList<ImageMarker> ql_cptpos_output(ql_cptpos_input);
	for(long i=0;i<vec_cptpos_output.size();i++)
	{
		ql_cptpos_output[i].x=vec_cptpos_output[i].x*d_ratio_sample;
		ql_cptpos_output[i].y=vec_cptpos_output[i].y*d_ratio_sample;
	}

	if(b_flag_ouput[1])
	{
		wirteMarker_file(qs_filename_output_marker,ql_cptpos_output);
	}
	if(b_flag_ouput[2])
	{
		q_saveSkeleton2swcFile(ql_cptpos_output,vecvec_domain_smooth_ind,qs_filename_output_swc);
	}
	if(b_flag_ouput[3])
	{
		q_saveSkeleton2swcFile_cubicspline(ql_cptpos_output,vecvec_domain_smooth_ind,qs_filename_output_swc_cspline);
	}


	//------------------------------------------------------------------------------------------------------------------------------------
	//free memory
	printf(">>Free memory\n");
	if(p_img_input) 	{delete []p_img_input;		p_img_input=0;}
	if(sz_img_input) 	{delete []sz_img_input;		sz_img_input=0;}
	if(p_img_MIP) 		{delete []p_img_MIP;		p_img_MIP=0;}
	if(p_img_MIP_ref) 	{delete []p_img_MIP_ref;	p_img_MIP_ref=0;}
	if(p_img_sample) 	{delete []p_img_sample;		p_img_sample=0;}


	printf(">>Program exit successful!\n");
	return 0;
}


//read domain definition for principal skeleton detection
bool readDomain_file(const QString &qs_filename,
		vector< vector<long> > &vecvec_domain_length_ind,vector<double> &vec_domain_length_weight,
		vector< vector<long> > &vecvec_domain_smooth_ind,vector<double> &vec_domain_smooth_weight)
{
	vecvec_domain_length_ind.clear();	vec_domain_length_weight.clear();
	vecvec_domain_smooth_ind.clear();	vec_domain_smooth_weight.clear();

	QFile qf(qs_filename);
	if(!qf.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		printf("ERROR: open file [%s] fail.\n",qPrintable(qs_filename));
		return false;
	}

	long k=0;
    while(!qf.atEnd())
    {
		char curline[2000];
        qf.readLine(curline,sizeof(curline));
		k++;
		{
			if(curline[0]=='#' || curline[0]=='\n' || curline[0]=='\0') continue;

			QStringList qsl=QString(curline).trimmed().split(",");
			int qsl_count=qsl.size();
			if(qsl_count<=3)
			{
				printf("WARNING: invalid format found in line %d.\n",k);
				continue;
			}

			if(qsl[1].trimmed().toLower()=="length")
			{
				vec_domain_length_weight.push_back(qsl[2].toDouble());

				vector<long> vec_domain_length_ind;
				for(long i=3;i<qsl.size();i++)
					vec_domain_length_ind.push_back(qsl[i].toLong());
				vecvec_domain_length_ind.push_back(vec_domain_length_ind);
			}
			else if(qsl[1].trimmed().toLower()=="smooth")
			{
				vec_domain_smooth_weight.push_back(qsl[2].toDouble());

				vector<long> vec_domain_smooth_ind;
				for(long i=3;i<qsl.size();i++)
					vec_domain_smooth_ind.push_back(qsl[i].toLong());
				vecvec_domain_smooth_ind.push_back(vec_domain_smooth_ind);
			}
			else
				printf("WARNING: unknown constraint type found in line %d.\n",k);
		}
    }

    return true;
}

//Printing Help of Usage of this Program
void printHelp()
{
	//pirnt help messageshi:m:d:f:o:w:W:";
	printf("\nUsage: main_principalskeleton_detection -i <*> -m <*> -d <*> -f <*> [-o] <*> [-w] <*> [-W] <*>\n\n");
	printf("\t -i   <imageFilename>             input image filename.\n");
	printf("\t -m   <markerFilename>            input marker filename which contain the positon of initial principal skeleton.\n");
	printf("\t -d   <domainFilename>            input domain definition file of principal skeleton.\n");
	printf("\t -c   <refChannel>                input the reference channel - 0:red, 1:green, 2:blue(default).\n");
	printf("\t -s   <iniSkeletonZoom>           initial skeleton zoom factor, for larva suggest 0.1, VNC 1.0. (default 1)- .\n");
	printf("\t -f   <flagOutput>                a combination of some number which indicate what file need to output.\n");
	printf("\t           1: output deformed principal skeleton to .marker file.\n");
	printf("\t           2: output deformed principal skeleton to .swc file.\n");
	printf("\t           3: output cubic-spline interpolated deformed skeleton to .swc file.\n");
	printf("\t           9: output all the intermidiate image to current directory(for debug).\n");
	printf("\t[-o]  <markerFilename_skeleton>   output marker filename of principal skeleton(default in image directory).\n");
	printf("\t[-w]  <swcFilename_skeleton>      output swc filename of skeleton(default in image directory).\n");
	printf("\t[-W]  <swcFilename_cubicspline_skeleton>   output swc filename of cubic-spline interpolated skeleton(default in image directory).\n");
	printf("\n");
	printf("\t[-h]  print this message.\n");
	return;
}

//save larva skeleton to swc file
bool q_saveSkeleton2swcFile(const QList<ImageMarker> &ql_cptpos,const vector< vector<long> > &vecvec_domain_cptind,const QString &qs_filename_swc_skeleton_output)
{
	//check parameters
	if(ql_cptpos.isEmpty())
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: input skeleton position array is empty.\n");
		return false;
	}
	if(vecvec_domain_cptind.size()==0)
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: input domain definition is empty.\n");
		return false;
	}
	if(qs_filename_swc_skeleton_output.length()==0)
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: output file name is empty.\n");
		return false;
	}

	NeuronTree nt_skeleton;
	NeuronSWC ns_marker;
	long index=0;
	for(long i=0;i<vecvec_domain_cptind.size();i++)
	{
		for(long j=0;j<vecvec_domain_cptind[i].size();j++)
		{
			index++;
			long l_cptind=vecvec_domain_cptind[i][j];

			ns_marker.n=index;					//index
			ns_marker.r=2;						//radius
			ns_marker.x=ql_cptpos[l_cptind].x;	//x
			ns_marker.y=ql_cptpos[l_cptind].y;	//y
			if(j==0)							//parent index
				ns_marker.pn=-1;
			else
				ns_marker.pn=index-1;

			nt_skeleton.listNeuron.push_back(ns_marker);
		}
	}
	writeSWC_file(qs_filename_swc_skeleton_output,nt_skeleton);

	return true;
}

//save cubic spline interpolated skeleton to swc file
bool q_saveSkeleton2swcFile_cubicspline(const QList<ImageMarker> &ql_cptpos,const vector< vector<long> > &vecvec_domain_cptind,const QString &qs_filename_swc_cubicspline_skeleton_output)
{
	//check parameters
	if(ql_cptpos.isEmpty())
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: input skeleton position array is empty.\n");
		return false;
	}
	if(vecvec_domain_cptind.size()==0)
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: input domain definition is empty.\n");
		return false;
	}
	if(qs_filename_swc_cubicspline_skeleton_output.length()==0)
	{
		printf("ERROR: saveLarvaSkeleton2swcFile: output file name is empty.\n");
		return false;
	}

	//reorgnize the skeleton into branches according to the domain defintion
	vector< QList<ImageMarker> > vec_ql_brances;
	for(long i=0;i<vecvec_domain_cptind.size();i++)
	{
		QList<ImageMarker> ql_brances;
		ImageMarker im;
		for(long j=0;j<vecvec_domain_cptind[i].size();j++)
		{
			long index=vecvec_domain_cptind[i][j];
			im.x=ql_cptpos[index].x;
			im.y=ql_cptpos[index].y;
			im.z=ql_cptpos[index].z;

			ql_brances.push_back(im);
		}
		vec_ql_brances.push_back(ql_brances);
	}

	//cubic spline interpolate every branch respectively
	vector< QList<ImageMarker> > vec_ql_brances_cpline;
	for(long i=0;i<vec_ql_brances.size();i++)
	{
		QList<ImageMarker> ql_brances_cpline;
		if(vec_ql_brances[i].size()==2)
		{
			ql_brances_cpline.push_back(vec_ql_brances[i][0]);
			ql_brances_cpline.push_back(vec_ql_brances[i][1]);
		}
		else if(vec_ql_brances[i].size()>2)
		{
			if(!q_cubicSplineMarker(vec_ql_brances[i],ql_brances_cpline))
			{
				printf("ERROR: q_cubicSplineMarker() return false.\n");
				return false;
			}
		}
		vec_ql_brances_cpline.push_back(ql_brances_cpline);
	}

	//save to swc file
	NeuronTree nt_skeleton;
	NeuronSWC ns_marker;
	long index=0;
	for(long i=0;i<vec_ql_brances_cpline.size();i++)
	{
		for(long j=0;j<vec_ql_brances_cpline[i].size();j++)
		{
			index++;

			ns_marker.n=index;					//index
			ns_marker.r=2;						//radius
			ns_marker.x=vec_ql_brances_cpline[i][j].x;	//x
			ns_marker.y=vec_ql_brances_cpline[i][j].y;	//y
			if(j==0)							//parent index
				ns_marker.pn=-1;
			else
				ns_marker.pn=index-1;

			nt_skeleton.listNeuron.push_back(ns_marker);
		}
	}
	writeSWC_file(qs_filename_swc_cubicspline_skeleton_output,nt_skeleton);

	return true;
}

//cubic spline interpolate given marker series
bool q_cubicSplineMarker(const QList<ImageMarker> &ql_marker,QList<ImageMarker> &ql_marker_cubicspline)
{
	//check parameters
	if(ql_marker.isEmpty())
	{
		printf("ERROR: q_cubicSplieMarker: input ql_marker is empty.\n");
		return false;
	}
	if(!ql_marker_cubicspline.isEmpty())
	{
		printf("WARNING: q_cubicSplieMarker: ouput ql_marker_cubicspline is not empty, previoud contents will be deleted.\n");
		ql_marker_cubicspline.clear();
		return false;
	}

	//estimate the cubic spline parameters for given markers
	parameterCubicSpline **cpara=0;
	double *xpos=0, *ypos=0, *zpos=0;
	long NPoints=ql_marker.size();
	xpos=new double[NPoints];
	ypos=new double[NPoints];
	zpos=new double[NPoints];
	if(!xpos || !ypos || !zpos)
	{
		printf("ERROR: q_cubicSplieMarker: Fail to allocate memory for cubic splin control points.\n");
		if(xpos) {delete []xpos; xpos=0;}
		if(ypos) {delete []ypos; ypos=0;}
		if(zpos) {delete []zpos; zpos=0;}
		return false;
	}
	for(int i=0;i<NPoints;i++)
	{
		xpos[i]=ql_marker.at(i).x;
		ypos[i]=ql_marker.at(i).y;
		zpos[i]=ql_marker.at(i).z;
	}
	cpara=est_cubic_spline_2d(xpos,ypos,NPoints,false);

	//cubic spline interpolate the head and butt markers(find all the interpolated locations on the backbone (1-pixel spacing))
	double *cp_x=0,*cp_y=0,*cp_z=0,*cp_alpha=0;
	long cutPlaneNum=0;
	if(!interpolate_cubic_spline(cpara,2,cp_x,cp_y,cp_z,cp_alpha,cutPlaneNum))
	{
		printf("ERROR: q_cubicSplieMarker: interpolate_cubic_spline() return false! \n");
		if(xpos) {delete []xpos; xpos=0;}
		if(ypos) {delete []ypos; ypos=0;}
		if(zpos) {delete []zpos; zpos=0;}
		if(cp_x) {delete []cp_x; cp_x=0;}
		if(cp_y) {delete []cp_y; cp_y=0;}
		if(cp_z) {delete []cp_z; cp_z=0;}
		if(cp_alpha) {delete []cp_alpha; cp_alpha=0;}
		if(cpara) //delete the cubic spline parameter data structure
		{
			for(int i=0;i<2;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
			delete []cpara; cpara=0;
		}
		return false;
	}

	//fill output structure
	ImageMarker imgmarker;
	for(long i=1;i<cutPlaneNum;i++)//should skip the first point since it is wrong!
	{
		imgmarker.x=cp_x[i];
		imgmarker.y=cp_y[i];
		ql_marker_cubicspline.push_back(imgmarker);
	}

	//free memory
	if(xpos) {delete []xpos; xpos=0;}
	if(ypos) {delete []ypos; ypos=0;}
	if(zpos) {delete []zpos; zpos=0;}
	if(cp_x) {delete []cp_x; cp_x=0;}
	if(cp_y) {delete []cp_y; cp_y=0;}
	if(cp_z) {delete []cp_z; cp_z=0;}
	if(cp_alpha) {delete []cp_alpha; cp_alpha=0;}
	if(cpara) //delete the cubic spline parameter data structure
	{
		for(int i=0;i<2;i++) {if (cpara[i]) {delete cpara[i]; cpara[i]=0;}}
		delete []cpara; cpara=0;
	}

	return true;
}
