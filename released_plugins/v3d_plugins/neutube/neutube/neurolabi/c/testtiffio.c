#include <stdio.h>

#include <stdlib.h>
#include <stdio.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_farray.h"
#include "tz_iarray.h"
#include "tz_image_lib_defs.h"
#include "tz_fimage_lib.h"
#include "tz_stack_lib.h"
#include "tz_image_io.h"
#include "tz_utilities.h"
#include "tz_graph_utils.h"
#include "tz_stack_io.h"
#include "tz_tiff_io.h"
#include "tz_stack_stat.h"
#include "tz_int_histogram.h"
#include "tz_imatrix.h"

INIT_EXCEPTION_MAIN(e)
#if 0
static int lsm_thumbnail_flag(Tiff_IFD *ifd)
{
  Tiff_Type type = 0;
  int count = 0;
  uint32_t *val = Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
  
  if (val == NULL) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return *val & TIFF_VALUE_REDUCED_RESOLUTION;
}
#endif

int main(int argc, char* argv[]) 
{
#if 0
  Mc_Stack *mc_stack = Read_Mc_Stack("../data/benchmark/L3_12bit.lsm", -1);
  Stack *stack = Mc_Stack_To_Stack(mc_stack, COLOR, NULL);
  Write_Stack_U("../data/test.lsm", stack, "../data/benchmark/L3_12bit.lsm");
#endif

#if 0
  //Print_Lsm_Info("/Users/zhaot/Data/stitching_12bit/70208_2BcPhR_R1_GR1_B1_L001.lsm");

  Mc_Stack *stack = Read_Mc_Stack("../data/benchmark/L3_12bit.lsm", -1);
  Print_Mc_Stack_Info(stack);
  Mc_Stack_Grey16_To_8(stack, 1);
  Write_Mc_Stack("../data/test.lsm", stack, "../data/benchmark/L3_12bit.lsm");
#endif

#if 0
  Read_Stack("/Users/zhaot/Data/slice15overlay/L05.tif");
#endif

#if 0
  /*
  char *filePath = "test";
  if( strlen(filePath)>4 )
    if( strcmp(filePath+strlen(filePath)-4,".tif")==0 || 
	strcmp(filePath+strlen(filePath)-4,".TIF")==0 )
      printf("%d\n", 1);

  printf("%d\n", 0);
  */

  Is_Tiff("test");
  //Write_Stack("/Users/zhaot/Work/neurolabi/data/test.tif", Read_Stack("/Users/zhaot/Data/slice15overlay/L03.tif"));

  Write_Stack("../data/test.tif", Read_Stack("../data/L05.tif"));
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  
  int big_endian;
  Tiff_Reader *tif = Open_Tiff_Reader("/Users/zhaot/Data/Stacks for stitching/Set 1 65C07/GL_100708_R1_GR1_B1_L013.lsm", &big_endian, 1);
  
  Tiff_Writer *tif2 = Open_Tiff_Writer("../data/test.tif", 0);
  Tiff_IFD *ifd = NULL;

  int depth = 0;
  while ((ifd = Read_Tiff_IFD(tif)) != NULL) {
    if (Convert_LSM_2_RGB(ifd, 0, 0) != NULL) {
      Write_Tiff_IFD(tif2, ifd);
      depth++;
    }
    Free_Tiff_IFD(ifd);  
  }

  printf("%d\n", depth);

  //Kill_Tiff_IFD(ifd);
  Kill_Tiff_Reader(tif);

  Close_Tiff_Writer(tif2);
  
  Stack *stack = Read_Stack("../data/test.tif");
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Tiff_Reader *reader;

  reader = Open_Tiff_Reader("../data/test.lsm",NULL,1);
  Tiff_IFD *ifd = Read_Tiff_IFD(reader);
  Print_Tiff_IFD(ifd, stdout);
  Kill_Tiff_Reader(reader);
#endif

#if 0
  Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/stitch/Set 3 17F12/GL_100208_R1_GR1_B1_L07.lsm", -1);
  //Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/nathan/2p internal[497-545,565-649nm] 40x 2x12us 900nm 5pct cleared 1.lsm", 0);

  Tiff_IFD *ifd;
  { Tiff_Reader *reader;

    reader = Open_Tiff_Reader("/Users/zhaot/Data/stitch/Set 3 17F12/GL_100208_R1_GR1_B1_L08.lsm",NULL,1);

    while (lsm_thumbnail_flag(ifd = Read_Tiff_IFD(reader)) != 0) {
      //Advance_Tiff_Reader(reader);
      if (End_Of_Tiff(reader)) {
	ifd = NULL;
	TZ_ERROR(ERROR_IO_READ);
	break;
      }
    }
  }

  //Write_Stack("../data/test.tif", stack);

  Print_Tiff_IFD(ifd, stdout);

  if (stack != NULL) {
    Write_Lsm_Stack("../data/test.lsm", stack, ifd);
  }

  stack = Read_Lsm_Stack("../data/test.lsm", -1);
  Write_Stack("../data/test.tif", stack);
  
#endif

#if 0
  Mc_Stack *mc_stack = Read_Mc_Stack("../data/test.tif", 1);
  Print_Mc_Stack_Info(mc_stack);

  Write_Mc_Stack("../data/test2.tif", mc_stack, NULL);
  Free_Mc_Stack(mc_stack);
  mc_stack = Read_Mc_Stack("../data/test2.tif", -1);
  Print_Mc_Stack_Info(mc_stack);

  printf("%d\n", Mc_Stack_Usage());
#endif

#if 0
  Stack *stack = Read_Lsm_Stack("../data/test.lsm", -1);
  Print_Stack_Info(stack);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Fix_Lsm_File("../data/test/result1.lsm");
  Print_Lsm_Info("../data/test/result1.lsm");
#endif

#if 0
  Stack *stack = Make_Stack(1, 5, 5, 5);
  int nvoxel = Stack_Voxel_Number(stack);
  int i;
  for (i = 0; i < nvoxel; i++) {
    stack->array[i] = i;
  }

  Write_Stack("../data/test.tif", stack);

  stack = Read_Stack("../data/test.tif");
  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack(fullpath("../data/fly_neuron_n11/", "mask2.tif",
				     NULL));
  Print_Stack_Info(stack);
#endif

#if 0
  //Print_Lsm_Info("/Users/zhaot/Data/stitch/Set 3 17F12/GL_100208_R1_GR1_B1_L07.lsm");
  //FILE *fp = fopen("/Users/zhaot/Data/stitch/set1/GL_100708_R1_GR1_B1_L015.lsm", "r");
  FILE *fp = fopen("../data/test.lsm", "r");

  char endian[3];
  endian[2] = '\0';
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
  
  int offset = 88;

  fseek(fp, ifd_offset, SEEK_SET);
  /*
  uint32_t value = 0;
  fwrite(&value, 4, 1, fp);
  
  fseek(fp, ifd_offset + offset, SEEK_SET);
  */
  /*
  Fprint_File_Binary(fp, 24, stdout);
  fclose(fp);
  return 1;
  */
  Cz_Lsminfo lsminfo;
  fread(&lsminfo, sizeof(Cz_Lsminfo), 1, fp);
  printf("%lu\n", sizeof(Cz_Lsminfo));

  if (lsminfo.u32MagicNumber == 67127628) {
    printf("Version 1.5, 1.6 and 2.0\n");
  } else if (lsminfo.u32MagicNumber == 50350412) {
    printf("Version 1.3\n");
  } else {
    printf("Unknown version\n");
    return 1;
  }

  printf("Structure size: %d\n", lsminfo.s32StructureSize);
  printf("Stack size: %d x %d x %d\n", lsminfo.s32DimensionX,
	 lsminfo.s32DimensionY, lsminfo.s32DimensionZ);
  printf("Number of channels: %d\n", lsminfo.s32DimensionChannels);

  switch (lsminfo.s32DataType) {
  case 1:
    printf("8-bit unsigned integer.\n");
    break;
  case 2:
    printf("12-bit unsigned integer.\n");
    break;
  case 5:
    printf("32-bit float.\n");
    break;
  case 0:
    printf("Different channels have different types.\n");
    break;
  }
  
  printf("Thumbnail size: %d x %d\n", lsminfo.s32ThumbnailX, 
	 lsminfo.s32ThumbnailY);

  printf("Voxel size: %g x %g x %g um\n", lsminfo.f64VoxelSizeX * 1000000,
	 lsminfo.f64VoxelSizeY * 1000000, lsminfo.f64VoxelSizeZ * 1000000);

  printf("Scan type: ");
  switch (lsminfo.u16ScanType) {
  case 0:
    printf("normal x-y-z-scan\n");
    break;
  case 1:
    printf("Z-Scan\n");
    break;
  case 2:
    printf("Line Scan\n");
    break;
  case 3:
    printf("Time series x-y\n");
    break;
  case 4:
    printf("Time series x-z\n");
    break;
  case 5:
    printf("Time series - Mean of ROIS\n");
    break;
  }

  if (lsminfo.u32OffsetVectorOverlay == 0) {
    printf("There is no vector overlay\n");
  } else {
    printf("Vector overlay found\n");
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
    /*    
    Lsm_Scan_Info *info = 
      (Lsm_Scan_Info*) (data + lsminfo.u32OffsetScanInformation + 20);
    printf("Information type: %u\n", info->u32Entry);
    printf("Data type: %u\n", info->u32Type);
    printf("Data size: %u\n", info->u32Size);
    */
    /*
    uint8_t *byte = (uint8_t*) (data + lsminfo.u32OffsetScanInformation);
    printf("%u\n", byte[8]);
    */
  }
  
  if (lsminfo.u32OffsetKsData == 0) {
    printf("There is no Zeiss Vision KS-3d data\n");
  } else {
    printf("Zeiss Vision KS-3d data found. Offset: %u\n",
	   lsminfo.u32OffsetKsData);
  }

  if (lsminfo.u32OffsetRoi == 0) {
    printf("There is no ROI\n");
  } else {
    printf("ROI found\n");
  }

  if (lsminfo.u32OffsetNextRecording == 0) {
    printf("There is no second image\n");
  } else {
    printf("Second image found\n");
  }

  fclose(fp);
#endif

#if 0
  FILE *fp = fopen("/Users/zhaot/Data/stitch/set1/GL_100708_R1_GR1_B1_L014.lsm", "r+");
  
  char endian[3];
  endian[2] = '\0';
  fread(endian, 1, 2, fp);
  printf("Endian: %s\n", endian);
  
  fpos_t pos;
  fgetpos(fp, &pos);
  printf("%d\n", pos);
  
  fseek(fp, 0, SEEK_SET);
  uint32_t x = 254;
  printf("%lu bytes written\n", fwrite(&x, sizeof(uint32_t), 1, fp));
  
  fgetpos(fp, &pos);
  printf("%d\n", pos);
  
  Fprint_File_Binary(fp, 8, stdout);

  fclose(fp);
#endif

#if 0
  Stack *stack = Read_Lsm_Stack("../data/GL_100208_R1_GR1_B1_L07.lsm", -1);

  Write_Stack_U("../data/test.lsm", stack, 
		"../data/GL_100208_R1_GR1_B1_L07.lsm");
#endif

#if 0
  //Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/neurolineage/lsm/Twin-Spot_Gal4-GH146_nc_11-2.lsm", 0);
  Stack *stack = Read_Stack_U("../data/test2.tif");
 
  Write_Stack_U("../data/test.tif", stack, NULL);
#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_single_org/traced.tif");
  Write_Stack("../data/test.tif", stack);
#endif

#if 0 /* test writing large stack */
  Mc_Stack *stack = Make_Mc_Stack(GREY, 1024, 1024, 1024, 3);
  size_t n = (size_t) 1024 * 1024 * 1024 * 3;
  size_t i;
  for (i = 0; i < n; i++) {
    stack->array[i] = i % 255;
  }

  Write_Mc_Stack("../data/test.tif", stack, NULL);
#endif

#if 0 /* test reading large stack */
  Mc_Stack *stack = Read_Mc_Stack("../data/test.tif", -1);
  Print_Mc_Stack_Info(stack);
#endif

#if 0 /* test writing a raw stack */
  Mc_Stack *stack = Read_Mc_Stack("/Users/zhaot/Data/SynapseSpotted2_S124-3_1to59NF1Crop1_enhanced_Sample.tif", -1);
  //Print_Stack_Info(stack);
  Write_Mc_Stack("../data/test.raw", stack, NULL);
#endif

#if 0 /* test reading a raw stack */
  /*
  Stack *stack = Read_Raw_Stack("/Users/zhaot/Data/jinny/slice15_overplaped.raw");
  Print_Stack_Info(stack);
  Write_Stack("../data/test.tif", stack);
  */

  Stack *stack = Read_Sc_Stack("/Users/zhaot/Data/jinny/proofread_slice15_3to11/detectedresults/slice15_3to11_crop01_detected_test.raw", 1);

  Print_Stack_Info(stack);
  Write_Stack("../data/test.tif", stack);
  
#endif

#if 0
  Stack *stack = Read_Sc_Stack("/Users/zhaot/Data/jinny/drawMask/slice15_3to33_4_30_result.raw", 2);
  stack = Read_Stack_U("/Users/zhaot/Data/jinny/drawMask/slice15_3to33.raw");
#endif

#if 0
  Stack *stack = 
    Read_Stack_U("/Users/zhaot/Work/neurolabi/data/diadem_d1_095.xml");
  Print_Stack_Info(stack);
#endif

#if 0
  Stack *stack = 
    Read_Stack_U("/Users/zhaot/Work/neurolabi/data/diadem_d1_001.xml");
  Print_Stack_Info(stack);
  
  Mc_Stack *mc_stack = Mc_Stack_Rewrap_Stack(stack);
  printf("%d\n", Stack_Usage());
  printf("%d\n", Mc_Stack_Usage());
  Print_Mc_Stack_Info(mc_stack);
  Write_Mc_Stack("../data/test.tif", mc_stack, NULL);

  Free_Mc_Stack(mc_stack);
  mc_stack = Read_Mc_Stack("/Users/zhaot/Work/neurolabi/data/diadem_d1_001.xml", -1);
  printf("%d\n", Mc_Stack_Usage());
#endif

#if 0
  int size[3];
  Stack_Size_F("../data/benchmark/L3_12bit.lsm", size);
  iarray_print2(size, 3, 1);
#endif

#if 0
  Mc_Stack *stack =
    Read_Mc_Stack("/Users/zhaot/Data/Julie/All_tiled_nsyb5_Sum.lsm", -1);
  Print_Mc_Stack_Info(stack);
  Write_Mc_Stack("../data/test.lsm", stack, 
      "/Users/zhaot/Data/Julie/All_tiled_nsyb5_Sum.lsm");
#endif

#if 0
  Mc_Stack *stack =
    Read_Mc_Stack("../data/test.lsm", -1);
  Print_Mc_Stack_Info(stack);
#endif

#if 0
  Mc_Stack *stack = NULL;
  stack = Read_Mc_Stack("/Users/zhaot/Data/colorsep/16D01.1-14.lsm", 0);
  Write_Mc_Stack("/Users/zhaot/Data/colorsep/channel1.tif", stack, NULL);
  stack = Read_Mc_Stack("/Users/zhaot/Data/colorsep/16D01.1-14.lsm", 1);
  Write_Mc_Stack("/Users/zhaot/Data/colorsep/channel2.tif", stack, NULL);
  stack = Read_Mc_Stack("/Users/zhaot/Data/colorsep/16D01.1-14.lsm", 2);
  Write_Mc_Stack("/Users/zhaot/Data/colorsep/channel3.tif", stack, NULL);
  stack = Read_Mc_Stack("/Users/zhaot/Data/colorsep/16D01.1-14.lsm", 3);
  Write_Mc_Stack("/Users/zhaot/Data/colorsep/channel4.tif", stack, NULL);
  /*
  Write_Mc_Stack("../data/test.lsm", stack, 
      "/Users/zhaot/Data/Julie/All_tiled_nsyb5_Sum.lsm");
      */
#endif

#if 0
  Mc_Stack *stack = Read_Mc_Stack("/Users/zhaot/Data/colorsep/16D01.1-14.lsm",
      -1);
  Print_Mc_Stack_Info(stack);
  Stack grey_stack = Mc_Stack_Channel(stack, 3);  
  grey_stack.text = "\0";
  Write_Stack("../data/test.tif", &grey_stack);
#endif

#if 0
  printf("Channel number: %d\n", Lsm_Channel_Number("../data/12bit/70208_2BcPhR_R1_GR1_B1_L003.lsm"));
  printf("Image type:");
  switch (Lsm_Pixel_Type("../data/12bit/70208_2BcPhR_R1_GR1_B1_L003.lsm")) {
    case GREY8:
      printf(" uint8\n");
      break;
    case GREY16:
      printf(" uint16\n");
      break;
    case FLOAT32:
      printf(" float32\n");
      break;
    default:
      printf(" unknown\n");
  }
#endif

#if 0
  Mc_Stack *stack = Read_Mc_Stack("../data/brainbow/CA3.lsm", 0);
  Print_Mc_Stack_Info(stack);
  Write_Mc_Stack("../data/test.tif", stack, NULL);
#endif

#if 0
  Stack *stack = Read_Stack_U("../data/test.xml");
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  int width, height, depth, kind;

  Tiff_Attribute("../data/stack8.tif", 0, &kind, &width, &height, &depth);
  printf("%d %d %d %d\n", width, height, depth, kind);
#endif

#if 0
  Mc_Stack *stack = Read_Mc_Stack("../data/neuronsep/Lee_Lab/13C01_BLM00090_D2.v3dpdb", 1);
  Print_Mc_Stack_Info(stack);
  Write_Mc_Stack("../data/test.tif", stack, NULL);
#endif

#if 0
  //Mc_Stack *stack = Read_Mc_Stack("../data/benchmark/bfork_2d.tif", -1);
  //Mc_Stack *stack = Read_Mc_Stack("../data/neuronsep/Lee_Lab/13C01_BLM00090_D2.v3dpdb", 0);
  Mc_Stack *stack = Read_Mc_Stack("../data/neuronsep/stitched-1.v3dpdb", 0);
  //stack->depth = 1;
  Stack ch = Mc_Stack_Channel(stack, 0);
  printf("%g\n", Stack_Sum(&ch));
  int *hist = Stack_Hist(&ch);
  Print_Int_Histogram(hist);

  Print_Mc_Stack_Info(stack);
  Write_Mc_Stack("../data/test.tif", stack, NULL);
#endif

#if 0
  Mc_Stack *stack = Read_Mc_Stack("../data/neurosep/aljosha/stitched-1858872924438528098.v3draw", -1);
  Print_Mc_Stack_Info(stack);
#endif

#if 1
  IMatrix *mat = IMatrix_Read("../data/test/session2/body_map/body_map00161.imat");

  printf("%d: %d x %d x %d\n", (int) mat->ndim, mat->dim[0], mat->dim[1],
      mat->dim[2]);

  Stack *stack = Make_Stack(GREY, mat->dim[0], mat->dim[1], mat->dim[2]);
  size_t nvoxel = Stack_Voxel_Number(stack);
  for (size_t i = 0; i < nvoxel; ++i) {
    stack->array[i] = (mat->array[i]) >> 24;
  }
  Write_Stack("../data/test.tif", stack);
#endif

  return 0;
}
