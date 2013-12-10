#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <math.h>
#include <utilities.h>
#include "tz_image_lib_defs.h"
#include "tz_image_lib.h"
#include "tz_iimage_lib.h"
#include "tz_utilities.h"
#include "tz_iarray.h"
#include "tz_dimage_lib.h"
#include "tz_string.h"
#include "tz_image_attribute.h"
#include "tz_image_relation.h"
#include "tz_darray.h"
#include "tz_farray.h"
#include "tz_math.h"
#include "tz_error.h"

INIT_EXCEPTION

#include "private/tz_image_lib.c"

/* Unpack_Image(): unpack an image.
 *
 * Note: This function free the image pointer but keep the raw image data array.
 *
 * Args: image - pointer of the image to unpack.
 *
 * Return: raw image data.
 */
uint8* Unpack_Image(Image *image)
{
  if (image == NULL) {
    return NULL;
  }

  uint8* array2 = image->array;
  image->array = NULL;
  Kill_Image(image);

  return array2;
}

/*
 * Get_Pixel_Number(): Get the number of pixels of an image.
 */
int Get_Pixel_Number(Image* image)
{
  return (image->width)*(image->height);
}

/*
 * Turn an image into a double array. It only return pixel values of the image.
 * All other information will not be stored in the result.
 */
double* Get_Double_Array(Image* image) {
  long length = image->width*image->height;
  int i;
  double* array = NULL;

  switch(image->kind) {
  case GREY:
    array = (double*)malloc(length*sizeof(double));
    for(i=0;i<length;i++)
      array[i] = (double) image->array[i];
    break;
  case GREY16:
    array = (double*)malloc(length*sizeof(double));
    for(i=0;i<length;i++)
      array[i] = (double) ((uint16 *) image->array)[i];
    break;
  case FLOAT32:
    array = (double*)malloc(length*sizeof(double));
    for(i=0;i<length;i++)
      array[i] = (double) ((float32 *) image->array)[i];
    break;
  case COLOR:
    length *= 3;
    array = (double*)malloc(length*sizeof(double));
    for(i=0;i<length;i++)
      array[i] = (double) image->array[i];
    break;
  default:
    fprintf(stderr,"Get_Double_Array: Unsupported image kind.\n");
  }
  return array;
}

/**
 * Turn an image into a float array. Similar to Get_Double_Array.
 */
float* Get_Float_Array(Image* image) {
  long length = image->width*image->height;
  int i;
  float* array = NULL;

  switch(image->kind) {
  case GREY:
    array = (float*)malloc(length*sizeof(float));
    for(i=0;i<length;i++)
      array[i] = (float) image->array[i];
    break;
  case GREY16:
    array = (float*)malloc(length*sizeof(float));
    for(i=0;i<length;i++)
      array[i] = (float) ((uint16 *) image->array)[i];
    break;
  case FLOAT32:
    array = (float*)malloc(length*sizeof(float));
    memcpy(array,image->array,length*sizeof(float));
    break;
  case COLOR:
    length *= 3;
    array = (float*)malloc(length*sizeof(float));
    for(i=0;i<length;i++)
      array[i] = (float) image->array[i];
    break;
  default:
    fprintf(stderr,"Get_Double_Array: Unsupported image kind.\n");
  }
  
  return array;
}

#define IMAGE_REAL_ARRAY(image_array, type)		\
  for (i = 0; i < npixel; i++) {			\
    array[i] = (type) ((image_array)[i]);		\
  }

double* Image_Double_Array(const Image *image, double *array)
{
  int npixel = Image_Pixel_Number(image);
  if (array == NULL) {
    array = darray_malloc(npixel * Image_Channel_Number(image));
  }

  Image_Array ima;
  ima.array = image->array;
  int i;

  switch (image->kind) {
  case GREY:
    IMAGE_REAL_ARRAY(ima.array, double);
    break;
  case GREY16:
    IMAGE_REAL_ARRAY(ima.array16, double);
    break;
  case FLOAT32:
    IMAGE_REAL_ARRAY(ima.array32, double);
    break;
  case FLOAT64:
    memcpy(array, ima.array64, npixel * sizeof(double));
    break;
  case COLOR:
    for (i = 0; i < npixel; i++) {
      int j;
      for (j = 0; j < 3; j++) {
	array[i * 3 + j] = (double) ((ima.arrayc)[i][j]);
      }
    }
    break;
  }

  return array;
}

float* Image_Float_Array(const Image *image, float *array)
{
  int npixel = Image_Pixel_Number(image);
  if (array == NULL) {
    array = farray_malloc(npixel * Image_Channel_Number(image));
  }

  Image_Array ima;
  ima.array = image->array;
  int i;

  switch (image->kind) {
  case GREY:
    IMAGE_REAL_ARRAY(ima.array, float);
    break;
  case GREY16:
    IMAGE_REAL_ARRAY(ima.array16, float);
    break;
  case FLOAT32:
    memcpy(array, ima.array64, npixel * sizeof(float));
    break;
  case FLOAT64:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  case COLOR:
    for (i = 0; i < npixel; i++) {
      int j;
      for (j = 0; j < 3; j++) {
	array[i * 3 + j] = (float) ((ima.arrayc)[i][j]);
      }
    }
    break;
  }

  return array;
}

/**
 * Scale a float array and turn it into an image. The values of the image
 * will have largest possible range. For an image with kind FLOAT32, it is
 * scaled to [0,1]
 */
Image* Scale_Float_Image(float *fImage,int width,int height,int kind)
{
  long length = width*height;
  float maxgrey = maxfloat(fImage,length);
  float mingrey = minfloat(fImage,length);
  
  float dest_max = (float)MAXGREY(kind);

  int i;
  float maxdiff = maxgrey - mingrey;
  Image* image = Make_Image(kind,width,height);
  uint16* array16 = (uint16*)image->array;
  float32* array32 = (float32*)image->array;

  switch(kind) {
  case GREY:
    for(i=0;i<length;i++)
      image->array[i] = (uint8) (dest_max*(fImage[i]-mingrey)/maxdiff);
    break;
  case COLOR:
    length *= 3;
    for(i=0;i<length;i++)
      image->array[i] = (uint8) (dest_max*(fImage[i]-mingrey)/maxdiff);
    break;
  case GREY16:
    for(i=0;i<length;i++)
      array16[i] = (uint16) (dest_max*(fImage[i]-mingrey)/maxdiff);
    break;
  case FLOAT32:
    for(i=0;i<length;i++)
      array32[i] = (fImage[i]-mingrey)/maxdiff;
    break;
  default:
    fprintf(stderr,"Get_Double_Array: Unsupported image kind.\n");
  }

  return image;
}

/*
 * Scale_Double_Image(): Scale an double image.
 *
 * Note: Similar to Scale_Float_Image, except that the input is a double array.
 */
Image* Scale_Double_Image(double *fImage,int width,int height,int kind)
{
  long length = width*height;
  //double maxgrey = maxdouble(fImage,length);
  //double mingrey = mindouble(fImage,length);  
  double maxgrey = darray_max(fImage, length, NULL);
  double mingrey = darray_min(fImage, length, NULL);
  double dest_max = (double)MAXGREY(kind);

  int i;
  double maxdiff = maxgrey - mingrey;
  Image* image = Make_Image(kind,width,height);
  
  uint16* array16 = (uint16*)image->array;
  float32* array32 = (float32*)image->array;

  switch(kind) {
  case GREY:
    for(i=0;i<length;i++)
      image->array[i] = (uint8) (dest_max*(fImage[i]-mingrey)/maxdiff);
    break;
  case COLOR:
    length *= 3;
    for(i=0;i<length;i++)
      image->array[i] = (uint8) (dest_max*(fImage[i]-mingrey)/maxdiff);
    break;
  case GREY16:
    for(i=0;i<length;i++)
      array16[i] = (uint16) (dest_max*(fImage[i]-mingrey)/maxdiff);
    break;
  case FLOAT32:
    for(i=0;i<length;i++)
      array32[i] = (float)(fImage[i]-mingrey)/maxdiff;
    break;
  default:
    fprintf(stderr,"Get_Double_Array: Unsupported image kind.\n");
  }
  
  /*
  if(kind==GREY16) {
    uint16 * _array = (uint16 *)image->array;
    for(i=0;i<length;i++) {
      _array[i] = (uint16) (dest_max*(fImage[i]-mingrey)/maxdiff);
    }
  */
    /*
    uint16 debug_value;
    MAX(_array,length,debug_value);
    printf("debug_value: %d",debug_value);
    
  } else {
    for(i=0;i<length;i++)
      image->array[i] = (uint8) (dest_max*(fImage[i]-mingrey)/maxdiff);
  }
    */
  return image;
}

/**
 * Print image information.
 */
void Print_Image_Info(Image *image)
{
  if(image)
    printf("kind: %d; width: %d, height: %d\n",image->kind,image->width,image->height);
  else
    printf("Null image\n");
}

/**
 * Set all pixel intensities to 0 for an image.
 */
void Zero_Image(Image *image)
{
  int i;
  long length = image->width*image->height;
  float* array32 = (float*)image->array;
  double *array64 = (double *)image->array;

  switch(image->kind) {
  case COLOR:
  case GREY:
  case GREY16:
    length  *= image->kind;
    for(i=0;i<length;i++)
      image->array[i] = 0x00;
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
    fprintf(stderr,"Unrecognized image kind: %s","Zeor_Image");
  }
}

/* Copy_Image_Array(): copy one image to another.
 */
void Copy_Image_Array(Image *des, const Image *src)
{
  if (Image_Same_Attribute(src, des) == FALSE) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  memcpy(des->array, src->array, Image_Array_Bsize(src));
}

/* Crop_Image(): crop an image.
 * 
 * crop an image by a rectangle [left, top, right, bottom]
 * 0s will be padded if the rectangle is out of range
 */
Image* Crop_Image(const Image* image, int left, int top, int width, int height,
		  Image *desimage) 
{
  if(image == NULL) {
    return NULL;
  }

  /*The range is exactly the same as the original image*/
  if(left==0 && top==0 && width==image->width && height==image->height) {
    if (desimage == NULL) {
      return Copy_Image((Image *) image);
    } else {
      Copy_Image_Array(desimage, image);
      return desimage;
    }
  }
  
  if (desimage == NULL) {
    desimage = Make_Image(image->kind,width,height);
  }

  long length =  desimage->width*desimage->height*desimage->kind;
  long k;
  for(k=0;k<length;k++) {
    desimage->array[k]=0;
  }

  /*determine copy range*/
  int srcleft,srctop,desleft,destop,copy_width,copy_height;
  srcleft = imax2(left,0);
  srctop = imax2(top,0);
  desleft = imax2(-left,0);
  destop = imax2(-top,0);

  copy_width = imin2(image->width-srcleft,width-desleft)*image->kind;
  copy_height = imin2(image->height-srctop,height-destop);

  int i,srcoffset,desoffset;
  srcoffset = (srcleft+image->width*srctop)*image->kind;
  desoffset = (desleft+width*destop)*image->kind;

  int nsrcrow = image->width*image->kind;
  int ndesrow = width*image->kind;
  
  /*copy row by row*/
  for(i=0;i<copy_height;i++) {
    memcpy((void *)(desimage->array+desoffset),(void *)(image->array+srcoffset),(size_t)copy_width);
    srcoffset += nsrcrow;
    desoffset += ndesrow;
  }

  //Print_Image_Value(desimage);
  /*
  double* test = Get_Double_Array(desimage);
  darray_print2(test,desimage->width,desimage->height);
  free(test);
  */
  return desimage;
}



/**
 * Cumulative sum of an image.
 */
double* Cumsum_Image(const Image *image)
{
  if(!image)
    return NULL;
  
  double* cumsum = (double*)malloc(sizeof(double)*image->width*image->height);
  double cur_state = 0;
  int x,y,offset=1;
  uint16* array16 = (uint16*)image->array;
  float32* array32 = (float32*)image->array;

  switch(image->kind) {
  case GREY:
    //start
    cumsum[0] = (double)image->array[0];

    /*The first row*/
    for(x=1;x<image->width;x++) {
      cumsum[offset] = cumsum[offset-1] + (double)image->array[offset];
      offset++;
    }

    
    for(y=1;y<image->height;y++) {
      cur_state = 0;
      for(x=0;x<image->width;x++) {
	//Cumulate row sum of the current row
	cur_state += (double)image->array[offset];
	//Sum the cumsum of the up neighbor and current row sum
	cumsum[offset] = cumsum[offset-image->width]+cur_state;
	offset++;
      }
    }
    break;
  case GREY16:
    cumsum[0] = (double)array16[0];
    for(x=1;x<image->width;x++) {
      cumsum[offset] = cumsum[offset-1] + (double)array16[offset];
      offset++;
    }

    for(y=1;y<image->height;y++) {
      cur_state = 0;
      for(x=0;x<image->width;x++) {
	cur_state += (double)array16[offset];
	cumsum[offset] = cumsum[offset-image->width]+cur_state;
	offset++;
      }
    }
    break;
  case FLOAT32:
    cumsum[0] = array32[0];
    for(x=1;x<image->width;x++) {
      cumsum[offset] = cumsum[offset-1] + array32[offset];
      offset++;
    }

    for(y=1;y<image->height;y++) {
      cur_state = 0;
      for(x=0;x<image->width;x++) {
	cur_state += array32[offset];
	cumsum[offset] = cumsum[offset-image->width]+cur_state;
	offset++;
      }
    }
    break;
  default:
    fprintf(stderr,"Unsupported image kind in Cumsum_Image()\n");
  }

  return cumsum;
}

/**
 * x[i,j] -> x[w-1-i,h-1-j]. This rotates the image by 180 degrees.
 * If in_place is not 0, the result will overwrite the input.
 */
Image* Reflect_Image(Image *image, int in_place)
{
  Image* image2;
  if(in_place)
    image2 = image;
  else
    image2 = Make_Image(image->kind,image->width,image->height);

  long offset,offset2;
  long length=image->width*image->height; 

  switch(image->kind) {
  case GREY:
    offset2 = length - 1;
    if(in_place) {
      uint8 temp; //for swapping
      long length2 = length/2; //swap half of the array
      for(offset=0;offset<length2;offset++,offset2--) {
	temp = image->array[offset];
	image->array[offset] = image2->array[offset2];
	image2->array[offset2] = temp;
      }
    } else {  
      for(offset=0;offset<length;offset++,offset2--)
	image2->array[offset2] = image->array[offset];
    }
    break;
  case GREY16: {
    offset2 = length - 1;
    uint16* image_array = (uint16 *)image->array;
    uint16* image_array2 = (uint16 *)image2->array;
    if(in_place) {
      uint16 temp; //for swapping
      long length2 = length/2; //swap half of the array

      for(offset=0;offset<length2;offset++,offset2--) {
	temp = image_array[offset];
	image_array[offset] = image_array2[offset2];
	image_array2[offset2] = temp;
      }
    } else {  
      for(offset=0;offset<length;offset++,offset2--)
	image_array2[offset2] = image_array[offset];
    }
               }
    break;
  case FLOAT32: {
    offset2 = length - 1;
    float32* array32 = (float32 *)image->array;
    float32* array2_32 = (float32 *)image2->array;
    if(in_place) {
      float32 temp; //for swapping
      long length2 = length/2; //swap half of the array

      for(offset=0;offset<length2;offset++,offset2--) {
	temp = array32[offset];
	array32[offset] = array2_32[offset2];
	array2_32[offset2] = temp;
      }
    } else {  
      for(offset=0;offset<length;offset++,offset2--)
	array2_32[offset2] = array32[offset];
    }
                }
    break;
  default:
    fprintf(stderr,"Unsupported image kind in Reflect_Image()\n");
  }
  return image2;
}

void Print_Image_Value(Image* image) 
{
  int i,j;
  long offset=0;
  uint16* array16 = (uint16*) (image->array);
  switch(image->kind) {
  case GREY:
    for(j=0;j<image->height;j++) {
      for(i=0;i<image->width;i++) {
	printf("%d ",image->array[offset]);
	offset++;
      }
      printf("\n");
    }
    printf("\n");
    break;
  case GREY16:
    for(j=0;j<image->height;j++) {
      for(i=0;i<image->width;i++) {
	printf("%d ",array16[offset]);
	offset++;
      }
      printf("\n");
    }
    printf("\n");    
    break;
  case FLOAT32:
    farray_print2((float *) image->array, image->width, image->height);
    break;
  case FLOAT64:
    darray_print2((double *) image->array, image->width, image->height);
    break;
  default:
    fprintf(stderr,"Unsupported image kind in Print_Image_Value()\n");
  }
}

Image* Resize_Image(const Image* image,int width,int height)
{
  Image* image2 = Make_Image(image->kind,width,height);
  uint16* image2_array16 = (uint16*) image2->array;
  //uint16* image_array16 = (uint16*) image->array;

  int* col_idx = (int*) malloc(sizeof(int)*width);
  int* row_idx = (int*) malloc(sizeof(int)*height);
  map_index(image->width,width,col_idx);
  map_index(image->height,height,row_idx);

  int i,j,offset=0;
  switch(image->kind) {
  case GREY:
    for(i=0;i<height;i++)
      for(j=0;j<width;j++) {
	image2->array[offset] = 
	  (*IMAGE_PIXEL_8(image,col_idx[j],row_idx[i],1));
	offset++;
      }
    break;
  case GREY16:
    for(i=0;i<height;i++)
      for(j=0;j<width;j++) {
	image2_array16[offset] = 
	  (*IMAGE_PIXEL_16(image,col_idx[j],row_idx[i],1));
	offset++;
      }
    break;
  default:
    fprintf(stderr,"Resize_Image: Unsupported image kind.\n");
  }

  free(col_idx);
  free(row_idx);

  return image2;
}

int* Image_Hist_M(Image *image, Image *mask)
{
  if (mask != NULL) {
    if (mask->kind != GREY) {
      fprintf(stderr, "Invalid kind of mask.");
      TZ_ERROR(ERROR_DATA_TYPE);
    }

    if (Image_Same_Size(image, mask) == FALSE) {
      fprintf(stderr, "image and mask have different sizes.");
      TZ_ERROR(ERROR_DATA_VALUE);
    }
  }

  int* hist = NULL;
  Pixel_Range *pr;
  int mingrey,maxgrey;
  long i;
  long pixel_number = Get_Pixel_Number(image);
  uint16* array16 = (uint16*) image->array;

  switch(image->kind) {
  case GREY:
    pr = Image_Range(image,0);
    mingrey = (int)pr->minval;
    maxgrey = (int)pr->maxval;
    hist = (int *)malloc(sizeof(int)*(maxgrey-mingrey+3));
    hist[0] = maxgrey-mingrey+1; /* pixel range */
    hist[1] = mingrey; /* offset */
    bzero(hist+2,sizeof(int)*hist[0]);
    hist -= mingrey-2;
    if (mask == NULL) {
      for(i=0;i<pixel_number;i++) {
	hist[image->array[i]]++;
      }
    } else {
      for(i=0;i<pixel_number;i++) {
	if (mask->array[i] == 1) {
	  hist[image->array[i]]++;
	}
      }
    }
    hist += mingrey-2;
    break;
  case GREY16:
    pr = Image_Range(image,0);
    mingrey = (int)pr->minval;
    maxgrey = (int)pr->maxval;
    hist = (int *)malloc(sizeof(int)*(maxgrey-mingrey+3));
    hist[0] = maxgrey-mingrey+1; /* pixel range */
    hist[1] = mingrey; /* offset */
    bzero(hist+2,sizeof(int)*hist[0]);
    hist -= mingrey-2;
    if (mask == NULL) {
      for(i=0;i<pixel_number;i++) {
	hist[array16[i]]++;
      }
    } else {
       for(i=0;i<pixel_number;i++) {
	 if (mask->array[i] == 1) { 
	   hist[array16[i]]++;
	 }
       }
    }
    hist += mingrey-2;
    break;
  default:
    fprintf(stderr,"Image_Hist: Unsupported image kind.\n");
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  return hist;
}

int* Image_Hist(Image *image)
{
  return Image_Hist_M(image, NULL);
}


void Empty_Image(Image *image)
{
  if (image == NULL) {
    return;
  }

  image->kind = 0;
  image->width = 0;
  image->height = 0;
  image->array = NULL;
}

BOOL Is_Image_Empty(const Image *image)
{
  if (image != NULL) {
    if ((image->array == NULL) || (image->kind == 0) || (image->width == 0) ||
	(image->height == 0)) {
      return TRUE;
    }
  }

  return FALSE;
}

Image* Image_Block_Sum(const Image *image, int w, int h)
{
  Image *out = Make_Image(FLOAT64, image->width, image->height);

  DMatrix dm;
  dm.ndim = 2;
  dm.dim[0] = image->width;
  dm.dim[1] = image->height;
  dm.array = (double *) out->array;

  Image_Double_Array(image, dm.array);

  dim_type bdim[2];
  bdim[0] = w;
  bdim[1] = h;
  DMatrix *dm2 = DMatrix_Blocksum(&dm, bdim, NULL);

  int offset[2];
  offset[0] = w / 2;
  offset[1] = h / 2;

  Crop_DMatrix(dm2, offset, dm.dim, &dm);

  return out;
}
