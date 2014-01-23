#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <utilities.h>
#include "tz_error.h"
#ifdef HAVE_LIBGSL
#  include <gsl/gsl_statistics_double.h>
#endif
#include "tz_constant.h"
#include "tz_iarray.h"
#include "tz_stack_attribute.h"
#include "tz_image_lib.h"
#include "tz_objdetect.h"
#include "tz_stack_math.h"
#include "tz_stack_bwmorph.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_darray.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_voxel_graphics.h"
#include "tz_stack.h"
#include "tz_math.h"
#include "tz_string.h"

static int help(int argc, char *argv[], char *spec[])
{
  if (argc == 2) {
    if (strcmp(argv[1], "-help") == 0) {
      while (*spec) {
	printf("%s ", *(spec++)); 
      }
      printf("\n");
      printf("-mrdist: save distance transformation;\n");
      printf("-mrseed: save seed mask;\n");
      printf("-w: weight the transformation by signal;\n");
      printf("-cs: seeds on the centers of local maximal regions;\n");
      printf("-boost: get more seeds.\n");
      printf("image: input mask image;\n");
      printf("-o: output.\n");
      return 1;
    }
  }

  return 0;
}

static void mark_seed_from_file(Stack *seeds, const char *path)
{
  String_Workspace *sw = New_String_Workspace();
  char *line = NULL;
  FILE *fp = fopen(path, "r");
  while ((line = Read_Line(fp, sw)) != NULL) {
    int pos[3];
    int n = 0;
    String_To_Integer_Array(line, pos, &n);
    if (n == 3) {
      Set_Stack_Pixel(seeds, pos[1], pos[0], pos[2], 0, 1);
    }
  }
  Kill_String_Workspace(sw);
  fclose(fp);
}

/*
 * trace_seed - creats seeds for tracing from a binary mask
 *
 * trace_seed [-mrdist <string>] [-mrseed <string>] [-w <string>] [-cs]
 *   <image:string> -o <string>
 *
 */
int main(int argc, char* argv[]) 
{
  static char *Spec[] = {
    "[-mrdist <string>] [-mrseed <string>] [-w <string>] [-cs] [-boost]",
    "[-seed_file <string>]"
    " <image:string> -o <string>",
    NULL};

  if (help(argc, argv, Spec) == 1) {
    return 0;
  }
  
  Process_Arguments(argc, argv, Spec, 1);

  printf("Creating seeds for tracing ...\n");

  Stack *mask = Read_Stack(Get_String_Arg("image"));
  Translate_Stack(mask, GREY, 1);

  //Stack_Not(mask, mask);

  /*
  const char *matlab_path = "/Applications/MATLAB_R2007b/bin/matlab";
  if (Is_Arg_Matched("-matlab")) {
    matlab_path = Get_String_Arg("-matlab");
  }
  */
  
  printf("  Building distance map ...\n");
  Stack *dist = Stack_Bwdist_L_U16(mask, NULL, 0);

  if (Is_Arg_Matched("-mrdist")) {
    /*
    Stack *dist_grey = Scale_Double_Stack((double *) dist->array, 
					  dist->width, dist->height,
					  dist->depth, GREY);
    Write_Stack(Get_String_Arg("-mrdist"), dist_grey);

    Kill_Stack(dist_grey);
    */
    Write_Stack(Get_String_Arg("-mrdist"), dist);
  }

  Kill_Stack(mask);

  Stack *seeds = NULL;
  if (Is_Arg_Matched("-seed_file")) {
    seeds = Make_Stack(GREY, Stack_Width(dist), Stack_Height(dist), 
        Stack_Depth(dist));
    mark_seed_from_file(seeds, Get_String_Arg("-seed_file"));
  }

  Stack *distw = dist;
  if (seeds == NULL) {
    if (Is_Arg_Matched("-w")) {
      distw = Make_Stack(FLOAT32, dist->width, dist->height, dist->depth);
      Stack *signal = Read_Stack(Get_String_Arg("-w"));
      double sigma[] = {1.0, 1.0, 0.5};
      FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
      FMatrix *f = Filter_Stack_Fast_F(signal, filter, NULL, 0);
      Kill_Stack(signal);

      Stack weight;
      weight.kind = FLOAT32;
      weight.width = dist->width;
      weight.height = dist->height;
      weight.depth = dist->depth;
      weight.array = (uint8*) (f->array);

      Stack_Mul(dist, &weight, distw);
      Kill_FMatrix(f);
    }

    if (Is_Arg_Matched("-cs")) {
      //seeds = Stack_Local_Max(dist, NULL, STACK_LOCMAX_FLAT);
      //Stack_Clean_Locmax(dist, seeds);
      seeds = Stack_Locmax_Region(distw, 26);

      Object_3d_List *objs = Stack_Find_Object_N(seeds, NULL, 1, 0, 26);
      Zero_Stack(seeds);
      int objnum = 0;
      while (objs != NULL) {
        Object_3d *obj = objs->data;
        Voxel_t center;
        Object_3d_Central_Voxel(obj, center);
        Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);

        if (Is_Arg_Matched("-boost")) {
          int erase_size = iround(Get_Stack_Pixel(dist, center[0], center[1],
                center[2], 0)) * 2;
          int u, v, w;
          for (w = -erase_size; w <= erase_size; w++) {
            for (v = -erase_size; v <= erase_size; v++) {
              for (u = -erase_size; u <= erase_size; u++) {
                int sub[3];
                sub[0] = center[0] + u;
                sub[1] = center[1] + v;
                sub[2] = center[2] + w;
                if (IS_IN_OPEN_RANGE3(sub[0], sub[1], sub[2], -1, seeds->width,
                      -1, seeds->height, -1, seeds->depth)) {
                  Set_Stack_Pixel(distw, sub[0], sub[1], sub[2], 0, 0.0);
                }
              }
            }
          }
        }

        objs = objs->next;
        objnum++;
      }

      if (Is_Arg_Matched("-boost")) {
        Stack *seed_stack2 = Stack_Locmax_Region(distw, 26);
        objs = Stack_Find_Object_N(seed_stack2, NULL, 1, 0, 26);
        while (objs != NULL) {
          Object_3d *obj = objs->data;
          Voxel_t center;
          Object_3d_Central_Voxel(obj, center);
          Set_Stack_Pixel(seeds, center[0], center[1], center[2], 0, 1);
          objs = objs->next;
        }
      }
    } else {
      seeds = Stack_Local_Max(distw, NULL, STACK_LOCMAX_CENTER);
    }
  }
  
  if (Is_Arg_Matched("-mrseed")) {
    Write_Stack(Get_String_Arg("-mrseed"), seeds);
  }

  Voxel_List *list = Stack_To_Voxel_List(seeds);  
  Pixel_Array *pa = Voxel_List_Sampling(dist, list);
  Voxel_P *voxel_array = Voxel_List_To_Array(list, 1, NULL, NULL);
  //double *pa_array = (double *) pa->array;
  uint16 *pa_array = (uint16 *) pa->array;

  printf("%d seeds found.\n", pa->size);

  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(pa->size);
  field->size = 0;
  int i;
  for (i = 0; i < pa->size; i++) {
    if (IS_IN_OPEN_RANGE3(voxel_array[i]->x, voxel_array[i]->y,
			  voxel_array[i]->z, 0, seeds->width - 1,
			  0, seeds->height - 1, 0, seeds->depth - 1)) {
      field->points[field->size][0] = voxel_array[i]->x;
      field->points[field->size][1] = voxel_array[i]->y;
      field->points[field->size][2] = voxel_array[i]->z;
      field->values[field->size] = sqrt((double)pa_array[i]);
      field->size++;
    }
  }

  Kill_Stack(dist);

  Write_Geo3d_Scalar_Field(Get_String_Arg("-o"), field);

  printf("%s created\n", Get_String_Arg("-o"));

  /*
  char file_path[100];
  strcpy(file_path, Get_String_Arg("-o"));
  strcat(file_path, ".pa");
  Pixel_Array_Write(file_path, pa);
  */

  /*
  printf("mean: %g, std: %g\n", gsl_stats_mean(field->values, 1, pa->size), 
	 sqrt(gsl_stats_variance(field->values, 1, pa->size)));
  printf("max: %g\n", gsl_stats_max(field->values, 1, pa->size));
  */

  free(voxel_array);
  Kill_Geo3d_Scalar_Field(field);
  Kill_Stack(seeds);
  Kill_Pixel_Array(pa);

  return 0;
}
