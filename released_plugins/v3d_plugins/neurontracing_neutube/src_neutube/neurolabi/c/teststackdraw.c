/* testsampling.c
 *
 * 29-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <ctype.h>
#include <image_lib.h>
#include "tz_stack_lib.h"
#include "tz_stack_draw.h"
#include "tz_stack_attribute.h"
#include "tz_dmatrix.h"
#include "tz_stack_io.h"
#include "tz_interface.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_stack_math.h"
#include "tz_stack.h"
#include "tz_error.h"
#include "tz_color.h"
#include "tz_geo3d_ball.h"
#include "tz_swc_tree.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
#if 0
  double x = 0.5;
  double y = 0.5;
  double z = 0.5;
  double v = 1.0;
 		
  char msg[100];

  sprintf(msg, "Please input y (default: %g):", x);
  Input_Dialog(msg, "%lf", &x);

  sprintf(msg, "Please input y (default: %g):", y);
  Input_Dialog(msg, "%lf", &y);

  sprintf(msg, "Please input z (default: %g):", z);
  Input_Dialog(msg, "%lf", &z);

  sprintf(msg, "Please input v (default: %g):", v);
  Input_Dialog(msg, "%lf", &v);

  Stack *stack = Make_Stack(GREY, 3, 3, 3);
  int i;
  for (i = 0; i < Stack_Voxel_Number(stack); i++) {
    stack->array[i] = 0;
  }

  Stack_Draw_Point(stack, x, y, z, v, 0);

  Print_Stack(stack);

  Kill_Stack(stack);
#endif


#if 0
  Stack *stack = Read_Stack("../data/fly_neuron2/seeds.tif");
  Stack *blob = Read_Stack("../data/fly_neuron2/blobmask.tif");
  Stack_Not(blob, blob);
  Stack_And(stack, blob, stack);

  Stack *signal = Read_Stack("../data/fly_neuron2.tif");
  Stack *canvas = Translate_Stack(signal, COLOR, 0);
  Rgb_Color color;
  color.r = 255;
  color.g = 0;
  color.b = 0;
  Stack_Draw_Voxel_Mc(canvas, stack, 4, color);
  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *signal = Read_Stack("../data/fly_neuron2.tif");
  Stack *canvas = Translate_Stack(signal, COLOR, 0);
  Stack *blob = Read_Stack("../data/fly_neuron2/blobmask.tif");
  Stack_Blend_Mc(canvas, blob, 0.0);
  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *signal = Read_Stack("../data/mouse_neuron3_org.tif");
  Stack_Bc_Autoadjust(signal);
  Stack *canvas = Translate_Stack(signal, COLOR, 0);

  Geo3d_Scalar_Field *field = 
    Read_Geo3d_Scalar_Field("../data/mouse_neuron3_org/seeds");

  int i;
  for (i = 0; i < field->size; i++) {
    Stack_Draw_Voxel_C(canvas, field->points[i][0], field->points[i][1],
		       field->points[i][2], 4, 255, 0, 0);
  }

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  Stack *canvas = Read_Stack("../data/mouse_neuron3_org/traced.tif");

  Geo3d_Scalar_Field *field = 
    Read_Geo3d_Scalar_Field("../data/mouse_neuron3_org/seeds");

  int i;
  for (i = 0; i < field->size; i++) {
    if (Get_Stack_Pixel(canvas, field->points[i][0], field->points[i][1],
			field->points[i][2], 2) != 0) {
      Stack_Draw_Voxel_C(canvas, field->points[i][0], field->points[i][1],
			 field->points[i][2], 4, 0, 0, 255);
    }
  }

  Write_Stack("../data/test.tif", canvas);

  Kill_Stack(canvas);
  Kill_Geo3d_Scalar_Field(field);
#endif

#if 0
  //uint8_t color_map[] = {0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF};
  //int ncolor = sizeof(color_map) / 3;

  Stack *stack = Read_Stack("../data/benchmark/rice_bw.tif");
  Stack *label = Read_Stack("../data/benchmark/rice_label.tif");

  Stack *out = Stack_Blend_Label_Field(stack, label, 0.5, Jet_Colormap, 
      Jet_Color_Number, NULL);
  Write_Stack("../data/test.tif", out);
#endif

#if 0
  Stack *stack = Make_Stack(COLOR, 50, 50, 50);
  Zero_Stack(stack);
  Geo3d_Ball ball;
  ball.r = 10;
  ball.center[0] = 20;
  ball.center[1] = 20;
  ball.center[2] = 20;

  Geo3d_Ball_Label_Workspace ws;
  Default_Geo3d_Ball_Label_Workspace(&ws);
  ws.sdw.color.r = 255;
  ws.sdw.color.g = 0;
  ws.sdw.color.b = 255;
  Geo3d_Ball_Label_Stack(&ball, stack, &ws);

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = Make_Stack(COLOR, 50, 50, 50);
  Zero_Stack(stack);
  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  tn->node.d = 5;
  tn->node.x = 20;
  tn->node.y = 20;
  tn->node.z = 20;

  Swc_Tree_Node *tn2 = New_Swc_Tree_Node();
  tn2->node.d = 5;
  tn2->node.x = 30;
  tn2->node.y = 30;
  tn2->node.z = 30;

  Swc_Tree_Node_Set_Parent(tn, tn2);

  Swc_Tree_Node_Label_Workspace ws;
  Default_Swc_Tree_Node_Label_Workspace(&ws);
  ws.sdw.color.r = 255;
  ws.sdw.color.g = 0;
  ws.sdw.color.b = 255;
  Swc_Tree_Node_Label_Stack(tn, stack, &ws);

  Write_Stack("../data/test.tif", stack);
#endif

#if 1
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/mouse_single_org.swc");
  Stack *stack = Make_Stack(COLOR, 512, 512, 159);
  Zero_Stack(stack);

  Swc_Tree_Node_Label_Workspace ws;
  Default_Swc_Tree_Node_Label_Workspace(&ws);
  ws.sdw.color.r = 255;
  ws.sdw.color.g = 0;
  ws.sdw.color.b = 255;
  Swc_Tree_Label_Stack(tree, stack, &ws);

  Write_Stack("../data/test.tif", stack);
#endif

  return 0;
}
