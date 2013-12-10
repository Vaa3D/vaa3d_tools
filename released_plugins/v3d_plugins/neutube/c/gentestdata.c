/* gentestdata.c
 * Initial write: Ting Zhao
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_lib.h"
#include "tz_darray.h"
#include "tz_image_io.h"
#include "tz_stack_attribute.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_draw.h"
#include "tz_random.h"
#include "tz_fimage_lib.h"
#include "tz_math.h"
#include "math.h"
#include "tz_constant.h"
#include "tz_mc_stack.h"
#include "tz_string.h"
#include "tz_testdata.h"
#include "tz_stack_utils.h"
#include "tz_stack_stat.h"
#include "tz_stack_objlabel.h"
#include "tz_image_trans.h"
#include "tz_arrayview.h"
#include "tz_file_list.h"
#include "tz_stack_threshold.h"

INIT_EXCEPTION_MAIN(e)

#if 0
static void draw_sphere(Stack *stack, int r1, int r2, int cx, int cy, int cz)
{
  int i, j, k;
  int offset = 0;
  for (k = 0; k < stack->depth; k++) {
    for (j = 0; j < stack->height; j++) {
      for (i = 0; i < stack->width; i++) {
	int dx = i - cx;
	int dy = j - cy;
	int dz = k - cz;
	
	if ((dx * dx + dy * dy + dz * dz >= r1 *r1) &&
	    (dx * dx + dy * dy + dz * dz <= r2 *r2)) {
	  stack->array[offset] = 1;
	}
	offset++;
      }
    }
  } 
}
#endif

#if 0
static void create_data(const char *imgfile, const char *data_name, int flip)
{
  Stack *stack = Read_Stack_U(imgfile);
  Stack *grey_stack = stack;
  char out1[100], out2[100];
  sprintf(out1, "../data/%s.tif", data_name);
  if (flip) {
    Flip_Stack_Y(grey_stack, grey_stack);
  }
  Write_Stack(out1, grey_stack);
  Kill_Stack(stack);

  sprintf(out2, "../data/%s_org.tif", data_name);
  int stat = symlink(out1, out2);
  if (stat != 0) {
    perror(strerror(stat));
  }

  sprintf(out1, "mkdir ../data/%s", data_name);
  system(out1);

  printf("%s created from %s\n", data_name, imgfile);
}
#endif

#if 0
static void create_data3(Stack *stack, const char *data_name, int flip,
			 const double *res)
{
  stack->text = "\0";
  char out1[100], out2[100];
  sprintf(out1, "../data/%s.tif", data_name);
  if (flip) {
    Flip_Stack_Y(stack, stack);
  }
  Write_Stack(out1, stack);

  sprintf(out2, "../data/%s_org.tif", data_name);
  int stat = symlink(out1, out2);
  if (stat != 0) {
    perror(strerror(stat));
  }

  sprintf(out1, "mkdir ../data/%s", data_name);
  system(out1);

  if (res != NULL) {
    int i;
    sprintf(out1, "../data/%s.res", data_name);
    FILE *fp = fopen(out1, "w");
    for (i = 0; i < 3; i++) {
      fprintf(fp, "%g ", res[i]);
    }
    fclose(fp);
  }
}
#endif


#if 0
static void create_data2(const char *imgfile, const char *data_name, 
			 int channel, int flip, const double *res)
{
  Mc_Stack *mc_stack = Read_Mc_Stack(imgfile, channel);
  Stack stack = Mc_Stack_Channel(mc_stack,0);
  Stack *grey_stack = &stack;
  /*
  grey_stack->text = "\0";
  char out1[100], out2[100];
  sprintf(out1, "../data/%s.tif", data_name);
  if (flip) {
    Flip_Stack_Y(grey_stack, grey_stack);
  }
  Write_Stack(out1, grey_stack);
  */

  create_data3(grey_stack, data_name, flip, res);

  printf("%s created from %s\n", data_name, imgfile);

  Kill_Mc_Stack(mc_stack);
  /*
  sprintf(out2, "../data/%s_org.tif", data_name);
  int stat = symlink(out1, out2);
  if (stat != 0) {
    perror(strerror(stat));
  }

  sprintf(out1, "mkdir ../data/%s", data_name);
  system(out1);

  printf("%s created from %s\n", data_name, imgfile);

  if (res != NULL) {
    int i;
    sprintf(out1, "../data/%s.res", data_name);
    FILE *fp = fopen(out1, "w");
    for (i = 0; i < 3; i++) {
      fprintf(fp, "%g ", res[i]);
    }
    fclose(fp);
  }
  */
}
#endif

#if 0
static void draw_2d_gaussian(Stack *stack, double mu_x, double mu_y, 
    double sigma_x, double sigma_y, double sigma_xy)
{
  double inv_x, inv_y, inv_xy;
  double det = sigma_x * sigma_y - sigma_xy * sigma_xy;
  inv_x = sigma_y / det;
  inv_y = sigma_x / det;
  inv_xy = -sigma_xy / det;
  int i, j;
  int offset = 0;
  for (j = 0; j < stack->height; j++) {
    for (i = 0; i < stack->width; i++) {
      double dx = (double) i - mu_x;
      double dy = (double) j - mu_y;
      int v = iround(exp(-(dx * dx * inv_x + dy * dy * inv_y + 
            2.0 * dx * dy * inv_xy) * 0.5) * 255.0);
      
      v += stack->array[offset];
      if (v > 255) {
        v = 255;
      }
      stack->array[offset] = v;
      offset++;
    }
  } 
}
#endif

int main(int argc, char *argv[])
{
#if 0
  File_Bundle fb;
  fb.prefix = "../data/neuromorph/disk1/696_019/696_019_50x_1/696_019_50x_1_Series005_z";
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;
  
  Stack *stack = Read_Stack_Planes(&fb);
  
  Stack *stack2 = Crop_Stack(stack, 96, 176, 0, 256, 256, 314);
  Write_Stack("../data/testneurotrace.tif", stack2);
  
  Kill_Stack(stack);
  Kill_Stack(stack2);
#endif

#if 0 /* collection of spheres */
  Stack *stack = Make_Stack(GREY, 128, 100, 64);
  Zero_Stack(stack);

  draw_sphere(stack, 5, 8, 30, 30, 30);
  draw_sphere(stack, 7, 8, 50, 50, 50);
  draw_sphere(stack, 0, 8, 70, 70, 60);

  Write_Stack("../data/benchmark/sphere_bw.tif", stack);
#endif

#if 0
  Stack  *stack = Read_Stack("/Users/zhaot/Data/JHS_Elav_3A-GAL4_20071004_B03_20080304144605258.lsm__HP-F__GF_4ting.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain.tif", grey_stack);
#endif

#if 0
  Stack  *stack = Read_Stack("/Users/zhaot/Data/JHS_24B-GAL4_20070906_B04_20080214144417835.lsm__HP-F__GF.tif.seg.raw.cp.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain2.tif", grey_stack);
#endif

#if 0
  Stack  *stack = Read_Stack("/Users/zhaot/Data/JHS_57B10-GAL4_20070710_B07_20080214145339456.lsm__HP-F__GF.tif.seg.raw.cp.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain3.tif", grey_stack);
#endif

#if 0
  Stack  *stack = Read_Stack("/Users/zhaot/Data/JHS_78Y-GAL4_20071219_B05_20080214151100269.lsm__HP-F__GF.tif.seg.raw.cp.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain4.tif", grey_stack);
#endif

#if 0
  Stack  *stack = Read_Stack("/Users/zhaot/Data/JHS_BG487-GAL4_20070911_B05_20080229140454557.lsm__HP-F__GF.tif.seg.raw.cp.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain5.tif", grey_stack);
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Data/to_Ting_080910/s1.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain6.tif", grey_stack);
  system("ln -s ../data/fly_brain6.tif ../data/fly_brain6_org.tif");
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Data/to_Ting_080910/s2.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain7.tif", grey_stack);
  system("ln -s ../data/fly_brain7.tif ../data/fly_brain7_org.tif");
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Data/gmr_t1.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 0, NULL);

  Write_Stack("../data/fly_brain_tmp.tif", grey_stack);
#endif

#if 0
  Image *image = Make_Image(GREY, 10, 10);
  int i;
  for (i = 0; i < 100; i++) {
    image->array[i] = i;
  }
  Write_Image("../data/testimg.tif", image);
#endif

#if 0
  //Print_Tiff_Info("/Users/zhaot/Data/neurolineage/Volker_9D11/GMR_9D11_AE_01_33-fA01b_C071025_20071106221221390.tif");
  
  Stack *stack = Read_Stack("/Users/zhaot/Data/neurolineage/Volker_9D11/GMR_9D11_AE_01_33-fA01b_C071025_20071106221221390.tif");
  Print_Stack_Info(stack);
  Stack *synapse = Make_Stack(stack->kind, stack->width, stack->height, 
			      stack->depth / 2);
  int i;
  int slice_nbyte = Stack_Voxel_Bsize(stack) * stack->width * stack->height;
 
  for (i = 0; i < synapse->depth; i++) {
    memcpy(synapse->array + i * slice_nbyte, 
	   stack->array + (2 * i + 1) * slice_nbyte,
	   slice_nbyte);
  }

  Write_Stack("../data/fly_neuron_a1_org/synapse.tif", synapse);
  
#endif

#if 0
  Stack *stack = Read_Stack("../data/fly_neuron_a1_org/synapse_aligned.tif");
  Translate_Stack(stack, COLOR, 1);
  Write_Stack("../data/fly_neuron_a1_org/synapse_aligned_c.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Data/neurolineage/clones and lines for Gene/Adult_BALa1_20X.tif");
  Stack *grey_stack = Stack_Channel_Extraction(stack, 1, NULL);
  Write_Stack("../data/fly_neuron_a2_org.tif", grey_stack);
  system("mkdir ../data/fly_neuron_a2_org");
  int stat = 
    symlink("../data/fly_neuron_a2_org.tif", "../data/fly_neuron_a2_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Data/neuron/n1_03_4-17-08_45-50hrALH(126).lsm.tif");
  Stack *grey_stack = stack;
  Write_Stack("../data/fly_neuron_b2.tif", grey_stack);
  system("mkdir ../data/fly_neuron_b2");
  int stat = 
    symlink("../data/fly_neuron_b2.tif", "../data/fly_neuron_b2_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Data/neuron/n1_11_4-16-08_45-50hrALH(119).lsm.tif");
  Stack *grey_stack = stack;
  Write_Stack("../data/fly_neuron_b3.tif", grey_stack);
  system("mkdir ../data/fly_neuron_b3");
  int stat = 
    symlink("../data/fly_neuron_b3.tif", "../data/fly_neuron_b3_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  double res[] = {0.21, 0.21, 1.0};
  darray_write("../data/mouse_neuron4_org.res", res, 3);

  Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/nathan/2p internal[497-545,565-649nm] 40x 2x12us 900nm 5pct cleared 1.lsm", 0);
  Stack *grey_stack = stack;
  Write_Stack("../data/mouse_neuron4_org.tif", grey_stack);
  system("mkdir ../data/mouse_neuron4_org");
  int stat = 
    symlink("../data/mouse_neuron4_org.tif", "../data/mouse_neuron4_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  File_Bundle fb;
  fb.prefix = "/Users/zhaot/Data/neuromorph/disk1/696_019/696_019_50x_1/696_019_50x_1_Series005_z";
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;
  Stack *stack = Read_Stack_Planes(&fb);
  Stack *grey_stack = stack;
  Write_Stack("../data/lobster_neuron_org.tif", grey_stack);
  system("mkdir ../data/lobster_neuron_org");
  int stat = 
    symlink("../data/lobster_neuron_org.tif", "../data/lobster_neuron_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  File_Bundle fb;
  fb.prefix = "/Users/zhaot/Data/neuromorph/disk1/696_019/696_019_50x_2/696_019_50x_2_Series002_z";
  fb.suffix = "_ch00";
  fb.num_width = 3;
  fb.first_num = 0;
  Stack *stack = Read_Stack_Planes(&fb);
  Stack *grey_stack = stack;
  Write_Stack("../data/lobster_neuron2_org.tif", grey_stack);
  system("mkdir ../data/lobster_neuron2_org");
  int stat = 
    symlink("../data/lobster_neuron2_org.tif", "../data/lobster_neuron2_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t start = {10, 10, 10};
  voxel_t end = {70, 50, 30};
  Object_3d *obj = Line_To_Object_3d(start, end);
  Stack_Draw_Object_Bw(stack, obj, 1);

  Write_Stack("../data/test/line.tif", stack);
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t start = {10, 10, 10};
  voxel_t end = {70, 50, 30};
  Object_3d *obj = Line_To_Object_3d(start, end);
    int i;
  for (i = 0; i < obj->size; i++) {
    Set_Stack_Pixel(stack, obj->voxels[i][0], obj->voxels[i][1], 
		    obj->voxels[i][2], 0, Unifrnd_Int(5) + 1);
  }

  Write_Stack("../data/test/greyline.tif", stack);
#endif

#if 0
  double res[] = {0.21, 0.21, 1.0};
  darray_write("../data/mouse_neuron4_org.res", res, 3);

  Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/nathan/2p internal[497-545,565-649nm] 40x 2x12us 900nm 5pct cleared 1.lsm", 0);
  Stack *grey_stack = stack;
  Write_Stack("../data/mouse_neuron4_org.tif", grey_stack);
  system("mkdir ../data/mouse_neuron4_org");
  int stat = 
    symlink("../data/mouse_neuron4_org.tif", "../data/mouse_neuron4_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Data/neuromorph/For_Nathan/Lo_Ch2_Stack.tif");
  Stack *grey_stack = stack;
  Write_Stack("../data/mouse_neuron_jd_org.tif", grey_stack);
  system("mkdir ../data/mouse_neuron_jd_org");
  int stat = 
    symlink("../data/mouse_neuron_jd_org.tif", "../data/mouse_neuron_jd_org_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }

#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_neuron_org.tif");
  Stack *substack = Crop_Stack(stack, 202, 183, 0, 134, 140, 81, NULL);
  Write_Stack("../data/mouse_neuron_org_crop.tif", substack);
  symlink("../data/mouse_neuron_org_crop.tif", 
	  "../data/mouse_neuron_org_crop_org.tif");
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t pt1 = {50, 20, 50};
  voxel_t pt2 = {50, 50, 50};
  voxel_t pt3 = {20, 80, 50};
  voxel_t pt4 = {80, 80, 50};
  Object_3d *obj1 = Line_To_Object_3d(pt1, pt2);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  Object_3d *obj2 = Line_To_Object_3d(pt2, pt3);
  Stack_Draw_Object_Bw(stack, obj2, 1);
  Object_3d *obj3 = Line_To_Object_3d(pt2, pt4);
  Stack_Draw_Object_Bw(stack, obj3, 1);
  
  double sigma[] = {1.5, 1.5, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/rn003/fork.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t pt1 = {50, 20, 40};
  voxel_t pt2 = {50, 50, 40};
  voxel_t pt3 = {20, 80, 40};
  voxel_t pt4 = {80, 80, 40};
  Object_3d *obj1 = Line_To_Object_3d(pt1, pt2);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  Object_3d *obj2 = Line_To_Object_3d(pt2, pt3);
  Stack_Draw_Object_Bw(stack, obj2, 1);
  Object_3d *obj3 = Line_To_Object_3d(pt2, pt4);
  Stack_Draw_Object_Bw(stack, obj3, 1);

  pt1[2] = 60;
  pt2[2] = 60;
  pt3[2] = 60;
  pt4[2] = 60;
  obj1 = Line_To_Object_3d(pt1, pt2);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  obj2 = Line_To_Object_3d(pt2, pt3);
  Stack_Draw_Object_Bw(stack, obj2, 1);
  obj3 = Line_To_Object_3d(pt2, pt4);
  Stack_Draw_Object_Bw(stack, obj3, 1);
  
  double sigma[] = {1.5, 1.5, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/benchmark/fork2/fork2.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t pt1 = {50, 20, 50};
  voxel_t pt4 = {80, 80, 50};
  Object_3d *obj1 = Line_To_Object_3d(pt1, pt4);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  
  double sigma[] = {1.5, 1.5, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/rn003/line.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t pt1 = {50, 20, 50};
  voxel_t pt2 = {50, 50, 50};
  voxel_t pt3 = {20, 80, 50};
  voxel_t pt4 = {80, 80, 50};
  voxel_t pt5 = {51, 50, 50};
  voxel_t pt6 = {51, 20, 50};
  Object_3d *obj1 = Line_To_Object_3d(pt1, pt2);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  Object_3d *obj2 = Line_To_Object_3d(pt2, pt3);
  Stack_Draw_Object_Bw(stack, obj2, 1);
  Object_3d *obj3 = Line_To_Object_3d(pt5, pt4);
  Stack_Draw_Object_Bw(stack, obj3, 1);
  Object_3d *obj4 = Line_To_Object_3d(pt6, pt5);
  Stack_Draw_Object_Bw(stack, obj4, 1);

  double sigma[] = {1.5, 1.5, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  Random_Seed(200);
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/rn003/fork2.tif", out);
  system("mkdir ../data/rn003/fork2");
#endif

#if 0
  int n = 0;
  char *dir_name = "/Users/zhaot/Data/neuron";
  DIR *dir = opendir(dir_name);
  char imgfile[100], data_name[100];
  struct dirent *ent = readdir(dir);
  while (ent != NULL) {
    const char *file_ext = fextn(ent->d_name);
    if (file_ext != NULL) {
      if (strcmp(file_ext, "tif") ==0) {
	sprintf(imgfile, "%s/%s", dir_name, ent->d_name);
	sprintf(data_name, "fly_neuron_n%d", n + 1);
	create_data(imgfile, data_name, 1);
	n++;
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t pt1 = {20, 20, 50};
  voxel_t pt2 = {80, 80, 50};
  voxel_t pt3 = {80, 20, 55};
  voxel_t pt4 = {20, 80, 55};
  voxel_t pt5 = {21, 20, 50};
  voxel_t pt6 = {81, 80, 50};
  Object_3d *obj1 = Line_To_Object_3d(pt1, pt2);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  Object_3d *obj2 = Line_To_Object_3d(pt3, pt4);
  Stack_Draw_Object_Bw(stack, obj2, 1);
  Object_3d *obj3 = Line_To_Object_3d(pt5, pt6);
  Stack_Draw_Object_Bw(stack, obj3, 1);

  double sigma[] = {1.5, 1.5, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  Random_Seed(200);
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/rn003/fork3.tif", out);
  system("mkdir ../data/rn003/fork3");
#endif

#if 0 /* 45 degree cross over */
  Stack *stack = Make_Stack(1, 100, 100, 100);
  voxel_t pt1 = {20, 20, 50};
  voxel_t pt2 = {80, 80, 50};
  voxel_t pt3 = {80, 20, 55};
  voxel_t pt4 = {20, 80, 55};
  voxel_t pt5 = {21, 20, 50};
  voxel_t pt6 = {81, 80, 50};
  Object_3d *obj1 = Line_To_Object_3d(pt1, pt2);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  Object_3d *obj2 = Line_To_Object_3d(pt3, pt4);
  Stack_Draw_Object_Bw(stack, obj2, 1);
  Object_3d *obj3 = Line_To_Object_3d(pt5, pt6);
  Stack_Draw_Object_Bw(stack, obj3, 1);

  double sigma[] = {1.5, 1.5, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  Random_Seed(200);
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/benchmark/fork3.tif", out);

  //Write_Stack("../data/rn003/fork3.tif", out);
  //system("mkdir ../data/rn003/fork3");  
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 100);
   
  Zero_Stack(stack);

  voxel_t pt1 = {10, 50, 50};
  voxel_t pt2 = {90, 50, 50};
  voxel_t pt3 = {10, 51, 50};
  voxel_t pt4 = {90, 51, 50};
  
  Object_3d *obj1 = Line_To_Object_3d(pt1, pt2);
  Stack_Draw_Object_Bw(stack, obj1, 1);
  Object_3d *obj2 = Line_To_Object_3d(pt3, pt4);
  Stack_Draw_Object_Bw(stack, obj2, 1);
  
  Stack *stack2 = Copy_Stack(stack);
  char filepath[100];

  double sigma[] = {1.0, 1.0, 2.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);

  int a = 15;
  int d = 0;
  for (a = 15; a <=90; a += 15) {
    for (d = 0; d <= 10; d += 2) {
      double t = ((double) a) * TZ_PI / 180.0;
      pt1[0] = 50 + iround(40.0 * cos(t));
      pt1[1] = 50 + iround(40.0 * sin(t));
      pt1[2] = 50 + d;
      pt2[0] = 50 - iround(40.0 * cos(t));
      pt2[1] = 50 - iround(40.0 * sin(t));
      pt2[2] = 50 + d;
      printf("%d, %d, %d\n", pt1[0], pt1[1], pt1[2]);
      Kill_Object_3d(obj1);
      obj1 = Line_To_Object_3d(pt1, pt2);
      Stack_Draw_Object_Bw(stack2, obj1, 1);
      sprintf(filepath, "../data/rn003/cross_%d_%d.tif", a, d);
      
      FMatrix *f = Filter_Stack_Fast_F(stack2, filter, NULL, 0);
      Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				     GREY);
  
      int i;
      int nvoxel = Stack_Voxel_Number(out);
      double mu = 10.0;
      Random_Seed(200);
      for (i = 0; i < nvoxel; i++) {
	unsigned int noise = Poissonrnd(mu);
	
	noise += out->array[i];
	if (noise > 255) {
	  noise = 255;
	}
	out->array[i] = noise;
      }

      Free_FMatrix(f);

      Write_Stack(filepath, out);
      sprintf(filepath, "mkdir ../data/rn003/cross_%d_%d", a, d);
      system(filepath);

      Free_Stack(out);
      Copy_Stack_Array(stack2, stack);
    }
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/zt/Pollen.tif");
  Stack *grey_stack = stack;
  Write_Stack("../data/pollen.tif", grey_stack);
  system("mkdir ../data/pollen");
#endif

#if 0
  Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/2009/011209_13304B_Ng2Av79/temp_1/slice13_R1_GR1_B1_L16.lsm", 1);
  Stack *grey_stack = stack;
  Write_Stack("../data/mouse_neuron_sp1.tif", grey_stack);
  system("mkdir ../data/mouse_neuron_sp1");
  int stat = 
    symlink("../datamouse_neuron_sp1.tif", "../data/mouse_neuron_sp1_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/2009/011209_13304B_Ng2Av79/temp_1/slice13_R1_GR1_B1_L16.lsm", 0);
  Stack *grey_stack = stack;
  Write_Stack("../data/mouse_neuron_sp1_g.tif", grey_stack);
#endif

#if 0
  Stack *stack = Read_Lsm_Stack("/Users/zhaot/Data/2009/011009_13304B_Ng2Av79/Slice15.mdb/slice15_L33_Sum.lsm", 1);
  Stack *grey_stack = stack;
  Write_Stack("../data/mouse_neuron_sp2.tif", grey_stack);
  system("mkdir ../data/mouse_neuron_sp2");
  int stat = 
    symlink("../datamouse_neuron_sp2.tif", "../data/mouse_neuron_sp2_org.tif");
  if (stat != 0) {
    perror(strerror(stat));
    return 1;
  }
#endif

#if 0
  system("cp ../data/pollen/mask.tif ../data/benchmark/pollen_bw.tif");
#endif

#if 0
  create_data("/Users/zhaot/Data/neurolineage/lsm/Twin-Spot_Gal4-GH146_nc_02-2.lsm.tif", "fly_neuron_h1", 1);
#endif

#if 0
  create_data("/Users/zhaot/Data/neurolineage/lsm/Twin-Spot_Gal4-GH146_nc_02-2.lsm.tif", "fly_neuron_h1", 1);
#endif

#if 0
  create_data("/Users/zhaot/Data/neurolineage/lsm/07_05LHregion.lsm.tif", "fly_neuron_h2", 1);
#endif

#if 0
  double res[3] = {0.1, 0.1, 0.2};
  create_data2("/Users/zhaot/Data/jc/S152_1NAsyanseLabled_CanWeTrace.tif", "../data/mouse_neuron_rj8_a", 0, 0, res);  
#endif

#if 0
  double res[3] = {0.1, 0.1, 0.2};
  create_data2("/Users/zhaot/Data/jc/S152_1NAsyanseLabled_CanWeTrace.tif", "../data/mouse_neuron_rj8_b", 1, 0, res);  
#endif

#if 0
  double res[3] = {0.1, 0.1, 0.2};
  create_data2("/Users/zhaot/Data/jc/S152_1NAsyanseLabled_CanWeTrace2.tif", "../data/mouse_neuron_rj9_a", 0, 0, res);  
#endif

#if 0
  double res[3] = {0.1, 0.1, 0.2};
  create_data2("/Users/zhaot/Data/jc/S152_1NAsyanseLabled_CanWeTrace2.tif", "../data/mouse_neuron_rj9_b", 1, 0, res);  
#endif

#if 0
  double res[3] = {0.13, 0.13, 0.75};
  create_data2("/Users/zhaot/Data/wayne/R57F07_041409_1024.lsm", "../data/fly_neuron_w1_a", 0, 0, res);
#endif

#if 0
  double res[3] = {0.13, 0.13, 0.75};
  create_data2("/Users/zhaot/Data/wayne/R57F07_041409_1024.lsm", "../data/fly_neuron_w1_b", 1, 0, res);
#endif

#if 0
  double res[3] = {0.26, 0.26, 0.75};
  Stack *stack = Read_Stack("../data/fly_neuron_w1_a.tif");
  Downsample_Stack_Mean(stack, 1, 1, 0, stack);

  create_data3(stack, "fly_neuron_w1_a_ds", 0, res);
#endif

#if 0
  double res[3] = {0.46, 0.46, 2.00};
  create_data2("/Users/zhaot/Data/wayne/stitching_bad_set/060808_R1_GR1_B1_L008.lsm", "../data/fly_neuron_w2_a", 0, 0, res);
#endif

#if 0
  double res[3] = {0.26, 0.26, 0.75};
  create_data2("/Users/zhaot/Data/wayne/R57F07-e-ivs-041209B.lsm", "../data/fly_neuron_w3_a", 0, 0, res);
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 15);
  voxel_t start = {20, 20, 7};
  voxel_t end = {70, 50, 10};
  Object_3d *obj = Line_To_Object_3d(start, end);
  Stack_Draw_Object_Bw(stack, obj, 1);

  Write_Stack("../data/benchmark/line.tif", stack);
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 15);
  Zero_Stack(stack);
  draw_sphere(stack, 0, 5, 20, 20, 5);
  Write_Stack("../data/benchmark/stack_graph/mask1.tif", stack);

#endif

#if 0 /* process diadem_c1 info */
  FILE *fp = fopen("../data/diadem_c1_rootinfo.txt", "r");
  
  String_Workspace *sw = New_String_Workspace();

  char *line = NULL;
  int n = 0;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(line) == FALSE) {
      n++;
    }
  }

  fseek(fp, 0, SEEK_SET);
  
  FILE *fp2 = fopen("../data/diadem_c1_root.txt", "w");
  FILE *fp3 = fopen("../data/diadem_c1_root.swc", "w");
  FILE *fp4 = fopen("../data/diadem_c1_root_org.txt", "w");
  FILE *fp5 = fopen("../data/diadem_c1_root_org.swc", "w");
  int array[4];
  n = 1;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(line) == FALSE) {
      int m;
      String_To_Integer_Array(line, array, &m);
      fprintf(fp2, "%d %d %d\n", array[1], array[2] + 14, array[3] + 21);
      fprintf(fp3, "%d 3 %d %d %d 3.0 -1\n", n, 
	      array[1], array[2] + 14, array[3] + 21);
      fprintf(fp4, "%d %d %d\n", array[1], array[2], array[3]);
      fprintf(fp5, "%d 3 %d %d %d 3.0 -1\n", n++, 
	      array[1], array[2], array[3]);
    }
  }
  fclose(fp4);
  fclose(fp5);

  Kill_String_Workspace(sw);
  fclose(fp);
#endif

#if 0
  FILE *fp = fopen("../data/diadem_d1_rootinfo.txt", "r");
  
  String_Workspace *sw = New_String_Workspace();

  char *line = NULL;
  int n = 0;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(line) == FALSE) {
      n++;
    }
  }

  fseek(fp, 0, SEEK_SET);
  
  FILE *fp2 = fopen("../data/diadem_d1_root.txt", "w");
  FILE *fp3 = fopen("../data/diadem_d1_root.swc", "w");
  FILE *fp4 = fopen("../data/diadem_d1_root_org.txt", "w");
  FILE *fp5 = fopen("../data/diadem_d1_root_org.swc", "w");
  int array[4];
  n = 1;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(line) == FALSE) {
      int m;
      String_To_Integer_Array(line, array, &m);
      fprintf(fp2, "%d %d 0\n", array[1], array[2] - 7382);
      fprintf(fp3, "%d 3 %d %d 0 3.0 -1\n", n, 
	      array[1], array[2] - 7382);
      fprintf(fp4, "%d %d 0\n", array[1], array[2]);
      fprintf(fp5, "%d 3 %d %d 0 3.0 -1\n", n++, array[1], array[2]);
    }
  }
  fclose(fp4);
  fclose(fp5);

  Kill_String_Workspace(sw);
  fclose(fp);
#endif

#if 0
  printf("static void geo3d_ball_field_u(Geo3d_Scalar_Field *field)\n");
  printf("{\n");
  printf("  double x, y, z;\n");

  double x, y, z;
  int k = 0;
  for (z = -1.0; z <= 1.0; z += 0.2) {
    for (y = -1.0; y <= 1.0; y += 0.2) {
      for (x = -1.0; x <= 1.0; x += 0.2) {
	if (x * x + y * y + z * z <= 1.0001) {
	  if (fabs(x) < 0.01) {
	    x = 0.0;
	  }
	  if (fabs(y) < 0.01) {
	    y = 0.0;
	  }
	  if (fabs(z) < 0.01) {
	    z = 0.0;
	  }
	  printf("  x = %.1f;\n", x);
	  printf("  y = %.1f;\n", y);
	  printf("  z = %.1f;\n", z);
	  printf("  field->points[%d][0] = x;\n", k);		
	  printf("  field->points[%d][1] = y;\n", k);		
	  printf("  field->points[%d][2] = z;\n", k);
	  printf("  field->values[%d] = exp(-(x*x + y*y + z*z));\n", k);
	  k++;
	}
      }
    }
  }
  printf("}\n");
#endif

#if 0
  Stack *stack = Index_Stack(GREY, 5, 5, 3);
  Print_Stack_Value(stack);
#endif

#if 0
  int start[3] = {50, 50, 10};
  int end[3] = {50, 50, 40};
  double sigma[3] = {1.5, 1.5, 3.0};
  Stack *stack = Tube_Stack(start, end, 40, sigma);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Circle_Image(10.0);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Noisy_Ellipse_Image(5.0, 10.0, 100);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Two_Circle_Image(10.0, 25.0, 25.0, 10.0, 39.0, 39.0);
  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack =
    Read_Lsm_Stack("/Users/zhaot/Data/jinny/Stitching_problem/4Stitching/"
	"slice07_L12_Sum.lsm", 1);
  Stack *substack = Crop_Stack(stack, 383, 245, 77, 704-383, 429-195, 124-77,
      NULL); 
  Write_Stack("../data/test/soma.tif", substack);
#endif

#if 0
  Stack *stack =
    Read_Lsm_Stack("/Users/zhaot/Data/jinny/Stitching_problem/4Stitching/"
	"slice07_L12_Sum.lsm", 1);
  Stack *substack = Crop_Stack(stack, 0, 124, 0, 1024, 679-124+1, 146,
      NULL); 
  Write_Stack("../data/test/soma2.tif", substack);
#endif

#if 0
  Stack *stack = Index_Stack(GREY, 5, 5, 5);
  Write_Stack("../data/test/grasp/index.tif", stack);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 5, 5, 5);
  uint8 value = 5;
  Stack_Set_Constant(stack, &value);
  Write_Stack("../data/test/grasp/const.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/diadem/diadem_e1.tif");
  Noisy_Stack(stack, 10.0);
  Write_Stack("../data/benchmark/diadem/diadem_e1_n10.tif", stack);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 20, 20, 1);
  Zero_Stack(stack);
  Set_Stack_Pixel(stack, 5, 5, 0, 0, 1);
  Set_Stack_Pixel(stack, 3, 11, 0, 0, 1);
  Set_Stack_Pixel(stack, 3, 17, 0, 0, 1);
  Set_Stack_Pixel(stack, 7, 15, 0, 0, 1);
  Write_Stack("../data/benchmark/swc/puncta_tree_blue.tif", stack);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 100, 100, 1);
  Zero_Stack(stack);
  draw_2d_gaussian(stack, 50, 50, 5, 3, 0);
  draw_2d_gaussian(stack, 30, 50, 5, 3, 1);
  draw_2d_gaussian(stack, 30, 30, 7, 7, 0);
  draw_2d_gaussian(stack, 70, 70, 2, 7, 0);
  draw_2d_gaussian(stack, 20, 20, 10, 1, 0);
  draw_2d_gaussian(stack, 20, 70, 10, 1, -3);
  Write_Stack("../data/benchmark/gaussians.tif", stack);
  
#endif

#if 0
  Stack *stack = Read_Stack("/mnt/Ginkgo_Images/benchmark/fake_neuron2.tif");
  draw_2d_gaussian(stack, 50, 50, 1, 3, 0);
  draw_2d_gaussian(stack, 30, 50, 1, 2, 1);
  draw_2d_gaussian(stack, 30, 30, 1, 1, 0);
  draw_2d_gaussian(stack, 70, 70, 1, 2, 0);
  draw_2d_gaussian(stack, 20, 20, 1, 1, 0);
  draw_2d_gaussian(stack, 20, 70, 1, 1, 0.1);
  Write_Stack("../data/benchmark/fake_spine.tif", stack);
#endif

/*faint fiber*/
#if 0
  Stack *stack = Make_Stack(1, 100, 100, 1);
  voxel_t start = {10, 10, 0};
  voxel_t end = {70, 50, 0};
  Object_3d *obj = Line_To_Object_3d(start, end);
  Stack_Draw_Object_Bw(stack, obj, 2);
  double sigma[] = {0.8, 0.8, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i] / 15;
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/benchmark/faint_fiber.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(1, 100, 100, 1);
  voxel_t start = {10, 10, 0};
  voxel_t end = {70, 50, 0};
  Object_3d *obj = Line_To_Object_3d(start, end);
  Stack_Draw_Object_Bw(stack, obj, 2);

  voxel_t start2 = {70, 50, 0};
  voxel_t end2 = {50, 50, 0};
  Object_3d *obj2 = Line_To_Object_3d(start2, end2);
  Stack_Draw_Object_Bw(stack, obj2, 2);

  double sigma[] = {1.0, 1.0, 3.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  Stack *out = Scale_Float_Stack(f->array, f->dim[0], f->dim[1], f->dim[2], 
				 GREY);
  
  int i;
  int nvoxel = Stack_Voxel_Number(out);
  double mu = 10.0;
  for (i = 0; i < nvoxel; i++) {
    unsigned int noise = Poissonrnd(mu);

    noise += out->array[i];
    if (noise > 255) {
      noise = 255;
    }
    out->array[i] = noise;
  }
  
  Write_Stack("../data/benchmark/sharp_turn.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 100, 100, 1);
  Zero_Stack(stack);
  draw_2d_gaussian(stack, 50, 50, 1, 3, 0);
  draw_2d_gaussian(stack, 30, 50, 1, 2, 1);
  draw_2d_gaussian(stack, 30, 30, 1, 1, 0);
  draw_2d_gaussian(stack, 70, 70, 1, 2, 0);
  draw_2d_gaussian(stack, 20, 20, 1, 1, 0);
  draw_2d_gaussian(stack, 20, 70, 1, 1, 0.1);
  Stack_Binarize(stack);

  Write_Stack("../data/benchmark/isolated_objects.tif", stack);
#endif

#if 0
  char filePath[500];
  int x[15] = { 
    10, 20, 30, 40, 50, 60, 70, 80, 90, 10, 20, 30, 40, 50, 60
  };
  int y[15] = { 
    10, 20, 30, 40, 50, 60, 70, 80, 90, 60, 50, 40, 30, 20, 10
  };
  int r[15] = { 
    3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5
  };
    
  int i;
  for (i = 0; i < 15; i++) {
    Stack *stack = Make_Stack(GREY, 200, 200, 1);
    Zero_Stack(stack);
    draw_2d_gaussian(stack, x[i] * 2, y[i] * 2, r[i], r[i], 0);
    Stack_Binarize(stack);
    sprintf(filePath, "../data/benchmark/series/image%d.tif", i);
    Write_Stack(filePath, stack);
    Free_Stack(stack);
  }
#endif

#if 0
  /*Make coordinates
   * For each image, save it as sp_map.%05dplane.tif (turn it into png with
   * imagej afterwards)
   */
  
  int planeNumber = 12;
  int startPlane = 30;

  int coordx[] = {100, 140, 180, 220, 260, 300, 340, 380, 420, 460};
  int coordy[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
  int superpixel_id_r[] = {128, 128, 128, 128, 128, 128, 128, 128, 255, 255};
  int superpixel_id_g[] = {0, 0, 0, 128, 128, 128, 255, 255, 0, 0};
  int superpixel_id_b[] = {0, 128, 255, 0, 128, 255, 0, 128, 255, 0, 128};
  int zoffset = 0;

  int coordx2[10];
  int i;
  for (i = 0; i < 10; i++) {
    coordx2[9 - i] = coordx[i];
  }
  int coordy2[] = {400, 400, 400, 400, 400, 400, 400, 400, 400, 400};
  int superpixel_id_r2[] = {64, 64, 64, 64, 64, 64, 64, 64, 192, 192};
  int superpixel_id_g2[] = {0, 0, 0, 128, 128, 128, 255, 255, 0, 0};
  int superpixel_id_b2[] = {0, 128, 255, 0, 128, 255, 0, 128, 255, 0, 128};
  int zoffset2 = 1;

  int body_id1;
  int body_id2;
  int body_id3;

  int coordx3[] = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310};
  int coordy3[] = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310};
  int superpixel_id_r3[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100};
  int superpixel_id_g3[] = {0, 0, 0, 128, 128, 128, 255, 255, 0, 0, 10, 0};
  int superpixel_id_b3[] = {0, 128, 255, 0, 128, 255, 0, 128, 255, 0, 128, 0, 10};
  int zoffset3 = 0;

  color_t body_color;
  body_color[0] = 255;
  body_color[1] = 255;
  body_color[2] = 0;
  body_id1 = Color_To_Value(body_color);
  
  body_color[0] = 255;
  body_color[1] = 0;
  body_color[2] = 255;
  body_id2 = Color_To_Value(body_color);

  body_id3 = 100000000;

  double r = 100.0;
  Stack *stack = Make_Stack(COLOR, 1024, 1024, 1);
  Stack *mask = Make_Stack(GREY, 1024, 1024, 1);
  int nvoxel = Stack_Voxel_Number(stack);

  color_t *arrayc = (color_t*) stack->array;
  char file_path[500];
  FILE *fp1 = fopen("../data/benchmark/flyem/superpixel_to_segment_map.txt",
      "w");

  FILE *fp2 = fopen("../data/benchmark/flyem/segment_to_body_map.txt",
      "w");

  int zEnd = sizeof(coordx) / sizeof(coordx[0]) + zoffset - 1;
  int zEnd2 = sizeof(coordx2) / sizeof(coordx2[0]) + zoffset2 - 1;
  int zEnd3 = sizeof(coordx3) / sizeof(coordx3[0]) + zoffset3 - 1;

  for (i = 0; i < planeNumber; i++) {
    int plane_id = i + startPlane;
    color_t superpixel_color;

    Zero_Stack(stack);

    if (i >= zoffset && i <= zEnd) {
      int k = i - zoffset;
      Zero_Stack(mask);
      draw_2d_gaussian(mask, coordx[k] * 2, coordy[k] * 2, r, r, 0);
      Stack_Binarize(mask);
      int offset = 0;
      for (offset = 0; offset < nvoxel; offset++) {
        if (mask->array[offset] == 1) {
          arrayc[offset][0] = superpixel_id_r[k];
          arrayc[offset][1] = superpixel_id_g[k];
          arrayc[offset][2] = superpixel_id_b[k];
        }
      }
      superpixel_color[0] = superpixel_id_r[k];
      superpixel_color[1] = superpixel_id_g[k];
      superpixel_color[2] = superpixel_id_b[k];
      int superpixel_id = Color_To_Value(superpixel_color);
      int segment_id = i * 2 % 4;
      fprintf(fp1, "%d\t%d\t%d\n", plane_id, superpixel_id, segment_id); 
      fprintf(fp2, "%d\t%d\n", segment_id, body_id1);
    }

    if (i >= zoffset2 && i <= zEnd2) {
      int k = i - zoffset2;
      Zero_Stack(mask);
      draw_2d_gaussian(mask, coordx2[k] * 2, coordy2[k] * 2, r, r, 0);
      Stack_Binarize(mask);
      int offset = 0;
      for (offset = 0; offset < nvoxel; offset++) {
        if (mask->array[offset] == 1) {
          arrayc[offset][0] = superpixel_id_r2[k];
          arrayc[offset][1] = superpixel_id_g2[k];
          arrayc[offset][2] = superpixel_id_b2[k];
        }
      }
      superpixel_color[0] = superpixel_id_r2[k];
      superpixel_color[1] = superpixel_id_g2[k];
      superpixel_color[2] = superpixel_id_b2[k];
      int superpixel_id =  Color_To_Value(superpixel_color);
      int segment_id = i * 2 + 1;
      fprintf(fp1, "%d\t%d\t%d\n", plane_id, superpixel_id, segment_id); 
      fprintf(fp2, "%d\t%d\n", segment_id, body_id2);
    }

    if (i >= zoffset3 && i <= zEnd3) {
      int k = i - zoffset3;
      Zero_Stack(mask);
      draw_2d_gaussian(mask, coordx3[k] * 2, coordy3[k] * 2, r, r, 0);
      Stack_Binarize(mask);
      int offset = 0;
      for (offset = 0; offset < nvoxel; offset++) {
        if (mask->array[offset] == 1) {
          arrayc[offset][0] = superpixel_id_r3[k];
          arrayc[offset][1] = superpixel_id_g3[k];
          arrayc[offset][2] = superpixel_id_b3[k];
        }
      }
      superpixel_color[0] = superpixel_id_r3[k];
      superpixel_color[1] = superpixel_id_g3[k];
      superpixel_color[2] = superpixel_id_b3[k];
      int superpixel_id =  Color_To_Value(superpixel_color);
      int segment_id = (i + 10) * 5;
      fprintf(fp1, "%d\t%d\t%d\n", plane_id, superpixel_id, segment_id); 
      fprintf(fp2, "%d\t%d\n", segment_id, body_id3);
    }

    sprintf(file_path,
        "../data/benchmark/flyem/superpixel_maps/tif/sp_map.%05d.tif", plane_id);
    Write_Stack(file_path, stack);
  }

  fclose(fp1);
  fclose(fp2);
#endif

#if 0
  /*Make coordinates
   * For each image, save it as sp_map.%05dplane.tif (turn it into png with
   * imagej afterwards)
   */
  
  int planeNumber = 12;
  int startPlane = 30;

  int coordx[] = {100, 140, 180, 220, 260, 300, 340, 380, 420, 460};
  int coordy[] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100};
  int zoffset = 0;

  int coordx2[10];
  int i;
  for (i = 0; i < 10; i++) {
    coordx2[9 - i] = coordx[i];
  }
  int coordy2[] = {400, 400, 400, 400, 400, 400, 400, 400, 400, 400};
  int zoffset2 = 1;

  int body_id1;
  int body_id2;
  int body_id3;

  int coordx3[] = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310};
  int coordy3[] = {200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300, 310};
  int zoffset3 = 0;

  color_t body_color;
  body_color[0] = 255;
  body_color[1] = 255;
  body_color[2] = 0;
  body_id1 = Color_To_Value(body_color);
  
  body_color[0] = 255;
  body_color[1] = 0;
  body_color[2] = 255;
  body_id2 = Color_To_Value(body_color);

  body_id3 = 100000000;

  int zEnd = sizeof(coordx) / sizeof(coordx[0]) + zoffset - 1;
  int zEnd2 = sizeof(coordx2) / sizeof(coordx2[0]) + zoffset2 - 1;
  int zEnd3 = sizeof(coordx3) / sizeof(coordx3[0]) + zoffset3 - 1;

  double r = 100.0;
  Stack *mask = Make_Stack(GREY, 1024, 1024, 1);

  char filePath[500];
  for (i = 0; i < planeNumber; i++) {
    int plane_id = i + startPlane;
    sprintf(filePath, 
        "../data/benchmark/flyem/grayscale-data/xy-grayscale-%d.raw", plane_id);

    if (i >= zoffset && i <= zEnd) {
      int k = i - zoffset;
      Zero_Stack(mask);
      draw_2d_gaussian(mask, coordx[k] * 2, coordy[k] * 2, r, r, 0);
    }

    if (i >= zoffset2 && i <= zEnd2) {
      int k = i - zoffset2;
      draw_2d_gaussian(mask, coordx2[k] * 2, coordy2[k] * 2, r, r, 0);
    }

    if (i >= zoffset3 && i <= zEnd3) {
      int k = i - zoffset3;
      draw_2d_gaussian(mask, coordx3[k] * 2, coordy3[k] * 2, r, r, 0);
    }

    Write_Flyem_Raw_Stack_Plane(filePath, mask);
    //Write_Stack(filePath, mask);
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/sphere_bw.tif");
  Stack *out = Crop_Stack(stack, 59, 62, 50, 30, 30, 5, NULL);
  Write_Stack("../data/benchmark/sphere_bw_crop.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  dim_type dim[3] = {3, 3, 3};

  FILE *fp = fopen("conn26_cut_id.txt", "w");
  IMatrix *chord = Make_IMatrix(dim, 3);
  int i;
  for (i = 0; i < 67108863; ++i) {
  //for (i = 100000; i < 200000; ++i) {
    int id = i;
    printf("%d :  ", id);
    Zero_Stack(stack);
    int index = 0;
    while (id > 0) {
      if (id % 2 > 0) {
        stack->array[index] = 1;
      }
      id /= 2;
      ++index;
      if (index == 13) {
        ++index;
      }
    }
    //printf("%g\n", Stack_Sum(stack));
    //Print_Stack_Value(stack);

    int nobj = Stack_Label_Objects_N(stack, chord, 1, 2, 26);
    if (nobj > 1) {
      fprintf(fp, "%d\n", i);
      printf("*** A cut. ***\n");
    }
    printf("\n");
  }
  fclose(fp);
  //Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 100, 100, 100);

  One_Stack(stack);
  Write_Stack("../data/test.tif", stack);

  Stack *stack2 = Make_Stack(GREY, 100, 200, 50);
  One_Stack(stack2);
  Write_Stack("../data/test2.tif", stack2);

  Stack *stack3 = Make_Stack(GREY, 50, 50, 50);
  One_Stack(stack3);
  Write_Stack("../data/test3.tif", stack3);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/number3.tif");
  Stack *numstack = Make_Stack(GREY, stack->width, stack->height, 1);
  int area = stack->width * stack->height;

  int i;
  char outfile[500];
  for (i = 0; i < 10; ++i) {
    numstack->array = stack->array + i * area;
    Stack *cropped_numstack = Stack_Bound_Crop(numstack, 0);
    sprintf(outfile, "../data/benchmark/digit%d.tif", i);
    Write_Stack(outfile, cropped_numstack);
    Kill_Stack(cropped_numstack);
  }
#endif

#if 0
  Image *image = Read_Em_Raw("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/gray/xy-grayscale-1000.raw");
  Stack stack = Stack_View_Image(image);
  Stack *dsstack = Downsample_Stack(&stack, 9, 9, 0);
  Write_Stack("../data/test.tif", dsstack);
#endif

#if 0
  File_List *list = File_List_Load_Dir("/groups/flyem/data/medulla-TEM-fall2008/integrate-20120422/data/grayscale-data/xy", "raw", NULL);
  File_List_Sort_By_Number(list);

  int i;
  char output[500];
  for (i = 0; i < list->file_number; ++i) {
    Image *image = Read_Em_Raw(list->file_path[i]);
    Stack stack = Stack_View_Image(image);
    Stack *dsstack = Downsample_Stack_Mean(&stack, 9, 9, 0, NULL);
    sprintf(output, "../data/flyem/TEM/gray_ds_avg/xy-grayscale-%05d.tif", i + 161);
    printf("Writing %s ...\n", output);
    Write_Stack(output, dsstack);
    Kill_Image(image);
  }
#endif

#if 0
  File_List *list = File_List_Load_Dir("/groups/flyem/data/medulla-TEM-fall2008/integrate-20120422/data/grayscale-data/xy", "raw", NULL);
  File_List_Sort_By_Number(list);

  int i;
  char output[500];
  for (i = 1106; i < 1107; ++i) {
    Image *image = Read_Em_Raw(list->file_path[i]);
    Stack stack = Stack_View_Image(image);
    sprintf(output, "../data/flyem/TEM/xy-grayscale-%05d.tif", i + 161);
    printf("Writing %s ...\n", output);
    Write_Stack_U(output, &stack, NULL);
    Kill_Image(image);
  }
#endif

#if 0
  Stack *stack = Read_Stack("/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session3/body_maps/body_map01267.tif");
  color_t* array = (color_t*) stack->array;
  size_t offset = 0;
  size_t area = stack->width * stack->height;
  Stack *out = Make_Stack(GREY, stack->width, stack->height, 1);

  for (offset = 0; offset < area; ++offset) {
    if ((array[offset][0] == 161) || (array[offset][1] == 5) ||
        (array[offset][2] == 7)) {
      out->array[offset] = 1;
    } else {
      out->array[offset] = 0;
    }
  }

  Stack *dsstack = Downsample_Stack_Max(out, 1, 1, 0, NULL);
  
  int seed = 660 * 1200 + 660;
  Stack_Label_Object_N(dsstack, NULL, seed, 1, 255, 26);

  Stack_Threshold_Binarize(dsstack, 128);
  

  Write_Stack("../data/flyem/TEM/T4_mask.tif", dsstack);

#endif

#if 0
  Stack *stack = Read_Stack("/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session3/body_maps/body_map01267.tif");
  color_t* array = (color_t*) stack->array;
  size_t offset = 0;
  size_t area = stack->width * stack->height;
  Stack *out = Make_Stack(GREY, stack->width, stack->height, 1);

  for (offset = 0; offset < area; ++offset) {
    if ((array[offset][0] == 55) || (array[offset][1] == 207) ||
        (array[offset][2] == 6)) {
      out->array[offset] = 1;
    } else {
      out->array[offset] = 0;
    }
  }

  Stack *dsstack = Downsample_Stack_Max(out, 1, 1, 0, NULL);
  
  int seed = 1246 / 2 * 1200 + 1356 / 2;
  Stack_Label_Object_N(dsstack, NULL, seed, 1, 255, 26);

  Stack_Threshold_Binarize(dsstack, 128);
  

  Write_Stack("../data/flyem/TEM/Mi1_mask.tif", dsstack);

#endif

#if 0
  Stack *stack = Read_Stack("../data/flyem/TEM/gray_ds10_avg/xy-grayscale-01267.tif");
  Stack *mask1 = Read_Stack("../data/flyem/TEM/T4_mask.tif"); //green
  Stack *mask2 = Read_Stack("../data/flyem/TEM/Mi1_mask.tif"); //blue

  Stack *out = Make_Stack(COLOR, stack->width, stack->height, stack->depth);

  color_t *out_array = (color_t*) out->array;
  size_t voxelNumber = Stack_Voxel_Number(stack);

  size_t i;
  for (i = 0; i < voxelNumber; ++i) {
    out_array[i][0] = stack->array[i];
    out_array[i][1] = out_array[i][0];
    out_array[i][2] = out_array[i][0];
    int v = out_array[i][0];
    v += 127;
    if (v > 255) {
      v = 255;
    }
    if (mask1->array[i] > 0) {
      out_array[i][1] = v;
    } else if (mask2->array[i] > 0) {
      out_array[i][2] = v;
    }
  }

  Write_Stack("../data/flyem/TEM/movie/actor/colored_slice.tif", out);
#endif

#if 0
  Stack *stack = Read_Stack("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/gray_ds10_avg/xy-grayscale-01267.tif");
  Stack *mask1 = Read_Stack("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/movie/actor/T4-12-mask.tif"); //green
  Stack *mask2 = Read_Stack("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/movie/actor/Mi1-mask.tif"); //blue

  Stack *out = Make_Stack(COLOR, stack->width, stack->height, stack->depth);

  color_t *out_array = (color_t*) out->array;
  size_t voxelNumber = Stack_Voxel_Number(stack);

  size_t i;
  for (i = 0; i < voxelNumber; ++i) {
    out_array[i][0] = stack->array[i];
    out_array[i][1] = out_array[i][0];
    out_array[i][2] = out_array[i][0];
    int v = out_array[i][0];
    v += 127;
    if (v > 255) {
      v = 255;
    }
    if (mask1->array[i] > 0) {
      out_array[i][1] = v;
    } else if (mask2->array[i] > 0) {
      out_array[i][2] = v;
    }
  }

  Write_Stack("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/movie/actor/colored_slice.tif", out);
#endif

#if 1
  Stack *stack = Make_Stack(GREY, 5, 5, 1);
  Zero_Stack(stack);
  int i;
  for (i = 0; i < 25; ++i) {
    stack->array[i] = i * 10;
  }
  Write_Stack("../data/test.tif", stack);
#endif

  return 0;
}
