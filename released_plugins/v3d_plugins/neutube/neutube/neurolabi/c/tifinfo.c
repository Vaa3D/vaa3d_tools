/* tifinfo.c
 *
 * 22-Sep-2008 Initial write: Ting Zhao 
 */

/* tifinfo - show information of a tif file
 * 
 * tifinfo file
 */

#include <utilities.h>
#include "tz_image_io.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<image:string>", "[-s]",
			 NULL};

  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-s")) {
    int size[3];
    if (Is_Lsm(Get_String_Arg("image")) == 1) {
      Lsm_Size(Get_String_Arg("image"), size);
    } else {
      Tiff_Size(Get_String_Arg("image"), size);
    }
    printf("%d x %d x %d\n", size[0], size[1], size[2]);
  } else {
    if (Is_Lsm(Get_String_Arg("image")) == 1) {
      Print_Lsm_Info(Get_String_Arg("image"));
    } else {
      Print_Tiff_Info(Get_String_Arg("image"));
    }
  }

  return 0;
}
