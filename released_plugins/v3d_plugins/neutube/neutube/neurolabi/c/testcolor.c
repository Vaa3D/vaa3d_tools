/* testcolor.c
 *
 * 28-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_stack_draw.h"
#include "tz_error.h"
#include "tz_image_lib_defs.h"

INIT_EXCEPTION_MAIN(e)

int main()
{
#if 0
  Rgb_Color color;
  Set_Color_Hsv(&color, 4.9, 0.53, 0.06);
  Print_Rgb_Color(&color);

  double h, s, v;
  Rgb_Color_To_Hsv(&color, &h, &s, &v);

  printf("%g, %g, %g\n", h, s, v);
#endif

#if 1
  Rgb_Color color;
  Set_Color(&color, 128, 255, 0);
  Print_Rgb_Color(&color);

  double h, s, v;
  Rgb_Color_To_Hsv(&color, &h, &s, &v);

  printf("%g, %g, %g\n", h, s, v);
#endif

#if 0
  Stack *stack = Make_Stack(COLOR, 100, 64, 1);
  int i, j;
  int offset = 0;
  Rgb_Color rgb;
  color_t *array = (color_t*) stack->array;
  for (j = 0; j < 64; j++) {
    Set_Color_Jet(&rgb, j);
    for (i = 0; i < 100; i++) {
      array[offset][0] = rgb.r;
      array[offset][1] = rgb.g;
      array[offset][2] = rgb.b;
      offset++;
    }
  }
  Write_Stack("../data/test.tif", stack);
#endif

  return 1;
}
