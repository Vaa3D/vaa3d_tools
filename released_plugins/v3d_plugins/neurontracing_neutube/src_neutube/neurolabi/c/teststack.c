/* teststack.c
 *
 * 12-Jun-2006 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <string.h>
#include "tz_stack.h"
#include "tz_stack_draw.h"
#include "tz_stack_lib.h"
#include "tz_stack_threshold.h"
#include "tz_stack_utils.h"
#include "tz_int_histogram.h"
#include "tz_stack_math.h"
#include "tz_objdetect.h"
#include "tz_u8matrix.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_attribute.h"
#include "tz_stack_relation.h"
#include "tz_image_io.h"
#include "tz_testdata.h"
#include "tz_stack_document.h"
#include "tz_xml_utils.h"
#include "tz_error.h"
#include "tz_geo3d_utils.h"
#include "tz_darray.h"
#include "tz_mc_stack.h"
#include "tz_stack_graph.h"
#include "tz_cuboid_i.h"

int main(int argc, char *argv[])
{
#if 0
  Stack *stack = Read_Stack("../data/fly_neuron.tif");

  Stretch_Stack_Value_Q(stack, 0.99);
  Translate_Stack(stack, GREY, 1);

  Write_Stack("../data/test.tif", stack);

  Kill_Stack(stack);
#endif

#if 0
  int idx1, idx2, width, height;
  idx1 = 33332;
  idx2 = 65535;
  width = 111;
  height = 112;

  printf("%g\n", Stack_Util_Voxel_Distance(idx1, idx2, width, height));

  int x1, y1, z1, x2, y2, z2;
  Stack_Util_Coord(idx1, width, height, &x1, &y1, &z1);
  Stack_Util_Coord(idx2, width, height, &x2, &y2, &z2);

  printf("%d, %d, %d\n", x1 - x2, y1 - y2, z1 - z2);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron.tif");
  //Translate_Stack(stack, GREY16, 1);
  Image *image = Proj_Stack_Zmax(stack);
  Write_Image("../data/test.tif", image);
#endif
  
#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_a1_org.tif");
  //stack = Crop_Stack(stack, 256, 256, 0, 512, 512, 170, NULL);
  
  int i;
  Stack stack2;
  stack2.width = stack->width;
  stack2.height = stack->height;
  stack2.kind = stack->kind;
  stack2.depth = 1;

  for (i = 0; i < stack->depth; i++) {
    stack2.array = stack->array + i * stack->width * stack->height;
    //Stack *locmax = Stack_Locmax_Region(&stack2, 8);
    Stack *locmax = Stack_Local_Max(&stack2, NULL, STACK_LOCMAX_SINGLE);
    int *hist = Stack_Hist_M(&stack2, locmax);
    int low, high;
    Int_Histogram_Range(hist, &low, &high);
    int thre = Int_Histogram_Triangle_Threshold(hist, low, high);
    printf("Threshold: %d\n", thre);

    Stack_Threshold_Binarize(&stack2, thre);

    Kill_Stack(locmax);
    free(hist);
  }
  //Stack_Bc_Autoadjust(result);
  /*
  Translate_Stack(stack, COLOR, 1);
  Stack_Blend_Mc(stack, result, 0.1);
  */
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_crop.tif");
  Filter_3d *filter = Gaussian_Filter_3d(1.0, 1.0, 0.5);
  Stack *out = Filter_Stack(stack, filter);

  Write_Stack("../data/test.tif", out);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_a2_org.tif");
  Stack *locmax = Stack_Locmax_Region(stack, 18);
  Stack *mask = Read_Stack("../data/fly_neuron_a2_org/threshold_s.tif");
  //Stack_And(locmax, mask, locmax);

  Object_3d_List *objs = Stack_Find_Object_N(locmax, NULL, 1, 0, 18);
  Zero_Stack(locmax);
  int objnum = 0;
  while (objs != NULL) {
    Object_3d *obj = objs->data;
    Voxel_t center;
    Object_3d_Central_Voxel(obj, center);
    Set_Stack_Pixel(locmax, center[0], center[1], center[2], 0, 1);
    objs = objs->next;
    objnum++;
  }

  Write_Stack("../data/fly_neuron_a2_org/locmax.tif", locmax);

  printf("objnum: %d\n", objnum);

  U8Matrix mat;
  mat.ndim = 3;
  mat.dim[0] = stack->width;
  mat.dim[1] = stack->height;
  mat.dim[2] = stack->depth;
  mat.array = locmax->array;

  dim_type bdim[3];
  bdim[0] = 7;
  bdim[1] = 7;
  bdim[2] = 5;
  U8Matrix *mat2 = U8Matrix_Blocksum(&mat, bdim, NULL);

  int offset[3];
  offset[0] = bdim[0] / 2;
  offset[1] = bdim[1] / 2;
  offset[2] = bdim[2] / 2;
  
  Crop_U8Matrix(mat2, offset, mat.dim, &mat);

  Write_Stack("../data/fly_neuron_a2_org/locmax_sum.tif", locmax);

  Stack_Threshold_Binarize(locmax, 6);
  
  Stack *clear_stack = Stack_Majority_Filter_R(locmax, NULL, 26, 4);
  Struct_Element *se = Make_Cuboid_Se(3, 3, 3);
  Stack *dilate_stack = Stack_Dilate(clear_stack, NULL, se);
  Stack *fill_stack = Stack_Fill_Hole_N(dilate_stack, NULL, 1, 4, NULL);
  Kill_Stack(dilate_stack);
  

  Stack_Not(fill_stack, fill_stack);
  Stack_And(fill_stack, mask, mask);

  Write_Stack("../data/test.tif", mask);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_t1.tif");
  Stack *locmax = Stack_Locmax_Region(stack, 6);
  Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, 6);
  
  int nvoxel = Stack_Voxel_Number(locmax);
  int i;
  int s[26];
  for (i = 0; i < nvoxel; i++) {
    if (locmax->array[i] < 3) {
      locmax->array[i] = 0;
    } else {
      locmax->array[i] = 1;
      printf("%u\n", stack->array[i]);
      Stack_Neighbor_Sampling_I(stack, 6, i, -1, s);
      iarray_print2(s, 6, 1);
    }
  }

  //Stack *locmax = Stack_Local_Max(stack, NULL, STACK_LOCMAX_SINGLE);
  Write_Stack("../data/test.tif", locmax);
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_n1.tif");
  Stack *stack2 = Flip_Stack_Y(stack, NULL);
  Flip_Stack_Y(stack2, stack2);
  if (!Stack_Identical(stack, stack2)) {
    printf("bug found\n");
  }

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Mc_Stack *stack = Read_Mc_Stack("../data/benchmark/L3_12bit.lsm", -1);
  Mc_Stack_Grey16_To_8(stack, 3);
  Write_Mc_Stack("../data/test.lsm", stack, "../data/benchmark/L3_12bit.lsm");
#endif

#if 0
  //Stack *stack = Read_Stack("../data/C2-Slice06_R1_GR1_B1_L18.tif");
  Stack *stack = Read_Stack("../data/fly_neuron_n1/traced.tif");
  Print_Stack_Info(stack);
#endif

#if 0
  Mc_Stack *stack = Make_Mc_Stack(GREY, 1024, 1024, 1024, 5);

  /*
  stack.width = 1024;
  stack.height = 1024;
  stack.depth = 1024;
  stack.kind = GREY;
  stack.nchannel = 5;
  printf("%zd\n", ((size_t)stack.kind * stack.width * stack.height *
		 stack.depth * stack.nchannel));
  */
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 1, 1, 1);
  printf("stack usage: %d\n", Stack_Usage());
  uint8 *data = stack->array;
  stack->array = NULL;
  Kill_Stack(stack);
  stack = Read_Stack("../data/benchmark/line.tif");
  free(data);
  printf("stack usage: %d\n", Stack_Usage());
#endif

#if 0
  Stack *stack = Read_Stack("../data/test.tif");
  int *hist = Stack_Hist(stack);
  Print_Int_Histogram(hist);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/mouse_neuron_single/stack.tif");
  Stack dst;
  dst.text = "\0";
  dst.array = stack->array;

  Crop_Stack(stack, 0, 0, 0, stack->width - 100, stack->height - 100, 
	     stack->depth - 30, &dst);
  Write_Stack("../data/test.tif", &dst);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 5, 5, 3);
  Zero_Stack(stack);
  Set_Stack_Pixel(stack, 2, 2, 1, 0, 1.0);
  
  Print_Stack_Value(stack);

  Stack *out = Stack_Running_Max(stack, 0, NULL);
  out = Stack_Running_Max(out, 1, out);
  out = Stack_Running_Max(out, 2, out);

  Print_Stack_Value(out);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  Stack *out = Stack_Running_Median(stack, 0, NULL);
  Stack_Running_Median(out, 1, out);
  //Stack_Running_Max(stack, 0, out);
  //Stack_Running_Max(out, 1, out);

  Write_Stack("../data/test.tif", out);

  Stack *out2 = Stack_Running_Median(stack, 0, NULL);
  Stack *out3 = Stack_Running_Median(out2, 1, NULL);
  
  if (Stack_Identical(out, out3)) {
    printf("Same in-place and out-place\n");
  }
#endif

#if 0
  Stack *stack = Read_Stack_U("../data/diadem_d1_147.xml");
  printf("%d\n", Stack_Threshold_Quantile(stack, 0.9));
#endif

#if 0
  const char *filepath = "/Users/zhaot/Data/Julie/All_tiled_nsyb5_Sum.lsm";
  char filename[100];
  fname(filepath, filename);
  
  Mc_Stack *stack = Read_Mc_Stack(filepath, -1);
  Print_Mc_Stack_Info(stack);

  Mc_Stack *tmpstack = Make_Mc_Stack(stack->kind, stack->width, stack->height,
      stack->depth / 8, stack->nchannel);

  size_t channel_size = stack->kind * stack->width *stack->height
    * stack->depth;
  size_t channel_size2 = tmpstack->kind * tmpstack->width *tmpstack->height
    * tmpstack->depth;

  int i;
  int k;
  uint8_t *array = stack->array;
  for (k = 0; k < 8; k++) {
    int offset = 0;
    int offset2 = 0;
    for (i = 0; i < stack->nchannel; i++) {
      memcpy(tmpstack->array + offset2, array + offset, channel_size2);
      offset += channel_size;
      offset2 += channel_size2;
    }
    array += channel_size2;

    char outpath[500];
    
    sprintf(outpath, "../data/test/%s_%03d.lsm", filename, k);
    Write_Mc_Stack(outpath, tmpstack, filepath);
  }
#endif

#if 0
  Stack *stack = Index_Stack(GREY16, 5, 5, 1);
  Set_Stack_Pixel(stack, 1, 1, 0, 0, 0);
  Set_Stack_Pixel(stack, 1, 2, 0, 0, 0);
  Print_Stack_Value(stack);
  Stack *out = Stack_Neighbor_Median(stack, 8, NULL);
  Print_Stack_Value(out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 10, 10, 3);

  Zero_Stack(stack);
  Cuboid_I bound_box;

  Set_Stack_Pixel(stack, 1, 1, 1, 0, 1);
  Set_Stack_Pixel(stack, 1, 2, 1, 0, 1);
  Set_Stack_Pixel(stack, 3, 1, 2, 0, 1);

  Stack_Bound_Box(stack, &bound_box);

  Print_Cuboid_I(&bound_box);
  
#endif

#if 0
  Stack_Document *doc = Xml_Read_Stack_Document("../data/test.xml");
  File_List *list = (File_List*) doc->ci;

  Cuboid_I bound_box;
  Stack_Bound_Box_F(list, &bound_box);
  Print_Cuboid_I(&bound_box);
#endif
  
#if 0
  Stack_Document *doc = Xml_Read_Stack_Document("../data/test.xml");
  File_List *list = (File_List*) doc->ci;
  Print_File_List(list);
  Stack *stack = Read_Image_List_Bounded(list);

  Stack *out = stack;
  out = Stack_Region_Expand(stack, 8, 1, NULL);
  out = Downsample_Stack(out, 4, 4, 0);
  Write_Stack("../data/test.tif", out);

#endif

#if 0
  Stack_Document *doc = Xml_Read_Stack_Document(
      "../data/ting_example_stack/test.xml");
  File_List *list = (File_List*) doc->ci;
  Print_File_List(list);

  int i;
  for (i = 0; i < list->file_number; i++) {
    Stack *stack = Read_Stack_U(list->file_path[i]);
    Stack *ds = Downsample_Stack(stack, 39, 39, 0);
    char file_path[500];
    sprintf(file_path, "../data/ting_example_stack/thumbnails/tb%05d.tif", i);
    Write_Stack(file_path, ds);
    Free_Stack(stack);
  }

#endif

#if 0
  Stack *stack = Read_Stack("../data/test2.tif");

  Stack_Threshold_Binarize(stack, 6);

  Objlabel_Workspace ow;
  STACK_OBJLABEL_OPEN_WORKSPACE(stack, (&ow));

  Object_3d *obj = Stack_Find_Largest_Object_N(stack, ow.chord, 1, 26);
  //Print_Object_3d(obj);
  //printf("%llu\n", obj->size);

  double vec[3];
  Object_3d_Orientation(obj, vec, MAJOR_AXIS);
  double center[3];
  Object_3d_Centroid(obj, center);

  darray_print2(vec, 3, 1);

  double span[2] = {100000, -100000};

  for (int i = 0; i < obj->size; i++) {
    double proj = Geo3d_Dot_Product(vec[0], vec[1], vec[2], 
        (double) obj->voxels[i][0] - center[0], 
        (double) obj->voxels[i][1] - center[1], 
        (double) obj->voxels[i][2] - center[2]);
    if (proj < span[0]) {
      span[0] = proj;
    }
    if (proj > span[1]) {
      span[1] = proj;
    }
  }
  
  darray_print2(span, 2, 1);
  
  double vec2[3];
  Object_3d_Orientation(obj, vec2, PLANE_NORMAL); 
  darray_print2(vec2, 3, 1);

  double span2[2] = {100000, -100000};

  for (int i = 0; i < obj->size; i++) {
    double proj = Geo3d_Dot_Product(vec2[0], vec2[1], vec2[2], 
        (double) obj->voxels[i][0] - center[0], 
        (double) obj->voxels[i][1] - center[1], 
        (double) obj->voxels[i][2] - center[2]);
    if (proj < span2[0]) {
      span2[0] = proj;
    }
    if (proj > span2[1]) {
      span2[1] = proj;
    }
  }
  
  darray_print2(span2, 2, 1);

  double vec3[3];
  Geo3d_Cross_Product(vec[0], vec[1], vec[2], vec2[0], vec2[1], vec2[2],
      vec3, vec3+1, vec3+2);
  double span3[2] = {100000, -100000};

  int i;
  for (i = 0; i < obj->size; i++) {
    double proj = Geo3d_Dot_Product(vec3[0], vec3[1], vec3[2], 
        (double) obj->voxels[i][0] - center[0], 
        (double) obj->voxels[i][1] - center[1], 
        (double) obj->voxels[i][2] - center[2]);
    if (proj < span3[0]) {
      span3[0] = proj;
    }
    if (proj > span3[1]) {
      span3[1] = proj;
    }
  }
  
  darray_print2(span3, 2, 1);

  coordinate_3d_t vertex[8];
  for (i = 0; i < 8; i++) {
    Coordinate_3d_Copy(vertex[i], center);
    int j;
    for (j = 0; j < 3; j++) {
      vertex[i][j] += 
        span[0] * vec[j] + span2[0] * vec2[j] + span3[0] * vec3[j];    
    }
  }

  for (i = 0; i < 3; i++) {
    vertex[1][i] += (span[1] - span[0]) * vec[i]; 
    vertex[2][i] += (span2[1] - span2[0]) * vec2[i]; 
    vertex[3][i] += (span3[1] - span3[0]) * vec3[i]; 

    vertex[4][i] = vertex[1][i] + (span2[1] - span2[0]) * vec2[i]; 
    vertex[5][i] = vertex[2][i] + (span3[1] - span3[0]) * vec3[i]; 
    vertex[6][i] = vertex[3][i] + (span[1] - span[0]) * vec[i]; 

    vertex[7][i] = vertex[5][i] + (span[1] - span[0]) * vec[i]; 
  }

  FILE *fp = fopen("../data/test.swc", "w");
  fprintf(fp, "%d %d %g %g %g %g %d\n", 1, 2, vertex[0][0], vertex[0][1],
      vertex[0][2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 2, 2, vertex[1][0], vertex[1][1],
      vertex[1][2], 3.0, 1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 3, 2, vertex[2][0], vertex[2][1],
      vertex[2][2], 3.0, 1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 4, 2, vertex[3][0], vertex[3][1],
      vertex[3][2], 3.0, 1);

  fprintf(fp, "%d %d %g %g %g %g %d\n", 5, 2, vertex[4][0], vertex[4][1],
      vertex[4][2], 3.0, 2);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 6, 2, vertex[5][0], vertex[5][1],
      vertex[5][2], 3.0, 3);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 7, 2, vertex[6][0], vertex[6][1],
      vertex[6][2], 3.0, 4);

  fprintf(fp, "%d %d %g %g %g %g %d\n", 8, 2, vertex[7][0], vertex[7][1],
      vertex[7][2], 3.0, 7);

  fprintf(fp, "%d %d %g %g %g %g %d\n", 9, 2, vertex[4][0], vertex[4][1],
      vertex[4][2], 3.0, 8);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 10, 2, vertex[4][0], vertex[4][1],
      vertex[4][2], 3.0, 3);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 11, 2, vertex[5][0], vertex[5][1],
      vertex[5][2], 3.0, 8);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 12, 2, vertex[5][0], vertex[5][1],
      vertex[5][2], 3.0, 4);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 13, 2, vertex[6][0], vertex[6][1],
      vertex[6][2], 3.0, 2);
  /*
  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(6);
  field->points[0][0] = span[0] * vec[0] + center[0];
  field->points[0][1] = span[0] * vec[1] + center[1];
  field->points[0][2] = span[0] * vec[2] + center[2];
  field->points[1][0] = span[1] * vec[0] + center[0];
  field->points[1][1] = span[1] * vec[1] + center[1];
  field->points[1][2] = span[1] * vec[2] + center[2];

  field->points[2][0] = span2[0] * vec2[0] + center[0];
  field->points[2][1] = span2[0] * vec2[1] + center[1];
  field->points[2][2] = span2[0] * vec2[2] + center[2];
  field->points[3][0] = span2[1] * vec2[0] + center[0];
  field->points[3][1] = span2[1] * vec2[1] + center[1];
  field->points[3][2] = span2[1] * vec2[2] + center[2];

  field->points[4][0] = span3[0] * vec3[0] + center[0];
  field->points[4][1] = span3[0] * vec3[1] + center[1];
  field->points[4][2] = span3[0] * vec3[2] + center[2];
  field->points[5][0] = span3[1] * vec3[0] + center[0];
  field->points[5][1] = span3[1] * vec3[1] + center[1];
  field->points[5][2] = span3[1] * vec3[2] + center[2];

  coordinate_3d_t corner[2];
  Geo3d_Scalar_Field_Boundbox(field, corner);
  darray_print2(corner[0], 3, 1);
  darray_print2(corner[1], 3, 1);

  fprintf(fp, "%d %d %g %g %g %g %d\n", 1, 2, corner[0][0], corner[0][1],
      corner[0][2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 2, 2, corner[1][0], corner[0][1],
      corner[0][2], 3.0, 1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 3, 2, corner[1][0], corner[1][1],
      corner[0][2], 3.0, 2);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 4, 2, corner[0][0], corner[1][1],
      corner[0][2], 3.0, 3);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 5, 2, corner[0][0], corner[0][1],
      corner[0][2], 3.0, 4);

  fprintf(fp, "%d %d %g %g %g %g %d\n", 6, 2, corner[0][0], corner[0][1],
      corner[1][2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 7, 2, corner[1][0], corner[0][1],
      corner[1][2], 3.0, 6);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 8, 2, corner[1][0], corner[1][1],
      corner[1][2], 3.0, 7);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 9, 2, corner[0][0], corner[1][1],
      corner[1][2], 3.0, 8);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 10, 2, corner[0][0], corner[0][1],
      corner[1][2], 3.0, 9);

  fprintf(fp, "%d %d %g %g %g %g %d\n", 11, 2, corner[0][0], corner[0][1],
      corner[1][2], 3.0, 1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 12, 2, corner[1][0], corner[0][1],
      corner[1][2], 3.0, 2);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 13, 2, corner[1][0], corner[1][1],
      corner[1][2], 3.0, 3);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 14, 2, corner[0][0], corner[1][1],
      corner[1][2], 3.0, 4);
      */
  /*
  fprintf(fp, "%d %d %g %g %g %g %d\n", 5, 2, corner[1][0], corner[1][1],
      corner[1][2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 6, 2, corner[0][0], corner[1][1],
      corner[1][2], 3.0, 6);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 7, 2, corner[1][0], corner[0][1],
      corner[1][2], 3.0, 7);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 8, 2, corner[1][0], corner[1][1],
      corner[0][2], 3.0, 7);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 12, 2, corner[1][0], corner[1][1],
      corner[0][2], 3.0, 2);

  fprintf(fp, "%d %d %g %g %g %g %d\n", 9, 2, corner[1][0], corner[1][1],
      corner[1][2], 3.0, 6);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 10, 2, corner[0][0], corner[1][1],
      corner[1][2], 3.0, 3);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 11, 2, corner[1][0], corner[0][1],
      corner[1][2], 3.0, 4);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 12, 2, corner[1][0], corner[1][1],
      corner[0][2], 3.0, -1);
      */

  fprintf(fp, "%d %d %g %g %g %g %d\n", 21, 2, span[0] * vec[0] + center[0], 
      span[0] * vec[1] + center[1], span[0] * vec[2] + center[2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 22, 2, span[1] * vec[0] + center[0], 
      span[1] * vec[1] + center[1], span[1] * vec[2] + center[2], 3.0, 21);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 23, 2, span2[0] * vec2[0] + center[0], 
      span2[0] * vec2[1] + center[1], span2[0] * vec2[2] + center[2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 24, 2, span2[1] * vec2[0] + center[0], 
      span2[1] * vec2[1] + center[1], span2[1] * vec2[2] + center[2], 3.0, 23);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 25, 2, span3[0] * vec3[0] + center[0], 
      span3[0] * vec3[1] + center[1], span3[0] * vec3[2] + center[2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 26, 2, span3[1] * vec3[0] + center[0], 
      span3[1] * vec3[1] + center[1], span3[1] * vec3[2] + center[2], 3.0, 25);

  fclose(fp);
  //double corner[6];

  /*
  FILE *fp = fopen("../data/test.swc", "w");
  fprintf(fp, "%d %d %g %g %g %g %d\n", 1, 2, span[0] * vec[0] + center[0], 
      span[0] * vec[1] + center[1], span[0] * vec[2] + center[2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 2, 2, span[1] * vec[0] + center[0], 
      span[1] * vec[1] + center[1], span[1] * vec[2] + center[2], 3.0, 1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 3, 2, span2[0] * vec2[0] + center[0], 
      span2[0] * vec2[1] + center[1], span2[0] * vec2[2] + center[2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 4, 2, span2[1] * vec2[0] + center[0], 
      span2[1] * vec2[1] + center[1], span2[1] * vec2[2] + center[2], 3.0, 3);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 5, 2, span3[0] * vec3[0] + center[0], 
      span3[0] * vec3[1] + center[1], span3[0] * vec3[2] + center[2], 3.0, -1);
  fprintf(fp, "%d %d %g %g %g %g %d\n", 6, 2, span3[1] * vec3[0] + center[0], 
      span3[1] * vec3[1] + center[1], span3[1] * vec3[2] + center[2], 3.0, 5);
  fclose(fp);
  */
  //calculate corners

  //Draw the six line of the corners

  /*
  Stack *stack2 = Copy_Stack(stack);
  Zero_Stack(stack2);
  int i = 0;
  for (i = 0; i < obj->size; i++) {

    stack2->array[Stack_Util_Offset(obj->voxels[i][0], obj->voxels[i][1],
        obj->voxels[i][2], stack->width, stack->height, stack->depth)] = 1;
  }

  Write_Stack("../data/test.tif", stack2);
  */
#endif

#if 0
  Stack *stack = Read_Stack("../data/test2.tif");

  Stack_Threshold_Binarize(stack, 6);

  Objlabel_Workspace ow;
  STACK_OBJLABEL_OPEN_WORKSPACE(stack, (&ow));
  ow.conn = 26;
  ow.init_chord = TRUE;

  int obj_size = Stack_Label_Largest_Object_W(stack, 1, 2, &ow);

  Object_3d *obj = Make_Object_3d(obj_size, ow.conn);
  extract_object(ow.chord, ow.seed, obj);
  //Print_Object_3d(obj);

  /*
  STACK_OBJLABEL_CLOSE_WORKSPACE((&ow));
  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  ow->conn = 26;
  ow->init_chord = TRUE;

  STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow);
  Stack_Label_Largest_Object_W(stack, 1, 2, ow); 
*/
  Write_Stack("../data/test3.tif", stack);
#endif
  

#if 0
  Mc_Stack *stack = Read_Mc_Stack("../data/test2.tif", -1);

  Print_Mc_Stack_Info(stack);

  size_t offset;
  size_t voxelNumber = Mc_Stack_Voxel_Number(stack);

  uint8_t* arrayc[3] = {NULL, NULL, NULL};
  arrayc[0] = stack->array;
  arrayc[1] = stack->array + voxelNumber;
  arrayc[2] = stack->array + voxelNumber * 2;

  for (offset = 0; offset < voxelNumber; ++offset) {
    if ((arrayc[0][offset] != 128) || (arrayc[1][offset] != 6) ||
        (arrayc[2][offset] != 0)) {
      arrayc[0][offset] = 0;
      arrayc[1][offset] = 0;
      arrayc[2][offset] = 0;
    }
  }

  Write_Mc_Stack("../data/test.tif", stack, NULL);

  Kill_Mc_Stack(stack);
#endif

#if 0
  Mc_Stack *stack = Read_Mc_Stack("../data/flyem/TEM/slice_figure/segmentation/selected_body.tif", -1);

  Print_Mc_Stack_Info(stack);

  size_t offset;
  size_t voxelNumber = Mc_Stack_Voxel_Number(stack);

  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);

  uint8_t* arrayc[4] = {NULL, NULL, NULL, NULL};
  int i;
  for (i = 0; i < 4; ++i) {
    arrayc[i] = stack->array + voxelNumber * i;
  }

  for (offset = 0; offset < voxelNumber; ++offset) {
    if ((arrayc[0][offset] > 0) || (arrayc[1][offset] > 0) ||
        (arrayc[2][offset] > 0) || (arrayc[3][offset] > 0)) {
      mask->array[offset] = 1;
    } else {
      mask->array[offset] = 0;
    }
  }

  mask = Downsample_Stack_Max(mask, 7, 7, 0, NULL);

  Write_Stack("../data/test.tif", mask);
#endif

#if 0
  Stack *stack = Read_Stack("../data/test2.tif");
  size_t offset;
  size_t voxelNumber = Stack_Voxel_Number(stack);
  color_t *arrayc = (color_t*) stack->array;
  for (offset = 0; offset < voxelNumber; ++offset) {
    if ((arrayc[offset][0] != 128) || (arrayc[offset][1] != 6) ||
        (arrayc[offset][2] != 0)) {
      arrayc[offset][0] = 0;
      arrayc[offset][1] = 0;
      arrayc[offset][2] = 0;
    }
  }

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/flyem/TEM/slice_figure/segmentation/selected_body_volume.tif");
  Stack *out = Make_Stack(COLOR, stack->width, stack->height, stack->depth);
  Zero_Stack(out);

  Object_3d_List *objs = Stack_Find_Object_N(stack, NULL, 255, 0, 26);
  Print_Object_3d_List_Compact(objs);
  uint8_t color[] = {0, 200, 50, 200, 0, 0};
  uint8_t *color2 = color;
  while (objs != NULL) {
    Object_3d *obj = objs->data;

    Stack_Draw_Object_C(out, obj, color2[0], color2[1], color2[2]);
    color2 += 3;

    objs = objs->next;
    break;
  }

  Write_Stack("../data/test.tif", out);
  
#endif

#if 0
  //Stack *stack = Read_Stack("../data/benchmark/binary/2d/btrig2.tif");
  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  One_Stack(stack);
  //Zero_Stack(stack);
  //Set_Stack_Pixel(stack, 0, 1, 1, 1, 1);
  Set_Stack_Pixel(stack, 0, 1, 1, 1, 0);
  Set_Stack_Pixel(stack, 0, 1, 1, 0, 0);
  Set_Stack_Pixel(stack, 0, 0, 0, 0, 0);
  Set_Stack_Pixel(stack, 0, 0, 2, 0, 0);
  Set_Stack_Pixel(stack, 0, 2, 0, 0, 0);
  Set_Stack_Pixel(stack, 0, 2, 2, 0, 0);
  Set_Stack_Pixel(stack, 0, 0, 0, 2, 0);
  Set_Stack_Pixel(stack, 0, 0, 2, 2, 0);
  Set_Stack_Pixel(stack, 0, 2, 0, 2, 0);
  Set_Stack_Pixel(stack, 0, 2, 2, 2, 0);
  //Set_Stack_Pixel(stack, 0, 1, 1, 2, 0);
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  //Default_Stack_Graph_Workspace(sgw);
  sgw->signal_mask = stack;
  Graph *graph = Stack_Graph_W(stack, sgw);
  sgw->signal_mask = NULL;
  //Print_Graph(graph);
  //Graph_To_Dot_File(graph, "../data/test.dot");

  if (Graph_Has_Hole(graph) == TRUE) {
    printf("The graph has a hole.\n");
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/flyem/skeletonization/session3/T1_207.tif");

  size_t voxel_number = Stack_Voxel_Number(stack);
  size_t i;
  for (i = 0; i < voxel_number; ++i) {
    if (stack->array[i] == 1) {
      stack->array[i] = 255;
    }
  }
  
  Filter_3d *filter = Gaussian_Filter_3d(0.5, 0.5, 0.5);
  Stack *out = Filter_Stack(stack, filter);

  Write_Stack("../data/test2.tif", out);
#endif

#if 1
  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  Zero_Stack(stack);
  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, 0, 0, 0, 4, 2, 3);
  Cuboid_I_Label_Stack(&cuboid, 1, stack);
  Print_Stack_Value(stack);

#endif


  return 0;
}
