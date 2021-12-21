//main cell segmentation program
//by Fuhui Long
//2008-03-07
//2008-08-23: separated as the main function by Hanchuan Peng

#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"

#include "FL_cellSegmentation3D.h"

#include <string>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

extern char *optarg;
extern int optind, opterr;

void printHelp();
void printHelp()
{
   printf("\nUsage: prog_name -i <infilename> -o <outfilename> -c <channel> -m <segMethod> -w <medianFiltWid> -r <gaussinFiltWid>  -g <gaussinFiltSigma> -d <adaptiveThreWid> -s <adaptThreStp> -t <adaptThreStpMethod>\n");
	printf("[h]			help\n");
	return;
}


int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printHelp ();
		return 0;
	}

	segParameter segpara;

	// set default values
	segpara.channelNo = 2;
//	segpara.backgndLevel = 15;
	segpara.medianFiltWid = 2;
	segpara.gaussinFiltWid = 1;
	segpara.gaussinFiltSigma = 1;	
	segpara.adaptThreWid = 10;
	segpara.adaptThreStp = 5;
//	segpara.segMethod = 1; // shape-based watershed
	segpara.segMethod = 0; // intensity-based watershed
//	segpara.adpatThreMethod = 0; //local adpative thresholding
//	segpara.adpatThreMethod = 1; //global thresholding
	segpara.adpatThreMethod = 2; //global thresholding in 2D (slide by slide)

	/* Read arguments */

	V3DLONG c;
	static char optstring[] = "h:i:o:c:m:w:r:g:d:s:t:";
	opterr = 0;

	while ((c = getopt (argc, argv, optstring)) != -1)
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
				segpara.infilename = optarg;
				break;

			case 'o':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -o.\n");
					return 1;
				}
				segpara.outfilename = optarg;
				break;

			case 'c':
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

//			case 'b':
//				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
//				{
//					fprintf (stderr, "Found illegal or NULL parameter for the option -b.\n");
//					return 1;
//				}
//				segpara.backgndLevel = atoi(optarg);
//				break;
//
			case 'w':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -w.\n");
					return 1;
				}
				segpara.medianFiltWid = atoi(optarg);
				break;

			case 'r':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -r.\n");
					return 1;
				}
				segpara.gaussinFiltWid = atoi(optarg);
				break;

			case 'g':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -g.\n");
					return 1;
				}
				segpara.gaussinFiltSigma = atof(optarg);
				break;

			case 'd':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				segpara.adaptThreWid = atoi(optarg);
				break;

			case 't':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -t.\n");
					return 1;
				}
				segpara.adpatThreMethod = atoi(optarg);
				break;

			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -s.\n");
					return 1;
				}
				segpara.adaptThreStp = atoi(optarg);
				break;

//			case 'v':
//				segpara.tag_verbose_print = 1;
//				break;
//
			case 'm':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -m.\n");
					return 1;
				}
				segpara.segMethod = atoi(optarg);
				break;

//			case 'a': // attenuation correction
//				segpara.tag_attenuation_corr = 1;
//				break;

        }
    }

	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);

	unsigned char * inimg1d = 0; /* note that this variable must be initialized as NULL. */
	V3DLONG * sz = 0; /* note that this variable must be initialized as NULL. */
	int datatype = 0;

	//////////////////////////////////////////////////////////////////////////////////
	////20080917 by RZC for win32(mingw): to get rid of error: crosses initialization
	Vol3DSimple <unsigned char> *img3d = 0;
	V3DLONG *data_sz = 0;
	Vol3DSimple <unsigned short int> *outimg3d = 0;
	bool b_res = false;
	//////////////////////////////////////////////////////////////////////////////////

	/* Read file and display some information. */

	if (loadImage(segpara.infilename, inimg1d, sz, datatype)!=true)
	{
		fprintf (stderr, "Error happens in reading the file [%s]. Exit. \n", segpara.infilename);
		goto Label_exit;
	}

	//Vol3DSimple <unsigned char> *img3d = new Vol3DSimple <unsigned char> (inimg1d, sz, segpara.channelNo); //input 3D image
	img3d = new Vol3DSimple <unsigned char> (inimg1d, sz, segpara.channelNo); //input 3D image

	printf("%d\n", segpara.channelNo);

	if (inimg1d) {delete []inimg1d; inimg1d=0;}

	//V3DLONG *data_sz = new V3DLONG [3];
	data_sz = new V3DLONG [3];
	V3DLONG len;
	data_sz[0]= img3d->sz0();
	data_sz[1]= img3d->sz1();
	data_sz[2]= img3d->sz2();
	len = data_sz[0] * data_sz[1] * data_sz[2];

	//Vol3DSimple <unsigned short int> *outimg3d = new Vol3DSimple <unsigned short int> (data_sz[0], data_sz[1], data_sz[2]); //output 3D image
	outimg3d = new Vol3DSimple <unsigned short int> (data_sz[0], data_sz[1], data_sz[2]); //output 3D image

	//now do computation

	b_res = FL_cellseg(img3d, outimg3d, segpara);

	/* making labels continous and reordering the label of the nuclei */

	/* save files */
	if (b_res)
	{
		V3DLONG sz[4];
		sz[0] = img3d->sz0(); sz[1]= img3d->sz1(); sz[2]= img3d->sz2(); sz[3] = 1;
		if (saveImage(segpara.outfilename,(const unsigned char *) outimg3d->getData1dHandle(), sz, sizeof(unsigned short int))!=true)
		{
			fprintf(stderr, "Error happens in file writing. Exit. \n");
		}
    }
	else
	{
		fprintf(stderr, "Fail to generate the cell segmentation results. \n");
	}

	/* clean all workspace variables */

Label_exit:

	if (sz) {delete [] sz; sz=0;}
	if (data_sz) {delete [] data_sz; data_sz =0;}
	if (outimg3d) {delete outimg3d; outimg3d =0;}
	if (img3d) {delete img3d; img3d=0;}

	return 0;

}
