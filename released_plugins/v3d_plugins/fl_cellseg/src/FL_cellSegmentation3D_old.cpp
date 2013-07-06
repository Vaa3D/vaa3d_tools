//cell segmentation
//by Fuhui Long
//2008-03-07

#include "./local_basic_c_fun/volimg_proc.h"
#include "./local_basic_c_fun/img_definition.h"
#include "./local_basic_c_fun/stackutil.h"

#include "FL_cellSegmentation3D.h"
#include "FL_defType.h"

#include <string>
#include <unistd.h>
#include <math.h>
extern char *optarg;
extern int optind, opterr;


void printHelp()
{
   printf("\nUsage: nuclei_segmentation -i <infilename> -o <outfilename> -c <channel> -b <backgndLevel> -w <noiseRemovalFilterWid> -r <smoothFilterWid> -d <adaptiveThre>\n");
	printf("[h]			help\n");
	printf("[v]			verbose print\n");
	printf("[a]			attenuation correction\n");
	
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
	segpara.backgndLevel = 15;
	segpara.noiseRemFiltWid = 2;
	segpara.smoothFiltWid = 1;
	segpara.adaptThreWid = 10;
	segpara.adaptThreStp = 5;
	
	/* Read arguments */
	  
	V3DLONG c;
	static char optstring[] = "hva:i:o:c:b:w:r:d:s";
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
												
			case 'b':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -b.\n");
					return 1;
				}
				segpara.backgndLevel = atoi(optarg);
				break;
								
//			case 'X':
//				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
//				{
//					fprintf (stderr, "Found illegal or NULL parameter for the option -X.\n");
//					return 1;
//				}
//				segpara.xy_rez = atof (optarg);
//				if (segpara.xy_rez<=0)
//				{
//					fprintf (stderr, "xy rez of a image must be bigger than 0.\n");
//					return 1;
//				}
//				break;
//				
//			case 'Z':
//				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
//				{
//					fprintf (stderr, "Found illegal or NULL parameter for the option -Z.\n");
//					return 1;
//				}
//				segpara.z_rez = atof (optarg);
//				if (segpara.z_rez<=0)
//				{
//					fprintf (stderr, "z rez of a image must be bigger than 0.\n");
//					return 1;
//				}
//				break;

			case 'w':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -b.\n");
					return 1;
				}
				segpara.noiseRemFiltWid = atoi(optarg);		
				break;	

			case 'r':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -b.\n");
					return 1;
				}
				segpara.smoothFiltWid = atoi(optarg);			
				break;
				
			case 'd':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				segpara.adaptThreWid = atoi(optarg);			
				break;
				
			case 's':
				if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
				{
					fprintf (stderr, "Found illegal or NULL parameter for the option -d.\n");
					return 1;
				}
				segpara.adaptThreStp = atoi(optarg);			
				break;
				
			case 'v':
				segpara.tag_verbose_print = 1;
				break;
			
			case 'a': // attenuation correction
				segpara.tag_attenuation_corr = 1;
				break;
				
        }
    }
	
	if (optind < argc)
		printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);
		
	unsigned char * inimg4d = 0; /* note that this variable must be initialized as NULL. */
	V3DLONG * sz = 0; /* note that this variable must be initialized as NULL. */
	int datatype = 0;
	
	/* Read file and display some information. */	
	
	if (loadImage(segpara.infilename, inimg4d, sz, datatype)!=true)
	{
		fprintf (stderr, "Error happens in reading the file [%s]. Exit. \n", segpara.infilename);
		goto Label_exit;
	}
		
//	Vol3DSimple <unsigned char> *inimg3d = new Vol3DSimple <unsigned char> (inimg4d, sz, segpara.channelNo); //input 3D image
//	Vol3DSimple <unsigned char> *img3d = new Vol3DSimple <unsigned char> (inimg3d); // 3d image used for operation
//	Vol3DSimple <unsigned short int> *img3d_label = new Vol3DSimple <unsigned short int> (sz[0], sz[1], sz[2]); 

//	Vol3DSimple <float> *inimg3d = new Vol3DSimple <float> (inimg4d, sz, segpara.channelNo); //input 3D image	
//	Vol3DSimple <float> *img3d = new Vol3DSimple <float> (inimg3d); 

	Vol3DSimple <unsigned char> *inimg3d = new Vol3DSimple <unsigned char> (inimg4d, sz, segpara.channelNo); //input 3D image	
	Vol3DSimple <unsigned char> *img3d = new Vol3DSimple <unsigned char> (inimg3d); 

//	Vol3DSimple <unsigned char> *img3d_label = new Vol3DSimple <unsigned char> (sz[0], sz[1], sz[2]); 

    /* remove noise and smooth */
    {
	printf("Median filtering ...\n");
	medfilt3d(img3d, segpara.noiseRemFiltWid); //median filtering
	
	printf("Gaussian filtering ...\n");
	gaussfilt3d(img3d, segpara.smoothFiltWid, 2); //gaussian filtering
	}
	
//	/* attenuation correction if 'a' parameter is given */
//	attenCorr3d(imgch);
//	
	/* local adpative thresholding */
//  {
//		V3DLONG kernelsz[]= {10,10,10};
//		V3DLONG kernelstp[] = {5,5,5};
//		
//		kernelsz[0] = kernelsz[1] = kernelsz[2] = segpara.adaptThreWid;
//		kernelstp[0] = kernelstp[1] = kernelstp[2] = segpara.adaptThreStp;
//		
//		printf("Adaptive thresholding ...\n");
//		adaptiveThre3d(img3d, kernelsz, kernelstp);	
//	}

	V3DLONG *data_sz = new V3DLONG [3];
	
	data_sz[0]= img3d->sz0();
	data_sz[1]= img3d->sz1();
	data_sz[2]= img3d->sz2();
	
//	
//// test morphological operation
//{
//	Strel *se = new Strel;
//	V3DLONG radius = 2;
//	
//	printf("Binary dilation ...\n");
//			
//	se->createBallStrel(radius); 
//	se->printStrelInfo();
//	
//	bwDilate(img3d, se);
////	bwErode(img3d, se);
////	bwOpen(img3d, se);
////	bwClose(img3d, se);
//	 
////}


//	find connected components
//	{
//		
//		printf("Labeling ...\n");		 
//		findConnectedComponent(img3d->getData1dHandle(),data_sz, 3, 3, img3d_label->getData1dHandle());
//	}
	
	
// test distance transform
//{	
//	V3DLONG len = sz[0]*sz[1]*sz[2];
//	float *data1d = new float [len];
//	
//	V3DLONG i,j,k;
//	V3DLONG count = 0;
//	unsigned char *p = img3d->getData1dHandle();
//	V3DLONG *pix_index = new V3DLONG [len];
//	
//	unsigned short int *p = img3d_label->getData1dHandle();
//
//	V3DLONG tmp_k, tmp_j;
//	
//	
//	for (k=0;k<sz[2];k++)
//	{
//		tmp_k = k*sz[0]*sz[1];
//		
//		for (j=0;j<sz[1];j++)
//		{
//			tmp_j = j*sz[0];
//			
//			for (i=0;i<sz[0];i++)
//			{
//
//				data1d[count] = (float) *(p+tmp_k+tmp_j+i);		
//										
//				count++;
//			}
//		}
//	}
////	
//	printf("distance transform ...\n");
//	dt3d_binary(data1d, pix_index, data_sz, 1); // compute the distance transform for foreground (non-zero) pixels	
//	// not free pix_index yet
//	count = 0;
//	for (k=0;k<sz[2];k++)
//	{
//		tmp_k = k*sz[0]*sz[1];
//		
//		for (j=0;j<sz[1];j++)
//		{
//			tmp_j = j*sz[0];
//			
//			for (i=0;i<sz[0];i++)
//			{
//				if (*(p+count) == 0)
//					data1d[count] = 0;
//				else
//					data1d[count] = 50-sqrt(data1d[count]);
//
//				data1d[count] = sqrt(data1d[count]);
//				count++;
//			}
//		}
//	}
//	
//	/* watershed segmentation for low convexity regions */
//   {
//		printf("watershed segmentation....\n");
//		unsigned char *conn;
//		V3DLONG *conn_sz;
//		float *label_data = 0;
//		
//		watershed_vs(img3d->getData1dHandle(), data_sz, 3, 26, label_data);
//		watershed_vs(data1d, data_sz, 3, 26, label_data);
//
//	}
//

//	
//	/* remove non-nuclei regions */
//	
//	/* making labels continous and reordering the label of the nuclei */
	
	/* save files */
	{
		V3DLONG sz[4];
		sz[0] = img3d->sz0(); sz[1]= img3d->sz1(); sz[2]= img3d->sz2(); sz[3] = 1;
//		if (saveImage(segpara.outfilename, (const unsigned char *)img3d->getData1dHandle(), sz, sizeof(unsigned short int))!=true) 
		
		if (saveImage(segpara.outfilename,img3d->getData1dHandle(), sz, sizeof(unsigned char))!=true) 
//		if (saveImage(segpara.outfilename,(const unsigned char *) data1d, sz, sizeof(float))!=true) 

//		if (saveImage(segpara.outfilename,(const unsigned char *) label_data, sz, sizeof(float))!=true) 
//
////		if (saveImage(segpara.outfilename,(const unsigned char *) img3d->getData1dHandle(), sz, sizeof(float))!=true) 

		{
			fprintf(stderr, "Error happens in file writing. Exit. \n");
		}

    }
					
	/* clean all workspace variables */
	
Label_exit:
		
	if (sz) {delete [] sz; sz=0;}
		
	return 0;

}
