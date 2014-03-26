/**@file tz_image_io.h
 * @brief image I/O
 * @author Ting Zhao
 * @date 17-Feb-2008
 */

#ifndef _TZ_IMAGEIO_H_
#define _TZ_IMAGEIO_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"
#include "tz_file_list.h"
#include "tz_png_io.h"

__BEGIN_DECLS

#ifndef TIF_CZ_LSMINFO
#  define TIF_CZ_LSMINFO 34412
#endif

#ifndef CZ_LSMINFO_MAGIC_NUMBER13
#  define CZ_LSMINFO_MAGIC_NUMBER13 0x00300494C
#endif

#ifndef CZ_LSMINFO_MAGIC_NUMBER14
#  define CZ_LSMINFO_MAGIC_NUMBER14 0x00400494C
#endif

#define CZ_LSMINFO_DIMX_OFFSET 8
#define CZ_LSMINFO_DIMY_OFFSET 12
#define CZ_LSMINFO_DIMZ_OFFSET 16

/*
typedef struct _Cz_Lsminfo {
  uint32_t u32MagicNumber;
  int32_t s32StructureSize;
  int32_t s32DimensionX;
  int32_t s32DimensionY;
  int32_t s32DimensionZ;
  int32_t     s32DimensionChannels;
  int32_t     s32DimensionTime;
  int32_t     s32DataType;                
  int32_t     s32ThumbnailX;               
  int32_t     s32ThumbnailY;                        
  double  f64VoxelSizeX;               
  double  f64VoxelSizeY;               
  double  f64VoxelSizeZ;               
  uint32_t     u32ScanType;
  uint32_t     u32DataType;
  uint32_t     u32OffsetVectorOverlay;
  uint32_t     u32OffsetInputLut;
  uint32_t     u32OffsetOutputLut;
  uint32_t     u32OffsetChannelColors;
  double    f64TimeInterval;
  uint32_t     u32OffsetChannelDataTypes;
  uint32_t     u32OffsetScanInformation;
  uint32_t     u32OffsetKsData;
  uint32_t     u32OffsetTimeStamps; 
  uint32_t     u32OffsetEventList;
  uint32_t     u32OffsetRoi;
  uint32_t     u32OffsetBleachRoi;
  uint32_t     u32OffsetNextRecording;
  uint32_t     u32Reserved[90];
} Cz_Lsminfo;
*/

typedef struct _Cz_Lsminfo {
  uint32_t u32MagicNumber;
  int32_t s32StructureSize;
  int32_t s32DimensionX;
  int32_t s32DimensionY;
  int32_t s32DimensionZ;
  int32_t s32DimensionChannels;
  int32_t s32DimensionTime;
  int32_t s32DataType;                
  int32_t s32ThumbnailX;               
  int32_t s32ThumbnailY;                        
  double f64VoxelSizeX;               
  double f64VoxelSizeY;               
  double f64VoxelSizeZ;    
  double f64OriginX;
  double f64OriginY;
  double f64OriginZ;
  uint16_t u16ScanType;
  uint16_t u16SpectralScan;
  uint32_t u32DataType;
  uint32_t u32OffsetVectorOverlay;
  uint32_t u32OffsetInputLut;
  uint32_t u32OffsetOutputLut;
  uint32_t u32OffsetChannelColors;
  double f64TimeInterval;
  uint32_t u32OffsetChannelDataTypes;
  uint32_t u32OffsetScanInformation;
  uint32_t u32OffsetKsData;
  uint32_t u32OffsetTimeStamps;
  uint32_t u32OffsetEventList;
  uint32_t u32OffsetRoi;
  uint32_t u32OffsetBleachRoi;
  uint32_t u32OffsetNextRecording;
  double f64DisplayAspectX;
  double f64DisplayAspectY;
  double f64DisplayAspectZ;
  double f64DisplayAspectTime;
  uint32_t u32OffsetMeanOfRoisOverlay;
  uint32_t u32OffsetTopolsolineOverlay;
  uint32_t u32OffsetTopoProfileOverlay;
  uint32_t u32OffsetLinescanOverlay;
  uint32_t u32ToolbarFlags;
  uint32_t u32OffsetChannelWavelength;
  uint32_t u32OffsetChannelFactors;
  double f64objectiveSphereCorrection;
  uint32_t u32OffsetunmixParameters;
  uint32_t     u32Reserved[69];
} Cz_Lsminfo;

typedef struct _Lsm_Scan_Info {
  uint32_t u32Entry;
  uint32_t u32Type;
  uint32_t u32Size;
  void *data;
} Lsm_Scan_Info;

typedef struct _Lsm_Channel_Colors {
  int32_t s32BlockSize;
  int32_t s32NumberColors;
  int32_t s32NumberNames;
  int32_t s32ColorsOffset;
  int32_t s32NamesOffset;
  int32_t s32Mono;
  int32_t s32Reserved[4];
} Lsm_Channel_Colors;

typedef struct _Lsm_Time_Stamp_Info {
  int32_t s32Size;
  int32_t s32NumberTimeStamps;
} Lsm_Time_Stamp_Info;

typedef struct _Lsm_Lookup_Table {
  uint32_t u32Size;
  uint32_t u32NumberSubBlocks;
  uint32_t u32NumberChannels;
  uint32_t u32LutType;
  uint32_t u32Advanced;
  uint32_t u32CurrentChannel;
  uint32_t u32Reserved[9];
  void *data;
} Lsm_Lookup_Table;

/* suffix supported file bundle */
typedef struct _File_Bundle_S {
  char *prefix;
  char *suffix;
  int   num_width;  /* bundle names are "%s.%0*d",prefix,num_width,first_num */
  int   first_num;
  int   last_num;
} File_Bundle_S;

/**@brief read a raw image
 *
 * Read_Raw() reads an image from a raw image file. The attributes of the image
 * are specified by <kind>, <width> and <height>. <offset> is the number of 
 * bytes of metadata.
 */
Image* Read_Raw(char* filename, int kind, int width, int height, int offset);

/**@brief Read a raw EM image.
 *
 * Specifically for TEM data.
 */
Image* Read_Em_Raw(const char* filename);

/*
 * Is_Tiff() returns 1 if the input file is a tif file, otherwise it returns 0.
 *
 * Is_Fbdf() returns 1 if the input file is a file bundle file, otherwise it
 * returns 0.
 */
int Is_Tiff(const char *filePath);
int Is_Fbdf(const char *filePath);
int Is_Lsm(const char *filepath);
int Is_Raw(const char *filePath);
int Is_V3dpbd(const char *filePath);

/*
 * initfb() initializes a file bundle pointer with necessar memories.
 * freefb() frees the memories allocated by initfb().
 */
void initfb(File_Bundle_S *fb);
void freefb(File_Bundle_S *fb);

/*
 * LoadFbdf() reads a file bundle from the file <filePath> and save it into 
 * <fb>. It returns 1 if the file is read successfully; Otherwise it returns 0.
 */
int Load_Fbdf(const char *filePath,File_Bundle_S *fb);

void Tiff_Size(const char *filepath, int *size);
void Lsm_Size(const char *filepath, int *size);
void Stack_Size_F(const char *filepath, int *size);
int Lsm_Channel_Number(const char *filepath);
int Lsm_Pixel_Type(const char *filepath);
int Tiff_Kind(const char *filepath, int is_lsm);
int V3dpbd_Channel_Number(const char *filepath);

void Tiff_Attribute(const char *filepath, int is_lsm,
    int *kind, int *width, int *height, int *depth);

void Print_Tiff_Info(const char *filepath);
void Print_Lsm_Info(const char *filepath);

void Fix_Lsm_File(const char *filepath);

Stack* Read_Lsm_Stack(const char *filepath, int channel);
void Write_Lsm_Stack(const char *filepath, const Stack *stack, Tiff_IFD *ifd);

Stack* Read_Raw_Stack(const char *filepath);
Stack* Read_Raw_Stack_C(const char *filepath, int channel);
/**@brief Read a slice from a raw stack.
 *
 * <slice> is 0-indexed.
 */
Stack* Read_Raw_Stack_Slice(const char *filepath, int slice);
void Write_Raw_Stack(const char *filepath, const Stack *stack);

/**@brief Read a stack from an XML file.
 *
 * Read_Xml_Stack() returns a stack that is specified in the XML file that has
 * the path <filepath>. In the XML file, the format image is specified by 
 * <data><image></image></data> and it must be at the second level.
 */
Stack* Read_Xml_Stack(const char *filepath);

Stack* Read_Stack_U(const char *filepath);
void Write_Stack_U(const char *filepath, const Stack *stack, 
		   const char *metafile);

/**@brief read a multi-channel stack
 *
 * Read_Stack() reads a multi-channel stack from <filepath>. <channel> specifies
 * which channel to read and is 0-indexed. To read all channels, set <channel> to
 * -1.
 */
Mc_Stack* Read_Mc_Stack(const char *filepath, int channel);
void Write_Mc_Stack(const char *filepath, const Mc_Stack *stack, 
		    const char *metafile);

Stack* Read_Sc_Stack(const char *filepath, int channel);

File_Bundle_S *Parse_Stack_Name_S(char *file_name);
Stack *Read_Stack_Planes_S(File_Bundle_S *bundle);

Stack *Read_Stack_Planes_Sc(File_Bundle_S *bundle, int channel);

void Clean_File_Bundle_S(File_Bundle_S *bundle);
File_Bundle_S *Copy_File_Bundle_S(File_Bundle_S *src);

Stack* Read_Stack_Dir(const char *dirpath, const char *ext);

Stack* Read_Image_List(File_List *list);
Stack* Read_Image_List_Bounded(File_List *list);
Stack* Read_Image_List_Bounded_M(File_List *list, Stack *mask);

Stack* Read_Flyem_Raw_Stack_Plane(const char *filepath);
void Write_Flyem_Raw_Stack_Plane(const char *filepath, const Stack *stack);

__END_DECLS

#endif
