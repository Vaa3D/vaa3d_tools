//main brain segmentation program
//by Fuhui Long
//2008-10-24

#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"
#include "./local_basic_c_fun/basic_memory.h"

#include "FL_main_brainseg.h"
#include "FL_downSample3D.h"

#include <string>

#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

extern char *optarg;
extern int optind, opterr;

//run -i test.tif -s templateseg.raw -o 1.raw -c 1 -D 2 -r [12] -n [1,34] ; for segmenting special regions
//run -i test.tif -s templateseg.raw -o 1.raw -c 1 -D 2 ; for segmenting all regions and all use models

void printHelp();
void printHelp()
{
   printf("\nUsage: prog_name -i <infilename> -s <segfilename> -o <outfilename> -c <channel> -D <dimension> -r <region> -n <nomodel> -l <lamda> -a <alf> -e <epsilon> -d <delt> -g <gama>\n");
	printf("[h]			help\n");
	return;
}

// parse array input
void parse_Radii_Paremeter(char * optarg, double * &radius, V3DLONG & len_Radii)
{
	//the input must be set as 0
	if (!radius)
	{
		delete []radius;
		radius=0;
	}
	
	if (len_Radii!=0)
		len_Radii=0;
	
	//
	
	V3DLONG i=0, len, n;
	while (optarg[i++]!='\0') {}
	len=i-1;
	if (optarg[0]!='[' || optarg[len-1]!=']' || len>=1024)
	{
		printf("The radii parameter must have the format [xxx,xxx,...] where comma is the separator and no space is allowed, the first and last characters must be brackets.\n");
		return;
	}
	
	n=1;
	for (i=1;i<=len-2;i++)
	{
		n = (optarg[i]==',')?n+1:n;
		if ((optarg[i]>'9' || optarg[i] <'0') && optarg[i]!=',')
		{
			printf("Detected illegal characters in the radii parameters.\n");
			return;
		}
	}
	
	radius = new double [n];
	if (!radius)
	{
		printf("Fail to allocate memory for radii.\n");
		len_Radii=0;
		return;
	}
	
	char buf[1024]; //use a big 1K buffer
	
	if (n==1)
	{
		strncpy(buf, optarg+1, len-2); buf[len-2]='\0';
		radius[0] = atof(buf);
		len_Radii=1;
		return;
	}
	
	V3DLONG cb,ce,cb_new=1;
	int k=0;
	printf("n=%d\n",n);
	while (k<n)
	{
		cb=cb_new;
		int b_foundcomma=0;
		for (int i=cb;i<=len-2;i++)
		{
			if (optarg[i]==',')
			{
				ce = i-1;
				if (ce<cb) //detect a case like ",," or "[,"
				{
					b_foundcomma=1;
					cb_new=ce+2;
					break;
				}
				
				if (ce-cb+1>3)
				{
					printf("You have specified a number with more than 3 digits V3DLONG, which is unlikely the correct input. Exit!");
					exit(1);
				}
				else
				{
					strncpy(buf, optarg+cb, ce-cb+1); buf[ce-cb+1]='\0';
					radius[k] = atof(buf);
				}
				cb_new=ce+2;
				b_foundcomma=1;
				break;
			}
		}
		if (b_foundcomma==0) //this is the last value which has no comma left
		{
			ce=len-2;
			if (ce>=cb)
			{
				strncpy(buf, optarg+cb, ce-cb+1); buf[ce-cb+1]='\0';
				radius[k] = atof(buf);
			}
		}
		
		if (ce<cb) //i.e. ce-cb+1=1, or the case of ",,",",]","[,"
		{
			n--; //in this case, decease the total valid number
		}
		else
		{
			k++;
		}
	}
	len_Radii=n;
	printf("You have specified radius range to be %d number series {", len_Radii);
	for(i=0;i<len_Radii;i++)
	{
		printf("%5.2f",radius[i]);
		if (i<len_Radii-1) printf(",");
	}
	printf("}\n");
    
	return;
}



int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}

	levelset_segParameter segpara;

//	// set default values
//	segpara.channelNo = 2;
//	segpara.lamda = 0.5; //coefficient of the weighted length term Lg(\phi)
//	segpara.alf =1; //coefficient of the weighted area term Ag(\phi);
//	segpara.epsilon = 1.5; // the papramater in the definition of smoothed Dirac function
//	segpara.delt = 5; // time step
//	segpara.mu = 0.1/segpara.delt; //coefficient of the internal (penalizing) energy term P(\phi)	
//	segpara.gama = 0.001;	
////	segpara.method = 1;
	
	/* Read arguments */

	V3DLONG c;
	static char optstring[] = "h:i:s:o:c:D:r:n:l:a:e:d:g:";
	opterr = 0;
	
	double *paraArray;
	V3DLONG len;
					

	while ((c = getopt (argc, argv, optstring)) != -1)
    {
		switch (c)
        {
			case 'h':
				printHelp ();
				return 0;
				break;

			case 'i': // file name of input image of which need to be segmented
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				segpara.infilename = optarg;
				break;

			case 's': // file name of segmented template
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -i.\n");
					return 1;
				}
				segpara.segfilename = optarg;
				break;

			case 'o': // filename of output image
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				segpara.outfilename = optarg;
				break;

			case 'c': // which channel to process
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				segpara.channelNo = atoi (optarg); // should be 0, 1, 2,...
				if (segpara.channelNo < 0)
				{
					fprintf (stderr, "Illeagal channelNo found! It must be >=0.\n");
					return 1;
				}
				break;

			case 'D': // 2D or 3D segmentation
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				segpara.dimension = atoi (optarg); // should be 2 or 3
				if ((segpara.dimension < 2) || (segpara.dimension >3))
				{
					fprintf (stderr, "Illeagal channelNo found! It must be >=0.\n");
					return 1;
				}
				break;

			case 'r': 
				{// which regions to segment, command line should be sth like: -r [1, 10, 28, 43]
					if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
					{
						fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
						return 1;
					}
					
					paraArray = 0;
					
					parse_Radii_Paremeter(optarg, paraArray, len);
					
					for (int i=0; i<len; i++)
					{
						if (paraArray[i] < 0)
						{
							fprintf (stderr, "Illeagal region label found! It must be >=0.\n");
							return 1;
						}				
						segpara.regions[i] = (int)paraArray[i];
					}
					
					segpara.regionnum  = len;
					break;
				}
			case 'n': // which regions should not use model-driven segmentation, it should have the same format as parameter -r 
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -b.\n");
					return 1;
				}


				paraArray = 0;
				
				parse_Radii_Paremeter(optarg, paraArray, len);
				
				for (int i=0; i<len; i++)
				{
					if (paraArray[i] < 0)
					{
						fprintf (stderr, "Illeagal region label found! It must be >=0.\n");
						return 1;
					}				
					segpara.regionNoModel[i] = (int)paraArray[i];
				}
				segpara.regionNoModelNum = len;
				break;

			case 'l': //lamda
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -b.\n");
					return 1;
				}
				segpara.lamda = (float)(atof(optarg));
				break;

			case 'a': //alf
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				segpara.alf = (float)(atof(optarg));
				break;

			case 'e': //epsilon
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				segpara.epsilon = (float)(atof(optarg));
				break;

			case 'd': //delt, i.e., time step
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				segpara.delt = (float)(atof(optarg));
				break;

			case 'g': //gama
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				segpara.gama = (float)(atof(optarg));
				break;

        }
    }

	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);

	/* Read input file and display some information. */

	unsigned char * inimg1d = 0; /* note that this variable must be initialized as NULL. */
	V3DLONG * sz = 0; /* note that this variable must be initialized as NULL. */
	int datatype = 0;

	//////////////////////////////////////////////////////////////////////////////////
	////20080917 by RZC for win32(mingw): to get rid of error: crosses initialization
	Vol3DSimple <unsigned char> *rawimg3d = 0;
	Vol3DSimple <unsigned char> *rawimgsmall3d = 0;
	Vol3DSimple <unsigned char> *rawimgsmallRotate3d = 0;	
	Vol3DSimple <unsigned short int> *segimg3d = 0;
	V3DLONG *data_sz = 0;
	Vol3DSimple <unsigned short int> *segimgsmall3d = 0;
	Vol3DSimple <unsigned short int> *segimgsmallRotate3d = 0;
	Vol3DSimple <unsigned short int> *outimg3d = 0;
	Vol3DSimple <unsigned char> *outimgBoundary3d = 0;
	
	unsigned char ***rawimgsmalldata3d;
	unsigned short int ***outimgdata3d;
	unsigned char ***outimgBoundaryData3d;
	
	stack4DSimple <unsigned short int> *outimg4d = 0;
	unsigned short int ****outimgdata4d;
	
	bool b_res = false;
	unsigned char * segimg1d = 0;
	V3DLONG * sz2 = 0;
	Vol3DSimple <unsigned char> * tmp=0;
	int i;
	double dfactor[3]; //downsample factor
	dfactor[0] = 4; dfactor[1]=4; dfactor[2] = 2;
//	dfactor[0] = 2; dfactor[1]=2; dfactor[2] = 2;

	//////////////////////////////////////////////////////////////////////////////////


	if (loadImage(segpara.infilename, inimg1d, sz, datatype)!=true)
	{
		fprintf (stderr, "Error happens in reading the file [%s]. Exit. \n", segpara.infilename);
		goto Label_exit;
	}

	//Vol3DSimple <unsigned char> *img3d = new Vol3DSimple <unsigned char> (inimg1d, sz, segpara.channelNo); //input 3D image
	rawimg3d = new Vol3DSimple <unsigned char> (inimg1d, sz, segpara.channelNo); //inimg1d is a 4d image, rawimg3d get a particular channel
	
	//downsample
	downsample3dvol(rawimgsmall3d, rawimg3d, dfactor, 0);

	printf("%d\n", segpara.channelNo);
	printf("%d, %d, %d\n", rawimgsmall3d->sz0(), rawimgsmall3d->sz1(), rawimgsmall3d->sz2()); 
	if (inimg1d) {delete []inimg1d; inimg1d=0;}
	

	/* Read template label file and display some information. */

	if (loadImage(segpara.segfilename, segimg1d, sz2, datatype)!=true)
	{
		fprintf (stderr, "Error happens in reading the file [%s]. Exit. \n", segpara.segfilename);
		goto Label_exit;
	}

	if (datatype==1)
	{
		tmp = new Vol3DSimple <unsigned char > (segimg1d, sz, 0); 
		unsigned char *tmp1d = tmp->getData1dHandle();		
		segimg3d = new Vol3DSimple <unsigned short int > (sz[0], sz[1], sz[2]); //assumuing channel 1 is the label channel
		unsigned short int *segimg3ddata1d = segimg3d->getData1dHandle();
		
		for (i=0; i<sz[2]*sz[1]*sz[0]; i++)
			segimg3ddata1d[i] = (unsigned short int) tmp1d[i];
		
		if (tmp) {delete tmp; tmp=0;}		
	}
		
	if (datatype==2)
		segimg3d = new Vol3DSimple <unsigned short int > ((unsigned short int *)segimg1d, sz, 0); //assumuing channel 1 is the label channel

	//downsample
	downsample3dvol(segimgsmall3d, segimg3d, dfactor, 1);
	printf("%d, %d, %d\n", segimgsmall3d->sz0(), segimgsmall3d->sz1(), segimgsmall3d->sz2()); 
	
	if (segimg1d) {delete []segimg1d; segimg1d=0;}

	//V3DLONG *data_sz = new V3DLONG [3];
	data_sz = new V3DLONG [3];
//	data_sz[0]= rawimg3d->sz0();
//	data_sz[1]= rawimg3d->sz1();
//	data_sz[2]= rawimg3d->sz2();
	data_sz[0]= rawimgsmall3d->sz0();
	data_sz[1]= rawimgsmall3d->sz1();
	data_sz[2]= rawimgsmall3d->sz2();

	len = data_sz[0] * data_sz[1] * data_sz[2];
	
	// KEEP THIS
//	//switch x,y dimension if x dimension size is smaller than y dimension size, make sure orientation is standard
//	if (data_sz[0]>data_sz[1])
//	{
//	
//		segimgsmallRotate3d = new Vol3DSimple <unsigned short int> (data_sz[1], data_sz[0], data_sz[2]);
//		unsigned short int *** segimgsmallRotate3ddata3d = segimgsmallRotate3d->getData3dHandle();
//		unsigned short int *** segimgsmall3ddata3d = segimgsmall3d->getData3dHandle();
//		
//		for (int k=0; k<data_sz[2]; k++)
//		for (int j=0; j<data_sz[1]; j++)
//		for (int i=0; i<data_sz[0]; i++)
//		{
//			segimgsmallRotate3ddata3d[k][i][j] = segimgsmall3ddata3d[k][j][i];
//		}
//		
//		printf("%d, %d, %d\n", segimgsmallRotate3d->sz0(), segimgsmallRotate3d->sz1(), segimgsmallRotate3d->sz2()); 
//
//	}
	

	outimg3d = new Vol3DSimple <unsigned short int> (data_sz[0], data_sz[1], data_sz[2]); //output 3D image
	outimgdata3d = outimg3d->getData3dHandle();
	
	//now do computation

//	b_res = brainSeg2D(rawimg3d, segimg3d, outimg3d, segpara); //use original size image to compute
	b_res = brainSeg2D(rawimgsmall3d, segimgsmall3d, outimg3d, segpara); //use downsampled image to compute
	
//	b_res = 1;
	
	// extract region boundary
	outimgBoundary3d = new Vol3DSimple <unsigned char> (data_sz[0], data_sz[1], data_sz[2]);
	outimgBoundaryData3d = outimgBoundary3d->getData3dHandle();
	
	for (int k=0; k<data_sz[2]; k++)
	for (int j=0; j<data_sz[1]; j++)
	for (int i=0; i<data_sz[0]; i++)
	{
		outimgBoundaryData3d[k][j][i] = 0;
	}
	
	for (int k=1; k<data_sz[2]-1; k++)
	for (int j=1; j<data_sz[1]-1; j++)
	for (int i=1; i<data_sz[0]-1; i++)
	{
		if (outimgdata3d[k][j][i]>0)
		{
			unsigned char tag = 0;
			int p=-1;
			
			while ((p<=1)&&(tag==0))
			{
				int q=-1;
				while ((q<=1)&&(tag==0))
				{
					if (outimgdata3d[k][j+p][i+q]!=outimgdata3d[k][j][i])
					{
						outimgBoundaryData3d[k][j][i] = 255;
						tag = 1;
					}
					else
					{
						q++;
					}
				}
				p++;
			}
		}
	}
	
	// overlay boundary of segmenation result on top of the original image, generating a 4d stack for v3d to visualize
	outimg4d = new stack4DSimple <unsigned short int> (data_sz[0], data_sz[1], data_sz[2], 2); //stack for overlaying original image and the segmenation boundary
	outimgdata4d = outimg4d->getData4dHandle();
	
	rawimgsmalldata3d = rawimgsmall3d->getData3dHandle();
	
	for (int k=0; k<data_sz[2]; k++)
	for (int j=0; j<data_sz[1]; j++)
	for (int i=0; i<data_sz[0]; i++)
	{
		outimgdata4d[0][k][j][i] = rawimgsmalldata3d[k][j][i];
	}
	
	for (int k=0; k<data_sz[2]; k++)
	for (int j=0; j<data_sz[1]; j++)
	for (int i=0; i<data_sz[0]; i++)
	{
		outimgdata4d[1][k][j][i] = outimgBoundaryData3d[k][j][i];
	}
		

	/* save files */
	if (b_res)
	{
		V3DLONG sz[4];
//		sz[0] = rawimgsmall3d->sz0(); sz[1]= rawimgsmall3d->sz1(); sz[2]= rawimgsmall3d->sz2(); sz[3] = 1;
//		if (saveImage(segpara.outfilename,(const unsigned char *) outimg3d->getData1dHandle(), sz, sizeof(unsigned short int))!=true) // save segmenation mask
////		if (saveImage(segpara.outfilename,(const unsigned char *) outimgBoundary3d->getData1dHandle(), sz, sizeof(unsigned char))!=true) //save segmenation boundary

		sz[0] = rawimgsmall3d->sz0(); sz[1]= rawimgsmall3d->sz1(); sz[2]= rawimgsmall3d->sz2(); sz[3] = 2;
		if (saveImage(segpara.outfilename,(const unsigned char *) outimg4d->getData1dHandle(), sz, sizeof(unsigned short int))!=true) //save original image overlaied with segmenation boundary
		
		
//		if (saveImage(segpara.outfilename,(const unsigned char *) segimgsmall3d->getData1dHandle(), sz, sizeof(unsigned short int))!=true)
//		if (saveImage(segpara.outfilename,(const unsigned char *) rawimgsmall3d->getData1dHandle(), sz, sizeof(unsigned char))!=true)

		{
			fprintf(stderr, "Error happens in file writing. Exit. \n");
		}
    }
	else
	{
		fprintf(stderr, "Fail to generate the brain segmentation results. \n");
	}

	/* clean all workspace variables */

Label_exit:

	if (sz) {delete [] sz; sz=0;}
	if (sz2) {delete [] sz2; sz2=0;}
	if (data_sz) {delete [] data_sz; data_sz =0;}
	if (outimg3d) {delete outimg3d; outimg3d =0;}
	if (rawimg3d) {delete rawimg3d; rawimg3d=0;}
	if (rawimgsmall3d) {delete rawimgsmall3d; rawimgsmall3d=0;}
//	if (rawimgsmallRotate3d) {delete rawimgsmallRotate3d; rawimgsmallRotate3d=0;}	
	
	if (segimg1d) {delete segimg1d; segimg1d=0;}
	if (segimg3d) {delete segimg3d; segimg3d=0;}	
	if (segimgsmall3d) {delete segimgsmall3d; segimgsmall3d=0;}	
//	if (segimgsmallRotate3d) {delete segimgsmallRotate3d; segimgsmallRotate3d=0;}	
		
	if (outimgBoundary3d) {delete outimgBoundary3d; outimgBoundary3d=0;}
	if (outimg4d) {delete outimg4d; outimg4d =0;}
	
//	if (tmp) {delete tmp; tmp=0;}
		
	return 0;

}
