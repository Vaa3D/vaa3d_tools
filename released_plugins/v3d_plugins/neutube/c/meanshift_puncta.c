/**@file meanshift_puncta.c
 * @brief >> meanshift each puncta in swc file
 * @author Linqing Feng
 * @date 21-July-2011
 */


#include "tz_utilities.h"
#include "image_lib.h"
#include "tz_geo3d_ball.h"
#include "tz_swc_cell.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> [-o <string>] [-stack <string>] [-scaleradius <double(1.1)>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);


  int n,i;
  Swc_Node *punctas = Read_Swc_File(Get_String_Arg("input"), &n);
  FILE *fp = GUARDED_FOPEN(Get_String_Arg("-o"), "w");
  Stack *puncta_stack = Read_Stack(Get_String_Arg("-stack"));
  for (i=0; i<n; i++) {
    Geo3d_Ball *gb = New_Geo3d_Ball();
    gb->center[0] = punctas[i].x;
    gb->center[1] = punctas[i].y;
    gb->center[2] = punctas[i].z;
    gb->r = punctas[i].d * Get_Double_Arg("-scaleradius");
    Geo3d_Ball_Mean_Shift(gb, puncta_stack, 1, 0.5);
    punctas[i].x = gb->center[0];
    punctas[i].y = gb->center[1];
    punctas[i].z = gb->center[2];
    Swc_Node_Fprint(fp, punctas+i);

    Delete_Geo3d_Ball(gb);
  }
  fclose(fp);
  Kill_Stack(puncta_stack);
}
