/* private functions for swc tree */

/* Calculate positions of key nodes in the dendrogram of <tree>, which is a
 * reduced tree. */
static void swc_tree_dendrogram_position(Swc_Tree *tree, int max_vx, int max_vy,
					 double *x, double *y, double *dm,
					 double *xscale)
{
  int count = Swc_Tree_Iterator_Start(tree, 1, TRUE) + 1;
  
  /* alloc <decided> */
  uint8 *decided = u8array_calloc(count);
  
  /* alloc <isleaf> */
  uint8 *isleaf = u8array_calloc(count);

  /* the left-top corner of the bounding box is <margin> */
  double margin = 10.0;
  x[1] = 0.0;
  x[2] = 0.0;
  
  Swc_Tree_Node *root = tree->root;
  
  Swc_Tree_Node *node = root;
  int leaf_count = 0;
  double xmax = x[1];

  /* Calculate the x positions and the right most position. */
  /* x(tn) = x(p(tn)) + length(p(tn)->tn) */
  while ((node = node->next) != NULL) {
    x[Swc_Tree_Node_Data(node)->label] = node->weight + 
      x[Swc_Tree_Node_Data(node->parent)->label];
    xmax = dmax2(x[Swc_Tree_Node_Data(node)->label], xmax);
    if (node->first_child == NULL) {
      isleaf[Swc_Tree_Node_Data(node)->label] = 1;
      leaf_count++;
    }
  }

  
  *dm = 10.0;
  if (leaf_count > 1) {
    *dm = ((double) max_vy - 2.0 * margin) / (leaf_count - 1);
  }

  *xscale = (max_vx - margin * 2) / xmax;

  node = root;
  x[1] += margin;
  double cur_y = margin;
  //double ymax = 0.0;

  /* Calculate y positions of the leaves. */
  while ((node = node->next) != NULL) {
    if (Swc_Tree_Node_Label(node) > 1) {
      x[Swc_Tree_Node_Label(node)] = x[Swc_Tree_Node_Label(node)] * *xscale +
        margin;
      if (isleaf[Swc_Tree_Node_Data(node)->label]) {
        y[Swc_Tree_Node_Data(node)->label] = cur_y;
        decided[Swc_Tree_Node_Data(node)->label] = 1;
        cur_y += *dm;
      }
    }
  }
  
  Swc_Tree_Iterator_Start(tree, 2, FALSE);
  Swc_Tree_Iterator_Start(tree, -1, FALSE);
  /* Calculate the y positions by traversing backward */
  while ((node = Swc_Tree_Next(tree)) != NULL) {
    double tmp_miny = -1.0;
    double tmp_maxy = -1.0;
    if (decided[Swc_Tree_Node_Data(node)->label] == 0) {
      Swc_Tree_Node *child = node->first_child;
      int n = 0;
      cur_y = 0.0;
      while (child != NULL) {
        //cur_y += y[Swc_Tree_Node_Data(child)->label];
        cur_y = y[Swc_Tree_Node_Data(child)->label];
        if (tmp_miny < 0.0) {
          tmp_miny = cur_y;
          tmp_maxy = cur_y;
        } else {
          if (tmp_miny > cur_y) {
            tmp_miny = cur_y;
          } else if (tmp_maxy < cur_y) {
            tmp_maxy = cur_y;
          }
        }
        n++;
        child = child->next_sibling;
      }
      //y[Swc_Tree_Node_Data(node)->label] = cur_y / n;
      y[Swc_Tree_Node_Data(node)->label] = (tmp_miny + tmp_maxy) / 2.0;
      decided[Swc_Tree_Node_Data(node)->label] = 1;
    }
  }

  x[0] = x[1];
  y[0] = y[1];

  /* free <decided> */
  free(decided);
  
  /* free <isleaf> */
  free(isleaf);
}
