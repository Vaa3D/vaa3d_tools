/**@file mask_puncta.c
 * @brief >> using traced result to mask puncta
 * @author Linqing Feng
 * @date 21-July-2011
 */

#include <stdio.h>
#include "tz_utilities.h"
#include "tz_swc_tree.h"
#include "tz_locseg_chain_com.h"
#include "tz_locseg_chain.h"
#include "image_lib.h"
#include "tz_stack_stat.h"
#include "tz_swc_cell.h"
#include "tz_stack_neighborhood.h"
#include "tz_geo3d_ball.h"

int main(int argc, char *argv[])
{
  char puncta_swc_path[] = "/home/feng/otherSource/hand2.swc";
  char puncta_meanshifted_swc_path[] = "/home/feng/otherSource/hand_meanshifted.swc";
  char trace_result_path[] = "/home/feng/otherSource/015.trace/traced/";
  char masked_swc_path[] = "/home/feng/otherSource/masked_hand.swc";
  char masked_meanshifted_swc_path[] = "/home/feng/otherSource/masked_hand_meanshifted.swc";
  char puncta_stack_path[] = "/home/feng/otherSource/C1-slice09_L5_Sum.tif";

  int n;
  Locseg_Chain* chains = Dir_Locseg_Chain_N(trace_result_path, "^chain[[:digit:]]*\\.tb", &n, NULL);

  printf("found %d tb file\n", n);
  int i=0;
  Locseg_Label_Workspace* ws = New_Locseg_Label_Workspace();
  Stack *mask = Make_Stack(GREY, 1024, 1024, 128);
  Zero_Stack(mask);
  
  double z_scale = 1;
  double pixelperumxy = 9.66;
  double pixelperumz = 2;
  double maskextendbyum = 2.5;
  double maskextendbypixel = maskextendbyum * pixelperumxy;
  for (i=0; i<n; i++) {
    Default_Locseg_Label_Workspace(ws);
    ws->option = 1;
    ws->sdiff = maskextendbypixel;
    ws->value = 255;
    Locseg_Chain_Label_W(&(chains[i]), mask, z_scale, 0, Locseg_Chain_Length(&(chains[i])), ws);
  }

  Write_Stack("/home/feng/otherSource/maskstack.tif", mask);


  Swc_Node *punctas = Read_Swc_File(puncta_swc_path, &n);
  printf("found %d puncta\n", n);

  FILE *fp = fopen(masked_swc_path, "w");
  for (i=0; i<n; i++) {
//  if (*(STACK_PIXEL_8(mask, iround(punctas[i].x), iround(punctas[i].y), iround(punctas[i].z), 0)) == 255) {
//    Swc_Node_Fprint(fp, &(punctas[i]));
//  }
    if (Stack_Neighbor_Mean(mask, 26, iround(punctas[i].x), iround(punctas[i].y), iround(punctas[i].z)) > 0) {
      Swc_Node_Fprint(fp, &(punctas[i]));
    }
  }
  fclose(fp);

  fp = fopen(puncta_meanshifted_swc_path, "w");
  Stack *puncta_stack = Read_Stack(puncta_stack_path);
  for (i=0; i<n; i++) {
    Geo3d_Ball *gb = New_Geo3d_Ball();
    gb->center[0] = punctas[i].x;
    gb->center[1] = punctas[i].y;
    gb->center[2] = punctas[i].z;
    gb->r = 3;
    Geo3d_Ball_Mean_Shift(gb, puncta_stack, 1, 0.5);
    punctas[i].x = gb->center[0];
    punctas[i].y = gb->center[1];
    punctas[i].z = gb->center[2];
    Swc_Node_Fprint(fp, punctas+i);

    Delete_Geo3d_Ball(gb);
  }
  fclose(fp);
  Kill_Stack(puncta_stack);


  fp = fopen(masked_meanshifted_swc_path, "w");
  for (i=0; i<n; i++) {
    if (Stack_Neighbor_Mean(mask, 26, iround(punctas[i].x), iround(punctas[i].y), iround(punctas[i].z)) > 0) {
      Swc_Node_Fprint(fp, &(punctas[i]));
    }
  }
  fclose(fp);

  return 0;
}

