/* draw_mask.c
 *
 * 22-Sep-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_stack_draw.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_lib.h"
#include "tz_stack_attribute.h"
#include "tz_image_io.h"

/* drawmask - draw a mask in a stack#include <ui_mainwindow.h>
 *
 * drawmask image -m mask -o out -s style
 */

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.0") == 1) {
    return 0;
  }

  static char *Spec[] = {"<image:string> -m <string> [-r <string>]",
			 "-o <string>",
			 "[-l <int>] [-s <int>] [-h <double>] [-c <int>]",
			 "[-n <int>] [-f]",
			 NULL};

  Process_Arguments(argc, argv, Spec, 1);

  int style = 1;
  if (Is_Arg_Matched("-s")) {
    style = Get_Int_Arg("-s");
  }

  Stack *canvas = Read_Stack_U(Get_String_Arg("image"));

  if (Is_Arg_Matched("-f")) {
    Flip_Stack_Y(canvas, canvas);
  }

  if (canvas->kind != COLOR) {
    Translate_Stack(canvas, COLOR, 1);
  }

  double h = 0.0;
  if (Is_Arg_Matched("-h")) {
    h = Get_Double_Arg("-h");
  }
  Rgb_Color color;

  if (Is_Arg_Matched("-c")) {
    switch (Get_Int_Arg("-c")) {
    case 0:
      color.r = 255;
      color.g = 0;
      color.b = 0;
      break;
    case 1:
      color.r = 0;
      color.g = 255;
      color.b = 0;
      break;
    case 2:
      color.r = 0;
      color.g = 0;
      color.b = 255;
      break;
    default:
      exit(1);
      break;
    } 
  } else {
    Set_Color_Hsv(&color, h, 1.0, 1.0);
  }

  double h2 = h + 2.0;

  int channel = 0;
  
  if (Is_Arg_Matched("-n")) {
    channel = Get_Int_Arg("-n") - 1;
  }

  Stack *mask = Read_Sc_Stack(Get_String_Arg("-m"), channel);

  if (Is_Arg_Matched("-l")) {
    Stack_Level_Mask(mask, Get_Int_Arg("-l"));
  }

  Stack *mask2 = NULL;
  if (Is_Arg_Matched("-r")) {
    mask2 = Read_Stack_U(Get_String_Arg("-r"));
  }

  switch (style) {
  case 1:
    Stack_Blend_Mc2(canvas, mask, h, mask2, h2);
    break;
  case 2:
    {
      Stack *bm = Stack_Perimeter(mask, NULL, 8);
      if (Is_Arg_Matched("-c")) {
	int i;
	int offset = 0;
	int nvoxel = Stack_Voxel_Number(canvas);
	int channel = Get_Int_Arg("-c");
	for (i = 0; i < nvoxel; i++) {
	  canvas->array[offset + channel] = 0;
	  if (bm->array[i] > 0) {
	    canvas->array[offset] = color.r;
	    canvas->array[offset + 1] = color.g;
	    canvas->array[offset + 2] = color.b;
	  }
	  offset += 3;
	}
      } else {
	Stack_Label_Bwc(canvas, bm, color);
      }
    }
    break;
  case 3:
    if (mask2 == NULL) {
      Stack_Draw_Voxel_Mc(canvas, mask, 4, color.r, color.g, color.b);
    } else {
      color_t *arrayc = (color_t *) canvas->array;
      int nvoxel = Stack_Voxel_Number(canvas);
      int i;
      Struct_Element *se = Make_Ball_Se(2);
      Stack *mask3 = Stack_Dilate(mask, NULL, se);
      Free_Stack(mask);
      mask = Stack_Dilate(mask2, NULL, se);
      Free_Stack(mask2);
      mask2 = mask;
      mask = mask3;

      for (i = 0; i < nvoxel; i++) {
	if ((mask->array[i] == 1) || (mask2->array[i] == 1)) {
	  arrayc[i][0] = 0;
	  arrayc[i][1] = 0;
	  arrayc[i][2] = 0;
	  
	  if (mask->array[i] == 1) {
	    arrayc[i][0] = 255;
	  } 

	  if (mask2->array[i] == 1){
	    arrayc[i][1] = 255;
	  }
	}
      }
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
    return 1;
  }

  Write_Stack_U(Get_String_Arg("-o"), canvas, NULL);

  printf("%s is created.\n", Get_String_Arg("-o"));

  return 0;
}
