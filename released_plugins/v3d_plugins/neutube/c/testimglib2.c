#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fftw3.h>
#include <zlib.h>
#include <assert.h>
#include <utilities.h>

#include <water_shed.h>
#include <level_set.h>
#include "tz_image_lib.h"
#include "tz_math_lib.h"
#include "tz_fftwf.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_farray.h"
#include "tz_dmatrix.h"
#include "tz_fmatrix.h"
#include "tz_utilities.h"
#include "tz_matlabio.h"
#include "tz_array.h"
#include "tz_error.h"
#include "tz_fimage_lib.h"
#include "tz_dimage_lib.h"
#include "tz_string.h"
#include "tz_complex.h"
#include "tz_pipe.h"
#include "tz_arraytrans.h"
#include "tz_3dgeom.h"
#include "tz_neurotrace.h"
#include "tz_constant.h"
#include "tz_object_tmpl.h"
#include "tz_stack_bwmorph.h"
#include "tz_exception.h"
#include "tz_image_disp.h"

/**
 * Load stack data.
 * 
 */
int load_data(char *filepath,int *nstack,char *dirname,char *tileName,char *tileName2,int *mag,int *series,char *stackBase,char *stack[],int *flip)
{
  FILE *fp;
  char line[100];
  char *token1,*token2;
  int i;
  if((fp=fopen(filepath,"r"))!=NULL) {
    while(!feof(fp)) {
      if(fgets(line,100,fp)==NULL)
	continue;
      token1 = strtok(line,"=");
      if(token1==NULL)
	continue;
      strtrim(token1);
      token2 = strtok(NULL,"=");
      strtrim(token2);
      if(!strcmp(token1,"nstack")) {
	*nstack = atoi(token2);
	continue;
      }
      if(!strcmp(token1,"dirname")) {
	strcpy(dirname,token2);
	continue;
      }
      if(!strcmp(token1,"tilename")) {
	strcpy(tileName,token2);
	continue;
      }
      if(!strcmp(token1,"tilename2")) {
	strcpy(tileName2,token2);
	continue;
      }
      if(!strcmp(token1,"magnification")) {
	*mag = atoi(token2);
	continue;
      }
      if(!strcmp(token1,"series")) {
	token1 = strtok(token2,"{,");
	series[0] = atoi(token1);
	for(i=1;i<*nstack;i++) {
	  token1 = strtok(NULL,"{,}");
	  series[i] = atoi(token1);
	}
	continue;
      }
      if(!strcmp(token1,"stack")) {
	token1 = strtok(token2,"{,");
	stack[0] = strcpy(stackBase,token1);
	int offset = strlen(token1)+1;
	for(i=1;i<*nstack;i++) {
	  token1 = strtok(NULL,"{,}");
	  stack[i] = strcpy(stackBase+offset,token1);
	  offset += strlen(token1)+1;
	}
	continue;
      }
      if(!strcmp(token1,"flip")) {
	token1 = strtok(token2,"{,");
	flip[0] = atoi(token1);
	for(i=1;i<*nstack;i++) {
	  token1 = strtok(NULL,"{,}");
	  flip[i] = atoi(token1);
	}
	continue;
      }
    }
  } else {
    return 0;
  }

  fclose(fp);
  return 1;
}

Image *loadTestImage()
{
  Image *image = Read_Image("../data/696_019_50x_1_Series005_z090_ch00.tif");
  return image;
}

Stack *loadTestStack()
{
  File_Bundle fb;
  fb.prefix = "../data/neuromorph/disk1/696_019/696_019_50x_1/696_019_50x_1_Series005_z";
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;

  Stack *stack = Read_Stack_Planes(&fb);
  
  return stack;
}

Stack *loadTestStack2()
{
  File_Bundle fb;
  fb.prefix = "../data/testimg/out";
  fb.suffix = "";
  fb.num_width = 3;
  fb.first_num = 1;

  Stack *stack = Read_Stack_Planes(&fb);
  
  return stack;
}

Stack *loadBinaryStack()
{
  File_Bundle fb;
  fb.prefix = "/tmp/dimimg/fb";
  fb.suffix = "t";
  fb.num_width = 3;
  fb.first_num = 1;
  
  Stack *stack = Read_Stack_Planes(&fb);

  return stack;
}

INIT_EXCEPTION_MAIN(e)

int main(int argc, char* argv[]) {
  if(argc==1) {
    printf("Usage: testimglib <file path>\n");
    return 1;
  }

#ifdef _DEBUG_
  printf("Checking ...\n");
  printf("int size: %ld; short size: %ld; long size: %ld; pointer size: %ld\n",
	 sizeof(int),sizeof(short),sizeof(long),sizeof(int*));
  if(sizeof(int)==4 && sizeof(long)==4 && sizeof(void*)==4 && sizeof(long long)==8)
    printf("Possible IPL32LL model.\n");
#endif

  assert(sizeof(int)==4);

  if(sizeof(int)<4) {
    printf("This program assumes the int type to have at least 4 types.");
    return 1;
  }  

  /* test object definition macro */
#if 0
  Object_T *object_t = Make_Object_T(10);

  printf("%d\n", object_t->size);
  printf("%d\n", Object_T_Usage());

  Kill_Object_T(object_t);
  object_t = Make_Object_T(20);
 
  int i;
  for (i = 0; i < 20; i++) {
    object_t->array[i] = i;
  }

  iarray_print2(object_t->array, 20, 1);

  Object_T *obj2 = Copy_Object_T(object_t);

  iarray_print2(obj2->array, 20, 1);
  iarray_print2(object_t->array, 20, 1);

  printf("%d\n", object_t->size);
  printf("%d\n", Object_T_Usage());
#endif

  /********** test resampling **********/
#if 0
  Stack *stack = loadTestStack2();
  Print_Stack_Info(stack);
  
  Stack *stack2 = Resample_Stack_Depth(stack, NULL, 1.1988);
  Print_Stack_Info(stack2);

  Write_Stack("../data/resimg.tif", stack2);
#endif

  /*
  initpipein();  
  Stack *s = (Stack*) pipein();
  
  Kill_Stack(s);

  initpipeout("matlab","tz_readtifstack;/Users/zhaot/c/neurolabi/data/substack1.tif");
  pipeout();
  */

  /****** test Process_Arguments ***********/
  /*  char *spec[] = { "[-m <int>] [!xyz] [-c'(<int>')]",
		   " <arg1:int> <arg2:int>",
		   NULL };
  Process_Arguments(argc,argv,spec,0);
  Print_Argument_Usage(stdout,0);
  */

  /****** test Load_Fbdf ********/
  
  /*
  File_Bundle fb;
  initfb(&fb);
  printf("%d\n",Load_Fbdf("../data/fbtest.fbd",&fb));
  printf("%s %s %d %d\n",fb.prefix,fb.suffix,fb.num_width,fb.first_num);
  freefb(&fb);
  */

  /******** test  array transformation **********/
  /*
  Image *image = loadTestImage();
  Print_Image_Info(image);
  printf("%p\n", image->array);

  Array_Link *a = Image_To_Stack(image);
  Print_Stack_Info(Get_Stack_At(a));
  printf("%p\n", ((Stack *) a->data)->array);

  Free_Array_Link(a);
  a = NULL;

  a = Image_To_Gm(image);
  tic();
  printf("%d\n", gsl_matrix_uchar_max(Get_Gmuc_At(a)));
  printf("%ld\n", toc());
  
  Free_Array_Link(a);
  a = NULL;

  int max_value;
  tic();
  IMAGE_MAX(image, max_value);
  printf("%d\n", max_value);
  printf("%ld\n", toc());
  */

  /*
  int *p = malloc(sizeof(int) * 10);
  printf("%p\n",p);
  safe_free((void **)&p,free);
  printf("%p\n",p);
  */

  //free(NULL);
  //TZ_WARN(20);


  /*
  FILE *fp = fopen("../data/fbtest.fbd","r");
  size_t len;
  char *line;
  while ( !feof(fp) ) {
    line = fgetln(fp, &len);
    if (len > 0) {
      printf("%lu\n", len);
      line[len-1] = '\0';
      printf("%s\n",line);
    } else {
      printf("\n");
    }
  }
   
  fclose(fp);
  */
  /*
  int nstack;
  char dirname[100];
  char tileName[20];
  char tileName2[20];
  int mag;
  int series[100];
  char stackbase[500];
  char *stack[100];
  int flip[100];
  int i;

  for(i=0;i<100;i++)
    flip[i] = 0;

  load_data("data/neurodata_696_068.txt",&nstack,dirname,tileName,tileName2,&mag,series,stackbase,stack,flip);

  printf("nstack = %d\n",nstack);
  printf("dirname = %s\n",dirname);
  printf("tilename = %s\n",tileName);
  printf("tilename2 = %s\n",tileName2);
  printf("magnification = %d\n",mag);
  printf("series = ");
  iarray_print2(series,nstack,1);
  printf("stack = ");
  for(i=0;i<nstack;i++)
    printf("%s ",stack[i]);
  printf("\n");
  printf("flip = ");
  iarray_print2(flip,nstack,1);
  */

  /*
  char line[100];
  char *token;
  FILE *fp;
  if((fp=fopen("data/neurodata_696_069.txt","r"))!=NULL)
    fgets(line,100,fp);
  
  token = strtok(line,"=");
  printf("%s\n",line);
  token = strtok(NULL,"=");
  int nstack = atoi(token);
  printf("%d\n",nstack);

  char str[100];
  strcpy(str,"Hello world!");
  strtrim(str);
  printf("%s%s\n",str,str);
  
  fclose(fp);
  */

  //test data type
  /*
  if( (sizeof(double)!=gDataSize[DOUBLE_T]) || 
      (sizeof(float)!=gDataSize[FLOAT_T]) ||
      (sizeof(int)!=gDataSize[INT_T]) )
    printf("Uncompatible type\n");
  
  tz_int16 d[] = {1,1,2,4,21};
  array_print(d,5,INT16_T);

  tz_int16 *d2;
  d2 = malloc(5*sizeof(tz_int16));
  
  int err;
  err = array_write(d,5*sizeof(tz_int16),"data/datatest.arr");
  if(err)
    printf("Error in array_write: %s \n",tz_errmsg(err));

  int length;
  err = array_read(d2,&length,"data/datatest.arr");
  if(err)
    printf("Error in array_write: %s \n",tz_errmsg(err));
  array_print(d2,length/sizeof(tz_int16),INT16_T);
  
  err = array_writet(d,INT16_T,5,"data/datatest.arr");
  if(err)
    printf("Error in array_write: %s \n",tz_errmsg(err));

  Data_Type_t type;
  err = array_readt(d2,&type,&length,"data/datatest.arr");
  array_print(d2,length,type);

  free(d2);
  */
  /*
  int n = 10;
  float * restrict a1;
  float * restrict a2;
  float * t = malloc(2 * n * sizeof(float));
  a1 = t; // a1 refers to 1st half
  a2 = t + n; // a2 refers to 2nd half
  */

  /*
  float* t = malloc(SIZE_MAX);
  printf("%u\n",SIZE_MAX);
  printf("%d\n",t);
  free(t);
  */
  /*
  int i=100;
  printf("%ld\n",(long)i);
  */
  
  /*
  double d1[] = {2.0,3.0,1.0,2.3,-9.0,0.0};
  double d2[] = {3.0,1.0,3.2,1.2,-0.1,0.0};
  int idx;
  double max_value = darray_max(d1,6,&idx);

  printf("%d : %f\n",idx,max_value);

  darray_print2(darray_sqrt(d1,6),6,1);
  
  uLongf destLen = sizeof(double)*10;
  Bytef* dest = (Bytef *)malloc(destLen);

  compress(dest,&destLen,(Bytef *)d1,sizeof(double)*6);
  printf("%lu\n",destLen);
  
  uLongf uncompLen = 100;
  double* d3 = (double *)malloc(uncompLen);
  uncompress((Bytef *)d3,&uncompLen,(Bytef *)dest,destLen);
  printf("%lu\n",uncompLen);

  darray_print2(d3,6,1);
  */
  /*
  Matlab_Array mr1,mr3;
  mr_read("data/mattest1.mat",&mr1);
  tz_uint32 paddim = (mr1.dim[0]/2+1)*2;
  double *in = fftw_malloc(mr1.dim[1]*paddim*sizeof(double));
  darraycpy2(in,mr1.array,paddim,mr1.dim[1],mr1.dim[0],mr1.dim[1],0,0);
  fftw_plan p = fftw_plan_dft_r2c_2d(mr1.dim[1],mr1.dim[0],in,in,FFTW_ESTIMATE);
  fftw_execute(p);
  fftw_destroy_plan(p);
  darray_print2(in,paddim,mr1.dim[1]);

  p = fftw_plan_dft_c2r_2d(mr1.dim[1],mr1.dim[0],in,in,FFTW_ESTIMATE);
  fftw_execute(p);
  fftw_destroy_plan(p);
  darray_divc(in,mr1.dim[0]*mr1.dim[1],paddim*mr1.dim[1]);
  fftw_pack_c2r_result(in,mr1.dim[0],mr1.dim[1]);
  darray_print2(in,mr1.dim[0],mr1.dim[1]);

  fftw_free(in);
  */

  /*
  Matlab_Array mr1,mr2;
  mr_read("../tmp/mattest1.mat",&mr1);
  print_mrinfo(&mr1);
  //iarray_print2((int*)mr1.array,mr1.dim[1],mr1.dim[0]);
  
  mr_read("../tmp/mattest2.mat",&mr2);
  print_mrinfo(&mr2);
  //iarray_print2((int*)mr2.array,mr2.dim[1],mr2.dim[0]);

  int length1 = mr1.dim[1]*mr1.dim[0];
  int length2 = mr2.dim[1]*mr2.dim[0];
  int length = length1+length2-1;

  double *conv = fftw_malloc(sizeof(fftw_complex)*R2C_LENGTH(length));

  darray_convolve(mr1.array,length1,mr2.array,length2,1,conv);
  
  darray_print2(conv,length,1);

  fftw_free(conv);
  */

  /*
  printf("%f\n",darray_corrcoef(mr1.array,mr2.array,mr1.dim[0]*mr1.dim[1]));
  
  tic();
  fftw_complex *out = darray_fft(mr1.array,mr1.dim[0]*mr1.dim[1],0,1);
  printf("%ld\n",toc());

  fftw_print(out,mr1.dim[0]*(mr1.dim[1]/2+1));
 
  double *out2 = darray_ifft(out,mr1.dim[0]*mr1.dim[1],1,0,1);
  
  darray_print2(mr1.array,mr1.dim[1],mr1.dim[0]);
  darray_print2(out2,mr1.dim[1],mr1.dim[0]);
  */

  /*
  Stack stack1,stack2;
  stack1.width = 10;
  stack1.height = 10;
  stack1.depth = mr1.dim[1]/100;
  stack1.kind = GREY16;
  stack1.array = mr1.array;

  stack2.width = 10;
  stack2.height = 10;
  stack2.depth = mr1.dim[1]/100;
  stack2.kind = GREY16;
  stack2.array = mr2.array;

  Print_Stack_Value(&stack1);
  Flip_Stack(&stack1);
  Print_Stack_Value(&stack1);
  
  Stack *downstack1,*downstack2;

  Print_Stack_Value(&stack1);
  downstack1 = Downsample_Stack_Mean(&stack1,3,3,3);
  Print_Stack_Value(downstack1);
  */

  /*
  //printf("%f\n",Stack_Corrcoef(&stack1,&stack2));
  //Print_Stack_Value(&stack1);
  Stack_Subc(&stack1,55);
  //Print_Stack_Value(&stack1);
  Stack_Binarize(&stack1);
  Print_Stack_Value(&stack1);

  //Print_Stack_Value(&stack2);
  Stack_Subc(&stack2,55);
  //Print_Stack_Value(&stack2);
  Stack_Binarize(&stack2);
  Print_Stack_Value(&stack2);

  IMatrix *im = Stack_Foreoverlap(&stack1,&stack2);
  printf("here\n");
  IMatrix_Print(im);
	
  im = Stack_Foreunion(&stack1,&stack2);
  IMatrix_Print(im);
  */

  /*
  dim_type bdim[3];
  Reflect_Stack(&stack1,1);
  IMatrix *istack1 = Get_Int_Matrix3(&stack1);
  bdim[0] = stack2.width; bdim[1] = stack2.height; bdim[2] = stack2.depth;
  IMatrix *ib1 = IMatrix_Blocksum(istack1,bdim);

  IMatrix *istack2 = Get_Int_Matrix3(&stack2);
  bdim[0] = stack1.width; bdim[1] = stack1.height; bdim[2] = stack1.depth;
  IMatrix *ib2 = IMatrix_Blocksum(istack2,bdim);
  iarray_add(ib1->array,ib2->array,matrix_size(ib1->dim,ib1->ndim));
  iarray_sub(ib1->array,im->array,matrix_size(ib1->dim,ib1->ndim));
  IMatrix_Print(ib1);
  */

  /*
  //darray_div((double*)mr1.array,(double*)mr2.array,mr2.dim[1]);
  darray_cumsum((double*)mr1.array,mr2.dim[1]);
  darray_print2((double*)mr1.array,mr1.dim[1],mr1.dim[0]);

  free(mr2.array);
  mr_read("data/mattest3.mat",&mr2);
  print_mrinfo(&mr2);
  darray_print2((double*)mr2.array,mr2.dim[1],mr2.dim[0]);

  int i;
  double *result1,*result2;
  result1 = (double*)mr1.array;
  result2 = (double*)mr2.array;
  for(i=0;i<mr2.dim[1];i++)
    if(result1[i]!=result2[i])
      printf("Possible bug found\n");
  
  free(mr1.array);
  free(mr2.array);
  */

  /*
  FILE *fp  = fopen("/Users/zhaot/matlab/shared/data/mattest.mat","r");

  char fileHeader[128];
  char descrp[DESCRP_LENGTH+1];
  tz_uint64 subsys_offset;
  tz_uint32 dataType,dataSize,dataOffset,totalOffset;
  tz_uint16 version;
  tz_uint16 endian;
  void* data;
  fread(fileHeader,128,sizeof(char),fp);
  printf("%d\n",parse_header(fileHeader,descrp,&subsys_offset,&version,&endian));
  descrp[DESCRP_LENGTH] = '\0';
  printf("%s\n",descrp);
  printf("version: %u, endian: %u\n",version,endian);
  
  char tag[8];
  fread(tag,8,1,fp);
  parse_tag(tag,&dataType,&dataSize);

  data = malloc(dataSize);
  fread(data,dataSize,1,fp);
  void* uncomData = data;
  tz_uint32 uncomDatasize = dataSize;

  if(dataType==miCOMPRESSED) {
    printf("This is compressed data\n");
    uncomData = uncompress_data(data,dataSize,&uncomDatasize);
    printf("uncomDataSize: %u\n",uncomDatasize);
    FILE* fp2 = fopen("/Users/zhaot/matlab/shared/data/mattest_uncom.mat","w+");
    fwrite(fileHeader,1,128,fp2);
    fwrite(uncomData,1,uncomDatasize,fp2);
    fclose(fp2);
    free(data);
  }

  Matlab_Array mr;
  parse_data(uncomData,&mr);
  print_mrinfo(&mr);
  darray_print2((double*)mr.array,mr.dim[1],mr.dim[0]);
  free(mr.array);
  */

  /*
  parse_tag(uncomData,&dataType,&dataSize);
  if(dataType == miMATRIX)
    printf("This is a matlab array. Size: %u\n",dataSize);

  tz_uint8 flags,data_class;
  totalOffset = 8;
  parse_arrayflags(uncomData+totalOffset,&flags,&data_class,&dataOffset);
  totalOffset += dataOffset;

  tz_uint32 ndim,dim[5];
  parse_dim(uncomData+totalOffset,&ndim,dim,&dataOffset);
  totalOffset += dataOffset;

  printf("%u : %u : %u\n",flags,data_class,dataOffset);
  printf("%u : %u : %u\n",ndim,dim[0],dim[1]);

  char varname[256];
  parse_varname(uncomData+totalOffset,varname,&dataOffset);
  printf("%s\n",varname);
  */

  /*
  fread(&dataType,1,sizeof(int),fp);
  fread(&dataSize,1,sizeof(int),fp);
  printf("%d : %d\n",dataType,dataSize);
  data = (Bytef*)malloc(dataSize);
  fread(data,dataSize,1,fp);

  Bytef* uncomData = NULL;

  if(dataType==miCOMPRESSED) {
    printf("This is compressed data\n");
    
    destLen = dataSize*2+12;

    uncomData = (Bytef*)malloc(destLen);   
    uncompress(uncomData,&destLen,data,dataSize);
  } else
    uncomData = data;
    
  parse_tag((void*)uncomData,&dataType,&dataSize);
  if(dataType == miMATRIX)
    printf("This is a matlab array\n");

  dataSize = *( ((int*)uncomData)+1 );
  printf("data size: %d\n",dataSize);

  if(parse_arrayflags( ((void*)uncomData)+8,&dataType,&dataSize,&dataOffset))
    printf("%d : %d : %d\n",dataType,dataSize,dataOffset);
  */
  /*
  //dataType = *( ((char*)uncomData)+16 );
  dataType = *( ((int*)uncomData)+4 );
  printf("%d\n",dataType);

  int dim[2];
  dim[0] = *( ((int*)uncomData)+8 );
  dim[1] = *( ((int*)uncomData)+9 );

  if((dataType&0x000000FF)==mxDOUBLE_CLASS)
    printf("This is a double array: %d x %d\n",dim[0],dim[1]);

  int name_len;
  char* var_name = NULL;

  if( *( ((short*)uncomData)+20 )!=0) {
    name_len = *( ((short*)uncomData)+21 );
    var_name = (char *)malloc(name_len+1);
    memcpy(var_name,uncomData+44,name_len);
  }
  else {
    name_len = *( ((int*)uncomData)+11 );
    var_name = (char *)malloc(name_len+1);
    memcpy(var_name,uncomData+48,name_len);
  }
  var_name[name_len] = '\0';

  printf("%d\n",name_len);

  printf("%s\n",var_name);
  
  */
  //process uncompressed data

  //fclose(fp);

  /*
  tic();
  int i,t=0;
  for(i=0;i<10000000;i++)
    t=t+1;
  printf("%ld uSec\n",toc());
  */

  /*  
  int dim1[3] = {15,15,10};
  int dim2[3] = {10,10,10};
  
  Stack* stack1 = Make_Stack(1,dim1[0],dim1[1],dim1[2]);
  Stack* stack2 = Make_Stack(1,dim2[0],dim2[1],dim2[2]);
  int i,j,k,offset;
  int start[3] = {10,1,10};
  int end[3] = {14,4,14};
  int dim[3],corr_dim[3];
  for(i=0;i<3;i++) {
    dim[i] = end[i]-start[i]+1;
    corr_dim[i] = dim2[i]+dim1[i]-1;
  }
  
  offset = 0;
  for(k=0;k<dim1[2];k++)
    for(j=0;j<dim1[1];j++)
      for(i=0;i<dim1[0];i++) {
	stack1->array[offset] = offset;
	offset++;
      }
	
  offset = 0;
  for(k=0;k<dim2[2];k++)
    for(j=0;j<dim2[1];j++)
      for(i=0;i<dim2[0];i++) {
	stack2->array[offset] = offset*offset+offset;
	offset++;
      }	
  
  DMatrix* corr = Normcorr_Stack_Part(stack1,stack2,1,start,end);
  //DMatrix* dm = darray2dmatrix(corr,3,dim[0],dim[1],dim[2]);
  DMatrix_Print(corr);

  DMatrix* corr2 = Normcorr_Stack(stack1,stack2,1);
  //DMatrix* dm = darray2dmatrix(corr,3,corr_dim[0],corr_dim[1],corr_dim[2]);
  DMatrix_Print_Part(corr2,start,end);
  //DMatrix_Print(corr);
  */
  /* 
  DMatrix* corr = Correlate_Stack_Part(stack1,stack2,start,end);
  DMatrix* dm = darray2dmatrix(corr,3,dim[0],dim[1],dim[2]);
  DMatrix_Print(dm);
  
   */ 
  /*   
  Stack* ref_stack1 = Reflect_Stack(stack1,0);
  Stack* padded_stack1 = Crop_Stack(ref_stack1,-dim2[0],-dim2[1],-dim2[2],
				    corr_dim[0]+1,corr_dim[1]+1,corr_dim[2]+1);
  Stack* padded_stack2 = Crop_Stack(stack2,-dim1[0],-dim1[1],-dim1[2],
				    corr_dim[0]+1,corr_dim[1]+1,corr_dim[2]+1);
  double* corr = Correlate_Stack(padded_stack1,padded_stack2,0);
  DMatrix* dm = darray2dmatrix(corr,3,corr_dim[0]+1,corr_dim[1]+1,corr_dim[2]+1);
  DMatrix_Print_Part(dm,start,end);

  Kill_Stack(stack1);
  Kill_Stack(stack2);
  Kill_DMatrix(dm);
  */  

  /*
  dim_type dim[]={10,10,20};
  dim_type bdim[] = {4,4,10};
  
  dim_type offset[] = {3,1,15};
  dim_type op1[6],op2[6];
  
  DMatrix_Overlap(dim,bdim,offset,3,op1,op2);

  int i;
  for(i=0;i<6;i++)
    printf("%d ",op2[i]);
  printf("\n");
  */

  /*  
  dim_type dim[]={10,10,20};
  dim_type bdim[] = {4,4,10};
  dim_type start[] = {9,9,10};
  dim_type end[] = {12,12,15};

  int *array = Get_Area_Part(bdim,dim,start,end,3);
  array_print(array,(end[2]-start[2]+1)*(end[1]-start[1]+1)*(end[0]-start[0]+1),INT_TD);

  DMatrix* dm = DMatrix_Ones(dim,3);
  tic();
  DMatrix* dm2 = DMatrix_Blocksum_Part(dm,bdim,start,end);
  printf("%ld\n",toc());
  DMatrix_Print(dm2);
  */

  /*
  //  printf("%d\n",sub2ind(dim,3,start));
  DMatrix* dm = ones(dim,3);
  int i,j,k,offset = 0;
  for(k=0;k<dim[2];k++)
    for(j=0;j<dim[1];j++)
      for(i=0;i<dim[0];i++) {
	dm->array[offset] = offset*offset;
	offset++;
      }

  //DMatrix* dm3 = DMatrix_Blocksum_Part(dm,bdim,start,end);
  DMatrix* dm2 = DMatrix_Blocksum_Part(dm,bdim,start,end);
  DMatrix_Print(dm2);

  //DMatrix* dm2 = DMatrix_Blocksum(dm,bdim);
  //DMatrix_Print_Part(dm2,start,end);

  Kill_DMatrix(dm);
  Kill_DMatrix(dm2);
  */
  
  /*  time_t t1 = time(0);
  double t = 1.0/0.0;
  darray_write("data/test.bin",&t,1);
  time_t t2 = time(0);
  printf("%f",difftime(t1,t2));
  */

  /*
  tic();

  int dim[] ={100,100,100};
  DMatrix* dm = ones(dim,3);
 
  printf("Total time was %ld uSec.\n", toc());

  Kill_DMatrix(dm); 
  */

  /*
  double d[60],d2[12];
  int i;
  for(i=0;i<60;i++)
    d[i] = i;

  for(i=0;i<12;i++)
    d2[i] = -i-1;

  darraycpy2(d,d2,12,5,2,3,4,1);
  
  darray_print2(d,12,5);
  */

  /*
  double d[] = {7.0,4.3,2.5,9.0,3.0,15.7,2.0,14.0};
  double* d2 = darray_copy(d,8);

  int  idx[] = {0,1,2,3,4,5,6,7};
  darray_qsort(d,idx,8);
  darray_print2(d,8,1);
  int i;
  for(i=0;i<8;i++)
    printf("%.3f ",d2[idx[i]]);
  printf("\n");

  free(d2);
  */

  /*
  double d2[16];
  int start,end;
  start = 3;
  end = 5;
  d2[end-start+1] = 0;
  darray_linsum1_part(d,d2,8,9,start,end);
  //darray_linsum1(d,d2,8,1,9,1);
  darray_print2(d,8,1);
  darray_print2(d2,16,1);
  */

  /*
  darray_linsum2(d2,12,1,1);
  darray_print2(d2,12,1);

  DMatrix* dm = darray2dmatrix(d,2,4,2);
  dim_type bdim[] = {3,3};
  DMatrix* dm2 = DMatrix_Blocksum(dm,bdim);
  DMatrix_Print(dm);
  DMatrix_Print(dm2);

  Kill_DMatrix(dm2);
  free(dm);
  */
  //int idx;
  //printf("%.2f\n",darray_max(d,8,&idx));
  //printf("%d\n",idx);
  
  //darray_write("data/datatest.bin",d,8);
  

  /*
  int ndim = 5;
  int dim[] = {20,30,40,30,32};
  int sub[] = {10,2,1,2,3};
  
  ind2sub(dim,ndim,20301,sub);

  printf("%d\n",sub2ind(dim,ndim,sub));
  */

  /*
  int i;
  int *array = malloc(10*sizeof(int));
  for(i=0;i<10;i++)
    array[i] = i;

  printf("%d\n",iarray_centroid(array,10));

  
  Image* image1 = Read_Image("data/test1.tif");
  int *hist = Image_Hist(image1);
  iarray_print2(hist,hist[0]+2,1);
  printf("%d\n",Hist_Rcthre(hist,0,254));
  */
  /*
  Write_Image("data/out.tif",image1);
  Kill_Image(image1);
  */

  /*
  Image *image = Read_Image("../data/696_019_50x_1_Series005_z090_ch00.tif");
  Watershed *ws = Build_2D_Watershed(image, 1);
  Write_Image("/tmp/watershed1.tif",ws->labels);
  initpipeout("matlab","imread;/tmp/watershed1.tif");
  pipeout();
  */

  /*
  tic();
  Image* image2 = Resize_Image(image1,150,150);
  printf("%d uSec\n",toc());
  Write_Image("data/out.tif",image1);
  Write_Image("data/out.tif",image2);
  Kill_Image(image1);
  Kill_Image(image2);
  */

  /*Image* image2 = Read_Image("data/test2.tif");

  
  //Write_Image("data/out1.tif",image1);
  //Write_Image("data/out2.tif",image2);

  double* corrimg = Normcorr_Image(image1,image2);

  //darray_print2(corrimg,image1->width+image2->width-1,image1->height+image2->height-1);

  Image* image3 = Scale_Double_Image(corrimg,image1->width+image2->width-1,image1->height+image2->height-1,image2->kind);
  
  Write_Image("data/out.tif",image3);
 
  Kill_Image(image3);
  Kill_Image(image1);
  Kill_Image(image2);
  
  free(corrimg);
  */

  /*
  int i;
  Image* image2 = Make_Image(1,10,10);
  for(i=0;i<100;i++)
    image2->array[i] = i*i-1;

  Image* image1 = Crop_Image(image2,2,2,3,3);

  double* corrimg = Normcorr_Image(image1,image2);
  free(corrimg);

  Kill_Image(image1);
  Kill_Image(image2);
  */

  //printf("%d\n",fexist("data/corr.bin"));
  /*
  Stack* stack;
  
  stack = Make_Stack(GREY16,4,4,3);
  int length = stack->width*stack->height*stack->depth;
  int i;
  uint16* array16 = (uint16*) (stack->array);
  for(i=0;i<length;i++)
    array16[i] = (uint16)i*i+i/2;

  Stack*  stack2 = Crop_Stack(stack,1,1,1,3,3,2);

  Print_Stack_Value(stack);
  Print_Stack_Value(stack2);

  double* corr_stack = Normcorr_Stack(stack2,stack,1);

  int idx;
  int dim[3];
  dim[0] = stack->width+stack2->width-1;
  dim[1] = stack->height+stack2->height-1;
  dim[2] = stack->depth+stack2->depth-1;

  int length2 = (stack->width+stack2->width-1) *
    (stack->height+stack2->height-1) *
    (stack->depth+stack2->depth-1);

  darray_clean_edge3(corr_stack,dim[0],dim[1],dim[2],1);
  double max_value = darray_max(corr_stack,length2,&idx);

  printf("%f\n",max_value);

  printf("%d\n",idx);
  int sub[3];
  ind2sub(dim,3,idx,sub);

  for(i=0;i<3;i++)
    printf("%d ",sub[i]);
  printf("\n");

  darray_print3(corr_stack,stack->width+stack2->width-1,
  		stack->height+stack2->height-1,
   		stack->depth+stack2->depth-1);
  
  //fftw_free(out);
  free(corr_stack);
  //  free(darray);
  Kill_Stack(stack);
  Kill_Stack(stack2);
  */
  /*
  File_Bundle fb;
  fb.prefix = argv[1];
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;
  */

  /*
  int dim[] = {5,4,4,3};
  DMatrix* dm = Make_DMatrix(dim,4);
  int length = matrix_size(dim,4);
  int i;
  for(i=0;i<length;i++)
    dm->array[i] = i;
  
  int offset[] = {0,0,1,0};
  dim_type new_dim[] = {10,2,2,3};
  DMatrix_Print(dm);
  DMatrix_Print(Crop_DMatrix(dm,offset,new_dim));
  */

  /*
  int sub[3];
  DMatrix_Max(dm,sub);

  for(i=0;i<3;i++)
    printf("%d ",sub[i]);
  printf("\n");
  */

  /*
  int bdim[] = {4,3,5};

  DMatrix* dm2 = DMatrix_Blockmean(dm,bdim,0);
  DMatrix_Print(dm2);

  Kill_DMatrix(dm2);
  //stack = Read_Stack_Planes(&fb);
  
  //DMatrix* dm = Get_Double_Matrix3(stack);
  
  //DMatrix* dm2 = Copy_DMatrix(dm);
  
  Kill_DMatrix(dm);
  //Kill_DMatrix(dm2);
  //Kill_Stack(stack);
  */

  /*  
  tic();
  
  Stack *stack1,*stack2;
  char **prefixes;
  int nstack;
  char dirName[100];
  char tileName[20];
  char tileName2[20];
  int mag;
  int series[100];
  char stackbase[500];
  char *stackIdx[100];
  int flip[100];
  int i,j;

  for(i=0;i<100;i++)
    flip[i] = 0;

  load_data("../data/neuromorph/neurodata_696_019.txt",&nstack,dirName,tileName,tileName2,&mag,series,stackbase,stackIdx,flip);
  
  
  int overwrite = 0;

  int **offset;

  //select stacks
  //nstack = 16;
  int selstack[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
  //int selstack[] = {14,15};
  int jobid = 0;
  ////////////

  int npair = nstack*(nstack+1);
  

  prefixes = malloc(sizeof(char*) * nstack);
  for(i=0;i<nstack;i++)
    prefixes[i] = malloc(sizeof(char)*100);

  for(i=0;i<nstack;i++) {
    sprintf(prefixes[i],"%s/%s/%s_%dx_%s/%s_%dx_%s_Series%03d_z",dirName,tileName,tileName2,mag,stackIdx[selstack[i]],tileName2,mag,stackIdx[selstack[i]],series[selstack[i]]);
  }

  float *max_corr;
  max_corr = (float *) malloc(sizeof(float)*npair);
  //int stack_idx[] = {0,1,2,3,4};
  
  offset = malloc(sizeof(int *)*npair);
  for(i=0;i<npair;i++)
    offset[i] = (int *) malloc(3*sizeof(int));

  int intv[] = {3,3,3};
  int **pairs;
  pairs = malloc(sizeof(int *)*npair);
  for(i=0;i<npair;i++)
    pairs[i] = (int *) malloc(2*sizeof(int));

  int **stackSizes;
  stackSizes = malloc(sizeof(int *)*nstack);
  for(i=0;i<npair;i++)
    stackSizes[i] = (int *) malloc(3*sizeof(int));

  int idx = 0;
  
  File_Bundle fb;
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;

  char stackFile[100];
  Stack **downstacks;
  downstacks = (Stack**)malloc(sizeof(Stack*)*nstack);
  
  printf("Load stacks ...\n");
  for(i=0;i<nstack;i++) {
    printf("Stack %d ...\n",i);
    fb.prefix = prefixes[i];
    stack1 = Read_Stack_Planes(&fb);
    
    if(flip[i])
	Flip_Stack(stack1);
    
    stackSizes[i][0] = stack1->width;
    stackSizes[i][1] = stack1->height;
    stackSizes[i][2] = stack1->depth;
    if(jobid>0)
      sprintf(stackFile,"../data/%s_stack%d_%d.tif",tileName,i+1,jobid);
    else
      sprintf(stackFile,"../data/%s_stack%d.tif",tileName,i+1);

    if(!fexist(stackFile) || overwrite)
      Write_Stack(stackFile,stack1);
    
    printf("Downsample stack ...\n");
    downstacks[i] = Downsample_Stack_Mean(stack1,intv[0],intv[1],intv[2]);
    Pixel_Range* pr = Stack_Range(downstacks[i],0);
    Stack_Sub_Common(downstacks[i],0,(int)((pr->minval+pr->maxval)/2));

    Free_Stack(stack1);
  }
  Reset_Stack();

  for(i=0;i<nstack-1;i++) {
    for(j=i+1;j<nstack;j++) {
      max_corr[idx] = Align_Stack_MR_F(downstacks[i],downstacks[j],intv,-1,offset[idx],max_corr+idx);
      max_corr[idx] *= -1;

      pairs[idx][0] = i;
      pairs[idx][1] = j;
      printf("(%d,%d) : (%d,%d,%d) : %f\n",pairs[idx][0],pairs[idx][1],
	     offset[idx][0],offset[idx][1],offset[idx][2],max_corr[idx]);
      
      idx++;
    }
    Kill_Stack(downstacks[i]);
  }
  Kill_Stack(downstacks[nstack-1]);
  free(downstacks);
  
  int *permidx;
  permidx = (int *) malloc(sizeof(int)*npair);
  for(i=0;i<npair;i++)
    permidx[i] = i;

  int* labels;
  labels = (int *) malloc(sizeof(int)*nstack);
  for(i=0;i<nstack;i++)
    labels[i] = 0;

  int **selpairs;
  selpairs = malloc(sizeof(int*) * (nstack-1));
  for(i=0;i<nstack-1;i++)
    selpairs[i] = (int*) malloc(sizeof(int)*2);

  int **seloffset;
  seloffset = malloc(sizeof(int*) * (nstack-1));
  for(i=0;i<nstack-1;i++)
    seloffset[i] = (int*) malloc(sizeof(int)*3);

  farray_qsort(max_corr,permidx,npair);
  
  idx = 0;
  labels[pairs[permidx[0]][0]] = 1;
  //npair  = 1;
   i = 0;
  while(idx<nstack-1) {
    //one and only one idx has not been added
    if(labels[pairs[permidx[i]][0]] != labels[pairs[permidx[i]][1]]) {
      printf("(%d,%d)\n",pairs[permidx[i]][0],pairs[permidx[i]][1]);

      selpairs[idx][0] = pairs[permidx[i]][0];
      selpairs[idx][1] = pairs[permidx[i]][1];
      seloffset[idx][0] = offset[permidx[i]][0];
      seloffset[idx][1] = offset[permidx[i]][1];
      seloffset[idx][2] = offset[permidx[i]][2];

      //fine tuning
      fb.prefix = prefixes[selpairs[idx][0]];
      stack1 = Read_Stack_Planes(&fb);
      if(flip[selpairs[idx][0]])
	Flip_Stack(stack1);
      fb.prefix = prefixes[selpairs[idx][1]];
      stack2 = Read_Stack_Planes(&fb);
      if(flip[selpairs[idx][1]])
	Flip_Stack(stack2);
      Align_Stack_MR_F(stack1,stack2,intv,2,seloffset[idx],NULL);
      
      Free_Stack(stack1);
      Free_Stack(stack2);

      printf("(%d,%d,%d) : %f\n",
	     seloffset[idx][0],seloffset[idx][1],seloffset[idx][2],max_corr[i]);
      labels[pairs[permidx[i]][0]] = 1;
      labels[pairs[permidx[i]][1]] = 1;
      idx++;
      i = 0;
    }
    i++;
  }
  
  Reset_Stack();

  char resultFile[100];
  sprintf(resultFile,"../data/%s_result.bn",tileName);
  FILE *fp = fopen(resultFile,"w+");

  for(i=0;i<nstack;i++)
    fwrite(stackSizes[i],sizeof(int),3,fp);

  for(i=0;i<nstack-1;i++) {
    fwrite(selpairs[i],sizeof(int),2,fp);
    fwrite(seloffset[i],sizeof(int),3,fp);
  }

  fclose(fp);
  
  printf("%ld mSec\n",toc());
  */

  /*
  int idx;
  darray_max(max_corr,5,&idx);
  fb.prefix = prefixes[stack_idx[idx]];
  stack2 = Read_Stack_Planes(&fb);
  Align_Stack_MR(stack1,stack2,intv,2,offset[idx]);

  Write_Stack("data/orgstack1.tif",stack1);
  Write_Stack("data/orgstack2.tif",stack2);

  FILE *fp;
  if( !(fp=fopen("data/offset.bin","w+")) )
    exit(1);
  fwrite(offset[idx],sizeof(int),3,fp);
  fclose(fp);

  Kill_Stack(stack1);
  Kill_Stack(stack2);
  */
  /*
  File_Bundle fb;
  fb.prefix = "../data/neuromorph/disk1/696_019/696_019_50x_1/696_019_50x_1_Series005_z";
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;

  //Stack *stack = Read_Stack_Planes(&fb);
  */
  /*
  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  int i;
  for (i=0; i<27; i++)
    stack->array[i] = 1;
  stack->array[13] = 3;

  Print_Stack_Value(stack);
  //Stack *stack2 = Stack_Locmax(stack, NULL);
  //Print_Stack_Value(stack2);
  
  Component_Tree *tree = Build_3D_Component_Tree(stack, 1);
  
  Print_Component_Tree(tree, 2, stdout);
  */
  
  /*
  Matlab_Array mr;
  mr_read("/Users/zhaot/matlab/zhaot-lm1/data/test.mat", &mr);
  print_mrinfo(&mr);
  Array_Link *a = Mar_To_Stack(&mr);
  Print_Stack_Value(Get_Stack_At(a));
  Set_Chkcon(0);

  uint16 edge[10000];
  Stack *stack = Stack_Branch_Code(Get_Stack_At(a), NULL, NULL, edge, 30);
  Print_Stack_Value(stack);

  int i;
  for (i = 0; i < 20; i++) {
    printf("%u ", edge[i]);
  }
  printf("\n");

  // Tree_Code *tree = Image_Tree_Code(Get_Image_At(a), NULL, 30);
  //Print_Image_Value(Get_Tree_Branch(tree, NULL, Get_Image_At(a)->width, Get_Image_At(a)->height));
  */  

  /*
  Matlab_Array mr;
  mr_read("/Users/zhaot/matlab/zhaot-lm1/data/test.mat", &mr);
  print_mrinfo(&mr);
  Array_Link *a = Mar_To_Stack(&mr);
  Print_Stack_Value(Get_Stack_At(a));
  Tree_Code *tree = Stack_Tree_Code(Get_Stack_At(a), NULL, 30);

  Stack *image = Get_Tree_Branch_3D(tree, NULL, 
				   Get_Stack_At(a)->width, 
				   Get_Stack_At(a)->height,
				   Get_Stack_At(a)->depth);
  Print_Stack_Value(image);
  */  
  //Write_Image("/Users/zhaot/matlab/zhaot-lm1/data/test.tif", image);
  
  

  //Stack *stack = loadBinaryStack();

#if 0
  File_Bundle fb;
  fb.prefix = "/tmp/dimimg/fb";
  fb.suffix = "t";
  fb.num_width = 3;
  fb.first_num = 1;
  /*
  Stack *mask = Read_Stack_Planes(&fb);
  Stack_Threshold(mask, 3);
  */
  fb.prefix = "../data/bwimg1/fb";
  fb.suffix = "t";
  Stack *stack = Read_Stack_Planes(&fb);

  /*  for (i = 0; i < 100; i++)
    printf("%d: %u\n", i, stack->array[256 * 256 * 100 + 256 * 128 + 100 + i]);
  */
  
  Set_Chkcon(0);
  Image *image = Make_Image(GREY16, 1, 10000);
  uint16 edge[10000];
  int seed = 66 * 256 * 256 + (256 - 97) * 256 + 158;
  Stack *stack2 = Stack_Branch_Code(stack, NULL, NULL, edge, seed);
  //Stack *stack2 = Stack_Level_Code(stack, NULL, NULL, seed);

  //fb.prefix = "/tmp/codeimg/fb";
  fb.prefix = "../data/branchimg/fb";
  fb.suffix = "t";
  Write_Stack_Planes(&fb, stack2);

  int edge_end = 0;
  for (edge_end = 2; edge_end < 10000; edge_end++)
    if (edge[edge_end] == 0)
      break;
  edge_end--;

  image = Make_Image(GREY16, 1, edge_end);
  image->array = (uint8 *) (edge+1);
  Write_Image("../data/tree.tif", image);
#endif
  

  //Print_Image_Value(image);
  

  //Image *image = loadTestImage();
  

  /*
  Component_Tree *tree = Build_2D_Component_Tree(Get_Image_At(a), 1);  
  Print_Component_Tree(Level_Set_Child(tree), 2, stdout);
  printf("%p\n", Get_Current_Component_Tree());
  printf("%p\n", tree);
  */

  /*
  initpipein();
  Stack *stack = (Stack *) pipein();
  Stack *stack2 = Stack_Locmax(stack);
  File_Bundle fb;
  fb.prefix = "/tmp/seedimg/fb";
  fb.suffix = "t";
  fb.num_width = 3;
  fb.first_num = 0;
  Write_Stack_Planes(&fb, stack2);
  char interface[PIPE_INTF_SIZE];
  sprintf(interface,"tz_pipeinfbd;%s;%s;%d;%d",fb.prefix,fb.suffix,fb.num_width,fb.first_num);
  initpipeout("matlab",interface);
  pipeout();
  */

  //RegionGrow(stack, 128, 128, 128);
  /*  
  int i;

  Stack *orgstack1,*orgstack2;

  File_Bundle fb;
  fb.prefix = argv[1];
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;
						
  //Stack* stack1;
  orgstack1 = Read_Stack_Planes(&fb);
  Write_Stack("data/orgstack1.tif",orgstack1);

  //stack1 = Crop_Stack(stack,128,0,213,256,200,100); //for x1
  //stack1 = Crop_Stack(stack,128,300,150,256,200,100);

  //dim_type dim1[] = {100,100,50};
  //stack1 = Resize_Stack(stack,dim1[0],dim1[1],dim1[2]);

  //stack1 = Downsample_Stack(orgstack1,3,3,3);
  //Write_Stack("data/stack1.tif",stack1);
  
  //Stack* stack2;
  //fb.prefix = "data/696_019/696_019_50x_2/696_019_50x_2_Series002_z";
  fb.prefix = argv[2];
  orgstack2 = Read_Stack_Planes(&fb);  
  Write_Stack("data/orgstack2.tif",orgstack2);
  //stack2 = Crop_Stack(stack,128,300,167,256,200,100); //for x2
  //stack2 = Crop_Stack(stack,128,0,0,256,200,96);
  //dim_type dim2[] = {100,100,43};
  //stack2 = Resize_Stack(stack,dim2[0],dim2[1],dim2[2]);

  //stack2 = Downsample_Stack(orgstack2,3,3,3);
  //Write_Stack("data/stack2.tif",stack2);
  //int sub[3];
  int offset[3];
  */
  /*
  tic();
  DMatrix* corr_stack = Normcorr_Stack(stack2,stack1,1);
  printf("%ld uSec\n",toc());

  double max_value = DMatrix_Max(corr_stack,sub);

  printf("%f\n",max_value);

  for(i=0;i<3;i++)
    printf("%d ",sub[i]);
  printf("\n");

  //free(corr_stack);

  Free_DMatrix(corr_stack);
  */

  /*
  double max_value = Align_Stack(stack1,stack2,sub);

  printf("%f\n",max_value);
  for(i=0;i<3;i++)
    printf("%d ",sub[i]);
  printf("\n");
  Kill_Stack(stack1);
  Kill_Stack(stack2);
  int stack1_dim[3],stack2_dim[3];
  for(i=0;i<3;i++) {
    offset[i] = sub[i]*4-3;
  }

  stack1_dim[0] = orgstack1->width;
  stack1_dim[1] = orgstack1->height;
  stack1_dim[2] = orgstack1->depth;

  stack2_dim[0] = orgstack2->width;
  stack2_dim[1] = orgstack2->height;
  stack2_dim[2] = orgstack2->depth;
  
  int start1[3],start2[3],end1[3],end2[3];
  Stack_Overlap(stack1_dim,stack2_dim,offset,start1,end1,start2,end2);
  
  stack2 = Crop_Stack(orgstack2,start1[0],start1[1],start1[2],
		      end1[0]-start1[0]+1,end1[1]-start1[1]+1,end1[2]-start1[2]+1);
  stack1 = Crop_Stack(orgstack1,start2[0],start2[1],start2[2],
		      end2[0]-start2[0]+1,end2[1]-start2[1]+1,end2[2]-start2[2]+1);
  
  Write_Stack("data/substack1.tif",stack1);
  Write_Stack("data/substack2.tif",stack2);

  start1[0] = stack1->width-1;
  start1[1] = stack1->height-1;
  start1[2] = stack1->depth-1;
  for(i=0;i<3;i++)
    end1[i] = start1[i]+6;

  DMatrix* corr_stack = Normcorr_Stack_Part(stack2,stack1,1,start1,end1);
  //corr_stack = Normcorr_Stack(stack2,stack1,1);

  //dim[0] = stack2->width+stack1->width-1;
  //dim[1] = stack2->height+stack1->height-1;
  //dim[2] = stack2->depth+stack1->depth-1;

  max_value = DMatrix_Max(corr_stack,sub);

  printf("%f\n",max_value);

  for(i=0;i<3;i++) {
    offset[i] += sub[i];
    printf("%d ",offset[i]);
  }
  printf("\n");
 
  */
  /*
  int intv[] = {3,3,3};
  Align_Stack_MR(orgstack1,orgstack2,intv,offset);

  FILE *fp;
  if( !(fp=fopen("data/offset.bin","w+")) )
    exit(1);
  fwrite(offset,sizeof(int),3,fp);
  fclose(fp);
  */
  /*
  DMatrix* dm = ones(dim1,3);
  DMatrix* dm2 = DMatrix_Blocksum(dm,dim2);
  DMatrix_Write("data/blocksum.dm",dm2);
  Kill_DMatrix(dm);
  Kill_DMatrix(dm2);
  */

  
  /*
  File_Bundle fb;
  fb.prefix = argv[1];
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;

  Stack *stack;
  stack = Read_Stack_Planes(&fb);
  int* hist = Stack_Hist(stack);
  iarray_print2(hist,hist[0]+2,1);

  printf("%d\n",Hist_Most_Common(hist,100,65535));

  free(hist);
  */

  /*
  Stack* stack2 = Crop_Stack(stack,0,0,0,512,200,313);
  Image* image1 = Proj_Stack_Zmax(stack2);

  Kill_Stack(stack2);
  if(stack)
    Kill_Stack(stack);

  fb.prefix = "data/696_019/696_019_50x_2/696_019_50x_2_Series002_z";
  stack = Read_Stack_Planes(&fb);
  stack2 = Crop_Stack(stack,0,300,0,512,200,267);
  Image* image2 = Proj_Stack_Zmax(stack2);

  Kill_Stack(stack2);
  if(stack)
    Kill_Stack(stack);


  Write_Image("data/out1.tif",image1);
  Write_Image("data/out2.tif",image2);

  double* corrimg = Normcorr_Image(image1,image2);

  Image* image3 = Scale_Double_Image(corrimg,image1->width+image2->width-1,image1->height+image2->height-1,image2->kind);
  
  Write_Image("data/out.tif",image3);
 
  Kill_Image(image3);
  Kill_Image(image1);
  Kill_Image(image2);
  
  free(corrimg);
  */
  

  /*
  fftw_complex* out = Image_FFT(image2);
  double* out2 = Image_IFFT(out,image2->width,image2->height);
  Image* image3 = Scale_Double_Image(out2,image2->width,image2->height,image2->kind);

  Write_Image("data/out.tif",image3);

  if(image3)
    Kill_Image(image3);

  if(image)
    Kill_Image(image);

  if(stack)
    Free_Stack(stack);

  if(image2)
    Kill_Image(image2);

  if(out)
    fftw_free(out);

  if(out2)
    free(out2);
  */  

  //test fftw
  /*
  double *in, *out2;
  fftw_complex *out;
  fftw_plan p;
  int N = 64;

  in = (double *) fftw_malloc(sizeof(double) * N);
  out2 = (double *) fftw_malloc(sizeof(double) * N);

  //out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
  out = (fftw_complex*) fftw_malloc_r2c_2d(8,8);
  

  for(i=0;i<N;i++) {
    in[i] = i;
  }

  p = fftw_plan_dft_r2c_2d(8,8,in,out,FFTW_ESTIMATE);
  //p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
 
  fftw_execute(p);

  fftw_destroy_plan(p);

  p = fftw_plan_dft_c2r_2d(8,8,out,out2,FFTW_ESTIMATE);
  fftw_execute(p);
  for(i=0;i<64;i++)
    printf("%f ",out2[i]/64);
  printf("\n");
  fftw_destroy_plan(p);

  fftw_free(in); fftw_free(out);
  */
  
  /*
  Stack* stack = Read_Stack_Planes_Post(argv[1],3,0,"_ch00");
  if(stack) {
    printf("%d x %d x %d\n",stack->width,stack->height,stack->depth);
    Free_Stack(stack);
  }
  */

  /*
  Image* image = Read_Raw(argv[1],GREY16,1024,1024,0);
  Print_Image_Info(image);
  printf("%d\n",Get_Image_Pixel(image,100,100,0));

 
  Image* image2 = Crop_Image(image,-100,100,500,500);
  printf("%d\n",Get_Image_Pixel(image2,0,0,0));
  Print_Image_Info(image2);
   
  Write_Image("data/out.tif",image2);

  if(image)
    Kill_Image(image);

  if(image2)
    Kill_Image(image2);
  */

  /*
  int maxgrey = 0;
  int mingrey = 0;
  long iloop;
  long length = image->width*image->height;

  //IMAGE_MIN(image,mingrey);
  //printf("%d\n",mingrey);
  //IMAGE_MAX(image,maxgrey);
  //printf("%d\n",maxgrey);

  double * image_array =Get_Double_Array(image);

  if(image_array) {
    //fftw_complex *out = (fftw_complex *)fftw_malloc(length*sizeof(fftw_complex));
    fftw_complex* out = fftw_malloc_r2c_2d(image->width,image->height);
    fftw_plan p = fftw_plan_dft_r2c_2d(image->width,image->height,image_array,out,FFTW_ESTIMATE);
    fftw_execute(p);
    fftw_destroy_plan(p);
    
    //fftw_complete_2d(out,ima ge->width,image->height);

    
    //double* out2 = (double *)fftw_malloc(length*sizeof(double));
    //fftw_plan p2 = fftw_plan_dft_c2r_2d(image->width,image->height,out,out2,FFTW_ESTIMATE);
    //fftw_execute(p2);
    //fftw_destroy_plan(p2);
    
    complex_comp* outreal = fftw_complex_comp(out,length,0);
    for(i=0;i<100;i++)
      printf("%f\n",outreal[i]);

    //printf("%f\n",maxdouble(outreal,length));
    //printf("%f\n",mindouble(outreal,length));

    //Image* image2 = Scale_Double_Image(outreal,image->width,image->height,image->kind);
    //Write_Image("data/out.tif",image2);

    fftw_free(outreal);
    fftw_free(out);
    //fftw_free(out2);
    free(image_array);
    //if(image2)
    //  Kill_Image(image2);
  }  
  

  if(image)
    Kill_Image(image);
  */

  /*
  TIFF *tif = Open_Tiff(argv[1],"r");
  if(tif) {
    uint32 w, h;
    size_t npixels;
    int lastone;

    Image* image = Read_Tiff(tif,&lastone);
    printf("%d\n",lastone);
    printf("The size of the image is %d x %d \n",image->width,image->height);
    printf("The number of channels of the image is %d \n",image->kind);

    int x;
    for(x=0;x<image->width;x++)
      printf("%d ",Get_Image_Pixel(image,x,0,0));
    printf("\n");

    Kill_Image(image);
    Close_Tiff(tif);
  */						
    /*
    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    npixels = w*h;
    
    uint32* raster;
    raster = (uint32*)_TIFFmalloc(npixels*sizeof(uint32));
    TIFFReadRGBAImage(tif,w,h,raster,0);
    printf("%d\n",raster[0]);
    _TIFFfree(raster);
    //printf("The type of the image is %d",determine_kind(tif));
    printf("The size of the image is %d x %d (%d pixels)\n",w,h,npixels);
    TIFFClose(tif);
    */
  //}


  /*
  fftw_complex *in, *out;
  fftw_plan p;
  int N  = 128;

  in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) *N);
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) *N);
  p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_execute(p);
  fftw_destroy_plan(p);
  fftw_free(in); fftw_free(out);
  */

  /*** test 3dgeom ******/
#if 0
  Matlab_Array mr1;
  mr_read("../data/mattest1.mat",&mr1);
  print_mrinfo(&mr1);

  double *pts = (double*) mr1.array;
  
  darray_print2(pts, 3, 1);
 
  Rotate_XZ(pts, pts, 1, TZ_PI/5, 0.2, 0);

  darray_print2(pts, 3, 1);

  pts[0] = 10;
  pts[1] = -10;
  pts[2] = 0;

  Rotate_XZ(pts, pts, 1, TZ_PI_2, TZ_PI_2, 1);

  darray_print2(pts, 3, 1);
#endif

#if 0
  Set_Zscale(0.8342);
  //Set_Zscale(1.0);

  double *filter;
  Neuroseg seg;
  seg.r1 = 4.86913;
  seg.r2 = 4.85138;
  seg.h = 5;
  seg.theta = 1.58084;
  seg.psi = 0;

  Field_Range range;
  field_range(&seg, &range);

  Print_Field_Range(&range);

  /*
  Try {
    set_neuroseg(&seg, -1, 1, 1, 1, 1);
  } Catch(e) {
    TZ_ERROR(e);
  }
  */
#endif

#if 0
  filter = Neurofilter(&seg, NULL, &range);

  Stack *stack = loadTestStack2();
  
  int position[3] = {112, 93, 100};
  //double score = neuroseg_score(stack, &seg, position);
  //printf("score: %g\n", score);
  
  stack = Translate_Stack(stack, COLOR, 1);
  neuroseg_label(stack, &seg, position);

  seg.theta = TZ_PI_2;
  position[0] = 200;
  neuroseg_label(stack, &seg, position);
#endif
  
#if 0
  DMatrix in;
  in.ndim = 3;
  translate_intdim(range.size, in.dim, in.ndim);
  in.array = filter;

  DMatrix *out = Filter_Stack_Fast_D(stack, &in, NULL, 0);
  Kill_Stack(stack);
  stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1],
				   out->dim[2], GREY);

  Write_Stack("../data/test.tif", stack);

  /*
  darray_print3(filter, 11, 11, 3);
  printf("%f\n", darray_abssum(filter, 363));
  */
#endif

#if 0
  Stack* stack = Scale_Double_Stack(filter, range.size[0], range.size[1], 
				    range.size[2], GREY);
  Write_Stack("../data/neurofilter.tif", stack);
#endif

  /******** test stack filtering ************/
#if 0  
  Matlab_Array mr1;
  mr_read("../data/mattest1.mat",&mr1);
  print_mrinfo(&mr1);

  Array_Link *a = Mar_To_DMatrix(&mr1);
  DMatrix_Print(Get_DMatrix_At(a));

  Stack *stack = loadTestStack2();
  //DMatrix *out = Filter_Stack_Fast_D(stack, Get_DMatrix_At(a), NULL, 0);
  DMatrix *out = Filter_Stack_D(stack, Get_DMatrix_At(a), NULL);
  Kill_Stack(stack);

  stack = Scale_Double_Stack(out->array, out->dim[0], out->dim[1],
				   out->dim[2], GREY);
`  Write_Stack("../data/test.tif", stack);
#endif 
  
  /******** test neuron tracing *************/
#if 1
 Set_Zscale(0.8342);

  Neurochain *chain = Guarded_Malloc(sizeof(Neurochain), "main");
  //set_neuroseg(&(chain->seg), 2.16869, 2.16869, 5, 1.35498, -0.0205087);
  set_neuroseg(&(chain->seg), 5, 5, 5, TZ_PI_2, 0);
  chain->prev = NULL;
  chain->next = NULL;
  /*
  chain->position[0] = 112;
  chain->position[1] = 93;
  chain->position[2] = 103.0 * 0.8342;
  */

  chain->position[0] = 98;
  chain->position[1] = 235;
  chain->position[2] = 108.0 * 0.8342;

  /*
  chain->position[0] = 113;
  chain->position[1] = 94;
  chain->position[2] = 108.0 * 0.8342;
  */

  /*
  chain->position[0] = 104.897;
  chain->position[1] = 181.323;
  chain->position[2] = 86.9195;

  chain->next = (Neurochain *) malloc(sizeof(Neurochain));
  chain->next->seg = chain->seg;
  chain->next->position[0] = 105.197;
  chain->next->position[1] = chain->position[1] - 5;
  chain->next->position[2] = chain->position[2];
  chain->next->next = NULL;
  */

  Stack *stack = loadTestStack2();
  
#if 1
  Try {
    Fit_Neuroseg2(stack, &(chain->seg), chain->position);
  } Catch(e) {
    TZ_ERROR(e);
  }
#endif

#if 1
  Try {
    Trace_Neuron2(stack, chain);
  } Catch(e) {
    TZ_ERROR(e);
  }
#endif
  
  stack = Translate_Stack(stack, COLOR, 1);
  neurochain_label(stack, chain);

  Print_Neuroseg(&(chain->seg));
  Write_Stack("../data/test.tif", stack);
  
#endif

  /*************morph operation*************************/
#if 0
  Matlab_Array mr1;
  mr_read("../data/mattest2.mat",&mr1);
  print_mrinfo(&mr1);

  Array_Link *a = Mar_To_Stack(&mr1);
  

  Struct_Element *se = Make_Corner_Se_2D(4);
  Stack *stack = Se_To_Stack(se);
  Print_Stack_Value(stack);
  
  Coordinate center = {1, 1, 0};
  Struct_Element *se2 = Stack_To_Se(stack, center);
  Print_Se(se2);
  Print_Stack_Value(Se_To_Stack(se2));

  Print_Stack_Value(Get_Stack_At(a));

  Stack *stack2 = Copy_Stack(Get_Stack_At(a));
  Stack_Hitmiss(Get_Stack_At(a), stack2, se);
  Print_Stack_Value(stack2);
#endif 
  

  /*************test image display***************************/
#if 0
  Image *image = loadTestImage();

  Try {
    Display_Image(image);
  } Catch(e) {
    TZ_ERROR(e);
  } 
#endif

  return 0;  
}




