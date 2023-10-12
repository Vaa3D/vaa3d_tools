/* Save the stack to either tif or raw (Hanchuan Peng's raw) file.
   
   by Hanchuan Peng
   Updated 060828:
        mex -c mg_utilities.c
        mex -c mg_image_lib.c
        mex loadRaw2Stack_c.cpp mg_image_lib.o mg_utilities.o -v -ltiff
	
   Updated: 070426: add uint16 and single writing support			
*/

#include <stdio.h>
#include <strings.h>

#include "stackutil.cpp"

/* use the C_INTERFACE to control if compile the Matlab version interface to read the stacks. */

//#define C_INTERFACE 1 

#ifdef C_INTERFACE

/* main program for C interface */

void printHelp ();
void printHelp()
{
 printf("\nUsage: saveStack2File_c -i <rawImgFile>\n");
  printf("\t -i <rawImgFile>    .raw file containing the stack generated using saveStack2Raw.m program.\n");
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

  char * curFileSurfix = getSuffix(input_dfile);
  printf("The current input file has the surfix [%s]\n", curFileSurfix);
  if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0) //read tiff stacks
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
		printf("Error happens in RAW file reading. Exit. \n");
		return 1;
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
  if (nrhs!=2) mexErrMsgTxt("Usage: saveStack2File_c(img, infilename)");
  if (!mxIsChar(prhs[1])) mexErrMsgTxt("The second input argument must be String/CHAR.");
  if (!mxIsUint8(prhs[0]) && !mxIsUint16(prhs[0]) && !mxIsSingle(prhs[0])) mexErrMsgTxt("The first input argument must be UINT8 / UINt16 or SINGLE.");

  long i,j,k,c;

  //handle the image data
  long sz[4];
  unsigned char * img = (unsigned char *)mxGetData(prhs[0]);
  int * imgsz = (int *)mxGetDimensions(prhs[0]);
  int imgdims = mxGetNumberOfDimensions(prhs[0]);

  if (imgdims>4)
  {
      printf("Number of dimensions = %d (bigger than 4, -- only use the first 4 dimensions).\n", imgdims);
  }
  else if (imgdims<4)
  {
      printf("Number of dimensions = %d (smaller than 4, -- the unappearing dimensions are set as 1).\n", imgdims);
  }
  
  for (i=0;i<4;i++) sz[i] = 1; //set default
  for (i=0;i<((imgdims<4)?imgdims:4); i++)
      sz[i] = imgsz[i];
  printf("Save dims: %d %d %d %d\n", sz[0], sz[1], sz[2], sz[3]);

  /* Allocate enough memory to hold the converted string. */ 
  
  int buflen = mxGetNumberOfElements(prhs[1]) + 1;
  char * buf = new char [buflen];
  if (mxGetString(prhs[1], buf, buflen) != 0)
    mexErrMsgTxt("Could not convert string data.");

  /* File write */ //update on 070426
  int datatype=1; //default is uint8
  if (mxIsUint8(prhs[0])) datatype=1;
  else if (mxIsUint16(prhs[0])) datatype=2;
  else datatype=4;
  
  char * curFileSurfix = getSuffix(buf);
  printf("The current input file has the surfix [%s]\n", curFileSurfix);
  if (strcasecmp(curFileSurfix, "tif")==0 || strcasecmp(curFileSurfix, "tiff")==0) //read tiff stacks
  {
      if (saveStack2Tif(buf, img, sz, datatype))
	  {
		printf("Error happens in TIF file reading. Exit. \n");
		if (buf) {delete []buf; buf=0;}
		return;
	  }
  }
  else //then assume it is Hanchuan's RAW format
  {
	  if (saveStack2Raw(buf, img, sz, datatype))
	  {
		printf("Error happens in RAW file reading. Exit. \n");
		if (buf) {delete []buf; buf=0;}
		return;
	  }
  }

  
  /* clean all workspace variables */

  if (buf) {delete []buf; buf=0;}
  //if (imgsz) {delete []imgsz; imgsz=0;}

  return;
}

#endif

