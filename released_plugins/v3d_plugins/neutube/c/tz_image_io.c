/* tz_image_io.c
 *
 * 08-Dec-2007 Initial write: Ting Zhao
 */

#include "tz_image_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define __USE_BSD
#include <string.h>
#include <stdint.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/stat.h>
#include "tz_error.h"
#include "tz_string.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_lib.h"
#include "tz_stack_attribute.h"
#include "tz_xml_utils.h"
#include "tz_png_io.h"
#include "tz_stack_math.h"
#include "tz_object_3d_linked_list_ext.h"

static int lsm_thumbnail_flag(Tiff_IFD *ifd)
{
  Tiff_Type type = (Tiff_Type)0;
  int count = 0;
  uint32_t *val = (uint32_t*)Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
  
  if (val == NULL) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return *val==1;
}

/* copied from ../lib/myers.pack/tiff_image.c */
static inline unsigned int get_integer_tag(Tiff_IFD *ifd, int label, int *error)
{ void *p;
  Tiff_Type   type;
  int count;

  *error = 0;
  p = Get_Tiff_Tag(ifd,label,&type,&count);
  if (p == NULL)
    { *error = 1;
      return (0);
    }
  if (count != 1)
    { *error = 2;
      return (0);
    }
  if (type == TIFF_SHORT)
    return (*((unsigned short *) p));
  else if (type == TIFF_LONG)
    { unsigned int value = *((unsigned int *) p);
      if (value >= 0x80000000)
        return (0x7fffffff);
      else
        return (value);
    }
  else
    { *error = 3;
      return (0);
    }
}

static void image_ifd_size(Tiff_IFD *ifd, int *size)
{
  int eflag;
  size[0]  = get_integer_tag(ifd,TIFF_IMAGE_WIDTH,&eflag);
  if (eflag != 0) {
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  size[1] = get_integer_tag(ifd,TIFF_IMAGE_LENGTH,&eflag);
  if (eflag != 0) {
    TZ_ERROR(ERROR_DATA_VALUE);
  }  
}

int Tiff_Kind(const char *filepath, int is_lsm)
{
  Tiff *tif = Open_Tiff_E(filepath, "r", is_lsm);

  int kind = Determine_Kind(tif);
  Close_Tiff(tif);

  return kind;
}

void Tiff_Attribute(const char *filepath, int is_lsm,
    int *kind, int *width, int *height, int *depth)
{
  *kind = Tiff_Kind(filepath, is_lsm);

  int size[3] = {0, 0, 0};

  if (is_lsm) {
    Lsm_Size(filepath, size);
  } else {
    Tiff_Size(filepath, size);
  }

  *width = size[0];
  *height = size[1];
  *depth = size[2];
}

/* Read_Raw(): Read raw images.
 *
 * Args: filename - path of the image file;
 *       kind - kind of the image;
 *       width - width of the image;
 *       height - height of the image.
 *       offset - offset of the meta data in the file. The data right after
 *                <offset> should be where the image is stored.
 *
 * Return: an image.
 */
Image* Read_Raw(char* filename, int kind, int width, int height, int offset)
{
  FILE* fp;
  Image* image;

  if((fp=fopen(filename,"rb"))) {
    if(offset > 0) {
      fseek(fp,(long)offset,SEEK_SET);
    }

    image = Make_Image(kind,width,height);
    fread(image->array,sizeof(uint8),width*height*kind,fp);
    fclose(fp);
  } else {
    fprintf(stderr,"Failed to read %s",filename);
    image = NULL;
  }

  return image;
}

Image* Read_Em_Raw(const char* filename)
{
  FILE* fp;
  Image* image;

  if((fp=fopen(filename,"rb"))) {
    int width, height;
    fread(&width, sizeof(int), 1, fp);
    fread(&height, sizeof(int), 1, fp);

    image = Make_Image(GREY,width,height);
    fread(image->array, sizeof(uint8), width*height, fp);
    fclose(fp);
  } else {
    fprintf(stderr,"Failed to read %s",filename);
    image = NULL;
  }

  return image;
}
/* Is_Tiff(): Test if a file is a tif.
 *
 * Args: filepath - the path of the file to test.
 *
 * Return:  1 if the file is considered as a tif file. Otherwise it returns 0.
 *
 * Note: In this implementation the function uses the extension of the file 
 *       only to test if it is a tif.
 */
int Is_Tiff(const char *filePath)
{
  if (String_Ends_With(filePath, "tif") ||
      String_Ends_With(filePath, "TIF") ||
      String_Ends_With(filePath, "TIFF") ||
      String_Ends_With(filePath, "tiff")) {
    return 1;
  }

  return 0;
}

static void sprint_file_bundle_sc(File_Bundle_S *bundle,int depth,char *sname)
{
  if(bundle->suffix) {
    sprintf(sname, "%s%0*d%s", bundle->prefix, bundle->num_width, 
	    bundle->first_num+depth,bundle->suffix);
  } else {
    sprintf(sname, "%s%0*d.tif", bundle->prefix, bundle->num_width,
	    bundle->first_num+depth);
  }  
}


/* Is_Fbdf(): Test if a file is file bundle (fbd) file.
 * 
 * Args: filepath - the path of the file to test.
 *
 * Return:  1 if the file is considered as a fbd file. Otherwise it returns 0.
 *
 * Note: In this implementation the function uses the extension of the file 
 *       only to test if it is a fbd.
 */
int Is_Fbdf(const char *filePath)
{
  if( strlen(filePath)>4 )
    if( strcmp(filePath+strlen(filePath)-4,".fbd")==0 || 
	strcmp(filePath+strlen(filePath)-4,".FBD")==0 )
      return 1;

  return 0;
}

int Is_Lsm(const char *filePath)
{
  if( strlen(filePath)>4 )
    if( strcmp(filePath+strlen(filePath)-4,".lsm")==0 || 
	strcmp(filePath+strlen(filePath)-4,".LSM")==0 )
      return 1;

  return 0;
}

int Is_Raw(const char *filePath)
{
  if(strlen(filePath) > 4) {
    if(strcmp(filePath+strlen(filePath)-4,".raw")==0 || 
        strcmp(filePath+strlen(filePath)-4,".RAW")==0) {
      return 1;
    } else {
      if (strlen(filePath) > 7) {
        if (strcmp(filePath + strlen(filePath) - 7, ".v3draw") == 0 ||
            strcmp(filePath + strlen(filePath) - 7, ".V3DRAW") == 0) {
          return 1;
        }
      }
    }
  }

  return 0;
}

int Is_V3dpbd(const char *filePath)
{
  if( strlen(filePath) > 7 ) {
    if( strcmp(filePath+strlen(filePath)-7,".v3dpbd")==0 || 
	strcmp(filePath+strlen(filePath)-7,".V3DPBD")==0 ) {
      return 1;
    }
  }

  if( strlen(filePath) >= 4 ) {
    if( strcmp(filePath+strlen(filePath)-4,".pbd")==0 || 
	strcmp(filePath+strlen(filePath)-4,".PBD")==0 ) {
      return 1;
    }
  }

  return 0;
}

int Is_Nsp(const char *filePath)
{
  if( strlen(filePath) >= 4 ) {
    if( strcmp(filePath+strlen(filePath)-4,".nsp")==0 || 
	strcmp(filePath+strlen(filePath)-4,".NSP")==0 ) {
      return 1;
    }
  }

  return 0;
}

#define FB_PREFIX_LENGTH 200
#define FB_SUFFIX_LENGTH 10

void initfb(File_Bundle_S *fb)
{
  fb->prefix = (char *) malloc(FB_PREFIX_LENGTH);
  fb->suffix = (char *) malloc(FB_SUFFIX_LENGTH);  
}

void freefb(File_Bundle_S *fb)
{
  free(fb->prefix);
  free(fb->suffix);
}

/* Load_Fbdf(): Load file bundle.
 *
 * Args: filePath - path of the file to read;
 *       fb - the pointer to store the result.
 *
 * Return: 1 if loading succeed or 0.
 *
 * Note: fb should be initialized before passing to the function.
 */
int Load_Fbdf(const char *filePath, File_Bundle_S *fb)
{
  FILE *fp = fopen(filePath, "r");
  if (fp == NULL)
    return 0;
  
  char *line;
  char *value, *var;
  size_t len = 0;
  int count = 0;

  while ( !feof(fp) ) {
    line = fgetln(fp, &len);
    if(len == 0)
      continue;
    line[len-1] = '\0'; /* turn a line o a string */

    value = line;

    #if defined(_WIN64) || defined(_WIN32) 
	
	var = strtok(value, "=");

	#else
    
	var = strsep(&value, "=");

	#endif

    if (value == NULL)
      continue;
    
    strtrim(var);
    strtrim(value);

    if (strcmp(var, "prefix") == 0) {
      strcpy(fb->prefix, value);
      count++;
    }
    
    if (strcmp(var, "suffix") == 0) {
      strcpy(fb->suffix, value);
      count++;
    }
    
    if (strcmp(var, "num_width") == 0) {
      fb->num_width = atoi(value);
      count++;
    }
    
    if (strcmp(var, "first_num") == 0) {
      fb->first_num = atoi(value);
      count++;
    }
  }
  
  if (count < 4)
    return 0; 
  else
    return 1;
}

void Raw_Size(const char *filepath, int *size)
{
  FILE *fp = Guarded_Fopen((char*) filepath, "rb", "Read_Raw_Stack_C");

  char formatkey[] = "raw_image_stack_by_hpeng";
  int lenkey = strlen(formatkey);   
  fread(formatkey, 1, lenkey, fp);

  if (strcmp(formatkey, "raw_image_stack_by_hpeng") != 0) {
    TZ_ERROR(ERROR_IO_FORMAT);
  }

  char endian;
  fread(&endian, 1, 1, fp);

  uint16_t dataType;
  char sz_buffer[16];

  fread(&dataType, 2, 1, fp);  
  fread(sz_buffer, 1, 8, fp);

  int i;
  for (i = 0; i < 3; i++) {
    size[i] = *((uint16_t*) (sz_buffer + i * 2));
  }

  if ((size[0] == 0) || (size[1] == 0) || (size[2] == 0)) {
    fread(sz_buffer + 8, 1, 8, fp);

    for (i = 0; i < 3; i++) {
      size[i] = *((uint32_t*) (sz_buffer + i * 4));
    }
  }
}

void Tiff_Size(const char *filepath, int *size)
{
  size[0] = 0;
  size[1] = 0;
  size[2] = 0;

#ifdef HAVE_LIBTIFF
  TIFF *tiff = Open_Tiff((char *)filepath, "r");
  if (tiff != NULL) {
    uint32_t width, height;
    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
    uint32_t depth = 1;
    while (TIFFReadDirectory(tiff)) {
      depth++;
    }
    Close_Tiff(tiff);

    size[0] = width;
    size[1] = height;
    size[2] = depth;
  }
#else
  Tiff_Reader *reader = Open_Tiff_Reader((char *) filepath,NULL,0);

  if (reader != NULL) {
    Tiff_IFD *ifd = Read_Tiff_IFD(reader);

    image_ifd_size(ifd, size);

    size[2] = 1;

    while (!End_Of_Tiff(reader)) {
      size[2]++;
      Advance_Tiff_Reader(reader);
    }

    Kill_Tiff_IFD(ifd);
    Kill_Tiff_Reader(reader);
  }
#endif
}

void Lsm_Size(const char *filepath, int *size)
{
  FILE *fp = fopen(filepath, "rb");
  
  fseek(fp, 4, SEEK_SET);

  uint32_t ifd_offset;
  fread(&ifd_offset, 4, 1, fp);

  fseek(fp, ifd_offset, SEEK_SET);

  uint16_t nifd;
  fread(&nifd, 2, 1, fp);

  uint16_t ifd_label;
  fread(&ifd_label, 2, 1, fp);
  
  uint16_t i;
  for (i = 1; i < nifd; i++) {
    if (ifd_label == TIF_CZ_LSMINFO) {
      break;
    }
    fseek(fp, 10, SEEK_CUR); 
    fread(&ifd_label, 2, 1, fp);
  }

  fseek(fp, 6, SEEK_CUR);

  fread(&ifd_offset, 4, 1, fp);
  
  fseek(fp, ifd_offset, SEEK_SET);
  Cz_Lsminfo lsminfo;
  fread(&lsminfo, sizeof(Cz_Lsminfo), 1, fp);

  size[0] = lsminfo.s32DimensionX;
  size[1] = lsminfo.s32DimensionY;
  size[2] = lsminfo.s32DimensionZ;

  fclose(fp);
}

void Stack_Size_F(const char *filepath, int *size)
{
  if (Is_Tiff(filepath)) {
    Tiff_Size(filepath, size);
  } else if (Is_Lsm(filepath)) {
    Lsm_Size(filepath, size);
  } else if (Is_Raw(filepath)) {
    Raw_Size(filepath, size);
  } else if (fhasext(filepath, "xml")) {
    Stack_Document *doc = Xml_Read_Stack_Document(filepath);
    switch (doc->type) {
    case STACK_DOC_TIF_FILE:
      Tiff_Size((char*) doc->ci, size);
      break;
    case STACK_DOC_LSM_FILE:
      Lsm_Size((char*) doc->ci, size);
      break;
    case STACK_DOC_FILE_BUNDLE:
      {
	File_Bundle_S *bundle = (File_Bundle_S*) doc->ci;
	char sname[1000];
	int depth = 0;
	while (1) { 
	  sprint_file_bundle_sc(bundle,depth,sname);
	  
	  if (!fexist(sname)) {
	    break;
	  }
    
	  depth++;
          if (bundle->first_num + depth - 1 == bundle->last_num) {
            break;
          }
	}

	if (depth > 0) {
	  sprint_file_bundle_sc(bundle, 0, sname);
	  Stack_Size_F(sname, size);
	}
	size[2] = depth;
      }
      break;
    }
    Kill_Stack_Document(doc);
  }
}

void Print_Tiff_Info(const char *filepath)
{
#ifdef HAVE_LIBTIFF
  TIFF *tiff = Open_Tiff((char *)filepath, "r");
  uint32_t width, height;
  TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height);
  uint32_t depth = 1;
  while (TIFFReadDirectory(tiff)) {
    depth++;
  }

  printf("width %u x height %u x depth %u\n", width, height, depth);

  uint16_t nbit, nsample;
  TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &nbit);
  TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &nsample);
  printf("Bits per sample: %u; Samples per pixel: %u\n", nbit, nsample);

  uint16 minv, maxv;
  int succ = TIFFGetField(tiff, TIFFTAG_MAXSAMPLEVALUE, &minv);
  if (succ) {
    succ = TIFFGetField(tiff, TIFFTAG_MINSAMPLEVALUE, &maxv);
  }
  if (succ) {
    printf("Value range: [%u, %u]\n", minv, maxv);
  }

  uint16 tf;
  succ = TIFFGetField(tiff, TIFFTAG_TRANSFERFUNCTION, &tf);
  if (succ) {
    printf("Transfer function: %u\n", tf);
  }

  char *description = NULL;
  succ = TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &description);
  if (succ) {
    printf("%s\n", description);
  }

  float x_res, y_res;
  succ = TIFFGetField(tiff, TIFFTAG_XRESOLUTION, &x_res);
  if (succ) {
    printf("x resolution: %g\n", x_res);
  }
  succ = TIFFGetField(tiff, TIFFTAG_YRESOLUTION, &y_res);
  if (succ) {
    printf("x resolution: %g\n", y_res);
  }

  Close_Tiff(tiff);
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif
}

void Print_Lsm_Info(const char *filepath)
{
  FILE *fp = fopen(filepath, "rb");
  
  char endian[3];

  fread(endian, 1, 2, fp);
  printf("Endian: %s\n", endian);
  
  uint16_t magic;
  fread(&magic, 2, 1, fp);
  printf("Magic number: %u\n", magic);

  uint32_t ifd_offset;
  fread(&ifd_offset, 4, 1, fp);
  printf("1st IFD offset: %u\n", ifd_offset); 

  fseek(fp, ifd_offset, SEEK_SET);

  uint16_t nifd;
  fread(&nifd, 2, 1, fp);
  printf("Number of IFD: %u\n", nifd); 

  uint16_t ifd_label;
  fread(&ifd_label, 2, 1, fp);
  
  uint16_t i;
  for (i = 1; i < nifd; i++) {
    if (ifd_label == TIF_CZ_LSMINFO) {
      break;
    }
    fseek(fp, 10, SEEK_CUR); 
    fread(&ifd_label, 2, 1, fp);
  }

  printf("IFD label: %u\n", ifd_label);
  uint16_t ifd_type;
  fread(&ifd_type, 2, 1, fp);
  printf("IFD type: %u\n", ifd_type);

  uint32_t ifd_length;
  fread(&ifd_length, 4, 1, fp);
  printf("IFD length: %u\n", ifd_length);

  fread(&ifd_offset, 4, 1, fp);
  printf("Lsm info offset: %u\n", ifd_offset);

  /*
  fseek(fp, ifd_offset + CZ_LSMINFO_DIMZ_OFFSET, SEEK_SET);
  int32_t dimz = 80;
  fwrite(&dimz, 4, 1, fp);
  printf("Number of slices: %d\n", dimz);
  */
  
  fseek(fp, ifd_offset, SEEK_SET);
  Cz_Lsminfo lsminfo;
  fread(&lsminfo, sizeof(Cz_Lsminfo), 1, fp);
  printf("Size: %d x %d x %d\n", lsminfo.s32DimensionX, lsminfo.s32DimensionY,
	 lsminfo.s32DimensionZ);

  printf("Intensity format: ");

  switch (lsminfo.u32DataType) {
  case 0:
    printf("Different data types for different channels\n");

    if (lsminfo.u32OffsetChannelDataTypes > 0) {
      uint32_t *channelDataTypes =
	(uint32_t*) Guarded_Malloc(sizeof(uint32_t) * 
				   lsminfo.s32DimensionChannels,
				   "Print_Lsm_Info");
      fseek(fp, lsminfo.u32OffsetChannelDataTypes, SEEK_SET);
      fread(channelDataTypes, sizeof(uint32_t), 
	    lsminfo.s32DimensionChannels, fp);
      printf("%u %u %u\n", channelDataTypes[0], channelDataTypes[1], 
	     channelDataTypes[2]);
    }   
    break;
  case 1:
    printf("8-bit unsigned integer\n");
    break;
  case 2:
    printf("12-bit unsigned integer\n");
    break;
  case 5:
    printf("32-bit float\n");
    break;
  default:
    printf("unknown format.\n");
  }

  printf("Resolution: %g x %g x %g um\n", lsminfo.f64VoxelSizeX * 1000000.0,
	 lsminfo.f64VoxelSizeY * 1000000.0, 
	 lsminfo.f64VoxelSizeZ * 1000000.0);
  
  printf("Scan type: ");
  switch (lsminfo.u16ScanType) {
  case 0:
    printf("normal x-y-z scan\n");
    break;
  case 1: 
    printf("z-Scan\n");
    break;
  case 2:
    printf("line scan\n");
    break;
  case 3:
    printf("time series x-y\n");
    break;
  case 4:
    printf("time series x-z\n");
    break;
  case 5:
    printf("time series Mean of ROIs\n");
    break;
  case 6:
    printf("time series x-y-z\n");
    break;
  case 7:
    printf("spline scan\n");
    break;
  case 8:
    printf("spline plane x-z\n");
    break;
  case 9:
    printf("time series spline plane x-z\n");
    break;
  case 10: 
    printf("point mode\n");
    break;
  default:
    printf("unknown scan mode\n");
  }

  switch (lsminfo.u32DataType) {
  case 0:
    printf("original scan data\n");
    break;
  case 1:
    printf("calculated data\n");
    break;
  case 2:
    printf("animation\n");
    break;
  default:
    printf("unknown data type\n");
  }

  if (lsminfo.u32OffsetVectorOverlay > 0) {
    printf("Vector overlay found\n");
  }

  if (lsminfo.u32OffsetScanInformation > 0) {
    printf("Scan information found.\n");
  }

 if (lsminfo.u32OffsetInputLut == 0) {
    printf("There is no input LUT\n");
  } else {
    printf("Input LUT found\n");
  }

  if (lsminfo.u32OffsetOutputLut == 0) {
    printf("There is no color palette\n");
  } else {
    printf("Color palette found\n");
  }

  if (lsminfo.u32OffsetChannelColors == 0) {
    printf("There is no channel color or channel name\n");
  } else {
    printf("Channel colors and channel names fournd\n");
  }

  if (lsminfo.f64TimeInterval == 0) {
    printf("There is no time interval\n");
  } else {
    printf("Time interval: %lg sec\n", lsminfo.f64TimeInterval);
  }

  if (lsminfo.u32OffsetScanInformation == 0) {
    printf("There is no information about devide settings\n");
  } else {
    printf("Scan information found. Offset: %u\n", 
	   lsminfo.u32OffsetScanInformation);
  }
  
  if (lsminfo.u32OffsetKsData == 0) {
    printf("There is no Zeiss Vision KS-3d data\n");
  } else {
    printf("Zeiss Vision KS-3d data found. Offset: %u\n",
	   lsminfo.u32OffsetKsData);
  }

  if (lsminfo.u32OffsetTimeStamps == 0) {
    printf("It is not a time series\n");
  } else {
    printf("Time stamps found\n");
  }

  if (lsminfo.u32OffsetRoi == 0) {
    printf("There is no ROI\n");
  } else {
    printf("ROI found\n");
  }

  if (lsminfo.u32OffsetBleachRoi == 0) {
    printf("There is no bleach roi.\n");
  } else {
    printf("Bleach ROI found\n");
  }

  if (lsminfo.u32OffsetNextRecording == 0) {
    printf("There is no second image\n");
  } else {
    printf("Second image found\n");
  }

  printf("Display aspect: %g x %g x %g x %g", lsminfo.f64DisplayAspectX,
	 lsminfo.f64DisplayAspectY, lsminfo.f64DisplayAspectZ,
	 lsminfo.f64DisplayAspectTime);
  
  if (lsminfo.u32OffsetMeanOfRoisOverlay == 0) {
    printf("No ROI vector overlay.\n");
  } else {
    printf("ROI vector overlay found.\n");
  }

  if (lsminfo.u32OffsetTopolsolineOverlay == 0) {
    printf("There is no vector overlay for topography-iso-lines.\n");
  } else {
    printf("Vector overlay for topography-iso-lines found.\n");
  }

  if (lsminfo.u32OffsetTopoProfileOverlay == 0) {
    printf("There is no vector overlay for topography-profile.\n");
  } else {
    printf("Vector overlay for topography-iso-lines found.\n");
  }

  if (lsminfo.u32OffsetLinescanOverlay == 0) {
    printf("There is no vector overlay for line scan.\n");
  } else {
    printf("Vector overlay for line scan found.\n");
  }

  if ((lsminfo.u32ToolbarFlags & 0x00000001) > 0) {
    printf("Corp button disabled.\n");
  }

  if ((lsminfo.u32ToolbarFlags & 0x00000002) > 0) {
    printf("Reuse button disabled.\n");
  }

  if (lsminfo.u32OffsetChannelWavelength == 0) {
    printf("There no wavelength range information.\n");
  } else {
    printf("Wavelength range found.\n");
  }
  
  printf("The inverse radius of the spherical error: %g\n", 
	 lsminfo.f64objectiveSphereCorrection);

  if (lsminfo.u32OffsetunmixParameters == 0) {
    printf("There is no parameter for linear unmixing.\n");
  } else {
    printf("Linear unmixing parameters found.\n");
  }

  fclose(fp);
}

Stack *Read_Lsm_Stack(const char *filepath, int channel)
{  
  if (!fexist(filepath)) {
    return NULL;
  }

  int width = 0;
  int height = 0;
  int depth = 0;
  int kind = 0;
  int nchannel = 0;

  Tiff_Image *image = NULL;
  Tiff_IFD *ifd = NULL;

  { Tiff_Reader *reader;

    reader = Open_Tiff_Reader((char *) filepath,NULL,1);

    while (lsm_thumbnail_flag(ifd = Read_Tiff_IFD(reader)) != 0) {
      Free_Tiff_IFD(ifd);
      Advance_Tiff_Reader(reader);
      if (End_Of_Tiff(reader)) {
	ifd = NULL;
	TZ_ERROR(ERROR_IO_READ);
	break;
      }
    }

    //Print_Tiff_IFD(ifd, stdout);

    image = Extract_Image_From_IFD(ifd);

    if (image == NULL) {
      TZ_ERROR(ERROR_POINTER_NULL);
    } else {
      width = image->width;
      height = image->height;
      kind = image->channels[0]->bytes_per_pixel;
      
      depth  = 1;
      while (!End_Of_Tiff(reader)) {
	ifd = Read_Tiff_IFD(reader);
	if (lsm_thumbnail_flag(ifd) == 0) {
	  depth++;
	}
	Free_Tiff_IFD(ifd);
      }
    }

    Free_Tiff_Reader(reader);
    
    nchannel = image->number_channels;

    if (image->number_channels > 1) {
      if (channel < 0) {
	if (kind == 1) {
	  kind = 3;
	} else {
	  TZ_ERROR(ERROR_DATA_TYPE);
	}
      }
    }
    Free_Tiff_Image(image);
  }

  Stack *stack = Make_Stack(kind, width, height, depth);
  size_t bytes_per_plane = kind * width * height;
  size_t offset = 0;

  Tiff_Reader *tif = Open_Tiff_Reader((char *) filepath, NULL, 1);
  
  while (!End_Of_Tiff(tif)) {
    ifd = Read_Tiff_IFD(tif);
    if (lsm_thumbnail_flag(ifd) == 0) {
      image = Extract_Image_From_IFD(ifd);
      if (image == NULL) {
	TZ_ERROR(ERROR_POINTER_NULL);
      }

      if ((kind != 3) || (channel >= 0)) {
	if (channel < 0) {
	  channel = 0;
	}

	memcpy(stack->array + offset, image->channels[channel]->plane, 
	       bytes_per_plane);
	offset += bytes_per_plane;
      } else {
	color_t *arrayc = (color_t *) stack->array;
	size_t offset2 = 0;
	int i;
	int j;
	for (j = 0; j < stack->height; j++) {
	  for (i = 0; i < stack->width; i++) {
	    arrayc[offset][0] = ((uint8 *) image->channels[0]->plane)[offset2];
	    arrayc[offset][1] = ((uint8 *) image->channels[1]->plane)[offset2];
	    if (nchannel > 2) {
	      arrayc[offset][2] = ((uint8 *) image->channels[2]->plane)[offset2];
	    }
	    offset++;
	    offset2++;
	  }
	}
      }
      Free_Tiff_Image(image);
    }
    Free_Tiff_IFD(ifd);
  }

#ifdef _MSC_VER
  Reset_Tiff_Image();
#else
  Reset_Tiff_Image(image);
#endif
  Reset_Tiff_IFD();
  Kill_Tiff_Reader(tif);

  return stack;
}

int Lsm_Channel_Number(const char *filepath)
{
  int nchannel = 0;
  Tiff_Image *image = NULL;
  Tiff_IFD *ifd = NULL;
  Tiff_Reader *reader = Open_Tiff_Reader((char*) filepath, NULL, 1);
  while (lsm_thumbnail_flag(ifd = Read_Tiff_IFD(reader)) != 0) {
    Free_Tiff_IFD(ifd);
    Advance_Tiff_Reader(reader);
    if (End_Of_Tiff(reader)) {
      ifd = NULL;
      TZ_ERROR(ERROR_IO_READ);
      break;
    }
  }

  image = Extract_Image_From_IFD(ifd);

  if (image == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  nchannel = image->number_channels;
  Kill_Tiff_Image(image);
  Free_Tiff_Reader(reader);

  return nchannel;
}

int Lsm_Pixel_Type(const char *filepath)
{
  int type = 0;
  Tiff_Image *image = NULL;
  Tiff_IFD *ifd = NULL;
  Tiff_Reader *reader = Open_Tiff_Reader((char*) filepath, NULL, 1);
  while (lsm_thumbnail_flag(ifd = Read_Tiff_IFD(reader)) != 0) {
    Free_Tiff_IFD(ifd);
    Advance_Tiff_Reader(reader);
    if (End_Of_Tiff(reader)) {
      ifd = NULL;
      TZ_ERROR(ERROR_IO_READ);
      break;
    }
  }

  image = Extract_Image_From_IFD(ifd);

  if (image == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  }

  type = UNKNOWN_IMAGE_KIND;

  switch (image->channels[0]->type) {
    case CHAN_UNSIGNED:
      switch (image->channels[0]->bytes_per_pixel) {
        case 1:
          type = GREY8;
          break;
        case 2:
          type = GREY16;
          break;
        default:
          break;
      }
      break;
    case CHAN_SIGNED:
      switch (image->channels[0]->bytes_per_pixel) {
        case 1:
          type = SGREY;
          break;
        case 2:
          type = SGREY16;
          break;
        default:
          break;
      }
      break;
    case CHAN_FLOAT:
      type = FLOAT32;
      break;
    default:
      break;
  }

  Kill_Tiff_Image(image);
  Free_Tiff_Reader(reader);

  return type;
}

void Write_Lsm_Stack(const char *filepath, const Stack *stack, 
		     Tiff_IFD *templat)
{
  Tiff_Writer *tif = Open_Tiff_Writer((char *) filepath, 1);

  Tiff_IFD *ifd = NULL;
  int i;
  for (i = 0; i < stack->depth; i++) {
    Tiff_Image *image = Stack_Tiff_Image(stack, i, NULL);

    if (i == 0) {
      ifd = Make_IFD_For_Lsm_Image(image, 0, templat, stack->depth);
    } else {
      ifd = Make_IFD_For_Lsm_Image(image, 0, NULL, stack->depth);
    }
    
#ifdef _DEBUG_
    if (i == 0) {
      Print_Tiff_IFD(ifd, stdout);
    }
#endif 
    Write_Tiff_IFD(tif, ifd);

    Free_Tiff_Image(image);
    Free_Tiff_IFD(ifd);
  }

  Reset_Tiff_IFD();
  Reset_Tiff_Image();

  Close_Tiff_Writer(tif);
  Kill_Tiff_Writer(tif);
}

/*
void Write_Lsm_Stack(const char *filepath, const Stack *stack, Tiff_IFD *ifd)
{
  Tiff_Writer *tif = Open_Tiff_Writer((char *) filepath, 1);

  int i;
  for (i = 0; i < stack->depth; i++) {
    Tiff_Image *image = Stack_Tiff_Image(stack, i, NULL);
    //Tiff_Image *image = NULL;
    Make_IFD_For_Image_I(image, 0, ifd);
    
#ifdef _DEBUG_2
    if (i == 0) {
      Print_Tiff_IFD(ifd, stdout);
    }
#endif 
    Write_Tiff_IFD(tif, ifd);
    Free_Tiff_Image(image);
  }

  Reset_Tiff_Image();

  Close_Tiff_Writer(tif);
  Kill_Tiff_Writer(tif);
}
*/

void Write_Raw_Stack(const char *filepath, const Stack *stack)
{
  FILE *fp = Guarded_Fopen((char*) filepath, "wb", "Write_Raw_Stack");

  char formatkey[] = "raw_image_stack_by_hpeng";
  int lenkey = strlen(formatkey); 	
  fwrite(formatkey, 1, lenkey, fp);

  char endian = 'L';
  fwrite(&endian, 1, 1, fp);

  uint16_t dataType = stack->kind;
  uint32_t sz[4];
  sz[0] = stack->width;
  sz[1] = stack->height;
  sz[2] = stack->depth;
  sz[3] = 1;

  if (stack->kind == COLOR) {
    dataType = 1;
    sz[3] = 3;
  }

  fwrite(&dataType, 2, 1, fp);  
  fwrite(sz, 4, 4, fp);
  
  size_t nvoxel = Stack_Voxel_Number(stack);
  if (stack->kind != COLOR) {
    //fwrite(stack->array, dataType, nvoxel, fp);
    //use 1G buffer fix fwrite bug for big file 2012/5/31
    size_t buffersize = (size_t) 1024 * 1024 * 1024;
    size_t remaining = nvoxel * dataType;
    int j = 0;
    size_t nwrite = 0;
    fclose(fp);
    while (remaining > 0) {
      fp = Guarded_Fopen((char*) filepath, "ab", "Write_Raw_Stack");
      if (remaining / buffersize > 0) {
        nwrite += fwrite(stack->array + j * buffersize, 1, buffersize,
                fp);
        ++j;
        remaining -= buffersize;
      } else {
        nwrite += fwrite(stack->array + j * buffersize, 1, remaining, fp);
        remaining = 0;
      }
      fclose(fp);
    }

    if (ferror(fp) || feof(fp)) {
      printf("num of voxel: %zd\nactual written voxel: %zd\n", nvoxel, nwrite);
      perror("fwrite");
      printf("ferror: %d  feof: %d\n", ferror(fp), feof(fp));
      fclose(fp);
      TZ_ERROR(ERROR_IO_WRITE);
    }
  } else {
    size_t i;
    color_t *arrayc = (color_t*) stack->array;
    for (i = 0; i < 3; i++) {
      size_t j;
      for (j = 0; j < nvoxel; j++) {
	fwrite(arrayc[j] + i, dataType, 1, fp);
      }
    }
    fclose(fp);
  }
  
}

void Write_Flyem_Raw_Stack_Plane(const char *filepath, const Stack *stack)
{
  FILE *fp = Guarded_Fopen((char*) filepath, "wb", 
      "Write_Flyem_Raw_Stack_Plane");

  if ((stack->kind != GREY) || (stack->depth != 1)) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  uint32_t sz[2];
  sz[0] = stack->width;
  sz[1] = stack->height;

  fwrite(sz, 4, 2, fp);
  
  size_t nvoxel = Stack_Voxel_Number(stack);
  fwrite(stack->array, 1, nvoxel, fp);
  
  fclose(fp);
}

Stack* Read_Flyem_Raw_Stack_Plane(const char *filepath)
{
  FILE *fp = Guarded_Fopen((char*) filepath, "rb", 
      "Read_Flyem_Raw_Stack_Plane");

  uint32_t sz[2];
  fread(sz, 4, 2, fp);

  Stack *stack = Make_Stack(GREY, sz[0], sz[1], 1);
  
  size_t nvoxel = Stack_Voxel_Number(stack);
  fread(stack->array, 1, nvoxel, fp);
  
  fclose(fp);

  return stack;
}

Stack* Read_Raw_Stack_C(const char *filepath, int channel)
{
  FILE *fp = Guarded_Fopen((char*) filepath, "rb", "Read_Raw_Stack_C");

  char formatkey[] = "raw_image_stack_by_hpeng";
  int lenkey = strlen(formatkey); 	
  fread(formatkey, 1, lenkey, fp);

  if (strcmp(formatkey, "raw_image_stack_by_hpeng") != 0) {
    TZ_ERROR(ERROR_IO_FORMAT);
  }

  char endian;
  fread(&endian, 1, 1, fp);

  uint16_t dataType;
  char sz_buffer[16];
  uint32_t sz[4];

  fread(&dataType, 2, 1, fp);  
  fread(sz_buffer, 1, 8, fp);

  int i;
  for (i = 0; i < 4; i++) {
    sz[i] = *((uint16_t*) (sz_buffer + i * 2));
  }

  if ((sz[0] == 0) || (sz[1] == 0) || (sz[2] == 0) || (sz[3] == 0)) {
    fread(sz_buffer + 8, 1, 8, fp);

    for (i = 0; i < 4; i++) {
      sz[i] = *((uint32_t*) (sz_buffer + i * 4));
    }
  }

  if (sz[3] != 1) {
    if (channel >= 0 && channel < sz[3]) { //only one channel, good to go
    } else {
      if ((sz[3] != 3) || (dataType != 1) || (channel + 1 > sz[3])) {
        PRINT_EXCEPTION("unsupported format",
            "multi-channel image can not be read");
        fclose(fp);
        return NULL;
      } else {
        if (channel < 0) {
          dataType = 3;
        }
      }
    }
  }

  //size_t total_size = (size_t) sz[0] * sz[1] * sz[2] * dataType;

  Stack *stack = Make_Stack(dataType, sz[0], sz[1], sz[2]);;
  //if (total_size < 1024 * 1024 * 1600) {
  //  stack = Make_Stack(dataType, sz[0], sz[1], sz[2]);
  //} else {                                      // this part will cause Kill_Stack() crash
  //  GUARDED_MALLOC_ARRAY(stack, 1, Stack);
  //  stack->width = sz[0];
  //  stack->height = sz[1];
  //  stack->depth = sz[2];
  //  stack->kind = dataType;
  //  stack->array = (uint8*) Guarded_Malloc(total_size, "Read_Raw_Stack_C");
  //  stack->text = (char*) malloc(1);
  //  stack->text[0] = '\0';
  //}

  size_t nvoxel = Stack_Voxel_Number(stack);
  
  if (dataType == 3) {
    size_t i;
    color_t *arrayc = (color_t*) stack->array;
    for (i = 0; i < 3; i++) {
      size_t j;
      for (j = 0; j < nvoxel; j++) {
	fread(arrayc[j] + i, 1, 1, fp);
      }
    }
  } else {
    if (sz[3] == 1) {
         //use 1G buffer fix fread bug for big file 2011/11/17
        size_t buffersize = (size_t) 1024 * 1024 * 1024;
        size_t remaining = nvoxel * dataType;
        int j = 0;
        size_t nread = 0;
        while (remaining > 0) {
          if (remaining / buffersize > 0) {
            nread += fread(stack->array + j * buffersize, 1, buffersize,
                fp);
            ++j;
            remaining -= buffersize;
          } else {
             nread += fread(stack->array + j * buffersize, 1, remaining, fp);
             remaining = 0;
          }
        }

        if (ferror(fp) || feof(fp)) {
          printf("num of voxel: %zd\nactual readed voxel: %zd\n", nvoxel, nread);
          perror("fread");
          printf("ferror: %d  feof: %d\n", ferror(fp), feof(fp));
          fclose(fp);
          TZ_ERROR(ERROR_IO_READ);
        }
      //fread(stack->array, dataType, nvoxel, fp);
    } else {
      fseek(fp, nvoxel * dataType * channel, SEEK_CUR);
      //use 1G buffer fix fread bug for big file 2011/11/17
        size_t buffersize = (size_t) 1024 * 1024 * 1024;
        size_t remaining = nvoxel * dataType;
        int j = 0;
        size_t nread = 0;
        while (remaining > 0) {
          if (remaining / buffersize > 0) {
            nread += fread(stack->array + j * buffersize, 1, buffersize,
                fp);
            ++j;
            remaining -= buffersize;
          } else {
             nread += fread(stack->array + j * buffersize, 1, remaining, fp);
             remaining = 0;
          }
        }

         if (ferror(fp) || feof(fp)) {
          printf("num of voxel: %zd\nactual readed voxel: %zd\n", nvoxel, nread);
           perror("fread");
          printf("ferror: %d  feof: %d\n", ferror(fp), feof(fp));
          fclose(fp);
          TZ_ERROR(ERROR_IO_READ);
        }
      //fread(stack->array, dataType, nvoxel, fp);
    }
  }

  fclose(fp);

  return stack;
}

Stack* Read_Raw_Stack(const char *filepath)
{
  return Read_Raw_Stack_C(filepath, -1);
}

Stack* Read_Raw_Stack_Slice(const char *filepath, int slice)
{
  FILE *fp = Guarded_Fopen((char*) filepath, "rb", "Read_Raw_Stack_C");

  char formatkey[] = "raw_image_stack_by_hpeng";
  int lenkey = strlen(formatkey); 	
  fread(formatkey, 1, lenkey, fp);

  if (strcmp(formatkey, "raw_image_stack_by_hpeng") != 0) {
    TZ_ERROR(ERROR_IO_FORMAT);
  }

  char endian;
  fread(&endian, 1, 1, fp);

  uint16_t dataType;
  char sz_buffer[16];
  uint32_t sz[4];

  fread(&dataType, 2, 1, fp);  
  fread(sz_buffer, 1, 8, fp);

  int i;
  for (i = 0; i < 4; i++) {
    sz[i] = *((uint16_t*) (sz_buffer + i * 2));
  }

  if ((sz[0] == 0) || (sz[1] == 0) || (sz[2] == 0) || (sz[3] == 0)) {
    fread(sz_buffer + 8, 1, 8, fp);

    for (i = 0; i < 4; i++) {
      sz[i] = *((uint32_t*) (sz_buffer + i * 4));
    }
  }
  
  if (sz[3] != 1) {
    if ((sz[3] != 3) || (dataType != 1)) {
      PRINT_EXCEPTION("unsupported format", 
		      "multi-channel image can not be read");
      fclose(fp);
      return NULL;
    } else {
      dataType = 3;
    }
  }

  size_t slice_size = (size_t) sz[0] * sz[1];
  size_t channel_size = slice_size * sz[2];

  Stack *stack = Make_Stack(dataType, sz[0], sz[1], 1); 

  size_t nvoxel = Stack_Voxel_Number(stack);
  
  if (dataType == 3) {
    color_t *arrayc = (color_t*) stack->array;
    /* Read the first channel. */
    fseek(fp, slice_size * slice, SEEK_CUR); 
    size_t j;
    for (j = 0; j < nvoxel; j++) {
      fread(arrayc[j], 1, 1, fp);
    }
    /* Read the second channel. */
    fseek(fp, channel_size - slice_size, SEEK_CUR);
    for (j = 0; j < nvoxel; j++) {
      fread(arrayc[j]+1, 1, 1, fp);
    }
    /* Read the third channel. */
    fseek(fp, channel_size - slice_size, SEEK_CUR);
    for (j = 0; j < nvoxel; j++) {
      fread(arrayc[j]+2, 1, 1, fp);
    }
  } else {
    fseek(fp, slice_size * slice, SEEK_CUR);
    fread(stack->array, dataType, nvoxel, fp);
  }

  fclose(fp);

  return stack;
}

Stack* Read_Stack_U(const char *filepath)
{
  if (!fexist(filepath)) {
    fprintf(stderr, "The file %s cannot be found.\n", filepath);
    return NULL;
  }

  if (Is_Lsm(filepath)) {
    return Read_Lsm_Stack(filepath, -1);
  } else if (Is_Fbdf(filepath)) {
    File_Bundle_S fb;
    Load_Fbdf(filepath, &fb);
    return Read_Stack_Planes_S(&fb);
  } else if (Is_Raw(filepath)) {
    ///// small memory footprint
    //return Read_Raw_Stack(filepath);
    ///// fast version
    Mc_Stack *mc_stack = Read_Mc_Stack(filepath, -1);
    Stack *stack = NULL;
    if (mc_stack->nchannel == 1) {
      stack = Mc_Stack_To_Stack(mc_stack, mc_stack->kind, NULL);
    } else if (mc_stack->kind == 1 && mc_stack->nchannel == 3) {
      stack = Mc_Stack_To_Stack(mc_stack, 3, NULL);
    }
    Kill_Mc_Stack(mc_stack);
    return stack;
  } else if (fhasext(filepath, "xml")) {
    return Read_Xml_Stack(filepath);
  } else if (fhasext(filepath, "png")) {
    return Read_Png(filepath);
  } else if (Is_Nsp(filepath)) {
    int numchans = Infer_Neuron_File_Channel_Number(filepath);
    Neurons *neurons = Read_Neurons(filepath, numchans);
    
    Stack *stack = Neuron_To_Stack(neurons, NULL);

    return stack;
  } else {
    return Read_Stack((char*) filepath);
  }
}

Stack* Read_Xml_Stack(const char *filePath)
{
#ifdef HAVE_LIBXML2
  Stack_Document *stack_doc = Xml_Read_Stack_Document(filePath);
  Stack *stack = NULL;
  if (stack_doc != NULL) {
    stack = Import_Stack_Document(stack_doc);
    Kill_Stack_Document(stack_doc);
  }
  return stack;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

void Write_Stack_U(const char *filepath, const Stack *stack, 
		   const char *metafile)
{
  if (Is_Raw(filepath)) {
    Write_Raw_Stack(filepath, stack);
  } else if (Is_Lsm(filepath)) {
    Tiff_IFD *ifd = NULL;
    if (metafile != NULL) {
      Tiff_Reader *reader;
    
      reader = Open_Tiff_Reader((char*) metafile,NULL,1);
      
      while (lsm_thumbnail_flag(ifd = Read_Tiff_IFD(reader)) != 0) {
	Kill_Tiff_IFD(ifd);
	if (End_Of_Tiff(reader)) {
	  ifd = NULL;
	  TZ_ERROR(ERROR_IO_READ);
	  break;
	}
      }
      Kill_Tiff_Reader(reader);
    }
    Write_Lsm_Stack((char*) filepath, stack, ifd);
    Kill_Tiff_IFD(ifd);
  } else {
    Stack tmp_stack = *stack;
    if (stack->text == NULL) {
      tmp_stack.text = (char*) "";
    }
    Write_Stack((char*) filepath, &tmp_stack);
  }  
}

typedef uint64_t uint64;
typedef int64_t int64;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int16_t int16;

static char machine_endian()
{ uint32 x;
  uint8 *b;

  b = (uint8 *) (&x);
  x = 3;
  if (b[0] != 3)
    return ('B');
  else
    return ('L');
  return (b[0] != 3);
}

static int swap2(int16 *w)
{ uint8 *v = (uint8 *) w;
  uint8  x;

  x    = v[0];
  v[0] = v[1];
  v[1] = x;
  return (*w);
}

static int swap4(int32 *w)
{ uint8 *v = (uint8 *) w;
  uint8  x;

  x    = v[0];
  v[0] = v[3];
  v[3] = x;
  x    = v[1];
  v[1] = v[2];
  v[2] = x;
  return (*w);
}

static int64 swap8(int64 *w)
{ uint8 *v = (uint8 *) w;
  uint8  x;

  x    = v[0];
  v[0] = v[7];
  v[7] = x;
  x    = v[1];
  v[1] = v[6];
  v[6] = x;
  x    = v[2];
  v[2] = v[5];
  v[5] = x;
  x    = v[3];
  v[3] = v[4];
  v[4] = x;
  return (*w);
}

#define IO_BUFFER  100000
#define UNIQUE      0   //  Block designators
#define RUNLEN      1
#define DIFFBLOCK   2
#define DIFFBLOCK3  2
#define DIFFBLOCK4  3
#define DIFFBLOCK5  4
#define UNIQUE_SWAP 5
#define RUN_SWAP    6

#define FWRITE(p,s,n,f) ((int64) fwrite(p,s,n,f))

#if 0
static int64 compress_write8(FILE *file, uint8 *source, int64 size)
{ static int Deff[96] = { 0, 1, 2, 3, 4, 4, 4, 5, 6, 5,
                          5, 6, 6, 6, 6, 6, 7, 6, 6, 7,  // Deff[i] = ceil(2i/(1+ceil(i/4)))
                          7, 6, 7, 7, 7, 7, 7, 7, 7, 7,
                          7, 7, 8, 7, 7, 7, 8, 7, 7, 8,
                          8, 7, 7, 8, 8, 7, 8, 8, 8, 7,
                          8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                          8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                          8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                          8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                          8, 8, 8, 8, 8, 8
                        };

  uint8 target[IO_BUFFER + 100];   //  A fixed amount of working storages is needed !
  int   dbuffer[96];

  int64 i  = 0;        //  index into source stream 'source'
  int64 p  = 0;        //  index into compression buffer 'target'
  int   pv = -3;       //  prior value
  int   pu = 1;        //  prior block is *not* a half-filled unique block?
  int64 du;            //  index to unique block header if 'pu' is false
  int64 cl = 0;        //  compression length (accumulated whenever buffer dumped)

  while (i < size)
    { int rlen, dlen, act;

      int cv = source[i];

      { int64 cp, cmax = i+128;     //   Compute longest runlength from this point
        if (cmax > size)
          cmax = size;
        rlen = 1;
        for (cp = i+1; cp < cmax; cp++)
          { if (source[cp] != cv)
              break;
            rlen += 1;
          }
      }

      if (rlen >= 8)
        act = RUNLEN;    //  Nothing can do better than runlength at compression ratio >= 4
      else
        { int64 cp, cmax = i+95;     //   Compute longest diffblock from this point
          if (size < cmax)
            cmax = size;
          dlen = 0;
          for (cp = i; cp < cmax; cp++)
            { int nv = source[cp];
              int df = (nv - pv) + 1;
              if (df < 0 || df > 3)
                break;
              dbuffer[dlen++] = df;
              pv = nv;
            }

          if (Deff[dlen] > rlen)      //  Decide on which encoding block to use next
            if (dlen >= 3 - pu)          //  2-bit diff is strictly better than run length
              act = DIFFBLOCK;
            else
              act = UNIQUE;
          else
            if (rlen >= 3 - pu)          //  Run length is as good or better than 2-bit diff
              act = RUNLEN;
            else
              act = UNIQUE;
        }

#ifdef SHOW
      if (!pu && (act != UNIQUE || target[du] >= 31))
        { int j = target[du] + 1;
          printf("UNQ %3d:",j + (act == UNIQUE));
          for (; j > 0; j--)
            printf(" %2d",source[i-j]);
          if (act == UNIQUE)
            printf(" %2d",cv);
          printf("\n");
        }
#endif

      switch (act)             //  Proceed with encoding selected block
      { case UNIQUE:
          if (pu)
            target[du = p++] = pu = 0;
          else
            pu = (++target[du] >= 32);
          target[p++] = pv = cv;
          i += 1;
          break;
        case RUNLEN:
#ifdef SHOW
          printf("RUN %3d: %2d\n",rlen,cv);
#endif
          target[p++] = rlen+127;
          target[p++] = pv = cv;
          pu = 1;
          i += rlen;
          break;
        case DIFFBLOCK:
#ifdef SHOW
          printf("DIF %3d:",dlen);
#endif
          target[p++] = dlen+32;
          { int j = 0;
            while (j < dlen)
              { int d0 = dbuffer[j++];
                int d1 = dbuffer[j++];
                int d2 = dbuffer[j++];
                int d3 = dbuffer[j++];
                target[p++] = (d3 << 6) | (d2 << 4) | (d1 << 2) | d0;
              }
#ifdef SHOW
            for (j = 0; j < dlen; j++)
              printf(" %2d",dbuffer[j]-1);
            printf("\n");
#endif
          }
          // No need to set pv, it was already correctly set when computing dlen
          pu = 1;
          i += dlen;
          break;
      }

      //  p can never be larger than IO_BUFFER + max unique block + max (diff block or run block)
      //     which is IO_BUFFER + 35 + 24 = IO_BUFFER + 59 < IO_BUFFER+100 (the size of the buffer)

      if (p > IO_BUFFER && pu)
        { if (FWRITE(target, 1, p, file) != p)
            { fprintf(stderr,"Cannot write compressed data!\n");
              exit (0);
            }
          cl += p;
          p   = 0;
        }
    }

#ifdef SHOW
  if (!pu)
    { int j = target[du] + 1;
      printf("UNQ %3d:",j);
      for (; j > 0; j--)
        printf(" %2d",source[i-j]);
      printf("\n");
    }
#endif

  if (p > 0)
    { if (FWRITE(target, 1, p, file) != p)
        { fprintf(stderr,"Cannot write compressed data!\n");
          exit (0);
        }
      cl += p;
    }

  return (cl);
}

static int64 compress_write16(FILE *file, uint16 *source, int64 size)
{
  uint8 target[IO_BUFFER + 200];   //  A fixed amount of working storages is needed !
  int   dbuffer[104];

  int64 i  = 0;        //  index into source stream 'source'
  int64 p  = 0;        //  index into compression buffer 'target'
  int   pv = -17;      //  prior value
  int   pu = 1;        //  prio block is *not* a half-filled unique block?
  int   du;            //  index of unique block header if 'pu' is false
  int64 cl = 0;        //  compression length (accumulated whenever buffer dumped)

#ifdef SHOW
  printf("NEW STREAM\n");
#endif

  while (i < size)
    { int rlen, dlen3, dlen4, dlen5, act;

      int cv = source[i];

      { int64 cp, cmax = i+33;     //   Compute longest runlength from this point
        if (cmax > size)
          cmax = size;
        rlen = 1;
        for (cp = i+1; cp < cmax; cp++)
          { if (source[cp] != cv)
              break;
            rlen += 1;
          }
      }

      if (rlen >= 11)
        act = RUNLEN;    //  Nothing can do better than runlength at compression ratio >= 5.333
      else
        { int64 cp, cmax = i+103;     //   Compute longest  3-, 4-, and 5-diffblocks from this point
          if (size < cmax)
            cmax = size;
          dlen3 = dlen4 = dlen5 = 0;
          for (cp = i; cp < cmax; cp++)
            { int nv = source[cp];
              int df = (nv - pv) + 15;
              if (df < 0 || df > 31)
                break;
              if (dlen4 == dlen5 && df >= 8 && df <= 23)
                { if (dlen3 == dlen4 && df >= 12 && df <= 19)
                    dlen3 += 1;
                  dlen4 += 1;
                }
              dbuffer[dlen5++] = df;
              pv = nv;
            }
          if (dlen3 > 48) dlen3 = 48;
          if (dlen5 > 40) dlen5 = 40;

#ifdef SHOW
          // printf("Lengths: %d %d %d %d\n",rlen,dlen3,dlen4,dlen5);
#endif

          { double de, best = rlen/3.;   //   NB: 1/2 the actual efficiency!
            act  = RUNLEN;
            if ((de = dlen3/((dlen3*3+7)/8 + 1.)) > best)
              { best = de; act = DIFFBLOCK3; }
            if ((de = dlen4/((dlen4*4+7)/8 + 1.)) > best)
              { best = de; act = DIFFBLOCK4; }
            if ((de = dlen5/((dlen5*5+7)/8 + 1.)) > best)
              { best = de; act = DIFFBLOCK5; }
            if (pu)
              { if (best <= .3334)
                  act = UNIQUE;
              }
            else
              { if (best <= .5001)
                  act = UNIQUE;
              }
          }
        }

#ifdef SHOW
      if (!pu && (act != UNIQUE || target[du] >= 30))
        { int j = target[du] + 1;
          printf("UNQ (%lld) %3d:",i-j,j + (act == UNIQUE));
          for (; j > 0; j--)
            printf(" %5d",source[i-j]);
          if (act == UNIQUE)
            printf(" %5d",cv);
          printf("\n");
        }
#endif

      switch (act)             //  Proceed with encoding selected block
      { case UNIQUE:
          if (pu)
            target[du = p++] = pu = 0;
          else
            pu = (++target[du] >= 31);
          *((uint16 *) (target+p)) = pv = cv;
          p += 2;
          i += 1;
          break;
        case RUNLEN:
#ifdef SHOW
          printf("RUN (%lld) %3d: %5d\n",i,rlen,cv);
#endif
          target[p++] = rlen+222;
          *((uint16 *) (target+p)) = pv = cv;
          p += 2;
          pu = 1;
          i += rlen;
          break;
        case DIFFBLOCK3:
#ifdef SHOW
          printf("DF3 (%lld) %3d:",i,dlen3);
#endif
          target[p++] = dlen3+31;
          { int bp = 0;
            int sv = 0;
            int j  = 0;
            while (j < dlen3)
              { sv = (sv << 3) | (dbuffer[j++] - 12);
                bp = (bp + 3) % 8;
                if (bp < 3)
                  target[p++] = (sv >> bp) & 0xFF;     //  Is the mask superfluous???
              }
            if (bp > 0)
              target[p++] = (sv << (8-bp)) & 0xFF;
#ifdef SHOW
            for (j = 0; j < dlen3; j++)
              printf(" %3d",dbuffer[j]-15);
            printf("\n");
#endif
          }
          pu = 1;
          i += dlen3;
          pv = source[i-1];
          break;
        case DIFFBLOCK4:
#ifdef SHOW
          printf("DF4 (%lld) %3d:",i,dlen4);
#endif
          target[p++] = dlen4+79;
          { int j = 0;
            dbuffer[dlen4] = 8;
            while (j < dlen4)
              { int d0 = dbuffer[j++] - 8;
                int d1 = dbuffer[j++] - 8;
                target[p++] = (d0 << 4) | d1;
              }
#ifdef SHOW
            for (j = 0; j < dlen4; j++)
              printf(" %3d",dbuffer[j]-15);
            printf("\n");
#endif
          }
          pu = 1;
          i += dlen4;
          pv = source[i-1];
          break;
        case DIFFBLOCK5:
#ifdef SHOW
          printf("DF5 (%lld) %3d:",i,dlen5);
#endif
          target[p++] = dlen5+182;
          { int bp = 0;
            int sv = 0;
            int j  = 0;
            while (j < dlen5)
              { sv = (sv << 5) | dbuffer[j++];
                bp = (bp + 5) % 8;
                if (bp < 5)
                  target[p++] = (sv >> bp) & 0xFF;
              }
            if (bp > 0)
              target[p++] = (sv << (8-bp)) & 0xFF;
#ifdef SHOW
            for (j = 0; j < dlen5; j++)
              printf(" %3d",dbuffer[j]-15);
            printf("\n");
#endif
          }
          pu = 1;
          i += dlen5;
          pv = source[i-1];
          break;
      }

      //  p can never be larger than IO_BUFFER + max unique block + max (diff block or run block)
      //     which is IO_BUFFER + 65 + 53 = IO_BUFFER + 118 < IO_BUFFER+200 (the size of the buffer)

      if (p > IO_BUFFER && pu)
        { if (FWRITE(target, 1, p, file) != p)
            { fprintf(stderr,"Cannot write compressed data!\n");
              exit (0);
            }
          cl += p;
          p   = 0;
        }
    }

#ifdef SHOW
  if (!pu)
    { int j = target[du] + 1;
      printf("UNQ (%lld) %3d:",i-j,j);
      for (; j > 0; j--)
        printf(" %5d",source[i-j]);
      printf("\n");
    }
#endif

  if (p > 0)
    { if (FWRITE(target, 1, p, file) != p)
        { fprintf(stderr,"Cannot write compressed data!\n");
          exit (0);
        }
      cl += p;
    }

  return (cl);
}
#endif

void decompress_read8(FILE *file, uint8 *target, int64 size, int compatible)

{ uint8 source[IO_BUFFER+100];    //  A fixed amount of storage is needed !

  int64 smax;  //  # of bytes read into current image of 'source'
  int   pv;    //  prior value placed in decoded stream

  smax = fread(source,1,IO_BUFFER+100,file);

  int64 i  = 0;        //  index into target stream 'target'
  int64 p  = 0;        //  index into compression buffer 'source'
  while (i < size)
    { if (p > IO_BUFFER)   //  p can never be more than IO_BUFFER + max code block = 35.
        { smax -= p;
          memcpy(source,source+p,smax);
          smax += fread(source+smax,1,(IO_BUFFER+100)-smax,file);
          p = 0;
        }

      int j, v = source[p++];
      if (v < 33)
        {
#ifdef SHOW
          printf("UNQ %3d:",v+1); 
#endif
          for (j = 0; j < v; j++)          //   Unique value block
#ifdef SHOW
            { printf(" %2d",source[p]);
#endif
              target[i++] = source[p++];
#ifdef SHOW
            }
          printf(" %2d\n",source[p]);
#endif
          target[i++] = pv = source[p++];
        }
      else if (v < 128)
        { v -= 32;                         //    2-bit differences block
#ifdef SHOW
          printf("DIF %3d:",v); 
#endif
          j = 0;
          while (j < v)
            { int x = source[p++];
              int w = j+4;
              if (w > v)
                w = v;
              for ( ; j < w; j++) { 
                int inc = x & 0x3;
                //zhaot: inconsistency with v3dpdb file generated from fly workstation
                if (compatible) {
                  if (inc == 3) {
                    inc = -1;
                  }
                } else {
                  inc -= 1;
                }

                //target[i++] = pv += (x & 0x3)-1;
                target[i++] = pv += inc;
#ifdef SHOW
                  printf(" %2d", inc);
#endif
                  x >>= 2;
                }
            } 
#ifdef SHOW
          printf("\n");
#endif
        }
      else                                 //   Run-length block
        { v -= 127;
#ifdef SHOW
          printf("RUN %3d: %2d\n",v,source[p]); 
#endif
          target[i++] = pv = source[p++];
          for (j = 1; j < v; j++)
            target[i++] = pv;
        }
    }
}

void decompress_read16(FILE *file, uint16 *target, int64 size, int bswap,
    int compatible)
{ static int PlainDecode[] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };
  static int SwapDecode[] =
    { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 6,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    };

  uint8 source[IO_BUFFER+100];    //  A fixed amount of storage is needed !

  int   *key = (bswap ? SwapDecode : PlainDecode);
  uint8 *t8  = (uint8 *) target;

  int64  smax;  //  # of bytes read into current image of 'source'
  int    pv;    //  prior value placed in decoded stream

#ifdef SHOW
  printf("NEW STREAM\n");
#endif

  smax = fread(source,1,IO_BUFFER+100,file);

  int64 i  = 0;        //  index into target stream 'target'
  int64 p  = 0;        //  index into compression buffer 'source'
  while (i < size)
    { if (p > IO_BUFFER)   //  p can never be more than IO_BUFFER + max code block = 65.
        { smax -= p;
          memcpy(source,source+p,smax);
          smax += fread(source+smax,1,(IO_BUFFER+100)-smax,file);
          p = 0;
        }

      int j, v = source[p++];

      switch (key[v])
      { case UNIQUE:
          { uint16 *s16 = (uint16 *) (source+p);
#ifdef SHOW
            printf("UNQ %3d:",v+1);
#endif
            for (j = 0; j < v; j++) 
#ifdef SHOW
              { printf(" %5d",s16[j]);
#endif
                target[i++] = s16[j];
#ifdef SHOW
              }
            printf(" %5d\n",s16[v]);
#endif
            target[i++] = pv = s16[v];
            p += ((v+1) << 1);
            break;
          }
        case UNIQUE_SWAP:
          { int x = (i << 1);
#ifdef SHOW
            printf("UNB %3d:",v+1);
#endif
            for (j = 0; j <= v; j++) 
              { int y   = source[p++];
                t8[x++] = source[p++];
                t8[x++] = y;
#ifdef SHOW
                printf(" %5d",target[(x>>1)-1]);
#endif
              }
#ifdef SHOW
            printf("\n");
#endif
            i = (x >> 1);
            pv = target[i-1];
            break;
          }
        case DIFFBLOCK3:
          { v -= 31;
#ifdef SHOW
            printf("DF3 %3d:",v);
#endif
            int bp = 0;
            int sv = 0;
            for (j = 0; j < v; j++)
              { if (bp < 3)
                  sv = (source[p++] | (sv << 8));
                bp = (bp + 5) % 8;
                int inc = (sv >> bp) & 0x7;
                if (compatible) {
                  if (inc > 4) {
                    inc = 4 - inc;
                  }
                } else {
                  inc -= 3;
                }
                target[i++] = pv += inc;
                //target[i++] = pv += ((sv >> bp) & 0x7) - 3;
#ifdef SHOW
                printf(" %3d",((sv >> bp) & 0x7) - 3);
#endif
              }
#ifdef SHOW
            printf("\n");
#endif
            break;
          }
        case DIFFBLOCK4:
          v -= 79;
#ifdef SHOW
          printf("DF4 %3d:",v);
#endif
          for (j = 0; j < v; j++)
            { int sv = source[p++];
              int inc = (sv >> 4) & 0xf;
              if (compatible) {
                if (inc > 8) {
                  inc = 8 - inc;
                }
              } else {
                inc -= 7;
              }
              target[i++] = pv += inc;
              //target[i++] = pv += ((sv >> 4) & 0xf) - 7;
#ifdef SHOW
              printf(" %3d",((sv >> 4) & 0xf) - 7);
#endif
              if (++j >= v) break;
               
              inc = (sv & 0xf);
              if (compatible) {
                if (inc > 8) {
                  inc = 8 - inc;
                }
              } else {
                inc -= 7;
              }

              target[i++] = pv += inc;
              
              //target[i++] = pv += (sv & 0xf) - 7;
#ifdef SHOW
              printf(" %3d",(sv & 0xf) - 7);
#endif
            }
#ifdef SHOW
          printf("\n");
#endif
          break;
        case DIFFBLOCK5:
          { v -= 182;
#ifdef SHOW
            printf("DF5 %3d:",v);
#endif
            int bp = 0;
            int sv = 0;
            for (j = 0; j < v; j++)
              { if (bp < 5)
                  sv = (source[p++] | (sv << 8));
                bp = (bp + 3) % 8;
                int inc = (sv >> bp) & 0x1f;
                if (compatible) {
                  if (inc > 16) {
                    inc = 16 - inc;
                  }
                } else {
                  inc -= 15;
                }
                target[i++] = pv += inc;
                //target[i++] = pv += ((sv >> bp) & 0x1f) - 15;
#ifdef SHOW
                printf(" %3d",((sv >> bp) & 0x1f) - 15);
#endif
              }
#ifdef SHOW
            printf("\n");
#endif
            break;
          }
        case RUNLEN:
          v -= 222;
          target[i++] = pv = *((uint16 *) (source+p));
#ifdef SHOW
          printf("RUN %3d: %5d\n",v,pv);
#endif
          p += 2;
          for (j = 1; j < v; j++)
            target[i++] = pv;
          break;
        case RUN_SWAP:
          v -= 222;
          target[i++] = pv = swap2((int16 *) (source+p));
#ifdef SHOW
          printf("RUB %3d: %5d\n",v,pv);
#endif
          p += 2;
          for (j = 1; j < v; j++)
            target[i++] = pv;
          break;
      }
    }
}

#define V3D_RAW  0
#define PBD_RAW  1
#define PBD_SEAN 2

int V3dpbd_Channel_Number(const char *filepath)
{
  struct stat fdesc;
  uint64      file_size;

  char *formatkey2 = "v3d_volume_pkbitdf_encod";
  char *formatkey1 = "v3d_stack_pkbit_by_gene1";
  char *formatkey0 = "raw_image_stack_by_hpeng";

  int   lenkey     = strlen(formatkey0);
  char  readkey[100];
  int   format;

  char  data_endian;
  int16 pixel_bytes;
  int32 dims[4];

  int chans;
  int bswap;
  int i;

  FILE* file = fopen(filepath,"rb");
  if (file == NULL)
  { 
    fprintf(stderr,"Cannot open file %s for reading.\n",filepath);
    return 0;
  }

  fstat(fileno(file),&fdesc);
  file_size = fdesc.st_size;

  if ((4*sizeof(int32)+sizeof(int16)+sizeof(char)*(lenkey+1)) > file_size) { 
    fprintf(stderr,"File size %lld is too short for a v3d array file\n",
        file_size);
    return 0;
  }

  fread(readkey,sizeof(char),lenkey,file);
  readkey[lenkey] = '\0';
  if (strcmp(formatkey0,readkey) == 0)
    format = V3D_RAW;
  else if (strcmp(formatkey1,readkey) == 0)
    format = PBD_RAW;
  else if (strcmp(formatkey2,readkey) == 0)
    format = PBD_SEAN;
  else { 
    fprintf(stderr,"File %s does not start with a recognizable v3d header.\n",filepath);
    return 0;
  }

  fread(&data_endian,sizeof(char),1,file);
  if (data_endian!='B' && data_endian!='L') { 
    fprintf(stderr,"File %s does not have 'B' or 'L' as endian code.\n",filepath);
    return 0;
  }

  bswap = (data_endian != machine_endian());

  fread(&pixel_bytes,sizeof(int16),1,file);
  if (bswap)
    pixel_bytes = swap2(&pixel_bytes);

  if (pixel_bytes != 2 && pixel_bytes != 1) { 
    fprintf(stderr,"Pixel size must be 1 or 2 bytes\n");
    return 0;
  }

  fread(dims,sizeof(int32),4,file);
  if (bswap)
    for (i = 0; i < 4; i++)
      swap4(dims+i);

  chans  = dims[3];

  fclose(file);

  return chans;
}

//Adopted from v3dio.c
Mc_Stack* Read_V3dpbd(const char *filepath, int channel)
{
  struct stat fdesc;
  uint64      file_size;

  char *formatkey2 = "v3d_volume_pkbitdf_encod";
  char *formatkey1 = "v3d_stack_pkbit_by_gene1";
  char *formatkey0 = "raw_image_stack_by_hpeng";

  int   lenkey     = strlen(formatkey0);
  char  readkey[100];
  int   format;

  int64 *compsizes;
  int64  offs;

  char  data_endian;
  int16 pixel_bytes;
  int32 dims[4];
  int64 size;

  int chans;
  int bswap;
  int i;

  FILE* file = fopen(filepath,"rb");
  if (file == NULL)
  { 
    fprintf(stderr,"Cannot open file %s for reading.\n",filepath);
    return NULL;
  }

  fstat(fileno(file),&fdesc);
  file_size = fdesc.st_size;

  if ((4*sizeof(int32)+sizeof(int16)+sizeof(char)*(lenkey+1)) > file_size) { 
    fprintf(stderr,"File size %lld is too short for a v3d array file\n",
        file_size);
    return NULL;
  }

  fread(readkey,sizeof(char),lenkey,file);
  readkey[lenkey] = '\0';
  if (strcmp(formatkey0,readkey) == 0)
    format = V3D_RAW;
  else if (strcmp(formatkey1,readkey) == 0)
    format = PBD_RAW;
  else if (strcmp(formatkey2,readkey) == 0)
    format = PBD_SEAN;
  else { 
    fprintf(stderr,"File %s does not start with a recognizable v3d header.\n",filepath);
    return NULL;
  }

  fread(&data_endian,sizeof(char),1,file);
  if (data_endian!='B' && data_endian!='L') { 
    fprintf(stderr,"File %s does not have 'B' or 'L' as endian code.\n",filepath);
    return NULL;
  }

  bswap = (data_endian != machine_endian());

  fread(&pixel_bytes,sizeof(int16),1,file);
  if (bswap)
    pixel_bytes = swap2(&pixel_bytes);

  if (pixel_bytes != 2 && pixel_bytes != 1) { 
    fprintf(stderr,"Pixel size must be 1 or 2 bytes\n");
    return NULL;
  }

  fread(dims,sizeof(int32),4,file);
  if (bswap)
    for (i = 0; i < 4; i++)
      swap4(dims+i);

  size = 1;
  for (i = 0; i < 3; i++)
    size *= dims[i];

  chans  = dims[3];

  int kind;
  if (pixel_bytes == 2) {
    kind = GREY16;
  } else {
    kind= GREY8;
  }

  int final_chans = chans;
  if (channel >= 0) {
    final_chans = 1;
  }

  Mc_Stack *stack = Make_Mc_Stack(kind, dims[0], dims[1], dims[2], final_chans);
  size_t channelByteNumber = size * pixel_bytes;

  if (format == PBD_RAW) { 
    compsizes = (int64 *) Guarded_Malloc(sizeof(int64)*chans, "Read_V3dpbd");

    fread(compsizes,sizeof(int64),chans,file);
    if (bswap) {
      for (i = 0; i < chans; i++) {
        swap8(compsizes+i);
      }
    }

    offs = ftello(file);

    size_t array_offset = 0;
    for (i = 0; i < chans; i++) { 
      fseeko(file, offs, SEEK_SET);
      if (pixel_bytes == 2) {
        decompress_read16(file, (uint16*)(stack->array + array_offset), 
            size,bswap, 0);
      } else {
        decompress_read8(file, (uint8*)(stack->array + array_offset),size, 0);
      }
      if (channel < 0) {
        array_offset += channelByteNumber;
      }
      offs += compsizes[i];
    }

    free(compsizes);
  } else if (format == PBD_SEAN) { 
    void *buffer = stack->array;
    if (channel >= 0) {
      buffer = Guarded_Malloc(size*chans*pixel_bytes,"Read_V3dpbd");
    }

    if (pixel_bytes == 2) {
      decompress_read16(file,buffer,size*chans,bswap, 1);
    } else {
      decompress_read8(file,buffer,size*chans, 1);
    }

    if (channel >= 0) {
      memcpy(stack->array, ((uint8*) buffer) + channelByteNumber * channel, 
          channelByteNumber);
      free(buffer);
    }
    /*
    for (i = 0; i < chans; i++)
      memcpy(planes[i]->data,buffer+i*size*pixel_bytes,size*pixel_bytes);
    free(buffer);
    */
  } else {//  format == V3D_RAW
    size_t array_offset = 0;
    for (i = 0; i < chans; i++) {
      if ((int64) fread(stack->array + array_offset,
            pixel_bytes,size,file) != size) { 
        fprintf(stderr,"Failed to read data.\n");
        Kill_Mc_Stack(stack);
        return NULL;
      }
      if (channel < 0) {
        array_offset += channelByteNumber;
      }
    }

    if (bswap && pixel_bytes == 2) {
      for (i = 0; i < final_chans; i++) { 
        int64  j;
        int16 *data = (int16*) (stack->array + channelByteNumber * i);
        for (j = 0; j < size; j++) {
          swap2(data+j);
        }
      }
    }
  }

  fclose(file);

  return stack;
}

Mc_Stack* Read_Mc_Stack(const char *filepath, int channel)
{
  if (Is_Raw(filepath)) {
    FILE *fp = Guarded_Fopen((char*) filepath, "rb", "Read_Mc_Stack");

    char formatkey[] = "raw_image_stack_by_hpeng";
    int lenkey = strlen(formatkey); 	
    fread(formatkey, 1, lenkey, fp);

    if (strcmp(formatkey, "raw_image_stack_by_hpeng") != 0) {
      TZ_ERROR(ERROR_IO_FORMAT);
    }

    char endian;
    fread(&endian, 1, 1, fp);

    uint16_t dataType;
    char sz_buffer[16];
    size_t sz[4];

    fread(&dataType, 2, 1, fp);  
    fread(sz_buffer, 1, 8, fp);

    int i;
    for (i = 0; i < 4; i++) {
      sz[i] = *((uint16_t*) (sz_buffer + i * 2));
    }

    if ((sz[0] == 0) || (sz[1] == 0) || (sz[2] == 0) || (sz[3] == 0)) {
      fread(sz_buffer + 8, 1, 8, fp);

      for (i = 0; i < 4; i++) {
        sz[i] = *((uint32_t*) (sz_buffer + i * 4));
      }
    }

    if (channel >= (int)sz[3]) {
      PRINT_EXCEPTION("wrong channel",
            "multi-channel image can not be read");
      fclose(fp);
      return NULL;
    }

    if (sz[3] == 1 || channel < 0) {
      Mc_Stack *stack = Make_Mc_Stack(dataType, sz[0], sz[1], sz[2], sz[3]);
      size_t nvoxel = sz[0] * sz[1] * sz[2] * sz[3];
      //use 1G buffer fix fread bug for big file 2011/11/17
      size_t buffersize = (size_t) 1024 * 1024 * 1024;
      size_t remaining = nvoxel * dataType;
      int j = 0;
      size_t nread = 0;
      while (remaining > 0) {
        if (remaining / buffersize > 0) {
          nread += fread(stack->array + j * buffersize, 1, buffersize,
                fp);
          ++j;
          remaining -= buffersize;
        } else {
          nread += fread(stack->array + j * buffersize, 1, remaining, fp);
          remaining = 0;
        }
      }

      if (ferror(fp) || feof(fp)) {
        printf("num of voxel: %zd\nactual readed voxel: %zd\n", nvoxel, nread);
        perror("fread");
        printf("ferror: %d  feof: %d\n", ferror(fp), feof(fp));
        fclose(fp);
        TZ_ERROR(ERROR_IO_READ);
      }
      fclose(fp);
      return stack;
      //fread(stack->array, dataType, nvoxel, fp);
    } else {
      Mc_Stack *stack = Make_Mc_Stack(dataType, sz[0], sz[1], sz[2], 1);
      size_t nvoxel = sz[0] * sz[1] * sz[2];
      fseek(fp, nvoxel * dataType * channel, SEEK_CUR);
      //use 1G buffer fix fread bug for big file 2011/11/17
      size_t buffersize = (size_t) 1024 * 1024 * 1024;
      size_t remaining = nvoxel * dataType;
      int j = 0;
      size_t nread = 0;
      while (remaining > 0) {
        if (remaining / buffersize > 0) {
          nread += fread(stack->array + j * buffersize, 1, buffersize,
                fp);
          ++j;
          remaining -= buffersize;
        } else {
          nread += fread(stack->array + j * buffersize, 1, remaining, fp);
          remaining = 0;
        }
      }

      if (ferror(fp) || feof(fp)) {
        printf("num of voxel: %zd\nactual readed voxel: %zd\n", nvoxel, nread);
        perror("fread");
        printf("ferror: %d  feof: %d\n", ferror(fp), feof(fp));
        fclose(fp);
        TZ_ERROR(ERROR_IO_READ);
      }
      fclose(fp);
      return stack;
      //fread(stack->array, dataType, nvoxel, fp);
    }
  } else if (fhasext(filepath, "xml")) {
    Stack *stack = Read_Xml_Stack(filepath);
    return Mc_Stack_Rewrap_Stack(stack);
  } else if (Is_Lsm(filepath) || Is_Tiff(filepath)) {
    int width = 0;
    int height = 0;
    int depth = 0;
    int kind = 0;
    int nchannel = 0;

    Tiff_Image *image = NULL;
    Tiff_IFD *ifd = NULL;

    /* Get stack information */
    Tiff_Reader *reader;

    if (Is_Lsm(filepath)) {
      reader = Open_Tiff_Reader((char *) filepath,NULL,1);
    } else {
      reader = Open_Tiff_Reader((char *) filepath,NULL,0);
    }

    ifd = Read_Tiff_IFD(reader);
    //Print_Tiff_IFD(ifd, stdout);

    image = Extract_Image_From_IFD(ifd);

    if (image == NULL) {
      TZ_ERROR(ERROR_POINTER_NULL);
    } else {
      width = image->width;
      height = image->height;
      kind = image->channels[0]->bytes_per_pixel;

      depth  = 1;
      while (!End_Of_Tiff(reader)) {
        ifd = Read_Tiff_IFD(reader);
        if (!(Is_Lsm(filepath) && lsm_thumbnail_flag(ifd))) {
          depth++;
        }
        Free_Tiff_IFD(ifd);
      }
    }

    nchannel = image->number_channels;

    Free_Tiff_Image(image);

    int nchannel_out = nchannel;

    if (channel >= 0) {
      nchannel_out = 1;
    }

    Mc_Stack *mc_stack = Make_Mc_Stack(kind, width, height, depth, 
        nchannel_out);
    size_t bytes_per_plane = kind * width * height;
    size_t offset = 0;

    Rewind_Tiff_Reader(reader);

#ifdef _MSC_VER
    uint8 **mc_stack_array = (uint8**)malloc(sizeof(uint8*)*nchannel_out);
#else
    uint8* mc_stack_array[nchannel_out];
#endif

    if ((nchannel_out > 1) && (channel < 0)) {
      int length = bytes_per_plane * mc_stack->depth;
      mc_stack_array[0] = mc_stack->array;
      int i;
      for (i = 1; i < nchannel; i++) {
        mc_stack_array[i] = mc_stack_array[i-1] + length;
      }
    } else if (nchannel == 1) {
      channel = 0;
    }

    while (!End_Of_Tiff(reader)) {
      ifd = Read_Tiff_IFD(reader);
      if (!(Is_Lsm(filepath) && lsm_thumbnail_flag(ifd))) {
        image = Extract_Image_From_IFD(ifd);
        if (image == NULL) {
          TZ_ERROR(ERROR_POINTER_NULL);
        }

        if (nchannel == 1) {
          if (channel > 0) {
            TZ_ERROR(ERROR_DATA_TYPE);
          } else {
            memcpy(mc_stack->array + offset, image->channels[channel]->plane, 
                bytes_per_plane);
            offset += bytes_per_plane;
          }
        } else { /* multi-channel stack */
          if (channel >= 0) { /* only extract one channel */
            memcpy(mc_stack->array + offset, image->channels[channel]->plane, 
                bytes_per_plane);
            offset += bytes_per_plane;
          } else {     
            int i;
            for (i = 0; i < nchannel; i++) {
              memcpy(mc_stack_array[i] + offset, image->channels[i]->plane,
                  bytes_per_plane);
            }
            offset += bytes_per_plane;
          }
        }
        Free_Tiff_Image(image);
      }
      Free_Tiff_IFD(ifd);
    }

#ifdef _MSC_VER
    free(mc_stack_array);
    Reset_Tiff_Image();
#else
    Reset_Tiff_Image(image);
#endif
    Reset_Tiff_IFD();
    Kill_Tiff_Reader(reader);

    return mc_stack;  
  } else if (Is_Png(filepath)) {
    Stack *stack = Read_Stack_U(filepath);
    Mc_Stack *mc_stack = 
      Mc_Stack_Frame(stack->kind, stack->width, stack->height, stack->depth, 1);
    mc_stack->array = stack->array;
    stack->array = NULL;
    Kill_Stack(stack);

    return mc_stack;
  } else if (Is_V3dpbd(filepath)) {
    Mc_Stack *stack = Read_V3dpbd(filepath, channel);

    return stack;
  } else if (Is_Nsp(filepath)) {
    int numchans = Infer_Neuron_File_Channel_Number(filepath);
    Neurons *neurons = Read_Neurons(filepath, numchans);
    
    Stack *stack = Neuron_To_Stack(neurons, NULL);
    Mc_Stack *mc_stack = 
      Mc_Stack_Frame(stack->kind, stack->width, stack->height, stack->depth, 1);
    mc_stack->array = stack->array;
    stack->array = NULL;
    Kill_Stack(stack);

    return mc_stack;
  }

  return NULL;
}

Stack* Read_Sc_Stack(const char *filepath, int channel)
{
  if (!fexist(filepath)) {
    return NULL;
  }

  if (Is_Raw(filepath)) {
    return Read_Raw_Stack_C(filepath, channel);
  }

  if (Is_Png(filepath)) {
    if (channel == 0) {
      return Read_Png(filepath);
    }

    //no support for multi-channel png yet
    return NULL;
  }

  int width = 0;
  int height = 0;
  int depth = 0;
  int kind = 0;
  int nchannel = 0;

  Tiff_Image *image = NULL;
  Tiff_IFD *ifd = NULL;

  /* Get stack information */
  Tiff_Reader *reader;

  if (Is_Lsm(filepath)) {
    reader = Open_Tiff_Reader((char *) filepath,NULL,1);
  } else {
    reader = Open_Tiff_Reader((char *) filepath,NULL,0);
  }

  ifd = Read_Tiff_IFD(reader);
  //Print_Tiff_IFD(ifd, stdout);
    
  image = Extract_Image_From_IFD(ifd);

  if (image == NULL) {
    TZ_ERROR(ERROR_POINTER_NULL);
  } else {
    width = image->width;
    height = image->height;
    kind = image->channels[0]->bytes_per_pixel;
      
    depth  = 1;
    while (!End_Of_Tiff(reader)) {
      ifd = Read_Tiff_IFD(reader);
      if (!(Is_Lsm(filepath) && lsm_thumbnail_flag(ifd))) {
	depth++;
      }
      Free_Tiff_IFD(ifd);
    }
  }
    
  nchannel = image->number_channels;

  Free_Tiff_Image(image);

  int nchannel_out = nchannel;

  if (channel >= 0) {
    nchannel_out = 1;
  }

  Stack *stack = Make_Stack(kind, width, height, depth);
  size_t bytes_per_plane = kind * width * height;
  size_t offset = 0;

  Rewind_Tiff_Reader(reader);
  
#ifdef _MSC_VER
  uint8 **mc_stack_array = (uint8**)malloc(sizeof(uint8*)*nchannel_out);
#else
  uint8* mc_stack_array[nchannel_out];
#endif

  if ((nchannel_out > 1) && (channel < 0)) {
    int length = bytes_per_plane * stack->depth;
    mc_stack_array[0] = stack->array;
    int i;
    for (i = 1; i < nchannel; i++) {
      mc_stack_array[i] = mc_stack_array[i-1] + length;
    }
  } else if (nchannel == 1) {
    channel = 0;
  }

  while (!End_Of_Tiff(reader)) {
    ifd = Read_Tiff_IFD(reader);
    if (!(Is_Lsm(filepath) && lsm_thumbnail_flag(ifd))) {
      image = Extract_Image_From_IFD(ifd);
      if (image == NULL) {
	TZ_ERROR(ERROR_POINTER_NULL);
      }
      
      if (nchannel == 1) {
	if (channel > 0) {
	  TZ_ERROR(ERROR_DATA_TYPE);
	} else {
	  memcpy(stack->array + offset, image->channels[channel]->plane, 
		 bytes_per_plane);
	  offset += bytes_per_plane;
	}
      } else { /* multi-channel stack */
	if (channel >= 0) { /* only extract one channel */
	  memcpy(stack->array + offset, image->channels[channel]->plane, 
		 bytes_per_plane);
	  offset += bytes_per_plane;
	} else {     
	  int i;
	  for (i = 0; i < nchannel; i++) {
	    memcpy(mc_stack_array[i] + offset, image->channels[i]->plane,
		   bytes_per_plane);
	  }
	  offset += bytes_per_plane;
	}
      }
      Free_Tiff_Image(image);
    }
    Free_Tiff_IFD(ifd);
  }

#ifdef _MSC_VER
  free(mc_stack_array);
  Reset_Tiff_Image();
#else
  Reset_Tiff_Image(image);
#endif
  Reset_Tiff_IFD();
  Kill_Tiff_Reader(reader);

  return stack;  
}

Tiff_Image* Mc_Stack_Tiff_Image(const Mc_Stack *mc_stack, int s, 
				Tiff_Image *image)
{
  ASSERT(mc_stack->kind <= 4, "Too many bits for a pixel");
  ASSERT(mc_stack->kind != COLOR, "COLOR kind not supported");

  if (image == NULL) {
    image = Create_Tiff_Image(mc_stack->width, mc_stack->height);
  }

  size_t plane_bsize = mc_stack->width * mc_stack->height * mc_stack->kind;
  size_t array_offset =  plane_bsize * s;

  int i;
#ifdef _MSC_VER
  uint8 **mc_stack_array = (uint8**)malloc(sizeof(uint8*)*mc_stack->nchannel);
#else
  uint8* mc_stack_array[mc_stack->nchannel];
#endif

  if (mc_stack->nchannel > 0) {
    int channel_bsize = plane_bsize * mc_stack->depth;
    mc_stack_array[0] = mc_stack->array;
    for (i = 1; i < mc_stack->nchannel; i++) {
      mc_stack_array[i] = mc_stack_array[i-1] + channel_bsize;
    }
  }

  Channel_Type type = CHAN_UNSIGNED;
  if (mc_stack->kind == 4) {
    type = CHAN_FLOAT;
  }

  for (i = 0; i < mc_stack->nchannel; i++) {
    Add_Tiff_Image_Channel(image, CHAN_BLACK, 8 * mc_stack->kind, type);
    memcpy(image->channels[i]->plane, mc_stack_array[i] + array_offset,
	   plane_bsize);
  }

#ifdef _MSC_VER
  free(mc_stack_array);
#endif
  return image;
}

void Write_Mc_Stack(const char *filepath, const Mc_Stack *stack,
		    const char *metafile)
{
  if (Is_Raw(filepath)) {
    TZ_ASSERT((stack->kind != COLOR) || (stack->nchannel == 1), 
	      "Unsupported stack type"); 
    FILE *fp = Guarded_Fopen((char*) filepath, "wb", "Write_Raw_Stack");

    char formatkey[] = "raw_image_stack_by_hpeng";
    int lenkey = strlen(formatkey); 	
    fwrite(formatkey, 1, lenkey, fp);

    char endian = 'L';
    fwrite(&endian, 1, 1, fp);

    uint16_t dataType = stack->kind;
    uint32_t sz[4];
    sz[0] = stack->width;
    sz[1] = stack->height;
    sz[2] = stack->depth;
    sz[3] = stack->nchannel;

    if (stack->kind == COLOR) {
      dataType = 1;
    }

    fwrite(&dataType, 2, 1, fp);  
    fwrite(sz, 4, 4, fp);
  
    size_t nvoxel = ((size_t) stack->width) * ((size_t) stack->height) * 
      ((size_t) stack->depth);
    if (stack->kind != COLOR) {
      //fwrite(stack->array, dataType, nvoxel * stack->nchannel, fp);
    	//use 1G buffer fix fwrite bug for big file 2012/5/31
      size_t buffersize = (size_t) 1024 * 1024 * 1024;
      size_t remaining = nvoxel * dataType * stack->nchannel;
      int j = 0;
      size_t nwrite = 0;
      fclose(fp);
      while (remaining > 0) {
        fp = Guarded_Fopen((char*) filepath, "ab", "Write_Raw_Stack");
        if (remaining / buffersize > 0) {
          nwrite += fwrite(stack->array + j * buffersize, 1, buffersize,
                fp);
          ++j;
          remaining -= buffersize;
        } else {
          nwrite += fwrite(stack->array + j * buffersize, 1, remaining, fp);
          remaining = 0;
        }
        fclose(fp);
      }

      if (ferror(fp) || feof(fp)) {
        printf("num of voxel: %zd\nactual written voxel: %zd\n", nvoxel, nwrite);
        perror("fwrite");
        printf("ferror: %d  feof: %d\n", ferror(fp), feof(fp));
        fclose(fp);
        TZ_ERROR(ERROR_IO_WRITE);
      }
    } else {
      int i;
      sz[3] = 3;
      color_t *arrayc = (color_t*) stack->array;
      for (i = 0; i < 3; i++) {
	size_t j;
	for (j = 0; j < nvoxel; j++) {
	  fwrite(arrayc[j] + i, dataType, 1, fp);
	}
      }
      fclose(fp);
    }
    
  } else {
    if (stack->nchannel == 1) {
      Stack ss = Mc_Stack_Channel(stack, 0);
      ss.text = "\0";
      Write_Stack_U(filepath, &ss, metafile);
    } else {
      Tiff_Writer *tif = NULL;
      if (Is_Lsm(filepath)) { /* open it as an LSM file */
	tif = Open_Tiff_Writer((char *) filepath, 1);
      } else {
	tif = Open_Tiff_Writer((char *) filepath, 0);
      }
    
      int i;
      for (i = 0; i < stack->depth; i++) {
	Tiff_Image *image = Mc_Stack_Tiff_Image(stack, i, NULL);
	Tiff_IFD *ifd = NULL;
	if (Is_Lsm(filepath)) {
	  if ((i == 0) && (metafile != NULL)) {
	    Tiff_Reader *reader;
	    Tiff_IFD *templat;
	    reader = Open_Tiff_Reader((char*) metafile,NULL,1);
	
	    while (lsm_thumbnail_flag(templat = Read_Tiff_IFD(reader)) != 0) {
	      Kill_Tiff_IFD(templat);
	      if (End_Of_Tiff(reader)) {
		templat = NULL;
		TZ_ERROR(ERROR_IO_READ);
		break;
	      }
	    }
	
	    ifd = Make_IFD_For_Lsm_Image(image, 0, templat, stack->depth);
	    Kill_Tiff_IFD(templat);
	    Kill_Tiff_Reader(reader);	
	  } else {
	    ifd = Make_IFD_For_Lsm_Image(image, 0, NULL, stack->depth);
	  }
	} else {
	  ifd = Make_IFD_For_Image(image, 0);
	}
	//Tiff_IFD *ifd = Make_IFD_For_Image_I(image, 0, NULL);
	//Print_Tiff_IFD(ifd, stdout);
	Write_Tiff_IFD(tif, ifd);
	Free_Tiff_IFD(ifd);
	Free_Tiff_Image(image);
      }

      Reset_Tiff_IFD();
      Reset_Tiff_Image();

      Close_Tiff_Writer(tif);
      Kill_Tiff_Writer(tif);
    }
  }
}

void Fix_Lsm_File(const char *filepath)
{
  int size[3];
  Lsm_Size(filepath, size);

  FILE *fp = fopen(filepath, "rb+");
  
  char endian[3];
  endian[2] = '\0';
  fread(endian, 1, 2, fp);
  if ((endian[0] != 'I') || (endian[1] != 'I')) {
    fclose(fp);
    TZ_ERROR(ERROR_DATA_VALUE);
  }
  
  /* go to ifd offset */
  fseek(fp, 4, SEEK_SET);

  uint32_t ifd_offset;
  fread(&ifd_offset, 4, 1, fp);

  fseek(fp, ifd_offset, SEEK_SET);

  uint16_t nifd;
  fread(&nifd, 2, 1, fp);

  uint16_t ifd_label;
  fread(&ifd_label, 2, 1, fp);
  
  uint16_t i;
  for (i = 1; i < nifd; i++) {
    if (ifd_label == TIF_CZ_LSMINFO) {
      break;
    }
    fseek(fp, 10, SEEK_CUR); 
    fread(&ifd_label, 2, 1, fp);
  }

  if (ifd_label != TIF_CZ_LSMINFO) {
    fclose(fp);
  }

  /* escape type and length */
  fseek(fp, 6, SEEK_CUR);

  /* lsm info offset */
  fread(&ifd_offset, 4, 1, fp);

  fseek(fp, ifd_offset + CZ_LSMINFO_DIMX_OFFSET, SEEK_SET);
  int32_t dimx = size[0];
  fwrite(&dimx, 4, 1, fp);
  fseek(fp, ifd_offset + CZ_LSMINFO_DIMY_OFFSET, SEEK_SET);
  int32_t dimy = size[1];
  fwrite(&dimy, 4, 1, fp);
  fseek(fp, ifd_offset + CZ_LSMINFO_DIMZ_OFFSET, SEEK_SET);
  int32_t dimz = size[2];
  fwrite(&dimz, 4, 1, fp);

  fclose(fp);  
}

File_Bundle_S *Parse_Stack_Name_S(char *file_name)
{
  static File_Bundle_S my_bundle;

  static char *Prefix = NULL;
  static int   Prefix_Max = 0;

  char *s, *t, c;

  s = file_name + strlen(file_name) - 4;
  if (strcmp(s,".tif") != 0 && strcmp(s,".TIF") != 0) {
    PRINT_EXCEPTION("Invalid file", 
		    "1st file in stack does not have .tif extension");
  }
  t = s;
  while (t > file_name && isdigit(t[-1]))
    t -= 1;
  if (s-t <= 0) {
    PRINT_EXCEPTION("Invalid file", 
		    "No number sequence in stack file names");
  }

  if (t-file_name > Prefix_Max)
    { Prefix_Max = (t-file_name)*1.2 + 20;
      Prefix     = (char *) Guarded_Realloc(Prefix,Prefix_Max+1,"Parse_Stack_Name");
    }

  c = *t;
  *t = '\0';
  strcpy(Prefix,file_name);
  *t = c;

  my_bundle.prefix    = Prefix;
  my_bundle.suffix    = NULL;
  my_bundle.num_width = s-t;
  my_bundle.first_num = atoi(t);
  my_bundle.last_num = -1;
  return (&my_bundle);
}

static void Sprint_File_Bundle_S(File_Bundle_S *bundle,int depth,char *sname)
{
  if(bundle->suffix) {
    sprintf(sname, "%s%0*d%s", bundle->prefix, bundle->num_width, 
	    bundle->first_num+depth,bundle->suffix);
  } else {
    sprintf(sname, "%s%0*d.tif", bundle->prefix, bundle->num_width,bundle->first_num+depth);
  }  
}

Stack *Read_Stack_Planes_S(File_Bundle_S *bundle)
{ 
  char  sname[1000];
  int   depth = 0;

  while (1) { 
    Sprint_File_Bundle_S(bundle,depth,sname);
   
    if (!fexist(sname)) {
      break;
    }
   
    depth += 1;
    
    if (bundle->first_num + depth - 1 == bundle->last_num) {
      break;
    }
  }

  Sprint_File_Bundle_S(bundle,0,sname);

  Stack *image = Read_Stack_U(sname);
  int area = image->width * image->height;

  Stack *stack = Make_Stack(image->kind, image->width, image->height, depth);

  memcpy(stack->array, image->array, area * image->kind);

  int d;

  for (d = 1; d < depth; d++) {      
    Sprint_File_Bundle_S(bundle, d, sname);
    image = Read_Stack_U(sname);
    memcpy(stack->array + area * image->kind *d, 
	   image->array, area * image->kind);
    Free_Stack(image);
  }

  Reset_Stack();

  return (stack);
}

void Clean_File_Bundle_S(File_Bundle_S *bundle)
{
  if (bundle->prefix != NULL) {
    free(bundle->prefix);
    bundle->prefix = NULL;
  }

  if (bundle->suffix != NULL) {
    free(bundle->suffix);
    bundle->suffix = NULL;
  }
}

File_Bundle_S *Copy_File_Bundle_S(File_Bundle_S *src)
{
  File_Bundle_S *dst = (File_Bundle_S*) malloc(sizeof(File_Bundle_S));
  initfb(dst);
  memcpy(dst->prefix, src->prefix, strlen(src->prefix)+1);
  memcpy(dst->suffix, src->suffix, strlen(src->suffix)+1);
  dst->first_num = src->first_num;
  dst->last_num = src->last_num;
  dst->num_width = src->num_width;
  return dst;
}

Stack *Read_Stack_Planes_Sc(File_Bundle_S *bundle, int channel)
{
  char  sname[1000];
  int   depth = 0;

  while (1) { 
    sprint_file_bundle_sc(bundle,depth,sname);
    
    if (!fexist(sname)) {
      break;
    }
    
    depth += 1;

    if (bundle->first_num + depth - 1 == bundle->last_num) {
      break;
    }
  }

  if (depth == 0) {
    return NULL;
  }

  sprint_file_bundle_sc(bundle, 0, sname);

  Image *image = Read_Image(sname);
  int area = image->width * image->height;
  int kind = image->kind;

  if (kind == COLOR) {
    kind = GREY;
  }

  Stack *stack = Make_Stack(kind, image->width, image->height, depth);

  if (image->kind != COLOR) {
    memcpy(stack->array, image->array, area * image->kind);
  } else {
    int i;
    color_t *image_array = (color_t*) image->array;
    for (i = 0; i < area; i++) {
      stack->array[i] = image_array[i][channel];
    }
  }

  int d;

  for (d = 1; d < depth; d++) {      
    sprint_file_bundle_sc(bundle, d, sname);
    image = Read_Image(sname);
    if (image->kind != COLOR) {
      memcpy(stack->array + area * image->kind *d, 
	     image->array, area * image->kind);
    } else {
      int i;
      color_t *image_array = (color_t*) image->array;
      uint8 *stack_array = stack->array + area * d;
      for (i = 0; i < area; i++) {
	stack_array[i] = image_array[i][channel];
      }
    }
    Free_Image(image);
  }

  Reset_Image();

  return (stack);  
}

Stack* Read_Image_List(File_List *list)
{
  if (list->file_number == 0) {
    return NULL;
  }

  int depth = 0;
  int i;
  for (i = 0; i < list->file_number; i++) {
    depth++;
  }

  Stack *image = Read_Stack_U(list->file_path[0]);
  int area = image->width * image->height;

  Stack *stack = Make_Stack(image->kind, image->width, image->height, depth);

  memcpy(stack->array, image->array, area * image->kind);

  int d;

  for (d = 1; d < depth; d++) {      
    image = Read_Stack_U(list->file_path[d]);
    memcpy(stack->array + area * image->kind *d, 
        image->array, area * image->kind);
    Free_Stack(image);
  }

  Reset_Stack();

  return (stack);
}

Stack* Read_Image_List_Bounded(File_List *list)
{
  Cuboid_I bound_box;
  Stack_Bound_Box_F_M(list, &bound_box);

  int width = bound_box.ce[0] - bound_box.cb[0] + 1;
  int height = bound_box.ce[1] - bound_box.cb[1] + 1;

  if (list->file_number == 0) {
    return NULL;
  }

  /*
  int depth = 0;
  int i;
  for (i = 0; i < list->file_number; i++) {
    depth++;
  }
  */

  int depth = bound_box.ce[2] - bound_box.cb[2] + 1;

#ifdef _DEBUG_2
  width = 900;
  height = 900;
  bound_box.cb[0] = 650;
  bound_box.cb[1] = 650;
  depth = 500;
#endif

  Stack *image = Read_Stack_U(list->file_path[bound_box.cb[2]]);

  Stack *bounded_image = Make_Stack(image->kind, width, height, 1);
  size_t area = bounded_image->width * bounded_image->height;

#ifdef _DEBUG_2
  printf("%d, %d, %d\n", bounded_image->width, bounded_image->height, depth);
#endif

  Stack *stack = Make_Stack(image->kind, bounded_image->width, 
      bounded_image->height, depth);

  Crop_Stack(image, bound_box.cb[0], bound_box.cb[1], 0,
      width, height, 1, bounded_image);
  memcpy(stack->array, bounded_image->array, area * bounded_image->kind);

  int d;

  for (d = bound_box.cb[2] + 1; d <= bound_box.ce[2]; d++) {      
    image = Read_Stack_U(list->file_path[d]);
    Crop_Stack(image, bound_box.cb[0], bound_box.cb[1], 0,
        width, height, 1, bounded_image);
    memcpy(stack->array + area * image->kind * (d - bound_box.cb[2]), 
        bounded_image->array, area * bounded_image->kind);
    Free_Stack(image);
  }

  Kill_Stack(bounded_image);
  Reset_Stack();

  return (stack);
}

Stack* Read_Image_List_Bounded_M(File_List *list, Stack *mask)
{
  Cuboid_I bound_box;
  Stack_Bound_Box(mask, &bound_box);

  int front = -1;
  int back = 0;

  if (list != NULL) {
    if (list->file_number > 0) {
      size_t nvoxel = Stack_Voxel_Number(mask);

      int i;
      for (i = 0; i < list->file_number; i++) {
        BOOL all_zero = TRUE;
        Stack *stack = Read_Stack_U(list->file_path[i]);
        Stack_And(stack, mask, stack);
        size_t offset = 0;
        for (offset = 0; offset < nvoxel; offset++) {
          if (stack->array[offset] != 0) {
            all_zero = FALSE;
            break;
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
    }
  }

  bound_box.cb[2] = front;
  bound_box.ce[2] = back;

  int width = bound_box.ce[0] - bound_box.cb[0] + 1;
  int height = bound_box.ce[1] - bound_box.cb[1] + 1;
  int depth = bound_box.ce[2] - bound_box.cb[2] + 1;

  Stack *image = Read_Stack_U(list->file_path[bound_box.cb[2]]);

  Stack *bounded_image = Make_Stack(image->kind, width, height, 1);
  size_t area = bounded_image->width * bounded_image->height;

#ifdef _DEBUG_
  printf("%d, %d, %d\n", bounded_image->width, bounded_image->height, depth);
#endif

  Stack *stack = Make_Stack(image->kind, bounded_image->width, 
      bounded_image->height, depth);

  Crop_Stack(image, bound_box.cb[0], bound_box.cb[1], 0,
      width, height, 1, bounded_image);
  memcpy(stack->array, bounded_image->array, area * bounded_image->kind);

  int d;

  for (d = bound_box.cb[2] + 1; d <= bound_box.ce[2]; d++) {      
    image = Read_Stack_U(list->file_path[d]);
    Stack_And(mask, image, image);
    Crop_Stack(image, bound_box.cb[0], bound_box.cb[1], 0,
        width, height, 1, bounded_image);
    memcpy(stack->array + area * image->kind * (d - bound_box.cb[2]), 
        bounded_image->array, area * bounded_image->kind);
    Free_Stack(image);
  }

  Kill_Stack(bounded_image);
  Reset_Stack();

  return (stack);
}
