/**@file assign_puncta.c
 * @brief >> use distance to decide which tree puncta belong to, also mask
 * @author Linqing Feng
 * @date 21-July-2011
 */

#include <stdio.h>
#include <string.h>
#include "utilities.h"
#include "tz_string.h"
#include "tz_swc_tree.h"
#include "tz_workspace.h"
#include "tz_stack_graph.h"
#include "tz_stack_threshold.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_int_histogram.h"
#include "tz_graph.h"
#include "tz_math.h"
#include "tz_local_neuroseg.h"
#include "tz_locseg_chain_com.h"
#include "tz_locseg_chain.h"
#include "image_lib.h"
#include "tz_stack_stat.h"
#include "tz_swc_cell.h"
#include "tz_stack_io.h"
#include "tz_stack_neighborhood.h"
#include "tz_geo3d_ball.h"
#include "tz_apo.h"





int intensity_weighted_nearest_node_index(double x, double y, double z, Swc_Tree_Node **refNodes,
                                          int nswc, char* stackfilename, double zscale, double susfactor, BOOL *isSuspicious, int channel)
{
  //first crop out the region
  printf("%f %f %f\n",x,y,z);
  int left = iround(x);
  int right = iround(x);
  int up = iround(y);
  int down = iround(y);
  int zup = iround(z);
  int zdown = iround(z);
  int i,j;
  for (i=0; i<nswc; i++) {
    if (refNodes[i] != NULL) {
      Print_Swc_Tree_Node(refNodes[i]);
      Print_Swc_Tree_Node(refNodes[i]->parent);
      left = MIN3(left, iround(refNodes[i]->node.x), iround(refNodes[i]->parent->node.x));
      right = MAX3(right, iround(refNodes[i]->node.x), iround(refNodes[i]->parent->node.x));
      up = MIN3(up, iround(refNodes[i]->node.y), iround(refNodes[i]->parent->node.y));
      down = MAX3(down, iround(refNodes[i]->node.y), iround(refNodes[i]->parent->node.y));
      zup = MIN3(zup, iround(refNodes[i]->node.z), iround(refNodes[i]->parent->node.z));
      zdown = MAX3(zdown, iround(refNodes[i]->node.z), iround(refNodes[i]->parent->node.z));
    }
  }
  Stack *stack = Read_Part_Raw_Stack(stackfilename, left, right, up, down, zup, zdown, channel);
  Print_Stack(stack);
  //create graph
  Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
  sgw->conn = 26;
  sgw->wf = Stack_Voxel_Weight_S;
  int *hist = Stack_Hist(stack);
  double c1, c2;
  int thre = Hist_Rcthre_R(hist, Int_Histogram_Min(hist), 
                           Int_Histogram_Max(hist), &c1, &c2);
  free(hist);
  sgw->argv[3] = thre;
  sgw->argv[4] = c2 - c1;
  if (sgw->argv[4] < 1.0) {
    sgw->argv[4] = 1.0;
  }
  sgw->argv[4] /= 9.2;
  sgw->gw = New_Graph_Workspace();
  
  Stack_Graph_Workspace_Set_Range(sgw, 0, right-left, 0, down-up, 0, zdown-zup);
  int start = Stack_Util_Offset(iround(x)-left, iround(y)-up, iround(z)-zup, right-left+1, down-up+1, zdown-zup+1);
  Graph *graph = Stack_Graph_W(stack, sgw);
  Graph_Shortest_Path(graph, start, sgw->gw);
  Kill_Graph(graph);
  Kill_Stack(stack);

  Stack *maskstack = Make_Stack(1, right-left+1, down-up+1, zdown-zup+1);
  Zero_Stack(maskstack);
//Locseg_Label_Workspace* ws = New_Locseg_Label_Workspace();
//for (i=0; i<nswc; i++) {
//  Reset_Locseg_Label_Workspace(ws);
//  ws->option = 1;
//  ws->value = i+1;
//  Local_Neuroseg *ns = New_Local_Neuroseg();
//  Swc_Tree_Node_To_Locseg(refNodes[i], ns);
//  ns->pos[0] -= left;
//  ns->pos[1] -= up;
//  ns->pos[2] -= zup;
//  Local_Neuroseg_Label_W(ns, maskstack, 1, ws);
//  Kill_Local_Neuroseg(ns);
//}
//Kill_Locseg_Label_Workspace(ws);
  for (i=0; i<nswc; i++) {
    if (refNodes[i] != NULL) {
      Swc_Node *node = Copy_Swc_Node(Swc_Tree_Node_Data(refNodes[i]));
      Swc_Node *pnode = Copy_Swc_Node(Swc_Tree_Node_Data(refNodes[i]->parent));
      node->x -= left;
      node->y -= up;
      node->z -= zup;
      pnode->x -= left;
      pnode->y -= up;
      pnode->z -= zup;
      Label_Swc_Seg_In_Stack(maskstack, node, pnode, zscale, i+1);
      Kill_Swc_Node(node);
      Kill_Swc_Node(pnode);
      Set_Stack_Pixel(maskstack, iround(refNodes[i]->node.x)-left, iround(refNodes[i]->node.y)-up,
                  iround(refNodes[i]->node.z)-zup, 0, (i+1)*10);
      Set_Stack_Pixel(maskstack, iround(refNodes[i]->parent->node.x)-left, iround(refNodes[i]->parent->node.y)-up,
                  iround(refNodes[i]->parent->node.z)-zup, 0, (i+1)*10+1);
    }
  }

  Set_Stack_Pixel(maskstack, iround(x)-left, iround(y)-up, iround(z)-zup, 0, 255);
  Print_Stack(maskstack);
//if (maskstack->width == 129 && maskstack->height == 23 && left == 1683 && right  == 1811) {
//  Stack *maskstack1 = Make_Stack(1, right-left+1+20, down-up+1+20, zdown-zup+1+20);
//  Zero_Stack(maskstack1);
//  for (i=0; i<nswc; i++) {
//  if (refNodes[i] != NULL) {
//    Swc_Node *node = Copy_Swc_Node(Swc_Tree_Node_Data(refNodes[i]));
//    Swc_Node *pnode = Copy_Swc_Node(Swc_Tree_Node_Data(refNodes[i]->parent));
//    node->x -= left-10;
//    node->y -= up-10;
//    node->z -= zup-10;
//    pnode->x -= left-10;
//    pnode->y -= up-10;
//    pnode->z -= zup-10;
//    label_swc_in_stack(maskstack1, node, pnode, zscale, i+1);
//    Kill_Swc_Node(node);
//    Kill_Swc_Node(pnode);
//    Set_Stack_Pixel(maskstack1, iround(refNodes[i]->node.x)-left+10, iround(refNodes[i]->node.y)-up+10,
//                iround(refNodes[i]->node.z)-zup+10, 0, (i+1)*10);
//    Set_Stack_Pixel(maskstack1, iround(refNodes[i]->parent->node.x)-left+10, iround(refNodes[i]->parent->node.y)-up+10,
//                iround(refNodes[i]->parent->node.z)-zup+10, 0, (i+1)*10+1);
//  }
//  }
//  Write_Stack("/host/shape.tif", maskstack1);
//}
  
  int min_index = 0;
  double min_dist = 100000;
  double second_min_dist = min_dist;
  int second_min_index = 0;
  for (j=0; j<nswc; j++) {
    double min_dist_j = 100000;
    for (i=0; i<Stack_Voxel_Number(maskstack); i++) {
      if (Stack_Array_Value(maskstack, i) == j+1 && sgw->gw->dlist[i] < min_dist_j) {
        min_dist_j = sgw->gw->dlist[i];
      }
    }
    if (min_dist_j < min_dist) {
      second_min_dist = min_dist;
      min_dist = min_dist_j;
      second_min_index = min_index;
      min_index = j+1;
    } else if (min_dist_j < second_min_dist) {
      second_min_dist = min_dist_j;
      second_min_index = j+1;
    }
  }
  Kill_Stack(maskstack);
  Kill_Stack_Graph_Workspace(sgw);
  printf("weighted dist to tree %d:%f\nweighted dist to tree %d:%f\n", min_index, min_dist, second_min_index, second_min_dist);
  if (second_min_dist < susfactor*min_dist) {
    *isSuspicious = TRUE;
  } else {
    *isSuspicious = FALSE;
  }
  min_index--;
  return min_index;
}

int main(int argc, char *argv[])
{
  // neuronchannel: channel should start from 1
  static char *Spec[] = {"<input:string> ...", "[-apo <string>] [-nomask]", "[-neuronchannel <int(2)>]",
    "[-maskextendbyum <double(2.5)>]", "[-suspiciousfactor <double(1.5)>]", "[-stack <string>]", 
    "[-pixelperumxy <double(9.66)>]", "[-pixelperumz <double(2)>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int nswc = Get_Repeat_Count("input");
  char **outpath = (char**) malloc(sizeof(char*) * nswc);
  FILE **fps = (FILE**) malloc(sizeof(FILE*) * nswc);
  Swc_Tree **trees = (Swc_Tree**) malloc(sizeof(Swc_Tree*) * nswc);
  Swc_Tree_Node **refNodes = (Swc_Tree_Node**) malloc(sizeof(Swc_Tree_Node**) * nswc);
  double *dists = (double*) malloc(sizeof(double) * nswc);
  int i;
  for (i=0; i<nswc; i++) {
    outpath[i] = (char *) malloc(sizeof(char) * 500);
    //sprintf(outpath[i], "%s_puncta.swc", Get_String_Arg("-swc", i));
    sprintf(outpath[i], "%s_puncta.apo", Get_String_Arg("input", i));
    fps[i] = Guarded_Fopen(outpath[i], "w", "open output puncta files");
    trees[i] = Read_Swc_Tree(Get_String_Arg("input", i));
    printf("swc file: %s\n", Get_String_Arg("input", i));
  }

  int npuncta;
  //Swc_Node *punctas = Read_Swc_File(Get_String_Arg("input"), &npuncta);
  Apo_Node *punctas = Read_Apo_File(Get_String_Arg("-apo"), &npuncta);
  printf("found %d puncta\n", npuncta);

  char suspoutpath[500];
  sprintf(suspoutpath, "%s_suspicious.marker", Get_String_Arg("-apo"));
  FILE * sfp = Guarded_Fopen(suspoutpath, "w", "open suspicious puncta output marker file");
  char aposuspoutpath[500];
  sprintf(aposuspoutpath, "%s_suspicious.apo", Get_String_Arg("-apo"));
  FILE * aposfp = Guarded_Fopen(aposuspoutpath, "w", "open suspicious puncta output apo file");

  double pixelperumxy = Get_Double_Arg("-pixelperumxy");
  double pixelperumz = Get_Double_Arg("-pixelperumz"); 
  double maskextendbyum = Get_Double_Arg("-maskextendbyum");
  int channel = Get_Int_Arg("-neuronchannel");
  double zscale = pixelperumxy/pixelperumz;

  for (i=0; i<npuncta; i++) {
    printf("puncta: %d\n", i+1);
    int j;
    int min_dist_tree_index = -1;
    int num_of_tree_in_range = 0;
    double min_dist = 10000;
    for (j=0; j<nswc; j++) {
      dists[j] = 10000;
      refNodes[j] = NULL;
    }

    for (j=0; j<nswc; j++) {
      double dist;
      if (Is_Arg_Matched("-nomask")) {
        dist = Puncta_Tree_Distance(punctas[i].x, punctas[i].y, punctas[i].z, punctas[i].r, trees[j],
                                   pixelperumxy, pixelperumz, FALSE, 0, refNodes+j);
      } else {     
        dist = Puncta_Tree_Distance(punctas[i].x, punctas[i].y, punctas[i].z, punctas[i].r, trees[j],
                                   pixelperumxy, pixelperumz, TRUE, maskextendbyum, refNodes+j);
      }
      if (dist < dists[j]) {
        printf("dist to tree %d: %f\n", j+1, dist);
        dists[j] = dist;
        num_of_tree_in_range++;
      }
      if (dist < min_dist) {
        min_dist = dist;
        min_dist_tree_index = j;
      }
    }

    if (num_of_tree_in_range == 1 || min_dist < 0) {  //inside one tree's mask area
      Apo_Node_Fprint(fps[min_dist_tree_index], &(punctas[i]));
    } else if (num_of_tree_in_range > 1) {  //puncta inside many tree's mask area 
      BOOL isSuspicious;
      min_dist_tree_index = intensity_weighted_nearest_node_index(punctas[i].x, punctas[i].y, punctas[i].z,
                                                                  refNodes, nswc, Get_String_Arg("-stack"),
                                                                  zscale, Get_Double_Arg("-suspiciousfactor"), &isSuspicious, channel);
      if (isSuspicious) {
        Apo_Marker_Fprint(sfp, &(punctas[i]));
        Apo_Node_Fprint(aposfp, &(punctas[i]));
      } else {
        Apo_Node_Fprint(fps[min_dist_tree_index], &(punctas[i]));
      }
    }

  }

  fclose(aposfp);
  fclose(sfp);

  for (i=0; i<nswc; i++) {
    fclose(fps[i]);
  }
  for (i=0; i<nswc; i++) {
    Kill_Swc_Tree(trees[i]);
    free(outpath[i]);
  }
  free(outpath);
  free(trees);
  free(fps);
  free(refNodes);
  free(dists);
  return 0;
}

