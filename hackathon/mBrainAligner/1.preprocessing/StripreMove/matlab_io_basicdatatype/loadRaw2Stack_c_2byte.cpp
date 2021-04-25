/* Read the raw image file generated using Hanchuan Peng's program saveStack2Raw.m .
   
   by Hanchuan Peng
   Feb 15, 2006
   Update: 06-04-12: fix the bug of file not close
   Update: 06-04-14: add the support for endianness of machines
   Update: 06-08-03: change name by adding a surfix "2byte" as I use the 4 byte size now. I don't want to lose this program in general as I still have some 2byte-size-formated stacks (including the old Wano format)
*/

#include <stdio.h>
#include <strings.h>

int loadRaw2Stack(char * filename, unsigned char * & img, long * & sz, int & datatype);
void swap2bytes(void *targetp);
void swap4bytes(void *targetp);
char checkMachineEndian();


/* use the C_INTERFACE to control if compile the Matlab version interface to read the stacks. */

//#define C_INTERFACE 1 

#ifdef C_INTERFACE

/* main program for C interface */

void printHelp ();
void printHelp()
{
 printf("\nUsage: loadRaw2Stack_c -i <rawImgFile>\n");
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

  if (loadRaw2Stack(input_dfile, img, sz, datatype))
  {
    printf("Error happens in file reading. Exit. \n");
    return 1;
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
      printf("The data type is 32-bit float.\n");
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

#include "../elementmexheader.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (nrhs!=1) mexErrMsgTxt("Usage: img = loadRaw2Stack_c(infilename)");
  if (nlhs>1)  mexErrMsgTxt("Too many output arugments. Usage: img = loadRaw2Stack_c(infilename)");
  if (!mxIsChar(prhs[0])) mexErrMsgTxt("The first input argument must be types of String/CHAR.");

  /* Allocate enough memory to hold the converted string. */ 
  int buflen = mxGetNumberOfElements(prhs[0]) + 1;
  char * buf = new char [buflen];
  if (mxGetString(prhs[0], buf, buflen) != 0)
    mexErrMsgTxt("Could not convert string data.");

  /* File read */

  unsigned char * img = 0; /* note that this variable must be initialized as NULL. */
  long * sz = 0; /* note that this variable must be initialized as NULL. */
  int datatype = 0;

  if (loadRaw2Stack(buf, img, sz, datatype))
  {
    printf("Error happens in file reading. Exit. \n");
    if (buf) {delete []buf; buf=0;}
    return;
  }

  long i;

  int dims[4];
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


/* The following is the core function for image stack reading */

int loadRaw2Stack(char * filename, unsigned char * & img, long * & sz, int & datatype)
{
  /* This function reads 2-4D image stack from raw data generated by the program "saveStack2Raw.m". */
  /* The input parameters img, sz, and datatype should be empty, especially the pointers "img" and "sz". */

  int berror = 0;

  FILE * fid = fopen(filename, "rb");
  if (fid<0) 
  {
    printf("Fail to open file for reading.\n");
    berror = 1;
    return berror;
  }

  fseek (fid, 0, SEEK_END);
  long fileSize = ftell(fid);
  rewind(fid);

  /* Read header */

  char formatkey[] = "raw_image_stack_by_hpeng";
  int lenkey = strlen(formatkey); 

  if (fileSize<lenkey+2+4*2+1) /* datatype has 2 bytes, and sz has 4*2 bytes and endian flag has 1 byte. */
  {
    printf("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
    berror = 1;
    return berror;
  }

  char * keyread = new char [lenkey+1];
  if (!keyread)
  {
    printf("Fail to allocate memory.\n");
    berror = 1;
    return berror;
  }
  long nread = fread(keyread, 1, lenkey, fid);
  if (nread!=lenkey)
  {
    printf("File unrecognized or corrupted file.\n");
    berror = 1;
    return berror;
  }
  keyread[lenkey] = '\0';

  long i;
  if (strcmp(formatkey, keyread)) /* is non-zero then the two strings are different */
  {
    printf("Unrecognized file format.\n");
    if (keyread) {delete []keyread; keyread=0;}
    berror = 1;
    return berror;
  }
  
  char endianCodeData='B';
  fread(&endianCodeData, 1, 1, fid);
  if (endianCodeData!='B' && endianCodeData!='L')
  {
    printf("This program only supports big- or little- endian but not other format. heck your data endian.\n");
    berror = 1;
    if (keyread) {delete []keyread; keyread=0;}
    return berror;
  }
      
  char endianCodeMachine = checkMachineEndian();
  if (endianCodeMachine!='B' && endianCodeMachine!='L')
  {
    printf("This program only supports big- or little- endian but not other format. heck your data endian.\n");
    berror = 1;
    if (keyread) {delete []keyread; keyread=0;}
    return berror;
  }

  int b_swap = (endianCodeMachine==endianCodeData)?0:1;
    

  short int dcode = 0;
  fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
  if (b_swap)
      swap2bytes((void *)&dcode);

  switch (dcode)
  {
    case 1:
      datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
      break;

   case 2:
      datatype = 2;
      break;

   case 4:
      datatype = 4;
      break;

   default:
      printf("Unrecognized data type code [%d]. The file type is incorrect or this code is not supported in this version.\n", dcode);
      if (keyread) {delete []keyread; keyread=0;}
      berror = 1;
      return berror;
  }

  long unitSize = datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */

  short int mysz[4];
  fread(mysz, 2, 4, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
  if (b_swap)
  {
      for (i=0;i<4;i++)
          swap2bytes((void *)(mysz+i));
  }

  if (sz) {delete []sz; sz=0;}
  sz = new long [4]; /* reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img. */
  if (!sz)  
  {
    printf("Fail to allocate memory.\n");
    if (keyread) {delete []keyread; keyread=0;}
    berror = 1;
    return berror;
  }

  long totalUnit = 1;
  for (i=0;i<4;i++)
  {
    sz[i] = (long)mysz[i];
    totalUnit *= sz[i];
  }

  if (totalUnit*unitSize+4*2+2+1+lenkey != fileSize)
  {
    printf("The input file has a size [%d bytes] to what specified in the header [%d bytes]. Exit.\n", fileSize, totalUnit*unitSize+4*2+2+lenkey);
    if (keyread) {delete []keyread; keyread=0;}
    if (sz) {delete []sz; sz=0;}
    berror = 1;
    return berror;
  }

  if (img) {delete []img; img=0;}
  img = new unsigned char [totalUnit*unitSize];
  if (!img) 
  {
    printf("Fail to allocate memory.\n");
    if (keyread) {delete []keyread; keyread=0;}
    if (sz) {delete []sz; sz=0;}
    berror = 1;
    return berror;
  }

  nread = fread(img, unitSize, totalUnit, fid);
  if (nread!=totalUnit)
  {
    printf("Something wrong in file reading. The program reads [%d data points] but the file says there should be [%d data points].\n", nread, totalUnit);
    if (keyread) {delete []keyread; keyread=0;}
    if (sz) {delete []sz; sz=0;}
    if (img) {delete []img; img=0;}
    berror = 1;
    return berror;
  }
  
  /* swap the data bytes if necessary */
  
  if (b_swap==1)
  {
      if (unitSize==2)
      {
          for (i=0;i<totalUnit; i++)
          {
              swap2bytes((void *)(img+i*unitSize));
          }
      }
      else if (unitSize==4)
      {
          for (i=0;i<totalUnit; i++)
          {
              swap4bytes((void *)(img+i*unitSize));
          }
      }
  }


  /* clean and return */

  if (keyread) {delete [] keyread; keyread = 0;}
  fclose(fid); //bug fix on 060412

  return berror;
}

/* check the machine endianness */

char checkMachineEndian()
{
    char e='N'; //for unknown endianness
    
    long int a=0x44332211;
    unsigned char * p = (unsigned char *)&a;
    if ((*p==0x11) && (*(p+1)==0x22) && (*(p+2)==0x33) && (*(p+3)==0x44))
        e = 'L';
    else if ((*p==0x44) && (*(p+1)==0x33) && (*(p+2)==0x22) && (*(p+3)==0x11))
        e = 'B';
    else if ((*p==0x22) && (*(p+1)==0x11) && (*(p+2)==0x44) && (*(p+3)==0x33))
        e = 'M';
    else
        e = 'N';

    //printf("[%c] \n", e);
    return e;
}
        

void swap2bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
}

void swap4bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+3);
    *(tp+3) = *tp;
    a = *(tp+1);
    *(tp+1) = *(tp+2);
    *(tp+2) = a;
}
