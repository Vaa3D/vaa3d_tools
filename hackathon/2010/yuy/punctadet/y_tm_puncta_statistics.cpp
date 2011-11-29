//y_tm_puncta_statistics.cpp
// by Yang Yu
// 090726
// 20100614

//load neuron tracing swc file then define and computing statistics of GFP puncta
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctime>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iostream>

#include <QtGui>

#include "../../../v3d_main/basic_c_fun/basic_surf_objs.h"
#include "../../../v3d_main/basic_c_fun/stackutil.h"
#include "../../../v3d_main/basic_c_fun/volimg_proc.h"
#include "../../../v3d_main/basic_c_fun/img_definition.h"
#include "../../../v3d_main/basic_c_fun/basic_landmark.h"
#include "y_ytree.h"

#include <ctime>

#define INF 1e20

//region growing
#define NO_OBJECT 0 

//Defining Puncta structure
struct PUNCTA
{
	float x, y, z;
	float rx, ry, rz;
	float volsize;
	float intensity;
	float sdev, pixmax, mass;
	long num;
};

// for region growing
static int find( int set[], int x )
{
    int r = x;
    while ( set[r] != r )
        r = set[r];
    return r;
}
//statistics of count of labeling
class STCL
	{
	public:
		STCL(){}
		~STCL(){}
		
	public:
		int count;
		int label;
	};

//Gaussian kernel and correlation compuating copy from ../cellseg/template_matching_seg.cpp
Vol3DSimple<float> * genGaussianKernal3D(long szx, long szy, long szz, float sigmax, float sigmay, float sigmaz)
{
	if (szx<=0 || szy<=0 || szz<=0) {printf("Invalid sz parameter in genGaussianKernal3D().\n"); return 0;}
	
	Vol3DSimple<float> * g = 0;
	try 
	{
		g = new Vol3DSimple<float> (szx, szy, szz);
	}
	catch (...) 
	{
		printf("Fail to create a kernel object.\n");
		return 0;
	}
	
	float *** d3 = g->getData3dHandle();
	float sx2 = 2.0*sigmax*sigmax, sy2=2.0*sigmay*sigmay, sz2=2.0*sigmaz*sigmaz;
	long cx=(szx-1)>>1, cy=(szy-1)>>1, cz=(szz-1)>>1;
	long i,j,k;
	for (k=0;k<=cz;k++)
		for (j=0;j<=cy;j++)
			for (i=0;i<=cx;i++)
			{
				d3[szz-1-k][szy-1-j][szx-1-i] = d3[szz-1-k][szy-1-j][i] = 
				d3[szz-1-k][j][szx-1-i] = d3[szz-1-k][j][i] = 
				d3[k][szy-1-j][szx-1-i] = d3[k][szy-1-j][i] = 
				d3[k][j][szx-1-i] = d3[k][j][i] = 
				exp(-float(i-cx)*(i-cx)/sx2-float(j-cy)*(j-cy)/sy2-float(k-cz)*(k-cz)/sz2);
			}
	return g;
}

template <class T1, class T2> float compute_corrcoef_two_vectors(T1 *v1, T2 *v2, long len)
{
	if (!v1 || !v2 || len<=1) return 0;
	
	//first compute mean
	float m1=0,m2=0;
	long i;
	for (i=0;i<len;i++)
	{
		m1+=v1[i];
		m2+=v2[i];
	}
	m1/=len;
	m2/=len;
	
	//now compute corrcoef
	float tmp_s=0, tmp_s1=0, tmp_s2=0, tmp1, tmp2;
	for (i=0;i<len;i++)
	{
		tmp1 = v1[i]-m1;
		tmp2 = v2[i]-m2;
		tmp_s += tmp1*tmp2;
		tmp_s1 += tmp1*tmp1;
		tmp_s2 += tmp2*tmp2;
	}
	
	//the final score
	float s;
	s = (tmp_s / sqrt(tmp_s1) / sqrt(tmp_s2) + 1 )/2;
	return s;
}

//Compute a 3D line profile judge valley
template <class T> bool compute_3dlineprofile(Vol3DSimple <unsigned char> *img3d, T xb, T yb, T zb, T xe, T ye, T ze, const float threshold)
{
	
	T cnt;
	unsigned char *** img3d_p3d = img3d->getData3dHandle();
	
	long flag;
	if(abs(xe-xb)>=abs(ye-yb))
	{
		flag = 1;
		cnt = abs(xe-xb);
	}
	else
	{
		flag = 2;
		cnt = abs(ye-yb);
	}
	
	if(cnt<abs(ze-zb))
	{
		flag = 3;
		cnt = abs(ze-zb);
	}
	
	unsigned char *linePro = new unsigned char [cnt];
	
	T x=0, y=0, z=0;
	T loopstart = 0, loopend = 0;
	switch (flag) {
		case 1:
			loopstart = (xb<xe)? xb : xe;
			loopend = xb+xe-loopstart;
			x = xb;
			for(T i=loopstart; i<loopend; i++)
			{	
				y = yb + (ye-yb)/(xe-xb)*(x-xb);
				z = zb + (ze-zb)/(xe-xb)*(x-xb);
				
				linePro[i - loopstart] = img3d_p3d[z][y][x];
				
				if(loopstart<xe)
					x++;
				else
					x--;
			}
			break;
		case 2:
			loopstart = (yb<ye)? yb : ye;
			loopend = yb+ye-loopstart;
			y = yb;
			for(T i=loopstart; i<loopend; i++)
			{	
				x = xb + (xe-xb)/(ye-yb)*(y-yb);
				z = zb + (ze-zb)/(ye-yb)*(y-yb);
				
				linePro[i - loopstart] = img3d_p3d[z][y][x];
				
				if(loopstart<ye)
					y++;
				else
					y--;
			}
			break;
		case 3:
			loopstart = (zb<ze)? zb : ze;
			loopend = zb+ze-loopstart;
			z = zb;
			for(T i=loopstart; i<loopend; i++)
			{	
				y = yb + (ye-yb)/(ze-zb)*(z-zb);
				x = xb + (xe-xb)/(ze-zb)*(z-zb);
				
				linePro[i - loopstart] = img3d_p3d[z][y][x];
				
				if(loopstart<ze)
					z++;
				else
					z--;
			}
			break;
		default:
			break;
	}
	
	//judge whether line profile having a valley
	bool flagbool;
	for(T i=0; i<cnt; i++)
	{
		if((float)linePro[i]<threshold)
		{
			flagbool = false;
			break;
		}
		else
		{
			flagbool = true;
		}
		
	}
	
	return flagbool;
}

//open a series of inputs
QStringList importSeriesFileList_addnumbersort(const QString & individualFileName)
{
	QStringList myList;
	myList.clear();
	
	//Get the image files namelist in the directory
	
	QFileInfo fileInfo(individualFileName);
	QString curFilePath = fileInfo.path();
	QString curSuffix = fileInfo.suffix();
	
	QDir dir(curFilePath);
	if (!dir.exists())
	{
		qWarning("Cannot find the directory");
		return myList;
	}
	
	QStringList imgfilters;
	imgfilters.append("*." + curSuffix);
	foreach (QString file, dir.entryList(imgfilters, QDir::Files, QDir::Name))
	{
		myList += QFileInfo(dir, file).absoluteFilePath();
	}

	foreach (QString qs, myList)  qDebug() << qs;

	return myList;
}


//print HELP 
void printHelp();
void printHelp()
{
	//pirnt help messages
	printf("\nUsage: y_tm_puncta_statistics -s <image_subject> -t <SWCFile_subject> -j <single_or_multiple> -f <ration_resolution_z2xy> -u <volsize_xy> -v <volsize_z> -a <alpha> -b <beta> -g <global_threshold> -L <higher_local_threshold> -l <lower_local_threshold> -x <dim_x> -y <dim_y> -z <dim_z> -m <output_mask> -r <output_roc> -o <output_detectedpuncta> -w <overlap_images> -p <apo_file>\n");
	printf("Selecting ROI from neuron tracing swc file. \n");
	printf("\t [-h]                 HELP message.\n");
	printf("-s input image file (.tif .raw) \n");
	printf("-t input neuron traced file (.swc) \n");
	printf("-j flag (0: only read specified .swc file; 1: read all .swc files under the same folder) \n");
	printf("-a -b compute current radius (alpha*radius_of_node_in_neuron + beta) \n");
	printf("-g global threshold for telling foreground from background \n");
	printf("-L higher local threshold for detecting puncta \n");
	printf("-l lower local threshold for detecting puncta\n");
	printf("-x -y -z dimensions along x, y, and z when no image input \n");
	printf("-m output mask file when specified \n");
	printf("-f ratio of resolution of z compared to x, y \n");
	printf("-u volsize of x, y \n");
	printf("-v volsize of z \n");
	return;
}


#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

//--------------------------------------------------------------------------------------------------------------------------------
//main
int main(int argc, char *argv[])
{
	int start_t = clock();
	
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}

	// Read arguments	
	unsigned char * img_subject = 0;  
	long * sz_subject = 0; 
	int datatype_subject = 0;
	
	long sx=0, sy=0, sz=0, sc=0;
	
	char *dfile_subject = NULL; //ori image
	char *dfile_masked = NULL; //mask image
	char *dfile_warped = NULL; //output image
	char *dfile_traced = NULL; //neuron traced swc file
	char *dfile_overlaped = NULL; //overlap image
	char *dfile_apo = NULL; //apo file
	char *dfile_roc = NULL; //roc image (region of conflict)
	
	int flag_sm = 1; // single swc file: false, multiple swc files: true
	
	float scalar = 5.2382/1.0378; //for Jinny data
	float volsz_xy = 0, volsz_z=0;
	float alpha = 1, beta = 25; //parameters for roi selection // a=3 b=8
	float gthresh = 4, Lthresh =30, lthresh = 5; //global and local thresholding        ///////////////////////////////////////
	int c;
	static char optstring[] = "hs:t:a:b:g:L:l:m:o:u:v:w:p:x:y:z:r:j:f:";
	opterr = 0;
	while ((c = getopt (argc, argv, optstring)) != -1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;
				
			case 's':
				/* fprintf(stderr,"[%s]-> ",optarg); */
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				dfile_subject = optarg;
				break;
				
			case 't':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				dfile_traced = optarg;
				break;

			case 'j':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				flag_sm = atoi(optarg);
				break;
				
			case 'f':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				scalar = atof(optarg);
				break;
				
			case 'u':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -a.\n");
					return 1;
				}
				volsz_xy = atof(optarg);
				break;
				
			case 'v':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -a.\n");
					return 1;
				}
				volsz_z = atof(optarg);
				break;
				
			case 'a':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -a.\n");
					return 1;
				}
				alpha = atof(optarg);
				break;
				
			case 'b':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -b.\n");
					return 1;
				}
				beta = atof(optarg);
				break;
				
			case 'g':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -g.\n");
					return 1;
				}
				gthresh = atof(optarg);
				break;
				
			case 'L':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -L.\n");
					return 1;
				}
				Lthresh = atof(optarg);
				break;	
				
			case 'l':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -l.\n");
					return 1;
				}
				lthresh = atof(optarg);
				break;
				
			case 'x':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -x.\n");
					return 1;
				}
				sx = atol(optarg);
				break;	
				
			case 'y':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -y.\n");
					return 1;
				}
				sy = atol(optarg);
				break;	
				
			case 'z':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -z.\n");
					return 1;
				}
				sz = atol(optarg);
				break;	
				
			case 'm':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				dfile_masked = optarg;
				break;
				
			case 'r':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -r.\n");
					return 1;
				}
				dfile_roc = optarg;
				break;
				
			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				dfile_warped = optarg;
				break;
				
			case 'w':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -v.\n");
					return 1;
				}
				dfile_overlaped = optarg;
				break;
				
			case 'p':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -p.\n");
					return 1;
				}
				dfile_apo = optarg;
				break;
				
			case '?':
				fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
				return 0;
				
				/* default:        abort (); */
        }
    }
	
	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);

	
	//loading original files
	if(dfile_subject)
	{
		if (loadImage(dfile_subject, img_subject, sz_subject, datatype_subject)!=true)
		{
			fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n", dfile_subject);
			return false;
		}
		
		sx= sz_subject[0], sy = sz_subject[1], sz = sz_subject[2]; 
	}

	//initialization 
	sc = 1;
	long pagesz = sx*sy*sz;
	
	//ratio
	if(volsz_xy && volsz_z)
		scalar = volsz_z / volsz_xy;
	
	if(!scalar)
		return false;

	//loading swc files of neuron tracing results
	QString neurons_filename(dfile_traced);
	QStringList neuronsList;
	
	if(flag_sm)
		neuronsList = importSeriesFileList_addnumbersort(neurons_filename);
	else
		neuronsList.append(neurons_filename);

	long num_supported = 1000000; // each neuron .swc
	
	//float scalar = 5.2382/1.0378; //for Jinny slice15
	float scalar2 = scalar*scalar;
	
	std::vector <PUNCTA> detectedList;
	PUNCTA pos;

	//-----------------------------------computing mask----------------------------------------------
	//declaration
	long nc=neuronsList.size();
	
	int *pImMask = NULL;
	float *flag_rbuffer = NULL;
	bool *flag_roc = NULL;
	bool *flag_sfb = NULL;
	bool *flag_computed = NULL;
	
	try
	{
		pImMask = new int [pagesz];
		flag_rbuffer = new float [pagesz];
		flag_roc = new bool [pagesz];
		flag_sfb = new bool [pagesz*nc];
		flag_computed = new bool [pagesz];
		
		for(long i=0; i<pagesz; i++)
		{
			pImMask[i] = 0;
			flag_rbuffer[i] = 0;
			flag_roc[i] = false;
			flag_computed[i] = false;
			
			for(long c=0; c<nc; c++)
			{
				flag_sfb[i+c*pagesz] = false;
			}
		}
		
		
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return false;
	}
	
	//long maxr=0;
	long countSwc = 0;
	
	float avg_r = 15; //soma //changed later judge from type of .swc file
	
//	// testing soma and first branch
//	foreach (QString traced_neuron, neuronsList)
//	{
//		NeuronTree neurons;                     
//		neurons=readSWC_file(traced_neuron);  
//		NeuronSWC *p_tmp=0;
//		countSwc++;
//		
//		//compute the average radius of neurons
//		
//		//
//		int N_nodes = neurons.listNeuron.size();
//		for (int ii=0; ii<N_nodes; ii++)
//		{
//			p_tmp = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
//			
//			float xs = p_tmp->x;
//			float ys = p_tmp->y;
//			float zs = p_tmp->z;
//			float rs = p_tmp->r;
//			
//			//printf("num %ld \n", p_tmp->n);
//			
//			avg_r += rs;
//			
//		}
//		avg_r /= N_nodes;
//		
//	}
//  //comment here avg_r through mean will be too small (about 4)
	
	std::vector<NeuronTree> Neuron_List;
	
	foreach (QString traced_neuron, neuronsList)
	{
		NeuronTree neurons;                     
		neurons=readSWC_file(traced_neuron);  
		
		Neuron_List.push_back(neurons);
	}
	
	//find soma first
	foreach(NeuronTree neurons, Neuron_List)
	{
		//NeuronTree neurons;                     
		//neurons=readSWC_file(traced_neuron);  
		NeuronSWC *p_tmp=0;
		
		long offset_nc = countSwc*pagesz;
		
		countSwc++;
		
		//
		int N_nodes = neurons.listNeuron.size();
		for (int ii=0; ii<N_nodes; ii++)
		{
			p_tmp = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
			
			float xs = p_tmp->x;
			float ys = p_tmp->y;
			float zs = p_tmp->z;
			float rs = p_tmp->r;
			
			//if(rs>avg_r)
			if(p_tmp->type==1)
			{
				//find previous node
				NeuronSWC *pp=0;
				for(int j=0; j<neurons.listNeuron.size(); j++)
				{
					pp = (NeuronSWC *)(&(neurons.listNeuron.at(j)));
					
					if(pp->n == p_tmp->pn)
						break;
				}
				//no previous node
				if(pp->n != p_tmp->pn)
					continue; 
				
				float xe = pp->x;
				float ye = pp->y;
				float ze = pp->z;
				
				rs += 2; //8
				
				//finding the envelope
				float x_down = (xs>xe)? xe: xs;
				float x_top = (xs>xe)? xs: xe;
				float y_down = (ys>ye)? ye: ys;
				float y_top = (ys>ye)? ys: ye;
				float z_down = (zs>ze)? ze: zs;
				float z_top = (zs>ze)? zs: ze;
				
				if(x_down == xs)
				{
					if(x_down-rs > 0)
						x_down -= rs;
					else
						x_down = 0;
				}
				else
				{
					if(x_down-rs > 0)
						x_down -= rs; //re
					else
						x_down = 0;
				}
				
				if(y_down == ys)
				{
					if(y_down-rs > 0)
						y_down -= rs;
					else
						y_down = 0;
				}
				else
				{
					if(y_down-rs > 0)
						y_down -= rs; //re
					else
						y_down = 0;
				}
				
				if(z_down == zs)
				{
					if(z_down-rs > 0)
						z_down -= rs;
					else
						z_down = 0;
				}
				else
				{
					if(z_down-rs > 0)
						z_down -= rs; //re
					else
						z_down = 0;
				}
				
				if(x_top == xs)
				{
					if(x_top+rs < sx)
						x_top += rs;
					else
						x_top = sx;
				}
				else
				{
					if(x_top+rs < sx)
						x_top += rs; //re
					else
						x_top = sx;
				}
				
				if(y_top == ys)
				{
					if(y_top+rs < sy)
						y_top += rs;
					else
						y_top = sy;
				}
				else
				{
					if(y_top+rs < sy)
						y_top += rs; //re
					else
						y_top = sy;
				}
				
				if(z_top == zs)
				{
					if(z_top+rs < sz)
						z_top += rs;
					else
						z_top = sz;
				}
				else
				{
					if(z_top+rs < sz)
						z_top += rs; //re
					else
						z_top = sz;
				}
				//printf("%lf %lf %lf %lf %lf %lf\n", x_down, y_down, z_down, x_top, y_top, z_top);
				
				//cylinder flag mask
				for(long k=long(z_down); k<long(z_top); k++)
				{
					long offset_k = k*sx*sy;
					for(long j=long(y_down); j<long(y_top); j++)
					{
						long offset_j = offset_k + j*sx;
						for(long i=long(x_down); i<long(x_top); i++)
						{
							long indLoop = offset_j + i;
							
							//Weisstein, Eric W. "Point-Line Distance--3-Dimensional." From MathWorld--A Wolfram Web Resource. 
							//http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html 
							//norm(cross(x0-x1,x1-x2))/norm(x1-x2)
							float norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k)*scalar2;
							float norms21 = (xe-xs)*(xe-xs) + (ye-ys)*(ye-ys) + (ze-zs)*(ze-zs)*scalar2; 
							float dots1021 = (xs-i)*(xe-xs) + (ys-j)*(ye-ys) + (zs-k)*(ze-zs)*scalar2; 
							
							float dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
							
							float t = -dots1021/norms21;
							
							if(t<0)
								dist = sqrt(norms10);
							else if(t>1)
								dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k)*scalar2);
							
							if(dist<=rs)
							{	
								if(flag_rbuffer[indLoop])
								{
									if(dist<(flag_rbuffer[indLoop] - long(flag_rbuffer[indLoop]/num_supported)*num_supported))
									{
										flag_rbuffer[indLoop] = countSwc*num_supported + dist;
										flag_sfb[indLoop + offset_nc] = true;
									}	
								}
								else
								{
									flag_rbuffer[indLoop] = countSwc*num_supported + dist;
									flag_sfb[indLoop + offset_nc] = true;
								}
							}
							
						}
					}
					
				}
			}
			else
				continue;
			
		}
		
	}
	
	//reinit
	for(long i=0; i<pagesz; i++)
	{
		flag_rbuffer[i] = 0;
	}
	
	// find end nodes in .swc
	foreach(NeuronTree neurons, Neuron_List)
	{
		NeuronSWC *p_curr=0;
		
		int N_nodes = neurons.listNeuron.size();
		int j=0;
		for (int ii=0; ii<N_nodes; ii++)
		{
			p_curr = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
			
			//find child node
			NeuronSWC *p_child=0;
			for( j=0; j<N_nodes; j++)
			{
				p_child = (NeuronSWC *)(&(neurons.listNeuron.at(j)));
				
				if(p_child->pn == p_curr->n)
					break;
			}
			
			if(j>=N_nodes)
				p_curr->type=99; // define end node in type "99"
			
		}
	}
		
	
	// 
	//compute mask except soma 
	//considering end nodes' cases
	countSwc = 0;
	foreach(NeuronTree neurons, Neuron_List)
	{
		NeuronSWC *p_tmp=0;
		
		long offset_nc = countSwc*pagesz;
		
		countSwc++;
		
		//
		int N_nodes = neurons.listNeuron.size();
		for (int ii=0; ii<N_nodes; ii++)
		{
			p_tmp = (NeuronSWC *)(&(neurons.listNeuron.at(ii)));
			
			float xs = p_tmp->x;
			float ys = p_tmp->y;
			float zs = p_tmp->z;
			float rs = p_tmp->r;
			
			//if(rs>avg_r)
			if(p_tmp->type==1) // define 1 as soma region
				continue;
			else
				rs = alpha*rs+beta;
			
			//find previous node
			NeuronSWC *pp=0;
			for(int j=0; j<N_nodes; j++)
			{
				pp = (NeuronSWC *)(&(neurons.listNeuron.at(j)));
				
				if(pp->n == p_tmp->pn)
					break;
			}
			//no previous node
			if(pp->n != p_tmp->pn)
				continue; 
			
			float xe = pp->x;
			float ye = pp->y;
			float ze = pp->z;
			
			//finding the envelope
			float x_down = (xs>xe)? xe: xs;
			float x_top = (xs>xe)? xs: xe;
			float y_down = (ys>ye)? ye: ys;
			float y_top = (ys>ye)? ys: ye;
			float z_down = (zs>ze)? ze: zs;
			float z_top = (zs>ze)? zs: ze;
			
			if(x_down == xs)
			{
				if(x_down-rs > 0)
					x_down -= rs;
				else
					x_down = 0;
			}
			else
			{
				if(x_down-rs > 0)
					x_down -= rs; //re
				else
					x_down = 0;
			}
			
			if(y_down == ys)
			{
				if(y_down-rs > 0)
					y_down -= rs;
				else
					y_down = 0;
			}
			else
			{
				if(y_down-rs > 0)
					y_down -= rs; //re
				else
					y_down = 0;
			}
			
			if(z_down == zs)
			{
				if(z_down-rs > 0)
					z_down -= rs;
				else
					z_down = 0;
			}
			else
			{
				if(z_down-rs > 0)
					z_down -= rs; //re
				else
					z_down = 0;
			}
			
			if(x_top == xs)
			{
				if(x_top+rs < sx)
					x_top += rs;
				else
					x_top = sx;
			}
			else
			{
				if(x_top+rs < sx)
					x_top += rs; //re
				else
					x_top = sx;
			}
			
			if(y_top == ys)
			{
				if(y_top+rs < sy)
					y_top += rs;
				else
					y_top = sy;
			}
			else
			{
				if(y_top+rs < sy)
					y_top += rs; //re
				else
					y_top = sy;
			}
			
			if(z_top == zs)
			{
				if(z_top+rs < sz)
					z_top += rs;
				else
					z_top = sz;
			}
			else
			{
				if(z_top+rs < sz)
					z_top += rs; //re
				else
					z_top = sz;
			}
			//printf("%lf %lf %lf %lf %lf %lf\n", x_down, y_down, z_down, x_top, y_top, z_top);
				
			//cylinder
			//flag mask
			for(long k=long(z_down); k<long(z_top); k++)
			{
				long offset_k = k*sx*sy;
				for(long j=long(y_down); j<long(y_top); j++)
				{
					long offset_j = offset_k + j*sx;
					for(long i=long(x_down); i<long(x_top); i++)
					{
						long indLoop = offset_j + i;
						
						//Weisstein, Eric W. "Point-Line Distance--3-Dimensional." From MathWorld--A Wolfram Web Resource. 
						//http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html 
						//norm(cross(x0-x1,x1-x2))/norm(x1-x2)
						float norms10 = (xs-i)*(xs-i) + (ys-j)*(ys-j) + (zs-k)*(zs-k)*scalar2;
						float norms21 = (xe-xs)*(xe-xs) + (ye-ys)*(ye-ys) + (ze-zs)*(ze-zs)*scalar2; 
						float dots1021 = (xs-i)*(xe-xs) + (ys-j)*(ye-ys) + (zs-k)*(ze-zs)*scalar2; 
						
						float dist = sqrt( norms10 - (dots1021*dots1021)/(norms21) );
						
						float t = -dots1021/norms21;
						
						if(t<0)
						{
							// deal with end nodes
							if(p_tmp->type==99)
								dist = INF;
							else
								dist = sqrt(norms10);
						}
						else if(t>1)
							dist = sqrt((xe-i)*(xe-i) + (ye-j)*(ye-j) + (ze-k)*(ze-k)*scalar2);
						

						if(flag_sfb[indLoop+offset_nc])
							continue;
						
						if(dist<=rs)
						{	
							if(flag_rbuffer[indLoop])
							{
								if(dist<(flag_rbuffer[indLoop] - long(flag_rbuffer[indLoop]/num_supported)*num_supported))
								{
									flag_rbuffer[indLoop] = countSwc*num_supported + dist;
									pImMask[indLoop] = countSwc*num_supported + p_tmp->n;
								}	
							}
							else
							{
								flag_rbuffer[indLoop] = countSwc*num_supported + dist;
								pImMask[indLoop] = countSwc*num_supported + p_tmp->n;
							}
						}
						
						if(flag_computed[indLoop]==true)
							flag_roc[indLoop]= true;
			
					}
				}
				
			}
			
		}
		for (long k=0;k<sz;k++)
		{
			long offsetk = k*sx*sy;
			for (long j=0;j<sy;j++)
			{
				long offsetj = offsetk + j*sx;
				for (long i=0;i<sx;i++)
				{
					long indLoop = offsetj + i;
					if(long(pImMask[indLoop]/num_supported)==countSwc) 
					{
						flag_computed[indLoop] = true;				
					}
				}
			}
		}
		
	}
	
	//de-alloc
	if(flag_rbuffer) {delete []flag_rbuffer; flag_rbuffer=0;}
	if(flag_computed) {delete []flag_computed; flag_computed=0;}
	
	
	//save output
	if(dfile_masked)
	{
		printf("save mask file...\n");
		
		//Save as 8-bit RGB channels image stacks for mask image
		unsigned char *pMask = new unsigned char [pagesz];

		for(long i=0; i<pagesz; i++)
		{
			if(pImMask[i])
				pMask[i] = 255;
			else
				pMask[i] = 0;
		}


		long save_sz[4];
		save_sz[0]=sx; save_sz[1]=sy; save_sz[2]=sz; save_sz[3]=sc;
		if (saveImage(dfile_masked, (unsigned char *)pMask, save_sz, 1)!=true)
		{
			fprintf(stderr, "Error happens in file writing. Exit. \n");
			return false;
		}
		
		//de-alloc
		if(pMask) {delete []pMask; pMask=0;}
	}
	
	if(dfile_roc)
	{
		printf("save roc file...\n");
		
		//Save as 8-bit RGB channels image stacks for mask image
		unsigned char *pROC = new unsigned char [pagesz];

		for(long i=0; i<pagesz; i++)
		{
			if(flag_roc[i])
				pROC[i] = 255;
			else
				pROC[i] = 0;
		}
		
		long save_sz[4];
		save_sz[0]=sx; save_sz[1]=sy; save_sz[2]=sz; save_sz[3]=sc;
		if (saveImage(dfile_roc, (unsigned char *)pROC, save_sz, 1)!=true)
		{
			fprintf(stderr, "Error happens in file writing. Exit. \n");
			return false;
		}
		
		//de-alloc
		if(pROC) {delete []pROC; pROC=0;}
		
	}
	
	//de-alloc
	if(flag_roc) {delete []flag_roc; flag_roc=0;}
	
	// detecting inert ("big") puncta in the ROI of (flag_sfb || pImgMask)
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	// ******
	// step 1 k-means thresholding
	// ******
	
	long K=2;
	
	unsigned char *p=img_subject;
	
	long *h=0, *hc=0;
	
	try
	{
		h = new long [256];
		hc = new long [256];
		
		memset(h, 0, 256*sizeof(long));
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return false;
	}
	
	// histogram
	for(long i=0; i<pagesz; i++)
	{
		h[*(p++)] ++;
	}
	
	// init center
	float mub=10, muf=80;
	
	//
	while (true) 
	{
		float oldmub=mub, oldmuf=muf;
		
		for(long i=0; i<256; i++)
		{
			if(h[i]==0)
				continue;
			
			float cb = fabs(float(i)-mub);
			float cf = fabs(float(i)-muf);
			
			hc[i] = (cb<=cf)?1:2; // class 1 and class 2
		}
		
		// update centers
		float sum_b=0, sum_bw=0, sum_f=0, sum_fw=0;
		
		for(long i=0; i<256; i++)
		{
			if(h[i]==0)
				continue;
			
			if(hc[i]==1)
			{
				sum_bw += (i+1)*h[i]; 
				sum_b += h[i];
			}
			else if(hc[i]==2)
			{
				sum_fw += (i+1)*h[i]; 
				sum_f += h[i];
			}
		}
		
		mub = (unsigned char) ( sum_bw/sum_b );
		muf = (unsigned char) ( sum_fw/sum_f );
		
		if(fabs(mub - oldmub)<1 && fabs(muf - oldmuf)<1)
			break;
		
	}
	
	// binary image: bw
	bool *bw = NULL;
	try
	{
		bw = new bool [pagesz];
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return false;
	}
	
	for(long i=0; i<pagesz; i++)
	{
		float tmp = img_subject[i];
		
		float dist_b = fabs(tmp-mub);
		float dist_f = fabs(tmp-muf);
		
		bw[i] = (dist_b>dist_f)?true:false; //
	}
	
	//de-alloc
	if(h) {delete []h; h=0;}
	if(hc) {delete []hc; hc=0;}
	
	//ROI
	for(long i=0; i<pagesz; i++)
	{
		bool b_flag = false;
		
		for(long c=0; c<nc; c++)
		{
			if(flag_sfb[i+c*pagesz])
				b_flag = true;
		}
		
		if(!(b_flag || pImMask[i]))
		{
			bw[i] = false;
		}
	}
	
	
	// ******
	//step 2. region growing compute volume size and histogram
	// ******
	
	unsigned int *L = NULL;
	int *lset = NULL;   // label table/tree
	
	try
	{
		L = new unsigned int [pagesz];
		lset = new int [pagesz]; 
		
		for(long i=0; i<pagesz; i++)
		{
			L[i] = 0;
		}
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return false;
	}

    int ntable;                     // number of elements in the component table/tree
	
	long offset_y, offset_z;
	
	offset_y=sx;
	offset_z=sx*sy;
	
	long neighborhood_13[13] = {-1, -offset_y, -offset_z,
								-offset_y-1, -offset_y-offset_z, 
								offset_y-1, offset_y-offset_z,
								offset_z-1, -offset_z-1,
								-1-offset_y-offset_z, -1+offset_y-offset_z,
								1-offset_y-offset_z, 1+offset_y-offset_z}; 
	
	
	ntable = 0;
    lset[0] = 0;
	
	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				
				if(i==0 || i==sx-1 || j==0 || j==sy-1 || k==0 || k==sz-1)
					continue;
				
				// find connected components
				if(bw[idx]) // if there is an object 
				{
					int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13;
					
					n1 = find(lset, (int)L[idx + neighborhood_13[0] ]);
					n2 = find(lset, (int)L[idx + neighborhood_13[1] ]);
					n3 = find(lset, (int)L[idx + neighborhood_13[2] ]);
					n4 = find(lset, (int)L[idx + neighborhood_13[3] ]);
					n5 = find(lset, (int)L[idx + neighborhood_13[4] ]);
					n6 = find(lset, (int)L[idx + neighborhood_13[5] ]);
					n7 = find(lset, (int)L[idx + neighborhood_13[6] ]);
					n8 = find(lset, (int)L[idx + neighborhood_13[7] ]);
					n9 = find(lset, (int)L[idx + neighborhood_13[8] ]);
					n10 = find(lset, (int)L[idx + neighborhood_13[9] ]);
					n11 = find(lset, (int)L[idx + neighborhood_13[10] ]);
					n12 = find(lset, (int)L[idx + neighborhood_13[11] ]);
					n13 = find(lset, (int)L[idx + neighborhood_13[12] ]);
					
					
					
					if(n1 || n2 || n3 || n4 || n5 || n6 || n7 || n8 || n9 || n10 || n11 || n12 || n13)
					{
						int tlabel;
						
						if(n1) tlabel = n1;
						else if(n2) tlabel = n2;
						else if(n3) tlabel = n3;
						else if(n4) tlabel = n4;
						else if(n5) tlabel = n5;
						else if(n6) tlabel = n6;
						else if(n7) tlabel = n7;
						else if(n8) tlabel = n8;
						else if(n9) tlabel = n9;
						else if(n10) tlabel = n10;
						else if(n11) tlabel = n11;
						else if(n12) tlabel = n12;
						else if(n13) tlabel = n13;
						
						L[idx] = tlabel;
						
						if(n1 && n1 != tlabel) lset[n1] = tlabel;
						if(n2 && n2 != tlabel) lset[n2] = tlabel;
						if(n3 && n3 != tlabel) lset[n3] = tlabel;
						if(n4 && n4 != tlabel) lset[n4] = tlabel;
						if(n5 && n5 != tlabel) lset[n5] = tlabel;
						if(n6 && n6 != tlabel) lset[n6] = tlabel;
						if(n7 && n7 != tlabel) lset[n7] = tlabel;
						if(n8 && n8 != tlabel) lset[n8] = tlabel;
						if(n9 && n9 != tlabel) lset[n9] = tlabel;
						if(n10 && n10 != tlabel) lset[n10] = tlabel;
						if(n11 && n11 != tlabel) lset[n11] = tlabel;
						if(n12 && n12 != tlabel) lset[n12] = tlabel;
						if(n13 && n13 != tlabel) lset[n13] = tlabel;
						
					}
					else
					{
						ntable++;
						L[idx] = lset[ntable] = ntable;
					}
					
				}
				else
				{
					L[idx] = NO_OBJECT;
					
				}
				
			}
		}
	}
	
	// consolidate component table
    for( int i = 0; i <= ntable; i++ )
        lset[i] = find( lset, i );
	
    // run image through the look-up table
   	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
    
    // count up the objects in the image
    for( int i = 0; i <= ntable; i++ )
        lset[i] = 0;
	
   	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				lset[ (int)L[idx] ]++;
			}
		}
	}
	
    // number the objects from 1 through n objects
    int nobj = 0;
    lset[0] = 0;
    for( int i = 1; i <= ntable; i++ )
        if ( lset[i] > 0 )
            lset[i] = ++nobj;
	
	// run through the look-up table again
	for(long idx=0; idx<pagesz; idx++) 
	{
		L[idx] = lset[ (int)L[idx] ];
	}

	// histogram of L
	int *a = new int [nobj+1];
	
	for(long i=0;  i<=nobj; i++)
	{
		a[i] = 0;
	}
	
	for(long i=0; i<pagesz; i++)
	{
		a[ L[i] ] ++;
	}
	
	//histogram plot
	FILE *pFileSta=0;
	
	string str;
	
	str = dfile_subject;
	
	str.erase(str.end()-4, str.end());
	
	string str1 = str + "_volsz.m";
	const char* strfile = new char [sizeof(str1)+1];
	strfile = str1.c_str();
	
	pFileSta = fopen(strfile,"wt");
	
	fprintf(pFileSta, "label=[ ");
	
	for(int j=0; j<nobj; j++)
	{
		fprintf(pFileSta, "%d\n", a[j]);
	}
	
	fprintf(pFileSta, "]; ");
	
	fclose(pFileSta);
	
	//choose volume size threshold **************************	
	for(int i=1;  i<=nobj; i++) // 0 is background
	{
		if(a[i]<125) //*************************
			a[i]=0;		
	}
	
	for(long i=0; i<pagesz; i++) // 
	{
		if(a[ L[i] ] && L[i])
			bw[i] = true;
		else
			bw[i] = false;
	}
	
	//reinit
	// L lset
	for(long i=0; i<pagesz; i++)
	{
		L[i] = 0;
		lset[i] = 0;
	}
	
	//de-alloc
	if(a) {delete []a; a=0;}
	
	// ******
	//step 3 erosion - min filtering for big puncta detection
	//                 max filtering for masking big puncta region when detect small puncta
	// ******
	bool *bw_minfiltered = NULL;
	bool *bw_maxfiltered = NULL;
	try
	{
		bw_minfiltered = new bool [pagesz];
		bw_maxfiltered = new bool [pagesz];
	}
	catch (...) 
	{
		printf("Fail to allocate memory.\n");
		return false;
	}
	
	unsigned int Wx=1, Wy=1, Wz=0; // 2, 2, 0
	float maxfl = 0, minfl = INF;
	
	//min Max filtering
	for(long iz = 0; iz < sz; iz++)
	{
		long offsetk = iz*sx*sy;
		for(long iy = 0; iy < sy; iy++)
		{
			long offsetj = offsetk + iy*sx;
			for(long ix = 0; ix < sx; ix++)
			{
				maxfl = 0; minfl = INF;
				
				long xb = ix-Wx; if(xb<0) xb = 0;
				long xe = ix+Wx; if(xe>=sx-1) xe = sx-1;
				long yb = iy-Wy; if(yb<0) yb = 0;
				long ye = iy+Wy; if(ye>=sy-1) ye = sy-1;
				long zb = iz-Wz; if(zb<0) zb = 0;
				long ze = iz+Wz; if(ze>=sz-1) ze = sz-1;
				
				for(long k=zb; k<=ze; k++)
				{
					long offsetkl = k*sx*sy;
					for(long j=yb; j<=ye; j++)
					{
						long offsetjl = offsetkl + j*sx;
						for(long i=xb; i<=xe; i++)
						{
							long dataval = bw[ offsetjl + i];
							
							if(maxfl<dataval) maxfl = dataval;
							if(minfl>dataval) minfl = dataval;
						}
					}
				}
				
				//set value
				long index_pim = offsetj + ix;
				
				bw_minfiltered[index_pim] = minfl;
				bw_maxfiltered[index_pim] = maxfl;
				
				
			}
		}
	}
	
	//de-alloc
	if(bw) {delete []bw; bw=0;}
	
//	if(dfile_masked)
//	{
//		printf("save mask file...\n");
//		
//		//Save as 8-bit RGB channels image stacks for mask image
//		unsigned char *pMask = new unsigned char [pagesz];
//		
//		for(long i=0; i<pagesz; i++)
//		{
//			if(bw_minfiltered[i])
//				pMask[i] = 255;
//			else
//				pMask[i] = 0;
//		}
//		
//		
//		long save_sz[4];
//		save_sz[0]=sx; save_sz[1]=sy; save_sz[2]=sz; save_sz[3]=sc;
//		if (saveImage(dfile_masked, (unsigned char *)pMask, save_sz, 1)!=true)
//		{
//			fprintf(stderr, "Error happens in file writing. Exit. \n");
//			return false;
//		}
//		
//		//de-alloc
//		if(pMask) {delete []pMask; pMask=0;}
//		
//		return true;
//	}
	
	
	// ******
	//step 4 detect inert (big) puncta using region growing
	// ******
	
	ntable = 0;
    lset[0] = 0;
	
	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				
				if(i==0 || i==sx-1 || j==0 || j==sy-1 || k==0 || k==sz-1)
					continue;
				
				// find connected components
				if(bw_minfiltered[idx]) // if there is an object 
				{
					int n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13;
					
					n1 = find(lset, (int)L[idx + neighborhood_13[0] ]);
					n2 = find(lset, (int)L[idx + neighborhood_13[1] ]);
					n3 = find(lset, (int)L[idx + neighborhood_13[2] ]);
					n4 = find(lset, (int)L[idx + neighborhood_13[3] ]);
					n5 = find(lset, (int)L[idx + neighborhood_13[4] ]);
					n6 = find(lset, (int)L[idx + neighborhood_13[5] ]);
					n7 = find(lset, (int)L[idx + neighborhood_13[6] ]);
					n8 = find(lset, (int)L[idx + neighborhood_13[7] ]);
					n9 = find(lset, (int)L[idx + neighborhood_13[8] ]);
					n10 = find(lset, (int)L[idx + neighborhood_13[9] ]);
					n11 = find(lset, (int)L[idx + neighborhood_13[10] ]);
					n12 = find(lset, (int)L[idx + neighborhood_13[11] ]);
					n13 = find(lset, (int)L[idx + neighborhood_13[12] ]);
					
					
					
					if(n1 || n2 || n3 || n4 || n5 || n6 || n7 || n8 || n9 || n10 || n11 || n12 || n13)
					{
						int tlabel;
						
						if(n1) tlabel = n1;
						else if(n2) tlabel = n2;
						else if(n3) tlabel = n3;
						else if(n4) tlabel = n4;
						else if(n5) tlabel = n5;
						else if(n6) tlabel = n6;
						else if(n7) tlabel = n7;
						else if(n8) tlabel = n8;
						else if(n9) tlabel = n9;
						else if(n10) tlabel = n10;
						else if(n11) tlabel = n11;
						else if(n12) tlabel = n12;
						else if(n13) tlabel = n13;
						
						L[idx] = tlabel;
						
						if(n1 && n1 != tlabel) lset[n1] = tlabel;
						if(n2 && n2 != tlabel) lset[n2] = tlabel;
						if(n3 && n3 != tlabel) lset[n3] = tlabel;
						if(n4 && n4 != tlabel) lset[n4] = tlabel;
						if(n5 && n5 != tlabel) lset[n5] = tlabel;
						if(n6 && n6 != tlabel) lset[n6] = tlabel;
						if(n7 && n7 != tlabel) lset[n7] = tlabel;
						if(n8 && n8 != tlabel) lset[n8] = tlabel;
						if(n9 && n9 != tlabel) lset[n9] = tlabel;
						if(n10 && n10 != tlabel) lset[n10] = tlabel;
						if(n11 && n11 != tlabel) lset[n11] = tlabel;
						if(n12 && n12 != tlabel) lset[n12] = tlabel;
						if(n13 && n13 != tlabel) lset[n13] = tlabel;
						
					}
					else
					{
						ntable++;
						L[idx] = lset[ntable] = ntable;
					}
					
				}
				else
				{
					L[idx] = NO_OBJECT;
					
				}
				
			}
		}
	}
	
	// consolidate component table
    for( int i = 0; i <= ntable; i++ )
        lset[i] = find( lset, i );
	
    // run image through the look-up table
   	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				L[idx] = lset[ (int)L[idx] ];
			}
		}
	}
    
    // count up the objects in the image
    for( int i = 0; i <= ntable; i++ )
        lset[i] = 0;
	
   	for(long k = 0; k < sz; k++) 
	{				
		long idxk = k*offset_z;
		for(long j = 0;  j < sy; j++) 
		{
			long idxj = idxk + j*offset_y;
			
			for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
			{
				lset[ (int)L[idx] ]++;
			}
		}
	}
	
    // number the objects from 1 through n objects
	nobj = 0;
    lset[0] = 0;
    for( int i = 1; i <= ntable; i++ )
        if ( lset[i] > 0 )
            lset[i] = ++nobj;
	
	// run through the look-up table again
	for(long idx=0; idx<pagesz; idx++) 
	{
		L[idx] = lset[ (int)L[idx] ];
	}
	
	int np = nobj;
	// histogram of L
	int *aa = new int [nobj+1];
	
	for(long i=0;  i<=nobj; i++)
	{
		aa[i] = 0;
	}
	
	for(long i=0; i<pagesz; i++)
	{
		aa[ L[i] ] ++;
	}
	

	std::vector<STCL> labelList;

	for(int i=1;  i<=np; i++) // 0 is background
	{
		
		STCL s;
		
		s.count = aa[i];
		s.label = i;
		
		//
		if(labelList.size()<1)
			labelList.push_back(s);
		else
		{
			for(unsigned int it=labelList.size(); it!=0; it--)
			{
				if(s.count<=labelList.at(it-1).count)
				{
					labelList.insert(labelList.begin() + it, 1, s);
					
					if(labelList.size()>np) // pick np points
						labelList.erase(labelList.end());
					
					break;
				}
				else
					continue;
				
			}
			
			//
			if(s.count>labelList.at(0).count && labelList.size()<np) // pick np points
				labelList.insert(labelList.begin(), s);
		}
		
	}
	
	//save output to apo file
	FILE *pBigApo=0;
	countSwc = 0;
	
	std::ostringstream os;
	os<<lthresh;
	
	foreach (QString traced_neuron, neuronsList)
	{
		string str;
		
		str = traced_neuron.toStdString();

		//str -= ".swc";
		str += "_big.apo";
		const char* strfile = new char [sizeof(str)+1];
		strfile = str.c_str();
		
		pBigApo = fopen(strfile,"wt");
		
		long offset_nc = countSwc*pagesz;
		
		countSwc++;
		
		for(int i_n = 0; i_n<np; i_n++)
		{
			float scx=0,scy=0,scz=0,si=0;
			
			int label=labelList.at(i_n).label;
			
			for(long k = 0; k < sz; k++) 
			{				
				long idxk = k*offset_z;
				for(long j = 0;  j < sy; j++) 
				{
					long idxj = idxk + j*offset_y;
					
					for(long i = 0, idx = idxj; i < sx;  i++, idx++) 
					{
						
						//
						if(L[idx]==label)
						{
							float cv = img_subject[ idx ];
							
							scz += k*cv;
							scy += j*cv;
							scx += i*cv;
							si += cv;
						}
						
						
					}
				}
			}
			
			//
			if (si>0)
			{
				long ncx = scx/si + 0.5 ;//+1; 
				long ncy = scy/si + 0.5 ;//+1; 
				long ncz = scz/si + 0.5 ;//+1;
				
				long idx = offset_nc + ncz*sx*sy + ncy*sx + ncx;
				if(!(flag_sfb[idx]  || (pImMask[idx-offset_nc]/num_supported)==countSwc))
					continue;
				
				CellAPO S;
				
				S.n = i_n+1;
				S.orderinfo = QString::fromStdString(os.str()); //""; //local threshold
				S.name = "";
				S.comment = "";
				S.z = ncz;
				S.x = ncx;
				S.y = ncy;
				S.pixmax = 1;
				S.intensity = 1;
				S.sdev = 1;
				S.volsize = 10;
				S.mass = 10;
				S.color.r = 0; S.color.g = 0; S.color.b = 0;
				
				fprintf(pBigApo, "%d,%s,%s,%s,%ld,%ld,%ld,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
						S.n, //i+1,
						qPrintable(S.orderinfo),
						qPrintable(S.name),
						qPrintable(S.comment),
						long(S.z),
						long(S.x),
						long(S.y),
						S.pixmax,
						S.intensity,
						S.sdev,
						S.volsize,
						S.mass
						
						//					S.color.r,
						//					S.color.g,
						//					S.color.b
						);
				
				
			}
			
		}

		fclose(pBigApo);
	}
	
	
	// detecting active ("small") puncta in the ROI of (!(flag_sfb || bw_maxfiltered) && pImMask)
	//-----------------------------------------------------------------------------------------------------------------------------------------------
	
	

	
	//-----------------------------------segment and label puncta------------------------------------
	//template matching
	unsigned char *pGImg = img_subject;
	
	//float thresh = 0, meanVal=0, stdVal=0;
	//mean_and_std(pGImg, pagesz, meanVal, stdVal);
	//thresh = meanVal + 2*stdVal;
	
	Vol3DSimple <unsigned char> *inimg3d = new Vol3DSimple <unsigned char> (sx, sy, sz); 
	unsigned char ***img3d = inimg3d->getData3dHandle();
	
	Vol3DSimple <unsigned short int> *outimg3d = new Vol3DSimple <unsigned short int> (sx, sy, sz); 
	unsigned short int ***labelimgdata3d = outimg3d->getData3dHandle();
	
	Vol3DSimple<unsigned char> * flagimg = new Vol3DSimple<unsigned char> (inimg3d);
	if (!flagimg) return false;
	
	//initialization
	long i,j,k;
	unsigned char *** flag_p3d = flagimg->getData3dHandle();
	long cellcnt=0;
	
	for(long iz = 0; iz < sz; iz++)
	{
		long offset_k = iz*sx*sy;
		for(long iy =0; iy < sy; iy++)
		{
			long offset_j = offset_k + iy*sx;
			for(long ix = 0; ix < sx; ix++)
			{
				img3d[iz][iy][ix] = pGImg[offset_j+ix];
				flag_p3d[iz][iy][ix] = 0;
				labelimgdata3d[iz][iy][ix] = 0;
			}
		}
	}

	//Set Parameters		
	long rx = 2, ry = 2, rz = 1;                        ////////////////////////////////////////////////////
	long wx=2*rx+1, wy=2*ry+1, wz=2*rz+1;                 /////////////////////////////////////////////////////
	float sigmax=1.5, sigmay=1.5, sigmaz=0.5;               ///////////////////////////////////////////////////////
	float t_pixval=gthresh, t_rgnval=lthresh, t_corrcoef=0.66;//=0.3; //0.66           ////////////////////////////////////////////////
	float merge_radius = 25;			/////////////////////////////////
	float shift_thresh = 25;		/////////////////////////////////

	printf("thresholding %f \n", t_pixval);
	
	//computing
	Vol3DSimple<float> * g = genGaussianKernal3D(wx, wy, wz, sigmax, sigmay, sigmaz);
	if (!g) return false;
	Vol3DSimple<unsigned char> * d = new Vol3DSimple<unsigned char> (wx, wy, wz);
	if (!d) return false;
	long kernel_len = g->getTotalElementNumber();
	
	float * g_1d = g->getData1dHandle();
//	float *** g_3d = g->getData3dHandle();
	unsigned char * d_1d = d->getData1dHandle();
	unsigned char *** d_3d = d->getData3dHandle();
	
	std::vector <LocationSimple> detectedPos;
	//
	for(t_rgnval = Lthresh; t_rgnval >=lthresh; 	t_rgnval--)
	{
		printf("t_rgnval %f \n", t_rgnval);
		
		for (k=0;k<sz;k++)
		{
			if (k<rz || k>(sz-1-rz)) //if at the border, then skip
				continue; 
			
			for (j=0;j<sy;j++)
			{
				if (j<ry || j>(sy-1-ry)) //if at the border, then skip
					continue; 
				
				for (i=0;i<sx;i++)
				{
					if (i<rx || i>(sx-1-rx)) //if at the border, then skip
						continue; 
					
					flag_p3d[k][j][i]=0;
				}
			}
		}
		
		// handle sampling detected puncta here
		for(long ii = 0; ii < detectedList.size(); ii++)
		{
			long x = detectedList.at(ii).x;
			long y = detectedList.at(ii).y;
			long z = detectedList.at(ii).z;
			
			float rx = detectedList.at(ii).rx;
			float ry = detectedList.at(ii).ry;
			float rz = detectedList.at(ii).rz;
			
			long xs = x-rx; if(xs<0) xs = 0;
			long xe = x+rx; if(xe>sx) xe = sx;
			
			long ys = y-ry; if(ys<0) ys = 0;
			long ye = y+ry; if(ye>sy) ye = sy;
			
			long zs = z-rz; if(zs<0) zs = 0;
			long ze = z+rz; if(ze>sz) ze = sz;
			
			for(long k=zs; k<ze; k++)
			{
				for(long j=ys; j<ye; j++)
				{
					for(long i=xs; i<xe; i++)
					{
						flag_p3d[k][j][i]=1;
					}
				}
			}
		}
		

		for (k=0;k<sz;k++)
		{
			if (k<rz || k>(sz-1-rz)) //if at the border, then skip
				continue; 
			
			long offset_k = k*sx*sy; 
			for (j=0;j<sy;j++)
			{
				if (j<ry || j>(sy-1-ry)) //if at the border, then skip
					continue; 
				
				long offset_j = offset_k + j*sx;
				for (i=0;i<sx;i++)
				{
					if (i<rx || i>(sx-1-rx)) //if at the border, then skip
						continue; 
					
					if (flag_p3d[k][j][i]) //if the location has been masked, then skip
						continue;
					
					long idx = offset_j + i;
					
					if (img3d[k][j][i]<=t_pixval || pImMask[idx]==0 ) //do not compute dark pixels and non-ROI region
					{	
						flag_p3d[k][j][i]=1;
						continue;
					}
					
					if(labelimgdata3d[k][j][i])
						continue;
					
					//
					if(bw_maxfiltered[idx])
						continue;
					
					bool b_flag = false;
					
					for(long c=0; c<nc; c++)
					{
						if(flag_sfb[idx+c*pagesz])
							b_flag = true;
					}
					
					if(b_flag)
						continue;
					
					bool b_skip=false;
					//copy data
					long i1,j1,k1, i2,j2,k2;
					for (k1=k-rz,k2=0;k1<=k+rz;k1++,k2++)
						for (j1=j-ry,j2=0;j1<=j+ry;j1++,j2++)
							for (i1=i-rx,i2=0;i1<=i+rx;i1++,i2++)
							{
								d_3d[k2][j2][i2] = img3d[k1][j1][i1];
								if (labelimgdata3d[k1][j1][i1])
									b_skip=true;
							}	
					
					if (b_skip==true)
						continue;
					
					//test regional mean
					unsigned char d_mean, d_std;
					mean_and_std(d_1d, kernel_len, d_mean, d_std);
					if (d_mean<1.5*t_rgnval)
					{	
						flag_p3d[k][j][i]=1;
						continue;
					}
					
					//compute similarity measure using cross correlation
					float score = compute_corrcoef_two_vectors(d_1d, g_1d, kernel_len);
					flag_p3d[k][j][i] = 1; //do not search later
					if (score>=t_corrcoef)
					{
						//first re-estimate the center
						float ncx=i,ncy=j,ncz=k; //new center position
						float ocx,ocy,ocz; //old center position
						float scx=0,scy=0,scz=0,si=0;
						while (1) //mean shift to estimate the true center
						{
							ocx=ncx; ocy=ncy; ocz=ncz;
							
							long rx_ms = 1.5*rx, ry_ms = 1.5*ry, rz_ms = 1.5*rz; //enlarge the radius              /////////////////////////////////////////////////
							
							for (k1=ocz-rz_ms;k1<=ocz+rz_ms;k1++)
							{	
								if (k1<0 || k1>=sz)
									continue;
								for (j1=ocy-ry_ms;j1<=ocy+ry_ms;j1++)
								{
									if (j1<0 || j1>=sy)
										continue;
									for (i1=ocx-rx_ms;i1<=ocx+rx_ms;i1++)
									{
										if (i1<0 || i1>=sx)
											continue;
										float cv = img3d[k1][j1][i1];
										
										if(cv<shift_thresh) cv=0; /////////////////////////////////////////
										
										scz += k1*cv;
										scy += j1*cv;
										scx += i1*cv;
										si += cv;
									}
								}
							}
							if (si>0)
							{ncx = scx/si; ncy = scy/si; ncz = scz/si;}
							else
							{ncx = ocx; ncy = ocy; ncz = ocz;}
							
							if (ncx<rx || ncx>=sx-1-rx || ncy<ry || ncy>=sy-1-ry || ncz<rz || ncz>=sz-1-rz) //move out of boundary
							{
								ncx = ocx; ncy = ocy; ncz = ocz; //use the last valid center
								break;
							}
							
							if (sqrt((ncx-ocx)*(ncx-ocx)+(ncy-ocy)*(ncy-ocy)+(ncz-ocz)*(ncz-ocz))<=1)
								break;
						}
											
						bool b_skip=false;
						//copy data
						long i1,j1,k1, i2,j2,k2;
						for (k1=ncz-rz,k2=0;k1<=ncz+rz;k1++,k2++)
							for (j1=ncy-ry,j2=0;j1<=ncy+ry;j1++,j2++)
								for (i1=ncx-rx,i2=0;i1<=ncx+rx;i1++,i2++)
								{
									d_3d[k2][j2][i2] = img3d[k1][j1][i1];
									if (labelimgdata3d[k1][j1][i1])
										b_skip=true;
								}	
						
						if (b_skip==true)
							continue;
						
						//test regional mean
						mean_and_std(d_1d, kernel_len, d_mean, d_std);
						if (d_mean<t_rgnval)
						{	
							flag_p3d[k][j][i]=1;
							continue;
						}
						
						float lncx=ncx,lncy=ncy,lncz=ncz;
						
						//then estimate the new radius
						long rx_ms = 1.1*rx, ry_ms = 1.1*ry, rz_ms = 1.1*rz; //enlarge the radius         //////////////////////////////////////////
						
						scz=0; scy=0;scx=0;si=0;
						for (k1=lncz-rz_ms;k1<=lncz+rz_ms;k1++)
						{	
							for (j1=lncy-ry_ms;j1<=lncy+ry_ms;j1++)
							{
								for (i1=lncx-rx_ms;i1<=lncx+rx_ms;i1++)
								{
									if(i1<0 || i1>sx-1 || j1 < 0 || j1 > sy-1 || k1 < 0 || k1 > sz-1)
										continue;
									else
									{
										float cv = img3d[k1][j1][i1];
										
										scz += cv*(k1-lncz)*(k1-lncz);
										scy += cv*(j1-lncy)*(j1-lncy);
										scx += cv*(i1-lncx)*(i1-lncx);
										si += cv;
									}
								}
							}
						}
						float stdx, stdy, stdz;
						if (si>0)
						{
							stdx = sqrt(scx/si); stdy = sqrt(scy/si); stdz = sqrt(scz/si); 
							//printf("cell=%d, %5.3f (%5.3f) %5.3f (%5.3f) %5.3f (%5.3f)\n", cellcnt+1, lncx, stdx, lncy, stdy, lncz, stdz);
						}
						else
						{
							printf("Error happens in estimating the standard deviation in template_matching_seg(). Force setting std to be 1.\n");
							stdx = 1; stdy = 1; stdz = 1; 
						}
						
						//mask
						bool b_merge=false; int celllabel;
						float lpthresh = 50;                             ////////////////////////////////////////////////////////  
						for (unsigned short int pii=0;pii<detectedPos.size();pii++)
						{
							register float tmpx = detectedPos.at(pii).x-lncx; 
							register float tmpy = detectedPos.at(pii).y-lncy; 
							register float tmpz = detectedPos.at(pii).z-lncz;
							if (tmpx*tmpx+tmpy*tmpy<merge_radius*merge_radius && tmpz<=5 && 
								compute_3dlineprofile(inimg3d, (long)lncx, (long)lncy, (long)lncz,  (long)detectedPos.at(pii).x, (long)detectedPos.at(pii).y, (long)detectedPos.at(pii).z, lpthresh)) 
							{
								b_merge=true;
								celllabel = pii+1; //index start from 1
								
								printf("merge!\n");
								break;
							}
						}
						
						if (b_merge==false)
						{
							LocationSimple pp(lncx, lncy, lncz);
							detectedPos.push_back(pp);
							cellcnt++;
							celllabel = cellcnt;
						}
						else
						{
							while (1) //mean shift to reestimate the true center after merge
							{
								ocx=ncx; ocy=ncy; ocz=ncz;
								
								long rx_ms = 1.1*rx, ry_ms = 1.1*ry, rz_ms = 1.1*rz; //enlarge the radius              /////////////////////////////////////////////////
								
								for (k1=ocz-rz_ms;k1<=ocz+rz_ms;k1++)
								{	
									if (k1<0 || k1>=sz)
										continue;
									for (j1=ocy-ry_ms;j1<=ocy+ry_ms;j1++)
									{
										if (j1<0 || j1>=sy)
											continue;
										for (i1=ocx-rx_ms;i1<=ocx+rx_ms;i1++)
										{
											if (i1<0 || i1>=sx)
												continue;
											float cv = img3d[k1][j1][i1];
											
											if(cv<shift_thresh) cv=0; /////////////////////////////////////////
											
											scz += k1*cv;
											scy += j1*cv;
											scx += i1*cv;
											si += cv;
										}
									}
								}
								if (si>0)
								{ncx = scx/si; ncy = scy/si; ncz = scz/si;}
								else
								{ncx = ocx; ncy = ocy; ncz = ocz;}
								
								if (ncx<rx || ncx>=sx-1-rx || ncy<ry || ncy>=sy-1-ry || ncz<rz || ncz>=sz-1-rz) //move out of boundary
								{
									ncx = ocx; ncy = ocy; ncz = ocz; //use the last valid center
									break;
								}
								
								if (sqrt((ncx-ocx)*(ncx-ocx)+(ncy-ocy)*(ncy-ocy)+(ncz-ocz)*(ncz-ocz))<=1)
									break;
							}
							//then estimate the new radius
							long rx_ms = rx, ry_ms = ry, rz_ms = rz; //enlarge the radius         //////////////////////////////////////////
							
							scz=0; scy=0;scx=0;si=0;
							for (k1=lncz-rz_ms;k1<=lncz+rz_ms;k1++)
							{	
								for (j1=lncy-ry_ms;j1<=lncy+ry_ms;j1++)
								{
									for (i1=lncx-rx_ms;i1<=lncx+rx_ms;i1++)
									{
										if(i1<0 || i1>sx-1 || j1 < 0 || j1 > sy-1 || k1 < 0 || k1 > sz-1)
											continue;
										else
										{
											float cv = img3d[k1][j1][i1];
											
											scz += cv*(k1-lncz)*(k1-lncz);
											scy += cv*(j1-lncy)*(j1-lncy);
											scx += cv*(i1-lncx)*(i1-lncx);
											si += cv;
										}
									}
								}
							}
							float stdx, stdy, stdz;
							if (si>0)
							{
								stdx = sqrt(scx/si); stdy = sqrt(scy/si); stdz = sqrt(scz/si); 
								//printf("cell=%d, %5.3f (%5.3f) %5.3f (%5.3f) %5.3f (%5.3f)\n", cellcnt+1, lncx, stdx, lncy, stdy, lncz, stdz);
							}
							else
							{
								printf("Error happens in estimating the standard deviation in template_matching_seg(). Force setting std to be 1.\n");
								stdx = 1; stdy = 1; stdz = 1; 
							}
						}
						
						float sa2b2c2 = 8*stdx*stdx*stdy*stdy*stdz*stdz;
						float sa2b2 = 4*stdx*stdx*stdy*stdy;
						float sb2c2 = 4*stdy*stdy*stdz*stdz;
						float sa2c2 = 4*stdx*stdx*stdz*stdz;
						
						//output
						pos.pixmax = 0;
						pos.intensity = 0;
						pos.sdev = 0;
						pos.volsize = 0;
						pos.mass = 0;
						
						for (k1=lncz-2*stdz,k2=0;k1<=lncz+2*stdz;k1++,k2++)
						{
							for (j1=lncy-2*stdy,j2=0;j1<=lncy+2*stdy;j1++,j2++)
							{
								for (i1=lncx-2*stdx,i2=0;i1<=lncx+2*stdx;i1++,i2++)
								{	
									if (k1>=0 && k1<sz && j1>=0 && j1<sy && i1>=0 && i1<sx)
									{	
										if ((k1-lncz)*(k1-lncz)*sa2b2+(j1-lncy)*(j1-lncy)*sa2c2+(i1-lncx)*(i1-lncx)*sb2c2 <= sa2b2c2)
										{
											flag_p3d[k1][j1][i1] = 1;
											labelimgdata3d[k1][j1][i1] = celllabel;
											
											float tmp_val = img3d[k1][j1][i1];
											
											if(pos.pixmax<tmp_val) pos.pixmax = tmp_val;
											pos.volsize ++;
											pos.mass += tmp_val;
											
										}
									}
								}
							}
						}
						//printf("%ld done \n", cellcnt);
						
						pos.intensity = pos.mass / pos.volsize;
						for (k1=lncz-2*stdz,k2=0;k1<=lncz+2*stdz;k1++,k2++)
						{
							for (j1=lncy-2*stdy,j2=0;j1<=lncy+2*stdy;j1++,j2++)
							{
								for (i1=lncx-2*stdx,i2=0;i1<=lncx+2*stdx;i1++,i2++)
								{	
									if (k1>=0 && k1<sz && j1>=0 && j1<sy && i1>=0 && i1<sx)
									{	
										if ((k1-lncz)*(k1-lncz)*sa2b2+(j1-lncy)*(j1-lncy)*sa2c2+(i1-lncx)*(i1-lncx)*sb2c2 <= sa2b2c2)
										{
											
											float tmp_val = img3d[k1][j1][i1];

											pos.sdev += (tmp_val-pos.intensity)*(tmp_val-pos.intensity);
										}
									}
								}
							}
						}
						pos.sdev = sqrt(pos.sdev);
						pos.x = ncx; pos.y = ncy; pos.z = ncz;
						pos.rx = stdx; pos.ry = stdy; pos.rz = stdz;
						pos.num = pImMask[k*sx*sy+j*sx+i];
						detectedList.push_back(pos);
					}
				}
			}
		}
	}
		
	//free space
	if (inimg3d) {delete inimg3d; inimg3d=0;}
	if (flagimg) {delete flagimg; flagimg=0;}
	if (d) {delete d; d=0;}
	if (g) {delete g; g=0;}
	
	int end_segmentation = clock();
	
	printf("time eclapse %d ms for segmentation!\n", (end_segmentation-start_t)/1000);
	
		
	//de-alloc
	if(flag_computed) {delete []flag_computed; flag_computed=0;}
	if(img_subject) {delete img_subject; img_subject=0;}
//	
//	//-----------------------------------Saving Output Images------------------------------------
//	//Save as 16-bit gray image stack for puncta detected image
//	if(dfile_warped)
//	{
//		sz_subject[3]=1;
//		if (saveImage(dfile_warped, (unsigned char *)outimg3d->getData1dHandle(), sz_subject, 2)!=true)
//		{
//			fprintf(stderr, "Error happens in file writing. Exit. \n");
//			return false;
//		}
//	}
//	
	//de-alloc
	if(outimg3d) {delete outimg3d; outimg3d=0;}
//	
//	if(dfile_masked)
//	{
//		printf("save mask file...\n");
//		
//		//Save as 8-bit RGB channels image stacks for mask image
//		unsigned char *pMask = new unsigned char [pagesz];
//		for(long k=0; k<sz; k++)
//		{
//			for(long j=0; j<sy; j++)
//			{
//				for(long i=0; i<sx; i++)
//				{
//					long indLoop = k*sx*sy + j*sx + i;
//					
//					if(pImMask[indLoop])
//						pMask[indLoop] = 255;
//					else
//						pMask[indLoop] = 0;
//				}
//			}
//		}
//
//		long save_sz[4];
//		save_sz[0]=sx; save_sz[1]=sy; save_sz[2]=sz; save_sz[3]=sc;
//		if (saveImage(dfile_masked, (unsigned char *)pMask, save_sz, 1)!=true)
//		{
//			fprintf(stderr, "Error happens in file writing. Exit. \n");
//			return false;
//		}
//		
//		//Save the overlaped image
//		if(dfile_overlaped)
//		{
//			sz_subject[3]=3;
//			long totalpxls = sz_subject[3]*pagesz;
//			long offsets2 = 2*pagesz;
//			unsigned char *pOLImg = new unsigned char [totalpxls];
//			if (!pOLImg) 
//			{
//				printf("Fail to allocate memory.\n");
//				return false;
//			}
//			else
//			{
//				//red channel
//				for(long i=0; i<pagesz; i++ )
//					pOLImg[i] = img_subject[i];
//				
//				//green channel
//				for(long k=0; k<sz; k++)
//				{
//					for(long j=0; j<sy; j++)
//					{
//						for(long i=0; i<sx; i++)
//						{
//							long ind = pagesz + k*sx*sy + j*sx + i; 
//							if(labelimgdata3d[k][j][i]>0)
//								pOLImg[ind] = 255;	//img_subject[k*sx*sy + j*sx + i];
//							else
//								pOLImg[ind] = 0;
//						}
//					}
//				}
//				
//				//blue channel
//				for(long i=offsets2; i<totalpxls; i++)
//					pOLImg[i] = pMask[i-offsets2];
//			}
//			if (saveImage(dfile_overlaped, (unsigned char *)pOLImg, sz_subject, 1)!=true)
//			{
//				fprintf(stderr, "Error happens in file writing. Exit. \n");
//				return false;
//			}
//			
//			//de-alloc
//			if(pOLImg) {delete []pOLImg; pOLImg=0;}
//		}
//		
//		//de-alloc
//		if(pMask) {delete []pMask; pMask=0;}
//
//	}
//	
//	//de-alloc
//	if(pImMask) {delete []pImMask; pImMask=0;}
//	
	//save output to apo file
	FILE *pFileApo=0;
	countSwc = 0;
	
	std::ostringstream os1;
	os1<<lthresh;
	
	foreach (QString traced_neuron, neuronsList)
	{
		string str;
		
		str = traced_neuron.toStdString();

		//str -= ".swc";
		str += "_small.apo";
		const char* strfile = new char [sizeof(str)+1];
		strfile = str.c_str();
		
		pFileApo = fopen(strfile,"wt");
		
		countSwc++;
		
		long LABELBIN = detectedList.size();
		for(long ii = 0; ii < LABELBIN; ii++)
		{

			if(long(detectedList.at(ii).num/num_supported)==countSwc)
			{
				CellAPO S;

				S.n = ii+1;
				S.orderinfo = QString::fromStdString(os1.str()); //""; //local threshold
				S.name = "";
				S.comment = "";
				S.z = detectedList.at(ii).z;
				S.x = detectedList.at(ii).x;
				S.y = detectedList.at(ii).y;
				S.pixmax = detectedList.at(ii).pixmax;
				S.intensity = detectedList.at(ii).intensity;
				S.sdev = detectedList.at(ii).sdev;
				S.volsize = detectedList.at(ii).volsize;
				S.mass = detectedList.at(ii).mass;
				S.color.r = 0; S.color.g = 0; S.color.b = 0;
				
				fprintf(pFileApo, "%d,%s,%s,%s,%ld,%ld,%ld,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
						S.n, //i+1,
						qPrintable(S.orderinfo),
						qPrintable(S.name),
						qPrintable(S.comment),
						long(S.z+0.5),
						long(S.x+0.5),
						long(S.y+0.5),
						S.pixmax,
						S.intensity,
						S.sdev,
						S.volsize,
						S.mass
						
						//					S.color.r,
						//					S.color.g,
						//					S.color.b
						);
				
			}
		}
		fclose(pFileApo);
	}
//	
//	//saving ROC file
////	FILE *pFileMarker=0;
////	
////	string str;
////				
////	str = dfile_traced;
////
////	str += ".marker";
////	const char* strfile = new char [sizeof(str)+1];
////	strfile = str.c_str();
////				
////	pFileMarker = fopen(strfile,"wt");
////
////	for (long k=0;k<sz;k++)
////	{
////		long offsetk = k*sx*sy;
////		for (long j=0;j<sy;j++)
////		{
////			long offsetj = j*sx;
////			for (long i=0;i<sx;i++)
////			{
////
////				if(flag_roc[offsetk+offsetj+i])
////				{
////					ImageMarker S;
////					
////					S.x = i;
////					S.y = j;
////					S.z = k;
////					S.radius = 1;
////					S.shape = 1;
////					S.name = "";
////					S.comment = "";
////					
////					S.color = random_rgba8(255);
////					S.color.r = 128;
////					S.color.g = 128;
////					S.color.b = 128;
////					
////					S.type = (S.x==-1 || S.y==-1 || S.z==-1) ? 0 : 2;
////					
////					S.on = true;
////					S.selected = false;
////					
////					fprintf(pFileMarker, "%5.3f, %5.3f, %5.3f, %d, %ld, %s, %s, %d,%d,%d\n",
////							S.x,
////							S.y,
////							S.z,
////							int(S.radius), S.shape,
////							qPrintable(S.name), qPrintable(S.comment),
////							S.color.r,S.color.g,S.color.b
////							);
////						
////				}
////
////			}
////		}
////	}
////	fclose(pFileMarker);
//
//	string str1;		
//	str1 = dfile_traced;
//
//	str1 += ".raw";
//	const char* strfile1 = new char [sizeof(str1)+1];
//	strfile1 = str1.c_str();
//
//	sz_subject[3]=1;
//	
//	unsigned char *pROC = new unsigned char [pagesz];
//	for(long i=0; i<pagesz; i++)
//	{
//		
//		if(flag_roc[i])
//			pROC[i] = 255;
//		else
//			pROC[i] = 0;
//	}
//	
//	if (saveImage(strfile1, (unsigned char *)pROC, sz_subject, 1)!=true)
//	{
//		fprintf(stderr, "Error happens in file writing. Exit. \n");
//		return false;
//	}
//	
//	//de-alloc
//	if(flag_roc) {delete []flag_roc; flag_roc=0;}
//	if(pROC) {delete []pROC; pROC=0;}
//	
//	//other option for saving
//	if(dfile_apo)
//	{
//		FILE *pFileAPO=0;
//		pFileAPO = fopen(dfile_apo,"wt");
//		
//		QList <CellAPO> apoList;
//		long LABELBIN = detectedList.size(); 
//		
//		for(long ii = 0; ii < LABELBIN; ii++)
//		{
//			CellAPO S;
//			
//			S.n = ii+1;
//			S.orderinfo = "";
//			S.name = "";
//			S.comment = "";
//			S.z = detectedList.at(ii).z;
//			S.x = detectedList.at(ii).x;
//			S.y = detectedList.at(ii).y;
//			S.pixmax = detectedList.at(ii).pixmax;
//			S.intensity = detectedList.at(ii).intensity;
//			S.sdev = detectedList.at(ii).sdev;
//			S.volsize = detectedList.at(ii).volsize;
//			S.mass = detectedList.at(ii).mass;
//			S.color.r = 0; S.color.g = 0; S.color.b = 0;
//			
//			fprintf(pFileAPO, "%d,%s,%s,%s,%ld,%ld,%ld,%5.3f,%5.3f,%5.3f,%5.3f,%5.3f,,,\n",
//					S.n, //i+1,
//					qPrintable(S.orderinfo),
//					qPrintable(S.name),
//					qPrintable(S.comment),
//					long(S.z+0.5),
//					long(S.x+0.5),
//					long(S.y+0.5),
//					S.pixmax,
//					S.intensity,
//					S.sdev,
//					S.volsize,
//					S.mass
//
////					S.color.r,
////					S.color.g,
////					S.color.b
//					);
//			
//			
//			apoList.append(S);
//		}
//		fclose(pFileAPO);
//	}



	return true;
}
