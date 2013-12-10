/**@file testsmap.c
 * @author Ting Zhao
 * @date 05-07-2012
 */

#include "tz_utilities.h"
#include "tz_png_io.h"
#include "tz_image_io.h"
#include "tz_string.h"
#include "tz_iarray.h"

typedef struct _Superpixel_Map {
  int segment_id;
  int body_id;
} Superpixel_Map;

#define MAX_SUPERPIXEL_NUMBER_PER_IMAGE 10000
#define MAX_SEGMENT_ID 1000000
#define MAX_PLANE_ID 1000

typedef struct _Plane_Map {
  int superpixel_number;
  Superpixel_Map *map;
} Plane_Map;

int main(int argc, char *argv[])
{
  static char *Spec[] = {"--plane_id <int> --neuron_id <int> -o <string> [-t]",
    NULL};
  Process_Arguments(argc, argv, Spec, 1);

  

  Plane_Map plane_map[MAX_PLANE_ID];
  int i;
  for (i = 0; i < MAX_PLANE_ID; i++) {
    plane_map[i].superpixel_number = 0;
    plane_map[i].map = (Superpixel_Map*) malloc(sizeof(Superpixel_Map) *
        MAX_SUPERPIXEL_NUMBER_PER_IMAGE);
    int j;
    for (j = 0; j < MAX_SUPERPIXEL_NUMBER_PER_IMAGE; j++) {
      plane_map[i].map[j].segment_id = -1;
      plane_map[i].map[j].body_id = -1;
    }
  }

  /*
  int segment_plane[MAX_SEGMENT_ID];
  int segment_index[MAX_SEGMENT_ID];
  for (int i = 0; i < MAX_SEGMENT_ID; i++) {
    segment_plane[i] = -1;
    segment_index[i] = -1;
  }
  */

  int plane_id = Get_Int_Arg("--plane_id");
  int neuron_id = Get_Int_Arg("--neuron_id");

  /* Read superpixel map and build segment-index map */
  /*
  FILE *fp = fopen("../data/ting_example_stack/superpixel_to_segment_map.txt",
      "r");
      */

  FILE *fp = NULL;
  String_Workspace *sw = New_String_Workspace();
  char *line = NULL;
  int n;

  int *segment_to_body_map;
  segment_to_body_map = iarray_malloc(MAX_SEGMENT_ID);
  /* Read segment map */
  int segment_map[3];
  fp = fopen("../data/ting_example_stack/segment_to_body_map.txt", "r");
  //fp = fopen("../data/segment_to_body_map.txt", "r");
  /* For each entry of the file */
  while ((line = Read_Line(fp, sw)) != NULL) {
    String_To_Integer_Array(line, segment_map, &n);
    if (n == 2) {
      segment_to_body_map[segment_map[0]] = segment_map[1];
#if 0
      /* If the segment is in the target plane */
      int segidx = segment_index[segment_map[0]];
      if (segidx >= 0) {
        int segplane = segment_plane[segidx];
        if (segplane == plane_id) { 
          /* Add cell body id to the superpixel map */
          plane_map[segplane].map[segidx].body_id = segment_map[1];

          printf("%d, %d, %d, %d\n", segplane, segidx, 
              plane_map[segplane].map[segidx].segment_id,
              plane_map[segplane].map[segidx].body_id);
        }
      }
#endif
    }
  }

  //fp = fopen("../data/superpixel_to_segment_map.txt", "r");
  fp = fopen("../data/ting_example_stack/superpixel_to_segment_map.txt", "r");

  int superpixel_map[3];
  /* For each row of the file */
  while ((line = Read_Line(fp, sw)) != NULL) {
    String_To_Integer_Array(line, superpixel_map, &n);
    if (n == 3) {
    /* If the plane id is the target */
      if (superpixel_map[0] == plane_id) {
        /* Add the plane id to the map at the superpixel ID position */
        plane_map[plane_id].superpixel_number++;
        plane_map[plane_id].map[superpixel_map[1]].segment_id = 
          superpixel_map[2];
        plane_map[plane_id].map[superpixel_map[1]].body_id = 
          segment_to_body_map[superpixel_map[2]];

        /* Add the index (superpixel id) to segment map */
        /*
        segment_plane[superpixel_map[2]] = plane_id;
        segment_index[superpixel_map[2]] = superpixel_map[1];
        */
      }
    }
  }

  fclose(fp);

  printf("%d superpixels\n", plane_map[plane_id].superpixel_number);
  for (i = 0; i < MAX_SUPERPIXEL_NUMBER_PER_IMAGE; i++) {
    Superpixel_Map map = plane_map[plane_id].map[i];
    if (map.body_id == neuron_id) {
    //if (map.segment_id >= 0) {
      printf("%d, %d, %d, %d\n", plane_id, i, map.segment_id, map.body_id); 
    }
    //}
  }

  /* Read image */
  File_Bundle_S file_bundle;
  file_bundle.prefix = "../data/ting_example_stack/superpixel_maps/sp_map.";
  file_bundle.suffix = ".png";
  file_bundle.num_width = 5;
  file_bundle.first_num = plane_id;
  file_bundle.last_num = plane_id;

  Stack *stack = Read_Stack_Planes_S(&file_bundle);
  Image_Array imarray;
  imarray.array = stack->array;

  int j;
  int offset = 0;
  Plane_Map pmap = plane_map[plane_id];
  /* For each pixel in the image*/
  for (j = 0; j < stack->height; j++) {
    for (i = 0; i < stack->width; i++) {
      int superpixel_id = imarray.array16[offset];
      /* If the pixel is from the target neuron */
      if (pmap.map[superpixel_id].body_id == neuron_id) {
        imarray.array16[offset] = 1;
      } else {
        imarray.array16[offset] = 0;
      }
      offset++;
    }
  }

  Translate_Stack(stack, GREY, 1);

  Write_Stack(Get_String_Arg("-o"), stack);
  //Stack_Binarize(

  return 0;
}
