//main cell segmentation program
//by Fuhui Long
//2008-03-07
//2008-08-23: separated as the main function by Hanchuan Peng

//run -i /Users/longf/work/celegans_improvement/seed_data/aap1SD1486L1_1023071_crop_straight.raw -o /test.raw -c 2 -f 1 -m 50 -d 20 -s 0.5

#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"

#include "FL_gvfCellSeg.h"

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

	printf(" 3D cell segmentation using diffused gradient vector field\n");
	printf(" Usage: prog_name -i <infilename> -o <outfilename> -c <channel> -f fusionThreshold -m minRegion -d diffusionIteration -s sigma\n");
	printf(" Default parameters: fusionThreshold 3, minRegion 50, diffusionIteration 15, sigma 1.0\n");

	return;
}


int main(int argc, char *argv[])
{
	if (argc <= 2)
	{
		printHelp ();
		return 0;
	}

	gvfsegPara segpara;

	// set default values
	segpara.channelNo = 2;
	segpara.fusionThreshold = 3;
	segpara.diffusionIteration = 15;
	segpara.minRegion = 50;
	segpara.sigma = 1.0;

	/* Read arguments */

	V3DLONG c;
	static char optstring[] = "h:i:o:c:f:m:d:s:";
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

			case 'f':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				segpara.fusionThreshold = atoi (optarg); // should be 0, 1, 2,...
				if (segpara.fusionThreshold < 0)
				{
					fprintf (stderr, "Illeagal channelNo found! It must be >=0.\n");
					return 1;
				}
				break;

			case 'm':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				segpara.minRegion = atoi (optarg); // should be 0, 1, 2,...
				if (segpara.minRegion < 0)
				{
					fprintf (stderr, "Illeagal channelNo found! It must be >=0.\n");
					return 1;
				}
				break;

			case 'd':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				segpara.diffusionIteration = atoi (optarg); // should be 0, 1, 2,...
				if (segpara.diffusionIteration < 0)
				{
					fprintf (stderr, "Illeagal channelNo found! It must be >=0.\n");
					return 1;
				}
				break;

			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -c.\n");
					return 1;
				}
				segpara.sigma = atof (optarg); 
				if (segpara.sigma < 0)
				{
					fprintf (stderr, "Illeagal channelNo found! It must be >=0.\n");
					return 1;
				}
				break;

        }
    }

	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);

	unsigned char * inimg1d = 0; /* note that this variable must be initialized as NULL. */
	V3DLONG * sz = 0; /* note that this variable must be initialized as NULL. */
	int datatype = 0;

	//////////////////////////////////////////////////////////////////////////////////
	////20080920 by RZC for win32(mingw): to get rid of error: crosses initialization
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
	if (inimg1d) {delete []inimg1d; inimg1d=0;}

	//V3DLONG *data_sz = new V3DLONG [3];
	data_sz = new V3DLONG [3];
	V3DLONG len;
	data_sz[0]= img3d->sz0();
	data_sz[1]= img3d->sz1();
	data_sz[2]= img3d->sz2();
	len = data_sz[0] * data_sz[1] * data_sz[2];
	printf("%d, %d, %d\n", data_sz[0], data_sz[1], data_sz[2]);

	//Vol3DSimple <unsigned short int> *outimg3d = new Vol3DSimple <unsigned short int> (data_sz[0], data_sz[1], data_sz[2]); //output 3D image
	outimg3d = new Vol3DSimple <unsigned short int> (data_sz[0], data_sz[1], data_sz[2]); //output 3D image

	//now do computation

	//bool b_res = gvfCellSeg(img3d, outimg3d, segpara);
	b_res = gvfCellSeg(img3d, outimg3d, segpara);

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
