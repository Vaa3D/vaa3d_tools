/**@file teasar.c
 * @author Ting Zhao
 * @date 06-Jul-2012
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
#include "tz_sp_grow.h"
#include "tz_stack_utils.h"


int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  Stack *objstack = Read_Stack_U(Get_String_Arg("input"));
  Stack_Binarize(objstack);

  Swc_Tree *tree = New_Swc_Tree();
  tree->root = Make_Virtual_Swc_Tree_Node();

  Object_3d_List *objs = Stack_Find_Object(objstack, 1, 300);
  Object_3d_List *iterator = objs;
  while (iterator != NULL) {
    Object_3d *obj = iterator->data;
    Swc_Tree *subtree = Swc_Tree_From_Object_3d(obj);
    Swc_Tree_Node *regularRoot = Swc_Tree_Regular_Root(subtree);
    Swc_Tree_Node_Set_Parent(regularRoot, tree->root);
    if (subtree->root == regularRoot) {
      subtree->root = NULL;
    }

    Kill_Swc_Tree(subtree);

    iterator = iterator->next;
  }
  Kill_Object_3d_List(objs);

  Swc_Tree_Erase_Stack(tree, objstack, 1.1, 3.0);

  objs = Stack_Find_Object(objstack, 2, 100);
  iterator = objs;
  while (iterator != NULL) {
    Object_3d *obj = iterator->data;
    Swc_Tree *subtree = Swc_Tree_From_Object_3d(obj);
    Swc_Tree_Node *regularRoot = Swc_Tree_Regular_Root(subtree);
    Swc_Tree_Node_Set_Parent(regularRoot, tree->root);
    if (subtree->root == regularRoot) {
      subtree->root = NULL;
    }

    Kill_Swc_Tree(subtree);

    iterator = iterator->next;
  }
  Kill_Object_3d_List(objs);
  //Write_Stack("../data/test.tif", objstack);

#if 0
  Stack *tmpdist = Stack_Bwdist_L_U16P(objstack, NULL, 0);

#ifdef _DEBUG_2
  Write_Stack("../data/test.tif", tmpdist);
  exit(1);
#endif
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->wf = Stack_Voxel_Weight_I;
  int max_index;
  double max_value = Stack_Max(tmpdist, &max_index);

  Stack *mask = Make_Stack(GREY, Stack_Width(tmpdist), Stack_Height(tmpdist),
      Stack_Depth(tmpdist));
  Zero_Stack(mask);

  size_t nvoxel = Stack_Voxel_Number(objstack);
  size_t i;
  for (i = 0; i < nvoxel; i++) {
    if (objstack->array[i] == 0) {
      mask->array[i] = SP_GROW_BARRIER;
    }
  }
  mask->array[max_index] = SP_GROW_SOURCE;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  Stack_Sp_Grow(tmpdist, NULL, 0, NULL, 0, sgw);

  Swc_Tree *tree = New_Swc_Tree();
  int x, y, z;
  int area = Stack_Width(objstack) * Stack_Height(objstack);
  STACK_UTIL_COORD(max_index, Stack_Width(objstack), area, x, y, z);
  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  Swc_Tree_Node_Data(tn)->x = x;
  Swc_Tree_Node_Data(tn)->y = y;
  Swc_Tree_Node_Data(tn)->z = z;
  Swc_Tree_Node_Data(tn)->d = sqrt(max_value);

  tree->root = tn;

  int max_count;
  int cur_index = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
  Int_Arraylist *path = New_Int_Arraylist();
  Swc_Tree_Node *prev_tn = NULL;

#define SWC_TREE_FROM_OBJECT_3D_ADD_NODE \
    STACK_UTIL_COORD(cur_index, Stack_Width(objstack), area, x, y, z); \
    tn = New_Swc_Tree_Node(); \
    Swc_Tree_Node_Data(tn)->x = x; \
    Swc_Tree_Node_Data(tn)->y = y; \
    Swc_Tree_Node_Data(tn)->z = z; \
    Swc_Tree_Node_Data(tn)->d = sqrt(Stack_Array_Value(tmpdist, cur_index)); \
    if (prev_tn != NULL) { \
      double d = Swc_Tree_Node_Dist(prev_tn, tn); \
      double r1 = Swc_Tree_Node_Radius(tn); \
      double r2 = Swc_Tree_Node_Radius(prev_tn); \
      if ((d > r2) || (d > r1)) { \
	Swc_Tree_Node_Set_Parent(prev_tn, tn); \
	prev_tn = tn; \
      } \
    } else { \
      prev_tn = tn; \
    } \
 \
    int tmp_index = cur_index; \
    cur_index = sgw->path[cur_index]; \
    sgw->path[tmp_index] = -1;

  while (cur_index >= 0) {
    Int_Arraylist_Insert(path, 0, cur_index);
    if (cur_index == max_index) {
      break;
    }
    SWC_TREE_FROM_OBJECT_3D_ADD_NODE
  }

  int neighbor[26];
  int n_nbr = 18;
  Stack_Neighbor_Offset(n_nbr, objstack->width, objstack->height, neighbor);
  int is_in_bound[26];
  for (i = 1; i < path->length; i++) {
    int center = path->array[i];

    int n_in_bound = Stack_Neighbor_Bound_Test_I(n_nbr, objstack->width, 
	objstack->height, objstack->depth, center, is_in_bound);
    int j;
    if (n_in_bound == n_nbr) {
      for (j = 0; j < n_nbr; j++) {
	sgw->path[center + neighbor[j]] = -1;
      }
    } else {
      for (j = 0; j < n_nbr; j++) {
	if (is_in_bound[j]) {
	  sgw->path[center + neighbor[j]] = -1;
	}
      }
    }
  }

  Kill_Int_Arraylist(path);

  if (prev_tn != NULL) {
    Swc_Tree_Node_Set_Parent(prev_tn, tree->root);
  }

  cur_index = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
  //cur_index = sgw->path[idx];
  prev_tn = NULL;
  while (cur_index >= 0) {
    if (sgw->path[cur_index] == -1) {
      break;
    }

    SWC_TREE_FROM_OBJECT_3D_ADD_NODE
  }

  if (prev_tn != NULL) {
    Swc_Tree_Node_Set_Parent(prev_tn, tree->root);
  }

  Kill_Stack(mask);
  Kill_Sp_Grow_Workspace(sgw);
  Kill_Stack(tmpdist);
  Kill_Stack(objstack);

#endif
  Swc_Tree_Resort_Id(tree);
  Print_Swc_Tree(tree);

#ifdef _DEBUG_2
  Write_Stack("../data/test.tif", tmpdist);
  exit(1);
#endif

  Write_Swc_Tree(Get_String_Arg("-o"), tree);
  
  return 0;
}
