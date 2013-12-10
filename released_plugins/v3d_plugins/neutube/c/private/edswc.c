static void geo3d_scalar_field_export_marker(const Geo3d_Scalar_Field *field, 
    const char *swcfile)
{
  FILE *fp = GUARDED_FOPEN(Get_String_Arg("-o"), "w");
  int i;
  for (i = 0; i < field->size; i++) {
    fprintf(fp, "%g,%g,%g,%g,0,,,,,,\n", field->points[i][0], 
        field->points[i][1], field->points[i][2], 
        Cube_Root(0.75 / TZ_PI * field->values[i]));
  }
  fclose(fp);
}

static void geo3d_scalar_field_export_swc(const Geo3d_Scalar_Field *field, 
    const char *swcfile)
{
  FILE *fp = GUARDED_FOPEN(Get_String_Arg("-o"), "w");
  Swc_Node tmp_node;
  int i;
  for (i = 0; i < field->size; i++) {
    tmp_node.x = field->points[i][0];
    tmp_node.y = field->points[i][1];
    tmp_node.z = field->points[i][2];
    tmp_node.id = i + 1;
    tmp_node.parent_id = -1;
    tmp_node.d = Cube_Root(0.75 / TZ_PI * field->values[i]);
    Swc_Node_Fprint(fp, &tmp_node);
  }
  fclose(fp);
}

static void swc_tree_subtract(Swc_Tree *tree, char * const *filepath, 
    int nfile)
{
  if (nfile > 1) {
    Swc_Tree *sub = Read_Swc_Tree(filepath[1]);
    int k;
    for (k = 2; k < nfile; k++) {
      Swc_Tree *tmp = Read_Swc_Tree(filepath[k]);
      Swc_Tree_Merge(sub, tmp);
      Kill_Swc_Tree(tmp);
    } 
    Swc_Tree_Subtract(tree, sub);	  
  }
}

static void swc_tree_translate(Swc_Tree *tree, double x, double y, double z)
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

static void swc_tree_clean_small(Swc_Tree *tree, double threshold)
{
  Swc_Tree_Node *tn = tree->root;
  int n = 0;
  int total = 0;
  if (Swc_Tree_Node_Is_Virtual(tn)) {
    Swc_Tree_Node *child = tn->first_child;
    TZ_ASSERT(Swc_Tree_Node_Is_Regular(child), "virtual");
    tn = child;
    while (tn != NULL) {
      total++;
      Swc_Tree tmp_tree;
      Swc_Tree_Node *tmp_tn1 = tn->next_sibling;
      Swc_Tree_Node *tmp_tn2 = tn->parent;
      tn->next_sibling = NULL;
      tn->parent = NULL;
      tmp_tree.root = tn;
      double length = Swc_Tree_Overall_Length(&tmp_tree);
      tn->next_sibling = tmp_tn1;
      tn->parent = tmp_tn2;
      child = tn->next_sibling;
      //printf("%g\n", length);
      if (length < threshold) {
        n++;
        Swc_Tree_Node_Detach_Parent(tn);
        Swc_Tree_Node_Kill_Subtree(tn);
      }
      tn = child;
    }
  }
  printf("%d trees removed from %d trees.\n", n, total);
}

static void swc_tree_resize(Swc_Tree *tree, double x_scale, double y_scale,
    double z_scale)
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = tree->root;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      Swc_Tree_Node_Data(tn)->x *= x_scale;
      Swc_Tree_Node_Data(tn)->y *= y_scale;
      Swc_Tree_Node_Data(tn)->z *= z_scale;
      Swc_Tree_Node_Data(tn)->d *= sqrt(x_scale * y_scale);
    }
  }
}

static void swc_tree_decompose(Swc_Tree *tree, const char *prefix) 
{
  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = tree->root;
  int index = 0;
  char filepath[500];
  BOOL start = TRUE;
  int length = 0;
  FILE *fp = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (start) {
        if (fp != NULL) {
          fclose(fp);
        }
        sprintf(filepath, "%s%0*d.swc", prefix,  4, index);
        fp = fopen(filepath, "w");
        start = FALSE;
        if (Swc_Tree_Node_Is_Branch_Point(tn->parent)) {
          tn->parent->node.parent_id = -1;
          Swc_Node_Fprint(fp, &(tn->parent->node));
        } else {
          tn->node.parent_id = -1;
        }
        length = 0;
      }

      Swc_Node_Fprint(fp, &(tn->node));
      length++;

      if (Swc_Tree_Node_Is_Leaf(tn)) {
        start = TRUE;
        index++;
      } else {
        if (length > 1) {
          if (Swc_Tree_Node_Is_Branch_Point(tn)) {
            start = TRUE;
            index++;
          }
        }
      }
    }
  }
}
