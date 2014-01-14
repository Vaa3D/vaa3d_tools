/* @file testswc.h
 * @author Ting Zhao
 * @date 28-Nov-2009
 */

#ifndef _TESTSWC_H_
#define _TESTSWC_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

static int test_swc_tree_node_property()
{
  if (Swc_Tree_Node_Is_Virtual(NULL)) {
    PRINT_EXCEPTION("Bug?", "Unexpected virtual node.");
    return 1;
  }

  if (Swc_Tree_Node_Is_Root(NULL)) {
    PRINT_EXCEPTION("Bug?", "Unexpected root.");
    return 1;
  }

  Swc_Tree_Node *tn = New_Swc_Tree_Node();
  Swc_Tree_Node_Data(tn)->id = -1;

  if (!Swc_Tree_Node_Is_Virtual(tn)) {
    PRINT_EXCEPTION("Bug?", "Virtual node cannot be identified.");
    return 1;    
  }

  if (Swc_Tree_Node_Is_Regular(tn)) {
    PRINT_EXCEPTION("Bug?", "Unexpected regular node.");
    return 1;    
  }

  if (!Swc_Tree_Node_Is_Root(tn)) {
    PRINT_EXCEPTION("Bug?", "Should be root.");
    return 1;
  }

  Swc_Tree_Node *tn2 = New_Swc_Tree_Node();
  Swc_Tree_Node_Set_Parent(tn2, tn);
  if (!Swc_Tree_Node_Is_Root(tn2)) {
    PRINT_EXCEPTION("Bug?", "Should be root.");
    return 1;
  }

  Swc_Tree_Node_Data(tn)->id = 0;
  if (Swc_Tree_Node_Is_Virtual(tn)) {
    PRINT_EXCEPTION("Bug?", "Unexpected virtual node.");
    return 1;    
  }

  if (!Swc_Tree_Node_Is_Regular(tn)) {
    PRINT_EXCEPTION("Bug?", "Unexpected virtual node.");
    return 1;    
  }

  Swc_Tree_Node *tn3 = New_Swc_Tree_Node();
  Swc_Tree_Node_Set_Parent(tn3, tn2);
  if (Swc_Tree_Node_Is_Root(tn3)) {
    PRINT_EXCEPTION("Bug?", "Unexpected root.");
    return 1;    
  }

  Swc_Tree_Node_Data(tn2)->id = -1;
  if (Swc_Tree_Node_Is_Root(tn3)) {
    PRINT_EXCEPTION("Bug?", "Unexpected root.");
    return 1;
  }

  Swc_Tree_Node_Data(tn)->id = -1;
  if (!Swc_Tree_Node_Is_Root(tn3)) {
    PRINT_EXCEPTION("Bug?", "Should be root.");
    return 1;
  }

  Kill_Swc_Tree_Node(tn);
  Kill_Swc_Tree_Node(tn2);
  Kill_Swc_Tree_Node(tn3);

  if (Swc_Tree_Node_Is_Regular(NULL)) {
    PRINT_EXCEPTION("Bug?", "Unexpected regular node.");
    return 1;
  }

  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/depth_first.swc");
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
    
  double r = Swc_Tree_Node_Average_Thickness(tree->begin, 
					     tree->begin->next->next->next);

  if (r != 2.0) {
    printf("%g\n", r);
    PRINT_EXCEPTION(":( Bug?", "Unexpected radius.");
    return 1;
  }

  r = Swc_Tree_Node_Average_Thickness(NULL, 
				      tree->begin->next->next->next);
  
  if (r != 2.0) {
    printf("%g\n", r);
    PRINT_EXCEPTION(":( Bug?", "Unexpected radius.");
    return 1;
  }

  r = Swc_Tree_Node_Average_Thickness(tree->begin->next, 
				      tree->begin->next->next->next);
  
  if (r != 2.0) {
    printf("%g\n", r);
    PRINT_EXCEPTION(":( Bug?", "Unexpected radius.");
    return 1;
  }

  r = Swc_Tree_Node_Average_Thickness(tree->begin->next->next->next, 
				      tree->begin->next->next->next);
  
  if (r != 3.0) {
    printf("%g\n", r);
    PRINT_EXCEPTION(":( Bug?", "Unexpected radius.");
    return 1;
  }

  double length = Swc_Tree_Node_Length(tree->begin);
  if (length != 0.0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected length.");
    return 1;
  }

  length = Swc_Tree_Node_Length(tree->begin->next);
  if (length != 0.0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected length.");
    return 1;
  }

  length = Swc_Tree_Node_Length(tree->begin->next->next);
  if (length != 1.0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected length.");
    return 1;
  }

  length = Swc_Tree_Node_Length(tree->begin->next->next->next);
  if (length != 1.0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected length.");
    return 1;
  }

  Swc_Tree_Node_Data(tree->begin->next->next)->id = -1.0;
  length = Swc_Tree_Node_Length(tree->begin->next->next->next);
  if (length != 2.0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected length.");
    return 1;
  }

  Kill_Swc_Tree(tree);

  return 0;
}

static int test_swc_tree_property()
{
  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/depth_first.swc");
  
  double length = Swc_Tree_Overall_Length(tree);
  if (length != 6.0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected length.");
    return 1;
  }
  
  Kill_Swc_Tree(tree);

  if (fexist("../data/benchmark/swc/regular.swc")) {
    tree = Read_Swc_Tree("../data/benchmark/swc/regular.swc");
    if (Swc_Tree_Leaf_Number(tree) != 12) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected leaf number.");
      return 1;
    }

    if (Swc_Tree_Overall_Length(tree) != 440) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected leaf number.");
      return 1;
    }

    Kill_Swc_Tree(tree);
  }

  if (Swc_Tree_Has_Branch(NULL) == TRUE) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected branch.");
    return 1;
  }

  tree = New_Swc_Tree();
  if (Swc_Tree_Has_Branch(tree) == TRUE) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected branch.");
    return 1;
  }
  
  tree->root = Make_Virtual_Swc_Tree_Node();
  if (Swc_Tree_Has_Branch(tree) == TRUE) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected branch.");
    return 1;
  }

  tree->root->first_child = New_Swc_Tree_Node();
  if (Swc_Tree_Has_Branch(tree) == TRUE) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected branch.");
    return 1;
  }

  tree->root->first_child->first_child = New_Swc_Tree_Node();
  if (Swc_Tree_Has_Branch(tree) == FALSE) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected property.");
    return 1;
  }

  return 0;
}

static int test_swc_tree_regularize()
{
  Swc_Tree *tree = New_Swc_Tree();
  
  Swc_Tree_Regularize(tree);

  if (tree->root != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected root.");
    return 1;
  }

  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Regularize(tree);
  if (tree->root == NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);

  Swc_Node *sn = New_Swc_Node();
  sn->id = 1;

  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Node *tn = Make_Swc_Tree_Node(sn);
  Swc_Tree_Node_Set_Parent(tn, tree->root);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tn);
  Swc_Tree_Regularize(tree);
  
  if (!Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tree->root->first_child != tn) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tn->first_child != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);
  
  tree->root = Make_Virtual_Swc_Tree_Node();
  tn = Make_Swc_Tree_Node(sn);
  Swc_Tree_Node_Set_Parent(tn, tree->root);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tn);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tn->first_child);

  Swc_Tree_Regularize(tree);
  
  if (!Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tree->root->first_child != tn) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tn->first_child != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);

  tree->root = Make_Virtual_Swc_Tree_Node();
  tn = Make_Swc_Tree_Node(sn);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tree->root);
  Swc_Tree_Node_Set_Parent(tn, tree->root->first_child);

  Swc_Tree_Regularize(tree);
  
  if (!Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tree->root->first_child != tn) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tn->first_child != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);

  tree->root = Make_Virtual_Swc_Tree_Node();
  tn = Make_Swc_Tree_Node(sn);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tree->root);

  Swc_Tree_Regularize(tree);
  
  if (!Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tree->root->first_child != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }
  
  tree->root = Make_Virtual_Swc_Tree_Node();
  tn = Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  sn->id++;
  tn = Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tn->parent, Make_Virtual_Swc_Tree_Node());
  Swc_Tree_Node_Add_Child(tn->parent, Make_Virtual_Swc_Tree_Node());
  Swc_Tree_Node_Add_Child(tn->parent, Make_Virtual_Swc_Tree_Node());
  tn = tn->next_sibling;
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  tn = tn->next_sibling;
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  tn = tn->next_sibling;
  tn = Swc_Tree_Node_Add_Child(tn, Make_Virtual_Swc_Tree_Node());
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));

  //Print_Swc_Tree(tree);
  Swc_Tree_Regularize(tree);

  int n = Swc_Tree_Iterator_Start(tree, 1, FALSE);
  if (n != 8) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;    
  }

  tn = Swc_Tree_Next(tree);
  if (!Swc_Tree_Node_Is_Virtual(tn)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;    
  }

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) != 8 - (--n)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
      return 1;    
    }
  }

  //Swc_Tree_To_Dot_File(tree, "../data/test.dot");

  Kill_Swc_Node(sn);
  Kill_Swc_Tree(tree);

  return 0;
}

static int test_swc_tree_canonical()
{
  Swc_Tree *tree = New_Swc_Tree();
  
  Swc_Tree_Canonicalize(tree);

  if (tree->root != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected root.");
    return 1;
  }

  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Canonicalize(tree);
  if (tree->root != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);

  Swc_Node *sn = New_Swc_Node();
  sn->id = 1;

  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Node *tn = Make_Swc_Tree_Node(sn);
  Swc_Tree_Node_Set_Parent(tn, tree->root);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tn);
  Swc_Tree_Canonicalize(tree);
  
  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tree->root != tn) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tn->first_child != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);
  
  tree->root = Make_Virtual_Swc_Tree_Node();
  tn = Make_Swc_Tree_Node(sn);
  Swc_Tree_Node_Set_Parent(tn, tree->root);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tn);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tn->first_child);

  Swc_Tree_Canonicalize(tree);
  
  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tree->root != tn) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tn->first_child != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);

  tree->root = Make_Virtual_Swc_Tree_Node();
  tn = Make_Swc_Tree_Node(sn);
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tree->root);
  Swc_Tree_Node_Set_Parent(tn, tree->root->first_child);

  Swc_Tree_Canonicalize(tree);
  
  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tree->root != tn) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  if (tn->first_child != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }

  Clean_Swc_Tree(tree);

  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Node_Set_Parent(Make_Virtual_Swc_Tree_Node(), tree->root);

  Swc_Tree_Canonicalize(tree);
  
  if (tree->root != NULL) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;
  }
  
  tree->root = Make_Virtual_Swc_Tree_Node();
  tn = Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  sn->id++;
  tn = Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tn->parent, Make_Virtual_Swc_Tree_Node());
  Swc_Tree_Node_Add_Child(tn->parent, Make_Virtual_Swc_Tree_Node());
  Swc_Tree_Node_Add_Child(tn->parent, Make_Virtual_Swc_Tree_Node());
  tn = tn->next_sibling;
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  tn = tn->next_sibling;
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  tn = tn->next_sibling;
  tn = Swc_Tree_Node_Add_Child(tn, Make_Virtual_Swc_Tree_Node());
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));
  sn->id++;
  Swc_Tree_Node_Add_Child(tn, Make_Swc_Tree_Node(sn));

  //Print_Swc_Tree(tree);
  Swc_Tree_Canonicalize(tree);

  int n = Swc_Tree_Iterator_Start(tree, 1, FALSE);
  if (n != 7) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;    
  }

  tn = Swc_Tree_Next(tree);
  if (Swc_Tree_Node_Is_Virtual(tn)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;    
  }

  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_NO_UPDATE, FALSE);
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) != 7 - (--n)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
      return 1;    
    }
  }

  //Swc_Tree_To_Dot_File(tree, "../data/test.dot");

  Clean_Swc_Tree(tree);
  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  Swc_Tree_Canonicalize(tree);

  if (!Swc_Tree_Node_Is_Virtual(tree->root)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree.");
    return 1;    
  }

  Kill_Swc_Node(sn);
  Kill_Swc_Tree(tree);

  return 0;
}

static int test_swc_tree_node_point() 
{
  Swc_Tree_Node *tn = New_Swc_Tree_Node();

  if (Swc_Tree_Node_Hit_Test(tn, Swc_Tree_Node_Data(tn)->x, 
			     Swc_Tree_Node_Data(tn)->y,
			     Swc_Tree_Node_Data(tn)->z) == FALSE) {
    PRINT_EXCEPTION(":( Bug?", "Missing hit.");
    return 1;        
  }

  Kill_Swc_Tree_Node(tn);

  Swc_Tree *tree = Read_Swc_Tree("../data/benchmark/swc/breadth_first.swc");
  tn = tree->root->first_child->first_child;
  if (Swc_Tree_Node_Hit_Test(tn, Swc_Tree_Node_Data(tn)->x, 
			     Swc_Tree_Node_Data(tn)->y,
			     Swc_Tree_Node_Data(tn)->z) == FALSE) {
    PRINT_EXCEPTION(":( Bug?", "Missing hit.");
    return 1;        
  }

  if (Swc_Tree_Node_Hit_Test(tn, 50, 50, 0) == FALSE) {
    PRINT_EXCEPTION(":( Bug?", "Missing hit.");
    return 1;        
  }

  if (Swc_Tree_Node_Hit_Test(tn, 5.0, 0.1, 0.1)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected hit.");
    return 1;        
  }

  if (Swc_Tree_Node_Hit_Test(tn, 2.0, 0.1, 0.1)) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected hit.");
    return 1;        
  }

  if (Swc_Tree_Node_Hit_Test(tn, 49, 52, 0.0) == FALSE) {
    PRINT_EXCEPTION(":( Bug?", "Missing hit.");
    return 1;        
  }

  if (Swc_Tree_Node_Hit_Test(tn, 50, 49, 0.0) == FALSE) {
    PRINT_EXCEPTION(":( Bug?", "Missing hit.");
    return 1;        
  }

  if (Swc_Tree_Node_Hit_Test(tn, 50, 50, 0.5) == FALSE) {
    PRINT_EXCEPTION(":( Bug?", "Missing hit.");
    return 1;        
  }

  Kill_Swc_Tree(tree);

  return 0;
}

static int test_swc_tree_subtract() 
{
  const char *whole_file = "../data/test/swc/subtract/whole1.swc";
  const char *sub_file = "../data/test/swc/subtract/sub1.swc";
  if (fexist(whole_file)) {
  Swc_Tree *tree1 = Read_Swc_Tree(whole_file);
  Swc_Tree *tree2 = Read_Swc_Tree(sub_file);
  Swc_Tree_Subtract(tree1, tree2);

  if ((Swc_Tree_Node_Is_Virtual(tree1->root) == FALSE) ||
      (Swc_Tree_Node_Id(tree1->root->first_child) != 2) ||
      (Swc_Tree_Node_Fsize(tree1->root) != 2)) {
    PRINT_EXCEPTION(":( Bug?", "Subtraction error.");
    return 1;
  }

  Kill_Swc_Tree(tree1);
  Kill_Swc_Tree(tree2);
  } else {
    printf("%s cannot be found\n", whole_file);
  }

  return 0;
}

static int test_swc_tree_number()
{
  Swc_Tree *tree = New_Swc_Tree();

  if (Swc_Tree_Number(tree) != 0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree number.");
    return 1;    
  }

  Swc_Node *sn = New_Swc_Node();
  sn->id = 1;
  
  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  if (Swc_Tree_Number(tree) != 1) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree number.");
    return 1;    
  }

  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));

  if (Swc_Tree_Number(tree) != 3) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected tree number.");
    return 1;    
  }

  Kill_Swc_Node(sn);
  Kill_Swc_Tree(tree);

  return 0;
}

static int test_swc_tree_branch_number()
{
  Swc_Tree *tree = New_Swc_Tree();

  if (Swc_Tree_Branch_Number(tree) != 0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected number of branches.");
    return 1;    
  }

  Swc_Node *sn = New_Swc_Node();
  sn->id = 1;
  
  tree->root = Make_Virtual_Swc_Tree_Node();
  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  if (Swc_Tree_Branch_Number(tree) != 0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected number of branches.");
    return 1;    
  }

  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tree->root, Make_Swc_Tree_Node(sn));

  if (Swc_Tree_Branch_Number(tree) != 0) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected number of branches.");
    return 1;    
  }

  Swc_Tree_Node_Add_Child(tree->root->first_child, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tree->root->first_child->next_sibling, 
      Make_Swc_Tree_Node(sn));

  if (Swc_Tree_Branch_Number(tree) != 2) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected number of branches.");
    return 1;    
  }

  Swc_Tree_Node_Add_Child(tree->root->first_child, Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tree->root->first_child->first_child, 
      Make_Swc_Tree_Node(sn));
  Swc_Tree_Node_Add_Child(tree->root->first_child, Make_Swc_Tree_Node(sn));

  if (Swc_Tree_Branch_Number(tree) != 4) {
    PRINT_EXCEPTION(":( Bug?", "Unexpected number of branches.");
    return 1;    
  }

  Kill_Swc_Node(sn);
  Kill_Swc_Tree(tree);

  return 0;
}

__END_DECLS

#endif
