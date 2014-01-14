/**@file testpng.c
 * @author Ting Zhao
 * @date 03-Jul-2012
 */

#include "tz_utilities.h"
#include "tz_png_io.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"
#include "tz_string.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-t")) {
  }

  if (Is_Png("../data/sp_map.png") == TRUE) {
    printf("It is a png file.\n");
  } else {
    printf("It is not a png file.\n");
  }


  if (Is_Png("../data/benchmark/fork_2d.tif") == TRUE) {
    printf("It is a png file.\n");
  } else {
    printf("It is not a png file.\n");
  }

#if 0
  Stack *stack = Read_Png("/media/wd/data/dalhousie-20120717/superpixel_maps/sp_map.00257.png");
  stack = Downsample_Stack(stack, 4, 4, 0);
  Write_Stack("/media/wd/data/dalhousie-20120717/ds/sp_map.00257.tif", stack);
#endif

#if 0
  File_List *file_list = File_List_Load_Dir("/media/wd/data/dalhousie-20120717/superpixel_maps", "png", NULL);
  int i;
  for (i = 0; i < file_list->file_number; i++) {
    char file_path[500];
    sprintf(file_path, "/media/wd/data/dalhousie-20120717/ds/sp_map.%05d.tif", 
        String_Last_Integer(file_list->file_path[i]));
    if (!fexist(file_path)) {
      printf("%d/%d\n", i, file_list->file_number);
      printf("%s\n", file_list->file_path[i]);
      Stack *stack = Read_Png(file_list->file_path[i]);
      printf("-->\n");
      Stack *ds = Downsample_Stack(stack, 4, 4, 0);
      printf("%s\n", file_path);
      Write_Stack(file_path, ds);
      Free_Stack(ds);
      Free_Stack(stack);
    }
  }

#endif

#if 0
  File_Bundle_S file_bundle;
  file_bundle.prefix = "../data/ting_example_stack/superpixel_maps/sp_map.";
  file_bundle.suffix = ".png";
  file_bundle.num_width = 5;
  file_bundle.first_num = 611;
  file_bundle.last_num = 621;

  stack = Read_Stack_Planes_S(&file_bundle);
  Write_Stack("../data/test.tif", stack);
#endif

  return 0;
}
