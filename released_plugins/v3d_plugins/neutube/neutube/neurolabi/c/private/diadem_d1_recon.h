static void swc_tree_node_vec(Swc_Tree_Node *tn, double range, double *vec)
{
  Swc_Tree_Node_Pos(tn, vec);

  double length = Swc_Tree_Node_Length(tn);
  coordinate_3d_t pos;
  int n = 1;

  while ((tn = tn->parent) != NULL) {
    Swc_Tree_Node_Pos(tn, pos);
    Coordinate_3d_Add(vec, pos, vec);
    length += Swc_Tree_Node_Length(tn);
    n++;
    if ((Swc_Tree_Node_Is_Continuation(tn) == FALSE) || 
	(length > range)) {
      break;
    }
  }
   
  Coordinate_3d_Scale(pos, n);
  Coordinate_3d_Sub(vec, pos, vec); 
}

static double swc_tree_node_connect_strength(Swc_Tree_Node *tn1,
					     Swc_Tree_Node *tn2)
{
  coordinate_3d_t tn1_vec;
  coordinate_3d_t tn12_vec;
  swc_tree_node_vec(tn1, 30.0, tn1_vec);
  coordinate_3d_t pos1, pos2;
  Swc_Tree_Node_Pos(tn1, pos1);
  Swc_Tree_Node_Pos(tn2, pos2);
  Coordinate_3d_Sub(pos2, pos1, tn12_vec);
  Coordinate_3d_Unitize(tn1_vec);
  Coordinate_3d_Unitize(tn12_vec);
  return (1.0 + Coordinate_3d_Dot(tn1_vec, tn12_vec)) / (1.0 + Swc_Tree_Node_Dist(tn1, tn2));
}

static Swc_Tree_Node* swc_tree_continuation(Swc_Tree_Node *seeker, 
					    Swc_Tree *forest)
{
  coordinate_3d_t seeker_vector;
  coordinate_3d_t seeker_pos;
  const double length_thre = 30.0;
  
  Swc_Tree_Node_Pos(seeker, seeker_vector);
  Swc_Tree_Node_Pos(seeker, seeker_pos);

  Swc_Tree_Node *tn = seeker;
  double length = Swc_Tree_Node_Length(seeker);
  coordinate_3d_t pos;
  int n = 1;

  while ((tn = tn->parent) != NULL) {
    Swc_Tree_Node_Pos(tn, pos);
    Coordinate_3d_Add(seeker_vector, pos, seeker_vector);
    length += Swc_Tree_Node_Length(tn);
    n++;
    if ((Swc_Tree_Node_Is_Continuation(tn) == FALSE) || 
	(length > length_thre)) {
      break;
    }
  }
 
  TZ_ASSERT(n > 1, "Invalid seeker");
   
  Coordinate_3d_Scale(pos, n);
  Coordinate_3d_Sub(seeker_vector, pos, seeker_vector);
  Coordinate_3d_Unitize(seeker_vector);
 
  Swc_Tree_Iterator_Start(forest, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  //double min_dist = Infinity;
  double max_conn = 0.0;

  Swc_Tree_Node *target = NULL;
  coordinate_3d_t vec;

  while ((tn = Swc_Tree_Next(forest)) != NULL) {
    if (Swc_Tree_Node_Is_Leaf(tn) || 
	(Swc_Tree_Node_Is_Regular_Root(tn) && 
	 Swc_Tree_Node_Child_Number(tn) == 1)) {
      Swc_Tree_Node_Pos(tn, pos);
      coordinate_3d_t target_vec;
      if (Swc_Tree_Node_Is_Leaf(tn)) {
	Swc_Tree_Node_Vector(tn, SWC_TREE_NODE_BACKWARD, target_vec);
      } else {
	Swc_Tree_Node_Vector(tn, SWC_TREE_NODE_FORWARD, target_vec);
      }
      Coordinate_3d_Unitize(seeker_vector);
      Coordinate_3d_Unitize(target_vec);
      Coordinate_3d_Sub(pos, seeker_pos, vec);
      Coordinate_3d_Unitize(vec);

      if (Coordinate_3d_Dot(seeker_vector, target_vec) > 0.0) {
	if (Coordinate_3d_Dot(seeker_vector, vec) > 0.5) {
	  double conn = swc_tree_node_connect_strength(seeker, tn);
	  if (conn > max_conn) {
	    max_conn = conn;
	    target = tn;
	  }
				
	  /*
	  double dist = Coordinate_3d_Distance(seeker_pos, pos);
	  if (dist < min_dist) {
	    min_dist = dist;
	    target = tn;
	  }
	  */
	}
      }
    }
  }

  return target;
}

static int swc_tree_node_regular_neghbor_number(Swc_Tree_Node *tn)
{
  int n = Swc_Tree_Node_Child_Number(tn);
  
  if (Swc_Tree_Node_Is_Regular(tn->parent)) {
    n++;
  }

  return n;
}

static void swc_tree_set_up_root(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Node *root = NULL;
  double miny = Infinity;

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Is_Root(tn)) {
	if (root != NULL) {
	  Swc_Tree_Node_Set_Root(root);
	  miny = Infinity;
	}
      }

      if (swc_tree_node_regular_neghbor_number(tn) == 1) {
	if (Swc_Tree_Node_Data(tn)->y < miny) {
	  miny = Swc_Tree_Node_Data(tn)->y;
	  root = tn;
	}
      }
    }
  }

  if (root != NULL) {
    Swc_Tree_Node_Set_Root(root);
    if (Swc_Tree_Node_Is_Virtual(tree->root)) {
      Swc_Tree_Node_Set_Parent(root, tree->root);
    } else {
      tree->root = root;
    }
  }
}

static void swc_tree_set_type(Swc_Tree *tree, int type)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    Swc_Tree_Node_Data(tn)->type = type;
  }
}

static void swc_tree_down_sample(Swc_Tree *tree, double dist)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = tree->begin;
  while (tn != NULL) {
    Swc_Tree_Node *next = tn->next;
    if (Swc_Tree_Node_Is_Continuation(tn)) {
      if (Swc_Tree_Node_Length(tn) < dist) {
	Swc_Tree_Node_Merge_To_Parent(tn);
      }
    }
    tn = next;
  }
}

static Swc_Tree_Node* swc_tree_node_regular_root(Swc_Tree_Node *tn)
{
  if ((tn == NULL) || Swc_Tree_Node_Is_Virtual(tn)) {
    return NULL;
  }

  Swc_Tree_Node *root = tn;
  while (Swc_Tree_Node_Is_Regular_Root(root) == FALSE) {
    root = root->parent;
  }

  return root;
}

static BOOL swc_tree_node_sharing_regular_root(Swc_Tree_Node *tn1, 
					       Swc_Tree_Node *tn2)
{
  Swc_Tree_Node *root1 = swc_tree_node_regular_root(tn1);
  Swc_Tree_Node *root2 = swc_tree_node_regular_root(tn2);
  
  return (root1 == root2);
}

static void swc_tree_label_color(Swc_Tree *tree)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  int type = 0;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular_Root(tn)) {
      type++;
      type = type % 12;
    }
    Swc_Tree_Node_Data(tn)->type = type;    
  }
}

Swc_Tree_Node *swc_tree_query_by_id(Swc_Tree *tree, int id)
{
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);

  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Id(tn) == id) {
      break;
    }
  }

  return tn;
}

static int swc_tree_break_cross(Swc_Tree *tree)
{
  Swc_Tree_Resort_Id(tree);
  Swc_Tree *tree2 = Copy_Swc_Tree(tree);
  Swc_Tree_Reduce(tree2);
 
  Swc_Tree_Iterator_Start(tree2, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  int changed = 0;
  while ((tn = Swc_Tree_Next(tree2)) != NULL) {
    if (Swc_Tree_Node_Is_Branch_Point(tn)) {
      if (Swc_Tree_Node_Is_Branch_Point(tn->first_child)) {
	if (Swc_Tree_Node_Length(tn->first_child) < 200.0) {
	  if (Swc_Tree_Node_Length(tn->first_child->first_child) + 
	      Swc_Tree_Node_Length(tn->first_child->first_child->next_sibling)
	      > 800) { 
	    Swc_Tree_Node *tn2 = 
	      swc_tree_query_by_id(tree, Swc_Tree_Node_Id(tn->first_child));
	    if (tn2 != NULL) {
	      Swc_Tree_Node_Detach_Parent(tn2);
	      Swc_Tree tmp_tree;
	      tmp_tree.root = tn2;
	      swc_tree_set_up_root(&tmp_tree);
	      Swc_Tree_Merge(tree, &tmp_tree);
	      changed = 1;
	      break;
	    }
	  }
	}
      }
    }
  }

  Kill_Swc_Tree(tree2);

  return changed;
}

Cuboid_I* read_tile_array(const char *path, int *n)
{
  *n = File_Line_Number(path, FALSE);
  Cuboid_I *boxes;
  GUARDED_MALLOC_ARRAY(boxes, *n, Cuboid_I);
  *n = 0;
  String_Workspace *sw = New_String_Workspace();
  char *line = NULL;
  FILE *fp = GUARDED_FOPEN(path, "r");
  int value[10];
  while ((line = Read_Line(fp, sw)) != NULL) {
    int k;
    String_To_Integer_Array(line, value, &k);
    Cuboid_I_Set_S(boxes + *n, value[2], value[3], 0, value[4], value[5], 1);
    (*n)++;
  }

  fclose(fp);
  Kill_String_Workspace(sw);

  return boxes;
}

Cuboid_I* read_tile_array2(const char *path, int *n)
{
  *n = File_Line_Number(path, FALSE);
  Cuboid_I *boxes;
  GUARDED_MALLOC_ARRAY(boxes, ((*n) + 1), Cuboid_I);
  *n = 0;
  String_Workspace *sw = New_String_Workspace();
  char *line = NULL;
  FILE *fp = GUARDED_FOPEN(path, "r");
  int value[10];
  while ((line = Read_Line(fp, sw)) != NULL) {
    int k;
    String_To_Integer_Array(line, value, &k);
    Cuboid_I_Set_S(boxes + 1 + *n, value[1], value[2], value[3], 
		   1024, 1024, 1);
    (*n)++;
  }

  fclose(fp);
  Kill_String_Workspace(sw);
 
  return boxes;
}

static Swc_Tree* swc_tree_reconnect(Swc_Tree *tree)
{
  Swc_Tree_Node *tn = NULL;
  Swc_Tree_Resort_Id(tree);

  Swc_Tree *tree2 = Copy_Swc_Tree(tree);
  int n = Swc_Tree_Iterator_Start(tree2, SWC_TREE_ITERATOR_DEPTH_FIRST, TRUE);

  Swc_Tree_Node **tn_array2;
  GUARDED_MALLOC_ARRAY(tn_array2, n+1, Swc_Tree_Node*);

  while ((tn = Swc_Tree_Next(tree2)) != NULL) {
    tn_array2[Swc_Tree_Node_Id(tn)] = tn;
  }

  Swc_Tree_Node *best_pair[2];
  
  BOOL stop = FALSE;

  while (stop == FALSE) {
    best_pair[0] = NULL;
    best_pair[1] = NULL;
    double max_conn = 0.0;
    stop = TRUE;

    /* Find matches. */
    Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
    while ((tn = Swc_Tree_Next(tree)) != NULL) {
      if (Swc_Tree_Node_Is_Leaf(tn)) {
	coordinate_3d_t vec;
	swc_tree_node_vec(tn, 30.0, vec);
	Coordinate_3d_Unitize(vec);
	if (vec[1] > 0.0) {
	  //Swc_Tree_Node_Data(tn)->type = 5;	  
	  Swc_Tree_Node *match = swc_tree_continuation(tn, tree2);

	  tn = tn_array2[Swc_Tree_Node_Id(tn)];

	  if (swc_tree_node_sharing_regular_root(match, tn) == FALSE) {
	    if (match != NULL) {
	      double conn = swc_tree_node_connect_strength(tn, match);
	      if (conn > max_conn) {
		max_conn = conn;
		best_pair[0] = tn;
		best_pair[1] = match;
	      }
	    
	      /*
		if (Swc_Tree_Node_Dist(tn, match) < 200.0) {
		Swc_Tree_Node_Data(match)->type = 5;
		Swc_Tree_Node_Set_Root(match);
		Swc_Tree_Node_Set_Parent(match, tn);
		}
	      */
	    }
	  }
	}
      }
    }

    if (best_pair[0] != NULL) {
      /*
      Print_Swc_Tree_Node(best_pair[0]);
      printf("%g\n", Swc_Tree_Node_Dist(best_pair[0], best_pair[1]));
      */
      if (Swc_Tree_Node_Dist(best_pair[0], best_pair[1]) < 200.0) {
	Swc_Tree_Node_Data(best_pair[1])->type = 5;
	Swc_Tree_Node_Set_Root(best_pair[1]);
	Swc_Tree_Node_Set_Parent(best_pair[1], best_pair[0]);
	Kill_Swc_Tree(tree);
	tree = Copy_Swc_Tree(tree2);
	stop = FALSE;
      }
    }
  }

  //swc_tree_break_cross(tree2);

  while (swc_tree_break_cross(tree2));
  Swc_Tree_Remove_Terminal_Branch(tree2, 30.0);

  return tree2;
}

static void stack_label_collapse(Stack *stack)
{
  int *hist = Stack_Hist(stack);
  int n = Int_Histogram_Max(hist) + 1; 
  int *map = iarray_malloc(n);

  int start = Int_Histogram_Min(hist);
  if (start == 0) {
    start = 1;
  }
  int index = 1;
  n = Int_Histogram_Length(hist);
  const int *array = Int_Histogram_Const_Array(hist);
  map[0] = 0;
  int i;
  for (i = 0; i < n; i++) {
    if (array[i] > 0) {
      map[start + i] = index++;
    }
  }
  free(hist);

  int nvoxel = Stack_Voxel_Number(stack);
  for (i = 0; i < nvoxel; i++) {
    if (stack->array[i] > 0) {
      stack->array[i] = map[stack->array[i]];
    }
  }
}

static void swc_tree_seed_mask(Swc_Tree *tree, Stack *stack, 
			       const double *offset)
{
  Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Root(tn) == FALSE) {
      Local_Neuroseg locseg;
      Swc_Tree_Node_To_Locseg(tn, &locseg);
      Local_Neuroseg_Translate(&locseg, offset);
      Local_Neuroseg_Label_G(&locseg, stack, 0, Swc_Tree_Node_Data(tn)->label,
			     1.0);
    }
  }
 
  
}
