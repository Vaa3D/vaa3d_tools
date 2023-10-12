/* Read the raw image file generated using Hanchuan Peng's program saveStack2Raw.m .
   
   by Hanchuan Peng
   Feb 15, 2006
   Update: 06-04-12: fix the bug of file not close
   Update: 06-04-14: add the support for endianness of machines
   Update: 060803: change to the 4-byte-size format by including the stackutil.cpp program.
   Updated: 060828: add multipage tif support. Note that when use mex compile, need to link the other two pre-compiled object files:
        mex -c mg_utilities.cpp
        mex -c mg_image_lib.cpp
        mex loadRaw2Stack_c.cpp mg_image_lib.o mg_utilities.o -v -ltiff
		
	Updated: 060920: add back 2-byte raw support	
	Updated: 070713: George added the lsm reading function
    070717/18: note that printf() may not work if for a mex version, use mexPrintf() instead. But I don't understand why in mexFunction it works!
	070819: add a LSM thumbnail reading 
*/

#include <stdio.h>
#include <strings.h>

//#include "stackutil.h"
#include "stackutil.cpp"

/* use the C_INTERFACE to control if compile the Matlab version interface to read the stacks. */

//#define C_INTERFACE 1 

#ifdef C_INTERFACE

/* main program for C interface */

void printHelp ();
void printHelp()
{
 printf("\nUsage: loadRaw2Stack_c -i <rawImgFile>\n");
  printf("\t -i <rawImgFile>    .raw/.tif/.lsm image stacks.\n");
  printf ("\t [-h] print this message.\n");
  return;
}

#include <unistd.h>
extern char *optarg;
extern int optind, opterr;

int main (int argc, char *argv[])
{
  if (argc <= 1)
  {
    printHelp ();
    return 0;
  }

  /* Read arguments */

  char *input_dfile = NULL;

  int c;
  static char optstring[] = "hi:";
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
          /* fprintf(stderr,"[%s]-> ",optarg); */
          if (strcmp (optarg, "(null)") == 0 || optarg[0] == '-')
            {
              fprintf (stderr,
                       "Found illegal or NULL parameter for the option -i.\n");
              return 1;
            }
          input_dfile = optarg;
          break;

        case '?':
          fprintf (stderr, "Unknown option `-%c' or incomplete argument lists.\n", optopt);
          return 0;

        /* default:        abort (); */
        }
    }

  if (optind < argc)
    printf ("Stop parsing arguments list. Left off at %s\n", argv[optind]);

  /* Read file and display some information. */

  unsigned char * img = 0; /* note that this variable must be initialized as NULL. */
  long * sz = 0; /* note that this variable must be initialized as NULL. */
  int datatype = 0;

  char * curFileSuffix = getSuffix(input_dfile);
  printf("The current input file has the suffix [%s]\n", curFileSuffix);
  if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0) //read tiff stacks
  {
      if (loadTif2Stack(input_dfile, img, sz, datatype))
	  {
		printf("Error happens in TIF file reading. Exit. \n");
		return 1;
	  }
  }
  else //then assume it is Hanchuan's RAW format
  {
	  if (loadRaw2Stack(input_dfile, img, sz, datatype))
	  {
		printf("Error happens in reading 4-byte-size RAW file. Trying 2-byte-raw. \n");
	    if (loadRaw2Stack_2byte(input_dfile, img, sz, datatype))
	    {
		   printf("Error happens in reading 2-byte-size RAW file. Exit. \n");
		   return 1;
		}   
	  }
  }


  switch (datatype)
  {
    case 1:
      printf("The data type is 8-bit integer.\n");
      printf("The size of this image stack is [%d %d %d %d]\n", sz[0], sz[1], sz[2], sz[3]);
      break;

   case 2:
      printf("The data type is 16-bit integer.\n");
      printf("The size of this image stack is [%d %d %d %d]\n", sz[0], sz[1], sz[2], sz[3]);
      break;

   case 4:
      printf("The data type is 32-bit float/long/int.\n");
      printf("The size of this image stack is [%d %d %d %d]\n", sz[0], sz[1], sz[2], sz[3]);
      break;

   default:
     printf("Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
     if (img) {delete [] img; img=0;}
     if (sz) {delete []sz; sz=0;}
     return 1;
  }

  /* clean all workspace variables */

  if (img) {delete [] img; img=0;}
  if (sz) {delete []sz; sz=0;}

  return 0;
}

#else

#include "elementmexheader.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs<1 || nrhs>4) mexErrMsgTxt("Usage: img = loadRaw2Stack_c(infilename, intensity_dfactor, b_readThumbNail (0/1), b_readMiddleFrameOnly (0/1)). Note: the last two paras are only effective for LSM thumbnails.");
  if (nlhs>1)  mexErrMsgTxt("Too many output arugments. Usage: img = loadRaw2Stack_c(infilename, intensity_dfactor, b_readThumbNail (0/1), b_readMiddleFrameOnly(0/1)). Note: the last two paras are only effective for LSM thumbnails.");
  if (!mxIsChar(prhs[0])) mexErrMsgTxt("The first input argument must be types of String/CHAR.");

  //default not to read thumbnail
  int Dfactor=0;
  if (nrhs==1) Dfactor=0;
  else {Dfactor = int(*(double *)mxGetPr(prhs[1]));}
	
  int b_readThumbNail=0;
  if (nrhs==2) b_readThumbNail=0;
  else {b_readThumbNail = int(*(double *)mxGetPr(prhs[2]));}

  int b_readMiddleFrameOnly=0;
  if (nrhs==3) b_readMiddleFrameOnly=0;
  else {b_readMiddleFrameOnly = int(*(double *)mxGetPr(prhs[3]));}

  /* Allocate enough memory to hold the converted string. */ 
  int buflen = mxGetNumberOfElements(prhs[0]) + 1;
  char * buf = new char [buflen];
  if (mxGetString(prhs[0], buf, buflen) != 0)
    mexErrMsgTxt("Could not convert string data.");

  /* File read */

  unsigned char * img = 0; /* note that this variable must be initialized as NULL. */
  long * sz = 0; /* note that this variable must be initialized as NULL. */
  int datatype = 0;

  char * curFileSuffix = getSuffix(buf);
  printf("The current input file has the suffix [%s]\n", curFileSuffix);
  if (strcasecmp(curFileSuffix, "tif")==0 || strcasecmp(curFileSuffix, "tiff")==0) //read tiff stacks
  {
      //if (loadTif2Stack(buf, img, sz, datatype, Dfactor))
	  if (loadTif2Stack(buf, img, sz, datatype)) //commented out Dfactor part, by PHC, 081009
	  {
		printf("Error happens in TIF file reading. Exit. \n");
		if (buf) {delete []buf; buf=0;}
		return;
	  }
  }
  else if ( strcasecmp(curFileSuffix, "lsm")==0 ) //read lsm stacks
  {
	  if (b_readThumbNail==0)
	  {
		if (b_readMiddleFrameOnly==0)
		{
			if (loadLsm2Stack(buf, img, sz, datatype))
			{
				printf("Error happens in LSM file reading. Stop. \n");
				if (buf) {delete []buf; buf=0;}
				return;
			}
		}
		else
		{
			if (loadLsm2Stack_middle(buf, img, sz, datatype))
			{
				printf("Error happens in LSM file reading. Stop. \n");
				if (buf) {delete []buf; buf=0;}
				return;
			}
		}
	  }
	  else
	  {
		if (b_readMiddleFrameOnly==0)
		{
			if (loadLsmThumbnail2Stack(buf, img, sz, datatype))
			{
				printf("Error happens in LSM file reading. Stop. \n");
				if (buf) {delete []buf; buf=0;}
				return;
			}
		}
		else
		{
			if (loadLsmThumbnail2Stack_middle(buf, img, sz, datatype))
			{
				printf("Error happens in LSM file reading. Stop. \n");
				if (buf) {delete []buf; buf=0;}
				return;
			}
		}
	  }
  }
  else //then assume it is Hanchuan's RAW format
  {
	  printf("The data is not with a TIF/LSM suffix, -- now this program assumes it is RAW format defined by Hanchuan Peng. \n");
	  if (loadRaw2Stack(buf, img, sz, datatype))
	  {
		printf("The data doesn't look like a correct 4-byte-size RAW file. Try 2-byte-raw. \n");
	    if (loadRaw2Stack_2byte(buf, img, sz, datatype))
	    {
		   printf("Error happens in reading 2-byte-size RAW file. Exit. \n");
  		   if (buf) {delete []buf; buf=0;}
		   return;
		}   
	  }
  }


  long i;

  int dims[4]; //there is a risk of error is the system "int" type is 2-byte and "long" is 4-byte. However here seems they are all 4-bytes
  for (i=0;i<4;i++) dims[i] = (int)sz[i]; 

  /* Allocate memory for output */
  long unitSize=0;

  switch (datatype)
  {
    case 1:
      printf("The data type is 8-bit integer.\n");
      printf("The size of this image stack is [%d %d %d %d]\n", sz[0], sz[1], sz[2], sz[3]);
      unitSize = 1;
      plhs[0] = mxCreateNumericArray(4, (const int *)dims, mxUINT8_CLASS, mxREAL);
      break;

   case 2:
      printf("The data type is 16-bit integer.\n");
      printf("The size of this image stack is [%d %d %d %d]\n", sz[0], sz[1], sz[2], sz[3]);
      unitSize = 2;
      plhs[0] = mxCreateNumericArray(4, (const int *)dims, mxUINT16_CLASS, mxREAL);      
      break;

   case 4:
      printf("The data type is 32-bit float.\n");
      printf("The size of this image stack is [%d %d %d %d]\n", sz[0], sz[1], sz[2], sz[3]);
      unitSize = 4;
      plhs[0] = mxCreateNumericArray(4, (const int *)dims, mxSINGLE_CLASS, mxREAL);      
      break;

   default:
      printf("Something wrong with the program, -- should NOT display this message at all. Check your program. \n");
      if (buf) {delete []buf; buf=0;}
      if (img) {delete [] img; img=0;}
      if (sz) {delete []sz; sz=0;}
      return;
  }

  /* Copy over the data */

  UBYTE * p_outimg = (UBYTE *)mxGetData(plhs[0]);
  long totalBytes = long(mxGetNumberOfElements(plhs[0])) * unitSize;
  for (long i=0;i<totalBytes;i++)
  {
    p_outimg[i] = img[i];
  }
  
  /* clean all workspace variables */

  if (buf) {delete []buf; buf=0;}
  if (img) {delete [] img; img=0;}
  if (sz) {delete []sz; sz=0;}

  return;
}

#endif

