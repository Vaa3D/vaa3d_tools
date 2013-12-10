/* @file tz_swc_tree.c
 *
 * @author Ting Zhao
 * @date 16-Jul-2009
 */

#include <time.h>
#ifndef _MSC_VER
#include <unistd.h>
#else
#include <process.h>
#define getpid _getpid
#endif
#include <utilities.h>
#include <string.h>
#include "tz_error.h"
#include "tz_swc_tree.h"
#include "tz_swc.h"
#include "tz_geo3d_utils.h"
#include "tz_unipointer_linked_list.h"
#include "tz_u8array.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_math.h"
#include "tz_xml_utils.h"
#include "tz_random.h"
#include "tz_sp_grow.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_stat.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_stack_neighborhood.h"
#include "tz_string.h"

#include "private/tzp_swc_tree.c"

Swc_Tree_Node* New_Swc_Tree_Node()
{
  Swc_Tree_Node *node = 
    (Swc_Tree_Node*) Guarded_Malloc(sizeof(Swc_Tree_Node),
				    "New_Swc_Tree_Node");

  Default_Swc_Tree_Node(node);

  return node;
}

void Default_Swc_Tree_Node(Swc_Tree_Node *node)
{
  if (node != NULL) {
    Default_Swc_Node(&(node->node));
    node->parent = NULL;
    node->first_child = NULL;
    node->next_sibling = NULL;
    node->next = NULL;
    node->weight = 0.0;
    node->feature = 0.0;
    node->index = -1;
    node->flag = 0;
  }
}

void Delete_Swc_Tree_Node(Swc_Tree_Node *tn)
{
  free(tn);
}

void Clean_Swc_Tree_Node(Swc_Tree_Node *tn)
{
  UNUSED_PARAMETER(tn);
}

Swc_Tree_Node* Make_Swc_Tree_Node(const Swc_Node *node)
{
  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  tn->node = *node;
  
  return tn;
}

Swc_Tree_Node* Make_Virtual_Swc_Tree_Node()
{
  Swc_Tree_Node *node = New_Swc_Tree_Node();
  Swc_Tree_Node_Data(node)->id = -1;
  Swc_Tree_Node_Data(node)->parent_id = -2;

  return node;
}

void Kill_Swc_Tree_Node(Swc_Tree_Node *tn)
{
  Clean_Swc_Tree_Node(tn);
  Delete_Swc_Tree_Node(tn);
}

void Print_Swc_Tree_Node(const Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    printf("Null swc tree node\n");
  } else {
    if (tn->node.id < 0) {
      printf("virtual ");
    }
    Print_Swc_Node(Swc_Tree_Node_Const_Data(tn));
    if (tn->weight > 0.0) {
      printf("weight: %g\n", tn->weight);
    }
    BOOL id_matched = TRUE;
    if (Swc_Tree_Node_Parent_Id(tn) >= 0) {
      if (tn->parent == NULL) {
	id_matched = FALSE;
      } else {
	if (Swc_Tree_Node_Parent_Id(tn) != Swc_Tree_Node_Id(tn->parent)) {
	  id_matched = FALSE;
	} else {
	  if (tn->parent->first_child == NULL) {
	    id_matched = FALSE;
	  } else {
	    Swc_Tree_Node *sibling = tn->parent->first_child;
	    while (sibling != NULL) {
	      if (sibling == tn) {
		break;
	      } else {
		sibling = sibling->next_sibling;
	      }
	    }
	    if (sibling == NULL) {
	      id_matched = FALSE;
	    }
	  }
	}
      }
    }
    if (id_matched == FALSE) {
      printf("ID unmatched.\n");
    }
  }
}

void Swc_Tree_Node_Copy_Property(const Swc_Tree_Node *src, Swc_Tree_Node *dst)
{
  TZ_ASSERT(dst, "NULL pointer");
  dst->node = src->node;
  dst->weight = src->weight;
  dst->feature = src->feature;
}

void Swc_Tree_Node_To_Virtual(Swc_Tree_Node *tn)
{
  Swc_Tree_Node_Data(tn)->id = -1;
  Swc_Tree_Node_Data(tn)->parent_id = -2;
}

void Swc_Tree_Node_Kill_Subtree(Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    Swc_Tree_Node_Detach_Parent(tn);
    Swc_Tree *tree = New_Swc_Tree();
    tree->root = tn;    
    Kill_Swc_Tree(tree);
  }
}

Swc_Node *Swc_Tree_Node_Data(Swc_Tree_Node *tn)
{
  return &(tn->node);
}

const Swc_Node *Swc_Tree_Node_Const_Data(const Swc_Tree_Node *tn)
{
  return &(tn->node);
}

int Swc_Tree_Node_Id(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->id;
}

int Swc_Tree_Node_Parent_Id(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->parent_id;
}

void Swc_Tree_Node_Set_Id(Swc_Tree_Node *tn, int id)
{
  Swc_Tree_Node_Data(tn)->id = id;
}

void Swc_Tree_Node_Set_Parent_Id(Swc_Tree_Node *tn, int id)
{
  Swc_Tree_Node_Data(tn)->parent_id = id;
}

int Swc_Tree_Node_Label(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->label;
}

void Swc_Tree_Node_Set_Label_D(Swc_Tree_Node *tn, int label, int overwrite)
{
  Swc_Tree_Node *next = tn;
  while (next != NULL) {
    if (((overwrite >= 0) && (Swc_Tree_Node_Label(next) == overwrite)) || 
	((overwrite < 0) && (Swc_Tree_Node_Label(next) != -overwrite))) {
      Swc_Tree_Node_Set_Label(next, label);
    }
    next = Swc_Tree_Node_Next(next);
    if (next != NULL) {
      if (next == tn->next_sibling) {
	break;
      }
    }
  }
}

void Swc_Tree_Node_Set_Label(Swc_Tree_Node *tn, int label)
{
  Swc_Tree_Node_Data(tn)->label = label;
}

double Swc_Tree_Node_Radius(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->d;
}

double Swc_Tree_Node_Weight(const Swc_Tree_Node *tn)
{
  return tn->weight;
}

double Swc_Tree_Node_Feature(const Swc_Tree_Node *tn)
{
  return tn->feature;
}

void Swc_Tree_Node_Set_Weight(Swc_Tree_Node *tn, double weight)
{
  tn->weight = weight;
}

Swc_Tree_Node* Swc_Tree_Node_Next(Swc_Tree_Node * tree)
{
  if (tree == NULL) {
    return NULL;
  }

  Swc_Tree_Node *next = tree->first_child;

  if (next == NULL) {
    if (tree->next_sibling != NULL) {
      next = tree->next_sibling;
    } else {
      next = tree->parent;
      if (next != NULL) {
	while (next->next_sibling == NULL) {
	  next = next->parent;
	  if (next == NULL) {
	    break;
	  } 
	}
	if (next != NULL){
	  next = next->next_sibling;
	}
      }
    }
  }

  return next;
}

int Swc_Tree_Node_Fsize(Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return 0;
  }

  int n = 1;
  
  Swc_Tree_Node *pointer = tn;
  Unipointer_List *p = NULL;
  //Unipointer_Queue_En(&p, pointer);

  do {
    Swc_Tree_Node *child = pointer->first_child;

    while (child != NULL) {
      Unipointer_Stack_Push(&p, child);
      n++;
      child = child->next_sibling;
    }
    pointer = (Swc_Tree_Node*) Unipointer_Stack_Pop(&p);
#ifdef _DEBUG_2
    Print_Swc_Tree_Node(pointer);
#endif
  } while (pointer != NULL);

  return n;
}

int Swc_Tree_Node_Build_Downstream_List(Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return 0;
  }

  int n = 1;
  Swc_Tree_Node *pointer = tn;
  Unipointer_List *p = NULL;

  do {
    Swc_Tree_Node *child = pointer->first_child;

    while (child != NULL) {
      Unipointer_Stack_Push(&p, child);
      n++;
      tn->next = child;
      tn = tn->next;
      child = child->next_sibling;
    }
    pointer = (Swc_Tree_Node*) Unipointer_Stack_Pop(&p);
#ifdef _DEBUG_2
    Print_Swc_Tree_Node(pointer);
#endif
  } while (pointer != NULL);

  tn->next = NULL;

  return n;
}

int Swc_Tree_Node_Build_Downstream_List_E(Swc_Tree_Node *tn, 
    Swc_Tree_Node *excluded)
{
  if (tn == NULL) {
    return 0;
  }

  int n = 1;
  Swc_Tree_Node *pointer = tn;
  Unipointer_List *p = NULL;

  do {
    Swc_Tree_Node *child = pointer->first_child;

    while (child != NULL) {
      if (child != excluded) {
        Unipointer_Stack_Push(&p, child);
        n++;
        tn->next = child;
        tn = tn->next;
      }
      child = child->next_sibling;
    }
    pointer = (Swc_Tree_Node*) Unipointer_Stack_Pop(&p);
#ifdef _DEBUG_2
    Print_Swc_Tree_Node(pointer);
#endif
  } while (pointer != NULL);

  tn->next = NULL;

  return n;
}

int Swc_Tree_Node_Build_Downstream_List_L(Swc_Tree_Node *tn, 
    int label)
{
  if (tn == NULL) {
    return 0;
  }

  int n = 1;
  Swc_Tree_Node *pointer = tn;
  Unipointer_List *p = NULL;

  do {
    Swc_Tree_Node *child = pointer->first_child;

    while (child != NULL) {
      if (Swc_Tree_Node_Label(child) == label) {
        Unipointer_Stack_Push(&p, child);
        n++;
        tn->next = child;
        tn = tn->next;
      }
      child = child->next_sibling;
    }
    pointer = (Swc_Tree_Node*) Unipointer_Stack_Pop(&p);
#ifdef _DEBUG_2
    Print_Swc_Tree_Node(pointer);
#endif
  } while (pointer != NULL);

  tn->next = NULL;

  return n;
}

Swc_Tree_Node* Swc_Tree_Node_Furthest_Leaf(Swc_Tree_Node *tn)
{
  Swc_Tree_Node_Build_Downstream_List(tn);
 
  double max_dist = 0.0;
  Swc_Tree_Node *leaf = tn->next;
  Swc_Tree_Node *target = tn;

  while (leaf != NULL) {
    if (Swc_Tree_Node_Is_Leaf(leaf)) {
      double dist = Swc_Tree_Node_Dist(tn, leaf);
      if (dist > max_dist) {
        max_dist = dist;
        target = leaf;
      }
    }
    leaf = leaf->next;
  }

  return target;
}

Swc_Tree_Node* Swc_Tree_Node_Furthest_Leaf_E(Swc_Tree_Node *tn, 
    Swc_Tree_Node *excluded)
{
  Swc_Tree_Node_Build_Downstream_List_E(tn, excluded);
 
  double max_dist = 0.0;
  Swc_Tree_Node *leaf = tn->next;
  Swc_Tree_Node *target = tn;

  while (leaf != NULL) {
    if (Swc_Tree_Node_Is_Leaf(leaf)) {
      double dist = Swc_Tree_Node_Dist(tn, leaf);
      if (dist > max_dist) {
        max_dist = dist;
        target = leaf;
      }
    }
    leaf = leaf->next;
  }

  return target;
}

Swc_Tree_Node* Swc_Tree_Node_Furthest_Leaf_L(Swc_Tree_Node *tn, 
    int label)
{
  Swc_Tree_Node_Build_Downstream_List_L(tn, label);
 
  double max_dist = 0.0;
  Swc_Tree_Node *leaf = tn->next;
  Swc_Tree_Node *target = tn;

  while (leaf != NULL) {
    if (Swc_Tree_Node_Is_Leaf(leaf)) {
      double dist = Swc_Tree_Node_Dist(tn, leaf);
      if (dist > max_dist) {
        max_dist = dist;
        target = leaf;
      }
    }
    leaf = leaf->next;
  }

  return target;
}

Swc_Tree_Node* Swc_Tree_Node_Thickest_Child(const Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return NULL;
  }

  Swc_Tree_Node *child = tn->first_child;
  Swc_Tree_Node *result = child;

  while (child != NULL) {
    if (Swc_Tree_Node_Radius(result) < Swc_Tree_Node_Radius(child)) {
      result = child;
    }
    child = child->next_sibling;
  }

  return result;
}

Swc_Tree_Node* Swc_Tree_Node_Last_Child(const Swc_Tree_Node *tn)
{
  Swc_Tree_Node *child = tn->first_child;
  
  if (child != NULL) {
    while (child->next_sibling != NULL) {
      child = child->next_sibling;
    }
  }

  return child;
}

Swc_Tree_Node* Swc_Tree_Node_Prev_Sibling(const Swc_Tree_Node *tn)
{
  Swc_Tree_Node *sibling = NULL;

  if (tn->parent != NULL) {
    if (tn->parent->first_child != tn) {
      sibling = tn->parent->first_child;
      while ((sibling != NULL) && (sibling->next_sibling != tn)) {
	sibling = sibling->next_sibling;
      }
      if (!((sibling != NULL) && (sibling->next_sibling == tn))) {
	TZ_ASSERT((sibling != NULL) && (sibling->next_sibling == tn), 
		  "Sibling search failed");
      }
    }
  }

  return sibling;
}

Swc_Tree_Node* Swc_Tree_Node_Add_Break(Swc_Tree_Node *tn, double lambda)
{
  if (tn == NULL) {
    return NULL;
  }

  if (Swc_Tree_Node_Is_Root(tn) == TRUE) {
    return tn;
  }

  if (lambda <= 0.0) {
    return tn->parent;
  }

  if (lambda >= 1.0) {
    return tn;
  }

  double start[3], end[3];
  Swc_Tree_Node_Pos(tn->parent, start);
  Swc_Tree_Node_Pos(tn, end);

  Swc_Tree_Node *new_tn = New_Swc_Tree_Node();
  Swc_Tree_Node_Data(new_tn)->x = lambda * end[0] 
    + (1.0 - lambda) * start[0];
  Swc_Tree_Node_Data(new_tn)->y = lambda * end[1] 
    + (1.0 - lambda) * start[1];
  Swc_Tree_Node_Data(new_tn)->z = lambda * end[2] 
    + (1.0 - lambda) * start[2];
  Swc_Tree_Node_Data(new_tn)->d = lambda * Swc_Tree_Node_Data(tn)->d + 
    (1.0 - lambda) * Swc_Tree_Node_Data(tn->parent)->d;
  
  Swc_Tree_Node_Insert_Before(tn, new_tn);

  /*
  if (tn->parent->first_child == tn) {
    tn->parent->first_child = new_tn;
  } else {
    Swc_Tree_Node_Prev_Sibling(tn)->next_sibling = new_tn;
  }

  new_tn->parent = tn->parent;
  new_tn->next_sibling = tn->next_sibling;
  new_tn->first_child = tn;
  tn->next_sibling = NULL;
  tn->parent = new_tn;
  */

  /*
  Swc_Tree_Node_Set_Parent(new_tn, tn->parent);
  Swc_Tree_Node_Set_Parent(tn, new_tn);
  */

  return new_tn;
}

/*
Swc_Tree_Node* Swc_Tree_Node_Cut(Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    if (tn->parent != NULL) {
      Swc_Tree_Node_Set_Parent(tn, NULL);
    }
  }

  return tn;
}
*/

Swc_Tree_Node* Swc_Tree_Node_Merge_To_Parent(Swc_Tree_Node *tn)
{
  Swc_Tree_Node *parent = tn->parent;

  if (tn->parent == NULL) {
    if (tn->first_child == NULL) { 
      /* <tn> has no child, delete it */
      Kill_Swc_Tree_Node(tn);
    } else { 
      /* <tn> has a child, make tn a virtual node */
      tn->node.id = -1;
      tn->node.parent_id = -2;
    }
  } else {
    if (tn->first_child != NULL) { /* tn has children */
      /* link all its children with its parent */
      Swc_Tree_Node *child = tn->first_child;
      while (child != NULL) {
	child->parent = tn->parent;
	child->node.parent_id = tn->parent->node.id;
	if (child->next_sibling == NULL) {
	  /* link the last child of <tn> with the next sibling of <tn> */
	  child->next_sibling = tn->next_sibling;
	  child = NULL;
	} else {
	  child = child->next_sibling;
	}
      }
    }

    if (tn->parent->first_child == tn) { /* tn is the first child */
      if (tn->first_child != NULL) { /* tn has children */
	tn->parent->first_child = tn->first_child;
      } else { /* tn doesn't have children */
	tn->parent->first_child = tn->next_sibling;
      }
    } else { /* tn isn't the first child */
      Swc_Tree_Node *sibling = Swc_Tree_Node_Prev_Sibling(tn);
      if (tn->first_child != NULL) { /* tn has children */
	sibling->next_sibling = tn->first_child;
      } else {
	sibling->next_sibling = tn->next_sibling;
      }
    }
    Kill_Swc_Tree_Node(tn);
  }

  return parent;
}

void Swc_Tree_Node_Merge_Sibling(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  if ((tn1->parent == tn2->parent) && (tn1->parent != NULL)) {
    Swc_Tree_Node_Set_Parent(tn2, tn1);
    Swc_Tree_Node_Merge_To_Parent(tn2);
  }
}

void Swc_Tree_Node_Set_Root(Swc_Tree_Node *tn)
{
  if ((tn == NULL) || (Swc_Tree_Node_Is_Root(tn) == TRUE)) {
    return;
  }

  double weight[2];

  Swc_Tree_Node *buffer1, *buffer2, *buffer3;
  buffer1 = tn;
  buffer2 = buffer1->parent;
  Swc_Tree_Node_Detach_Parent(buffer1);

  weight[0] = buffer1->weight;
  while (Swc_Tree_Node_Is_Regular(buffer1) == TRUE) {
    if (Swc_Tree_Node_Is_Regular(buffer2) == TRUE) {
      weight[1] = buffer2->weight;
      buffer3 = buffer2->parent;
      buffer2->weight = weight[0];
      weight[0] = weight[1];
      Swc_Tree_Node_Add_Child(buffer1, buffer2);
    }
    buffer1 = buffer2;
    buffer2 = buffer3;
  }

  Swc_Tree_Node_Set_Parent(tn, buffer1);
}

void Swc_Tree_Node_Set_Root_A(Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return;
  }

  if (tn->parent == NULL) {
    return;
  }

  double weight[2];

  Swc_Tree_Node *buffer1, *buffer2, *buffer3;
  buffer1 = tn;
  buffer2 = buffer1->parent;
  Swc_Tree_Node_Detach_Parent(buffer1);

  weight[0] = buffer1->weight;
  while ((buffer1 != NULL)){
    if (buffer2 != NULL) {
      weight[1] = buffer2->weight;
      buffer3 = buffer2->parent;
      buffer2->weight = weight[0];
      weight[0] = weight[1];
      Swc_Tree_Node_Add_Child(buffer1, buffer2);
    }
    buffer1 = buffer2;
    buffer2 = buffer3;
  }

  Swc_Tree_Node_Set_Parent(tn, buffer1);
}

BOOL Swc_Tree_Node_Is_Upstream(Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  BOOL is_upstream = FALSE;

  if (tn1 == NULL || tn2 == NULL) {
    return FALSE;
  }

  Swc_Tree_Node *tn = tn2->parent;
  while (tn != NULL) {
    if (tn == tn1) {
      is_upstream = TRUE;
      break;
    }

    tn = tn->parent;
  }

  return is_upstream;
}

Swc_Tree_Node* Swc_Tree_Node_Add_Child(Swc_Tree_Node *tn, Swc_Tree_Node *child)
{
  Swc_Tree_Node *tmp;

  if (child != NULL) {
    /*
    if ((tmp = Swc_Tree_Node_Prev_Sibling(child)) != NULL) {
      tmp->next_sibling = child->next_sibling;
    } else {
      if (child->parent != NULL) {
	child->parent->first_child = child->next_sibling;
      }
    }

    child->next_sibling = NULL;
    child->parent = tn;
    */

    Swc_Tree_Node_Detach_Parent(child);
    child->parent = tn;
    if (tn != NULL) {
      Swc_Tree_Node_Data(child)->parent_id = Swc_Tree_Node_Id(tn);
      if ((tmp = Swc_Tree_Node_Last_Child(tn)) != NULL) {
	tmp->next_sibling = child;
      } else {
	tn->first_child = child;
      }
    } else {
      Swc_Tree_Node_Data(child)->parent_id = -1;
    }
  }

  return child;
}

Swc_Tree_Node* Swc_Tree_Node_Detach_Parent(Swc_Tree_Node *tn)
{
  Swc_Tree_Node *parent = NULL;
  if (tn != NULL) {
    if (tn->parent != NULL) {
      parent = tn->parent;
      Swc_Tree_Node_Remove_Child(tn->parent, tn);
    }
  }

  return parent;
}

void Swc_Tree_Node_Remove_Child(Swc_Tree_Node *tn, Swc_Tree_Node *child)
{
  if (child == NULL) {
    return;
  }

  int succ = 0;

  if (tn->first_child == child) {
    tn->first_child = child->next_sibling;
    succ = 1;
  } else {
    Swc_Tree_Node *sibling = tn->first_child;
    while ((sibling != NULL) && (sibling->next_sibling != child)) {
      sibling = sibling->next_sibling;
    }

    if ((sibling != NULL) && (sibling->next_sibling == child)) {
      sibling->next_sibling = child->next_sibling;
      succ = 1;
    }
  }

  if (succ == 1) {
    child->parent = NULL;
    child->next_sibling = NULL;
  }
}

void Swc_Tree_Node_Set_Parent(Swc_Tree_Node *tn, Swc_Tree_Node *parent)
{
  if (tn == NULL) {
    return;
  }

  TZ_ASSERT(tn != parent, "same node");
  
  if (parent == NULL) {
    Swc_Tree_Node *old_parent = tn->parent;
    if (old_parent != NULL) {
      if (old_parent->first_child == tn) {
	old_parent->first_child = tn->next_sibling;
      } else {
	Swc_Tree_Node_Prev_Sibling(tn)->next_sibling = tn->next_sibling;
      }
      tn->next_sibling = NULL;
      tn->parent = NULL;
    }
    Swc_Tree_Node_Data(tn)->parent_id = -1;
  } else {
    if (tn->parent != parent) {
      Swc_Tree_Node_Data(tn)->parent_id = Swc_Tree_Node_Id(parent);
      Swc_Tree_Node_Add_Child(parent, tn);
    }
  } 
}

void Swc_Tree_Node_Replace_Child(Swc_Tree_Node *old_child, 
				 Swc_Tree_Node *new_child)
{
  TZ_ASSERT(new_child != NULL, "Null tree node");

  if ((old_child->parent == NULL) || (old_child == new_child)) {
    return;
  }
  
  Swc_Tree_Node *parent = old_child->parent;
  Swc_Tree_Node *prev_sibling = Swc_Tree_Node_Prev_Sibling(old_child);
  Swc_Tree_Node_Detach_Parent(new_child);
  if (prev_sibling != NULL) {
    prev_sibling->next_sibling = new_child;
  } else {
    parent->first_child = new_child;
  }
  new_child->next_sibling = old_child->next_sibling;
  new_child->parent = old_child->parent;
  old_child->parent = NULL;
  old_child->next_sibling = NULL;
}

void Swc_Tree_Node_Graft(Swc_Tree_Node *r1, Swc_Tree_Node *t1,
			 Swc_Tree_Node *r2, Swc_Tree_Node *t2)
{
  TZ_ASSERT(!((t1 != NULL) && (t2 == NULL)), "Invalid graft setup.");

  Swc_Tree_Node_Replace_Child(r1, r2);
  if (t1 != NULL) {
    Swc_Tree_Node *child = t1->first_child;
    while (child != NULL) {
      Swc_Tree_Node *next_child = child->next_sibling;
      Swc_Tree_Node_Add_Child(t2, child);
      child = next_child;
    }
  }
}

void Swc_Tree_Node_Insert_Before(Swc_Tree_Node *tn, Swc_Tree_Node *tn2)
{
  if (Swc_Tree_Node_Is_Regular(tn) && Swc_Tree_Node_Is_Regular(tn2)) {
    if (tn->parent != NULL) {
      if (tn->parent->first_child == tn) {
	tn->parent->first_child = tn2;
      } else {
	Swc_Tree_Node_Prev_Sibling(tn)->next_sibling = tn2;
      }
    }
    tn2->next_sibling = tn->next_sibling;
    tn2->parent = tn->parent;
    tn->parent = tn2;
    tn->next_sibling = tn2->first_child;
    tn2->first_child = tn;
  }
}

void Swc_Tree_Node_Set_As_First_Child(Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return;
  }
  if (tn->parent != NULL) {
    if (tn != tn->parent->first_child) {
      Swc_Tree_Node *parent = tn->parent;
      Swc_Tree_Node_Detach_Parent(tn);
      tn->next_sibling = parent->first_child;
      tn->parent = parent;
      parent->first_child = tn;
    }
  }
}

void Swc_Tree_Node_Direction(const Swc_Tree_Node *tn, double *vec)
{
  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;
  
  if (tn->parent != NULL) {
    vec[0] = Swc_Tree_Node_Const_Data(tn)->x - 
      Swc_Tree_Node_Const_Data(tn->parent)->x;
    vec[1] = Swc_Tree_Node_Const_Data(tn)->y - 
      Swc_Tree_Node_Const_Data(tn->parent)->y;
    vec[2] = Swc_Tree_Node_Const_Data(tn)->z - 
      Swc_Tree_Node_Const_Data(tn->parent)->z;
  }
}

Swc_Tree_Node* Swc_Tree_Node_Common_Ancestor(const Swc_Tree_Node *tn1,
    const Swc_Tree_Node *tn2)
{
  if (tn1 == NULL || tn2 == NULL) {
    return NULL;
  }

  Swc_Tree_Node *tn = (Swc_Tree_Node*) tn1;
  while (tn != NULL) {
    tn->flag = 1;
    tn = tn->parent;
  }

  const Swc_Tree_Node *ancestor = NULL;
  const Swc_Tree_Node *tn2_up = tn2;
  while (tn2_up->flag == 0) {
    tn2_up = tn2_up->parent;
    if (tn2_up == NULL) {
      break;
    }
  }

  if (tn2_up != NULL) {
    ancestor = tn2_up;
  }

  /* clear the flags */
  tn = (Swc_Tree_Node*) tn1;
  while (tn != NULL) {
    tn->flag = 0;
    tn = tn->parent;
  }

  return (Swc_Tree_Node*) ancestor;
}

void Swc_Tree_Node_Vector(const Swc_Tree_Node *tn, int direction,
			  double *vec)
{
  vec[0] = 0.0;
  vec[1] = 0.0;
  vec[2] = 0.0;

  if (Swc_Tree_Node_Is_Regular(tn) == FALSE) {
    return;
  }
  
  switch (direction) {
    case SWC_TREE_NODE_BACKWARD:
      if (Swc_Tree_Node_Is_Regular(tn->parent)) {
        vec[0] = Swc_Tree_Node_Const_Data(tn->parent)->x - 
          Swc_Tree_Node_Const_Data(tn)->x;
        vec[1] = Swc_Tree_Node_Const_Data(tn->parent)->y - 
          Swc_Tree_Node_Const_Data(tn)->y;
        vec[2] = Swc_Tree_Node_Const_Data(tn->parent)->z - 
          Swc_Tree_Node_Const_Data(tn)->z;
      }
      break;
    case SWC_TREE_NODE_FORWARD:
      if (tn->first_child != NULL) {
        vec[0] = Swc_Tree_Node_Const_Data(tn->first_child)->x - 
          Swc_Tree_Node_Const_Data(tn)->x;
        vec[1] = Swc_Tree_Node_Const_Data(tn->first_child)->y - 
          Swc_Tree_Node_Const_Data(tn)->y;
        vec[2] = Swc_Tree_Node_Const_Data(tn->first_child)->z - 
          Swc_Tree_Node_Const_Data(tn)->z;      
      }
      break;
    case SWC_TREE_NODE_BIDIRECT:
      {
        double start_pos[3], end_pos[3];
        start_pos[0] = Swc_Tree_Node_Const_Data(tn)->x;
        start_pos[1] = Swc_Tree_Node_Const_Data(tn)->y;
        start_pos[2] = Swc_Tree_Node_Const_Data(tn)->z;
        end_pos[0] = Swc_Tree_Node_Const_Data(tn)->x;
        end_pos[1] = Swc_Tree_Node_Const_Data(tn)->y;
        end_pos[2] = Swc_Tree_Node_Const_Data(tn)->z;

        if (Swc_Tree_Node_Is_Regular(tn->parent)) {
          start_pos[0] = Swc_Tree_Node_Const_Data(tn->parent)->x;
          start_pos[1] = Swc_Tree_Node_Const_Data(tn->parent)->y;
          start_pos[2] = Swc_Tree_Node_Const_Data(tn->parent)->z;
        } else if (Swc_Tree_Node_Child_Number(tn) > 1) {
          start_pos[0] = 
            Swc_Tree_Node_Const_Data(tn->first_child->next_sibling)->x;
          start_pos[1] = 
            Swc_Tree_Node_Const_Data(tn->first_child->next_sibling)->y;
          start_pos[2] = 
            Swc_Tree_Node_Const_Data(tn->first_child->next_sibling)->z;
        }

        if (tn->first_child != NULL) {
          end_pos[0] = Swc_Tree_Node_Const_Data(tn->first_child)->x;
          end_pos[1] = Swc_Tree_Node_Const_Data(tn->first_child)->y;
          end_pos[2] = Swc_Tree_Node_Const_Data(tn->first_child)->z;      
        }
        vec[0] = end_pos[0] - start_pos[0];
        vec[1] = end_pos[1] - start_pos[1];
        vec[2] = end_pos[2] - start_pos[2];
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
      break;
  }
}

BOOL Swc_Tree_Node_Is_Virtual(const Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    if (Swc_Tree_Node_Id(tn) < 0) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Regular(const Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    if (Swc_Tree_Node_Is_Virtual(tn) == FALSE) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Root(const Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return FALSE;
  }

  while (tn->parent != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn->parent)) {
      return FALSE;
    }
    tn = tn->parent;
  }

  return TRUE;

  /* old implementation
  if (tn != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn->parent) == FALSE) {
      return TRUE;
    }
  }

  return FALSE;
  */
}

BOOL Swc_Tree_Node_Is_Regular_Root(const Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Root(tn) && Swc_Tree_Node_Is_Regular(tn)) {
    return TRUE;
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Last_Child(const Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    if (tn->parent != NULL) {
      if (tn->next_sibling == NULL) {
	return TRUE;
      }
    }
  }

  return FALSE;
}

double Swc_Tree_Node_Dot(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2,
    const Swc_Tree_Node *tn3)
{
  if (!Swc_Tree_Node_Is_Regular(tn1) || !Swc_Tree_Node_Is_Regular(tn2) || 
      !Swc_Tree_Node_Is_Regular(tn3)) {
    return 0.0;
  }

  if (!(((tn2->parent == tn1) && (tn3->parent == tn2)) ||
	((tn2->parent == tn3) && (tn1->parent == tn2)))) {
    return 0.0;
  }

  double vec1[3], vec2[3];
  vec1[0] = Swc_Tree_Node_Const_Data(tn1)->x - 
    Swc_Tree_Node_Const_Data(tn2)->x;
  vec1[1] = Swc_Tree_Node_Const_Data(tn1)->y - 
    Swc_Tree_Node_Const_Data(tn2)->y;
  vec1[2] = Swc_Tree_Node_Const_Data(tn1)->z - 
    Swc_Tree_Node_Const_Data(tn2)->z;

  vec2[0] = Swc_Tree_Node_Const_Data(tn2)->x - 
    Swc_Tree_Node_Const_Data(tn3)->x;
  vec2[1] = Swc_Tree_Node_Const_Data(tn2)->y - 
    Swc_Tree_Node_Const_Data(tn3)->y;
  vec2[2] = Swc_Tree_Node_Const_Data(tn3)->z - 
    Swc_Tree_Node_Const_Data(tn3)->z;

  Coordinate_3d_Unitize(vec1);
  Coordinate_3d_Unitize(vec2);

  double d = 
    Geo3d_Dot_Product(vec1[0], vec1[1], vec1[2], vec2[0], vec2[1], vec2[2]);

  return d;
}

BOOL Swc_Tree_Node_Forming_Turn(const Swc_Tree_Node *tn1, 
    const Swc_Tree_Node *tn2, const Swc_Tree_Node *tn3)
{
  if (!Swc_Tree_Node_Is_Regular(tn1) || !Swc_Tree_Node_Is_Regular(tn2) || 
      !Swc_Tree_Node_Is_Regular(tn3)) {
    return FALSE;
  }

  if (!(((tn2->parent == tn1) && (tn3->parent == tn2)) ||
	((tn2->parent == tn3) && (tn1->parent == tn2)))) {
    return FALSE;
  }

  double vec1[3], vec2[3];
  vec1[0] = Swc_Tree_Node_Const_Data(tn1)->x - 
    Swc_Tree_Node_Const_Data(tn2)->x;
  vec1[1] = Swc_Tree_Node_Const_Data(tn1)->y - 
    Swc_Tree_Node_Const_Data(tn2)->y;
  vec1[2] = Swc_Tree_Node_Const_Data(tn1)->z - 
    Swc_Tree_Node_Const_Data(tn2)->z;

  vec2[0] = Swc_Tree_Node_Const_Data(tn2)->x - 
    Swc_Tree_Node_Const_Data(tn3)->x;
  vec2[1] = Swc_Tree_Node_Const_Data(tn2)->y - 
    Swc_Tree_Node_Const_Data(tn3)->y;
  vec2[2] = Swc_Tree_Node_Const_Data(tn3)->z - 
    Swc_Tree_Node_Const_Data(tn3)->z;

  Coordinate_3d_Unitize(vec1);
  Coordinate_3d_Unitize(vec2);

  if (Geo3d_Dot_Product(vec1[0], vec1[1], vec1[2], vec2[0], vec2[1], vec2[2])
      > 0.0) {
    return FALSE;
  }
  
  return TRUE;
}

BOOL Swc_Tree_Node_Is_Turn(Swc_Tree_Node *tn)
{
  if (!Swc_Tree_Node_Is_Continuation(tn)) {
    return FALSE;
  }

  return Swc_Tree_Node_Forming_Turn(tn->parent, tn, tn->first_child);
}

BOOL Swc_Tree_Node_Is_Overshoot(Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Turn(tn)) {
    if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
      if (!Swc_Tree_Node_Is_Branch_Point(tn->first_child)) {
        return TRUE;
      }
    } else {
      if (Swc_Tree_Node_Is_Branch_Point(tn->first_child)) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Leaf(const Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Regular(tn)) {
    if (tn->first_child == NULL) {
      if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
	return TRUE;
      }
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Branch_Point(const Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Regular(tn)) {
    if (tn->first_child != NULL) {
      if (tn->first_child->next_sibling != NULL) {
	return TRUE;
      }
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Branch_Point_S(const Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return FALSE;
  }

  if (tn->first_child != NULL) {
    if (tn->first_child->next_sibling != NULL) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Continuation(const Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Regular(tn)) {
    if ((Swc_Tree_Node_Is_Root(tn) == FALSE) &&
	(Swc_Tree_Node_Is_Leaf(tn) == FALSE) &&
	(Swc_Tree_Node_Is_Branch_Point(tn) == FALSE)) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Spur(const Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Leaf(tn) == TRUE) {
    if (Swc_Tree_Node_Is_Branch_Point(tn->parent) == TRUE) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Is_Sibling(const Swc_Tree_Node *tn1, 
			      const Swc_Tree_Node *tn2)
{
  if ((tn1 != NULL) && (tn2 != NULL)) {
    if (tn1->parent == tn2->parent) {
      return TRUE;
    }
  }

  return FALSE;
}

double Swc_Tree_Node_Dist(const Swc_Tree_Node *tn1, const Swc_Tree_Node *tn2)
{
  if (Swc_Tree_Node_Is_Regular(tn1) && (Swc_Tree_Node_Is_Regular(tn2))) {
    double x = Swc_Tree_Node_Const_Data(tn1)->x - 
      Swc_Tree_Node_Const_Data(tn2)->x;
    double y = Swc_Tree_Node_Const_Data(tn1)->y - 
      Swc_Tree_Node_Const_Data(tn2)->y;
    double z = Swc_Tree_Node_Const_Data(tn1)->z - 
      Swc_Tree_Node_Const_Data(tn2)->z;

    return sqrt(x * x + y * y + z * z); 
  }

  return 0.0;
}

double Swc_Tree_Node_Backtrace_Weight(const Swc_Tree_Node *tn, int n)
{
  double weight = Swc_Tree_Node_Weight(tn);
  
  while (n != 0) {
    if (tn->parent != NULL) {
      tn = tn->parent;
      weight += Swc_Tree_Node_Weight(tn);
    } else {
      break;
    }
    
    if (Swc_Tree_Node_Is_Branch_Point(tn)) {
      n--;
    }
  }

  if (Swc_Tree_Node_Is_Branch_Point(tn)) {
    /* do not include the weight of the last branch point */
    weight -= Swc_Tree_Node_Weight(tn);
  }

  return weight;
}

void Swc_Tree_Node_Label_Branch(Swc_Tree_Node *tn, int label)
{
  Swc_Tree_Node_Data(tn)->label = label;

  if (!Swc_Tree_Node_Is_Branch_Point(tn)) {
    Swc_Tree_Node *current = tn->parent;

    while (Swc_Tree_Node_Is_Continuation(current)) {
      Swc_Tree_Node_Data(current)->label = label;
      current = current->parent;
    }

    if (current != NULL) {
      Swc_Tree_Node_Data(current)->label = label;
    }

    Swc_Tree_Node *child = tn->first_child;
    while (child != NULL) {
      current = child;
      while (Swc_Tree_Node_Is_Continuation(current)) {
	Swc_Tree_Node_Data(current)->label = label;
	current = current->first_child;
      }
      if (current != NULL) {
	Swc_Tree_Node_Data(current)->label = label;
      }
      child = child->next_sibling;
    }
  }
}

void Swc_Tree_Node_Label_Branch_U(Swc_Tree_Node *tn, int label)
{
  Swc_Tree_Node_Data(tn)->label = label;

  Swc_Tree_Node *current = tn->parent;

  while (Swc_Tree_Node_Is_Continuation(current)) {
    Swc_Tree_Node_Data(current)->label = label;
    current = current->parent;
  }
/*
  if (current != NULL) {
    Swc_Tree_Node_Data(current)->label = label;
  }
  */
}

double Swc_Tree_Node_Length(const Swc_Tree_Node *tn)
{
  double length = 0.0;

  if (Swc_Tree_Node_Is_Regular(tn) && (Swc_Tree_Node_Is_Root(tn) == FALSE)) {
    Swc_Tree_Node *parent_tn = tn->parent;
    while (!Swc_Tree_Node_Is_Regular(parent_tn)) {
      parent_tn = parent_tn->parent;
      if (parent_tn == NULL) {
	break;
      }
    }
    
    if (parent_tn != NULL) {
      double x = Swc_Tree_Node_Const_Data(tn)->x - 
	Swc_Tree_Node_Const_Data(parent_tn)->x;
      double y = Swc_Tree_Node_Const_Data(tn)->y - 
	Swc_Tree_Node_Const_Data(parent_tn)->y;
      double z = Swc_Tree_Node_Const_Data(tn)->z - 
	Swc_Tree_Node_Const_Data(parent_tn)->z;
      
      length = sqrt(x * x + y * y + z * z);
    }
  }

  return length;
}

double Swc_Tree_Node_Surface_Area(const Swc_Tree_Node *tn)
{
  double area = 0.0;

  if (Swc_Tree_Node_Is_Regular(tn) && (Swc_Tree_Node_Is_Root(tn) == FALSE)) {
    Swc_Tree_Node *parent_tn = tn->parent;
    while (!Swc_Tree_Node_Is_Regular(parent_tn)) {
      parent_tn = parent_tn->parent;
      if (parent_tn == NULL) {
	break;
      }
    }
    
    if (parent_tn != NULL) {
      double r1 = Swc_Tree_Node_Radius(tn);
      double r2 = Swc_Tree_Node_Radius(parent_tn);
      double h = Swc_Tree_Node_Length(tn);
      double s = sqrt((r2-r1)*(r2-r1) + h * h);
      //area = 2.0 * TZ_2PI * sqrt((r2-r1)*(r2-r1) + h * h);
      //area = TZ_PI * (r2+r1) * h;
      area = TZ_PI * (r2+r1) * s;
    }
  }

  return area;
}

void Swc_Tree_Node_Radial_Shrink(Swc_Tree_Node *tn)
{
  if (Swc_Tree_Node_Is_Leaf(tn)) {
    double length = Swc_Tree_Node_Length(tn);
    const double factor = 1.5;
    double radius = Swc_Tree_Node_Const_Data(tn)->d * factor;
    while (length < radius) {
      tn = Swc_Tree_Node_Merge_To_Parent(tn);
      
      /* Stop shrinking if it's no longer a leaf. */
      if (Swc_Tree_Node_Is_Leaf(tn) == FALSE) {
	break;
      }
      radius -= length;
      length = Swc_Tree_Node_Length(tn);
    }
    
    /* Fine adjustment. */
    if (Swc_Tree_Node_Is_Leaf(tn)) {
      double ratio = radius / length;
      
      Swc_Tree_Node_Data(tn)->x = (1.0 - ratio) * Swc_Tree_Node_Data(tn)->x
	+ ratio * Swc_Tree_Node_Data(tn->parent)->x;
      Swc_Tree_Node_Data(tn)->y = (1.0 - ratio) * Swc_Tree_Node_Data(tn)->y
	+ ratio * Swc_Tree_Node_Data(tn->parent)->y;
      Swc_Tree_Node_Data(tn)->z = (1.0 - ratio) * Swc_Tree_Node_Data(tn)->z
	+ ratio * Swc_Tree_Node_Data(tn->parent)->z;
      Swc_Tree_Node_Data(tn)->d = (1.0 - ratio) * Swc_Tree_Node_Data(tn)->d
	+ ratio * Swc_Tree_Node_Data(tn->parent)->d;
    }
  }
}

/* Swc_Tree_Node_Connect() connects <tn> and <tn2> by looking for the closest
 * the surface point between B = p(tn)->tn->fs(tn) and <tn2>. If <tn2> does 
 * not hit B, the closest surface point is used to find the closest skeleton
 * point. Otherwise, <tn2> itself is used to find the closest skeleton point.
 */
void Swc_Tree_Node_Connect(Swc_Tree_Node *tn, Swc_Tree_Node *tn2)
{
  TZ_ASSERT(tn2 != NULL, "Null pointer.");
  TZ_ASSERT(tn != NULL, "Null pointer.");

  double eps = 0.01;
  double pt[3];
  Swc_Tree_Node *tmp_tn;
  Local_Neuroseg locseg;
  
  double mindist = -1.0;
  double tmp_pt[3], cpt[3];
  
  Swc_Tree_Node_Pos(tn2, pt);

  /* Calculate surface distance between p(tn)->tn and the center of tn2 */
  if (Swc_Tree_Node_To_Locseg(tn, &locseg) != NULL) {
    double d =
      Local_Neuroseg_Point_Dist_S(&locseg, pt[0], pt[1], pt[2], tmp_pt);
    if ((mindist > d) || (mindist < 0.0)) {
      mindist = d;
      cpt[0] = tmp_pt[0];
      cpt[1] = tmp_pt[1];
      cpt[2] = tmp_pt[2];
      tmp_tn = tn;
    }
  }

  /* Calculate surface distance between tn->fs(tn) and the center of tn2 */
  /* <mindist> becomes the surface distance between p(tn)->fs(tn)->tn and the
   * center of tn2 */
  if (Swc_Tree_Node_To_Locseg(tn->first_child, &locseg) != NULL) {
    double d = 
      Local_Neuroseg_Point_Dist_S(&locseg, pt[0], pt[1], pt[2], tmp_pt);
    if ((mindist > d) || (mindist < 0.0)) {
      mindist = d;
      cpt[0] = tmp_pt[0];
      cpt[1] = tmp_pt[1];
      cpt[2] = tmp_pt[2];
      tmp_tn = tn->first_child;
    }
  }

  if (mindist < 0.0) {
    return;
  }

  double lambda;
  double tmp_pos[3];

  /* Determine the node that connects to the branch point directly */
  if (mindist == 0.0) {
    Swc_Tree_Node_Pos(tn2, tmp_pos);
  } else {
    Swc_Tree_Node *new_tn = New_Swc_Tree_Node();
    Swc_Tree_Node_Data(new_tn)->x = cpt[0];
    Swc_Tree_Node_Data(new_tn)->y = cpt[1];
    Swc_Tree_Node_Data(new_tn)->z = cpt[2];
    Swc_Tree_Node_Data(new_tn)->d = Swc_Tree_Node_Data(tn2)->d;
    
    //Swc_Tree_Node_Set_Parent(tn2, new_tn);
    Swc_Tree_Node_Insert_Before(tn2, new_tn);

    tn2 = new_tn;
    Swc_Tree_Node_Pos(new_tn, tmp_pos);
  }

  double start[3], end[3];
  Swc_Tree_Node_Pos(tmp_tn, end);
  Swc_Tree_Node_Pos(tmp_tn->parent, start);

  /* Determine where is the branch point. */
  Geo3d_Point_Lineseg_Dist(tmp_pos, start, end, &lambda);
  if (lambda - eps < 0.0) {
    if (tn2->parent != tmp_tn->parent) {
      Swc_Tree_Node_Set_Parent(tn2, tmp_tn->parent);
    }
  } else if (lambda + eps > 1.0) {
    if (tn2->parent != tmp_tn) {
      Swc_Tree_Node_Set_Parent(tn2, tmp_tn);
    }
  } else {
    Swc_Tree_Node *new_tn = Swc_Tree_Node_Add_Break(tmp_tn, lambda);
    Swc_Tree_Node_Set_Parent(tn2, new_tn);
  }
}

Local_Neuroseg* Swc_Tree_Node_To_Locseg(const Swc_Tree_Node *tn,
					Local_Neuroseg *locseg)
{
  if (Swc_Tree_Node_Is_Root(tn) == TRUE) {
    return NULL;
  }

  if ((Swc_Tree_Node_Is_Regular(tn) == FALSE) ||
      (Swc_Tree_Node_Is_Regular(tn->parent) == FALSE)) {
    return NULL;
  }

  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
  }

  locseg->pos[0] = Swc_Tree_Node_Const_Data(tn->parent)->x;
  locseg->pos[1] = Swc_Tree_Node_Const_Data(tn->parent)->y;  
  locseg->pos[2] = Swc_Tree_Node_Const_Data(tn->parent)->z;
  
  double top[3];
  top[0] = Swc_Tree_Node_Const_Data(tn)->x;
  top[1] = Swc_Tree_Node_Const_Data(tn)->y;  
  top[2] = Swc_Tree_Node_Const_Data(tn)->z;
  
  Local_Neuroseg_Change_Top(locseg, top);
  locseg->seg.r1 = Swc_Tree_Node_Const_Data(tn->parent)->d;
  locseg->seg.scale = 1.0;
  locseg->seg.c = (Swc_Tree_Node_Const_Data(tn)->d - locseg->seg.r1) / 
    (locseg->seg.h - 1.0);
  locseg->seg.alpha = 0;
  locseg->seg.curvature = 0;

  
  return locseg;
}

Local_Neuroseg* Swc_Tree_Node_To_Locseg_P(const Swc_Tree_Node *tn,
    Local_Neuroseg *locseg)
{
  if (Swc_Tree_Node_Is_Root(tn) == TRUE) {
    return NULL;
  }

  if ((Swc_Tree_Node_Is_Regular(tn) == FALSE) ||
      (Swc_Tree_Node_Is_Regular(tn->parent) == FALSE)) {
    return NULL;
  }

  if (locseg == NULL) {
    locseg = New_Local_Neuroseg();
  }

  locseg->pos[0] = Swc_Tree_Node_Const_Data(tn->parent)->x;
  locseg->pos[1] = Swc_Tree_Node_Const_Data(tn->parent)->y;  
  locseg->pos[2] = 0;
  
  double top[3];
  top[0] = Swc_Tree_Node_Const_Data(tn)->x;
  top[1] = Swc_Tree_Node_Const_Data(tn)->y;  
  top[2] = 0;
  
  Local_Neuroseg_Change_Top(locseg, top);
  locseg->seg.r1 = Swc_Tree_Node_Const_Data(tn->parent)->d;
  locseg->seg.scale = 1.0;
  locseg->seg.c = (Swc_Tree_Node_Const_Data(tn)->d - locseg->seg.r1) / 
    (locseg->seg.h - 1.0);
  locseg->seg.alpha = 0;
  locseg->seg.curvature = 0;

  
  return locseg;
}

BOOL Swc_Tree_Node_Hit_Test(Swc_Tree_Node *tn, double x, double y, double z)
{
  if (Swc_Tree_Node_Is_Regular(tn) == FALSE) {
    return FALSE;
  }

  if (Geo3d_Dist_Sqr(Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
		     Swc_Tree_Node_Data(tn)->z, x, y, z) <=
      Swc_Tree_Node_Radius(tn) * Swc_Tree_Node_Radius(tn)) {
    return TRUE;
  }

  if (Swc_Tree_Node_Is_Regular(tn->parent)) {  
    if (Geo3d_Dist_Sqr(Swc_Tree_Node_Data(tn->parent)->x, 
		       Swc_Tree_Node_Data(tn->parent)->y,
		       Swc_Tree_Node_Data(tn->parent)->z, x, y, z) <=
	Swc_Tree_Node_Radius(tn->parent) * 
	Swc_Tree_Node_Radius(tn->parent)) {
      return TRUE;
    }

    Local_Neuroseg locseg;
    Swc_Tree_Node_To_Locseg(tn, &locseg);

    return Local_Neuroseg_Hit_Test(&locseg, x, y, z);
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Hit_Test_N(Swc_Tree_Node *tn, double x, double y, double z)
{
  if (Swc_Tree_Node_Is_Regular(tn) == FALSE) {
    return FALSE;
  }

  if (Geo3d_Dist_Sqr(Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
		     Swc_Tree_Node_Data(tn)->z, x, y, z) <=
      Swc_Tree_Node_Radius(tn) * Swc_Tree_Node_Radius(tn)) {
    return TRUE;
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Hit_Test_P(Swc_Tree_Node *tn, double x, double y)
{
  if (Swc_Tree_Node_Is_Regular(tn) == FALSE) {
    return FALSE;
  }

  if (Geo3d_Dist_Sqr(Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
		     0.0, x, y, 0.0) <=
      Swc_Tree_Node_Radius(tn) * Swc_Tree_Node_Radius(tn)) {
    return TRUE;
  }

  return FALSE;
}

BOOL Swc_Tree_Node_Edge(const Swc_Tree_Node *tn, double *start, double *end)
{
  if ((Swc_Tree_Node_Is_Root(tn) == TRUE) || (Swc_Tree_Node_Is_Virtual(tn))) {
    return FALSE;
  }

  Swc_Tree_Node_Pos(tn->parent, start);
  Swc_Tree_Node_Pos(tn, end);

  return TRUE;
}

void Swc_Tree_Node_Tune_Fork(Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return;
  }

  if ((Swc_Tree_Node_Is_Root(tn) == TRUE) || 
      (tn->first_child == NULL)) {
    return;
  }

  Swc_Tree_Node *child = tn->first_child;
  
  if (child->next_sibling == NULL) {
    return;
  }
  
  Swc_Tree_Node *next_child = child->next_sibling;
  
  //Swc_Tree_Node *next_child = child;
  Swc_Tree_Node *buffer;

  while (next_child != NULL) {
    buffer = next_child->next_sibling;
    if (next_child->weight > 0.0) {
      if (Swc_Tree_Node_Is_Turn(next_child)) {
	Swc_Tree_Node_Connect(tn, next_child);
      }
    }
    next_child = buffer;
  }
}

void Swc_Tree_Node_Tune_Branch(Swc_Tree_Node *tn)
{
  double dist;
  if ((Swc_Tree_Node_Is_Branch_Point(tn) == FALSE)) {
    if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) { /* parent is branching */
      if (tn->weight > 0.0) {
	double mindist = -1.0;
	/* set new_tn to p(tn) */
	Swc_Tree_Node *new_tn = tn->parent;
	/* new_tn->tn->fc(tn) does not form a turn.*/
	if (Swc_Tree_Node_Forming_Turn(tn->first_child, tn, tn->parent) 
	    == FALSE) {
	  mindist = Swc_Tree_Node_Dist(tn, tn->parent);
	}
      
	/* set new_tn to p(p(tn)) if dist(tn, p(p(tn))) < dist(tn, p(tn)) */
	if (Swc_Tree_Node_Forming_Turn(tn->first_child, tn, tn->parent->parent) 
	    == FALSE) {
	  dist = Swc_Tree_Node_Dist(tn, tn->parent->parent);
	  if ((mindist < 0.0) || (dist < mindist)) {
	    mindist = dist;
	    new_tn = tn->parent->parent;
	  }
	}

	Swc_Tree_Node *child = tn->parent->first_child;
	/* set new_tn to argmin_{tn2 \in s(tn)}(dist(tn, tn2)) if the distance
	 * is smaller than the prevous one.  */
	while (child != NULL) {
	  if (child != tn) {
	    if (Swc_Tree_Node_Forming_Turn(tn->first_child, tn, child)
		== FALSE) {
	      dist = Swc_Tree_Node_Dist(tn, child);
	      if ((mindist < 0.0) || (dist < mindist)) {
		mindist = dist;
		new_tn = child;
	      }
	    }
	  }
	  child = child->next_sibling;
	}

	if (new_tn != tn->parent) {
	  Swc_Tree_Node_Set_Parent(tn, new_tn);
	}
      }
    } 

    Swc_Tree_Node *child = tn->first_child;
    /* ? This part needs double check. */
    while (child != NULL) {
      if (Swc_Tree_Node_Is_Branch_Point(child)) { /* child is branching */
	if (child->weight > 0.0) {
	  double mindist = -1.0;
	  Swc_Tree_Node *new_tn = child;
	  if (Swc_Tree_Node_Forming_Turn(tn->parent, tn, child) == FALSE) {
	    mindist = Swc_Tree_Node_Dist(tn, child);
	  }
	  Swc_Tree_Node *grandchild = child->first_child;
	  while (grandchild != NULL) {
	    if (Swc_Tree_Node_Forming_Turn(tn->parent, tn, grandchild)
		== FALSE) {
	      dist = Swc_Tree_Node_Dist(tn, grandchild);
	      if ((mindist < 0.0) || (dist < mindist)) {
		mindist = dist;
		new_tn = grandchild;
	      }
	    }
	    grandchild = grandchild->next_sibling;
	  }

	  if (new_tn != child) {
	    Swc_Tree_Node_Replace_Child(child, new_tn);
	    Swc_Tree_Node_Set_Parent(child, new_tn);
	    child = new_tn;
	  }
	}
      }
      child = child->next_sibling;
    }
  }
}

double Swc_Tree_Node_Average_Thickness(const Swc_Tree_Node *start,
				       const Swc_Tree_Node *end)
{
  TZ_ASSERT(end != NULL, "Null pointer.");
  TZ_ASSERT(Swc_Tree_Node_Is_Virtual(end) == FALSE, "Virtual node.");

#ifdef _DEBUG_2
  if (start != NULL) {
    const Swc_Tree_Node *test_tn = end;
    BOOL found = FALSE;
    while (test_tn != NULL) {
      if (test_tn == start) {
	found = TRUE;
	break;
      }
      
      test_tn = test_tn->parent;
    }
    
    if (!found) {
      PRINT_EXCEPTION("Invalid input", "start and end are not in the same branch");
      TZ_ERROR(ERROR_DATA_VALUE);
    }
  }
#endif
  
  if (start == end) {
    return Swc_Tree_Node_Radius(start);
  }

  double length = 0.0;
  const Swc_Tree_Node *tn = end;
  double r = 0.0;
  
  while (tn != start) {
    if (!Swc_Tree_Node_Is_Root(tn)) {
      double d =Swc_Tree_Node_Length(tn); 
      length += d;
      r += (Swc_Tree_Node_Radius(tn) + Swc_Tree_Node_Radius(tn->parent)) * 0.5
	* d;
    } else {
      break;
    }
    
    tn = tn->parent;
  }
  
  if (length == 0.0) {
    r = Swc_Tree_Node_Radius(end);
  } else {
    r /= length;
  }

  return r;
}

void Print_Swc_Tree_Branch(const Swc_Tree_Branch *tb)
{
  printf("Swc branch: #%d\n", tb->n);
  if (tb->end == NULL) {
    printf("Null branch.\n");
  }

  int n = tb->n;
  Swc_Tree_Node *tn = tb->end;
  while (n > 0) {
    if (tn != NULL) {
      Print_Swc_Tree_Node(tn);
      tn = tn->parent;
      n--;
    } else {
      n = 0;
    }
  }
}

void Swc_Tree_Get_Branch(Swc_Tree_Node *tn, BOOL include_bp, 
    Swc_Tree_Branch *tb)
{
  tb->end = tn;
  tb->n = 0;

  if (tn != NULL) {
    tb->n = 1;
    while (Swc_Tree_Node_Is_Regular(tn->parent) &&
	(Swc_Tree_Node_Is_Branch_Point(tn->parent) == FALSE)) {
      tn = tn->parent;
      tb->n++;
    }

    if (include_bp) {
      if (Swc_Tree_Node_Is_Regular(tn->parent)) {
	tb->n++;
      }
    }
  }
}

void Set_Swc_Tree_Branch(Swc_Tree_Branch *tb, Swc_Tree_Node *tn, int n)
{
  tb->end = tn;
  tb->n = 0;
  if (n > 0) {
    while (n > 0) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
	tb->n++;
	tn = tn->parent;
	n--;
      } else {
	n = 0;
      }
    }
  } else {
    while (Swc_Tree_Node_Is_Regular(tn)) {
      tb->n++;
      tn = tn->parent;
    }
  }
}

double Swc_Tree_Branch_Length(const Swc_Tree_Branch *tb)
{
  if (tb->end == NULL) {
    return 0.0;
  }

  double length = 0.0;
  int n = tb->n - 1;
  Swc_Tree_Node *tn = tb->end;
  while (n > 0) {
    length += Swc_Tree_Node_Length(tn);
    if (tn->parent != NULL) {
      tn = tn->parent;
      n--;
    } else {
      n = 0;
    }
  }

  return length;
}

double Swc_Tree_Branch_Surface_Area(const Swc_Tree_Branch *tb)
{
  if (tb->end == NULL) {
    return 0.0;
  }

  double area = 0.0;
  int n = tb->n - 1;
  Swc_Tree_Node *tn = tb->end;
  while (n > 0) {
    area += Swc_Tree_Node_Surface_Area(tn);
    if (tn->parent != NULL) {
      tn = tn->parent;
      n--;
    } else {
      n = 0;
    }
  }

  return area;
}

Swc_Tree* New_Swc_Tree()
{
  Swc_Tree *tree = (Swc_Tree*) 
    Guarded_Malloc(sizeof(Swc_Tree), "New_Swc_Tree");

  Default_Swc_Tree(tree);

  return tree;
}

void Default_Swc_Tree(Swc_Tree *tree)
{
  if (tree != NULL) {
    tree->root = NULL;
    tree->iterator = NULL;
    tree->begin = NULL;
  }
}

void Clean_Swc_Tree(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *node = NULL;
  while ((node = Swc_Tree_Next(tree)) != NULL) {
#ifdef _DEBUG_2
    printf("Deleting %p\n", (void*) node);
#endif
    Kill_Swc_Tree_Node(node);
  }  
}

void Kill_Swc_Tree(Swc_Tree *tree)
{
  Clean_Swc_Tree(tree);
  free(tree);
}

Swc_Tree_Node* Swc_Tree_Regular_Root(Swc_Tree *tree)
{
  if (Swc_Tree_Node_Is_Regular(tree->root)) {
    return tree->root;
  } else {
    if (tree->root != NULL) {
      return tree->root->first_child;
    }
  }

  return NULL;
}

Swc_Tree* Read_Swc_Tree(const char *file_path)
{
  return Read_Swc_Tree_E(file_path);
#if 0
  int max_id = Swc_File_Max_Id(file_path);

  if (max_id < 0) {
    return NULL;
  }

  /* alloc <map> */
  Swc_Tree_Node_Map *map = (Swc_Tree_Node_Map *) 
    Guarded_Malloc(sizeof(Swc_Tree_Node_Map) * (max_id + 2), 
		   "Read_Swc_Tree");

  int i;
  for (i = 1; i <= max_id + 1; i++) {
    map[i].tree_node = NULL;
  }

  FILE *fp = Guarded_Fopen((char*) file_path, "r", "Read_Swc_Tree");

  int n = 0;

  Swc_Node node;
  Default_Swc_Node(&node);
  while (!feof(fp)) {
    if (Swc_Node_Fscan(fp, &node) == 1) {
      map[node.id + 1].tree_node = New_Swc_Tree_Node();
      map[node.id + 1].tree_node->node = node;
      n++;
    }
  }

  fclose(fp);

  Swc_Tree *tree = New_Swc_Tree();

  tree->root = Make_Virtual_Swc_Tree_Node();
  /*
  New_Swc_Tree_Node();
  tree->root->node.id = -1;
  tree->root->node.parent_id = -2;
  */

  map[0].tree_node = tree->root;

  /* construct tree data structure */
  for (i = 1; i <= max_id + 1; i++) {
    if (map[i].tree_node != NULL) {
      map[i].tree_node->parent = 
	map[map[i].tree_node->node.parent_id + 1].tree_node;
<<<<<<< HEAD

      if (map[map[i].tree_node->node.parent_id + 1].tree_node == NULL) {
        printf("WARNING reading %s : Node %d does not exist.\n", 
            file_path, map[i].tree_node->node.parent_id);
        map[i].tree_node->node.parent_id = -1;
        map[map[i].tree_node->node.parent_id + 1].tree_node = tree->root;
      }

=======
      if (map[map[i].tree_node->node.parent_id + 1].tree_node == NULL) {
        map[map[i].tree_node->node.parent_id + 1].tree_node = tree->root;
        map[i].tree_node->parent = tree->root;
      }
>>>>>>> master
      if (map[map[i].tree_node->node.parent_id + 1].tree_node->first_child 
	  == NULL) {
	map[map[i].tree_node->node.parent_id + 1].tree_node->first_child = 
	  map[i].tree_node;
      } else {
	Swc_Tree_Node *sibling = 
	  map[map[i].tree_node->node.parent_id + 1].tree_node->first_child;
	while (sibling->next_sibling != NULL) {
	  sibling = sibling->next_sibling;
	}
	sibling->next_sibling = map[i].tree_node;
      }
    }
  }

  /* free <map> */
  free(map);

  return tree;
#endif
}

Swc_Tree* Read_Swc_Tree_E(const char *file_path)
{
  FILE *fp = fopen(file_path, "r");
  if (fp == NULL) {
    TZ_WARN(ERROR_IO_OPEN);
    return NULL;
  }

  int max_id = Swc_File_Max_Id(file_path);

  if (max_id < 0) {
    return NULL;
  }

  /* alloc <map> */
  Swc_Tree_Node_Map *map = (Swc_Tree_Node_Map *) 
    Guarded_Malloc(sizeof(Swc_Tree_Node_Map) * (max_id + 2), 
		   "Read_Swc_Tree");

  int i;
  for (i = 1; i <= max_id + 1; i++) {
    map[i].tree_node = NULL;
  }

  String_Workspace *sw = New_String_Workspace();

  char *line = NULL;

  int n = 0;

#define MAX_SWC_FIELD_NUMBER 100

  double value[MAX_SWC_FIELD_NUMBER];

  while ((line = Read_Line(fp, sw)) != NULL) {
    int field_number;
    int cpos;
    int csize = strlen(line);
    BOOL commentFound = FALSE;
    BOOL specialCommentFound = FALSE;
    for (cpos = 0; cpos < csize; cpos++) {
      if (commentFound) {
        if (line[cpos] == '@') {
          specialCommentFound = !specialCommentFound;
        }
        if (specialCommentFound == FALSE) {
          line[cpos] = ' ';
        }
      }
      if (line[cpos] == '#') {
        commentFound = TRUE;
      }
    }

    String_To_Double_Array(line, value, &field_number);

    if (field_number >= 7) {
      Swc_Node node;
      Default_Swc_Node(&node);
      node.id = (int) value[0];
      node.type = (int) value[1];
      node.x = value[2];
      node.y = value[3];
      node.z = value[4];
      node.d = value[5];
      node.parent_id = (int) value[6];

      map[node.id + 1].tree_node = New_Swc_Tree_Node();
      if (field_number >= 8) {
        node.label = value[7];
      }
      if (field_number >= 9) {
        map[node.id + 1].tree_node->feature = value[8];
      }
      if (field_number >= 10) {
        map[node.id + 1].tree_node->weight = value[9];
      }

      map[node.id + 1].tree_node->node = node;
      n++;
    }
  }

  Kill_String_Workspace(sw);
  fclose(fp);

  Swc_Tree *tree = New_Swc_Tree();

  tree->root = New_Swc_Tree_Node();
  tree->root->node.id = -1;
  tree->root->node.parent_id = -2;

  map[0].tree_node = tree->root;

  for (i = 1; i <= max_id + 1; i++) {
    if (map[i].tree_node != NULL) {
      map[i].tree_node->parent = 
	map[map[i].tree_node->node.parent_id + 1].tree_node;

      if (map[map[i].tree_node->node.parent_id + 1].tree_node == NULL) {
        printf("WARNING reading %s : Node %d does not exist.\n", 
            file_path, map[i].tree_node->node.parent_id);
        map[i].tree_node->parent = tree->root;
        map[i].tree_node->node.parent_id = -1;
      }

      if (map[map[i].tree_node->node.parent_id + 1].tree_node->first_child 
	  == NULL) {
	map[map[i].tree_node->node.parent_id + 1].tree_node->first_child = 
	  map[i].tree_node;
      } else {
	Swc_Tree_Node *sibling = 
	  map[map[i].tree_node->node.parent_id + 1].tree_node->first_child;
	while (sibling->next_sibling != NULL) {
	  sibling = sibling->next_sibling;
	}
	sibling->next_sibling = map[i].tree_node;
      }
    }
  }

  /* free <map> */
  free(map);

  return tree;
}

BOOL Write_Swc_Tree(const char *file_path, Swc_Tree *tree)
{
  FILE *fp = fopen(file_path, "w");

  if (fp == NULL) {
    return FALSE;
  }

  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = NULL;


#ifdef _DEBUG_
  BOOL is_consistent = TRUE;
#endif

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (tn->node.id >= 0) {
#ifdef _DEBUG_
      if (tn->parent != NULL) {
        if ((Swc_Tree_Node_Parent_Id(tn) != Swc_Tree_Node_Id(tn->parent)) ||
            (Swc_Tree_Node_Id(tn) == Swc_Tree_Node_Id(tn->parent))) {
          is_consistent = FALSE;
        }
      }
#endif
      Swc_Node_Fprint(fp, &(tn->node));
    }
  }

#ifdef _DEBUG_
  if (is_consistent == FALSE) {
    fprintf(stderr, "WARNING: The swc file %s is created from an inconsistent"
        "tree. "
        "Swc_Tree_Resort_Id() can be called to resolve the insconsistency\n",
        file_path);
  }
#endif

  fclose(fp);

  return TRUE;
}

BOOL Write_Swc_Tree_E(const char *file_path, Swc_Tree *tree)
{
  FILE *fp = fopen(file_path, "w");

  if (fp == NULL) {
    return FALSE;
  }
  

  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = NULL;


#ifdef _DEBUG_
  BOOL is_consistent = TRUE;
#endif

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (tn->node.id >= 0) {
#ifdef _DEBUG_
      if (tn->parent != NULL) {
        if ((Swc_Tree_Node_Parent_Id(tn) != Swc_Tree_Node_Id(tn->parent)) ||
            (Swc_Tree_Node_Id(tn) == Swc_Tree_Node_Id(tn->parent))) {
          is_consistent = FALSE;
        }
      }
#endif
      fprintf(fp, "%d %d %g %g %g %g %d",
          tn->node.id, tn->node.type, tn->node.x, tn->node.y,
          tn->node.z, tn->node.d, tn->node.parent_id);
      if ((tn->node.label > 0) || (tn->weight > 0.0) || (tn->feature > 0.0)) {
        fprintf(fp, " #@%d,%g,%g@\n", tn->node.label, tn->weight, tn->feature);
      } else {
        fprintf(fp, "\n");
      }
    }
  }

#ifdef _DEBUG_
  if (is_consistent == FALSE) {
    fprintf(stderr, "WARNING: The swc file %s is created from an inconsistent"
        "tree. "
        "Swc_Tree_Resort_Id() can be called to resolve the insconsistency\n",
        file_path);
  }
#endif

  fclose(fp);
  
  return TRUE;
}

BOOL Swc_Tree_Node_Is_Child(const Swc_Tree_Node *parent, 
			    const Swc_Tree_Node *child)
{
  BOOL found = FALSE;
  Swc_Tree_Node *tn = parent->first_child;
  while (tn != NULL) {
    if (tn == child) {
      found = TRUE;
      break;
    }
    tn = tn->next_sibling;
  }

  return found;
}

int Swc_Tree_Node_Child_Number(const Swc_Tree_Node *tn)
{
  if (tn == NULL) {
    return 0;
  }

  int n = 0;
  Swc_Tree_Node *child = tn->first_child;
  while (child != NULL) {
    n++;
    child = child->next_sibling;
  }

  return n;
}

void Print_Swc_Tree(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *node = NULL;

  while ((node = Swc_Tree_Next(tree)) != NULL) {
#ifdef _DEBUG_
    if (node->parent != NULL) {
      if (Swc_Tree_Node_Is_Child(node->parent, node) == FALSE) {
	printf("bug found\n");
	break;
      }
    }
#endif
    Print_Swc_Tree_Node(node);
  }
}

Swc_Tree* Copy_Swc_Tree(Swc_Tree *tree)
{
  if (tree == NULL) {
    return NULL;
  }

  Swc_Tree *new_tree = New_Swc_Tree();
  
  if (tree->root == NULL) {
    new_tree->root = NULL;
  } else {
    new_tree->root = New_Swc_Tree_Node();
    Swc_Tree_Node_Copy_Property(tree->root, new_tree->root);

    int n = Swc_Tree_Iterator_Start(tree, 1, TRUE);
  
    if (n > 1) {
      Swc_Tree_Node **tn_array;
      GUARDED_MALLOC_ARRAY(tn_array, n, Swc_Tree_Node*);

      tn_array[0] = new_tree->root;

      int i;
      for (i = 1; i < n; i++) {
	tn_array[i] = New_Swc_Tree_Node();
      }
    
      Swc_Tree_Node *tn = Swc_Tree_Next(tree);

      while ((tn = Swc_Tree_Next(tree)) != NULL) {
	Swc_Tree_Node_Copy_Property(tn, tn_array[tn->index]);
	Swc_Tree_Node_Add_Child(tn_array[tn->parent->index], 
				tn_array[tn->index]);
      }
      
      free(tn_array);
    }
  }

  return new_tree;
}

BOOL Swc_Tree_Iterator_Is_Active(int option)
{
  return (option != SWC_TREE_ITERATOR_NO_UPDATE && 
      option != SWC_TREE_ITERATOR_VOID);
}

int Swc_Tree_Iterator_Start(Swc_Tree *tree, int option, BOOL indexing)
{
  if (tree->root == NULL) {
    tree->iterator = NULL;
    tree->begin = NULL;
    return 0;
  }

  Swc_Tree_Node *tn = NULL;
  int count = 1;

  switch(option) {
    case SWC_TREE_ITERATOR_VOID:
      if (indexing) {
        count = 0;
        tn = tree->iterator;
        while (tn != NULL) {
          tn->index = count;
          count++;
          tn = tn->next;
        }
      } else {
        count = -1;
      }
      break;
    case 0:
      if (indexing) {
        count = 0;
        tn = tree->begin;
        while (tn != NULL) {
          tn->index = count;
          count++;
          tn = tn->next;
        }
      } else {
        count = -1;
      }
      break;
    case 1:
      {
        tree->begin = tree->root;
        tn = tree->begin;
        if (indexing) {
          tn->index = 0;
        }

        /*
        if (tn != NULL) {
          tn->next = tn->first_child;
          if (tn->next != NULL) {
            if (indexing) {
              tn->next->index = tn->index + 1;
            }
          }
          count++;
          tn = tn->next;
        }
        */

        while (tn != NULL) {
          if (tn->first_child != NULL) {
            tn->next = tn->first_child;
          } else {
            if (tn != tree->root && tn->next_sibling != NULL) {
              tn->next = tn->next_sibling;
            } else {
              Swc_Tree_Node *parent_tn = tn->parent;
              while (parent_tn != tree->root->parent) {
                if (parent_tn != tree->root && 
                    parent_tn->next_sibling != NULL) {
                  tn->next = parent_tn->next_sibling;
                  break;
                } else {
                  parent_tn = parent_tn->parent;
                }
              }
              if (parent_tn == tree->root->parent) {
                tn->next = NULL;
              }
            }
          }

          if (tn->next != NULL) {
            if (indexing) {
              tn->next->index = tn->index + 1;
            }
            count++;
          }
          tn = tn->next;
        }
      }
      break;
    case 2:
      {
        tree->begin = tree->root;
        tn = tree->begin;
        if (indexing) {
          tn->index = 0;
        }
        Swc_Tree_Node *pointer = tn;
        pointer->next = NULL;
        while (pointer != NULL) {
          Swc_Tree_Node *child = pointer->first_child;
          while (child != NULL) {
            tn->next = child;
            if (indexing) {
              tn->next->index = tn->index + 1;
            }
            count++;
            tn = tn->next;
            if (tn != NULL) {
              tn->next = NULL;
            }
            child = child->next_sibling;
          }
          pointer = pointer->next;
        }
      }
      break;
    case -1:
      {
        tn = tree->begin;

        if (tn == NULL) { /* no iteration available*/
          return -1;
        }

        Swc_Tree_Node *tmp_tn1 = NULL;
        Swc_Tree_Node *tmp_tn2 = tn;
        count = 0;
        while (tn != NULL) { /* <-o<-o o->o-> */
          tmp_tn2 = tn->next;
          tn->next = tmp_tn1;
          tmp_tn1 = tn;
          tn = tmp_tn2;
          count++;
        }
        tree->begin = tmp_tn1;

        if (indexing) {
          tn = tree->begin;
          tn->index = 0;
          while (tn->next != NULL) {
            tn->next->index = tn->index + 1;
            tn = tn->next;
          }
        }
      }
      break;
    case SWC_TREE_ITERATOR_LEAF:
      {
        Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
        Swc_Tree_Node *current_node = NULL;
        Swc_Tree_Node *tn = NULL;
        tree->begin = NULL;
        count = 0;
        while ((tn = Swc_Tree_Next(tree)) != NULL) {
          if (Swc_Tree_Node_Is_Leaf(tn)) {
            count++;
            if (current_node == NULL) {
              current_node = tn;
              tree->begin = tn;
              if (indexing == TRUE) {
                current_node->index = 0;
              }
            } else {
              current_node->next = tn;
              if (indexing == TRUE) {
                tn->index = current_node->index + 1;
              }
              current_node = tn;
              current_node->next = NULL;
            }
          }
        }
      }
      break;
    default:
      TZ_ERROR(ERROR_DATA_VALUE);
      break;
  }

  if (option != SWC_TREE_ITERATOR_VOID) {
    tree->iterator = tree->begin;
  }

  return count;
}

void Swc_Tree_Iterator_Path(Swc_Tree *tree, Swc_Tree_Node *begin,
			    Swc_Tree_Node *end)
{
  TZ_ASSERT((begin != NULL) && (end != NULL), "Null begin or end");

  tree->begin = begin;
  tree->iterator = begin;
  end->next = NULL;

  if (begin == end) {
    return;
  }

  BOOL done = FALSE;
  Swc_Tree_Node *tn = begin;
  do {
    tn->next = tn->parent;
    if (tn->parent == end) {
      done = TRUE;
      break;
    }
    tn = tn->parent;
  } while (tn != NULL);

  if (done == FALSE) {
    tn = end;
    while (tn->parent != NULL) {
      tn->parent->next = tn;
      tn = tn->parent;
      if (tn == begin) {
	break;
      }
    }
  }
}

void Swc_Tree_Iterator_Leaf(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *leaf_header = NULL;
  Swc_Tree_Node *leaf = NULL;
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Leaf(tn)) {
      if (leaf_header == NULL) {
	leaf_header = tn;
	leaf = tn;
      } else {
	leaf->next = tn;
	leaf = tn;
      }
    }
  }
  tree->begin = leaf_header;
}

Swc_Tree_Node* Swc_Tree_Next(Swc_Tree *tree)
{
  Swc_Tree_Node *tn = tree->iterator;

  if (tn != NULL) {
    //tree->iterator = Swc_Tree_Node_Next(tn);
    tree->iterator = tn->next;
  }

  return tn;
}

int Swc_Tree_Number(Swc_Tree *tree)
{
  if (tree->root == NULL) {
    return 0;
  }

  int n = 0;

  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    n = Swc_Tree_Node_Child_Number(tree->root);
  } else {
    n = 1;
  }

  return n;
}

Swc_Tree_Node* Swc_Tree_Query_Node(Swc_Tree *tree, int id, int iter_option)
{
  Swc_Tree_Iterator_Start(tree, iter_option, FALSE);

  Swc_Tree_Node *tn = NULL;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) == id) {
      break;
    }
  }

  return tn;
}

Swc_Tree_Node* Swc_Tree_Query_Node_C(Swc_Tree *tree, int id)
{
  return Swc_Tree_Query_Node(tree, id, SWC_TREE_ITERATOR_DEPTH_FIRST);
}

void Swc_Tree_To_Dot_File(Swc_Tree *tree, const char *file_path)
{
  FILE *fp = Guarded_Fopen((char*) file_path, "w", "Swc_Tree_To_Dot_File");

  fprintf(fp, "digraph G {\n");
  
  if (tree != NULL) {
    Swc_Tree_Iterator_Start(tree, 2, FALSE);
    Swc_Tree_Node *tn = NULL;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (tn->node.id >= 0) {
	if (Swc_Tree_Node_Data(tn)->label > 0) {
	  fprintf(fp, "%d [style=filled]\n", Swc_Tree_Node_Data(tn)->id);
	}
	if (tn->parent != NULL) {
	  if (tn->parent->node.id >= 0) {   
	    fprintf(fp, "%d -> %d", tn->parent->node.id, tn->node.id);
	    if (tn->weight > 0.0) {
	      fprintf(fp, "[label=\"%g\"]", tn->weight);
	    }
	    fprintf(fp, ";\n");
	  }
	}
      }
    }
  }

  fprintf(fp, "}");

  fclose(fp);
}

void Swc_Tree_Resort_Id(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  int id = 1;
  Swc_Tree_Node *tn = NULL;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) >= 0) {
      Swc_Tree_Node_Set_Id(tn, id);
      id++;
    }
    if (tn->parent != NULL) {
      Swc_Tree_Node_Set_Parent_Id(tn, Swc_Tree_Node_Id(tn->parent));
    } else {
      Swc_Tree_Node_Set_Parent_Id(tn, -1);
    }
  }
}

void Swc_Tree_Node_Pos(const Swc_Tree_Node *tn, double *pos)
{
  pos[0] = Swc_Tree_Node_Const_Data(tn)->x;
  pos[1] = Swc_Tree_Node_Const_Data(tn)->y;
  pos[2] = Swc_Tree_Node_Const_Data(tn)->z;
}

int Swc_Tree_Node_Type(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->type;
}

double Swc_Tree_Node_X(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->x;
}

double Swc_Tree_Node_Y(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->y;
}

double Swc_Tree_Node_Z(const Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Const_Data(tn)->z;
}

void Swc_Tree_Node_Set_Pos(Swc_Tree_Node *tn, const double *pos)
{
  Swc_Tree_Node_Data(tn)->x = pos[0];
  Swc_Tree_Node_Data(tn)->y = pos[1];
  Swc_Tree_Node_Data(tn)->z = pos[2];
}

void Swc_Tree_Remove_Zigzag(Swc_Tree *tree)
{
  BOOL zigzag_found = TRUE;
  while (zigzag_found) {
    zigzag_found = FALSE;
    Swc_Tree_Iterator_Start(tree, 1, FALSE);

    Swc_Tree_Node *tn = Swc_Tree_Next(tree);
    Swc_Tree_Node *next = NULL;
    while (tn != NULL) {
      next = Swc_Tree_Next(tree);

      if (Swc_Tree_Node_Is_Turn(tn) && 
	  Swc_Tree_Node_Is_Turn(tn->first_child)) {
	next = tn->first_child->next;
	Swc_Tree_Node_Merge_To_Parent(tn->first_child);
	zigzag_found = TRUE;
      }

      tn = next;
    }
  }
}

void Swc_Tree_Remove_Overshoot(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  Swc_Tree_Node *next = NULL;
  while (tn != NULL) {
    next = Swc_Tree_Next(tree);
    
    if (Swc_Tree_Node_Is_Overshoot(tn)) {
      Swc_Tree_Node_Merge_To_Parent(tn);
    }
    
    tn = next;
  }
}

double Swc_Tree_Point_Dist(Swc_Tree *tree, double x, double y, double z,
			   double *cpt, Swc_Tree_Node **ctn)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  
  //Local_Neuroseg *locseg = New_Local_Neuroseg();
  Local_Neuroseg locseg;

  Swc_Tree_Node *tn = NULL;
  
  double mindist = -1.0;
  double tmp_pt[3];
  if (ctn != NULL) {
    *ctn = NULL;
  }

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_To_Locseg(tn, &locseg) != NULL) {
      double d = Local_Neuroseg_Point_Dist_S(&locseg, x, y, z, tmp_pt);
      if ((mindist > d) || (mindist < 0.0)) {
          mindist = d;
          if (cpt != NULL) {
              cpt[0] = tmp_pt[0];
              cpt[1] = tmp_pt[1];
              cpt[2] = tmp_pt[2];
          }
          if (ctn != NULL) {
              *ctn = tn;
          }
      }
      if (mindist == 0.0) {
          break;
      }
    }
  }

  //Delete_Local_Neuroseg(locseg);

  return mindist;
}

double Swc_Tree_Point_Dist_N(Swc_Tree *tree, double x, double y, double z,
    Swc_Tree_Node **ctn)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);

  Swc_Tree_Node *tn = NULL;
  
  double mindist = Infinity;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    double d = Geo3d_Dist(x, y, z, Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),
        Swc_Tree_Node_Z(tn)) - Swc_Tree_Node_Radius(tn);
    if (mindist >= 0) {
      if (mindist > d) {
        mindist = d;
        if (ctn != NULL) {
          *ctn = tn;
        }
      } 
    } else if (d < 0) {
      if (mindist / Swc_Tree_Node_Radius(*ctn) > d / Swc_Tree_Node_Radius(tn)) {
        mindist = d;
        if (ctn != NULL) {
          *ctn = tn;
        }
      } 
    }
  }

  return mindist;
}

double Swc_Tree_Point_Dist_N_Z(Swc_Tree *tree, double x, double y, double z,
    double z_scale, Swc_Tree_Node **ctn)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);

  Swc_Tree_Node *tn = NULL;
  
  double mindist = Infinity;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    double d = Geo3d_Dist(x, y, z * z_scale, 
        Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),
        Swc_Tree_Node_Z(tn) * z_scale) - Swc_Tree_Node_Radius(tn);
    if (mindist >= 0) {
      if (mindist > d) {
        mindist = d;
        if (ctn != NULL) {
          *ctn = tn;
        }
      } 
    } else if (d < 0) {
      if (mindist / Swc_Tree_Node_Radius(*ctn) > d / Swc_Tree_Node_Radius(tn)) {
        mindist = d;
        if (ctn != NULL) {
          *ctn = tn;
        }
      } 
    }
  }

  return mindist;
}

void Swc_Tree_Leaf_Shrink(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  Swc_Tree_Node *next;
  while (tn != NULL) {
    next = tn->next;
    Swc_Tree_Node_Radial_Shrink(tn);
    tn = next;
  }
}

void Swc_Tree_Connect_Node(Swc_Tree *tree, Swc_Tree_Node *tn)
{
  TZ_ASSERT(tree != NULL, "Null pointer.");
  TZ_ASSERT(tn != NULL, "Null pointer.");

  double eps = 0.05;

  double pt[3];

  Swc_Tree_Node *tmp_tn;
  double dist = Swc_Tree_Point_Dist(tree, Swc_Tree_Node_Const_Data(tn)->x,
				    Swc_Tree_Node_Const_Data(tn)->y,
				    Swc_Tree_Node_Const_Data(tn)->z, pt,
				    &tmp_tn);

  if (tmp_tn == NULL) {
    Swc_Tree_Node_Set_Parent(tn, tree->root);
    return;
  }

  if (Swc_Tree_Node_Is_Root(tmp_tn) == TRUE) {
    Swc_Tree_Node_Set_Parent(tn, tmp_tn);
    return;
  }

  double lambda;
  double tmp_pos[3];

  if (dist <= eps) {
    Swc_Tree_Node_Pos(tn, tmp_pos);
  } else {
    Swc_Tree_Node *new_tn = New_Swc_Tree_Node();
    Swc_Tree_Node_Data(new_tn)->x = pt[0];
    Swc_Tree_Node_Data(new_tn)->y = pt[1];
    Swc_Tree_Node_Data(new_tn)->z = pt[2];
    Swc_Tree_Node_Data(new_tn)->d = Swc_Tree_Node_Data(tn)->d;
    
    Swc_Tree_Node_Set_Parent(tn, new_tn);
    
    Swc_Tree_Node_Pos(new_tn, tmp_pos);
  }

  double start[3], end[3];
  Swc_Tree_Node_Pos(tmp_tn, end);
  Swc_Tree_Node_Pos(tmp_tn->parent, start);

  Geo3d_Point_Lineseg_Dist(tmp_pos, start, end, &lambda);
  if (lambda - eps < 0.0) {
    Swc_Tree_Node_Set_Parent(tn, tmp_tn->parent);
  } else if (lambda + eps > 1.0) {
    Swc_Tree_Node_Set_Parent(tn, tmp_tn);
  } else {
    Swc_Tree_Node *new_tn = Swc_Tree_Node_Add_Break(tmp_tn, lambda);
    Swc_Tree_Node_Set_Parent(tn, new_tn);
  }
}

Swc_Tree_Node* Swc_Tree_Connect_Branch(Swc_Tree *tree, Swc_Tree_Node *start_tn)
{
  TZ_ASSERT(tree != NULL, "Null pointer.");
  TZ_ASSERT(start_tn != NULL, "Null pointer.");

  Swc_Tree_Node *end_tn = start_tn;
  while (end_tn->first_child != NULL) {
    end_tn = end_tn->first_child;
  }
  
  if (start_tn == end_tn) {
    Swc_Tree_Connect_Node(tree, start_tn);
    return start_tn;
  }

  double eps = 0.05;

  double pt[3];
  double pt2[3];

  Swc_Tree_Node *tmp_tn1;
  Swc_Tree_Node *tmp_tn2;
  double dist1 = Swc_Tree_Point_Dist(tree, 
      Swc_Tree_Node_Const_Data(start_tn)->x,
      Swc_Tree_Node_Const_Data(start_tn)->y,
      Swc_Tree_Node_Const_Data(start_tn)->z, pt,
      &tmp_tn1);

  double dist2 = Swc_Tree_Point_Dist(tree, Swc_Tree_Node_Const_Data(end_tn)->x,
				    Swc_Tree_Node_Const_Data(end_tn)->y,
				    Swc_Tree_Node_Const_Data(end_tn)->z, pt2,
				    &tmp_tn2);

  Swc_Tree_Node *tmp_tn = tmp_tn1;
  Swc_Tree_Node *tn = start_tn;
  double dist = dist1;
  if (dist2 < dist1) {
    tmp_tn = tmp_tn2;
    tn = end_tn;
    Swc_Tree_Node_Set_Root(tn);
    dist = dist2;
    pt[0] = pt2[0];
    pt[1] = pt2[1];
    pt[2] = pt2[2];
  }
  
#ifdef _DEBUG_2
  printf("dist %g\n", dist);
#endif

  if (tmp_tn == NULL) {
    Swc_Tree_Node_Set_Parent(tn, tree->root);
    return tn;
  }

  if (Swc_Tree_Node_Is_Root(tmp_tn) == TRUE) {
    Swc_Tree_Node_Set_Parent(tn, tmp_tn);
    return tn;
  }

  double lambda;
  double tmp_pos[3];

  if (dist <= eps) {
    Swc_Tree_Node_Pos(tn, tmp_pos);
  } else {
    /*
    Swc_Tree_Node *new_tn = New_Swc_Tree_Node();
    Swc_Tree_Node_Data(new_tn)->x = pt[0];
    Swc_Tree_Node_Data(new_tn)->y = pt[1];
    Swc_Tree_Node_Data(new_tn)->z = pt[2];
    Swc_Tree_Node_Data(new_tn)->d = Swc_Tree_Node_Data(tn)->d;
    
    Swc_Tree_Node_Set_Parent(tn, new_tn);
    
    Swc_Tree_Node_Pos(new_tn, tmp_pos);
    */
    tmp_pos[0] = pt[0];
    tmp_pos[1] = pt[1];
    tmp_pos[2] = pt[2];
  }

  double start[3], end[3];
  Swc_Tree_Node_Pos(tmp_tn, end);
  Swc_Tree_Node_Pos(tmp_tn->parent, start);

  Geo3d_Point_Lineseg_Dist(tmp_pos, start, end, &lambda);
  if (lambda - eps < 0.0) {
    Swc_Tree_Node_Set_Parent(tn, tmp_tn->parent);
  } else if (lambda + eps > 1.0) {
    Swc_Tree_Node_Set_Parent(tn, tmp_tn);
  } else {
    Swc_Tree_Node *new_tn = Swc_Tree_Node_Add_Break(tmp_tn, lambda);
    Swc_Tree_Node_Set_Parent(tn, new_tn);
  }

  return tn;
}

void Swc_Tree_Tune_Fork(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  while (tn != NULL) {
    Swc_Tree_Node_Tune_Fork(tn);
    tn = tn->next;
  }
}

void Swc_Tree_Remove_Spur(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Iterator_Start(tree, -1, FALSE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  Swc_Tree_Node *buffer;
  while (tn != NULL) {
    buffer = tn->next;
    if (Swc_Tree_Node_Is_Spur(tn) == TRUE) {
      Swc_Tree_Node_Merge_To_Parent(tn);
    } 
    tn = buffer;
  }
}

Swc_Tree_Node* Swc_Tree_Find_Shortest_Terminal_Branch(Swc_Tree *tree,
						      double *blen)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
 
  double min_len = Infinity;
  Swc_Tree_Node *target = NULL;
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Leaf(tn)) {
      double length = Swc_Tree_Node_Length(tn);
      Swc_Tree_Node *tmp_tn = tn->parent;
      Swc_Tree_Node *tmp_target = tn;
      while (Swc_Tree_Node_Is_Continuation(tmp_tn)) {
	length += Swc_Tree_Node_Length(tmp_tn);
	tmp_target = tmp_tn;
	tmp_tn = tmp_tn->parent;
      }
      if (length < min_len) {
	min_len = length;
	target = tmp_target;
      }
    }
  }

  if (blen != NULL) {
    *blen = min_len;
  }

  return target;
}

#if 0
static Swc_Tree_Node* swc_tree_find_shortest_terminal_branch_i(Swc_Tree *tree,
							       double *blen)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
 
  double min_len = Infinity;
  Swc_Tree_Node *target = NULL;
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Leaf(tn)) {
      double length = Swc_Tree_Node_Length(tn);
      Swc_Tree_Node *tmp_tn = tn->parent;
      Swc_Tree_Node *tmp_target = tn;
      while (Swc_Tree_Node_Is_Continuation(tmp_tn)) {
	length += Swc_Tree_Node_Length(tmp_tn);
	tmp_target = tmp_tn;
	tmp_tn = tmp_tn->parent;
      }
      if (Swc_Tree_Node_Is_Root(tmp_tn->parent) == FALSE) {
	if (length < min_len) {
	  min_len = length;
	  target = tmp_target;
	}
      }
    }
  }

  if (blen != NULL) {
    *blen = min_len;
  }

  return target;
}
#endif

void Swc_Tree_Remove_Terminal_Branch(Swc_Tree *tree, double thre)
{
  double min_len;
  Swc_Tree_Node *tn = Swc_Tree_Find_Shortest_Terminal_Branch(tree, &min_len);
  while (min_len < thre) {
    /*
    if (Swc_Tree_Node_Is_Root(tn->parent) == FALSE) {
      Swc_Tree_Node_Kill_Subtree(tn);
    } else {
      break;
    }

    tn = swc_tree_find_shortest_terminal_branch_i(tree, &min_len);
    */

    Swc_Tree_Node_Kill_Subtree(tn);
    tn = Swc_Tree_Find_Shortest_Terminal_Branch(tree, &min_len);

#ifdef _DEBUG_2
    printf("%g\n", min_len);
#endif

  }
}

void Swc_Tree_Clean_Root(Swc_Tree *tree)
{
  Swc_Tree_Node *root = NULL;
  Swc_Tree tmp_tree;

  root = tree->root;
  Swc_Tree_Node *child = root->first_child;
  int max_size = Swc_Tree_Node_Fsize(child);
  Swc_Tree_Node *next = child->next_sibling;
  Swc_Tree_Node *remain = child;
  
  while (next != NULL) {
    int size = Swc_Tree_Node_Fsize(next);
    if (max_size < size) {
      max_size = size;
      remain = next;
      tmp_tree.root = child;
    } else {
      remain = child;
      tmp_tree.root = next;
    }
    
    Swc_Tree_Node_Detach_Parent(tmp_tree.root);
    Clean_Swc_Tree(&tmp_tree);
    child = remain;
    next = remain->next_sibling;
  }
  
  if (Swc_Tree_Node_Is_Virtual(tree->root) == TRUE) {
    TZ_ASSERT(tree->root->first_child->next_sibling == NULL, "bug found");
    tree->root = tree->root->first_child;
    Swc_Tree_Node_Detach_Parent(tree->root);
    Kill_Swc_Tree_Node(tree->root->parent);
    Swc_Tree_Clean_Root(tree);
  }
}

void Swc_Tree_Merge_Close_Node(Swc_Tree *tree, double thre)
{
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  Swc_Tree_Node *next;
  while (tn != NULL) {
    next = tn->next;
    if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
      if (Swc_Tree_Node_Length(tn) < thre) {
        Swc_Tree_Node_Merge_To_Parent(tn);
      } else if (Swc_Tree_Node_Is_Last_Child(tn)) {
        if (tn->parent->first_child != tn) {
          Swc_Tree_Node *next_child = tn->parent->first_child;
          while (next_child->next_sibling != NULL) {
            if (Swc_Tree_Node_Dist(next_child, next_child->next_sibling) 
                < thre) {
              Swc_Tree_Node_Merge_Sibling(next_child, 
                  next_child->next_sibling);
            } else {
              next_child = next_child->next_sibling;
            }
          }
        }
      }
    }
    tn = next;
  }  
}

#if 0
Swc_Tree_Node* Swc_Tree_Closest_Node(Swc_Tree *tree, const double *pos)
{
  if (tree == NULL) {
    return NULL;
  }

  if (tree->root == NULL) {
    return NULL;
  }

  if ((tree->root->first_child == NULL) && 
      Swc_Tree_Node_Is_Virtual(tree->root)) {
    return NULL;
  }

  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Node *close_tn = NULL;
  double mindist = -1.0;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      double tn_pos[3];
      Swc_Tree_Node_Pos(tn, tn_pos);
      if (mindist < 0) {
	close_tn = tn;
	mindist = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2], 
				 tn_pos[0], tn_pos[1], tn_pos[2]);
      } else {
	double d = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2], 
				  tn_pos[0], tn_pos[1], tn_pos[2]);
	if (d < mindist) {
	  mindist = d;
	  close_tn = tn;
	}
      }
    }
  }

  return close_tn;
}
#endif

Swc_Tree_Node* Swc_Tree_Closest_Node(Swc_Tree *tree, const double *pos)
{
  return Swc_Tree_Closest_Node_I(tree, pos, SWC_TREE_ITERATOR_DEPTH_FIRST);
}

Swc_Tree_Node* Swc_Tree_Closest_Node_I(Swc_Tree *tree, const double *pos, 
    int iterator_option)
{
  if (tree == NULL) {
    return NULL;
  }

  if (tree->root == NULL) {
    return NULL;
  }

  if ((tree->root->first_child == NULL) && 
      Swc_Tree_Node_Is_Virtual(tree->root)) {
    return NULL;
  }

  Swc_Tree_Iterator_Start(tree, iterator_option, FALSE);
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Node *close_tn = NULL;
  double mindist = -1.0;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      double tn_pos[3];
      Swc_Tree_Node_Pos(tn, tn_pos);
      if (mindist < 0) {
	close_tn = tn;
	mindist = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2], 
				 tn_pos[0], tn_pos[1], tn_pos[2]);
      } else {
	double d = Geo3d_Dist_Sqr(pos[0], pos[1], pos[2], 
				  tn_pos[0], tn_pos[1], tn_pos[2]);
	if (d < mindist) {
	  mindist = d;
	  close_tn = tn;
	}
      }
    }
  }

  return close_tn;
}

void Swc_Tree_Bound_Box(Swc_Tree *tree, double *corner)
{
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = NULL;
  BOOL first = TRUE;
  int i;
  for (i = 0; i < 6; i++) {
    corner[i] = 0.0;
  }
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      double tn_pos[3];
      Swc_Tree_Node_Pos(tn, tn_pos);
      double r =  Swc_Tree_Node_Radius(tn);
      if (first) {
	corner[0] = tn_pos[0] - r;
	corner[1] = tn_pos[1] - r;
	corner[2] = tn_pos[2] - r;
	corner[3] = tn_pos[0] + r;
	corner[4] = tn_pos[1] + r;
	corner[5] = tn_pos[2] + r;
	first = FALSE;
      } else {
	for (i = 0; i < 3; i++) {
	  if (corner[i] > tn_pos[i] - r) {
	    corner[i] = tn_pos[i] - r;
	  }
	  if (corner[3+i] < tn_pos[i] + r) {
	    corner[3+i] = tn_pos[i] + r;
	  }
	}
      }
    }
  }
}

void Swc_Tree_Tune_Branch(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Tune_Branch(tn);
  }
}

Swc_Tree_Node* Swc_Tree_Largest_Node(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Node *max_tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (max_tn == NULL) {
      max_tn = tn;
    } else {
      if (Swc_Tree_Node_Data(tn)->d > Swc_Tree_Node_Data(max_tn)->d) {
	max_tn = tn;
      }
    }
  }

  return max_tn;
}

Swc_Tree_Node* Swc_Tree_Outmost_Leaf(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, TRUE);
  double *length = Swc_Tree_Accm_Length(tree, NULL);
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_NO_UPDATE, FALSE);
  
  double max_length = 0.0;
  Swc_Tree_Node *leaf = NULL;
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Leaf(tn)) {
      if (length[tn->index] > max_length) {
	max_length = length[tn->index];
	leaf = tn;
      }
    }
  }
  
  free(length);

  return leaf;
}

void Swc_Tree_Reduce(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = NULL;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
      tn->weight = Swc_Tree_Node_Length(tn);
    }
  }

  Swc_Tree_Iterator_Start(tree, 0, FALSE);
  tn = Swc_Tree_Next(tree);
  Swc_Tree_Node *buffer_tn;
  while (tn != NULL) {
    buffer_tn = tn->next;
    if (Swc_Tree_Node_Is_Continuation(tn)) {
      tn->first_child->weight += tn->weight;
      Swc_Tree_Node *parent = tn->parent;
      Swc_Tree_Node_Detach_Parent(tn);
      Swc_Tree_Node_Set_Parent(tn->first_child, parent);
      Kill_Swc_Tree_Node(tn);
    }
    tn = buffer_tn;
  }
}

BOOL Swc_Tree_Label_Branch(Swc_Tree *tree, int label,
			   const double *pos, double thre)
{
  Swc_Tree_Node *tn = Swc_Tree_Closest_Node(tree, pos);

  if (Geo3d_Dist(Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
		 Swc_Tree_Node_Data(tn)->z, pos[0], pos[1], pos[2]) <= thre) {
    Swc_Tree_Node_Label_Branch(tn, label);
    return TRUE;
  }

  return FALSE;
}

int Swc_Tree_Label_Branch_All(Swc_Tree *tree)
{
  if (tree == NULL) {
    return 0;
  }

  if (tree->root == NULL) {
    return 0;
  }

  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  int label = 1;
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Node_Data(tree->root)->label = label;
  //Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  //Swc_Tree_Node_Data(tn)->label = label;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (tn->parent != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
        if ((Swc_Tree_Node_Is_Branch_Point_S(tn->parent) == FALSE) &&
            (Swc_Tree_Node_Is_Root(tn->parent) == FALSE)) {
          Swc_Tree_Node_Data(tn)->label = Swc_Tree_Node_Data(tn->parent)->label;
        } else {
          /* new branch */
          Swc_Tree_Node_Data(tn)->label = ++label;
        }
      }
    }
  }

  return label;
}

int Swc_Tree_Label_Branch_Order(Swc_Tree *tree)
{
  if (tree == NULL) {
    return 0;
  }

  if (tree->root == NULL) {
    return 0;
  }

  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  int label = 0;
  Swc_Tree_Node *tn = NULL;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Is_Root(tn)) {
        Swc_Tree_Node_Set_Label(tn, 1);
      } else {
        if (Swc_Tree_Node_Is_Branch_Point(tn->parent) &&
            !Swc_Tree_Node_Is_Root(tn->parent)) {
          Swc_Tree_Node_Set_Label(tn, Swc_Tree_Node_Label(tn->parent) + 1);
        } else {
          Swc_Tree_Node_Set_Label(tn, Swc_Tree_Node_Label(tn->parent));
        }
      }

      label = imax2(label, Swc_Tree_Node_Label(tn));
    }
  }

  return label;
}

void Swc_Tree_Svg_Fprint(Swc_Tree *tree, double scale, FILE *fp)
{
  tree->root->weight = 0.0;
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  
  Swc_Tree_Node *node = Swc_Tree_Next(tree);
  while ((node = Swc_Tree_Next(tree)) != NULL) {
    node->weight = Swc_Tree_Node_Dist(node, node->parent);
  }

  Swc_Tree_Reduce(tree);

  int count = Swc_Tree_Iterator_Start(tree, 1, TRUE);
  
  /* alloc <decided> */
  uint8 *decided = u8array_calloc(count);
  
  /* alloc <isleaf> */
  uint8 *isleaf = u8array_calloc(count);

  /* alloc <x> */
  double *x = darray_calloc(count);

  /* alloc <y> */
  double *y = darray_calloc(count);

  x[0] = 0.0;
  
  Swc_Tree_Node *root = tree->root;
  
  node = root;
  int leaf_count = 0;
  double xmax = x[0];

  while ((node = node->next) != NULL) {
    x[node->index] = scale * (node->weight + x[node->parent->index]);
    xmax = dmax2(x[node->index], xmax);
    if (node->first_child == NULL) {
      isleaf[node->index] = 1;
      leaf_count++;
    }
  }

  double dm = 10.0;
  if (leaf_count > 1) {
    dm = dmax2(dm, xmax / 2.0 / (leaf_count - 1));
  }

  node = root;
  x[0] += dm;
  double cur_y = dm;
  //double ymax = 0.0;

  while ((node = node->next) != NULL) {
    x[node->index] += dm;
    if (isleaf[node->index]) {
      y[node->index] = cur_y;
      decided[node->index] = 1;
      cur_y += dm;
    }
  }

  //ymax = cur_y;
  xmax += dm * 2;
  
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Iterator_Start(tree, -1, FALSE);
  while ((node = Swc_Tree_Next(tree)) != NULL) {
    if (decided[node->index] == 0) {
      Swc_Tree_Node *child = node->first_child;
      int n = 0;
      cur_y = 0.0;
      while (child != NULL) {
	cur_y += y[child->index];
	n++;
	child = child->next_sibling;
      }
      y[node->index] = cur_y / n;
      decided[node->index] = 1;
    }
  }

  Swc_Tree_Iterator_Start(tree, 2, FALSE);

  int i;
  for (i = 0; i < count; i++) {
    fprintf(fp, "<circle cx=\"%g\" cy=\"%g\" r=\"1\" stroke=\"red\" "
	    "stroke-width=\"1\"/>\n", x[i], y[i]);
  }

  Swc_Tree_Node *start_tn, *end_tn;
  start_tn = NULL;
  end_tn = NULL;
  double y1, y2;
  double x1, x2;
  while ((node = Swc_Tree_Next(tree)) != NULL) {
    if (node->weight > 0.0) {
      y1 = y2 = y[node->index];
      x2 = x[node->index];
      x1 = x2 - scale * node->weight;
      fprintf(fp, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2 = \"%g\" "
	      "stroke=\"%s\" stroke-width=\"2\"/>\n", x1, y1, x2, y2,
	      Swc_Node_Color_Code(Swc_Tree_Node_Data(node)));
    }

    if (Swc_Tree_Node_Is_Sibling(node, node->next)) {
      if (start_tn == NULL) {
	start_tn = node;
	end_tn = NULL;
      }
    } else if (start_tn != NULL) {
      end_tn = node;
    } else if ((Swc_Tree_Node_Is_Root(node) == FALSE) && 
	       (Swc_Tree_Node_Is_Root(node->parent) == FALSE)) {
      Print_Swc_Tree_Node(node);
      TZ_ERROR(ERROR_DATA_VALUE);
    }
    
    if ((start_tn != NULL) && (end_tn != NULL)) {
      x1 = x2 = x[start_tn->index] - scale * start_tn->weight;
      y1 = y[start_tn->index];
      y2 = y[end_tn->index];
      fprintf(fp, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2 = \"%g\" "
	      "stroke=\"black\" stroke-width=\"2\"/>\n", x1, y1, x2, y2);
      start_tn = NULL;
      end_tn = NULL;
    }
  }

  /* free <decided> */
  free(decided);
  
  /* free <isleaf> */
  free(isleaf);

  /* free <x> */
  free(x);

  /* free <y> */
  free(y);
}

void Swc_Tree_To_Svg_File_W(Swc_Tree *tree, const char *file,
    Swc_Tree_Svg_Workspace *ws)
{
  tree->root->weight = 0.0;
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  
  /* Calculating weight (corresponding to length in svg) */
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  Swc_Tree_Node_Set_Weight(tn, 0.0);
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    /* Two weight options: length and surface area */
    switch (ws->weight_option) {
      case SWC_SVG_LENGTH_WEIGHT:
        Swc_Tree_Node_Set_Weight(tn, Swc_Tree_Node_Dist(tn, tn->parent));
        break;
      case SWC_SVG_SURFAREA_WEIGHT:
        Swc_Tree_Node_Set_Weight(tn, Swc_Tree_Node_Surface_Area(tn));
        break;
      default:
        TZ_ERROR(ERROR_DATA_TYPE);
        break;
    }
  }

  /* Total branch number of the tree. */
  int branch_number = Swc_Tree_Label_Branch_All(tree) + 1;

#ifdef _DEBUG_
  printf("%d\n", branch_number);
#endif

  /* alloc <reduced_tree> */
  Swc_Tree *reduced_tree = Copy_Swc_Tree(tree);
  Swc_Tree_Canonicalize(reduced_tree);
  Swc_Tree_Reduce(reduced_tree);

  /* Resort labels for root-leaf */
  /*
  if (Swc_Tree_Node_Is_Regular(reduced_tree->root)) {
    if (Swc_Tree_Node_Child_Number(reduced_tree->root) == 1) {
      Swc_Tree_Iterator_Start(reduced_tree, 1, FALSE);
      Swc_Tree_Node *tn = Swc_Tree_Next(reduced_tree);
      while ((tn = Swc_Tree_Next(reduced_tree)) != NULL) {
        Swc_Tree_Node_Data(tn)->label++;
      }
      branch_number++;
    }
  }
  */
  
      
  Swc_Tree_Resort_Main_Trunk(reduced_tree);

  /* alloc <x> */
  double *x = darray_malloc(branch_number);
  /* alloc <y> */
  double *y = darray_malloc(branch_number);
  /* alloc <length> */
  double *length = darray_malloc(branch_number);

  /* <dm>, which will be calcualted later, 
   * is the interval between two adjacent sibling lines.*/
  double dm = 0.0;

  /* Calculate 2D layout postions for the branch points and leaves. */
  double xscale = 1.0; /* scaling of x axis in svg space */
  swc_tree_dendrogram_position(reduced_tree, ws->max_vx, ws->max_vy,
      x, y, &dm, &xscale);

#ifdef _DEBUG_2
  darray_print2(x, branch_number, 1);
  darray_print2(y, branch_number, 1);
#endif

  /* assign length array */
  Swc_Tree_Iterator_Start(reduced_tree, 1, FALSE);
  length[0] = 0.0;
  while ((tn = Swc_Tree_Next(reduced_tree)) != NULL) {
    length[Swc_Tree_Node_Data(tn)->label] = Swc_Tree_Node_Weight(tn);
#ifdef _DEBUG_2
    printf("length[%d]: %g\n", Swc_Tree_Node_Data(tn)->label, length[Swc_Tree_Node_Data(tn)->label]);
#endif
  }

  double xmax = ws->max_vx;
  double ymax = ws->max_vy;

  int viewbox[4];
  viewbox[0] = 0;
  viewbox[1] = 0;
  viewbox[2] = iround(xmax);
  viewbox[3] = iround(ymax);
   
  /* expand the view box based on what to show */
  if (ws->color_code != SWC_SVG_COLOR_CODE_NONE) {
    viewbox[3] += 40.0 + dm;
  }
  if (ws->showing_length || ws->showing_count) {
    viewbox[2] += 40.0;
  }

  /* Start writing the svg file. */
  FILE *fp = GUARDED_FOPEN(file, "w");
  Svg_Header_Fprint(fp, NULL);
  Svg_Begin_Fprint(fp, 100.0, 100.0, viewbox, NULL);

  /*
  double corner[6];
  Swc_Tree_Bound_Box(tree, corner);
  double z_min = corner[2];
  double z_max = corner[5];
  */

  double z_offset = 0.0;
  double z_scale = 0.0;

  if (ws->color_code != SWC_SVG_COLOR_CODE_NONE) {
    double z_min, z_max;
    Swc_Tree_Feature_Range(tree, &z_min, &z_max);

    /*** Start: Draw color bar ***/
    Geo3d_Point_Array *colorbar = Make_Geo3d_Point_Array(128);
    double colorbar_start = xmax / 3.0;
    double colorbar_step = xmax / 3.0 / 128.0;
    int i;
    for (i = 0; i < 64; i++) {
      if (i == 0) {
        colorbar->array[0][0] = colorbar_start;
        colorbar->array[1][0] = colorbar_start + colorbar_step;
      } else {
        colorbar->array[i*2][0] = colorbar->array[i*2-1][0] + colorbar_step;
        colorbar->array[i*2+1][0] =  colorbar->array[i*2][0] + colorbar_step;
      }
      colorbar->array[i*2][1] = ymax + dm;
      colorbar->array[i*2+1][1] = ymax + dm;
      colorbar->array[i*2][2] = (double) i / 63.0;
      colorbar->array[i*2+1][2] = (double) i / 63.0;
    }

    Svg_Polygonal_Chain_Fprint(fp, colorbar, "jet", 5);

    if (z_min != z_max) {
      z_offset = -z_min / (z_max - z_min);
      z_scale = 1.0 / (z_max - z_min);
    }

    char colorbar_text[100];
    sprintf(colorbar_text, "%.2f", z_min);
    Svg_Text_Fprint(fp, colorbar_text, colorbar_start - colorbar_step, 
        ymax + dm + 20.0, "k", 12);
    sprintf(colorbar_text, "%.2f", z_max);
    Svg_Text_Fprint(fp, colorbar_text, colorbar_start + colorbar_step * 127.0, 
        ymax + dm + 20.0, "k", 12);
    /*** End: Draw color bar ***/
  }

  Swc_Tree_Node *start_tn, *end_tn;
  start_tn = NULL;
  end_tn = NULL;
  double y1, y2;
  double x1, x2;
  Swc_Tree_Iterator_Start(reduced_tree, SWC_TREE_ITERATOR_BREADTH_FIRST, FALSE);
  Swc_Tree_Node *node = NULL;
  while ((node = Swc_Tree_Next(reduced_tree)) != NULL) {
    if (node->weight > 0.0) {
      y1 = y2 = y[Swc_Tree_Node_Data(node)->label];
      x2 = x[Swc_Tree_Node_Data(node)->label];
      x1 = x2 - node->weight * xscale;
      if (ws->color_code != SWC_SVG_COLOR_CODE_NONE) {
        Geo3d_Point_Array *pa = Swc_Tree_Branch_2d_Layout(tree,
            Swc_Tree_Node_Id(node->parent), Swc_Tree_Node_Id(node), x1, y1, x2,
            y2, z_offset, z_scale);
        Svg_Polygonal_Chain_Fprint(fp, pa, "jet", 2); 
        Kill_Geo3d_Point_Array(pa);
      } else {
        fprintf(fp, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2 = \"%g\" "
            "stroke=\"%s\" stroke-width=\"2\"/>\n", x1, y1, x2, y2,
            Swc_Node_Color_Code(Swc_Tree_Node_Data(node)));
      }
    }

    if (Swc_Tree_Node_Is_Sibling(node, node->next)) {
      if (start_tn == NULL) {
        start_tn = node;
        end_tn = NULL;
      }
    } else if (start_tn != NULL) {
      end_tn = node;
    } else if ((Swc_Tree_Node_Is_Root(node) == FALSE) && 
        (Swc_Tree_Node_Is_Root(node->parent) == FALSE)) {
      Print_Swc_Tree_Node(node);
      TZ_ERROR(ERROR_DATA_VALUE);
    }

    if ((start_tn != NULL) && (end_tn != NULL)) {
      x1 = x2 = x[Swc_Tree_Node_Data(start_tn)->label] - start_tn->weight *
        xscale;
      y1 = y[Swc_Tree_Node_Data(start_tn)->label];
      y2 = y[Swc_Tree_Node_Data(end_tn)->label];
      fprintf(fp, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2 = \"%g\" "
          "stroke=\"black\" stroke-width=\"1\"/>\n", x1, y1, x2, y2);
      start_tn = NULL;
      end_tn = NULL;
    }
  }
  /******************************/

  if (ws->showing_length && !ws->showing_count) {
    char numstr[100];
    int i;
    for (i = 0; i < branch_number; i++) {
      sprintf(numstr, "%d", iround(length[i]));
      Svg_Text_Fprint(fp, numstr, x[i] + dm / 6.0, y[i] + dm / 4.0, NULL,
	  11);
    }
  }

  static char *Puncta_Color_String[] = { "dimgrey", "limegreen" };

  if (ws->puncta != NULL) {
    if (ws->puncta->size > 0) {
      /* alloc <branch_id> */
      int *branch_id = iarray_malloc(ws->puncta->size);

      /* alloc <lambda> */
      double *lambda = darray_malloc(ws->puncta->size);

      Swc_Tree_Node *ctn = NULL;

      /* alloc <puncta_count> */
      int *puncta_count = iarray_malloc(branch_number);

      int i;

      for (i = 0; i < branch_number; i++) {
        puncta_count[i] = 0;
      }

#ifdef _DEBUG_2
      Print_Swc_Tree(tree);
#endif

      double pt[3];
      for (i = 0; i < ws->puncta->size; i++) {
        Swc_Tree_Point_Dist(tree, ws->puncta->points[i][0], 
            ws->puncta->points[i][1], ws->puncta->points[i][2], pt, &ctn);
        branch_id[i] = Swc_Tree_Node_Data(ctn)->label;

        if (ws->shuffling == FALSE) {
          int tmp_branch_id = branch_id[i];
          if (ws->on_root != NULL) {
            if (ws->on_root[i]) {
              tmp_branch_id = 1;
            }
          }
          puncta_count[tmp_branch_id]++;
        }

#ifdef _DEBUG_2
        iarray_print2(puncta_count, branch_number, 1);
#endif

        double cw = 0.0;
        double start[3];
        double end[3];
        Set_Coordinate_3d(start, Swc_Tree_Node_Data(ctn)->x, 
            Swc_Tree_Node_Data(ctn)->y, Swc_Tree_Node_Data(ctn)->z);
        Set_Coordinate_3d(end, Swc_Tree_Node_Data(ctn->parent)->x, 
            Swc_Tree_Node_Data(ctn->parent)->y, 
            Swc_Tree_Node_Data(ctn->parent)->z);
        Geo3d_Point_Lineseg_Dist(pt, start, end, &cw);
        if (cw < 0.0) {
          cw = 0.0;
        } else if (cw > 1.0) {
          cw = 1.0;
        }
        cw *= Coordinate_3d_Distance(start, end);

        lambda[i] = (Swc_Tree_Node_Backtrace_Weight(ctn, 1) - cw) / 
          length[branch_id[i]];
      }

      if (ws->shuffling) {
        Random_Seed(time(NULL) - getpid());
      }

      double *cum_length = darray_copy(length, branch_number);
      darray_cumsum(cum_length, branch_number);

      /* map puncta positions */
      for (i = 0; i < ws->puncta->size; i++) {
        int bidx = branch_id[i];
        double t = lambda[i];

        double puncta_pos[2];

        BOOL label_puncta = TRUE;
        if (ws->on_root != NULL) {
          if (ws->on_root[i]) {
            label_puncta = FALSE;
          }
        }

        /* Random shuffling for puncta not on root */
        if ((ws->shuffling) && (label_puncta == FALSE)) {
          label_puncta = TRUE;
          /* Randomly select a branch index */
          double r = Unifrnd() * cum_length[branch_number-1];
          int bidx = 0;
          while (bidx == 0) { /* Reshuffle if it gets root */
            for (bidx = 0; bidx < branch_number; bidx++) {
              if (r < cum_length[bidx]) {
                break;
              }
            }
          }
          puncta_count[bidx]++;
          t = Unifrnd();
        }
        puncta_pos[0] = x[bidx] + (t - 1.0) * length[bidx] * xscale;
        puncta_pos[1] = y[bidx];
#ifdef _DEBUG_2
        printf("%g\n", t);
        printf("%g, %g, %d\n", puncta_pos[0], puncta_pos[1], bidx);
#endif
        if (label_puncta) {
          if (ws->puncta_type == NULL) {
            Svg_Circle_Fprint(fp, puncta_pos[0], puncta_pos[1], 1.5, 1, 
                Puncta_Color_String[0]);
          } else {
            Svg_Circle_Fprint(fp, puncta_pos[0], puncta_pos[1], 1.5, 1, 
                Puncta_Color_String[ws->puncta_type[i]]);
          }
        }
      }

      if (ws->showing_count) {
#ifdef _DEBUG_
        printf("%g\n", dm);
#endif
        char numstr[100];
        double xoff = dmin2(20.0, dm) / 6.0;
        double yoff = dmin2(20.0, dm) / 4.0;
        for (i = 2; i < branch_number; i++) {
          if (ws->showing_length) {
            sprintf(numstr, "%d/%d", puncta_count[i], iround(length[i]));
          } else {
            sprintf(numstr, "%d", puncta_count[i]);
          }
          dm = 20.0;
          Svg_Text_Fprint(fp, numstr, x[i] + xoff, y[i] + yoff, NULL, 11);
        }
      }
      /* free <branch_id> */
      free(branch_id);

      /* free <lambda> */
      free(lambda);

      /* free <puncta_count> */
      free(puncta_count);  
    }
  }

  Svg_End_Fprint(fp);

  fclose(fp);

  /* free <x> */
  free(x);
  /* free <y> */
  free(y);
  /* free <length> */
  free(length);
  /* free <reduced_tree> */
  Kill_Swc_Tree(reduced_tree);

}

void 
Swc_Tree_To_Analysis_File(Swc_Tree *tree, Geo3d_Scalar_Field *puncta,
    const char *branch_file, const char *puncta_file)
{
  BOOL *on_root;
  GUARDED_MALLOC_ARRAY(on_root, puncta->size, BOOL);
  Swc_Tree_Identify_Puncta(tree, puncta, SWC_SOMA, on_root);

  Swc_Tree_Merge_Root_Group(tree, SWC_SOMA);
  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    Swc_Tree_Node_Merge_To_Parent(tree->root->first_child);
  } else {
    Swc_Tree_Node_To_Virtual(tree->root);
  }

  /* Total branch number of the tree. */
  int branch_number = Swc_Tree_Label_Branch_All(tree) + 1;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  FILE *fp = fopen(branch_file, "w");
  fprintf(fp, "# branch id, type, x, y, z, radius, blueness\n");
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
      /* duplcate branch point */
      fprintf(fp, "%d, %d, %g, %g, %g, %g, %g\n", Swc_Tree_Node_Label(tn), 
          Swc_Tree_Node_Type(tn),
          Swc_Tree_Node_X(tn->parent), Swc_Tree_Node_Y(tn->parent), 
          Swc_Tree_Node_Z(tn->parent), Swc_Tree_Node_Radius(tn->parent),
          Swc_Tree_Node_Feature(tn->parent));
    }

    fprintf(fp, "%d, %d, %g, %g, %g, %g, %g\n", Swc_Tree_Node_Label(tn),
        Swc_Tree_Node_Type(tn), Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),
        Swc_Tree_Node_Z(tn), Swc_Tree_Node_Radius(tn), 
        Swc_Tree_Node_Feature(tn));
  }
  fclose(fp);

  tree->root->weight = 0.0;
  Swc_Tree_Iterator_Start(tree, 2, FALSE);

  /* Calculating length */
  tn = Swc_Tree_Next(tree);
  Swc_Tree_Node_Set_Weight(tn, 0.0);
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Set_Weight(tn, Swc_Tree_Node_Dist(tn, tn->parent));
  }

  /* alloc <length> */
  double *length = darray_malloc(branch_number+1);

  /* alloc <reduced_tree> */
  Swc_Tree *reduced_tree = Copy_Swc_Tree(tree);
  Swc_Tree_Reduce(reduced_tree);

  /* assign length array */
  Swc_Tree_Iterator_Start(reduced_tree, 1, FALSE);
  length[0] = 0.0;
  while ((tn = Swc_Tree_Next(reduced_tree)) != NULL) {
    length[Swc_Tree_Node_Data(tn)->label] = Swc_Tree_Node_Weight(tn);
  }

  /* alloc <branch_id> */
  int *branch_id = iarray_malloc(puncta->size);

  /* alloc <lambda> */
  double *lambda = darray_malloc(puncta->size);

  Swc_Tree_Node *ctn = NULL;

  double pt[3];
  int i;
  fp = fopen(puncta_file, "w");
  fprintf(fp, "# puncta id, x, y, z, branch id, offset\n");
  for (i = 0; i < puncta->size; i++) {
    if (on_root[i] == FALSE) {
      Swc_Tree_Point_Dist(tree, puncta->points[i][0], puncta->points[i][1],
          puncta->points[i][2], pt, &ctn);
      branch_id[i] = Swc_Tree_Node_Data(ctn)->label;

      double cw = 0.0;
      double start[3];
      double end[3];
      Set_Coordinate_3d(start, Swc_Tree_Node_Data(ctn)->x, 
          Swc_Tree_Node_Data(ctn)->y, Swc_Tree_Node_Data(ctn)->z);
      Set_Coordinate_3d(end, Swc_Tree_Node_Data(ctn->parent)->x, 
          Swc_Tree_Node_Data(ctn->parent)->y, 
          Swc_Tree_Node_Data(ctn->parent)->z);
      Geo3d_Point_Lineseg_Dist(pt, start, end, &cw);
      if (cw < 0.0) {
        cw = 0.0;
      } else if (cw > 1.0) {
        cw = 1.0;
      }
      cw *= Coordinate_3d_Distance(start, end);

      lambda[i] = (Swc_Tree_Node_Backtrace_Weight(ctn, 1) - cw) / 
        length[branch_id[i]];

      fprintf(fp, "%d, %g, %g, %g, %d, %g\n", i, puncta->points[i][0], 
          puncta->points[i][1], puncta->points[i][2], branch_id[i], lambda[i]);
    }
  }
  fclose(fp);


  /* free <branch_id> */
  free(branch_id);

  /* free <lambda> */
  free(lambda);

  /* free <length> */
  free(length);
  /* free <reduced_tree> */
  Kill_Swc_Tree(reduced_tree);

}

void Swc_Tree_To_Svg_File(Swc_Tree *tree, int max_vx, int max_vy, 
    const char *path)
{
  Swc_Tree_Svg_Workspace *ws = New_Swc_Tree_Svg_Workspace();
  ws->max_vx = max_vx;
  ws->max_vy = max_vy;
  Swc_Tree_To_Svg_File_W(tree, path, ws);
  Kill_Swc_Tree_Svg_Workspace(ws);
}

void Swc_Tree_Merge(Swc_Tree *tree, Swc_Tree *prey)
{
  if ((tree == NULL) || (prey == NULL)) {
    return;
  }

  if (prey->root == NULL) {
    return;
  }

  if (tree->root == NULL) {
    tree->root = Make_Virtual_Swc_Tree_Node();
  }

  if (Swc_Tree_Node_Is_Virtual(tree->root) == FALSE) {
    Swc_Tree_Node *node = Make_Virtual_Swc_Tree_Node();
    Swc_Tree_Node_Set_Parent(tree->root, node);
    tree->root = node;
  }

  if (Swc_Tree_Node_Is_Virtual(prey->root) == FALSE) {
    Swc_Tree_Node_Add_Child(tree->root, prey->root);
    prey->root = NULL;
  } else {
    Swc_Tree_Node *child = prey->root->first_child;
    while (child != NULL) {
      Swc_Tree_Node_Add_Child(tree->root, child);
      child = prey->root->first_child;
    }
  }
}

void Swc_Tree_Merge_Node_As_Root(Swc_Tree *tree, Swc_Tree_Node *tn)
{
  if ((tree == NULL) || (tn == NULL)) {
    return;
  }

  if (tree->root == NULL) {
    tree->root = Make_Virtual_Swc_Tree_Node();
  }

  if (Swc_Tree_Node_Is_Virtual(tree->root) == FALSE) {
    Swc_Tree_Node *node = Make_Virtual_Swc_Tree_Node();
    Swc_Tree_Node_Set_Parent(tree->root, node);
    tree->root = node;
  }

  Swc_Tree_Node_Add_Child(tree->root, tn);
}

void Swc_Tree_Label_Main_Trunk(Swc_Tree *tree, int label)
{
  Swc_Tree_Node *start = tree->root;
  Swc_Tree_Node *tn = start->first_child;

  if (Swc_Tree_Node_Is_Root(tn)) {
    tn = tn->first_child;
  }

  while (Swc_Tree_Node_Is_Continuation(tn)) {
    tn = tn->first_child;
  }

  double r = Swc_Tree_Node_Average_Thickness(start, tn);
  Swc_Tree_Node *end = tn;

  Swc_Tree_Node *child = start->first_child->next_sibling;
  while (child != NULL) {
    tn = child;
    if (Swc_Tree_Node_Is_Root(tn)) {
      tn = tn->first_child;
    }    
    while (Swc_Tree_Node_Is_Continuation(tn)) {
      tn = tn->first_child;
    } 
    double tmp_r = Swc_Tree_Node_Average_Thickness(start, tn);
    if (tmp_r > r) {
      r = tmp_r;
      end = tn;
    }
    child = child->next_sibling;
  }

  while (end->first_child != NULL) {
    start = end;
    tn = start->first_child;
    while (Swc_Tree_Node_Is_Continuation(tn)) {
      tn = tn->first_child;
    }

    r = Swc_Tree_Node_Average_Thickness(start, tn);
    end = tn;

    child = start->first_child->next_sibling;

    while (child != NULL) {
      tn = child;

      while (Swc_Tree_Node_Is_Continuation(tn)) {
        tn = tn->first_child;
      } 
      double tmp_r = Swc_Tree_Node_Average_Thickness(start, tn);
      if (tmp_r > r) {
        r = tmp_r;
        end = tn;
      }
      child = child->next_sibling;
    }
  }

  tn = end;
  Swc_Tree_Node_Data(tn)->label = label;

  while (tn != tree->root) {
    tn = tn->parent;
    Swc_Tree_Node_Data(tn)->label = label;
  }
}

void Swc_Tree_Label_Main_Trunk_L(Swc_Tree *tree, int label, 
				 double length_lower_thre, 
				 double length_upper_thre)
{
  Swc_Tree_Iterator_Start(tree, 1, TRUE);

  /* alloc <length> */
  double *length = Swc_Tree_Accm_Length(tree, NULL);

  /* alloc <thickness> */
  double *thickness = Swc_Tree_Accm_Thickness(tree, NULL);

  double max_r = 0.0;
  Swc_Tree_Node *end = NULL;
  Swc_Tree_Node *tn = NULL;

  Swc_Tree_Iterator_Start(tree, 0, FALSE);
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Leaf(tn)) {
      if (length[tn->index] >= length_upper_thre) {
        if (max_r < thickness[tn->index] / length[tn->index]) {
          max_r = thickness[tn->index] / length[tn->index];
          end = tn;
        }
      }
    }
  }

  tn = end;
  if (tn != NULL) {
    max_r = 0.0;
    while (length[tn->index] >= length_lower_thre) {
      if (Swc_Tree_Node_Child_Number(tn) == 2) {
	double r = Swc_Tree_Node_Radius(tn->first_child) * 
	  Swc_Tree_Node_Radius(tn->first_child->next_sibling);
	if (max_r < r) {
	  max_r = r;
	  end = tn;
	}
      }
      tn = tn->parent;
    }
  }
  
  tn = end;
  max_r = Swc_Tree_Node_Radius(tn) * 2.0;

  if (tn != NULL) {
    while (!Swc_Tree_Node_Is_Root(tn)) {
      if (Swc_Tree_Node_Radius(tn) > max_r) {
	break;
      }
      Swc_Tree_Node_Data(tn)->label = label;
      tn = tn->parent;
    }
    if (Swc_Tree_Node_Radius(tn) <= max_r) {
      Swc_Tree_Node_Data(tn)->label = label;
    }
  }

  /* free <length> */
  free(length);

  /* free <thickness> */
  free(thickness);
}

Swc_Tree_Node* Swc_Tree_Thickest_Node(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Node *result = Swc_Tree_Next(tree);
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Radius(result) < Swc_Tree_Node_Radius(tn)) {
      result = tn;
    }
  }
  
  return result;
}

Swc_Tree_Node* Swc_Tree_Grow_Soma(Swc_Tree *tree, int label)
{
  /* Find the thickest node and set it as the root */
  Swc_Tree_Node *tn = Swc_Tree_Thickest_Node(tree);

  Swc_Tree_Node_Data(tn)->type = label;
  Swc_Tree_Node_Set_Label(tn, label);

  double r = Swc_Tree_Node_Radius(tn);
  Swc_Tree_Node_Set_Root(tn);
  Swc_Tree_Node *root = tn;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, FALSE);

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn->parent)) {
      if (Swc_Tree_Node_Label(tn->parent) == label) {
	if (Swc_Tree_Node_Radius(tn) * 3.0 >= r) {
	  Swc_Tree_Node_Set_Label(tn, label);
	  Swc_Tree_Node_Data(tn)->type = label;
	}
      }
    }
  }

  return root;
}

void Swc_Tree_Label_Soma(Swc_Tree *tree, int label)
{
  /* Find the thickest node and set it as the root */
  Swc_Tree_Node *tn = Swc_Tree_Thickest_Node(tree);

  Swc_Tree_Node_Data(tn)->type = label;
  Swc_Tree_Node_Set_Label(tn, label);

  double r = Swc_Tree_Node_Radius(tn);
  Swc_Tree_Node_Set_Root(tn);

  /* Check its neighbors and find two thickest ones */
  if (tn->first_child == NULL) {
    return;
  }

  Swc_Tree_Node *soma[2];

  soma[0] = NULL;
  soma[1] = NULL;

  Swc_Tree_Node *child = tn->first_child;
  while (child != NULL) {
    if (soma[0] == NULL) {
      soma[0] = child;
    } else {
      if (Swc_Tree_Node_Radius(soma[0]) < Swc_Tree_Node_Radius(child)) {
	soma[1] = soma[0];
	soma[0] = child;
      } else {
	if ((soma[1] == NULL) || 
	    (Swc_Tree_Node_Radius(soma[1]) < Swc_Tree_Node_Radius(child))) {
	  soma[1] = child;
	}
      }
    }

    child = child->next_sibling;
  }


  /* For each thick neighbor, walk downstream along the thickest path */
  int i = 0;
  for (i = 0; i < 2; i++) {
    while (soma[i] != NULL) {
      Swc_Tree_Node_Data(soma[i])->type = label;
      Swc_Tree_Node_Set_Label(soma[i], label);
      soma[i] = Swc_Tree_Node_Thickest_Child(soma[i]);
      if (soma[i] != NULL) {
	if (Swc_Tree_Node_Radius(soma[i]) * 3.0 < r) {
	  soma[i] = NULL;
	}
      }
    }
  }
}

void Swc_Tree_Label_Apical(Swc_Tree *tree, int label)
{
}

void Swc_Tree_Glue(Swc_Tree *tree, Swc_Tree_Node *tn)
{
  double pos[3];
  Swc_Tree_Node_Pos(tn, pos);
  Swc_Tree_Node *cnode = Swc_Tree_Closest_Node(tree, pos);
  Swc_Tree_Node_Set_Root(tn);
  Swc_Tree_Node_Add_Child(cnode, tn);
}

void Swc_Tree_Set_Feature(Swc_Tree *tree, double *feature)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    tn->feature = feature[tn->index];
  }
}

int Swc_Tree_Leaf_Number(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Leaf(tree);
  
  return Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_NO_UPDATE, TRUE);
}

int Swc_Tree_Branch_Number(Swc_Tree *tree)
{
  if (Swc_Tree_Number(tree) == 0) {
    return 0;
  }

  int n = 0;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, 
      SWC_TREE_ITERATOR_NO_UPDATE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  if (!Swc_Tree_Node_Is_Virtual(tn)) {
    n += Swc_Tree_Node_Child_Number(tn);
    if (n == 2) {
      n = 1;
    }
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Branch_Point(tn)) {
        n += Swc_Tree_Node_Child_Number(tn);
      }
    }
  } else {
    Swc_Tree tmp_tree;
    Swc_Tree_Node *tn = tree->root->first_child;
    while (tn != NULL) {
      tmp_tree.root = tn;
      n += Swc_Tree_Branch_Number(&tmp_tree);
      tn = tn->next_sibling;
    }
  }

  return n;
}

double Swc_Tree_Overall_Length(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  
  Swc_Tree_Node *tn = tree->begin;
  double length = 0.0;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    length += Swc_Tree_Node_Length(tn);
  }

  return length;
}

double Swc_Tree_Surface_Area(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);

  Swc_Tree_Node *tn = NULL;
  double area = 0.0;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    area += Swc_Tree_Node_Surface_Area(tn);
  }

  return area;
}

double* Swc_Tree_Accm_Length(Swc_Tree *tree, double *length)
{
  int n = Swc_Tree_Iterator_Start(tree, 1, FALSE);
  
  if (length == NULL) {
    length = darray_calloc(n);
  }
 
  Swc_Tree_Node *tn = tree->begin;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Root(tn)) {
      length[tn->index] = 0.0;
    } else {
      length[tn->index] = length[tn->parent->index] + Swc_Tree_Node_Length(tn);
    }
  }

  return length;
}

double* Swc_Tree_Accm_Thickness(Swc_Tree *tree, double *th)
{
  int n = Swc_Tree_Iterator_Start(tree, 1, FALSE);
  
  if (th == NULL) {
    th = darray_calloc(n);
  }
 
  Swc_Tree_Node *tn = tree->begin;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Root(tn)) {
      th[tn->index] = 0.0;
    } else {
      th[tn->index] = th[tn->parent->index] + 
	((Swc_Tree_Node_Radius(tn) + Swc_Tree_Node_Radius(tn->parent)) * 0.5 *
	 Swc_Tree_Node_Length(tn));
    }
  }

  return th;  
}

void Swc_Tree_Regularize(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = tree->begin;

  while (tn != NULL) {
    Swc_Tree_Node *next = tn->next;
    if (tn->parent != NULL) {
      if (Swc_Tree_Node_Is_Virtual(tn)) {
	Swc_Tree_Node_Merge_To_Parent(tn);
	//Swc_Tree_Node_Data(tn)->id = 1;
      }
    }
    tn = next;
  }
}

void Swc_Tree_Canonicalize(Swc_Tree *tree)
{
  if (tree->root == NULL) {
    return;
  }

  Swc_Tree_Regularize(tree);
  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    if (tree->root->first_child == NULL) {
      Kill_Swc_Tree_Node(tree->root);
      tree->root = NULL;
    } else if (tree->root->first_child->next_sibling == NULL) {
      Swc_Tree_Node *tmp_node = tree->root->first_child;
      Swc_Tree_Node_Detach_Parent(tmp_node);
      Kill_Swc_Tree_Node(tree->root);
      tree->root = tmp_node;
    }
  }
}

Swc_Tree* Swc_Tree_Pull_R(Swc_Tree *tree, const double *root)
{
  Swc_Tree_Node *tn = Swc_Tree_Closest_Node(tree, root);
  
  Swc_Tree_Node_Set_Root(tn);
  
  Swc_Tree *subtree = New_Swc_Tree();
  Swc_Tree_Node_Detach_Parent(tn);
  subtree->root = tn;

  if (tree->root == subtree->root) {
    tree->root = NULL;
  }

  return subtree;
}

BOOL Swc_Tree_Hit_Test(Swc_Tree *tree, int iter, 
		       double x, double y, double z)
{
  Swc_Tree_Iterator_Start(tree, iter, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree))) {
    if (Swc_Tree_Node_Hit_Test_N(tn, x, y, z)) {
      return TRUE;
    } else if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
      Local_Neuroseg locseg;
      Swc_Tree_Node_To_Locseg(tn, &locseg);
      if (Local_Neuroseg_Hit_Test(&locseg, x, y, z)) {
        return TRUE;
      }
    }
  }
  
  return FALSE;
}

BOOL Swc_Tree_Hit_Test_N(Swc_Tree *tree, int iter, 
    double x, double y, double z)
{
  Swc_Tree_Iterator_Start(tree, iter, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree))) {
    if (Swc_Tree_Node_Hit_Test_N(tn, x, y, z)) {
      return TRUE;
    }
  }
  
  return FALSE;
}

Swc_Tree_Node* Swc_Tree_Hit_Node(Swc_Tree *tree, double x, double y, double z)
{
  Swc_Tree_Node *hit = NULL;
  double mindist = Infinity;

  static const double Regularize_Number = 0.1;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree))) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Hit_Test_N(tn, x, y, z)) {
        double dist = Geo3d_Dist(tn->node.x, tn->node.y, tn->node.z, x, y, z) /
          (tn->node.d + Regularize_Number); //0.1 is for regularization
        if (dist < mindist) {
          hit = tn;
        }
      }
    }
  }

  return hit;
}

Swc_Tree_Node* Swc_Tree_Hit_Node_P(Swc_Tree *tree, double x, double y)
{
  Swc_Tree_Node *hit = NULL;
  double mindist = Infinity;

  static const double Regularize_Number = 0.1;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree))) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Hit_Test_P(tn, x, y)) {
        double dist = Geo3d_Dist(tn->node.x, tn->node.y, 0.0, x, y, 0.0) /
          (tn->node.d + Regularize_Number); //0.1 is for regularization
        if (dist < mindist) {
          hit = tn;
        }
      }
    }
  }

  return hit;
}

BOOL Swc_Tree_Hit_Test_E(Swc_Tree *tree, int iter, 
    double x, double y, double z, double scale, double delta)
{
  Swc_Tree *tmp_tree = Copy_Swc_Tree(tree);
  
  Swc_Tree_Iterator_Start(tmp_tree, iter, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tmp_tree))) {
    tn->node.d = tn->node.d * scale + delta;
  }

  Swc_Tree_Iterator_Start(tmp_tree, SWC_TREE_ITERATOR_NO_UPDATE, FALSE);
  while ((tn = Swc_Tree_Next(tmp_tree))) {
    if (Swc_Tree_Node_Hit_Test_N(tn, x, y, z)) {
      Kill_Swc_Tree(tmp_tree);
      return TRUE;
    } else if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
      Local_Neuroseg locseg;
      Swc_Tree_Node_To_Locseg(tn, &locseg);
      if (Local_Neuroseg_Hit_Test(&locseg, x, y, z)) {
        Kill_Swc_Tree(tmp_tree);
        return TRUE;
      }
    }
  }
  
  Kill_Swc_Tree(tmp_tree);

  return FALSE;
}

void Swc_Tree_Cut_Node(Swc_Tree *tree, Swc_Tree_Node *tn)
{
  if (tn->first_child == NULL) {
    Swc_Tree_Node_Detach_Parent(tn);
    Kill_Swc_Tree_Node(tn);
  } else {
    Swc_Tree_Node_Detach_Parent(tn);
    if (Swc_Tree_Node_Is_Virtual(tree->root) == FALSE) {
      Swc_Tree_Node *root = Make_Virtual_Swc_Tree_Node();
      Swc_Tree_Node_Set_Parent(tree->root, root);
      tree->root = root;
    }
    Swc_Tree_Node_Set_Parent(tn, tree->root);
    Swc_Tree_Node_Merge_To_Parent(tn);
  }
}

void Swc_Tree_Subtract(Swc_Tree *tree1, Swc_Tree *tree2)
{
  Swc_Tree_Iterator_Start(tree1, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree1);
  Swc_Tree_Node *next = NULL;
  Swc_Tree_Iterator_Start(tree2, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  if (Swc_Tree_Node_Is_Virtual(tn)) {
    tn = tn->next;
  }
  while (tn != NULL) {
    next = tn->next;
    if (Swc_Tree_Hit_Test(tree2, SWC_TREE_ITERATOR_NO_UPDATE, 
			  Swc_Tree_Node_Data(tn)->x, Swc_Tree_Node_Data(tn)->y,
			  Swc_Tree_Node_Data(tn)->z)) {
      Swc_Tree_Cut_Node(tree1, tn);
    }
    tn = next;
  }
}

Geo3d_Point_Array* Swc_Tree_Branch_2d_Layout(Swc_Tree *tree, int start_id,
    int end_id, double x1, double y1, double x2, double y2, double z_offset,
    double z_scale)
{
  Swc_Tree_Node *start_tn = Swc_Tree_Query_Node(tree, start_id, 
      SWC_TREE_ITERATOR_DEPTH_FIRST);

  Swc_Tree_Node *end_tn = Swc_Tree_Query_Node(tree, end_id, 
      SWC_TREE_ITERATOR_NO_UPDATE);

  Swc_Tree_Iterator_Path(tree, start_tn, end_tn);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);
  int n = 1;
  double total_length = 0.0;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    total_length += Swc_Tree_Node_Length(tn);
    n++;
  }

  Geo3d_Point_Array *pa = Make_Geo3d_Point_Array(n);
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_NO_UPDATE, FALSE);
  tn = Swc_Tree_Next(tree);
  pa->array[0][0] = x1;
  pa->array[0][1] = y1;
  pa->array[0][2] = tn->feature * z_scale + z_offset;
  int i = 1;
  double dist;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    dist = Swc_Tree_Node_Length(tn);
    pa->array[i][0] = pa->array[i-1][0] + dist / total_length * (x2 - x1);
    pa->array[i][1] = pa->array[i-1][1] + dist / total_length * (y2 - y1);
    pa->array[i][2] = tn->feature * z_scale + z_offset;
    i++;
  }

  return pa;
}

void Swc_Tree_Merge_Root_Group(Swc_Tree *tree, int label)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, FALSE);
  
  Swc_Tree_Node *tn = tree->begin;
  Swc_Tree_Node *next = NULL;
  while (tn != NULL) {
    next = tn->next;
    if (Swc_Tree_Node_Is_Regular_Root(tn->parent) == TRUE) {
      if ((Swc_Tree_Node_Label(tn->parent) == label) && 
	  (Swc_Tree_Node_Label(tn) == label)) {
	Swc_Tree_Node_Merge_To_Parent(tn); 
      }
    }
    tn = next;
  }
}

Swc_Tree_Node** Swc_Tree_Iterator_Array(Swc_Tree *tree, int *n)
{
  Swc_Tree_Node **tn_array = NULL;
  Swc_Tree_Node *tn = NULL;
  *n = Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_NO_UPDATE, TRUE);

  if (*n > 0) {
    GUARDED_MALLOC_ARRAY(tn_array, *n, Swc_Tree_Node*);
    int i = 0;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      tn_array[i++] = tn;
    }
  }

  return tn_array;
}

double Swc_Tree_Node_Apical_Score(Swc_Tree_Node *tn)
{
  return Swc_Tree_Node_Data(tn)->y;
}

void Swc_Tree_Label_Nw(Swc_Tree *tree, int label)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, TRUE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Label(tn) == 0) {
      Swc_Tree_Node_Set_Label(tn, label);
    }
  }
}

void swc_tree_node_set_label_d2(Swc_Tree_Node *tn, int label, int overwrite)
{
  Swc_Tree_Node *next = tn;
  while (next != NULL) {
    if (((overwrite >= 0) && (Swc_Tree_Node_Label(next) == overwrite)) || 
	((overwrite < 0) && (Swc_Tree_Node_Label(next) != -overwrite))) {
      Swc_Tree_Node_Set_Label(next, label);
    }
    next = Swc_Tree_Node_Next(next);
    if (next != NULL) {
      if (Swc_Tree_Node_Label(next->parent) == 1) {
	break;
      }
    }
  }
}

void Swc_Tree_Resort_Pyramidal(Swc_Tree *tree, BOOL label_soma, BOOL merge_soma)
{
  if (label_soma) {
    Swc_Tree_Grow_Soma(tree, 1);
  }

  Swc_Tree_Node *apical_tn = NULL;
  double apical_score = 0.0;

  if (merge_soma) {
    Swc_Tree_Merge_Root_Group(tree, 1);
    Swc_Tree_Node *root = Swc_Tree_Regular_Root(tree);
    Swc_Tree_Node *tn = root->first_child;
    Swc_Tree_Node *apical_tn = NULL;
    double apical_score = 0.0;
    while (tn != NULL) {
      double score = Swc_Tree_Node_Apical_Score(tn);
      if (score > apical_score) {
	apical_score = score;
	apical_tn = tn;
      }
      tn = tn->next_sibling;
    }

    Swc_Tree_Node_Set_Label_D(apical_tn, 4, -1);
    Swc_Tree_Label_Nw(tree, 3);
    /* make sure apcical_tn is the last child */
    Swc_Tree_Node_Detach_Parent(apical_tn);
    Swc_Tree_Node_Add_Child(root, apical_tn);
  } else {
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
    Swc_Tree_Node *tn = NULL;

    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn->parent)) {
	if ((Swc_Tree_Node_Label(tn->parent) == 1) && 
	    (Swc_Tree_Node_Label(tn) != 1)) {
	  double score = Swc_Tree_Node_Apical_Score(tn);
	  if (score > apical_score) {
	    apical_score = score;
	    apical_tn = tn;
	  }
	}
      }
    }

    swc_tree_node_set_label_d2(apical_tn, 4, -1);
    Swc_Tree_Label_Nw(tree, 3);
  }
}

void Swc_Tree_Set_Type_As_Label(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Label(tn) > 0) {
      Swc_Tree_Node_Data(tn)->type = Swc_Tree_Node_Label(tn);
    }
  }
}

void Swc_Tree_Set_Type(Swc_Tree *tree, int type)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Data(tn)->type = type;
  }
}

void Swc_Tree_Set_Label(Swc_Tree *tree, int label)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Set_Label(tn, label);
  }
}

void Swc_Tree_Identify_Puncta(Swc_Tree *tree, Geo3d_Scalar_Field *puncta, 
    int label, BOOL *on_label)
{
  int i;
  /* Try hit test first */
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  for (i = 0; i < puncta->size; i++) {
    on_label[i] = FALSE;
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_NO_UPDATE, FALSE);
    Swc_Tree_Node *tn = NULL;
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
	if ((Swc_Tree_Node_Label(tn) == label) &&
	    (Swc_Tree_Node_Label(tn->parent) == label)) {
	  Local_Neuroseg locseg;
	  if (Swc_Tree_Node_To_Locseg(tn, &locseg) != NULL) {
	    if (Local_Neuroseg_Hit_Test(&locseg, puncta->points[i][0],
		  puncta->points[i][1], puncta->points[i][2]) == TRUE) {
	      on_label[i] = TRUE;
	    }
	  }
	}
      }
    }
  }

  /* Distance check */
  Swc_Tree_Node *ctn = NULL;
  for (i = 0; i < puncta->size; i++) {
    if (on_label[i] == FALSE) {
      Swc_Tree_Point_Dist(tree, puncta->points[i][0], puncta->points[i][1],
	  puncta->points[i][2], NULL, &ctn);

      if (Swc_Tree_Node_Label(ctn) == label) {
	on_label[i] = TRUE;
      } else {
	on_label[i] = FALSE;
      }
    }
  }
}

Swc_Tree *Swc_Tree_From_Object_3d(const Object_3d *obj)
{
  int offset[3];
  Stack *objstack = Object_To_Stack_Bw(obj, 1, offset);

  Stack *tmpdist = Stack_Bwdist_L_U16P(objstack, NULL, 0);
#ifdef _DEBUG_2
  Write_Stack("../data/test.tif", tmpdist);
  exit(1);
#endif
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->wf = Stack_Voxel_Weight_I;
  size_t max_index;
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
  Swc_Tree_Node_Data(tn)->x = x + offset[0];
  Swc_Tree_Node_Data(tn)->y = y + offset[1];
  Swc_Tree_Node_Data(tn)->z = z + offset[2];
  Swc_Tree_Node_Data(tn)->d = sqrt(max_value);

  tree->root = tn;

  int max_count;
  int cur_index = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
  Int_Arraylist *path = New_Int_Arraylist();
  Swc_Tree_Node *prev_tn = NULL;

#define SWC_TREE_FROM_OBJECT_3D_ADD_NODE \
    STACK_UTIL_COORD(cur_index, Stack_Width(objstack), area, x, y, z); \
    tn = New_Swc_Tree_Node(); \
    Swc_Tree_Node_Data(tn)->x = x + offset[0]; \
    Swc_Tree_Node_Data(tn)->y = y + offset[1]; \
    Swc_Tree_Node_Data(tn)->z = z + offset[2]; \
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

  Swc_Tree_Resort_Id(tree);

#ifdef _DEBUG_2
  Write_Stack("../data/test.tif", tmpdist);
  exit(1);
#endif

  return tree;
}

Swc_Tree *Swc_Tree_From_Object_3d_M(const Object_3d *obj)
{
  int offset[3];
  Stack *objstack = Object_To_Stack_Bw(obj, 1, offset);

  Stack *tmpdist = Stack_Bwdist_L_U16P(objstack, NULL, 0);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->wf = Stack_Voxel_Weight_I;
  size_t max_index;
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
  Swc_Tree_Node_Data(tn)->x = x + offset[0];
  Swc_Tree_Node_Data(tn)->y = y + offset[1];
  Swc_Tree_Node_Data(tn)->z = z + offset[2];
  Swc_Tree_Node_Data(tn)->d = sqrt(max_value);

  tree->root = tn;

  int max_count;
  int cur_index = Stack_Sp_Grow_Furthest_Point(sgw, &max_count);
  Int_Arraylist *path = New_Int_Arraylist();
  Swc_Tree_Node *prev_tn = NULL;

#define SWC_TREE_FROM_OBJECT_3D_ADD_NODE \
    STACK_UTIL_COORD(cur_index, Stack_Width(objstack), area, x, y, z); \
    tn = New_Swc_Tree_Node(); \
    Swc_Tree_Node_Data(tn)->x = x + offset[0]; \
    Swc_Tree_Node_Data(tn)->y = y + offset[1]; \
    Swc_Tree_Node_Data(tn)->z = z + offset[2]; \
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

  Swc_Tree_Resort_Id(tree);

#ifdef _DEBUG_2
  Write_Stack("../data/test.tif", tmpdist);
  exit(1);
#endif

  return tree;
}

void Swc_Tree_Resort_Main_Trunk(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, FALSE);
  Swc_Tree_Node *tn = Swc_Tree_Next(tree);

  while (tn != NULL) {
    if (Swc_Tree_Node_Type(tn) == SWC_MAIN_TRUNK) {
      Swc_Tree_Node_Set_As_First_Child(tn);
    }
    tn = tn->next;
  }
}

double Swc_Tree_Node_Intensity_Distribution_E(Swc_Tree_Node *tn, Stack *signal, 
    Stack *mask, double margin)
{
  double r = Swc_Tree_Node_Radius(tn) + margin;

  int area = Stack_Width(signal) * Stack_Height(signal);
  int i, j, k;  
  int swidth = iround(r) * 2 + 1;
  int sheight = iround(r) * 2 + 1;
  int sdepth = iround(r) * 2 + 1;
  int c0 = swidth / 2;
  int c1 = sheight / 2;
  int c2 = sdepth / 2;
  int x = iround(Swc_Tree_Node_X(tn)) - c0;
  int y = iround(Swc_Tree_Node_Y(tn)) - c1;
  int z = iround(Swc_Tree_Node_Z(tn)) - c2;
  int offset = Stack_Util_Offset(x, y, z, Stack_Width(signal), 
      Stack_Height(signal), Stack_Depth(signal));
  int sj1 = Stack_Width(signal) - swidth;
  int sj2 = area - sheight * Stack_Width(signal);
  double intensity = 0.0;
  int count = 0;
  for (k = 0; k < swidth; k++) {
    for (j = 0; j < sheight; j++) {
      for (i = 0; i < swidth; i++) {
        if (offset >= 0) {
          BOOL signal_on = TRUE;
          if (mask != NULL) {
            if (mask->array[offset] == 0) {
              signal_on = FALSE;
            }
          }

          /* out of stack range */
          if (!IS_IN_OPEN_RANGE3(x + i, y + j, z + k, -1, Stack_Width(signal),
                -1, Stack_Height(signal), -1, Stack_Depth(signal))) {
            signal_on = FALSE;
          }

          if (signal_on) {
            if ((i - c0) * (i - c0) + (j - c1) * (j - c1) + (k - c2) * (k - c2)
                <= r * r) {
#ifdef _DEBUG_2
              printf("%d\n", offset);
#endif
              intensity += Stack_Array_Value(signal, offset);
              count++;
            }
          }
          offset++;
        } else {
          offset = Stack_Util_Offset(x + i, y + j, z + k, Stack_Width(signal), 
              Stack_Height(signal), Stack_Depth(signal));
        }
      }
      if (offset >= 0) {
        offset += sj1;
      }
    }
    if (offset >= 0) {
      offset += sj2;
    }
  }

  if (count > 0) {
    intensity /= count;
  }

  return intensity;
}

double Swc_Tree_Node_Intensity_Distribution(Swc_Tree_Node *tn, Stack *signal, 
    Stack *mask)
{
  return Swc_Tree_Node_Intensity_Distribution_E(tn, signal, mask, 0.0);
}

double Swc_Tree_Node_Intensity_Distribution_Ec(Swc_Tree_Node *tn, 
    Stack *signal, Stack *mask, double margin)
{
  double mr = Swc_Tree_Node_Radius(tn) + margin;
  double r = sqrt(mr * mr + Swc_Tree_Node_Radius(tn) *
      Swc_Tree_Node_Radius(tn));
#ifdef _DEBUG_
  printf("%g, %g\n", mr, r);
#endif

  int i, j, k;  
  int swidth = iround(r) * 2 + 1;
  int sheight = iround(r) * 2 + 1;
  int sdepth = iround(r) * 2 + 1;
  int c0 = swidth / 2;
  int c1 = sheight / 2;
  int c2 = sdepth / 2;
  int x = iround(Swc_Tree_Node_X(tn)) - c0;
  int y = iround(Swc_Tree_Node_Y(tn)) - c1;
  int z = iround(Swc_Tree_Node_Z(tn)) - c2;
  double intensity = 0.0;
  int count = 0;

  for (k = 0; k < sdepth; k++) {
    for (j = 0; j < sheight; j++) {
      for (i = 0; i < swidth; i++) {
        BOOL signal_on = TRUE;
        /* out of stack range */
        if (!IS_IN_OPEN_RANGE3(x + i, y + j, z + k, -1, Stack_Width(signal),
              -1, Stack_Height(signal), -1, Stack_Depth(signal))) {
          signal_on = FALSE;
        }

        if (signal_on) {
          double vec[3];
          double coord[3];
          coord[0] = i - c0;
          coord[1] = j - c1;
          coord[2] = k - c2;
          Swc_Tree_Node_Vector(tn, SWC_TREE_NODE_BIDIRECT, vec);
          vec[2] *= 2.4;
          coord[2] *= 2.4;
          Coordinate_3d_Unitize(vec);
          double proj = fabs(Coordinate_3d_Dot(vec, coord));
#ifdef _DEBUG_2
          printf("%g\n", proj);
#endif
          if ((proj <= Swc_Tree_Node_Radius(tn)) && 
              (Coordinate_3d_Length_Square(coord) - proj * proj <= mr * mr)) {
            size_t offset = Stack_Util_Offset(x + i, y + j, z + k,
                Stack_Width(signal), Stack_Height(signal), 
                Stack_Depth(signal)); 
#ifdef _DEBUG_2
            printf("%d\n", offset);
#endif
            intensity += Stack_Array_Value(signal, offset);
            count++;
          }
        }
      }
    }
  }

  if (count > 0) {
    intensity /= count;
  }

  return intensity;
}

void Swc_Tree_Intensity_Feature_E(Swc_Tree *tree, Stack *signal, Stack *mask,
    double margin)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, 
      SWC_TREE_ITERATOR_NO_UPDATE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    tn->feature = Swc_Tree_Node_Intensity_Distribution_Ec(tn, signal, mask, margin);
  }
}

void Swc_Tree_Intensity_Feature(Swc_Tree *tree, Stack *signal, Stack *mask)
{
  Swc_Tree_Intensity_Feature_E(tree, signal, mask, 0.0);
}

void Swc_Tree_Perimeter_Feature(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, 
      SWC_TREE_ITERATOR_NO_UPDATE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    tn->feature = Swc_Tree_Node_Radius(tn) * TZ_2PI;
  }
}

void Swc_Tree_Z_Feature(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, 
      SWC_TREE_ITERATOR_NO_UPDATE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    tn->feature = Swc_Tree_Node_Data(tn)->z;
  }
}

void Swc_Tree_Feature_Range(Swc_Tree *tree, double *fmin, double *fmax)
{
  if (fmin != NULL) {
    *fmin = Infinity;
  }
  if (fmax != NULL) {
    *fmax = -Infinity;
  }

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, 
      SWC_TREE_ITERATOR_NO_UPDATE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (fmin != NULL) {
      if (*fmin > tn->feature) {
        *fmin = tn->feature;
      }
    }
    if (fmax != NULL) {
      if (*fmax < tn->feature) {
        *fmax = tn->feature;
      }
    }
  }
}

void Swc_Tree_Puncta_Feature(Swc_Tree *tree, Geo3d_Scalar_Field *puncta)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST,
      SWC_TREE_ITERATOR_NO_UPDATE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    tn->feature = 0;
  }

  int i;
  for (i = 0; i < puncta->size; i++) {
    Swc_Tree_Point_Dist_N(tree, puncta->points[i][0], 
        puncta->points[i][1], puncta->points[i][2], &tn);
    tn->feature += 1.0;
  }
}

void Swc_Tree_Feature_To_Weight(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, 
      SWC_TREE_ITERATOR_NO_UPDATE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    tn->weight = tn->feature;
  }
}

void Swc_Tree_To_Branch_Feature_File(Swc_Tree *tree, const char *filepath)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = tree->root;
  BOOL start = TRUE;
  FILE *fp = fopen(filepath, "w");
  int start_node_id = -1;
  double length = 0;
  double surf_area = 0;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (start) { /* first node of the branch */
        start = FALSE;
        start_node_id = Swc_Tree_Node_Id(tn);
        length = 0.0;
        surf_area = 0.0;
      } else {
        length += Swc_Tree_Node_Length(tn);
        surf_area += Swc_Tree_Node_Surface_Area(tn);
      }

      /* restart if the end of a branch is reached */
      if (Swc_Tree_Node_Is_Leaf(tn)) {
        start = TRUE;
      } else {
        if (length > 1) {
          if (Swc_Tree_Node_Is_Branch_Point(tn)) {
            start = TRUE;
          }
        }
      }
      if (start) {
        fprintf(fp, "%d %g %g\n", start_node_id, length, surf_area); 
      }
    }
  }
  fclose(fp);
}

void Swc_Tree_Erase_Stack(Swc_Tree *tree, Stack *stack, double scale, 
    double delta)
{
  Swc_Tree *tmp_tree = Copy_Swc_Tree(tree);
  
  Swc_Tree_Iterator_Start(tmp_tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tmp_tree))) {
    tn->node.d = tn->node.d * scale + delta;
  }

  Swc_Tree_Iterator_Start(tmp_tree, SWC_TREE_ITERATOR_NO_UPDATE, FALSE);

  while ((tn = Swc_Tree_Next(tmp_tree))) {
    if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
      Local_Neuroseg locseg;
      Swc_Tree_Node_To_Locseg(tn, &locseg);
      Local_Neuroseg_Label_G(&locseg, stack, -1, 0, 1.0);
    }
  }

  int i, j, k;
  size_t offset = 0;
  for (k = 0; k < stack->depth; k++) {
    for (j = 0; j < stack->height; j++) {
      for (i = 0; i < stack->width; i++) {
        if (Swc_Tree_Hit_Test_N(tmp_tree, SWC_TREE_ITERATOR_NO_UPDATE, i, j, k)
            == TRUE) {
          stack->array[offset] = 0;
        }
        offset++;
      }
    }
  }

  Kill_Swc_Tree(tmp_tree);
}

void Swc_Tree_Translate(Swc_Tree *tree, double x, double y, double z)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = tree->root;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      Swc_Tree_Node_Data(tn)->x += x;
      Swc_Tree_Node_Data(tn)->y += y;
      Swc_Tree_Node_Data(tn)->z += z;
    }
  }
}

void Swc_Tree_Rotate(Swc_Tree *tree, double theta, double psi, 
    double cx, double cy, double cz)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = tree->root;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      Swc_Tree_Node_Rotate(tn, theta, psi, cx, cy, cz);
    }
  }
}

void Swc_Tree_Resize(Swc_Tree *tree, double x_scale, double y_scale, 
    double z_scale, BOOL change_node_size)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = tree->root;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      Swc_Tree_Node_Data(tn)->x *= x_scale;
      Swc_Tree_Node_Data(tn)->y *= y_scale;
      Swc_Tree_Node_Data(tn)->z *= z_scale;
      if (change_node_size == TRUE) {
        Swc_Tree_Node_Data(tn)->d *= sqrt(x_scale * y_scale);
      }
    }
  }
}

#if 1
/*
 * Cross talk score calculation:
 *   s(k) = I(k) * (1.0 + s(k-1) * 0.1)
 *   score(k) = sum(s(k)) / L
 */
double* Swc_Tree_Cross_Talk_Score(Swc_Tree *tree, Swc_Tree *source_tree, int *n)
{
  /* Label all the branches */
  *n = Swc_Tree_Label_Branch_All(tree);

  /* Relabel the regular roots */
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)&& (Swc_Tree_Node_Is_Regular(tn->parent))) {
      if (Swc_Tree_Node_Is_Root(tn->parent)) {
        if (Swc_Tree_Node_Child_Number(tn->parent) <= 2) {
          Swc_Tree_Node_Set_Label(tn, Swc_Tree_Node_Label(tn->parent));
        }
      } else {
        if (!Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
          Swc_Tree_Node_Set_Label(tn, Swc_Tree_Node_Label(tn->parent));
        }
      }
    }
  }


  double *score = darray_calloc(*n);

  TZ_ASSERT(darray_iszero(score, *n), "array initialization error");

  Swc_Tree_Iterator_Start(source_tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_REVERSE, FALSE);

  double s = 0.0;
  double sp = 0.0;
  int curr_label = 0;
  int prev_label = 0;
  int node_number = 0;
  int state = 0;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      BOOL hit = Swc_Tree_Hit_Test(source_tree, SWC_TREE_ITERATOR_NO_UPDATE, 
          Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn), Swc_Tree_Node_Z(tn));
      s = 0.0;
      if (hit == TRUE) {
        s = 1.0 + sp * 0.1;
      }

      curr_label = Swc_Tree_Node_Label(tn);
      if (curr_label != prev_label) {
        state = 0;
      }

      switch (state) {
        case 0:
          if (prev_label > 0) {
            score[prev_label - 1] += s;
            node_number++;
            TZ_ASSERT(node_number, "Zero node number");
            score[prev_label - 1] /= node_number;
            if (hit == TRUE) {
              s = 1.0;
            } else {
              s = 0.0;
            }
          }

          if (curr_label > 0) {
            score[curr_label - 1] += s;
            state = 1;
          }

          node_number = 1;
          break;
        case 1:
          if (curr_label > 0) {
            score[curr_label - 1] += s;
            node_number++;

            if ((tn->next == NULL) || Swc_Tree_Node_Is_Virtual(tn->next)) { 
              //about to quit
              score[curr_label - 1] /= node_number;
            }
          }
          break;
        default:
          break;
      }

      sp = s;
      prev_label = curr_label;
    }
  }

  return score;
}
#endif

BOOL Swc_Tree_Has_Branch(Swc_Tree *tree)
{
  BOOL has_branch = FALSE;

  if (tree != NULL) {
    Swc_Tree_Node *root = Swc_Tree_Regular_Root(tree);
    if (root != NULL) {
      if (root->first_child != NULL) {
        has_branch = TRUE;
      }
    }
  }

  return has_branch;
}

void Swc_Tree_Remove_Labeled(Swc_Tree *tree, int label)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  Swc_Tree_Node *tn = Swc_Tree_Next(tree);;
  /* For each node */
  while (tn != NULL) {
    Swc_Tree_Node *next_tn = tn->next;
    /* If the node has the specified label */
    if (Swc_Tree_Node_Label(tn) == label) {
      if (tn->next != NULL) {
        if ((Swc_Tree_Node_Label(tn->next) == 0) && (tn->next->parent == tn)) {
          Swc_Tree_Cut_Node(tree, tn);
        } else {
          /*  Merge it to its parent */
          Swc_Tree_Node_Merge_To_Parent(tn);
        }
      }
    }
    tn = next_tn;
  }
}

void Swc_Tree_Node_Translate(Swc_Tree_Node *tn, double dx, double dy,
    double dz)
{
  tn->node.x += dx;
  tn->node.y += dy;
  tn->node.z += dz;
}

void Swc_Tree_Node_Rotate(Swc_Tree_Node *tn, double theta, double psi,
    double cx, double cy, double cz)
{
  Swc_Tree_Node_Translate(tn, -cx, -cy, -cz);
  Geo3d_Rotate_Coordinate(&(tn->node.x), &(tn->node.y), &(tn->node.z),
      theta, psi, FALSE);
  Swc_Tree_Node_Translate(tn, cx, cy, cz);
}

void Swc_Tree_Centroid(Swc_Tree *tree, double *x, double *y, double *z)
{
  *x = 0.0;
  *y = 0.0;
  *z = 0.0;
  double w = 0.0;

  if (tree != NULL) {
    Swc_Tree_Node *tn = NULL;
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
        double wi = Swc_Tree_Node_Radius(tn);
        wi = wi * wi * wi;
        *x += Swc_Tree_Node_X(tn) * wi;
        *y += Swc_Tree_Node_Y(tn) * wi;
        *z += Swc_Tree_Node_Z(tn) * wi;
        w += wi;
      }
    }

    if (w > 0.0) {
      *x /= w;
      *y /= w;
      *z /= w;
    }
  }
}

int Swc_Tree_Sub_Degree(Swc_Tree *tree, int order)
{
  int degree = 0;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Is_Leaf(tn)) {
        degree++;
      } else if (Swc_Tree_Node_Is_Branch_Point(tn)) {
        if (Swc_Tree_Node_Label(tn) == order) {
          degree++;
        }
      }
    }
  }

  return degree;
}

int Swc_Tree_Sub_Size(Swc_Tree *tree, int order)
{
  int size = 0;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Label(tn) < order) {
        size++;
      }
    }
  }

  return size;
}

double Swc_Tree_Sub_Length(Swc_Tree *tree, int order)
{
  double length = 0;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Label(tn) < order) {
        length += Swc_Tree_Node_Length(tn);
      }
    }
  }

  return length;
}

Geo3d_Scalar_Field* Swc_Tree_Sub_To_Scalar_Field(Swc_Tree *tree, int order,
    Geo3d_Scalar_Field *field)
{
  if (field == NULL) {
    field = Make_Geo3d_Scalar_Field(
        Swc_Tree_Sub_Size(tree, order));
  }
  
  field->size = 0;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  Swc_Tree_Node *tn = NULL;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn) && Swc_Tree_Node_Label(tn) <= order) {
      field->points[field->size][0] = tn->node.x;
      field->points[field->size][1] = tn->node.y;
      field->points[field->size][2] = tn->node.z;
      field->values[field->size] = tn->node.d;
      field->size++;
    }
  }

  return field;
}

/*
static void swc_tree_sub_cov(Swc_Tree *tree, int order, double *cov)
{
  Geo3d_Scalar_Field *field = Swc_Tree_Sub_To_Scalar_Field(tree, order, NULL);

  Geo3d_Scalar_Field_Cov(field, cov);

  Kill_Geo3d_Scalar_Field(field);
}
*/

double Swc_Tree_Sub_Width(Swc_Tree *tree, int order)
{
  double cov[9];

  Geo3d_Scalar_Field *field = Swc_Tree_Sub_To_Scalar_Field(tree, order, NULL);
  Geo3d_Scalar_Field_Cov(field, cov);

  double eigv[3];
  double vec[3];
  Matrix_Eigen_Value_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], eigv);
  Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
      eigv[1], vec);

  double p = Geo3d_Dot_Product(field->points[0][0], field->points[0][1], 
      field->points[0][2], vec[0], vec[1], vec[2]);

  double minw = p - field->values[0];
  double maxw = p + field->values[0];

  int i;
  for (i = 1; i < field->size; i++) {
    p = Geo3d_Dot_Product(field->points[i][0], field->points[i][1], 
      field->points[i][2], vec[0], vec[1], vec[2]);
    double w = p - field->values[i];
    if (w < minw) {
      minw = w;
    }
    w = p + field->values[i];
    if (w > maxw) {
      maxw = w;
    }
  }

  Kill_Geo3d_Scalar_Field(field);

  return maxw - minw;
}

double Swc_Tree_Sub_Height(Swc_Tree *tree, int order)
{
  double cov[9];

  Geo3d_Scalar_Field *field = Swc_Tree_Sub_To_Scalar_Field(tree, order, NULL);
  Geo3d_Scalar_Field_Cov(field, cov);

  double eigv[3];
  double vec[3];
  Matrix_Eigen_Value_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], eigv);
  Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
      eigv[0], vec);

  double p = Geo3d_Dot_Product(field->points[0][0], field->points[0][1], 
      field->points[0][2], vec[0], vec[1], vec[2]);

  double minw = p - field->values[0];
  double maxw = p + field->values[0];

  int i;
  for (i = 1; i < field->size; i++) {
    p = Geo3d_Dot_Product(field->points[i][0], field->points[i][1], 
      field->points[i][2], vec[0], vec[1], vec[2]);
    double w = p - field->values[i];
    if (w < minw) {
      minw = w;
    }
    w = p + field->values[i];
    if (w > maxw) {
      maxw = w;
    }
  }

  Kill_Geo3d_Scalar_Field(field);

  return maxw - minw;
}

double Swc_Tree_Sub_Depth(Swc_Tree *tree, int order)
{
  double cov[9];

  Geo3d_Scalar_Field *field = Swc_Tree_Sub_To_Scalar_Field(tree, order, NULL);
  Geo3d_Scalar_Field_Cov(field, cov);

  double eigv[3];
  double vec[3];
  Matrix_Eigen_Value_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], eigv);
  Matrix_Eigen_Vector_Cs(cov[0], cov[4], cov[8], cov[1], cov[2], cov[5], 
      eigv[2], vec);

  double p = Geo3d_Dot_Product(field->points[0][0], field->points[0][1], 
      field->points[0][2], vec[0], vec[1], vec[2]);

  double minw = p - field->values[0];
  double maxw = p + field->values[0];

  int i;
  for (i = 1; i < field->size; i++) {
    p = Geo3d_Dot_Product(field->points[i][0], field->points[i][1], 
      field->points[i][2], vec[0], vec[1], vec[2]);
    double w = p - field->values[i];
    if (w < minw) {
      minw = w;
    }
    w = p + field->values[i];
    if (w > maxw) {
      maxw = w;
    }
  }

  Kill_Geo3d_Scalar_Field(field);

  return maxw - minw;
}

double Swc_Tree_Sub_Box_Volume(Swc_Tree *tree, int order)
{
  return Swc_Tree_Sub_Width(tree, order) * Swc_Tree_Sub_Height(tree, order) * 
    Swc_Tree_Sub_Depth(tree, order);
}

double Swc_Tree_Sub_Average_Angle(Swc_Tree *tree, int order)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  double angle = 0.0;
  int n = 0;
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Continuation(tn) && 
        Swc_Tree_Node_Label(tn) <= order) { 
      Swc_Tree_Node *p = tn->parent;
      Swc_Tree_Node *child = tn->first_child;

      double pt0[3], pt1[3], pt2[3];
      Swc_Tree_Node_Pos(p, pt0);
      Swc_Tree_Node_Pos(tn, pt1);
      Swc_Tree_Node_Pos(child, pt2);

      double vec1[3], vec2[3];
      Coordinate_3d_Sub(pt1, pt0, vec1);
      Coordinate_3d_Sub(pt2, pt1, vec2);

      angle += Coordinate_3d_Angle2(vec1, vec2);
      n++;
    }
  }

  return angle / n;;
}

double Swc_Tree_Sub_Max_Angle(Swc_Tree *tree, int order)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  double max_angle = 0.0;
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Continuation(tn) && 
        Swc_Tree_Node_Label(tn) <= order) { 
      Swc_Tree_Node *p = tn->parent;
      Swc_Tree_Node *child = tn->first_child;

      double pt0[3], pt1[3], pt2[3];
      Swc_Tree_Node_Pos(p, pt0);
      Swc_Tree_Node_Pos(tn, pt1);
      Swc_Tree_Node_Pos(child, pt2);

      double vec1[3], vec2[3];
      Coordinate_3d_Sub(pt1, pt0, vec1);
      Coordinate_3d_Sub(pt2, pt1, vec2);

      double angle = Coordinate_3d_Angle2(vec1, vec2);
      if (max_angle < angle) {
        max_angle = angle;
      }
    }
  }

  return max_angle;
}

double Swc_Tree_Sub_Average_Branching_Angle(Swc_Tree *tree, int order)
{
  double angle = 0.0;
  int n = 0;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Branch_Point(tn) && !Swc_Tree_Node_Is_Root(tn)) {
      Swc_Tree_Node *child = tn->first_child;
      Swc_Tree_Node *anotherChild = child->next_sibling;

      double pt0[3], pt1[3], pt2[3];
      Swc_Tree_Node_Pos(tn, pt0);
      Swc_Tree_Node_Pos(child, pt1);
      Swc_Tree_Node_Pos(anotherChild, pt2);

      Coordinate_3d_Sub(pt1, pt0, pt1);
      Coordinate_3d_Sub(pt2, pt0, pt2);
      angle += Coordinate_3d_Angle2(pt1, pt2);
      n++;
    }
  }

  return angle / n;
}

//double Swc_Tree_Sub_Max_Branching_Angle(Swc_Tree *tree, int order);
/*
double Swc_Tree_Sub_Max_Distance(Swc_Tree *tree, int order)
{
}
double Swc_Tree_Sub_Min_Distance(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Max_Length(Swc_Tree *tree, int order);
double Swc_Tree_Sub_Min_Legnth(Swc_Tree *tree, int order);
*/

/**@brief Resample an SWC tree.
 *
 * Swc_Tree_Resample() resamples <tree> by setting the distance between two 
 * adjacent nodes to be as close as <step>. The branch points and leaves will 
 * be kept at their original positions.
 */
#if 0
Swc_Tree* Swc_Tree_Resample(Swc_Tree *tree, double step)
{
  /* Label the branches */
  int n = Swc_Tree_Label_Branch_All(tree);

  Swc_Tree_Set_Branch_Point_Length_Feature(tree);

  /* Estimate the actual step for each branch */
  double *branchLength = darray_malloc(n);

  /* Resample the branch */
  /* While there is any terminal in the queue */
    /* */
}
#endif

void Default_Swc_Tree_Node_Label_Workspace(Swc_Tree_Node_Label_Workspace *ws)
{
  Default_Stack_Draw_Workspace(&(ws->sdw));
  ws->label_mode = SWC_TREE_LABEL_ALL;
  ws->offset[0] = 0.0;
  ws->offset[1] = 0.0;
  ws->offset[2] = 0.0;
  ws->z_proj = FALSE;
}

void Swc_Tree_Node_Label_Stack(const Swc_Tree_Node *tn, Stack *stack, 
    Swc_Tree_Node_Label_Workspace *ws)
{
  if (Swc_Tree_Node_Is_Regular(tn)) {
    if (ws->label_mode == SWC_TREE_LABEL_ALL ||
        ws->label_mode == SWC_TREE_LABEL_CONNECTION) {
      Local_Neuroseg locseg;
      Local_Neuroseg *tmpseg = NULL;
      if (ws->z_proj == TRUE) {
        tmpseg = Swc_Tree_Node_To_Locseg_P(tn, &locseg);
      } else {
        tmpseg = Swc_Tree_Node_To_Locseg(tn, &locseg);
      }

      if (tmpseg != NULL) {
        color_t color;
        color[0] = ws->sdw.color.r;
        color[1] = ws->sdw.color.g;
        color[2] = ws->sdw.color.b;
        Local_Neuroseg_Translate(&locseg, ws->offset);
        Local_Neuroseg_Label_C(&locseg, stack, 1.0, color);
      }
    }

    if (ws->label_mode == SWC_TREE_LABEL_NODE ||
        ws->label_mode == SWC_TREE_LABEL_ALL) {
      Geo3d_Ball_Label_Workspace ball_ws;
      Default_Geo3d_Ball_Label_Workspace(&ball_ws);
      ball_ws.sdw = ws->sdw;
      Geo3d_Ball ball;
      ball.r = tn->node.d;
      ball.center[0] = tn->node.x + ws->offset[0];
      ball.center[1] = tn->node.y + ws->offset[1];
      if (ws->z_proj == TRUE) {
        ball.center[2] = 0.0 + ws->offset[2];
      } else {
        ball.center[2] = tn->node.z + ws->offset[2];
      }
      Geo3d_Ball_Label_Stack(&ball, stack, &ball_ws);
    }
  }
}

void Swc_Tree_Label_Stack(Swc_Tree *tree, Stack *stack,
    Swc_Tree_Node_Label_Workspace *ws)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Label_Stack(tn, stack, ws);
  }
}

void Swc_Tree_Reconnect(Swc_Tree *tree, double z_scale, double distThre)
{
  if (tree == NULL) {
    return;
  }

  if (tree->root == NULL) {
    return;
  }

  if (Swc_Tree_Node_Is_Regular(tree->root)) {
    return;
  }

  if (Swc_Tree_Node_Child_Number(tree->root) < 2) {
    return;
  }

  int node_number = 
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, TRUE);

  /* alloc <node_array> */
  Swc_Tree_Node **node_array = (Swc_Tree_Node**) Guarded_Malloc(
      sizeof(Swc_Tree_Node*) * node_number, "Swc_Tree_Reconnect");

  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    node_array[tn->index] = tn;
  }

  int tree_number = Swc_Tree_Label_Forest(tree);

  Graph *graph = Make_Graph(node_number, 1, TRUE);
  
  int i;
  for (i = 0; i < node_number; ++i) {
    tn = node_array[i];

    if (Swc_Tree_Node_Is_Regular(tn) && 
        (Swc_Tree_Node_Is_Continuation(tn) == FALSE)) {
      double pos[3];
      Swc_Tree_Node_Pos(tn, pos);
      Swc_Tree_Node *root = tree->root->first_child;
      while (root != NULL) {
        if (Swc_Tree_Node_Label(tn) != Swc_Tree_Node_Label(root)) {
          Swc_Tree subtree;
          subtree.root = root;
          Swc_Tree_Node *ctn;
          double dist = Swc_Tree_Point_Dist_N_Z(
              &subtree, pos[0], pos[1], pos[2], z_scale, &ctn);
          if (dist < 0.0) {
            dist = 0.0;
          }
#ifdef _DEBUG_2
          if (tn->index == ctn->index) {
            printf("Debug here\n");
          }
#endif

          Graph_Add_Weighted_Edge(graph, tn->index, ctn->index, dist);
        }
        root = root->next_sibling;
      }
    }
  }

  Graph_Remove_Duplicated_Edge(graph);

  Graph *subgraph = Make_Graph(node_number, 1, TRUE);
  Graph *label_graph = Make_Graph(tree_number + 1, 1, TRUE);

  Graph_Workspace *gw = New_Graph_Workspace();

  for (i = 0; i < graph->nedge; ++i) {
    int v1 = graph->edges[i][0];
    int v2 = graph->edges[i][1];
    int label1 = Swc_Tree_Node_Label(node_array[v1]);
    int label2 = Swc_Tree_Node_Label(node_array[v2]);

    int edge_index = Graph_Edge_Index_U(label1, label2, gw);
    if (edge_index < 0) {
      Graph_Add_Weighted_Edge(label_graph, label1, label2, graph->weights[i]);
      Graph_Add_Weighted_Edge(subgraph, v1, v2, graph->weights[i]);
      Graph_Expand_Edge_Table(label1, label2, label_graph->nedge - 1, gw);
    } else {
      if (label_graph->weights[edge_index] > graph->weights[i]) {
        label_graph->weights[edge_index] = graph->weights[i];
        subgraph->edges[edge_index][0] = v1;
        subgraph->edges[edge_index][1] = v2;
        subgraph->weights[edge_index] = graph->weights[i];
      }
    }
  }

  Graph_To_Mst2(label_graph, gw);
  uint8_t *edge_in = gw->status;
  int j = 0;
  for (i = 0; i < subgraph->nedge; i++) {
    if (edge_in[i] == 1) {
      subgraph->edges[j][0] = subgraph->edges[i][0];
      subgraph->edges[j][1] = subgraph->edges[i][1];
      subgraph->weights[j] = subgraph->weights[i];
      j++;
    }
  }
  
  subgraph->nedge = j;

  /* Merge trees */
  for (i = 0; i < subgraph->nedge; ++i) {
    if (distThre < 0.0 || subgraph->weights[i] <= distThre) {
      Swc_Tree_Node *tn1 = node_array[subgraph->edges[i][0]];
      Swc_Tree_Node *tn2 = node_array[subgraph->edges[i][1]];

      Swc_Tree_Node_Set_Root(tn1);
      Swc_Tree_Node_Set_Parent(tn1, tn2);
    }
  }

  Kill_Graph(subgraph);
  Kill_Graph(label_graph);
  Kill_Graph_Workspace(gw);
  Kill_Graph(graph);

  /* free <node_array> */
  free(node_array);

  Swc_Tree_Resort_Id(tree);
}

int Swc_Tree_Label_Forest(Swc_Tree *tree)
{
  int tree_number = 0;

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular_Root(tn)) {
      Swc_Tree_Node_Set_Label(tn, ++tree_number);
    } else if (Swc_Tree_Node_Is_Regular(tn)) {
      Swc_Tree_Node_Set_Label(tn, Swc_Tree_Node_Label(tn->parent));
    }
  }

  return tree_number;
}
