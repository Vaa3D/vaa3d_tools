/**@file soma.c
 * @author Ting Zhao
 * @date 18-Jul-2010
 */

#include "tz_utilities.h"
#include "tz_stack_watershed.h"
#include "tz_stack_threshold.h"
#include "tz_stack.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_attribute.h"
#include "tz_stack_lib.h"
#include "tz_object_3d_linked_list.h"
#include "tz_objdetect.h"
#include "tz_stack_draw.h"
#include "tz_stack_stat.h"
#include "tz_stack_io.h"
#include "tz_stack_graph.h"
#include "tz_swc_tree.h"
#include "tz_math.h"
#include "tz_image_io.h"

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "0.11") == 1) {
    return 0;
  }

  static char *Spec[] = {"<input:string> -o <string> [-channel <int(0)>] [-xys <double>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  //Stack *stack = Read_Stack_U(Get_String_Arg("input"));
  Stack *stack = Read_Sc_Stack(Get_String_Arg("input"), Get_Int_Arg("-channel"));

  int thre = Stack_Find_Threshold_A(stack, THRESHOLD_LOCMAX_TRIANGLE);
  Filter_3d *filter = Gaussian_Filter_3d(1.0, 1.0, 0.5);
  Stack *foreground = Filter_Stack(stack, filter);
  stack = Copy_Stack(foreground);
  Stack_Threshold_Binarize(foreground, thre);
  Stack *out2 = Stack_Bwdist_L_U16P(foreground, NULL, 0);

  /*
     foreground = Stack_Locmax_Region(out2, 26);
     */
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
      Stack_Depth(stack));
  Zero_Stack(ws->mask);
  size_t nvoxel = Stack_Voxel_Number(stack);
  size_t offset;
  uint16_t *dist_array = (uint16_t*) out2->array;
  double res = 0.208;
  if (Is_Arg_Matched("-xys")) {
    res = Get_Double_Arg("-xys");
  }
  int dist_thre = iround(4.5 / res);
  dist_thre *= dist_thre;
  for (offset = 0; offset < nvoxel; offset++) {
    if (/*(foreground->array[offset] == 1) && */(dist_array[offset] > dist_thre/*1700*/)) {
      ws->mask->array[offset] = 1;
    }
  }

  //Objlabel_Workspace *ow = New_Objlabel_Workspace();
  //Stack_Label_Large_Objects_W(ws->mask, 1, 2, 10, ow);
  //Stack_Label_Objects_N(ws->mask, NULL, 1, 2, 26);
  Object_3d_List *objs = Stack_Find_Object(ws->mask, 1, 100);
  Zero_Stack(ws->mask);
  Stack_Draw_Objects_Bw(ws->mask, objs, -255);
  printf("%g\n", Stack_Max(ws->mask, NULL));

  ws->min_level = thre;
  ws->start_level = 65535;

  Stack *out3 = Stack_Watershed(stack, ws);
  dist_thre = iround(3.5 / res);
  dist_thre *= dist_thre;

  for (offset = 0; offset < nvoxel; offset++) {
    if (dist_array[offset] < dist_thre) {
      out3->array[offset] = 0;
    }
  }

  //int nregion = Stack_Max(out3, NULL);

  Kill_Stack(out2);
  /*
     Stack *filtered = Copy_Stack(stack);
     Kill_Stack(stack);
     */

  ws->conn = 8;
  stack = Stack_Region_Border_Shrink(out3, ws);
  out2 = Stack_Region_Expand(stack, 4, iround(6.24/res), NULL);

  /*
  for (offset = 0; offset < nvoxel; offset++) {
    if (foreground->array[offset] == 0) {
      out2->array[offset] = 0;
    }
  }
*/
  Kill_Stack(stack);

  if (fhasext(Get_String_Arg("-o"), "tif")) {
    Write_Stack(Get_String_Arg("-o"), out2);
  } else {
    int max_label = Stack_Max(out2, NULL);
    int i;
    char filepath[500];
    int soma_id = 0;
    for (i = max_label; i >= 1; i--) {
      Object_3d_List *objs = Stack_Find_Object(out2, i, 300);
      Object_3d_List *iterator = objs;
      while (iterator != NULL) {
        printf("Generating soma %d.\n", soma_id);
        Object_3d *obj = iterator->data;
        Swc_Tree *tree = Swc_Tree_From_Object_3d(obj);

        Swc_Tree_Node *tn = tree->root;
        while (tn->first_child != NULL) {
          tn = tn->first_child;
        }

        while (Swc_Tree_Node_Radius(tn) < 5.0) {
          Swc_Tree_Node *tmp_tn = tn->parent;
          Swc_Tree_Node_Detach_Parent(tn);
          Kill_Swc_Tree_Node(tn);
          tn = tmp_tn;
        }

        tn = tree->root->first_child->next_sibling;
        while (tn->first_child != NULL) {
          tn = tn->first_child;
        }

        while (Swc_Tree_Node_Radius(tn) < 5.0) {
          Swc_Tree_Node *tmp_tn = tn->parent;
          Swc_Tree_Node_Detach_Parent(tn);
          Kill_Swc_Tree_Node(tn);
          tn = tmp_tn;
        }

        sprintf(filepath, "%s/soma%d.swc", Get_String_Arg("-o"), soma_id++);
        Write_Swc_Tree(filepath, tree);
        Kill_Swc_Tree(tree);
        /*
           Stack *objstack = Object_To_Stack_Bw(obj, 1, NULL);
           Print_Stack_Info(objstack);
           Stack *tmpdist = Stack_Bwdist_L_U16P(objstack, NULL, 0);

           Stack_Graph_Workspace *sgw = New_Stack_Graph_Workspace();
           sgw->wf = Stack_Voxel_Weight_A;
           sgw->signal_mask = objstack;
           Graph_Workspace *gw = New_Graph_Workspace();
           Graph *graph = Stack_Graph_W(tmpdist, sgw);
           Graph_To_Mst2(graph, gw);
           Graph_Prune(graph, 3, gw);
           Graph_Mwt(graph, gw);

           int nvoxel = Stack_Voxel_Number(objstack);
           int v;
           for (v = 0; v < nvoxel; v++) {
           if (objstack->array[v] == 1) {
           objstack->array[v] = 128;
           }
           }
           for (v = 0; v < GRAPH_EDGE_NUMBER(graph); v++) {
           objstack->array[GRAPH_EDGE_NODE(graph, v, 0)] = 255;
           objstack->array[GRAPH_EDGE_NODE(graph, v, 1)] = 255;
           }
           Write_Stack("../data/test.tif", objstack);

           Kill_Graph_Workspace(gw);
           sgw->signal_mask = NULL;
           Kill_Stack_Graph_Workspace(sgw);
           Kill_Stack(objstack);
           Kill_Stack(tmpdist);
           */
        iterator = iterator->next;
      }
      Kill_Object_3d_List(objs);
    }
  }

  return 0;
}
