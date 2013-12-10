/* tz_stack_lib.c
 * Initial write: Ting Zhao
 */

#include "tz_stack_lib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <math.h>
#include <utilities.h>
#include "tz_image_lib.h"
#include "tz_iimage_lib.h"
#include "tz_utilities.h"
#include "tz_iarray.h"
#include "tz_dimage_lib.h"
#include "tz_stack_relation.h"
#include "tz_stack_attribute.h"
#include "tz_image_array.h"
#include "tz_stack_stat.h"
#include "tz_interface.h"
#include "tz_stack_neighborhood.h"
#include "tz_error.h"
#include "tz_darray.h"
#include "tz_stack_math.h"
#include "tz_math.h"
#include "tz_image_io.h"

INIT_EXCEPTION

#include "private/tz_image_lib.c"
#include "private/tz_stack_lib.c"

#define PROJ_STACK_ZMAX(stack_array, image_array)			\
  for(z=1;z<stack->depth;z++) {						\
    image_offset = 0;							\
    for(y=0;y<stack->height;y++)					\
      for(x=0;x<stack->width;x++) {					\
	if(image_array[image_offset]<stack_array[stack_offset])		\
	  image_array[image_offset] = stack_array[stack_offset];	\
	image_offset++;							\
	stack_offset++;							\
    }									\
  }

/* Proj_Stack_Zmax(): Z-projetion of a stack by maximum intensity.
 * 
 * Args: stack - input stack.
 *
 * Return: the projection image.
 */
Image* Proj_Stack_Zmax(const Stack* stack)
{
  if(stack) {
    int x,y,z;
    Image* image = NULL; 

    size_t image_offset=0;
    size_t stack_offset = 0;

    Image_Array sta;
    sta.array = stack->array;
    
    Image_Array ima;
    image = Make_Image(stack->kind,stack->width,stack->height);
    ima.array = image->array;

    stack_offset = stack->width*stack->height;
    memcpy(image->array,stack->array,(size_t)stack_offset * stack->kind);

    switch(stack->kind) {
    case GREY:
      PROJ_STACK_ZMAX(sta.array, ima.array);
      break;
    case GREY16:
      PROJ_STACK_ZMAX(sta.array16, ima.array16);
      break;
    case FLOAT32:
      PROJ_STACK_ZMAX(sta.array32, ima.array32);
      break;
    case FLOAT64:
      PROJ_STACK_ZMAX(sta.array64, ima.array64);
      break;
    case COLOR:
      for(z=1;z<stack->depth;z++) {
	image_offset = 0;
	for(y=0;y<stack->height;y++)
	  for(x=0;x<stack->width;x++) {
	    int c;
	    for (c = 0; c < 3; c++) {
	      if(ima.arrayc[image_offset][c]<sta.arrayc[stack_offset][c]) {
		ima.arrayc[image_offset][c] = sta.arrayc[stack_offset][c];
	      }
	    }
	    image_offset++;
	    stack_offset++;
	  }		
      }
      break;
    default:
      fprintf(stderr,"Unsupported image kind in Proj_Stack_Zmax()\n");
      exit(1);
    }

    return image;
  } else {
    return NULL;
  }
}

#define PROJ_STACK_ZMIN(stack_array, image_array)			\
  for(z=1;z<stack->depth;z++) {						\
    image_offset = 0;							\
    for(y=0;y<stack->height;y++)					\
      for(x=0;x<stack->width;x++) {					\
	if(image_array[image_offset] > stack_array[stack_offset])	\
	  image_array[image_offset] = stack_array[stack_offset];	\
	image_offset++;							\
	stack_offset++;							\
    }									\
  }

Image* Proj_Stack_Zmin(const Stack* stack)
{
  if(stack) {
    int x,y,z;
    Image* image = NULL; 

    size_t image_offset=0;
    size_t stack_offset = 0;

    Image_Array sta;
    sta.array = stack->array;
    
    Image_Array ima;
    image = Make_Image(stack->kind,stack->width,stack->height);
    ima.array = image->array;

    stack_offset = stack->width*stack->height;
    memcpy(image->array,stack->array,(size_t)stack_offset * stack->kind);

    switch(stack->kind) {
    case GREY:
      PROJ_STACK_ZMIN(sta.array, ima.array);
      break;
    case GREY16:
      PROJ_STACK_ZMIN(sta.array16, ima.array16);
      break;
    case FLOAT32:
      PROJ_STACK_ZMIN(sta.array32, ima.array32);
      break;
    case FLOAT64:
      PROJ_STACK_ZMIN(sta.array64, ima.array64);
      break;
    case COLOR:
      for(z=1;z<stack->depth;z++) {
	image_offset = 0;
	for(y=0;y<stack->height;y++)
	  for(x=0;x<stack->width;x++) {
	    int c;
	    for (c = 0; c < 3; c++) {
	      if(ima.arrayc[image_offset][c]<sta.arrayc[stack_offset][c]) {
		ima.arrayc[image_offset][c] = sta.arrayc[stack_offset][c];
	      }
	    }
	    image_offset++;
	    stack_offset++;
	  }		
      }
      break;
    default:
      fprintf(stderr,"Unsupported image kind in Proj_Stack_Zmin()\n");
      exit(1);
    }

    return image;
  } else {
    return NULL;
  }
}

#define PROJ_STACK_XMAX(stack_array, image_array)			\
{\
  for (z = 0; z < stack->depth; z++) { \
    for (y = 0; y < stack->height; y++) { \
      for (x = 0; x < stack->width; x++) { \
	if(image_array[image_offset] < stack_array[stack_offset]) {	\
	  image_array[image_offset] = stack_array[stack_offset];	\
        } \
        stack_offset++; \
      } \
      image_offset++; \
    } \
  } \
}

/* Proj_Stack_Xmax(): X-projetion of a stack by maximum intensity.
 * 
 * Args: stack - input stack.
 *
 * Return: the projection image.
 */
Image* Proj_Stack_Xmax(const Stack* stack)
{
  if(stack) {
    int x,y,z;
    Image* image = NULL; 

    size_t image_offset=0;
    size_t stack_offset = 0;

    Image_Array sta;
    sta.array = stack->array;
    
    Image_Array ima;
    image = Make_Image(stack->kind,stack->height,stack->depth);
    ima.array = image->array;
    Zero_Image(image);

    switch(stack->kind) {
    case GREY:
      PROJ_STACK_XMAX(sta.array, ima.array);
      break;
    case GREY16:
      PROJ_STACK_XMAX(sta.array16, ima.array16);
      break;
    case FLOAT32:
      PROJ_STACK_XMAX(sta.array32, ima.array32);
      break;
    case FLOAT64:
      PROJ_STACK_XMAX(sta.array64, ima.array64);
      break;
    default:
      TZ_ERROR(ERROR_DATA_TYPE);
    }

    return image;
  } else {
    return NULL;
  }
}

#define PROJ_STACK_YMAX(stack_array, image_array)			\
{\
  size_t area = stack->width * stack->height; \
  for (z = 0; z < stack->depth; z++) { \
    for (x = 0; x < stack->width; x++) { \
      stack_offset = area * z + x; \
      for (y = 0; y < stack->height; y++) { \
	if(image_array[image_offset] < stack_array[stack_offset]) {	\
	  image_array[image_offset] = stack_array[stack_offset];	\
        } \
        stack_offset += stack->width; \
      } \
      image_offset++; \
    } \
  } \
}

/* Proj_Stack_Ymax(): Y-projetion of a stack by maximum intensity.
 * 
 * Args: stack - input stack.
 *
 * Return: the projection image.
 */
Image* Proj_Stack_Ymax(const Stack* stack)
{
  if(stack) {
    int x,y,z;
    Image* image = NULL; 

    size_t image_offset=0;
    size_t stack_offset = 0;

    Image_Array sta;
    sta.array = stack->array;
    
    Image_Array ima;
    image = Make_Image(stack->kind,stack->width,stack->depth);
    ima.array = image->array;
    Zero_Image(image);

    switch(stack->kind) {
    case GREY:
      PROJ_STACK_YMAX(sta.array, ima.array);
      break;
    case GREY16:
      PROJ_STACK_YMAX(sta.array16, ima.array16);
      break;
    case FLOAT32:
      PROJ_STACK_YMAX(sta.array32, ima.array32);
      break;
    case FLOAT64:
      PROJ_STACK_YMAX(sta.array64, ima.array64);
      break;
    default:
      TZ_ERROR(ERROR_DATA_TYPE);
    }

    return image;
  } else {
    return NULL;
  }
}

#define STACK_HITTEST_Z(stack_array)					\
  for(z=0; z<stack->depth; z++) {					\
    if (stack_array[offset] > 0) {					\
      value = stack_array[offset];					\
      break;								\
    }									\
    offset += area;							\
  }

double Stack_Hittest_Z(const Stack *stack, int x, int y)
{
  TZ_ASSERT(stack != NULL, "Null stack");

  if ((x >= stack->width) || (x < 0) || (y >= stack->height) || (y < 0)) {
    return 0.0;
  }

  int z = 0;
  int area = stack->width * stack->height;
  int offset = y * stack->width + x;
  double value = 0.0;

  SCALAR_STACK_OPERATION(stack, STACK_HITTEST_Z);

  return value;
}


/* Zero_Stack(): Set all pixels in a stack to 0.
 *
 * Args: stack - input stack. All pixels in this stack will be 0 after function
 *               return.
 *
 * Return: void.
 */
void Zero_Stack(Stack *stack)
{
  size_t i;
  size_t length = Get_Stack_Size(stack);
  float* array32 = (float*)stack->array;
  double *array64 = (double *) stack->array;

  switch(stack->kind) {
  case COLOR:
  case GREY:
  case GREY16:
    length *= stack->kind;
    bzero(stack->array, length);
    /*
    for(i=0;i<length;i++)
      stack->array[i] = 0x00;
    */
    break;
  case FLOAT32:
    for(i=0;i<length;i++)
      array32[i] = 0.0;
    break;
  case FLOAT64:
    for(i=0;i<length;i++)
      array64[i] = 0.0;
    break;  
  default:
    TZ_WARN(ERROR_DATA_TYPE);
    fprintf(stderr, "Unsuppored stack kind: %d\n", stack->kind);
  }
}

void One_Stack(Stack *stack)
{
  DEFINE_ARRAY_ALL(array, stack);
  
  size_t length = Stack_Voxel_Number(stack);
  size_t i;
  
  switch (Stack_Kind(stack)) {
  case GREY:
    memset(array_grey, 1, length);
    /*
    for (i = 0; i < length; i++) {
      array_grey[i] = 1;
    }
    */
    break;
  case GREY16:
    for (i = 0; i < length; i++) {
      array_grey16[i] = 1;
    }
    break;
  case COLOR:
    for (i = 0; i < length; i++) {
      array_color[i][0] = 1;
      array_color[i][1] = 1;
      array_color[i][2] = 1;
    }
    break;
  case FLOAT32:
    for (i = 0; i < length; i++) {
      array_float32[i] = 1.0;
    }
    break;
  case FLOAT64:
    for (i = 0; i < length; i++) {
      array_float64[i] = 1.0;
    }
    break;
  default:
    THROW(ERROR_DATA_TYPE);
  }
}

/* Stack_Set_Constant(): Set all voxels in a stack to a constant.
 */
void Stack_Set_Constant(Stack  *stack, const void *value)
{
  DEFINE_ARRAY_ALL(array, stack);
  
  size_t length = Stack_Voxel_Number(stack);
  size_t i;
  
  switch (Stack_Kind(stack)) {
  case GREY:
    for (i = 0; i < length; i++) {
      array_grey[i] = *((uint8 *) value);
    }
    break;
  case GREY16:
    for (i = 0; i < length; i++) {
      array_grey16[i] = *((uint16 *) value);
    }
    break;
  case COLOR:
    for (i = 0; i < length; i++) {
      array_color[i][0] = ((uint8 *) value)[0];
      array_color[i][1] = ((uint8 *) value)[1];
      array_color[i][2] = ((uint8 *) value)[2];
    }
    break;
  case FLOAT32:
    for (i = 0; i < length; i++) {
      array_float32[i] = *((float32 *) value);
    }
    break;
  case FLOAT64:
    for (i = 0; i < length; i++) {
      array_float64[i] = *((float64 *) value);
    }
    break;
  default:
    THROW(ERROR_DATA_TYPE);
  }
}

/* Copy_Stack_Array(): Copy the array of one stack to the other's.
 *
 * Args: dst - destination stack;
 *       src - source stack.
 *
 * Return: void.
 */
void Copy_Stack_Array(Stack *dst, const Stack *src)
{
  if (Stack_Same_Attribute(dst, src) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  memcpy(dst->array, src->array, Stack_Array_Bsize(dst));
}

Stack* Copy_Stack_T(Stack *stack)
{
  BOOL null_text = FALSE;
  if (stack->text == NULL) {
    stack->text = "\0";
    null_text = TRUE;
  }
  
  Stack *out = Copy_Stack(stack);

  if (null_text == TRUE) {
    stack->text = NULL;
  }

  return out;
}

/* Get_White_Matrix3(): Get a binary matrix from a stack.
 *
 * Args: stack - input stack, which must be GREY  or GREY16 kind.
 *
 * Return: an integer matrix with values 0 and 1. 1 corresponds to the non-zero
 *         pixels in the stack and 0 corresponds to other pixels.
 */
IMatrix* Get_White_Matrix3(Stack* stack) 
{
  IMatrix* dm = NULL;

  dim_type dim[3];
  dim[0] = (dim_type)stack->width;
  dim[1] = (dim_type)stack->height;
  dim[2] = (dim_type)stack->depth;

  dm = Make_IMatrix(dim,3);
  
  uint16* array16 = (uint16*) (stack->array);
  size_t i,length;
  length = dim[0]*dim[1]*dim[2];

  switch(stack->kind) {
  case GREY:
    for(i=0;i<length;i++)
      dm->array[i] = stack->array[i]>0;
    break;
  case GREY16:
    for(i=0;i<length;i++)
      dm->array[i] = array16[i]>0;   
    break;
  default:
    TZ_WARN(ERROR_DATA_TYPE);
    fprintf(stderr, "Unsuppored stack kind: %d\n", stack->kind);
    Kill_IMatrix(dm);
    dm = NULL;
  }

  return dm;
}

IMatrix* Count_Stack_Fg(Stack *stack,dim_type bdim[])
{
  IMatrix *im = Get_White_Matrix3(stack);
  IMatrix *im2 = IMatrix_Blocksum(im, bdim, NULL);
  Kill_IMatrix(im);
  return im2;
}

IMatrix* Stack_Foreoverlap(Stack *stack1,Stack *stack2)
{
  double *corr = Correlate_Padstack_D(stack1,stack2);
  IMatrix* im = darray2imatrix(corr,3,stack1->width+stack2->width-1,
			stack1->height+stack2->height-1,
			stack1->depth+stack2->depth-1);
  free(corr);
  return im;
}

IMatrix* Stack_Foreunion(Stack *stack1,Stack *stack2)
{
  dim_type bdim[3];
  Reflect_Stack(stack1,1);
  IMatrix *istack1 = Get_Int_Matrix3(stack1);
  bdim[0] = stack2->width; bdim[1] = stack2->height; bdim[2] = stack2->depth;
  IMatrix *ib1 = IMatrix_Blocksum(istack1, bdim, NULL);
  Kill_IMatrix(istack1);

  IMatrix *istack2 = Get_Int_Matrix3(stack2);
  bdim[0] = stack1->width; bdim[1] = stack1->height; bdim[2] = stack1->depth;
  IMatrix *ib2 = IMatrix_Blocksum(istack2, bdim, NULL);
  Kill_IMatrix(istack2);
  //iarray_add(ib1->array,ib2->array,matrix_size(ib1->dim,ib1->ndim));
  IMatrix_Add(ib1,ib2);
  Kill_IMatrix(ib2);
  Reflect_Stack(stack1,1);
  IMatrix *im = Stack_Foreoverlap(stack1,stack2);
  //iarray_sub(ib1->array,im->array,matrix_size(ib1->dim,ib1->ndim));
  IMatrix_Sub(ib1,ib2);
  Kill_IMatrix(im);

  return ib1;
}

IMatrix* Stack_Foreunion_cthr(Stack *stack1,Stack *stack2,int minol)
{
  dim_type bdim[3];
  Reflect_Stack(stack1,1);
  IMatrix *istack1 = Get_Int_Matrix3(stack1);
  bdim[0] = stack2->width; bdim[1] = stack2->height; bdim[2] = stack2->depth;
  IMatrix *ib1 = IMatrix_Blocksum(istack1, bdim, NULL);
  Kill_IMatrix(istack1);

  IMatrix *istack2 = Get_Int_Matrix3(stack2);
  bdim[0] = stack1->width; bdim[1] = stack1->height; bdim[2] = stack1->depth;
  IMatrix *ib2 = IMatrix_Blocksum(istack2, bdim, NULL);
  Kill_IMatrix(istack2);

  int length = matrix_size(ib1->dim,ib1->ndim);
  //iarray_add(ib1->array,ib2->array,length);
  IMatrix_Add(ib1,ib2);
  Kill_IMatrix(ib2);
  Reflect_Stack(stack1,1);
  IMatrix *im = Stack_Foreoverlap(stack1,stack2);
  //iarray_sub(ib1->array,im->array,length);
  IMatrix_Sub(ib1,ib2);
  int i;
  for(i=0;i<length;i++)
    if(im->array[i]<=minol)
      ib1->array[i] = 0;

  Kill_IMatrix(im);

  return ib1;
}

/**@fn Stack* Crop_Stack(const Stack* stack,int left,int top,int front,
		  int width,int height,int depth, Stack *desstack)
 */
Stack* Crop_Stack(const Stack* stack,int left,int top,int front,
		  int width,int height,int depth, Stack *desstack)
{
  if((stack == NULL) || (width <= 0) || (height <= 0) || (depth <= 0)) {
    return NULL;
  }

  int stack_width = stack->width;
  int stack_height = stack->height;
  int stack_depth = stack->depth;

  /*The range is exactly the same as the original stack*/
  if(left==0 && top==0 && front==0 && 
     width==stack_width && height==stack_height && depth==stack_depth) {
    if (desstack == NULL) {
      return Copy_Stack((Stack *) stack);
    } else {
      Copy_Stack_Array(desstack, stack);
      return desstack;
    }
  }

  if (desstack == NULL) {
    desstack = Make_Stack(stack->kind, width, height, depth);
  } else {
    desstack->kind = stack->kind;
    desstack->width = width;
    desstack->height = height;
    desstack->depth = depth;
  }

  long length =  desstack->width*desstack->height*desstack->depth*desstack->kind;
  long k;
  if (desstack->array != stack->array) {
    for(k=0;k<length;k++) {
      desstack->array[k]=0;
    }
  }

  /*determine copy range*/
  int srcleft,srctop,srcfront,desleft,destop,desfront,copy_width,copy_height,copy_depth;
  srcleft = imax2(left,0);
  srctop = imax2(top,0);
  srcfront = imax2(front,0);
  desleft = imax2(-left,0);
  destop = imax2(-top,0);
  desfront = imax2(-front,0);

  copy_width = imin2(stack_width-srcleft,width-desleft)*stack->kind;
  copy_height = imin2(stack_height-srctop,height-destop);
  copy_depth = imin2(stack_depth-srcfront,depth-desfront);

  size_t i,j,srcoffset,desoffset;
  int nsrcrow = stack_width*stack->kind;
  int ndesrow = width*stack->kind;
  int nsrcplane = nsrcrow*stack_height;
  int ndesplane = ndesrow*height;
  
  srcoffset = srcleft*stack->kind+nsrcrow*srctop+nsrcplane*srcfront;
  desoffset = desleft*stack->kind+ndesrow*destop+ndesplane*desfront;

  size_t temp_srcoffset,temp_desoffset;
  temp_srcoffset = srcoffset;
  temp_desoffset = desoffset;

  /*copy row by row*/
  for(j=0;j<copy_depth;j++) {
    for(i=0;i<copy_height;i++) {
      if (desstack->array == stack->array) {
	memmove((void *)(desstack->array+desoffset),
		(void *)(stack->array+srcoffset), (size_t)copy_width);
      } else {
	memcpy((void *)(desstack->array+desoffset),
	       (void *)(stack->array+srcoffset), (size_t)copy_width);
      }
      srcoffset += nsrcrow;
      desoffset += ndesrow;
    }
    srcoffset = temp_srcoffset+nsrcplane;
    desoffset = temp_desoffset+ndesplane;
    temp_srcoffset = srcoffset;
    temp_desoffset = desoffset;
  }

  return desstack;  
}


void Print_Stack_Value(Stack* stack) 
{
  if (stack == NULL) {
    printf("Null stack.\n");
    return;
  }

  int i,j,k;
  long offset=0;
  uint16* array16 = (uint16*) (stack->array);
  float32 *array32 = (float32 *) stack->array;
  float64 *array64 = (float64 *) stack->array;

  switch(stack->kind) {
  case GREY:
    for(k=0;k<stack->depth;k++) {
      printf("Slice %d:\n",k);
      for(j=0;j<stack->height;j++) {
	for(i=0;i<stack->width;i++) {
	  printf("%d ",stack->array[offset]);
	  offset++;
	}
	printf("\n");
      }
      printf("\n");
    }    
    break;
  case GREY16:
    for(k=0;k<stack->depth;k++) {
      printf("Slice %d:\n",k);
      for(j=0;j<stack->height;j++) {
	for(i=0;i<stack->width;i++) {
	  printf("%d ",array16[offset]);
	  offset++;
	}
	printf("\n");
      }
      printf("\n");
    }
    break;
  case FLOAT32:
    for(k=0;k<stack->depth;k++) {
      printf("Slice %d:\n",k);
      for(j=0;j<stack->height;j++) {
	for(i=0;i<stack->width;i++) {
	  printf("%.4f ",array32[offset]);
	  offset++;
	}
	printf("\n");
      }
      printf("\n");
    }
    break;
  case FLOAT64:
    for(k=0;k<stack->depth;k++) {
      printf("Slice %d:\n",k);
      for(j=0;j<stack->height;j++) {
	for(i=0;i<stack->width;i++) {
	  printf("%.g ",array64[offset]);
	  offset++;
	}
	printf("\n");
      }
      printf("\n");
    }
    break;
  default:
    fprintf(stderr,"Unsupported stack kind in Print_Stack_Value()\n");
  }
}

double Stack_Array_Value(const Stack *stack, size_t index)
{
  Image_Array ima;
  ima.array = stack->array;
  switch (stack->kind) {
  case GREY:
    return ima.array8[index];
  case GREY16:
    return ima.array16[index];
  case FLOAT32:
    return ima.array32[index];
  case FLOAT64:
    return ima.array64[index];
  case COLOR:
    return ima.array8[index];
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  return -1.0;
}

Stack* Scale_Float_Stack(float *fStack, int  width, int height, 
			 int depth, int kind)
{
  long length = width*height*depth;
  float maxgrey = maxfloat(fStack,length);
  float mingrey = minfloat(fStack,length);
  
  float dest_max = (float)MAXGREY(kind);

  int i;
  float maxdiff = maxgrey - mingrey;
  Stack* stack = Make_Stack(kind,width,height,depth);
  uint16* array16 = (uint16*)stack->array;
  float32* array32 = (float32*)stack->array;

  switch(kind) {
  case GREY:
    for(i=0;i<length;i++)
      stack->array[i] = (uint8) (dest_max*(fStack[i]-mingrey)/maxdiff);
    break;
  case COLOR:
    length *= 3;
    for(i=0;i<length;i++)
      stack->array[i] = (uint8) (dest_max*(fStack[i]-mingrey)/maxdiff);
    break;
  case GREY16:
    for(i=0;i<length;i++)
      array16[i] = (uint16) (dest_max*(fStack[i]-mingrey)/maxdiff);
    break;
  case FLOAT32:
    for(i=0;i<length;i++)
      array32[i] = (fStack[i]-mingrey)/maxdiff;
    break;
  default:
    fprintf(stderr,"Get_Double_Array: Unsupported image kind.\n");
  }

  return stack;  
}

/*
 * Scale_Double_Stack(): Scale a double array of a stack.
 *
 * Args: fStack - the double array to scale;
 *       width - width of the stack;
 *       height - height of the stack;
 *       depth - depth of the stack;
 *       kind - kind of the stack.
 *
 * Return: The scaled stack.
 *
 * Note: The caller is  responsible for freeing the returned stack.
 */
Stack* Scale_Double_Stack(double *fStack, int  width, int height, 
			  int depth, int kind)
{
  long length = width*height*depth;
  //double maxgrey = maxdouble(fStack,length);
  //double mingrey = mindouble(fStack,length);  
  double maxgrey = darray_max(fStack, length, NULL);
  double mingrey = darray_min(fStack, length, NULL);
  double dest_max = (double)MAXGREY(kind);

  int i;
  double maxdiff = maxgrey - mingrey;
  Stack* stack = Make_Stack(kind,width,height,depth);
  
  uint16* array16 = (uint16*)stack->array;
  float32* array32 = (float32*)stack->array;

  switch(kind) {
  case GREY:
    for(i=0;i<length;i++) {
      if (maxgrey == mingrey) {
	stack->array[i] = (uint8) (dest_max / 2);
      } else {
	stack->array[i] = (uint8) (dest_max*(fStack[i]-mingrey)/maxdiff);
      } 
    }
    break;
  case COLOR:
    length *= 3;
    for(i=0;i<length;i++) {
      if (maxgrey == mingrey) {
	stack->array[i] = (uint8) (dest_max / 2);
      } else {
	stack->array[i] = (uint8) (dest_max*(fStack[i]-mingrey)/maxdiff);
      }
    }
    break;
  case GREY16:
    for(i=0;i<length;i++) {
      if (maxgrey == mingrey) {
	array16[i] = (uint16) (dest_max / 2);
      } else {
	array16[i] = (uint16) (dest_max*(fStack[i]-mingrey)/maxdiff);
      }
    }
    break;
  case FLOAT32:
    for(i=0;i<length;i++) {
      if (maxgrey == mingrey) {
	array32[i] = (float) (dest_max / 2);
      } else {
	array32[i] = (float) (fStack[i]-mingrey)/maxdiff;
      }
    }
    break;
  default:
    fprintf(stderr,"Get_Double_Array: Unsupported image kind.\n");
  }
  
  return stack;
}

Stack* Scale_Double_Stack_P(double *fStack, int  width, int height, 
			    int depth, int kind)
{
  long length = width*height*depth;
  //double maxgrey = maxdouble(fStack,length);
  //double mingrey = mindouble(fStack,length);  
  double maxgrey = darray_max(fStack, length, NULL);
  double mingrey = darray_min(fStack, length, NULL);
  double dest_max = (double)MAXGREY(kind);

  int i;
  double maxdiff = maxgrey - mingrey;
  Stack* stack = Make_Stack(kind,width,height,depth);
  
  uint16* array16 = (uint16*)stack->array;
  float32* array32 = (float32*)stack->array;

  switch(kind) {
  case GREY:
    for(i=0;i<length;i++) {
      if (fStack[i] > 0) {
	if (maxgrey == mingrey) {
	  stack->array[i] = (uint8) (dest_max / 2);
	} else {
	  stack->array[i] = (uint8) (dest_max*(fStack[i]-mingrey)/maxdiff);
	} 
      } else {
	stack->array[i] = 0;
      }
    }
    break;
  case COLOR:
    length *= 3;
    for(i=0;i<length;i++) {
      if (fStack[i] > 0) {
	if (maxgrey == mingrey) {
	  stack->array[i] = (uint8) (dest_max / 2);
	} else {
	  stack->array[i] = (uint8) (dest_max*(fStack[i]-mingrey)/maxdiff);
	}
      } else {
	stack->array[i] = 0;
      }
    }
    break;
  case GREY16:
    for(i=0;i<length;i++) {
      if (fStack[i] > 0) {
	if (maxgrey == mingrey) {
	  array16[i] = (uint16) (dest_max / 2);
	} else {
	  array16[i] = (uint16) (dest_max*(fStack[i]-mingrey)/maxdiff);
	}
      } else {
	array16[i] = 0;
      }
    }
    break;
  case FLOAT32:
    for(i=0;i<length;i++) {
      if (fStack[i] > 0) {
	if (maxgrey == mingrey) {
	  array32[i] = (float) (dest_max / 2);
	} else {
	  array32[i] = (float) (fStack[i]-mingrey)/maxdiff;
	}
      } else {
	array32[i] = 0;
      }
    }
    break;
  default:
    fprintf(stderr,"Get_Double_Array: Unsupported image kind.\n");
  }
  
  return stack;
}

void Stack_Brighten_Bw(Stack *stack)
{
  ASSERT((stack->kind == GREY) || (stack->kind == GREY16),
	 "unsupported stack kind");

  int n = Stack_Voxel_Number(stack);
  int i;
  uint16 *array16 = (uint16 *) stack->array;

  switch (stack->kind) {
  case GREY:
    for (i = 0; i < n; i++) {
      stack->array[i] = ~(stack->array[i]) + 0x01;
    }
    break;
  case GREY16:
 
    for (i = 0; i < n; i++) {
      array16[i] = ~(array16[i]) + 0x0001;
    }
    break;
  default:
    PRINT_EXCEPTION("Unsupported kind", "GREY, GREY16 only.");
  }
}

void Stack_Brighten_Level(Stack *stack, int level)
{
  ASSERT((stack->kind == GREY) || (stack->kind == GREY16),
	 "unsupported stack kind");

  int n = Stack_Voxel_Number(stack);
  int i;
  uint16 *array16 = (uint16 *) stack->array;

  switch (stack->kind) {
  case GREY:
    for (i = 0; i < n; i++) {
      if (stack->array[i] == level) {
	stack->array[i] = 0xFF;
      } else {
	stack->array[i] = 0;
      }
    }
    break;
  case GREY16:
    for (i = 0; i < n; i++) {
      if (array16[i] == level) {
	array16[i] = 0xFFFF;
      } else {
	array16[i] = 0;
      }
    }
    break;
  default:
    PRINT_EXCEPTION("Unsupported kind", "GREY, GREY16 only.");
    break;
  }
}

size_t Stack_Level_Mask(Stack *stack, int level)
{
  ASSERT((stack->kind == GREY) || (stack->kind == GREY16),
	 "unsupported stack kind");

  size_t foreground_size = 0;

  size_t n = Stack_Voxel_Number(stack);
  size_t i;
  uint16 *array16 = (uint16 *) stack->array;

  switch (stack->kind) {
  case GREY:
    for (i = 0; i < n; i++) {
      if (stack->array[i] == level) {
	stack->array[i] = 1;
        foreground_size++;
      } else {
	stack->array[i] = 0;
      }
    }
    break;
  case GREY16:
    for (i = 0; i < n; i++) {
      if (array16[i] == level) {
	array16[i] = 1;
        foreground_size++;
      } else {
	array16[i] = 0;
      }
    }
    break;
  default:
    PRINT_EXCEPTION("Unsupported kind", "GREY, GREY16 only.");
  }

  return foreground_size;
}

size_t Stack_Level_Mask2(const Stack *stack, Stack *out, int level)
{
  ASSERT((stack->kind == GREY) || (stack->kind == GREY16),
	 "unsupported stack kind");

  size_t foreground_size = 0;

  size_t n = Stack_Voxel_Number(stack);
  size_t i;
  uint16 *array16 = (uint16 *) stack->array;
  Image_Array out_ima;
  out_ima.array = out->array;

  switch (stack->kind) {
  case GREY:
    for (i = 0; i < n; i++) {
      if (stack->array[i] == level) {
        foreground_size++;
      } else {
        switch (out->kind) {
          case GREY:
            out_ima.array[i] = 0;
            break;
          case GREY16:
            out_ima.array16[i] = 0;
            break;
        }
      }
    }
    break;
  case GREY16:
    for (i = 0; i < n; i++) {
      if (array16[i] == level) {
        foreground_size++;
      } else {
        switch (out->kind) {
          case GREY:
            out_ima.array[i] = 0;
            break;
          case GREY16:
            out_ima.array16[i] = 0;
            break;
        }
      }
    }
    break;
  default:
    PRINT_EXCEPTION("Unsupported kind", "GREY, GREY16 only.");
  }

  return foreground_size;
}

void Stretch_Stack_Value(Stack *stack)
{
  if (Stack_Channel_Number(stack) > 1) {
    PRINT_EXCEPTION("Unsupported kind", "Multichannel stack not supported.");
  }

  Pixel_Range *range = Stack_Range(stack, 0);
  double maxg = 0.0;
  switch (stack->kind) {
  case GREY:
    maxg = 255.0;
    break;
  case GREY16:
    maxg = 65535.0;
    break;
  case FLOAT32:
  case FLOAT64:
    maxg = 1.0;
    break;
  default:
    PRINT_EXCEPTION("Unsupported kind", "GREY, GREY16, FLOAT32, FLOAT64 only.");
  }
  
  double factor = maxg / (range->maxval - range->minval);
  double offset = -range->minval;
  Scale_Stack(stack, 0, factor, offset);
}

void Stretch_Stack_Value_Q(Stack *stack, double q)
{
  if (Stack_Channel_Number(stack) > 1) {
    PRINT_EXCEPTION("Unsupported kind", "Multichannel stack not supported.");
  }

  int *hist = Stack_Hist(stack); /* hist created */
  int *raw_hist = hist + 2;

  int min = hist[1];

  iarray_cumsum(raw_hist, hist[0]);

  int max;
  int length = Stack_Voxel_Number(stack);
  double threshold = q * length;

  for (max = hist[0] - 2; max >= 0; max--) {
    if (raw_hist[max] < threshold) {
      break;
    }
  }
  max++;

  double maxg = 0.0;
  int i;

  switch (stack->kind) {
  case GREY:
    maxg = 255.0;
    for (i = 0; i < length; i++) {
      if (stack->array[i] > max) {
	stack->array[i] = max;
      }
    }
    break;
  case GREY16:
    {
      uint16 *array16 = (uint16 *) stack->array;
      for (i = 0; i < length; i++) {
      if (array16[i] > max) {
	array16[i] = max;
      }
    }
      maxg = 65535.0;
    }
    break;
  default:
    free(hist);
    PRINT_EXCEPTION("Unsupported kind", "GREY, GREY16 only.");
  }
  
  printf("%d, %d\n", min, max);

  double factor = maxg / max;
  double offset = -min;
  Scale_Stack(stack, 0, factor, offset);  

  free(hist);
}

Stack* Reflect_Stack(Stack* stack,int in_place)
{
  Stack* stack2;
  if(in_place)
    stack2 = stack;
  else
    stack2 = Make_Stack(stack->kind,stack->width,stack->height,stack->depth);

  long offset,offset2;
  long length=stack->width*stack->height*stack->depth; 

  switch(stack->kind) {
  case GREY:
    offset2 = length - 1;
    if(in_place) {
      uint8 temp; /* for swapping */
      long length2 = length/2; /* swap half of the array */
      for(offset=0;offset<length2;offset++,offset2--) {
	temp = stack->array[offset];
	stack->array[offset] = stack->array[offset2];
	stack2->array[offset2] = temp;
      }
    } else {  
      for(offset=0;offset<length;offset++,offset2--)
	stack2->array[offset2] = stack->array[offset];
    }
    break;
  case GREY16: {
    offset2 = length - 1;
    uint16* stack_array = (uint16 *)stack->array;
    uint16* stack_array2 = (uint16 *)stack2->array;
    if(in_place) {
      uint16 temp; /* for swapping */
      long length2 = length/2; /* swap half of the array */

      for(offset=0;offset<length2;offset++,offset2--) {
	temp = stack_array[offset];
	stack_array[offset] = stack_array2[offset2];
	stack_array2[offset2] = temp;
      }
    } else {  
      for(offset=0;offset<length;offset++,offset2--)
	stack_array2[offset2] = stack_array[offset];
    }
               }
    break;
  case FLOAT32: {
    offset2 = length-1;
    float32* stack_array32 = (float32 *)stack->array;
    float32* stack2_array32 = (float32 *)stack2->array;
    if(in_place) {
      float32 temp; /* for swapping */
      long length2 = length/2; /* swap half of the array */

      for(offset=0;offset<length2;offset++,offset2--) {
	temp = stack_array32[offset];
	stack_array32[offset] = stack2_array32[offset2];
	stack2_array32[offset2] = temp;
      }
    } else {  
      for(offset=0;offset<length;offset++,offset2--)
	stack2_array32[offset2] = stack_array32[offset];
    }
                }
    break;
  default:
    fprintf(stderr,"Unsupported image kind in Reflect_Image()\n");
  }
  return stack2;  
}

Stack* Flip_Stack_Z(Stack *stack, Stack *out)
{
  if (out == NULL) {
    out = Make_Stack(stack->kind, stack->width, stack->height, stack->depth);
  }

  int plane_size = stack->width*stack->height*stack->kind;
  int depth2 = stack->depth/2;
  void *tmp_plane = malloc(plane_size);
  int offset1,offset2;
  int i;
  for(i=0;i<depth2;i++) {
    offset1 = i*plane_size;
    offset2 = (stack->depth-i-1)*plane_size;
    memcpy(tmp_plane,stack->array+offset1,plane_size);
    memcpy(out->array+offset1,stack->array+offset2,plane_size);
    memcpy(out->array+offset2,tmp_plane,plane_size);
  }

  free(tmp_plane);

  return out;
}

Stack* Flip_Stack_Y(const Stack *stack, Stack *out)
{
  if (out == NULL) {
    out = Make_Stack(stack->kind, stack->width, stack->height, stack->depth);
  }

  int i, j;
  int plane_bsize = Stack_Voxel_Bsize(stack) * stack->width * stack->height;
  int row_bsize = Stack_Voxel_Bsize(stack) * stack->width;
  int height2 = stack->height / 2;
  
  void *tmp_row = malloc(row_bsize);
  uint8 *in_array = stack->array;
  uint8 *out_array = out->array;

  int offset1, offset2;

  for (j = 0; j < stack->depth; j++) {
    for (i = 0; i < height2; i++) {
      offset1 = i * row_bsize;
      offset2 = (stack->height - i - 1) * row_bsize;
      memcpy(tmp_row, in_array + offset1, row_bsize);
      memcpy(out_array + offset1, in_array + offset2, row_bsize);
      memcpy(out_array + offset2, tmp_row, row_bsize);
    }
    in_array += plane_bsize;
    out_array += plane_bsize;
  }
  
  free(tmp_row);

  return out;
}

Stack* Resize_Stack(const Stack* stack,int width,int height,int depth)
{
  Stack* stack2 = Make_Stack(stack->kind,width,height,depth);
  uint16* stack2_array16 = (uint16*) stack2->array;

  int* col_idx = (int*) malloc(sizeof(int)*width);
  int* row_idx = (int*) malloc(sizeof(int)*height);
  int* dep_idx = (int*) malloc(sizeof(int)*depth);

  map_index(stack->width,width,col_idx);
  map_index(stack->height,height,row_idx);
  map_index(stack->depth,depth,dep_idx);

  int i,j,k,offset=0;
  switch(stack->kind) {
  case GREY:
    for(k=0;k<depth;k++)
      for(i=0;i<height;i++)
	for(j=0;j<width;j++) {
	  stack2->array[offset] = 
      (*STACK_PIXEL_8(stack,col_idx[j],row_idx[i],dep_idx[k],0));
	  offset++;
	}
    break;
  case GREY16:
    for (k = 0; k < depth; k++) {
      for(i=0;i<height;i++) {
	for(j=0;j<width;j++) {
	  stack2_array16[offset] = 
      (*STACK_PIXEL_16(stack,col_idx[j],row_idx[i],dep_idx[k],0));
	  offset++;
	}
      }
    }
    break;
  default:
    fprintf(stderr,"Resize_Stack: Unsupported stack kind.\n");
  }

  free(col_idx);
  free(row_idx);
  free(dep_idx);

  return stack2;
}

Stack* Downsample_Stack(const Stack* stack,int wintv,int hintv,int dintv)
{
  int width,height,depth;
  int i,j,k;
  width = (stack->width+wintv)/(wintv+1);
  height = (stack->height+hintv)/(hintv+1);
  depth = (stack->depth+dintv)/(dintv+1);

  Stack* stack2 = Make_Stack(stack->kind,width,height,depth);
  uint16* stack2_array16 = (uint16*) stack2->array;
  color_t* stack2_arrayc = (color_t*) stack2->array;

  int* col_idx = (int*) malloc(sizeof(int)*width);
  int* row_idx = (int*) malloc(sizeof(int)*height);
  int* dep_idx = (int*) malloc(sizeof(int)*depth);

  size_t offset = 0;
  for(i=0;i<width;i++) {
    col_idx[i] = offset;
    offset += wintv+1;
  }

  offset = 0;
  for(i=0;i<height;i++) {
    row_idx[i] = offset;
    offset += hintv+1;
  }

  offset = 0;
  for(i=0;i<depth;i++) {
    dep_idx[i] = offset;
    offset += dintv+1;
  }

  offset=0;
  switch(stack->kind) {
  case GREY:
    for(k=0;k<depth;k++)
      for(i=0;i<height;i++)
	for(j=0;j<width;j++) {
	  stack2->array[offset] = 
	    (*STACK_PIXEL_8(stack,col_idx[j],row_idx[i],dep_idx[k],0));
	  offset++;
	}
    break;
  case GREY16:
    for (k = 0; k < depth; k++) {
      for(i=0;i<height;i++) {
	for(j=0;j<width;j++) {
	  stack2_array16[offset] = 
	    (*STACK_PIXEL_16(stack,col_idx[j],row_idx[i],dep_idx[k],0));
	  offset++;
	}
      }
    }
    break;
  case COLOR:
    for (k = 0; k < depth; k++) {
      for(i=0;i<height;i++) {
	for(j=0;j<width;j++) {
	  stack2_arrayc[offset][0] = 
	    (*STACK_PIXEL_8(stack,col_idx[j],row_idx[i],dep_idx[k],0));
	  stack2_arrayc[offset][1] = 
	    (*STACK_PIXEL_8(stack,col_idx[j],row_idx[i],dep_idx[k],1));
	  stack2_arrayc[offset][2] = 
	    (*STACK_PIXEL_8(stack,col_idx[j],row_idx[i],dep_idx[k],2));
	  offset++;
	}
      }
    }
    break;
  default:
    fprintf(stderr,"Downsample_Image: Unsupported image kind.\n");
  }

  free(col_idx);
  free(row_idx);
  free(dep_idx);

  return stack2;
}

#define DOWNSAMPLE_STACK_BLOCK_MEAN(stack_array, out_array, subwdith_t, subheight_t, subdepth_t, subarea_t, wndsize_t, validate_value) \
  s = 0;								\
  old_offset = offset;							\
  for(rk=0;rk<subdepth_t;rk++) {					\
    for(rj=0;rj<subheight_t;rj++) {					\
      for(ri=0;ri<subwdith_t;ri++) {					\
	/*printf("%d ", offset);*/					\
	s += stack_array[offset];					\
	offset++;							\
      }									\
      offset += width - subwdith_t;					\
    }									\
    offset += area - subheight_t * width;				\
  }									\
  s /= wndsize_t;							\
  validate_value(s);							\
  out_array[offset2] = s;						\
  offset2++;								\
  /*printf("\n");*/							\
  /*printf("\n%d %d %d\n", old_offset, offset, wndsize_t);*/		\
  offset = old_offset + subwdith_t;


//foreground blockmean
#define DOWNSAMPLE_STACK_BLOCK_MEAN_F(stack_array, out_array, subwdith_t, subheight_t, subdepth_t, subarea_t, wndsize_t, validate_value) \
{\
  int fgcount = 0;\
  s = 0;								\
  old_offset = offset;							\
  for(rk=0;rk<subdepth_t;rk++) {					\
    for(rj=0;rj<subheight_t;rj++) {					\
      for(ri=0;ri<subwdith_t;ri++) {					\
	/*printf("%d ", offset);*/					\
        if (stack_array[offset] > 0) {\
          ++fgcount;\
          s += stack_array[offset];					\
        }\
	offset++;							\
      }									\
      offset += width - subwdith_t;					\
    }									\
    offset += area - subheight_t * width;				\
  }									\
  if (fgcount > 0) {\
  s /= fgcount;							\
  }\
  validate_value(s);							\
  out_array[offset2] = s;						\
  offset2++;								\
  /*printf("\n");*/							\
  /*printf("\n%d %d %d\n", old_offset, offset, wndsize_t);*/		\
  offset = old_offset + subwdith_t;\
}

#define DOWNSAMPLE_STACK_BLOCK_MEAN_C(stack_array, out_array, subwdith_t, subheight_t, subdepth_t, subarea_t, wndsize_t, validate_value) \
  sc[0] = 0;								\
  sc[1] = 0;								\
  sc[2] = 0;								\
  old_offset = offset;							\
  for(rk=0;rk<subdepth_t;rk++) {					\
    for(rj=0;rj<subheight_t;rj++) {					\
      for(ri=0;ri<subwdith_t;ri++) {					\
	/*printf("%d ", offset);*/					\
	sc[0] += stack_array[offset][0];				\
	sc[1] += stack_array[offset][1];				\
	sc[2] += stack_array[offset][2];				\
	offset++;							\
      }									\
      offset += width - subwdith_t;					\
    }									\
    offset += area - subheight_t * width;				\
  }									\
  for (c = 0; c < 3; c++) {						\
    sc[c] /= wndsize_t;							\
    validate_value(s);							\
    out_array[offset2][c] = sc[c];					\
  }									\
  offset2++;								\
  /*printf("\n");*/							\
  /*printf("\n%d %d %d\n", old_offset, offset, wndsize_t);*/		\
  offset = old_offset + subwdith_t;


#define DOWNSAMPLE_STACK(stack_array, out_array, validate_value, downsample_stack_block_mean)	\
  for (k=0; k<kd; k++) {						\
    for (j=0; j<kh; j++) {						\
      for (i=0; i<kw; i++) {						\
	downsample_stack_block_mean(stack_array, out_array, subwidth, subheight, subdepth, subarea, wndsize, validate_value); \
      }									\
      if (rw > 0) {							\
	int tmp_subarea = rw * subheight;				\
	int tmp_wndsize = tmp_subarea * subdepth;			\
	downsample_stack_block_mean(stack_array, out_array, rw, subheight, subdepth, tmp_subarea, tmp_wndsize, validate_value); \
      }									\
      offset += width * (subheight - 1);				\
    }									\
									\
    if (rh > 0) {							\
      int tmp_subarea = rh * subwidth;					\
      int tmp_wndsize = tmp_subarea * subdepth;				\
      for (i=0; i<kw; i++) {						\
	downsample_stack_block_mean(stack_array, out_array, subwidth, rh, subdepth, tmp_subarea, tmp_wndsize, validate_value); \
      }									\
      if (rw > 0) {							\
	tmp_subarea = rh * rw;						\
	tmp_wndsize = tmp_subarea * subdepth;				\
	downsample_stack_block_mean(stack_array, out_array, rw, rh, subdepth, tmp_subarea, tmp_wndsize, validate_value); \
      }									\
      offset += width * (rh - 1);					\
    }									\
    offset += area * (subdepth - 1);					\
  }									\
									\
  if (rd > 0) {								\
    int tmp_subarea = subwidth * subheight;				\
    int tmp_wndsize = tmp_subarea * rd;					\
    for (j=0; j<kh; j++) {						\
      for (i=0; i<kw; i++) {						\
	downsample_stack_block_mean(stack_array, out_array, subwidth, subheight, rd, tmp_subarea, tmp_wndsize, validate_value); \
      }									\
      if (rw > 0) {							\
	tmp_subarea = rw * subheight;					\
	tmp_wndsize = tmp_subarea * rd;					\
	downsample_stack_block_mean(stack_array, out_array, rw, subheight, rd, tmp_subarea, tmp_wndsize, validate_value); \
      }									\
      offset += width * (subheight - 1);				\
    }									\
    if (rh > 0) {							\
      tmp_subarea = rh * subwidth;					\
      tmp_wndsize = tmp_subarea * rd;					\
      for (i=0;i<kw;i++) {						\
	downsample_stack_block_mean(stack_array, out_array, subwidth, rh, rd, tmp_subarea, tmp_wndsize, validate_value); \
      }									\
      if (rw > 0) {							\
	tmp_subarea = rh * rw;						\
	tmp_wndsize = tmp_subarea * rd;					\
	downsample_stack_block_mean(stack_array, out_array, rw, rh, rd, tmp_subarea, tmp_wndsize, validate_value); \
      }									\
      offset += width * (rh - 1);					\
    }									\
  }

Stack* Downsample_Stack_Mean(Stack* stack,int wintv,int hintv,int dintv, 
			     Stack *stack2)
{
  if ((wintv == 0) && (hintv == 0) && (dintv ==0)) {
    if (stack2 == NULL) {
      return Copy_Stack(stack);
    } else {
      stack2->kind = stack->kind;
      stack2->width = stack->width;
      stack2->height = stack->height;
      stack2->depth = stack->depth;
      Copy_Stack_Array(stack2, stack);
    }
  }

  int width = stack->width;
  int height = stack->height;
  size_t area = width * height;

  size_t old_offset = 0;
  size_t offset = 0;
  size_t offset2 = 0;

  int kw,kh,kd,rw,rh,rd;
  int i,j,k,ri,rj,rk;
  double s;
  double sc[3];
  int c;

  kw = stack->width / (wintv+1);
  kh = stack->height / (hintv+1);
  kd = stack->depth / (dintv+1);

  rw = stack->width % (wintv+1);
  rh = stack->height % (hintv+1);
  rd = stack->depth % (dintv+1);

  int subwidth = wintv + 1;
  int subheight = hintv + 1;
  int subdepth = dintv + 1;

  //int subarea = subwidth * subheight;
  int wndsize = subwidth * subheight * subdepth;
  
  if (stack2 == NULL) {
    stack2 = Make_Stack(stack->kind, kw + (rw>0), kh + (rh>0), 
			kd + (rd>0));
  }
  
  Image_Array ima;
  ima.array = stack->array;
  Image_Array out_ima;
  out_ima.array = stack2->array;
  
  switch (stack->kind) {
  case GREY:
    DOWNSAMPLE_STACK(ima.array8, out_ima.array8, VALIDATE_INTENSITY_GREY, DOWNSAMPLE_STACK_BLOCK_MEAN);
    break;
  case GREY16:
    DOWNSAMPLE_STACK(ima.array16, out_ima.array16, VALIDATE_INTENSITY_GREY16, DOWNSAMPLE_STACK_BLOCK_MEAN);
    break;
  case FLOAT32:
    DOWNSAMPLE_STACK(ima.array32, out_ima.array32, VALIDATE_INTENSITY_FLOAT32, DOWNSAMPLE_STACK_BLOCK_MEAN);
    break;
  case FLOAT64:
    DOWNSAMPLE_STACK(ima.array64, out_ima.array64, VALIDATE_INTENSITY_FLOAT64, DOWNSAMPLE_STACK_BLOCK_MEAN);
    break;
  case COLOR:
    DOWNSAMPLE_STACK(ima.arrayc, out_ima.arrayc, VALIDATE_INTENSITY_COLOR, DOWNSAMPLE_STACK_BLOCK_MEAN_C);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  stack2->width = kw + (rw>0);
  stack2->height = kh + (rh>0);
  stack2->depth = kd + (rd>0);

  return stack2;
}

Stack* Downsample_Stack_Mean_F(Stack* stack,int wintv,int hintv,int dintv, 
    Stack *stack2)
{
  if ((wintv == 0) && (hintv == 0) && (dintv ==0)) {
    if (stack2 == NULL) {
      return Copy_Stack(stack);
    } else {
      stack2->kind = stack->kind;
      stack2->width = stack->width;
      stack2->height = stack->height;
      stack2->depth = stack->depth;
      Copy_Stack_Array(stack2, stack);
    }
  }

  int width = stack->width;
  int height = stack->height;
  size_t area = width * height;

  size_t old_offset = 0;
  size_t offset = 0;
  size_t offset2 = 0;

  int kw,kh,kd,rw,rh,rd;
  int i,j,k,ri,rj,rk;
  double s;
  //double sc[3];
  //int c;

  kw = stack->width / (wintv+1);
  kh = stack->height / (hintv+1);
  kd = stack->depth / (dintv+1);

  rw = stack->width % (wintv+1);
  rh = stack->height % (hintv+1);
  rd = stack->depth % (dintv+1);

  int subwidth = wintv + 1;
  int subheight = hintv + 1;
  int subdepth = dintv + 1;

  //int subarea = subwidth * subheight;
  //int wndsize = subwidth * subheight * subdepth;
  
  if (stack2 == NULL) {
    stack2 = Make_Stack(stack->kind, kw + (rw>0), kh + (rh>0), 
			kd + (rd>0));
  }
  
  Image_Array ima;
  ima.array = stack->array;
  Image_Array out_ima;
  out_ima.array = stack2->array;
  
  switch (stack->kind) {
  case GREY:
    DOWNSAMPLE_STACK(ima.array8, out_ima.array8, VALIDATE_INTENSITY_GREY, DOWNSAMPLE_STACK_BLOCK_MEAN_F);
    break;
  case GREY16:
    DOWNSAMPLE_STACK(ima.array16, out_ima.array16, VALIDATE_INTENSITY_GREY16, DOWNSAMPLE_STACK_BLOCK_MEAN_F);
    break;
  case FLOAT32:
    DOWNSAMPLE_STACK(ima.array32, out_ima.array32, VALIDATE_INTENSITY_FLOAT32, DOWNSAMPLE_STACK_BLOCK_MEAN_F);
    break;
  case FLOAT64:
    DOWNSAMPLE_STACK(ima.array64, out_ima.array64, VALIDATE_INTENSITY_FLOAT64, DOWNSAMPLE_STACK_BLOCK_MEAN_F);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  stack2->width = kw + (rw>0);
  stack2->height = kh + (rh>0);
  stack2->depth = kd + (rd>0);

  return stack2;
}

#if 0
Stack* Downsample_Stack_Mean(Stack* stack,int wintv,int hintv,int dintv)
{
  int width = stack->width;
  int height = stack->height;
  int area = width * height;

  int offset = 0;
  int offset2 = 0;

  int kw,kh,kd,rw,rh,rd;
  int i,j,k,ri,rj,rk;
  double s;

  kw = stack->width / (wintv+1);
  kh = stack->height / (hintv+1);
  kd = stack->depth / (dintv+1);

  rw = stack->width % (wintv+1);
  rh = stack->height % (hintv+1);
  rd = stack->depth % (dintv+1);

  int subwidth = wintv + 1;
  int subheight = hintv + 1;
  int subdepth = dintv + 1;

  int subarea = subwidth * subheight;
  
  int wndsize = subwidth * subheight * subdepth;

  int subarea_r = rw * rh;
  int wndsize_r = subarea_r * rd;
  
  Stack* stack2 = Make_Stack(stack->kind, kw + (rw>0), kh + (rh>0), 
			     kd + (rd>0));
  
  for(k=0;k<kd;k++) {
    for(j=0;j<kh;j++) {
      for(i=0;i<kw;i++) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<=hintv;rj++)
	    for(ri=0;ri<=wintv;ri++)
	      s += Get_Stack_Pixel(stack,i*(wintv+1)+ri,j*(hintv+1)+rj,k*(dintv+1)+rk,0);
	Set_Stack_Pixel(stack2,i,j,k,0,(double)s/wndsize+0.5);
      }
      if(rw>0) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<=hintv;rj++)
	    for(ri=0;ri<rw;ri++)
	      s += Get_Stack_Pixel(stack,kw*(wintv+1)+ri,j*(hintv+1)+rj,k*(dintv+1)+rk,0);
	Set_Stack_Pixel(stack2,kw,j,k,0,(double)s/rw/(hintv+1)/(dintv+1)+0.5);
      }
    }
    if(rh>0) {
      for(i=0;i<kw;i++) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<rh;rj++)
	    for(ri=0;ri<=wintv;ri++)
	      s += Get_Stack_Pixel(stack,i*(wintv+1)+ri,kh*(hintv+1)+rj,k*(dintv+1)+rk,0);
	Set_Stack_Pixel(stack2,i,kh,k,0,(double)s/(wintv+1)/rh/(dintv+1)+0.5);
      }
      if(rw>0) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<rh;rj++)
	    for(ri=0;ri<rw;ri++)
	      s += Get_Stack_Pixel(stack,kw*(wintv+1)+ri,kh*(hintv+1)+rj,k*(dintv+1)+rk,0);
	Set_Stack_Pixel(stack2,kw,kh,k,0,(double)s/rw/rh/(dintv+1)+0.5);
      }
    }
    if(rd>0) {
      for(j=0;j<kh;j++) {
	for(i=0;i<kw;i++) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<=hintv;rj++)
	      for(ri=0;ri<=wintv;ri++)
		s += Get_Stack_Pixel(stack,i*(wintv+1)+ri,j*(hintv+1)+rj,kd*(dintv+1)+rk,0);
	  Set_Stack_Pixel(stack2,i,j,kd,0,(double)s/(wintv+1)/(hintv+1)/rd+0.5);
	}
	if(rw>0) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<=hintv;rj++)
	      for(ri=0;ri<rw;ri++)
		s += Get_Stack_Pixel(stack,kw*(wintv+1)+ri,j*(hintv+1)+rj,rd*(dintv+1)+rk,0);
	  Set_Stack_Pixel(stack2,kw,j,kd,0,(double)s/rw/(hintv+1)/rd+0.5);
	}
      }
      if(rh>0) {
	for(i=0;i<kw;i++) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<rh;rj++)
	      for(ri=0;ri<=wintv;ri++)
	      s += Get_Stack_Pixel(stack,i*(wintv+1)+ri,kh*(hintv+1)+rj,kd*(dintv+1)+rk,0);
	  Set_Stack_Pixel(stack2,i,kh,kd,0,(double)s/(wintv+1)/rd/rh+0.5);
	}
	if(rw>0) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<rh;rj++)
	      for(ri=0;ri<rw;ri++)
	      s += Get_Stack_Pixel(stack,kw*(wintv+1)+ri,kh*(hintv+1)+rj,kd*(dintv+1)+rk,0);
	  Set_Stack_Pixel(stack2,kw,kh,kd,0,(double)s/rw/rh/rd+0.5);
	}
      }
    }
  }

  return stack2;
}
#endif

void Downsample_Stack_Max_Size(int width, int height, int depth, 
    int wintv,int hintv,int dintv, int *wd, int *hd, int *dd)
{
  int kw,kh,kd,rw,rh,rd;
  kw = width / (wintv+1);
  kh = height / (hintv+1);
  kd = depth / (dintv+1);
  rw = width % (wintv+1);
  rh = height % (hintv+1);
  rd = depth % (dintv+1);

  *wd = kw + (rw > 0);
  *hd = kh + (rh > 0);
  *dd = kd + (rd > 0);
}

Stack* Downsample_Stack_Max(Stack* stack,int wintv,int hintv,int dintv, 
    Stack *out)
{
  double s;
  int kw,kh,kd,rw,rh,rd;
  int i,j,k,ri,rj,rk;

  kw = stack->width / (wintv+1);
  kh = stack->height / (hintv+1);
  kd = stack->depth / (dintv+1);
  rw = stack->width % (wintv+1);
  rh = stack->height % (hintv+1);
  rd = stack->depth % (dintv+1);

  //Stack* stack2 = Make_Stack(stack->kind,kw+(rw>0),kh+(rh>0),kd+(rd>0));
  if (out == NULL) {
    out = Make_Stack(stack->kind,kw+(rw>0),kh+(rh>0),kd+(rd>0));
  }
  
  for(k=0;k<kd;k++) {
    for(j=0;j<kh;j++) {
      for(i=0;i<kw;i++) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<=hintv;rj++)
	    for(ri=0;ri<=wintv;ri++)
	      s = dmax2(s, Get_Stack_Pixel(stack,i*(wintv+1)+ri,j*(hintv+1)+rj,k*(dintv+1)+rk,0));
	Set_Stack_Pixel(out,i,j,k,0,s);
      }
      if(rw>0) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<=hintv;rj++)
	    for(ri=0;ri<rw;ri++)
	      s = dmax2(s, Get_Stack_Pixel(stack,kw*(wintv+1)+ri,j*(hintv+1)+rj,k*(dintv+1)+rk,0));
	Set_Stack_Pixel(out,kw,j,k,0,s);
      }
    }
    if(rh>0) {
      for(i=0;i<kw;i++) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<rh;rj++)
	    for(ri=0;ri<=wintv;ri++)
	      s = dmax2(s, Get_Stack_Pixel(stack,i*(wintv+1)+ri,kh*(hintv+1)+rj,k*(dintv+1)+rk,0));
	Set_Stack_Pixel(out,i,kh,k,0,s);
      }
      if(rw>0) {
	s = 0;
	for(rk=0;rk<=dintv;rk++)
	  for(rj=0;rj<rh;rj++)
	    for(ri=0;ri<rw;ri++)
	      s = dmax2(s, Get_Stack_Pixel(stack,kw*(wintv+1)+ri,kh*(hintv+1)+rj,k*(dintv+1)+rk,0));
	Set_Stack_Pixel(out,kw,kh,k,0,s);
      }
    }
    if(rd>0) {
      for(j=0;j<kh;j++) {
	for(i=0;i<kw;i++) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<=hintv;rj++)
	      for(ri=0;ri<=wintv;ri++)
		s = dmax2(s, Get_Stack_Pixel(stack,i*(wintv+1)+ri,j*(hintv+1)+rj,kd*(dintv+1)+rk,0));
	  Set_Stack_Pixel(out,i,j,kd,0,s);
	}
	if(rw>0) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<=hintv;rj++)
	      for(ri=0;ri<rw;ri++)
		s = dmax2(s, Get_Stack_Pixel(stack,kw*(wintv+1)+ri,j*(hintv+1)+rj,rd*(dintv+1)+rk,0));
	  Set_Stack_Pixel(out,kw,j,kd,0,s);
	}
      }
      if(rh>0) {
	for(i=0;i<kw;i++) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<rh;rj++)
	      for(ri=0;ri<=wintv;ri++)
		s = dmax2(s, Get_Stack_Pixel(stack,i*(wintv+1)+ri,kh*(hintv+1)+rj,kd*(dintv+1)+rk,0));
	  Set_Stack_Pixel(out,i,kh,kd,0,s);
	}
	if(rw>0) {
	  s = 0;
	  for(rk=0;rk<rd;rk++)
	    for(rj=0;rj<rh;rj++)
	      for(ri=0;ri<rw;ri++)
		s = dmax2(s, Get_Stack_Pixel(stack,kw*(wintv+1)+ri,kh*(hintv+1)+rj,kd*(dintv+1)+rk,0));
	  Set_Stack_Pixel(out,kw,kh,kd,0,s);
	}
      }
    }
  }

  return out;
}

#define UPSAMPLE_STACK(stack_array, out_array)				\
  /* interpolate rows */						\
  for (k = 0; k < stack->depth; k++) {					\
    for (j = 0; j < stack->height; j++) {				\
      for (i = 0; i < cwidth; i++) {					\
	out_array[offset2++] = stack_array[offset];			\
	step = stack_array[offset + 1];					\
	step -= stack_array[offset];					\
	step /= wintv + 1;						\
	step2 = step;							\
	for (u = 1; u <= wintv; u++) {					\
	  out_array[offset2++] = stack_array[offset] + step;		\
	  step += step2;						\
	}								\
	offset++;							\
      }									\
      									\
      for (u = 0; u <= wintv; u++) {					\
	out_array[offset2++] = stack_array[offset];			\
      }									\
      offset++;								\
      offset2 += out->width * hintv;					\
    }									\
    offset2 += out_area * dintv;					\
  }									\
  									\
  offset = 0;								\
  offset2 = 0;								\
									\
  /* interpolate columns */						\
  if (hintv >= 1) {							\
    for (k = 0; k < stack->depth; k++) {				\
      for (j = 0; j < cheight; j++) {					\
	for (i = 0; i < out->width; i++) {				\
	  offset3 = offset2 + out->width;				\
	  step = out_array[offset3 + out->width * hintv];		\
	  step -= out_array[offset2];					\
	  step /= hintv + 1;						\
	  step2 = step;							\
	  for (u = 1; u <= hintv; u++) {				\
	    out_array[offset3] = out_array[offset2] + step;		\
	    offset3 += out->width;					\
	    step += step2;						\
	  }								\
	  offset2++;							\
	}								\
	offset2 += out->width * hintv;					\
      }									\
    									\
      offset3++;							\
      for (u = 1; u <= hintv; u++) {					\
	for (i = 0; i < out->width; i++) {				\
	  out_array[offset3] = out_array[offset3 - out->width];		\
	  offset3++;							\
	}								\
      }									\
      offset2 += out->width * (hintv + 1);				\
      offset2 += out_area * dintv;					\
    }									\
  }									\
  									\
  offset = 0;								\
  offset2 =0;								\
  offset3 = 0;								\
  									\
  /* interpolate slices */						\
  if (dintv >= 1) {							\
    for (k = 0; k < cdepth; k++) {					\
      for (j = 0; j < out->height; j++) {				\
	for (i = 0; i < out->width; i++) {				\
	  offset3 = offset2 + out_area;					\
	  step = out_array[offset3 + out_area * dintv];			\
	  step -= out_array[offset2];					\
	  step /= dintv + 1;						\
	  step2 = step;							\
	  for (u = 1; u <= dintv; u++) {				\
	    out_array[offset3] = out_array[offset2] + step;		\
	    offset3 += out_area;					\
	    step += step2;						\
	  }								\
	  offset2++;							\
	}								\
      }									\
      offset2 += out_area * dintv;					\
    }									\
  }									\
  									\
  offset3++;								\
  for (u = 1; u <= dintv; u++) {					\
    for (j = 0; j < out->height; j++) {					\
      for (i = 0; i < out->width; i++) {				\
	out_array[offset3] = out_array[offset3 - out_area];		\
	offset3++;							\
      }									\
    }									\
  }

Stack* Upsample_Stack(const Stack *stack, int wintv, int hintv, int dintv,
		      Stack *out)
{
  if (out == NULL) {
    out = Make_Stack(stack->kind, stack->width * (wintv + 1),
		     stack->height * (hintv + 1), stack->depth * (dintv + 1));
  }

  int i, j, k;
  int offset = 0;
  int offset2 = 0;
  int u;

  Image_Array ima;
  ima.array = stack->array;

  Image_Array out_ima;
  out_ima.array = out->array;

  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  int out_area = out->width * out->height;

  double step;
  double step2;
  int offset3 = 0;

  switch (stack->kind) {
  case GREY:
    UPSAMPLE_STACK(ima.array8, out_ima.array8);
    break;
  case GREY16:
    UPSAMPLE_STACK(ima.array16, out_ima.array16);
    break;
  case FLOAT32:
    UPSAMPLE_STACK(ima.array32, out_ima.array32);
    break;
  case FLOAT64:
    UPSAMPLE_STACK(ima.array64, out_ima.array64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }
  
  return out;
}

/* Resample_Stack_Depth(): Resample a stack along depth.
 *
 * Args: stack - input stack;
 *       stack2 - output stack;
 *       dintv - resampling interval.
 *
 * Return: NULL if the function it is failed. Otherwise it returns the output 
 *         stack.
 */
#define RESAMPLE_STACK_DEPTH(src_array, src_array_start, src_array_next, dst_array, trtype) \
  for (k = 0; k < dst->depth; k++) {					\
    slice  = (int) (floor(z));						\
    lamda = z - floor(z);						\
									\
    /* if the slice is out of range, estimate the interpolation	*/	\
    /* based on the last two slices*/					\
    if (slice >= src->depth - 1) {					\
      lamda += 1.0;							\
      slice--;								\
    }									\
									\
    src_array = src_array_start + slice * area;				\
    src_array_next = src_array + area;					\
									\
    for (j = 0; j < area; j++) {					\
      value = (int)((double) src_array[j] +				\
		    lamda * ((trtype) src_array_next[j] -		\
			     (trtype) src_array[j]));			\
      if(value < 0) {							\
	dst_array[offset] = 0;						\
      } else if (value > max_grey){					\
	dst_array[offset] = max_grey;					\
      } else {								\
	dst_array[offset] = value;					\
      }									\
      offset++;								\
    }									\
    z += dintv;								\
  }
  
Stack* Resample_Stack_Depth(const Stack* src, Stack *dst, double dintv)
{
  if (src == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    return NULL;
  }

  if (dintv <= 0.0) {
    perror("Invalid resampling interval.");
    THROW(ERROR_DATA_VALUE);
  }

  if (dst == NULL) {
    int depth = (int) (floor((src->depth - 1) / dintv)) + 1;
    if (depth > STACK_DEPTH_MAX) {
      perror("The stack depth is out of range.");
      THROW(ERROR_DATA_VALUE);
    }

    dst = Make_Stack(src->kind, src->width, src->height, depth);
  } else {
    if (Stack_Same_Kind(src, dst) == 0) {
      perror("The two stacks must have the same kind");
      THROW(ERROR_DATA_COMPTB);
    }
  }

  DEFINE_SCALAR_ARRAY_ALL(src_array, src);
  DEFINE_SCALAR_ARRAY_ALL(src_array_start, src);
  DEFINE_SCALAR_ARRAY_ALL(src_array_next, src);
  DEFINE_SCALAR_ARRAY_ALL(dst_array, dst);

  int j, k;
  int area = src->width * src->height;

  double z = dintv;
  int slice = 0;
  int offset = 0;
  int value = 0;
  double lamda = 0.0;
  int max_grey = MAXGREY(dst->kind);

  switch (src->kind) {
  case GREY:
    RESAMPLE_STACK_DEPTH(src_array_grey, src_array_start_grey, 
			 src_array_next_grey, dst_array_grey, int);
    break;
  case GREY16:
    RESAMPLE_STACK_DEPTH(src_array_grey16, src_array_start_grey16, 
			 src_array_next_grey16, dst_array_grey16, int);
    break;
  case FLOAT32:
    RESAMPLE_STACK_DEPTH(src_array_float32, src_array_start_float32, 
			 src_array_next_float32, dst_array_float32, float);
    break;
  case FLOAT64:
    RESAMPLE_STACK_DEPTH(src_array_float64, src_array_start_float64, 
			 src_array_next_float64, dst_array_float64, double);
    break;
  default:
    perror("Unsupported image kind.");
    TZ_ERROR(ERROR_DATA_VALUE);
    break;
  }

  return dst;
}

/**
 * Get the overlap region of two stacks. start1,end1 is the region for the
 * moving image. start2,end2 is the region for the fixed image.
 */
void Stack_Overlap(const int dim[],const int bdim[],const int offset[],int start1[],int end1[],int start2[],int end2[])
{
  int i;
  for(i=0;i<3;i++) {
    start1[i] = imax2(0,bdim[i]-1-offset[i]);
    end1[i] = imin2(bdim[i]-1,bdim[i]+dim[i]-2-offset[i]);
    //end1[i] = imin2(bdim[i]-1,bdim[i]+dim[i]-1-offset[i]);
    start2[i] = imax2(0,offset[i]-bdim[i]+1);
    //start2[i] = imax2(0,offset[i]-bdim[i]);
    end2[i] = imin2(dim[i]-1,offset[i]);
  }
}


int* Stack_Hist(const Stack *stack)
{
  return Image_Array_Hist(stack->array, stack->kind, Get_Stack_Size(stack),
			  NULL);
}

int* Stack_Hist_M(const Stack *stack, Stack *mask)
{
  if (mask == NULL) {
    return Stack_Hist(stack);
  } else {
    if (mask->kind != GREY) {
      TZ_ERROR(ERROR_DATA_TYPE);
    }
    return Image_Array_Hist_M(stack->array, stack->kind, Get_Stack_Size(stack),
			      mask->array, NULL);
  }
}

size_t Get_Stack_Size(const Stack* stack)
{
  return ((size_t) stack->width) * ((size_t) stack->height) * 
    ((size_t) stack->depth);
}


#define STACK_BINARIZE(stack_array)		\
  for (i = 0; i < voxel_number; i++) {		\
    stack_array[i] = stack_array[i] > 0;	\
  }

void Stack_Binarize(Stack* stack) 
{ 
  if (stack == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    return;
  }

  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  int voxel_number = Stack_Voxel_Number(stack);
  size_t i;

  switch(stack->kind) {
  case GREY:
    STACK_BINARIZE(array_grey);
    break;
  case GREY16:
    STACK_BINARIZE(array_grey16);
    break;
  case FLOAT32:
    STACK_BINARIZE(array_float32);
    break;
  case FLOAT64:
    STACK_BINARIZE(array_float64);
    break;
  default:
    perror("Unrecongnzied stack kind.");
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

#define STACK_BINARIZE_LEVEL(stack_array, type)		\
  for (i = 0; i < voxel_number; i++) {		\
    stack_array[i] = (stack_array[i] == (type) level);	\
  }

void Stack_Binarize_Level(Stack* stack, double level) 
{ 
  if (stack == NULL) {
    TZ_WARN(ERROR_POINTER_NULL);
    return;
  }

  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  int voxel_number = Stack_Voxel_Number(stack);
  size_t i;

  switch(stack->kind) {
  case GREY:
    STACK_BINARIZE_LEVEL(array_grey, uint8_t);
    break;
  case GREY16:
    STACK_BINARIZE_LEVEL(array_grey16, uint16_t);
    break;
  case FLOAT32:
    STACK_BINARIZE_LEVEL(array_float32, float);
    break;
  case FLOAT64:
    STACK_BINARIZE_LEVEL(array_float64, double);
    break;
  default:
    perror("Unrecongnzied stack kind.");
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}
#undef STACK_BINARIZE_LEVEL

/* Find the most common value from a histogram.
 * <low> and <high> is the range of searching the value.
 * If you want to search the full range, you can set <low> 
 * to <hist[1]> and <high> to <hist[0]+hist[1]-1>.
 */
int Hist_Most_Common(int *hist,int low,int high)
{
  size_t common;
  int *hist2 = hist+2;
  int length = hist[0];
  int mingrey = hist[1];
  int maxgrey = hist[0]+hist[1]-1;
  
  if(low>mingrey) {
    hist2 += low-mingrey;
    mingrey = low;
  }

  if(high<maxgrey)
    maxgrey = high;

  length = high - low + 1;

  iarray_max(hist2,length,&common);

  common += mingrey;

  return common;
}

void Stack_Sub_Common(Stack* stack,int low,int high)
{
  int *hist = Stack_Hist(stack);
  int thre = Hist_Most_Common(hist,low,high);
#ifdef _DEBUG_
  printf("%d\n", thre);
#endif
  Stack_Subc(stack,thre+1);
  free(hist);
}


/**
 * Correlation coefficient between two stacks.
 * stack1 and stack2 should have the same size and kind.
 */
double Stack_Corrcoef(Stack *stack1,Stack *stack2)
{
  size_t length = Get_Stack_Size(stack1);
  double mu1 = Stack_Mean(stack1);
  double mu2 = Stack_Mean(stack2);
  double r,v1,v2;
  double sd1,sd2; 
  size_t i;
  uint16 *array16_1 = (uint16 *)stack1->array;
  uint16 *array16_2 = (uint16 *)stack2->array;

  r = v1 = v2 = 0.0;

  switch(stack1->kind) {
  case GREY:
    for(i=0;i<length;i++) {
      sd1 = (double)stack1->array[i] - mu1;
      sd2 = (double)stack2->array[i] - mu2;
      r += sd1*sd2;
      v1 += sd1*sd1;
      v2 += sd2*sd2;
    }
    break;
  case GREY16:
    for(i=0;i<length;i++) {
      sd1 = (double)array16_1[i] - mu1;
      sd2 = (double)array16_2[i] - mu2;
      r += sd1*sd2;
      v1 += sd1*sd1;
      v2 += sd2*sd2;
    }
    break;
  default:
    fprintf(stderr,"Stack_Corrcoef: Unsupported image kind.\n");
  }

  return r/sqrt(v1*v2);
}

/* Stack_Locmax(): Find local maxima in a stack. 
 *
 * Note: The caller is responsible for  clearing the returned stack.
 * 
 * Args: stack - input stack.
 *       stack_out - output stack. A new stack will be created if it is NULL.
 *                   The should be no aliasing between stack and stack_out.
 *                   The kind of stack_out should be GREY.
 * Return: a new stack containing information of local maxima.
 */
Stack* Stack_Locmax(Stack *stack, Stack *stack_out)
{
  if (stack == NULL)
    return NULL;

  if (stack->width <=1 || stack->height<=1 || stack->depth<=1) {
    printf("Each dimension of the stack should be at least 2.");
    return stack;
  }

  int i,j,k;
  int offset = 0;
  int planeOffset = stack->width * stack->height;
  int doublePlaneOffset = planeOffset * 2;
  uint16 *array16 = NULL; 
  float32 *array32 = NULL;

  if (stack_out == NULL)
    stack_out = Make_Stack(GREY,stack->width,stack->height,stack->depth+2);

  if ((stack_out->width != stack->width) || 
      (stack_out->height != stack->height) ||
      (stack_out->depth - 2 != stack->depth) ||
      (stack_out->kind != GREY))
    TZ_ERROR(ERROR_DATA_COMPTB);

  /* Initialization */
  /* clean first plane */
  for (offset=0; offset<planeOffset; offset++)
    stack_out->array[offset] = 2;
    
  for (k=1; k<stack->depth-1; k++) {
    /* clean first row */
    for (i=0; i<stack->width; i++)
      stack_out->array[offset++] = 2;

    for (j=1; j<stack->height-1; j++) {
      stack_out->array[offset++] = 2; /* clean first column*/
      for (i=1; i<stack->width-1; i++) {
	stack_out->array[offset++] = 1;
      }
      stack_out->array[offset++] = 2; /* clean last column*/
    }

    /* clean last row */
    for (i=0; i<stack->width; i++)
      stack_out->array[offset++] = 2;
  }

  /* clean last slice */
  for (i=0; i<planeOffset; i++)
    stack_out->array[offset++] = 2;

  /* clean last two additional slices */
  for (i=0; i<doublePlaneOffset; i++)
    stack_out->array[offset++] = 0;

  offset = 0;

  int nbOffset; /* offset */
  int neighbor[13];
  int ncon = 13;
  int c;
    
  neighbor[0] = 1; /* x-directon */
  neighbor[1] = stack->width; /* y-direction */
  neighbor[2] = planeOffset; /* z-direction */
  neighbor[3] = stack->width + 1;; /* x-y diagonal */
  neighbor[4] = stack->width - 1; /* x-y counterdiagonal */
  neighbor[5] = planeOffset + 1; /* x-z diagonal */
  neighbor[6] = planeOffset - 1; /* x-z counter diaagonal */
  neighbor[7] = planeOffset + stack->width; /* y-z diagonal */
  neighbor[8] = planeOffset - stack->width; /* y-z counter diaagonal */
  neighbor[9] = planeOffset + stack->width + 1; /* x-y-z diagonal */
  neighbor[10] = planeOffset + stack->width - 1; /* x-y-z diagonal -x*/
  neighbor[11] = planeOffset - stack->width + 1; /* x-y-z diagonal -y*/
  neighbor[12] = planeOffset - stack->width - 1; /* x-y-z diagonal -x -y*/

  switch (stack->kind) {
  case GREY:
    /* finding local maxima */
    for (k=0; k<stack->depth; k++) {
      for (j=0; j<stack->height; j++) {
	for (i=0; i<stack->width; i++) {
	  if (stack->array[offset] > 0) {
	    for (c=0; c<ncon; c++) {
	      nbOffset = offset + neighbor[c];
	      if (stack->array[offset] < stack->array[nbOffset]) {
		stack_out->array[offset] = 0;
	      } else if (stack->array[offset] > stack->array[nbOffset]) {
		stack_out->array[nbOffset] = 0;
	      }
	    } 
	  } else {
	    stack_out->array[offset] = 0;
	  } /* if (stack->array[offset] > 0) */
	  offset++;
	}
      }
    }
    break;
  case GREY16:
    array16 = (uint16 *) stack->array;
    /* finding local maxima */
    for (k=0; k<stack->depth; k++) {
      for (j=0; j<stack->height; j++) {
	for (i=0; i<stack->width; i++) {
	  if (array16[offset] > 0) {
	    for (c=0; c<ncon; c++) {
	      nbOffset = offset + neighbor[c];
	      if (array16[offset] < array16[nbOffset]) {
		stack_out->array[offset] = 0;
	      } else if (stack->array[offset] > stack->array[nbOffset]) {
		stack_out->array[nbOffset] = 0;
	      } 
	    } 
	  } else {
	    stack_out->array[offset] = 0;
	  } /* if (stack->array[offset] > 0) */
	  offset++;
	}
      }
    }
    break;
  case FLOAT32:
    array32 = (float32 *) stack->array;
    /* finding local maxima */
    for (k=0; k<stack->depth; k++) {
      for (j=0; j<stack->height; j++) {
	for (i=0; i<stack->width; i++) {
	  if (array32[offset] > 0.5) {
	    for (c=0; c<ncon; c++) {
	      nbOffset = offset + neighbor[c];
	      if (array32[offset] < array32[nbOffset]) {
		stack_out->array[offset] = 0;
	      } else if (stack->array[offset] > stack->array[nbOffset]) {
		stack_out->array[nbOffset] = 0;
	      } 
	    } 
	  } else {
	    stack_out->array[offset] = 0;
	  } /* if (stack->array[offset] > 0) */
	  offset++;
	}
      }
    }
    break;
  default:
    fprintf(stderr,"Stack_Locmax: Unsupported image kind.\n");
  }

  return stack_out;
}


/*
 */
Stack *Stack_Local_Max(const Stack *stack, Stack *stack_out, int option)
{
  if (stack == NULL)
    return NULL;

  int i,j,k;
  size_t offset = 0;
  int area = stack->width * stack->height;

  if (stack_out == NULL) {
    stack_out = Make_Stack(GREY,stack->width,stack->height,stack->depth);
  }

  if (!Stack_Same_Size(stack, stack_out) || (stack_out->kind != GREY)) {
    TZ_ERROR(ERROR_DATA_COMPTB);
  }

  One_Stack(stack_out);
  
  int boundary_size = area;
  if ((stack->width < stack->depth) || (stack->height < stack->depth)){
    if (stack->height < stack->width) {
      boundary_size = stack->width * stack->depth;
    } else {
      boundary_size = stack->height * stack->depth;
    }
  }

  int nboffset; /* neighbor offset */
  int ncon = 13;
  int *neighbor = (int*)malloc(sizeof(int)*ncon);
  int c;
  int *mask = (int*)malloc(sizeof(int)*ncon);
  int id;
  int *boundary = iarray_malloc(boundary_size);
  int nbr;

  uint8 *array_out = (uint8 *) stack_out->array;
  DEFINE_SCALAR_ARRAY_ALL(array, stack);

  init_scan_array(stack->width, stack->height, neighbor);

  /*********** macro for different kind ******************/
#define STACK_LOCAL_MAX(array)			\
  /* process boundaries first */		\
    for (id = 1; id <= 26;  id++) {		\
      copy_scan_array_mask(id, mask);		\
      if (stack->depth == 1) {			\
	scan_array_mask_depth(mask);		\
      }						\
      if (stack->height == 1) {			\
	scan_array_mask_height(mask);		\
      }						\
      if (stack->width == 1) {			\
	scan_array_mask_width(mask);		\
      }						\
									\
      nbr = boundary_offset(stack->width, stack->height, stack->depth,	\
			    id, boundary);				\
      for (i = 0; i < nbr; i++) {					\
	offset = boundary[i];						\
	if (array[offset] == 0) {					\
	  array_out[offset] = 0;					\
	}								\
	for (c=0; c<ncon; c++) {					\
	  if (mask[c] == 1) {						\
	    ARRAY_CMP(array, array_out, offset, c,			\
		      nboffset, neighbor, option);			\
	  }								\
	}								\
      }									\
    }									\
									\
    /* finding local maxima for internal voxels*/			\
    offset = area + stack->width + 1;					\
									\
    /*PROGRESS_BEGIN("Finding local maxima");*/				\
    for (k=1; k<stack->depth - 1; k++) {				\
      /*PROGRESS_STATUS(k * 100/ (stack->depth - 1));*/			\
      for (j=1; j<stack->height - 1; j++) {				\
	for (i=1; i<stack->width - 1; i++) {				\
	  if (array[offset] == 0) {					\
	    array_out[offset] = 0;					\
	  }								\
	  for (c=0; c<ncon; c++) {					\
	    ARRAY_CMP(array, array_out, offset, c,			\
		      nboffset, neighbor, option);			\
	  }								\
	  offset++;							\
	}								\
	offset += 2;							\
      }									\
      offset += stack->width * 2;					\
      /*PROGRESS_REFRESH;*/						\
    }									\
    /*PROGRESS_END("done");*/
  /*************************************/

  switch (stack->kind) {
  case GREY:
    STACK_LOCAL_MAX(array_grey);
    break;
  case GREY16:
    STACK_LOCAL_MAX(array_grey16);
    break;
  case FLOAT32:
    STACK_LOCAL_MAX(array_float32);
    break;
  case FLOAT64:
    STACK_LOCAL_MAX(array_float64);
    break;
  case COLOR:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  free(boundary);
  free(neighbor);
  free(mask);

#undef STACK_LOCAL_MAX

  return stack_out;
}

Stack* Stack_Channel_Extraction(const Stack *stack, int channel, Stack *out)
{
  if (stack == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  if (Stack_Channel_Number(stack) == 1) {
    if (stack != out) {
      if (out == NULL) {
        return Copy_Stack((Stack *) stack);
      } else {
        if (Stack_Same_Attribute(stack, out) == FALSE) {
          TZ_ERROR(ERROR_DATA_TYPE);
        } else {
          Copy_Stack_Array(out, stack);
        }
      }
    }
  } else {
    /* only this kind is supported for multi-channel image in this version */
    if (stack->kind != COLOR) {
      TZ_ERROR(ERROR_DATA_TYPE);
    }

    if (out == NULL) {
      out = Make_Stack(GREY, stack->width, stack->height, stack->depth);
    } else {
      if (out->kind != GREY) {
	TZ_ERROR(ERROR_DATA_TYPE);
      }

      if (Stack_Same_Size(stack, out) == FALSE) {
	TZ_ERROR(ERROR_DATA_TYPE);
      }
    }

    if (stack == out) {
      out->kind = GREY;
    } 

    if ((stack != out) || (channel != 0)) {
      color_t *array = (color_t *) stack->array;
      int i;
      int length = Stack_Voxel_Number(stack);

      for (i = 0; i < length; i++) {
        out->array[i] = array[i][channel];
      }
    }
  }

  return out;
}


void Empty_Stack(Stack *stack)
{
  if (stack == NULL) {
    return;
  }

  stack->kind = 0;
  stack->width = 0;
  stack->height = 0;
  stack->text = (char *) malloc(1);
  stack->text[0] = '\0';
  stack->array = NULL;
}

BOOL Is_Stack_Empty(const Stack *stack)
{
  return Stack_Is_Empty(stack);
}


void Stack_Clean_Locmax(Stack *stack, Stack *locmax)
{
  ASSERT(stack->kind == GREY16, "the first stack must be grey16 kind");
  ASSERT(locmax->kind == GREY, "the first stack must be grey kind");
  
  int i;
  int nvoxel = Stack_Voxel_Number(stack);
  int neighbor[26];
  Stack_Neighbor_Offset(26, stack->width, stack->height, neighbor);
  int cwidth = stack->width - 1;
  int cheight = stack->height- 1;
  int cdepth = stack->depth - 1;
  int x, y, z;
  size_t offset = 0;
  int is_in_bound[26];
  uint16 *stack_array = (uint16 *) stack->array;

  for (z = 0; z < stack->depth; z++) {
    for (y = 0; y < stack->height; y++) {
      for (x = 0; x < stack->width; x++) {
	if (locmax->array[offset] == 1) {
	  int nbound = Stack_Neighbor_Bound_Test(26, cwidth, cheight, cdepth, 
						 x, y, z, is_in_bound);
	  
	  if (nbound < 26) {
	    locmax->array[offset] = 2;
	  } else {
	    for (i = 0; i < 26; i++) {
	      int q = offset + neighbor[i];
	      ASSERT(stack_array[q] <= stack_array[offset],
		     "not loc max");
	      if ((stack_array[q] == stack_array[offset]) && 
		  (locmax->array[q] == 0)) {
		locmax->array[offset] = 2;
	      }
	    }
	  } /* end else */
	} /* end locmax->array[offset] == 1 */
	offset++;
      }
    }
  }

  for (i = 0; i < nvoxel; i++) {
    if (locmax->array[i] == 2) {
      locmax->array[i] = 0;
    }
  }
}

#define STACK_INVERT_VALUE(stack_array)			\
  for (i = 0; i < nvoxel; i++) {			\
    stack_array[i] = min + max - stack_array[i];	\
  }

void Stack_Invert_Value(Stack *stack)
{
  int nvoxel = Stack_Voxel_Number(stack);
  double min = Stack_Min(stack, NULL);
  double max = Stack_Max(stack, NULL);
  Image_Array ima;
  ima.array = stack->array;
  int i;
  switch (stack->kind) {
  case GREY:
    STACK_INVERT_VALUE(ima.array);
    break;
  case GREY16:
    STACK_INVERT_VALUE(ima.array16);
    break;
  case FLOAT32:
    STACK_INVERT_VALUE(ima.array32);
    break;
  case FLOAT64:
    STACK_INVERT_VALUE(ima.array64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

double Stack_Pixel(const Stack *stack, int x, int y, int z, int c)
{
  if (Stack_Contain_Point(stack, x, y, z) == FALSE) {
    return -1.0;
  }

  return Get_Stack_Pixel((Stack *) stack, x, y, z, c);
}

Stack *Stack_Merge(const Stack *stack1, const Stack *stack2, 
		   int x_offset, int y_offset, int z_offset, int option)
{
  int width = imax2(stack1->width, stack2->width + x_offset) - 
    imin2(0, x_offset);
  int height = imax2(stack1->height, stack2->height + y_offset) - 
    imin2(0, y_offset);
  int depth = imax2(stack1->depth, stack2->depth + x_offset) - 
    imin2(0, z_offset);

  printf("%d %d %d\n", width, height, depth);

  //int start1[3] = {0, 0, 0}; 
  int start2[3] = {0, 0, 0};
  //int end1[3];
  int end2[3];

  if (x_offset > 0) {
    start2[0] = x_offset;
  }

  if (y_offset > 0) {
    start2[1] = y_offset;
  }

  if (z_offset > 0) {
    start2[2] = z_offset;
  }

  Stack *new_stack = Crop_Stack(stack1, imin2(0, x_offset), imin2(0, y_offset),
				imin2(0, z_offset),
				width, height, depth, NULL);

  /*
  end1[0] = start1[0] + stack1->width - 1;
  end1[1] = start1[1] + stack1->height - 1;
  end1[2] = start1[2] + stack1->depth - 1;
  */
  end2[0] = start2[0] + stack2->width - 1;
  end2[1] = start2[1] + stack2->height - 1;
  end2[2] = start2[2] + stack2->depth - 1;


  int offset = 0;
  //int offset1 = 0;
  int offset2 = 0;
  
  printf("Merging %d %d %d...\n", start2[0], start2[1], start2[2]);

  int i, j, k;
  for (k = 0; k < new_stack->depth; k++) {
    for (j = 0; j < new_stack->height; j++) {
      for (i = 0; i < new_stack->width; i++) {
	if (IS_IN_CLOSE_RANGE3(i, j, k, start2[0], end2[0], 
			       start2[1], end2[1], start2[2], end2[2])) {
	  new_stack->array[offset] = MAX2(new_stack->array[offset], 
					  stack2->array[offset2]);
	  offset2++;
	}
	  
	offset++;
      }
    }
  }

  return new_stack;
}

#define STACK_MERGE_M(new_stack_array, type)				\
  {									\
    if (np > 0) {							\
      if (new_stack->kind != COLOR) {					\
	switch (option) {						\
	case 1: /* max */						\
	  new_stack_array[new_index] = (type) darray_max(p[0], np, NULL); \
	  break;							\
	case 2: /* min */						\
	  new_stack_array[new_index] = (type) darray_min(p[0], np, NULL); \
	  break;							\
	case 3: /* mean */						\
	  new_stack_array[new_index] = (type) darray_mean(p[0], np);	\
	  break;							\
	case 4: /* first */						\
	  new_stack_array[new_index] = (type) p[0][0];			\
	  break;							\
	default:							\
	  TZ_ERROR(ERROR_DATA_VALUE);					\
	}								\
      } else {								\
	switch (option) {						\
	case 1: /* max */						\
	  ima.arrayc[new_index][0] = (type) darray_max(p[0], np, NULL); \
	  ima.arrayc[new_index][1] = (type) darray_max(p[1], np, NULL); \
	  ima.arrayc[new_index][2] = (type) darray_max(p[2], np, NULL); \
	  break;							\
	case 2: /* min */						\
	  ima.arrayc[new_index][0] = (type) darray_min(p[0], np, NULL); \
	  ima.arrayc[new_index][1] = (type) darray_min(p[1], np, NULL); \
	  ima.arrayc[new_index][2] = (type) darray_min(p[2], np, NULL); \
	  break;							\
	case 3: /* mean */						\
	  ima.arrayc[new_index][0] = (type) darray_mean(p[0], np);	\
	  ima.arrayc[new_index][1] = (type) darray_mean(p[1], np);	\
	  ima.arrayc[new_index][2] = (type) darray_mean(p[2], np);	\
	  break;							\
	case 4: /* first */						\
	  ima.arrayc[new_index][0] = (type) p[0][0];			\
	  ima.arrayc[new_index][1] = (type) p[1][0];			\
	  ima.arrayc[new_index][2] = (type) p[2][0];			\
	  break;							\
	default:							\
	  TZ_ERROR(ERROR_DATA_VALUE);					\
	}								\
      }									\
    }									\
    new_index++;							\
  }	

Stack *Stack_Merge_M(Stack **stack, int n, int **offset, int option, 
		     Stack *new_stack)
{
  int *start[3];
  int *end[3];
  int i, j;
  /* alloc <start> <end> */
  for (i = 0; i < 3; i++) {
    start[i] = iarray_malloc(n);
    end[i] = iarray_malloc(n);
  }
  
  for (i = 0; i < n; i++) {
    for (j = 0; j < 3; j++) {
      start[j][i] = offset[i][j];
    }
  }

  int corner_offset[3];
  size_t min_idx[3];
  for (i = 0; i < 3; i++) {
    corner_offset[i] = iarray_min(start[i], n, min_idx + i);
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < 3; j++) {
      start[j][i] -= corner_offset[j];
    }
    end[0][i] = start[0][i] + stack[i]->width - 1;
    end[1][i] = start[1][i] + stack[i]->height - 1;
    end[2][i] = start[2][i] + stack[i]->depth - 1;
  }

  int width = iarray_max(end[0], n, NULL) - start[0][min_idx[0]] + 1;
  int height = iarray_max(end[1], n, NULL) - start[1][min_idx[1]] + 1;
  int depth = iarray_max(end[2], n, NULL) - start[2][min_idx[2]] + 1;

  printf("%d %d %d\n", width, height, depth);
  
  /*
  Stack *new_stack = Crop_Stack(stack[0], -start[0][0], -start[1][0],
				-start[2][0],
				width, height, depth, NULL);
  */
  
  if (new_stack == NULL) {
    new_stack = Make_Stack(stack[0]->kind, width, height, depth);
  }
  
  /* alloc <index> */
  size_t new_index = 0;
  size_t *index;// = iarray_calloc(n);
  GUARDED_CALLOC_ARRAY(index, n, size_t);
  
  printf("Merging ...\n");

  int k, v;
  
  /* alloc <p> */
  double *p[3];
  
  p[0] = darray_malloc(n);

  if (new_stack->kind == COLOR) {
    p[1] = darray_malloc(n);
    p[2] = darray_malloc(n);
  } else {
    p[1] = NULL;
    p[2] = NULL;
  }

  Image_Array ima;
  ima.array = new_stack->array;

  /* allocate <tile_arrays> */
  Image_Array *tile_arrays = (Image_Array *) malloc(sizeof(Image_Array) * n);
  for (v = 0; v < n; v++) {
    tile_arrays[v].array = stack[v]->array;
  }

  int np;

  for (k = 0; k < new_stack->depth; k++) {
    for (j = 0; j < new_stack->height; j++) {
      for (i = 0; i < new_stack->width; i++) {
	np = 0;
	for (v = 0; v < n; v++) {
	  if (IS_IN_CLOSE_RANGE3(i, j, k, start[0][v], end[0][v],
				 start[1][v], end[1][v],
				 start[2][v], end[2][v])) {
	    switch (new_stack->kind) {
	    case GREY:
	      p[0][np++] = (double) tile_arrays[v].array[index[v]];
	      break;
	    case GREY16:
	      p[0][np++] = (double) tile_arrays[v].array16[index[v]];
	      break;
	    case FLOAT32:
	      p[0][np++] = (double) tile_arrays[v].array32[index[v]];
	      break;
	    case FLOAT64:
	      p[0][np++] = (double) tile_arrays[v].array64[index[v]];
	      break;
	    case COLOR:
	      p[0][np] = (double) tile_arrays[v].arrayc[index[v]][0];
	      p[1][np] = (double) tile_arrays[v].arrayc[index[v]][1];
	      p[2][np++] = (double) tile_arrays[v].arrayc[index[v]][2];
	      break;
	    }
	    index[v]++;
	  }
	}

	switch(new_stack->kind) {
	case GREY:
	  STACK_MERGE_M(ima.array, uint8);
	  break;
	case GREY16:
	  STACK_MERGE_M(ima.array16, uint16);
	  break;
	case FLOAT32:
	  STACK_MERGE_M(ima.array32, float32);
	  break;
	case FLOAT64:
	  STACK_MERGE_M(ima.array64, float64);
	  break;
	case COLOR:
	  STACK_MERGE_M(ima.array, uint8);
	  break;
	default:
	  TZ_ERROR(ERROR_DATA_TYPE);
	}
      }									
    }									
  }

  /* free <tile_arrays> */
  free(tile_arrays);
  
  /* free <p> */
  free(p[0]);
  if (new_stack->kind == COLOR) {
    free(p[1]);
    free(p[2]);
  }

  printf("Merging done.\n");

  /* free <start> <end> */
  for (i = 0; i < 3; i++) {
    free(start[i]);
    free(end[i]);
  }

  /* free <index> */
  free(index);

  return new_stack;
}

/* fix two corner, region between corner1 and corner2 should be big 
enough for all stacks, corner1 is upper left corner, corner2 is lower right corner */
Stack *Stack_Merge_M_FC(Stack **stack, int n, int **offset, int option, 
         Stack *new_stack, int *corner1, int *corner2)
{
  int *start[3];
  int *end[3];
  int i, j;
  /* alloc <start> <end> */
  for (i = 0; i < 3; i++) {
    start[i] = iarray_malloc(n);
    end[i] = iarray_malloc(n);
  }
  
  for (i = 0; i < n; i++) {
    for (j = 0; j < 3; j++) {
      start[j][i] = offset[i][j];
    }
  }


  for (i = 0; i < n; i++) {
    for (j = 0; j < 3; j++) {
      start[j][i] -= corner1[j];
    }
    end[0][i] = start[0][i] + stack[i]->width - 1;
    end[1][i] = start[1][i] + stack[i]->height - 1;
    end[2][i] = start[2][i] + stack[i]->depth - 1;
  }

  int width = corner2[0] - corner1[0] + 1;
  int height = corner2[1] - corner1[1] + 1;
  int depth = corner2[2] - corner1[2] + 1;

  printf("%d %d %d\n", width, height, depth);
  
  /*
  Stack *new_stack = Crop_Stack(stack[0], -start[0][0], -start[1][0],
        -start[2][0],
        width, height, depth, NULL);
  */
  
  if (new_stack == NULL) {
    new_stack = Make_Stack(stack[0]->kind, width, height, depth);
  }
  
  /* alloc <index> */
  size_t new_index = 0;
  size_t *index;// = iarray_calloc(n);
  GUARDED_CALLOC_ARRAY(index, n, size_t);
  
  printf("Merging ...\n");

  int k, v;
  
  /* alloc <p> */
  double *p[3];
  
  p[0] = darray_malloc(n);

  if (new_stack->kind == COLOR) {
    p[1] = darray_malloc(n);
    p[2] = darray_malloc(n);
  } else {
    p[1] = NULL;
    p[2] = NULL;
  }

  Image_Array ima;
  ima.array = new_stack->array;

  /* allocate <tile_arrays> */
  Image_Array *tile_arrays = (Image_Array *) malloc(sizeof(Image_Array) * n);
  for (v = 0; v < n; v++) {
    tile_arrays[v].array = stack[v]->array;
  }

  int np;

  for (k = 0; k < new_stack->depth; k++) {
    for (j = 0; j < new_stack->height; j++) {
      for (i = 0; i < new_stack->width; i++) {
  np = 0;
  for (v = 0; v < n; v++) {
    if (IS_IN_CLOSE_RANGE3(i, j, k, start[0][v], end[0][v],
         start[1][v], end[1][v],
         start[2][v], end[2][v])) {
      switch (new_stack->kind) {
      case GREY:
        p[0][np++] = (double) tile_arrays[v].array[index[v]];
        break;
      case GREY16:
        p[0][np++] = (double) tile_arrays[v].array16[index[v]];
        break;
      case FLOAT32:
        p[0][np++] = (double) tile_arrays[v].array32[index[v]];
        break;
      case FLOAT64:
        p[0][np++] = (double) tile_arrays[v].array64[index[v]];
        break;
      case COLOR:
        p[0][np] = (double) tile_arrays[v].arrayc[index[v]][0];
        p[1][np] = (double) tile_arrays[v].arrayc[index[v]][1];
        p[2][np++] = (double) tile_arrays[v].arrayc[index[v]][2];
        break;
      }
      index[v]++;
    }
  }

  switch(new_stack->kind) {
  case GREY:
    STACK_MERGE_M(ima.array, uint8);
    break;
  case GREY16:
    STACK_MERGE_M(ima.array16, uint16);
    break;
  case FLOAT32:
    STACK_MERGE_M(ima.array32, float32);
    break;
  case FLOAT64:
    STACK_MERGE_M(ima.array64, float64);
    break;
  case COLOR:
    STACK_MERGE_M(ima.array, uint8);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
      }                 
    }                 
  }

  /* free <tile_arrays> */
  free(tile_arrays);
  
  /* free <p> */
  free(p[0]);
  if (new_stack->kind == COLOR) {
    free(p[1]);
    free(p[2]);
  }

  printf("Merging done.\n");

  /* free <start> <end> */
  for (i = 0; i < 3; i++) {
    free(start[i]);
    free(end[i]);
  }

  /* free <index> */
  free(index);

  return new_stack;
}

Tiff_Image* Stack_Tiff_Image(const Stack *stack, int s, Tiff_Image *image)
{
  if (image == NULL) {
    image = Create_Tiff_Image(stack->width, stack->height);
  }

  size_t plane_bsize = stack->width * stack->height * stack->kind;
  int area = stack->width * stack->height;
  size_t array_offset =  plane_bsize * s;

  switch (stack->kind) {
  case GREY:
    Add_Tiff_Image_Channel(image, CHAN_OTHER, 8, CHAN_UNSIGNED);
    memcpy(image->channels[0]->plane, stack->array + array_offset,
	   plane_bsize);
    break;
  case GREY16:
    Add_Tiff_Image_Channel(image, CHAN_OTHER, 16, CHAN_UNSIGNED);
    memcpy(image->channels[0]->plane, stack->array + array_offset,
	   plane_bsize);
    break;
  case COLOR:
    Add_Tiff_Image_Channel(image, CHAN_RED, 8, CHAN_UNSIGNED);
    Add_Tiff_Image_Channel(image, CHAN_GREEN, 8, CHAN_UNSIGNED);
    Add_Tiff_Image_Channel(image, CHAN_BLUE, 8, CHAN_UNSIGNED);
    
    { uint8 *red, *green, *blue;
      uint8 *out;
      int    i;

      out   = stack->array + array_offset;
      red   = (uint8*)image->channels[0]->plane;
      green = (uint8*)image->channels[1]->plane;
      blue  = (uint8*)image->channels[2]->plane;
      for (i = 0; i < area; i++)
	{ *red++ = *out++;
	  *green++ = *out++;
	  *blue++ = *out++;
	}
    }
    /*
    int i, j;
    color_t *arrayc = (color_t *) (stack->array + array_offset);
    size_t offset = 0;
    uint8 *array_r = (uint8 *) image->channels[0]->plane;
    uint8 *array_g = (uint8 *) image->channels[1]->plane;
    uint8 *array_b = (uint8 *) image->channels[2]->plane;
    for (j = 0; j < stack->height; j++) {
      for (i = 0; i < stack->width; i++) {
	array_r[offset] = arrayc[offset][0];
	array_g[offset] = arrayc[offset][1];
	array_b[offset] = arrayc[offset][2];
	offset++;
      }
      }
    */
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return image;
}

#define STACK_LOCMAX_ENHANCE(stack_array, out_array, validate_intensity) \
  for (z = 0; z < stack->depth; z++) {					\
    for (y = 0; y < stack->height; y++) {				\
      for (x = 0; x < stack->width; x++) {				\
	int n_in_bound = Stack_Neighbor_Bound_Test(n_nbr, cwidth,	\
						   cheight, cdepth,	\
						   x, y, z, is_in_bound); \
	double value = ninf;						\
	int i;								\
	if (n_in_bound > 0) {						\
	  if (n_in_bound == n_nbr) {					\
	    for (i = 0; i < n_nbr; i++) {				\
	      /*value += stack_array[center + neighbor[i]];*/		\
	      if (value < stack_array[center + neighbor[i]]) {		\
		value = stack_array[center + neighbor[i]];		\
	      }								\
	    }								\
	  } else {							\
	    int j;							\
	    for (j = 0; j < n_nbr; j++) {				\
	      if (is_in_bound[j] == TRUE) {				\
		/*value += stack_array[center + neighbor[j]];*/		\
		if (value < stack_array[center + neighbor[j]]) {	\
		  value = stack_array[center + neighbor[j]];		\
		}							\
	      }								\
	    }								\
	  }								\
	}								\
	if (value < stack_array[center]) {				\
	  /*value = 2.0 * stack_array[center] - value / n_in_bound;*/	\
	  value = 2.0 * stack_array[center] - value;			\
	  validate_intensity(value);					\
	  out_array[center] = value;					\
	} else {							\
	  out_array[center] = stack_array[center];			\
	}								\
	center++;							\
      }									\
    }									\
  }
 
Stack* Stack_Locmax_Enhance(Stack *stack, Stack *out)
{
  if (out == NULL) {
    out = Make_Stack(stack->kind, stack->width, stack->height, stack->depth);
  }

  BOOL in_place = FALSE;
  if (stack == out) {
    stack = Copy_Stack_T(out);    
    in_place = TRUE;
  }

  /*Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_NONFLAT);*/

  int n_nbr = 26;
  if (stack->depth == 1) {
    n_nbr = 8;
  }

#ifdef _MSC_VER
  int *neighbor = (int*)malloc(sizeof(int)*n_nbr);
#else
  int neighbor[n_nbr];
#endif
  Stack_Neighbor_Offset(n_nbr, stack->width, stack->height, neighbor);
  
  double ninf = -Infinity;

#ifdef _MSC_VER
  int *is_in_bound = (int*)malloc(sizeof(int)*n_nbr);
#else
  int is_in_bound[n_nbr];
#endif

  int x, y, z;
  int center = 0;
  
  Image_Array ima;
  Image_Array oa;
  ima.array = stack->array;
  oa.array = out->array;

  int cwidth = stack->width - 1;
  int cheight = stack->height - 1;
  int cdepth = stack->depth - 1;

  switch (stack->kind) {
  case GREY:
    STACK_LOCMAX_ENHANCE(ima.array8, oa.array8, VALIDATE_INTENSITY_GREY);
    break;
  case GREY16:
    STACK_LOCMAX_ENHANCE(ima.array16, oa.array16, VALIDATE_INTENSITY_GREY16);
    break;
  case FLOAT32:
    STACK_LOCMAX_ENHANCE(ima.array32, oa.array32, VALIDATE_INTENSITY_FLOAT32);
    break;
  case FLOAT64:
    STACK_LOCMAX_ENHANCE(ima.array64, oa.array64, VALIDATE_INTENSITY_FLOAT64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }
  
  //Kill_Stack(locmax);
  
  if (in_place == TRUE) {
    Kill_Stack(stack);
  }

#ifdef _MSC_VER
  free(neighbor);
  free(is_in_bound);
#endif

  return out;
}

#define STACK_RUNNING_MINMAX_INIT()					\
  if (out == NULL) {							\
    out = Make_Stack(Stack_Kind(stack), Stack_Width(stack), Stack_Height(stack), Stack_Depth(stack)); \
  }									\
									\
  Image_Array ima;							\
  ima.array = stack->array;						\
									\
  Image_Array out_ima;							\
  out_ima.array = out->array;						\
									\
  int i, j, k;								\
  int dim[3];								\
									\
  int offset = 0;							\
  int step[3];								\
									\
  switch (dim_idx) {							\
  case 0:								\
    dim[0] = Stack_Width(stack);					\
    dim[1] = Stack_Height(stack);					\
    dim[2] = Stack_Depth(stack);					\
    step[0] = 1;							\
    step[1] = Stack_Width(stack);					\
    step[2] = 0;							\
    break;								\
  case 1:								\
    dim[0] = Stack_Height(stack);					\
    dim[1] = Stack_Width(stack);					\
    dim[2] = Stack_Depth(stack);					\
    step[0] = Stack_Width(stack);					\
    step[1] = 1;							\
    step[2] = Stack_Width(stack) * (Stack_Height(stack) - 1);		\
    break;								\
  case 2:								\
    dim[0] = Stack_Depth(stack);					\
    dim[1] = Stack_Width(stack);					\
    dim[2] = Stack_Height(stack);					\
    step[0] = Stack_Width(stack) * Stack_Height(stack);			\
    step[1] = 1;							\
    step[2] = 0;							\
    break;								\
  }									\
									\
  int running_start = 0;
  
#define STACK_RUNNING_MINMAX(stack_array, out_array, minmax2, minmax3, type) \
  if (stack_array != out_array) {					\
    for (k = 0; k < dim[2]; k++) {					\
      for (j = 0; j < dim[1]; j++) {					\
	offset = running_start;						\
	for (i = 0; i < dim[0]; i++) {					\
	  if (i == 0) {							\
	    out_array[offset] =						\
	      minmax2(stack_array[offset], stack_array[offset + step[0]]); \
	  } else if (i == dim[0] - 1) {					\
	    out_array[offset] =						\
	      minmax2(stack_array[offset], stack_array[offset - step[0]]); \
	  } else {							\
	    out_array[offset] =						\
	      minmax3(stack_array[offset - step[0]], stack_array[offset], \
		      stack_array[offset + step[0]]);			\
	  }								\
	  offset += step[0];						\
	}								\
	running_start += step[1];					\
      }									\
      running_start += step[2];						\
    }									\
  } else {								\
    type *buffer;							\
    GUARDED_MALLOC_ARRAY(buffer, dim[0], type);				\
    for (k = 0; k < dim[2]; k++) {					\
      for (j = 0; j < dim[1]; j++) {					\
	offset = running_start;						\
	for (i = 0; i < dim[0]; i++) {					\
	  buffer[i] = stack_array[offset + step[0] * i];		\
	}								\
	for (i = 0; i < dim[0]; i++) {					\
	  if (i == 0) {							\
	    out_array[offset] =						\
	      minmax2(buffer[i], buffer[i+1]);				\
	  } else if (i == dim[0] - 1) {					\
	    out_array[offset] =						\
	      minmax2(buffer[i], buffer[i-1]);				\
	  } else {							\
	    out_array[offset] =						\
	      minmax3(buffer[i-1], buffer[i],				\
		      buffer[i+1]);					\
	  }								\
	  offset += step[0];						\
	}								\
	running_start += step[1];					\
      }									\
      running_start += step[2];						\
    }									\
    free(buffer);							\
  }


Stack* Stack_Running_Max(const Stack *stack, int dim_idx, Stack *out)
{
  STACK_RUNNING_MINMAX_INIT();

  switch (stack->kind) {
  case GREY:
    STACK_RUNNING_MINMAX(ima.array8, out_ima.array8, MAX2, MAX3, uint8);
    break;
  case GREY16:
    STACK_RUNNING_MINMAX(ima.array16, out_ima.array16, MAX2, MAX3, uint16);
    break;
  case GREY32:
    STACK_RUNNING_MINMAX(ima.array32, out_ima.array32, MAX2, MAX3, float32);
    break;
  case GREY64:
    STACK_RUNNING_MINMAX(ima.array64, out_ima.array64, MAX2, MAX3, float64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  return out;
}

Stack* Stack_Running_Min(const Stack *stack, int dim_idx, Stack *out)
{
  STACK_RUNNING_MINMAX_INIT();

  switch (stack->kind) {
  case GREY:
    STACK_RUNNING_MINMAX(ima.array8, out_ima.array8, MIN2, MIN3, uint8);
    break;
  case GREY16:
    STACK_RUNNING_MINMAX(ima.array16, out_ima.array16, MIN2, MIN3, uint16);
    break;
  case GREY32:
    STACK_RUNNING_MINMAX(ima.array32, out_ima.array32, MIN2, MIN3, float32);
    break;
  case GREY64:
    STACK_RUNNING_MINMAX(ima.array64, out_ima.array64, MIN2, MIN3, float64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  return out;
}

#define MEDIAN2(a, b) (a)

Stack* Stack_Running_Median(const Stack *stack, int dim_idx, Stack *out)
{
  STACK_RUNNING_MINMAX_INIT();

  switch (stack->kind) {
  case GREY:
    STACK_RUNNING_MINMAX(ima.array8, out_ima.array8, MEDIAN2, MEDIAN3, uint8);
    break;
  case GREY16:
    STACK_RUNNING_MINMAX(ima.array16, out_ima.array16, MEDIAN2, MEDIAN3, uint16);
    break;
  case GREY32:
    STACK_RUNNING_MINMAX(ima.array32, out_ima.array32, MEDIAN2, MEDIAN3, float32);
    break;
  case GREY64:
    STACK_RUNNING_MINMAX(ima.array64, out_ima.array64, MEDIAN2, MEDIAN3, float64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  return out;
}

Stack* Stack_Median_Filter_N(const Stack *stack, int conn, Stack *out)
{
  if (out == NULL) {
    out = Copy_Stack((Stack*) stack);
  } else {
    Copy_Stack_Array(out, stack);
  }

  int is_in_bound[26];
  int neighbor[26];
  double value[27];

  int width = Stack_Width(stack);
  int height = Stack_Height(stack);
  int depth = Stack_Depth(stack);
  Stack_Neighbor_Offset(conn, width, height, neighbor);
  int offset = 0;
  int nvoxel = Stack_Voxel_Number(stack);
  uint16_t *stack_array = (uint16_t*) stack->array;
  uint16_t *out_array = (uint16_t*) out->array;
  for (offset = 0; offset < nvoxel; offset++) {
    int nbound = Stack_Neighbor_Bound_Test_I(conn, width, height, depth, offset,
	is_in_bound);
    if (nbound == conn) {
      value[0] = stack_array[offset];
      int j;
      for (j = 1; j <= conn; j++) {
	value[j] = stack_array[offset + neighbor[j-1]];
      }
      darray_qsort(value, NULL, conn+1);
      //darray_print2(value, conn+1, 1);
      out_array[offset] = value[conn / 2];
    }
  }

  return out;
}

void Stack_Bound_Box(const Stack *stack, Cuboid_I *bound_box)
{
  TZ_ASSERT(bound_box != NULL, "null pointer");

  int i, j, k;
  size_t offset = 0;
  bound_box->ce[0] = 0;
  bound_box->ce[1] = 0;
  bound_box->ce[2] = 0;
  bound_box->cb[0] = stack->width - 1;
  bound_box->cb[1] = stack->height - 1;
  bound_box->cb[2] = stack->depth - 1;
  
  int pt[3];
  for (k = 0; k < stack->depth; k++) {
    pt[2] = k;
    for (j = 0; j < stack->height; j++) {
      pt[1] = j;
      for (i = 0; i < stack->width; i++) {
        pt[0] = i;
        if (stack->array[offset++] > 0) {
          Cuboid_I_Expand_P(bound_box, pt); 
        }
      }
    }
  }
}

void Stack_Bound_Box_F(File_List *list, Cuboid_I *bound_box)
{
  int i;
  for (i = 0; i < 3; i++) {
    bound_box->cb[i] = -1;
    bound_box->ce[i] = 0;
  }
  
  Cuboid_I cuboid;

  int front = -1;
  int back = 0;

  if (list != NULL) {
    if (list->file_number > 0) {
      for (i = 0; i < list->file_number; i++) {
        Stack *stack = Read_Stack_U(list->file_path[i]);
        if (i == 0) {
          Stack_Bound_Box(stack, bound_box);
          if (Cuboid_I_Is_Valid(bound_box)) {
            front = 0;
          }
        } else {
          Stack_Bound_Box(stack, &cuboid);
          if (Cuboid_I_Is_Valid(&cuboid)) {
            if (front < 0) {
              front = i;
            }
            back = i;
          }
          Cuboid_I_Union(&cuboid, bound_box, bound_box);
        }
        Free_Stack(stack);
      }
    }
  }

  bound_box->cb[2] = front;
  bound_box->ce[2] = back;

  Reset_Stack();
}

void Stack_Bound_Box_F_M(File_List *list, Cuboid_I *bound_box)
{
  int i;
  for (i = 0; i < 3; i++) {
    bound_box->cb[i] = -1;
    bound_box->ce[i] = 0;
  }
  
  int front = -1;
  int back = 0;

  if (list != NULL) {
    if (list->file_number > 0) {
      Stack *mip = Read_Stack_U(list->file_path[0]);
      size_t nvoxel = Stack_Voxel_Number(mip);

      for (i = 1; i < list->file_number; i++) {
        BOOL all_zero = TRUE;
        Stack *stack = Read_Stack_U(list->file_path[i]);
        size_t offset = 0;
        for (offset = 0; offset < nvoxel; offset++) {
          if (stack->array[offset] != 0) {
            all_zero = FALSE;
            
            if (stack->array[offset] > mip->array[offset]) {
              mip->array[offset] = stack->array[offset];
            }
          }
        }
        
        if (all_zero == FALSE) {
          if (front < 0) {
            front = i;
          }
          back = i;
        }

        Free_Stack(stack);
      }
      Stack_Bound_Box(mip, bound_box);
      Kill_Stack(mip);
    }
  }

  bound_box->cb[2] = front;
  bound_box->ce[2] = back;

  Reset_Stack();
}

Stack* Stack_Mip_F(File_List *list)
{
  Stack *mip = NULL;

  if (list != NULL) {
    if (list->file_number > 0) {
      mip = Read_Stack_U(list->file_path[0]);
      size_t nvoxel = Stack_Voxel_Number(mip);

      int i;
      for (i = 1; i < list->file_number; i++) {
        Stack *stack = Read_Stack_U(list->file_path[i]);
        size_t offset = 0;
        for (offset = 0; offset < nvoxel; offset++) {
          if (stack->array[offset] > mip->array[offset]) {
            mip->array[offset] = stack->array[offset];
          }
        }

        Free_Stack(stack);
      }
    }
  }

  Reset_Stack();

  return mip;
}

Stack* Stack_Bound_Crop(const Stack *stack, int margin)
{
  Cuboid_I bound_box;
  Stack_Bound_Box(stack, &bound_box);

  int width = 0; 
  int height = 0; 
  int depth = 0; 
  Cuboid_I_Size(&bound_box, &width, &height, &depth);

  Stack *out =  Crop_Stack(stack, bound_box.cb[0] - margin, 
      bound_box.cb[1] - margin, bound_box.cb[2] - margin,
      width + margin * 2, height + margin * 2, depth + margin * 2, NULL);

  return out;
}
