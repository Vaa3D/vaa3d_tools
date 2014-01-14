/* testswc.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <utilities.h>
#include "tz_constant.h"
#include "tz_swc_cell.h"
#include "tz_geo3d_utils.h"
#include "tz_darray.h"
#include "tz_swc_arraylist.h"
#include "tz_swc.h"
#include "tz_iarray.h"
#include "tz_swc_tree.h"
#include "tz_string.h"
#include "tz_error.h"
#include "tz_xml_utils.h"
#include "tz_testdata.h"
#include "tz_math.h"
#include "tz_file_list.h"

#include "private/testswc.h"

void validate_swc(Swc_Node *neuron, int n)
{
  int i;
  for (i = 0; i < n; i++) {
    if (neuron[i].id != i+1) {
      printf("Invalid swc.\n");
      abort();
    }
  }
}

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-t")) {
    /* Read a SWC tree*/
    Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/depth_first_2.swc");

    /* Iterate through the SWC tree using depth-first search */
    int count = Swc_Tree_Iterator_Start(tree, 1, TRUE);

    Swc_Tree_Node *tn = NULL;
    int i = 1;
    int index = 0;

    /* Check the order of the node */
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
	/* In this SWC file, the IDs are sorted by the depth-first order */
	if (Swc_Tree_Node_Id(tn) != i) {
	  fprintf(stderr, ":( bug found!\n");
	  return 1;
	}

	/* We can also access the index of a node in the iterator */
	if (tn->index != index) {
	  printf("%d, %d\n", tn->index, index);
	  PRINT_EXCEPTION(":( Bug?", "Inconsistent index");
	  return 1;
	}

	i++;
      }
      index++;
    } 

    if (count != index) {
      PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
      return 1; 
    }
    
    /* You can reuse the previously-built iterator */
    Swc_Tree_Iterator_Start(tree, 0, FALSE);
    
    i = 1;
    index = 0;
    /* Check the order of the node */
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
	/* In this SWC file, the IDs are sorted by the depth-first order */
	if (Swc_Tree_Node_Id(tn) != i) {
	  fprintf(stderr, ":( bug found!\n");
	  return 1;
	}

	if (tn->index != index) {
	  printf("%d, %d\n", tn->index, index);
	  PRINT_EXCEPTION(":( Bug?", "Inconsistent index");
	  return 1;
	}
	
	i++;
      }
      index++;
    }

    /* Obtain a node with a certain ID. */
    Swc_Tree_Iterator_Start(tree, 0, FALSE);

    /* Query a node */

    /* The query returns NULL if no node is found */
    tn = Swc_Tree_Query_Node(tree, -5, SWC_TREE_ITERATOR_NO_UPDATE);

    if (tn != NULL) {
      PRINT_EXCEPTION(":( Bug?", "Inconsistent ID.");
      return 1;            
    }

    /* You can reverse an iterator */
    int count2 = Swc_Tree_Iterator_Start(tree, -1, FALSE);

    if (count != count2) {
      printf("%d, %d\n", count, count2);
      PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
      return 1;      
    }

    index = 0;
    /* Check the order of the node */
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (tn->index +index != count - 1) {
	printf("%d, %d\n", tn->index, index);
	PRINT_EXCEPTION(":( Bug?", "Inconsistent index");
	return 1;
      }
      index++;
    }

    if (index != count2) {
      printf("%d, %d\n", index, count2);
      PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
      return 1;
    }

    /* Reversing again should recover the old iterator */
    count2 = Swc_Tree_Iterator_Start(tree, -1, TRUE);

    if (count != count2) {
      printf("%d, %d\n", count, count2);
      PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
      return 1;      
    }

    index = 0;
    /* Check the order of the node */
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (tn->index != index) {
	printf("%d, %d\n", tn->index, index);
	PRINT_EXCEPTION(":( Bug?", "Inconsistent index");
	return 1;
      }
      index++;
    }

    if (index != count2) {
      printf("%d, %d\n", index, count2);
      PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
      return 1;
    }

    /* this is to show that the iterator 1 or 2 doesn't depend on previous iterator initialization*/
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    Swc_Tree_Iterator_Start(tree, 2, TRUE);
    Swc_Tree_Iterator_Start(tree, 1, TRUE);

    i = 1;
    /* Check the order of the node */
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
	/* In this SWC file, the IDs are sorted by the depth-first order */
	if (Swc_Tree_Node_Id(tn) != i) {
	  fprintf(stderr, ":( bug found!\n");
	  return 1;
	}
	i++;
      }
    }

    /* free a tree*/
    Kill_Swc_Tree(tree);

    /* Read another testing tree */
    tree = Read_Swc_Tree("../data/benchmark/swc/breadth_first.swc");

    Swc_Tree_Iterator_Start(tree, 1, TRUE);

    /* Iterate through the SWC tree using breadth-first search */
    count = Swc_Tree_Iterator_Start(tree, 2, TRUE);
    
    i = 1;

    index = 0;
    /* Check the order of the node */
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Regular(tn)) {
	/* In this SWC file, the IDs are sorted by the breadth-first order */
	if (Swc_Tree_Node_Id(tn) != i) {
	  fprintf(stderr, ":( bug found!\n");
	  return 1;
	}

	i++;
      }
      if (tn->index != index) {
	printf("%d, %d\n", tn->index, index);
	PRINT_EXCEPTION(":( Bug?", "Inconsistent index");
	return 1;
      }
      index++;
    }    
    
    if (count != index) {
      printf("%d, %d\n", count, index);
      PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
      return 1; 
    }

    Swc_Tree_Iterator_Start(tree, 0, FALSE);

    tn = Swc_Tree_Query_Node(tree, 2, SWC_TREE_ITERATOR_NO_UPDATE);

    /* Label some nodes in a tree */
    Swc_Tree_Node_Label_Branch(tn, 2);

    Swc_Tree_Iterator_Start(tree, 0, FALSE);
    tn = Swc_Tree_Query_Node(tree, 2, SWC_TREE_ITERATOR_NO_UPDATE);
    if (Swc_Tree_Node_Data(tn)->label != 2) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected label.");
      return 1;       
    }

    Swc_Tree_Iterator_Start(tree, 0, FALSE);
    tn = Swc_Tree_Query_Node(tree, 1, SWC_TREE_ITERATOR_NO_UPDATE);
    if (Swc_Tree_Node_Data(tn)->label == 2) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected label.");
      return 1;       
    }

    Swc_Tree_Iterator_Start(tree, 0, FALSE);
    tn = Swc_Tree_Query_Node(tree, 3, SWC_TREE_ITERATOR_NO_UPDATE);
    if (Swc_Tree_Node_Data(tn)->label == 2) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected label.");
      return 1;       
    }

    Swc_Tree_Iterator_Start(tree, 0, FALSE);
    tn = Swc_Tree_Query_Node(tree, 4, SWC_TREE_ITERATOR_NO_UPDATE);
    if (Swc_Tree_Node_Data(tn)->label == 2) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected label.");
      return 1;       
    }

    Kill_Swc_Tree(tree);

    /* Modify a tree */
    tree = Read_Swc_Tree("../data/benchmark/swc/breadth_first.swc");
    
    Swc_Tree_Remove_Spur(tree);
    
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if ((Swc_Tree_Node_Id(tn) == 4) || (Swc_Tree_Node_Id(tn) == 6)) {
	PRINT_EXCEPTION(":( Bug?", "Unexpected node.");
	return 1;	
      }
    }
    
    Kill_Swc_Tree(tree);
    
    /* properties */
    tree = Read_Swc_Tree("../data/benchmark/swc/depth_first.swc");
    Swc_Tree_Iterator_Start(tree, 1, FALSE);
    
    double r = Swc_Tree_Node_Average_Thickness(tree->begin, 
					       tree->begin->next->next->next);

    if (r != 2.0) {
      printf("%g\n", r);
      PRINT_EXCEPTION(":( Bug?", "Unexpected radius.");
      return 1;
    }
    Kill_Swc_Tree(tree);

    tree = Read_Swc_Tree("../data/benchmark/swc/breadth_first.swc");

    /* Iterate the leaves */
    count = Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_LEAF, FALSE);
      
    /*
    if (count != 3) {
        PRINT_EXCEPTION(":( Bug?", "Unexpected count.");
        return 1;
    }
    */

    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Leaf(tn) == FALSE) {
        Print_Swc_Tree_Node(tn);
        PRINT_EXCEPTION(":( Bug?", "The node is not a leaf.");
        return 1;
      }
    }

    if (fexist("../data/benchmark/swc/double_node2.swc")) {
      tree = Read_Swc_Tree("../data/benchmark/swc/double_node2.swc");
      double area = Swc_Tree_Surface_Area(tree);
      if (Compare_Float(area, 270.69, 0.1) != 0) {
        printf("%g\n", r);
        PRINT_EXCEPTION(":( Bug?", "Unexpected area.");
        return 1;
      }
      Kill_Swc_Tree(tree);
    } else {
      printf("Cannot find ../data/benchmark/swc/double_node2.swc\n");
    }

    tree = Read_Swc_Tree("../data/benchmark/swc/mouse_single_org.swc");
    if ((Swc_Tree_Node_Common_Ancestor(
            tree->root->first_child->first_child,
            tree->root->first_child->first_child->next_sibling->first_child) !=
          tree->root->first_child)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected node.");
      return 1;
    }

    if ((Swc_Tree_Node_Common_Ancestor(
            tree->root->first_child->first_child->next_sibling->first_child,
            tree->root->first_child->first_child) !=
          tree->root->first_child)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected node.");
      return 1;
    }

    if (Swc_Tree_Node_Common_Ancestor(tree->root->first_child, NULL) != NULL) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected node.");
      return 1;
    }

    if (Swc_Tree_Node_Common_Ancestor(NULL, tree->root->first_child) != NULL) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected node.");
      return 1;
    }

    tn = New_Swc_Tree_Node();

    if ((Swc_Tree_Node_Common_Ancestor(
            tn,
            tree->root->first_child->first_child->next_sibling->first_child) !=
          NULL)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected node.");
      return 1;
    }

    if (Swc_Tree_Node_Common_Ancestor(tree->root->first_child->first_child,
          tn) != NULL) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected node.");
      return 1;
    }

    Kill_Swc_Tree(tree);

    //printf("area: %g\n", area);

    { /* unit test */
      Swc_Tree *tree = New_Swc_Tree();
      
      tree->root = New_Swc_Tree_Node();
      tree->root->next = New_Swc_Tree_Node();
      int count = Swc_Tree_Iterator_Start(tree, 2, TRUE);
      if (count != 1) {
	printf("%d\n", count);
	PRINT_EXCEPTION(":( Bug?", "Unexpected number.");
	return 1;
      }
      
      count = Swc_Tree_Iterator_Start(tree, 0, TRUE);
      if (count != 1) {
	printf("%d\n", count);
	PRINT_EXCEPTION(":( Bug?", "Unexpected number.");
	return 1;
      }

      tree->root->index = 10;
      count = Swc_Tree_Iterator_Start(tree, 1, TRUE);
      if (count != 1) {
	Print_Swc_Tree(tree);
	printf("%d\n", count);
	PRINT_EXCEPTION(":( Bug?", "Unexpected number.");
	return 1;
      }

      index = 0;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
	if (tn->index != index) {
	  printf("%d, %d\n", tn->index, index);
	  PRINT_EXCEPTION(":( Bug?", "Inconsistent index");
	  return 1;
	}
	index++;
      }
      if (count != index) {
	printf("%d, %d\n", count, index);
	PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
	return 1; 
      }

      tree->root->index = 10;
      count = Swc_Tree_Iterator_Start(tree, 2, TRUE);
      if (count != 1) {
	Print_Swc_Tree(tree);
	printf("%d\n", count);
	PRINT_EXCEPTION(":( Bug?", "Unexpected number.");
	return 1;
      }

      index = 0;
      while ((tn = Swc_Tree_Next(tree)) != NULL) {
	if (tn->index != index) {
	  printf("%d, %d\n", tn->index, index);
	  PRINT_EXCEPTION(":( Bug?", "Inconsistent index");
	  return 1;
	}
	index++;
      }
      if (count != index) {
	printf("%d, %d\n", count, index);
	PRINT_EXCEPTION(":( Bug?", "Inconsistent number");
	return 1; 
      }

      Kill_Swc_Tree(tree); 

      if (test_swc_tree_node_property()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }

      if (test_swc_tree_property()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }
      
      if (test_swc_tree_regularize()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }

      if (test_swc_tree_canonical()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }

      if (test_swc_tree_node_point()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }

      if (test_swc_tree_subtract()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }

      if (test_swc_tree_property()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }

      if (test_swc_tree_number()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }

      if (test_swc_tree_branch_number()) {
	PRINT_EXCEPTION(":( Bug?", "Unit test failed.");
	return 1;
      }
    } /* unit test */

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0
  const char *file_path = "/Users/zhaot/Data/neuron/Neurolucida_reconstructed_neurons/swc_z_rescaled/c_27_25LHregion0.ASC.swc";
  int n = Swc_File_Record_Number(file_path);
  printf("%d\n", n);

  Swc_Node *neuron = Read_Swc_File(file_path, &n);
  printf("%d\n", n);

  validate_swc(neuron, n);

  int k = 0;
  int i, j;

  double a[200];

  for (i = 0; i < n; i++) {
    neuron[i].z *= 1.00 / 0.32;
  }

  for (i = 0; i < n; i++) {
    for (j = i + 1; j < n; j++) {
      if (neuron[i].parent_id == neuron[j].parent_id) {
	int b = neuron[i].parent_id - 1;
	int m = neuron[b].parent_id - 1;
	/*
	printf("%d %d %d %d\n", i, j, b, m);
	Print_Swc_Node(neuron + i);
	Print_Swc_Node(neuron + j);
	Print_Swc_Node(neuron + b);
	Print_Swc_Node(neuron + m);
	*/
	a[k++] = Geo3d_Angle2(neuron[i].x - neuron[b].x,
			      neuron[i].y - neuron[b].y,
			      neuron[i].z - neuron[b].z,
			      neuron[j].x - neuron[b].x,
			      neuron[j].y - neuron[b].y,
			      neuron[j].z - neuron[b].z) * 180.0 / TZ_PI;
	
	a[k++] = Geo3d_Angle2(neuron[i].x - neuron[b].x,
			      neuron[i].y - neuron[b].y,
			      neuron[i].z - neuron[b].z,
			      neuron[m].x - neuron[b].x,
			      neuron[m].y - neuron[b].y,
			      neuron[m].z - neuron[b].z) * 180.0 / TZ_PI;

	a[k++] = Geo3d_Angle2(neuron[j].x - neuron[b].x,
			      neuron[j].y - neuron[b].y,
			      neuron[j].z - neuron[b].z,
			      neuron[m].x - neuron[b].x,
			      neuron[m].y - neuron[b].y,
			      neuron[m].z - neuron[b].z) * 180.0 / TZ_PI;
	
      }
    }
  }

  darray_write("../data/test.bn", a, k);
#endif

#if 0
  const char *file_path = "../data/fly_neuron_n1/graph_d.swc";
  int n = Swc_File_Record_Number(file_path);
  printf("%d\n", n);

  Swc_Node *neuron = Read_Swc_File(file_path, &n);
  printf("%d\n", n);  
  
  Swc_Sort(neuron, n);

  Swc_Arraylist *sa = Swc_Arraylist_Wrap(neuron, n);
  Print_Swc_Arraylist(sa);

  int *sq = Swc_Arraylist_Queue(sa, NULL);
  iarray_print2(sq, sa->length, 1);

  Graph *graph = Swc_Arraylist_Graph(sa, NULL);
  Print_Graph(graph);

  Graph_Workspace *gw = New_Graph_Workspace();
  int *in_degree = Graph_In_Degree(graph, gw);
  iarray_print2(in_degree, graph->nvertex, 1);

  int *out_degree = Graph_Out_Degree(graph, gw);
  iarray_print2(out_degree, graph->nvertex, 1);

  int *degree = Graph_Degree(graph, gw);
  iarray_print2(degree, graph->nvertex, 1);

#endif

#if 0
  const char *file_path = "../data/fly_neuron_n2/graph_d.swc";
  int n;
  Swc_Node *neuron = Read_Swc_File(file_path, &n);

  Swc_Arraylist *sa = Swc_Arraylist_Wrap(neuron, n);

  Graph *graph = Swc_Arraylist_Graph(sa, NULL);
  
  Graph_Workspace *gw = New_Graph_Workspace();

  Graph *graph2 = Graph_Main_Subgraph(graph, gw);
  Swc_Arraylist *sa2 = Swc_Arraylist_Rebuild(sa, graph2);
  Swc_Arraylist_To_Swc_File(sa2, "../data/test.swc");

#endif

#if 0
  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  Print_Swc_Tree_Node(tn);

  Print_Swc_Tree_Node(Swc_Tree_Node_Next(tn));

  Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/tube.swc");
  Print_Swc_Tree(tree);
  
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
#endif

#if 0 /* test memory leak */
  while (1) {
    //String_Workspace *sw = Make_String_Workspace(20);
    //Kill_String_Workspace(sw);
    //Swc_File_Max_Id("../data/diadem_e1/tube.swc");
    Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/tube.swc");
    Swc_Tree_Node_Fsize(tree->root);
    //Swc_Tree_To_Dot_File(tree, "../data/test.dot");
    Kill_Swc_Tree(tree);
  }
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/test.swc");
  Print_Swc_Tree(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");

  
  //Swc_Tree_Node *node = New_Swc_Tree_Node();

  Swc_Tree_Iterator_Start(tree, 2);
  
  
  //Swc_Tree_Node_Set_Root(tree->root->next->next->next->next->next);

  printf("%d\n", Swc_Tree_Node_Fsize(tree->root));

  //Swc_Tree_Node_Insert_Before(tree->root->next->next->next->next->next->next, node);
  

  //Swc_Tree_Remove_Spur(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
  
  return 1;
  /*
  Swc_Tree_Iterator_Start(tree, 2);

  Print_Swc_Tree_Node(tree->root->next);
  Print_Swc_Tree_Node(tree->root->next->next);

  Swc_Tree_Node *tn = Swc_Tree_Node_Cut(tree->root->next->next->next->next);
  
  Swc_Tree_Connect_Node(tree, tn);
  */

  Swc_Tree_Tune_Fork(tree);
  Swc_Tree_Resort_Id(tree);
  Print_Swc_Tree(tree);

  Write_Swc_Tree("../data/test2.swc", tree);
  /*
  printf("\n");

  Swc_Tree_Iterator_Start(tree, TRUE);
  Swc_Tree_Node_Merge_To_Parent(tree->root);
  Print_Swc_Tree(tree);
  
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
  */
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/graph_d.swc");
  //Print_Swc_Tree(tree);

  //printf("%d\n", Swc_Tree_Node_Is_Turning(tree->root->next->next->next->next));

  /* 
  Swc_Tree_Resort_Id(tree);
  Print_Swc_Tree(tree);
  */

  /*
  Swc_Tree_Remove_Zigzag(tree);
  Swc_Tree_Leaf_Shrink(tree);

  //Print_Swc_Tree(tree);

  Swc_Tree_Remove_Spur(tree);

  Swc_Tree_Tune_Fork(tree);
  */
  Swc_Tree_Tune_Branch(tree);
  Swc_Tree_Resort_Id(tree);

  Write_Swc_Tree("../data/test2.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/graph_d.swc");
  
  Local_Neuroseg *locseg = 
    Swc_Tree_Node_To_Locseg(tree->root->first_child->first_child, NULL);
  
  FILE *fp = fopen("../data/test.swc", "w");
  Local_Neuroseg_Swc_Fprint_T(fp, locseg, 1, -1, 3);
  fclose(fp);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e3/tube.swc");
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
  Swc_Tree_Clean_Root(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/test.swc");
  Print_Swc_Tree(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");

  Swc_Tree_Merge_Close_Node(tree, 0.01);
  Print_Swc_Tree(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/test.swc");
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
  Print_Swc_Tree(tree);

  Swc_Tree_Leaf_Shrink(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
  Swc_Tree_Merge_Close_Node(tree, 0.01);
  Print_Swc_Tree(tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/test.swc");
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
  Print_Swc_Tree(tree);

  Swc_Tree_Node *root = tree->root->first_child;
  Swc_Tree_Node *tmp = root->first_child;
  Swc_Tree_Node_Replace_Child(root->first_child, 
			      root->first_child->first_child);
  Print_Swc_Tree_Node(root->first_child);
  Swc_Tree_Node_Set_Parent(tmp, root->first_child);
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/test.swc");

  tree->root->first_child->first_child->first_child->weight = 2.0;
  tree->root->first_child->first_child->first_child->first_child->weight = 3.0;
  tree->root->first_child->first_child->weight = 1.0;
  Print_Swc_Tree(tree);
  printf("\n");

  Swc_Tree_To_Dot_File(tree, "../data/test.dot");

  //Swc_Tree_Tune_Branch(tree);
  Swc_Tree_Node_Set_Root(tree->root->first_child->first_child->first_child);
  Print_Swc_Tree(tree);

  /*
  Swc_Tree_Node *root = tree->root->first_child;
  Swc_Tree_Node_Tune_Branch(root->first_child->first_child->next_sibling);
  */
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
  /*
  Swc_Tree_Tune_Branch(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test2.swc", tree);
  */
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/jinny/4Dendrogram/Slice15_3to11dsUp_den.swc");

  Swc_Tree_Remove_Zigzag(tree);  
  Swc_Tree_Node_Set_Root(Swc_Tree_Largest_Node(tree));
  Swc_Tree_Resort_Id(tree);

  Swc_Tree_Reduce(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test2.swc", tree);
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/graph_d.swc");

  Swc_Tree_Remove_Zigzag(tree);

  Swc_Tree_Iterator_Start(tree, 1);

  Swc_Tree_Node *tn = NULL;
  int i = 1;
  /* Check the order of the node */
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Print_Swc_Tree_Node(tn);
  }
#endif

#if 0
  //Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/e1.swc");

  Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/graph_d.swc");

  
  Swc_Tree_Iterator_Start(tree, 1);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Data(tn)->id == 54) {
      if (Swc_Tree_Node_Is_Turning(tn)) {
	Swc_Tree_Node_Label_Branch(tn, 1);
      }
    }
  }
  
  /*
  double pos[3] = {143, 253, 40};
  Swc_Tree_Label_Branch(tree, 1, pos, 1.5);
  */
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
  
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/graph_d.swc");
  Swc_Tree_Remove_Spur(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  //Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/multi_branch.swc");
  //Swc_Tree *tree = Read_Swc_Tree("../data/diadem_e1/graph_d.swc");
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/jinny/110309_12L/tracing/cellL_woS.swc");
  Swc_Tree_Clean_Root(tree);
  Write_Swc_Tree("../data/test.swc", tree);
  Swc_Tree_To_Svg_File(tree, "../data/test.svg");
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/depth_first.swc");

  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  Swc_Tree_Node_Data(tn)->id = 7;
  Swc_Tree_Node *tn2 = New_Swc_Tree_Node();
  Swc_Tree_Node_Data(tn2)->id = 8;
  Swc_Tree_Node_Set_Parent(tn2, tn);

  Swc_Tree_Node *r1 = tree->root->first_child->first_child;
  Swc_Tree_Node_Graft(r1, NULL, tn, tn->first_child);
  Swc_Tree_Node_Kill_Subtree(r1);
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/e1_new.swc");
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) == 577) {
      break;
    }
  }
  Swc_Tree *tree2 = Read_Swc_Tree("../data/e1_seg.swc");
  Swc_Tree_Node *r2 = tree2->root->first_child;
  Swc_Tree_Node_Detach_Parent(r2);
  while (r2->first_child != NULL) {
    r2 = r2->first_child;
  }
  Swc_Tree_Node_Set_Root(r2);
  //Swc_Tree_Node_Detach_Parent(tn);
  Swc_Tree_Node_Graft(tn, NULL, r2, NULL);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/e1_fix.swc");
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) == 1423) {
      break;
    }
  }
  Swc_Tree *tree2 = Read_Swc_Tree("../data/e1_seg2.swc");
  Swc_Tree_Node *r2 = tree2->root->first_child;
  Swc_Tree_Node_Detach_Parent(r2);

  //Swc_Tree_Node_Detach_Parent(tn);
  Swc_Tree_Node_Graft(tn, NULL, r2, NULL);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/e1_fix.swc");
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) == 1365) {
      break;
    }
  }
 
  Swc_Tree_Node_Detach_Parent(tn);


  //Swc_Tree_Node_Graft(tn, NULL, r2, NULL);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/e1_fix.swc");
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) == 1340) {
      break;
    }
  }
 
  Swc_Tree_Node_Detach_Parent(tn);


  //Swc_Tree_Node_Graft(tn, NULL, r2, NULL);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  //Swc_File_Cat("../data/diadem_e1/graph_d.swc", "../data/diadem_e2/graph_d.swc",
  //	       "../data/test.swc");

  char *files[] = { "../data/diadem_e1/graph_d.swc", 
		    "../data/diadem_e2/graph_d.swc",
		    "../data/diadem_e3/graph_d.swc" };
  Swc_File_Cat_M(files, 3, "../data/test.swc");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/jinny/4Dendrogram/4dendro_ApicalDen_Up.swc");
  //Swc_Tree_Clean_Root(tree);
  Swc_Tree_Label_Main_Trunk_L(tree, 1, 600.0);
  //Swc_Tree_To_Dot_File(tree, "../data/test.dot");
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Data(tn)->label > 0) {
      Swc_Tree_Node_Data(tn)->type = 5;
    }
  }

  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);

  Kill_Swc_Tree(tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/e1_fix.swc");
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("/Users/zhaot/Data/diadem/e1_fix.swc", tree);
  Kill_Swc_Tree(tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/overshoot.swc");
  Swc_Tree_Remove_Overshoot(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/jinny/110309_12L/tracing/cellL_woS.swc");
  Swc_Tree_Merge_Close_Node(tree, 0.1);
  Swc_Tree_Remove_Overshoot(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/depth_first.swc");
  Swc_Tree_Label_Branch_All(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
  Print_Swc_Tree(tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/depth_first.swc");
  Print_Swc_Tree(tree);
  Swc_Tree *tree2 = Copy_Swc_Tree(tree);
  Kill_Swc_Tree(tree);
  Print_Swc_Tree(tree2);
  Swc_Tree *tree3 = Copy_Swc_Tree(tree2);
  Kill_Swc_Tree(tree2);
  Print_Swc_Tree(tree3);
  Swc_Tree *tree4 = Copy_Swc_Tree(tree3);
  Kill_Swc_Tree(tree3);
  Print_Swc_Tree(tree4);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/jinny/edswc_A copy/edswc_A0001.swc");
  Geo3d_Scalar_Field *field = Geo3d_Scalar_Field_Import_Apo("/Users/zhaot/Data/jinny/edswc_A copy/edswc_A0001.swc.apo");

  Swc_Tree_To_Svg_File_P(tree, field, "../data/test.svg");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/multi_tree.swc");
  //Print_Swc_Tree(tree);
  coordinate_3d_t root = {0, 0, 0};
  Swc_Tree *tree2 = Swc_Tree_Pull_R(tree, root);
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
  Swc_Tree_To_Dot_File(tree2, "../data/test3.dot");
  /*
  Print_Swc_Tree(tree);
  printf("tree2: \n");
  Print_Swc_Tree(tree2);
  */
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_11.swc");
  
  double root[3] = {288, 54, 77};
  Swc_Tree *tree2 = Swc_Tree_Pull_R(tree, root);
  Swc_Tree_Node *tn = Swc_Tree_Outmost_Leaf(tree2);

  Swc_Tree_Node_Data(tn)->type = 5;

  Write_Swc_Tree("../data/test.swc", tree2);

  /* Load other trees. */
  Kill_Swc_Tree(tree);
  tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_1.swc");
  
  Swc_Tree *tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_2.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_3.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_4.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  /* Look for continuation. */
  
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/multi_branch.swc");
  Swc_Tree_Remove_Terminal_Branch(tree, 15.0);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_1.swc");

  Swc_Tree *tree3;

  /*
  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_3.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);
  */
  
  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_5.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);
  
  /*
  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_6.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);
  */
  
  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_7.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);
  
  /*
  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_9.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_10.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  
  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_11.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);

  tree3 = Read_Swc_Tree("/Users/zhaot/Data/diadem/d1_swcs/diadem_d1_047_label_12.swc");
  Swc_Tree_Merge(tree, tree3);
  Kill_Swc_Tree(tree3);
  */

  Swc_Tree_Remove_Terminal_Branch(tree, 30.0);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  /* Read swc file */
  //Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/mouse_single_org.swc");
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Data/jinny/all_swc/02L_up.swc"); 

  /* Labe soma */
  Swc_Tree_Label_Soma(tree, 1);
  //Swc_Tree_Merge_Root_Group(tree, 1);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/breadth_first.swc");
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn1 = Swc_Tree_Query_Node(tree, 5);
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_NO_UPDATE, FALSE);
  Swc_Tree_Node *tn2 = Swc_Tree_Query_Node(tree, 4);

  Swc_Tree_Iterator_Path(tree, tn1, tn2);
  
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Print_Swc_Tree_Node(tn);
  }
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/mouse_single_org.swc");

  Geo3d_Point_Array *pa = Swc_Tree_Branch_2d_Layout(tree, 123, 708, 10, 50, 200,
      50);

  FILE *fp = fopen("../data/test.svg", "w");
  Svg_Header_Fprint(fp, NULL);
  Svg_Begin_Fprint(fp, 500, 500, NULL, NULL);
  Svg_Polygonal_Chain_Fprint(fp, pa, "jet", 1); 
  Svg_End_Fprint(fp);
  fclose(fp);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/multi_tree.swc");
  Print_Swc_Tree(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
  Swc_Tree_Node_Set_Label(tree->root->first_child, 1);
  Swc_Tree_Node_Set_Label(Swc_Tree_Query_Node_C(tree, 2), 1);
  Swc_Tree_Node_Set_Label(Swc_Tree_Query_Node_C(tree, 3), 1);
  Swc_Tree_Node_Set_Label(Swc_Tree_Query_Node_C(tree, 6), 1);
  Swc_Tree_Node_Set_Label(Swc_Tree_Query_Node_C(tree, 7), 1);
  Swc_Tree_Node_Set_Label(Swc_Tree_Query_Node_C(tree, 8), 1);
  Swc_Tree_Node_Set_Label(Swc_Tree_Query_Node_C(tree, 10), 1);
  Swc_Tree_Merge_Root_Group(tree, 1);
  Print_Swc_Tree(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");

#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/multi_tree.swc");
  Swc_Tree_Node *tn = Swc_Tree_Query_Node_C(tree, 12);

  Swc_Tree_Branch tb;
  Set_Swc_Tree_Branch(&tb, tn, 3);
  Swc_Tree_Get_Branch(tn, FALSE, &tb);
  Print_Swc_Tree_Branch(&tb);
#endif

#if 0
  Swc_Tree *tree =
    Read_Swc_Tree("/Users/zhaot/Data/jinny/all_swc/03aLentireRR_overshoot.swc");
  Swc_Tree_Resort_Pyramidal(tree, TRUE);
  Swc_Tree_Set_Type_As_Label(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
  //Swc_Tree_To_Svg_File_P(tree, NULL, 800, 600, "../data/test.svg");
#endif

#if 0
  Swc_Tree *tree =
    Read_Swc_Tree("/Users/zhaot/Data/jinny/all_swc/B_09L_entire.swc");
  Swc_Tree_Grow_Soma(tree, 1);
  Write_Swc_Tree("../data/test.swc", tree);
  //Swc_Tree_To_Svg_File_P(tree, NULL, 800, 600, "../data/test.svg");
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/regular.swc");
  Swc_Tree_Remove_Terminal_Branch(tree, 18.0);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree =
    Read_Swc_Tree("../data/benchmark/swc/regular.swc");
  Swc_Tree_Grow_Soma(tree, 1);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  /* Detect z-jump */
  //Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/regular.swc");
  char filepath[500];
  int i;
  for (i = 1; i <= 34; i++) {
    sprintf(filepath, "/Users/zhaot/Data/diadem/Neocortical Layer 6"
	" Axons/auto/NC_%02d.swc", i);
      Swc_Tree *tree = Read_Swc_Tree(filepath);
    Write_Swc_Tree("../data/test2.swc", tree);
    double thre = 50.0;
    BOOL stop = FALSE;

    while (stop == FALSE) {
      stop = TRUE;
      Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

      double length = 0.0; /* branch length */

      Swc_Tree_Node *tn = NULL;
      Swc_Tree_Node *prev_tn = NULL;
      int state = 0;
      prev_tn = Swc_Tree_Next(tree);
      if (Swc_Tree_Node_Is_Virtual(prev_tn)) {
	prev_tn = Swc_Tree_Next(tree);
      }
      if (Swc_Tree_Node_Is_Branch_Point(prev_tn)) {
	state = 1;
      }

      while ((tn = Swc_Tree_Next(tree)) != NULL) {
	switch (state) {
	  case 0: /* wait for starting branching point */
	    if (Swc_Tree_Node_Is_Branch_Point(tn)) {
	      state = 1;
	    }
	    break;
	  case 1: /* branching on */
	    if (tn->parent == prev_tn) {
	      length += Swc_Tree_Node_Length(tn);
	      if (Swc_Tree_Node_Is_Branch_Point(tn)) {
		state = 2;
	      } else if (Swc_Tree_Node_Is_Leaf(tn)) {
		length = 0.0;
	      }
	    } else {
	      length = Swc_Tree_Node_Length(tn);
	      if (Swc_Tree_Node_Is_Branch_Point(tn)) {
		state = 2;
	      } 
	    }
	    break;
	  default:
	    break;
	}

	if (state == 2) {
	  state = 1;

	  if (length <= thre) {
	    /* check orientation */
	    Swc_Tree_Node *tmp_tn = tn->parent;
	    double z = 0.0;
	    while (Swc_Tree_Node_Is_Continuation(tmp_tn)) {
	      tmp_tn = tmp_tn->parent;
	    }
	    z = fabs(Swc_Tree_Node_Data(tmp_tn)->z -
		Swc_Tree_Node_Data(tn)->z);
	    double a = Swc_Tree_Node_Dot(tn->parent, tn, tn->first_child);
	    printf("%g, %g\n", z/length, a);
	    if (z/length > 0.3 /*&& a < 0.7*/) {
	      //Swc_Tree_Node_Label_Branch_U(tn, 5);
	      Swc_Tree_Node_Detach_Parent(tn);
	      stop = FALSE;
	      break;
	    }
	  }
	  length = 0.0;
	}
	prev_tn = tn;
      }
    }

    Swc_Tree_Set_Type_As_Label(tree);

    Swc_Tree_Resort_Id(tree);
    sprintf(filepath, "../data/diadem_c1/rmct/%02d.swc", i);
    Write_Swc_Tree(filepath, tree);
  }
#endif

#if 0
  Geo3d_Scalar_Field *puncta = 
    Geo3d_Scalar_Field_Import_Apo("/Users/zhaot/Data/jinny/Shaul/020910c_07_2a.swc.apo");
  Swc_Tree *tree =
    Read_Swc_Tree("/Users/zhaot/Data/jinny/Shaul/020910c_07_2aNNewPy.swc");
  Swc_Tree_Grow_Soma(tree, SWC_SOMA);
  Swc_Tree_To_Analysis_File(tree, puncta, "../data/branch.txt",
      "../data/puncta.txt");
  
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Data(tn)->type = Swc_Tree_Node_Label(tn) % 12;
  }
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/regular.swc");
  Swc_Tree_Grow_Soma(tree, 1);
  Swc_Tree_Resort_Pyramidal(tree, FALSE, TRUE);
  Swc_Tree_Set_Type_As_Label(tree);

  //Swc_Tree_Node_Detach_Parent(tree->root->first_child->first_child->first_child);

  //Swc_Tree_Node_Detach_Parent(tree->root->first_child->first_child->next_sibling->first_child);
  //Swc_Tree_Node_Detach_Parent(tree->root->first_child->first_child->next_sibling->first_child);
  Swc_Tree_Svg_Workspace *ws = New_Swc_Tree_Svg_Workspace();
  Write_Swc_Tree("../data/test.swc", tree);
  Swc_Tree_To_Svg_File_W(tree, "../data/test.svg", ws);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Desktop/hc/diadem_b1.swc");
  Print_Swc_Tree(tree);
#endif

#if 0
  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  double pos[3] = {2, 2, 1};
  Swc_Tree_Node_Set_Pos(tn, pos);
  Stack *signal = Index_Stack(GREY, 5, 5, 5);
  double intensity = Swc_Tree_Node_Intensity_Distribution(tn, signal, NULL);
  printf("%g\n", intensity);
  
  Kill_Stack(signal);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("/media/KINGSTON/Work/data/grasp/0515_15Pyds.swc");
  Swc_Tree_Label_Branch_All(tree);
  Geo3d_Scalar_Field *small_puncta = Geo3d_Scalar_Field_Import_Apo("/media/KINGSTON/Work/data/grasp/0515_15Py.swc_small.apo"); 
  /*
  Geo3d_Scalar_Field *big_puncta = Geo3d_Scalar_Field_Import_Apo("/media/KINGSTON/Work/data/grasp/0515_15Py.swc_small.apo"); 
  Geo3d_Scalar_Field *puncta = Geo3d_Scalar_Field_Merge(small_puncta, big_puncta, NULL);
*/
  int i;
  for (i = 0; i < small_puncta->size; i++) {
    small_puncta->points[i][0] /= 2;
    small_puncta->points[i][1] /= 2;
  }

  Swc_Tree_Puncta_Feature(tree, small_puncta);
  Swc_Tree_Feature_To_Weight(tree);
  Stack *signal = Read_Stack("/media/KINGSTON/Work/data/grasp/slice15_3to33ds_ch2.tif");
  Swc_Tree_Intensity_Feature(tree, signal, NULL);

  FILE *fp = fopen("../data/test.txt", "w");

  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST,
      SWC_TREE_ITERATOR_NO_UPDATE);
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if(Swc_Tree_Node_Is_Regular(tn)) {
      fprintf(fp, "%d %d %g %g %g %g %g %g\n", Swc_Tree_Node_Label(tn), 
          Swc_Tree_Node_Type(tn), Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),  
          Swc_Tree_Node_Z(tn), Swc_Tree_Node_Radius(tn), tn->weight, 
          tn->feature);
    }
  }

  fclose(fp);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/breadth_first.swc");
  
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_BREADTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  
  double vec[3];
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Vector(tn, SWC_TREE_NODE_BIDIRECT, vec);
    printf("%g, %g, %g\n", vec[0], vec[1], vec[2]);
  }

#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/puncta_tree.swc");
  Print_Swc_Tree(tree);
  Swc_Tree_Label_Branch_All(tree);
  Geo3d_Scalar_Field *small_puncta = Geo3d_Scalar_Field_Import_Apo("../data/benchmark/swc/puncta_tree.apo"); 
  Print_Geo3d_Scalar_Field(small_puncta);
  Swc_Tree_Puncta_Feature(tree, small_puncta);
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, SWC_TREE_ITERATOR_NO_UPDATE);
  Stack *stack = Make_Stack(GREY, 10, 20, 1);
  Zero_Stack(stack);
  int k = 1;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      printf("%d %d %g %g %g %g %g\n", Swc_Tree_Node_Label(tn), 
          Swc_Tree_Node_Type(tn), Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),  
          Swc_Tree_Node_Z(tn), tn->weight, tn->feature);
      Set_Stack_Pixel(stack, Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),  
          Swc_Tree_Node_Z(tn), 0, k++);
    }
  }


  Swc_Tree_Feature_To_Weight(tree);
  Swc_Tree_Intensity_Feature(tree, stack, NULL);

  FILE *fp = fopen("../data/test.txt", "w");

  tn = NULL;
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST,
      SWC_TREE_ITERATOR_NO_UPDATE);
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    fprintf(fp, "%d %d %g %g %g %g %g\n", Swc_Tree_Node_Label(tn), 
        Swc_Tree_Node_Type(tn), Swc_Tree_Node_X(tn), Swc_Tree_Node_Y(tn),  
        Swc_Tree_Node_Z(tn), tn->weight, tn->feature);
  }

  fclose(fp);
  Kill_Stack(stack);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/test.swc");
  Print_Swc_Tree(tree);
  Swc_Tree_To_Dot_File(tree, "../data/test.dot");
#endif

#if 0
  Stack *stack = Make_Stack(GREY, 100, 100, 100);
  uint8_t value = 255;
  Stack_Set_Constant(stack, &value);

  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/erase.swc");

  Swc_Tree_Erase_Stack(tree, stack, 1.0, 0.0);

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Swc_Tree *source_tree = 
    Read_Swc_Tree("../data/benchmark/swc/compare/compare1.swc");

  /*
  Swc_Tree *tree = 
    Read_Swc_Tree("../data/benchmark/swc/compare/compare5.swc");
    */
  Swc_Tree *tree = Copy_Swc_Tree(source_tree);

  Swc_Tree_Node_Detach_Parent(tree->root->first_child->first_child->first_child);

  int n;
  double *score = Swc_Tree_Cross_Talk_Score(tree, source_tree, &n);

  darray_print2(score, n, 1);
#endif

#if 0
  Swc_Tree *source_tree = Read_Swc_Tree("/Users/zhaot/Work/neutube/neurolabi/data/Fab1/fab1r.swc");
  Swc_Tree *tree = Read_Swc_Tree("/Users/zhaot/Work/neutube/neurolabi/data/Fab1/fab1g.swc");
  int n;
  double *score = Swc_Tree_Cross_Talk_Score(tree, source_tree, &n);

  darray_print2(score, n, 1);

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (score[Swc_Tree_Node_Label(tn) - 1] > 0.5) {
      tn->node.label = 1;
      if (Swc_Tree_Node_Is_Regular_Root(tn->parent)) {
        Swc_Tree_Node_Set_Label(tn->parent, 1);
      }
    } else {
      tn->node.label = 0;
    }
  }

    /*
  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    Swc_Tree_Node *root = tree->root->first_child;
    while (root != NULL) {
      if ((Swc_Tree_Node_Label(root) == 0) && (root->first_child != NULL)) {
        Swc_Tree_Node *child = root->first_child;
        while (child != NULL) {
          if (Swc_Tree_Node_Label(child) == 1) {
            Swc_Tree_Node_Set_Label(root, 1);
            break;
          }
          child = child->next_sibling;
        }
      }

      root = root->next_sibling;
    }
  }
*/
  Swc_Tree_Remove_Labeled(tree, 1);

  Swc_Tree_Resort_Id(tree);
  
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree_E("../data/test.swc");
  Print_Swc_Tree(tree);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/fork.swc");
  Swc_Tree_Label_Branch_Order(tree);
  Print_Swc_Tree(tree);

  printf("%g\n", Swc_Tree_Sub_Depth(tree, 2));
#endif

#if 0
  /* Load all files */
  File_List *fileList = File_List_Load_Dir("../data/tmp/swc3/adjusted", "swc",
      NULL);

  int feature_number = 9;
  double *feature = darray_malloc(fileList->file_number * feature_number);

  /* For each swc file */
  int i;
  int offset = 0;
  int bigOrder = 65535;
  for (i = 0; i < fileList->file_number; i++) {
    //printf("%s\n", fileList->file_path[i]);
    Swc_Tree *tree = Read_Swc_Tree(fileList->file_path[i]);
    Swc_Tree_Label_Branch_Order(tree);
    /* Calculate the feature set */
    feature[offset++] = Swc_Tree_Sub_Degree(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Size(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Length(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Width(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Height(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Depth(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Average_Angle(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Max_Angle(tree, bigOrder);
    feature[offset++] = Swc_Tree_Sub_Average_Branching_Angle(tree, bigOrder);
  }

  darray_print2(feature, feature_number, fileList->file_number);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/multi_tree2.swc");
  /*
  Swc_Tree subtree;
  subtree.root = tree->root->first_child;

  Print_Swc_Tree(&subtree);
  */

  Swc_Tree_Reconnect(tree, 1.6);

  Print_Swc_Tree(tree);

  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Swc_Tree *tree  = Read_Swc_Tree_E("../data/MC0509C3X100-1.crash.swc");
  //Print_Swc_Tree(tree);
  Swc_Tree *tree2 = Copy_Swc_Tree(tree);
  Write_Swc_Tree("../data/test.swc", tree2);
#endif

#if 1
  Swc_Tree *tree = Read_Swc_Tree_E("../data/flyem/skeletonization/session3/len15/adjusted2/Mi1_215.swc");
  Swc_Tree_Node_Label_Workspace workspace;
  Default_Swc_Tree_Node_Label_Workspace(&workspace);
  
  double corner[6];
  Swc_Tree_Bound_Box(tree, corner);

  int width = iround(corner[3] - corner[0]);
  int height = iround(corner[4] - corner[1]);
  int depth = iround(corner[5] - corner[2]);

  workspace.offset[0] = -corner[0];
  workspace.offset[1] = -corner[1]; 
  workspace.offset[2] = -corner[2]; 

  Stack *stack = Make_Stack(GREY, width, height, depth);
  Zero_Stack(stack);
  Swc_Tree_Label_Stack(tree, stack, &workspace);

  Write_Stack("../data/test.tif", stack);

  Image *image = Proj_Stack_Ymax(stack);
  Write_Image("../data/test2.tif", image);
#endif


  return 0;
}
