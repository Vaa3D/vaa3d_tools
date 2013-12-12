#include <stdio.h>
#include "tz_image_lib.h"

int main(int argc, char *argv[])
{
  Image *image = Read_Image("../data/testimg.tif");
  Print_Image_Value(image);

  Image *image2 = Image_Block_Sum(image, 6, 4);
  Print_Image_Value(image2);

  return 0;
}
