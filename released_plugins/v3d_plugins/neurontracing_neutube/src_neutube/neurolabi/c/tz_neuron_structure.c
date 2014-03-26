/* tz_neuron_structure.c
 * @author Ting Zhao
 * @date 10-Sep-2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tz_error.h"
#include "tz_u8array.h"
#include "tz_neuron_structure.h"
#include "tz_locseg_chain.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_geo3d_circle.h"
#include "tz_geo3d_ball.h"
#include "tz_geo3d_utils.h"
#include "tz_xml_utils.h"
#include "tz_string.h"
#include "tz_utilities.h"

Neuron_Structure* New_Neuron_Structure()
{
  Neuron_Structure *ns = (Neuron_Structure *)
    Guarded_Malloc(sizeof(Neuron_Structure), "New_Neuron_Structure");
  ns->graph = NULL;
  ns->conn = NULL;
  ns->comp = NULL;

  return ns;
}

Neuron_Structure* Make_Neuron_Structure(int ncomp)
{
  Neuron_Structure *ns = New_Neuron_Structure();

  ns->graph = Make_Graph(ncomp, 1, FALSE);
  ns->conn = New_Neurocomp_Conn(); /* must have the same cacapciaty as graph */
  ns->comp = Make_Neuron_Component_Array(ncomp);

  return ns;
}

void Delete_Neuron_Structure(Neuron_Structure *ns)
{
  free(ns);
}

void Clean_Neuron_Structure(Neuron_Structure *ns)
{
  if (ns->comp != NULL) {
    Clean_Neuron_Component_Array(ns->comp, 
				 Neuron_Structure_Component_Number(ns));
    free(ns->comp);
  }
  ns->comp = NULL;
  if (ns->graph != NULL) {
    Kill_Graph(ns->graph);
  }
  ns->graph = NULL;
  if (ns->conn != NULL) {
    free(ns->conn);
  }
  ns->conn = NULL;
}

void Kill_Neuron_Structure(Neuron_Structure *ns)
{
  Clean_Neuron_Structure(ns);
  free(ns);
}

void Print_Neuron_Structure(const Neuron_Structure *ns)
{
  int i;
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    printf("%d -- %d ", ns->graph->edges[i][0], ns->graph->edges[i][1]);
    Print_Neurocomp_Conn(ns->conn + i);
  }
}

int Neuron_Structure_Link_Number(const Neuron_Structure *ns)
{
  return NEURON_STRUCTURE_LINK_NUMBER(ns);
}

int Neuron_Structure_Component_Number(const Neuron_Structure *ns)
{
  return NEURON_STRUCTURE_COMPONENT_NUMBER(ns);
}

Neurocomp_Conn* Neuron_Structure_Get_Conn(const Neuron_Structure *ns, 
					  int index)
{
  TZ_ASSERT(index < NEURON_STRUCTURE_LINK_NUMBER(ns), 
	    "Index out of bound");

  return ns->conn + index;
}

void Neuron_Structure_Add_Conn(Neuron_Structure *ns, 
			       int id1, int id2, const Neurocomp_Conn *conn)
{
  TZ_ASSERT(ns->graph != NULL, "null graph");

  int old_capacity = ns->graph->edge_capacity;
  Graph_Add_Edge(ns->graph, id1, id2);
  if (old_capacity < ns->graph->edge_capacity) {
    ns->conn = (Neurocomp_Conn *) 
      Guarded_Realloc(ns->conn, 
		      sizeof(Neurocomp_Conn) * ns->graph->edge_capacity, 
		      "Neuron_Structure_Add_Conn");
  }
  Neurocomp_Conn_Copy(ns->conn + Neuron_Structure_Link_Number(ns) -1, conn);
}

void Neuron_Structure_Remove_Conn(Neuron_Structure *ns, int id1, int id2)
{
  if (ns->graph != NULL) {
    int i;
    for (i = 0; i < GRAPH_EDGE_NUMBER(ns->graph); i++) {
      if ((ns->graph->edges[i][0] == id1) && (ns->graph->edges[i][1] == id2)) {
	int remain = GRAPH_EDGE_NUMBER(ns->graph) - i - 1;
	Graph_Remove_Edge(ns->graph, i);
	if (remain > 0) {
	  memmove(ns->conn + i, ns->conn + i + 1, 
		  remain * sizeof(Neurocomp_Conn));
	}
	break;
      }
    }
  }
}

void Neuron_Structure_Set_Component_Array(Neuron_Structure *ns,
					  Neuron_Component *comp, int n)
{
  if (ns->comp != NULL) {
    Clean_Neuron_Component_Array(ns->comp, 
				 Neuron_Structure_Component_Number(ns));
    free(ns->comp);
  }

  ns->comp = comp;

  if (ns->graph != NULL) {
    Kill_Graph(ns->graph);
  }
  ns->graph = Make_Graph(n, 0, FALSE);
}

void Neuron_Structure_Set_Component(Neuron_Structure *ns, int index, 
				    const Neuron_Component *comp)
{
  if (index >= NEURON_STRUCTURE_COMPONENT_NUMBER(ns)) {
    NEURON_STRUCTURE_COMPONENT_NUMBER(ns) = index + 1;
    GUARDED_REALLOC_ARRAY(ns->comp, index + 1, Neuron_Component);
  }

  /*
  TZ_ASSERT(index < NEURON_STRUCTURE_COMPONENT_NUMBER(ns), 
	    "Index out of range");
  */

  ns->comp[index] = *comp;
}

Neuron_Component* Neuron_Structure_Get_Component(const Neuron_Structure *ns, 
						 int index)
{
  TZ_ASSERT(index < NEURON_STRUCTURE_COMPONENT_NUMBER(ns), 
	    "Index out of range");

  return ns->comp + index;
}

/*
 * Process_Neuron_Structure() removes redundant edges in <ns>.
 */
void Process_Neuron_Structure(Neuron_Structure *ns)
{
  /* alloc <edge_mask> */
  uint8_t *edge_mask = u8array_malloc(Neuron_Structure_Link_Number(ns));
  int i;

  //Set edge mask to 2 for link connection, 1 for hook-loop connection and
  //0 for no connection
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    if (ns->conn[i].mode == NEUROCOMP_CONN_LINK) {
      edge_mask[i] = 2;
    } else if (ns->conn[i].mode == NEUROCOMP_CONN_HL) {
      edge_mask[i] = 1;
    } else {
      edge_mask[i] = 0;
    }
  }

  int j;
  /* Remove duplicated hook-loops by keeping the one with smaller cost */
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    if (edge_mask[i] == 1) {
      for (j = i + 1; j < NEURON_STRUCTURE_LINK_NUMBER(ns); j++) {
	if (edge_mask[j] == 1) {
	  if (((ns->graph->edges[i][0] == ns->graph->edges[j][1]) && 
	       (ns->graph->edges[i][1] == ns->graph->edges[j][0]))) {
	    if (ns->conn[i].cost < ns->conn[j].cost) {
	      edge_mask[j] = 0;
	    } else {
	      edge_mask[i] = 0;
	    }
	  }
	}
      }
    }
  }
  
  /* Keep the smallest edge for multiple-loop connections */
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    if (edge_mask[i] == 1) {
      for (j = i + 1; j < NEURON_STRUCTURE_LINK_NUMBER(ns); j++) {
	if (edge_mask[j] == 1) {
	  if (((ns->graph->edges[i][0] == ns->graph->edges[j][0]) && 
	       (ns->conn[i].info[0] == ns->conn[j].info[0]))) {
	    if (ns->conn[i].cost < ns->conn[j].cost) {
	      edge_mask[j] = 0;
	    } else {
	      edge_mask[i] = 0;
	    }
	  }
	}
      }
    }
  }

  //Move the edges into a compact array
  int nedge = 0;
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    if (edge_mask[i] > 0) {
      if (i != nedge) {
	ns->graph->edges[nedge][0] = ns->graph->edges[i][0];
	ns->graph->edges[nedge][1] = ns->graph->edges[i][1];
	Neurocomp_Conn_Copy(ns->conn + nedge, ns->conn + i);
      }
      nedge++;
    }
  }
  NEURON_STRUCTURE_LINK_NUMBER(ns) = nedge;

  /* free <edge_mask> */
  free(edge_mask);
}


void Neuron_Structure_To_Swc_File(const Neuron_Structure *ns, 
				  const char *file_path)
{
  switch (ns->comp->type) {
  case NEUROCOMP_TYPE_LOCSEG_CHAIN:
    {
      int i;

      /* alloc <parent_id> */
      int *parent_id = iarray_malloc(ns->graph->nvertex);
  
      /* alloc <direction> */
      Dlist_Direction_e *direction = (Dlist_Direction_e *) 
	malloc(sizeof(Dlist_Direction_e) * ns->graph->nvertex);

      /* alloc <start_id> */
      int *start_id = iarray_malloc(ns->graph->nvertex + 1);
  
      start_id[0] = 1;
      for (i = 0; i < ns->graph->nvertex; i++) {
	parent_id[i] = -1;
	direction[i] = DL_FORWARD;
	Neuron_Component *nc = Neuron_Structure_Get_Component(ns, i);
	start_id[i + 1] = start_id[i] + 
	  Locseg_Chain_Length(NEUROCOMP_LOCSEG_CHAIN(nc)) + 2;
      }

      for (i = 0; i < ns->graph->nedge; i++) {
	if (ns->conn[i].mode == NEUROCOMP_CONN_HL) {
	  int hook_idx = ns->graph->edges[i][0];
	  int loop_idx = ns->graph->edges[i][1];
	  if (parent_id[hook_idx] < 0) {
#ifdef _DEBUG_2
	    printf("id: %d %d\n", ns->conn[i].info[1], start_id[loop_idx]);
#endif
	    parent_id[hook_idx] = ns->conn[i].info[1] + 1 + 
	      start_id[loop_idx];
	    if (ns->conn[i].info[0] == 1) {
	      direction[hook_idx] = DL_BACKWARD;
	    }
	  }
	}
      }

#ifdef _DEBUG_2
      iarray_print2(parent_id, ns->graph->nvertex + 1, 1);
#endif

      FILE *fp = fopen(file_path, "w");

      int type = 1;
      for (i = 0; i < ns->graph->nvertex; i++) {
	Neuron_Component *nc = Neuron_Structure_Get_Component(ns, i);
#ifdef _DEBUG_2
	printf("parent_id: %d\n", parent_id[i]);
#endif
	if (NEUROCOMP_LOCSEG_CHAIN(nc) != NULL) {
	  Locseg_Chain_Swc_Fprint(fp, NEUROCOMP_LOCSEG_CHAIN(nc), type % 5,
				  start_id[i], parent_id[i], direction[i]);
	  type++;
	}
      }

      fclose(fp);
			  
      /* free <parent_id> */
      free(parent_id);

      /* free <direction> */
      free(direction);
  
      /* free <start_id> */
      free(start_id);
    }
    break;
  case NEUROCOMP_TYPE_GEO3D_CIRCLE:
    {
      Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns, 1.0, NULL);
      Swc_Tree_Resort_Id(tree);
      Write_Swc_Tree(file_path, tree);
      Kill_Swc_Tree(tree);
    }
    break;
  default:
    break;
  }
}

int Neuron_Structure_Find_Root_Circle(const Neuron_Structure *ns,
				      double *root_pos)
{
  Geo3d_Circle *circle = 
    NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, 0));
  

  if (circle == NULL) {
    return -1;
  }

  double min_dist = Geo3d_Dist_Sqr(root_pos[0], root_pos[1], root_pos[2],
				   circle->center[0], circle->center[1],
				   circle->center[2]);

  int i;
  int j = 0; /* index for min dist */
  for (i = 1; i < ns->graph->nvertex; i++) {
    circle = 
      NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, i));
    
    double dist = Geo3d_Dist_Sqr(root_pos[0], root_pos[1], root_pos[2],
				 circle->center[0], circle->center[1],
				 circle->center[2]);
    if (dist < min_dist) {
      min_dist = dist;
      j = i;
    }
  }

  return j;
}

void Neuron_Structure_To_Swc_File_Circle(const Neuron_Structure *ns, 
					 const char *file_path)
{
  Neuron_Structure_To_Swc_File_Circle_Z(ns, file_path, 1.0, NULL);
}

void Neuron_Structure_To_Swc_File_Circle_Z(const Neuron_Structure *ns, 
					   const char *file_path,
					   double z_scale, double *root_pos)
{
  FILE *fp = fopen(file_path, "w");

  /* new <gw> */
  Graph_Workspace *gw = New_Graph_Workspace();

  Geo3d_Circle *circle = NULL;
  
  /* Traverse the graph by breadth first */
  int i, j;

  Graph_Workspace_Prepare(gw, GRAPH_WORKSPACE_STATUS);
  Graph_Workspace_Load(gw, ns->graph);

  if (gw->status == NULL) {
    gw->status = u8array_malloc(ns->graph->nvertex);
  }

  for (i = 0; i < gw->nvertex; i++) {
    gw->status[i] = 0;
  }

  /* alloc <index> */
  int *index = iarray_malloc(ns->graph->nvertex);

  /* alloc <id_array> */
  int *id_array = iarray_malloc(gw->nvertex);

  for (i = 0; i < ns->graph->nvertex; i++) {
    index[i] = -1;
    id_array[i] = -1;
  }

  if (root_pos != NULL) {
    circle = 
	NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, 0));

    double min_dist = Geo3d_Dist_Sqr(root_pos[0], root_pos[1], root_pos[2],
				     circle->center[0], circle->center[1],
				     circle->center[2]);
    j = 0; /* index for min dist */
    for (i = 1; i < ns->graph->nvertex; i++) {
      circle = 
	NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, i));
      
      double dist = Geo3d_Dist_Sqr(root_pos[0], root_pos[1], root_pos[2],
				   circle->center[0], circle->center[1],
				   circle->center[2]);
      if (dist < min_dist) {
	min_dist = dist;
	id_array[j] = 0;
	j = i;
      } else {
	id_array[i] = 0;
      }
    }
  }

  int *index_pointer = index;
  int start_id = 1;

  int k;

  for (j = 0; j < ns->graph->nedge; j++) {
    for (k = 0; k < 2; k++) {
      int root = GRAPH_EDGE_NODE(ns->graph, j, k);

      if (id_array[root] < 0) { /* if the node has not been traversed */
	Arrayqueue aq = Graph_Traverse_B(ns->graph, root, gw);

	if (j == 0) {
	  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_CONNECTION, TRUE);
	}

	//int n = Arrayqueue_Max(&aq);

	/* Give each id an index so that the index of a node should be smaller than
	 * its children. */
	int n = Arrayqueue_To_Array(&aq, index_pointer);
  
	for (i = 0; i < n; i++) {
	  id_array[index_pointer[i]] = i + start_id;
	}

	/* Write the root. */
	/*
	  circle = 
	  NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, root));
	  Geo3d_Circle_Swc_Fprint_Z(fp, circle, root + 1, -1, z_scale);
	*/

	gw->status[root] = 1;

	index_pointer += n;
	start_id += n;
      }
    }
  }
  
#ifdef _DEBUG_
  printf("%d\n", gw->nvertex);
  iarray_print2(id_array, gw->nvertex, 1);
#endif

  int **neighbor_list = Graph_Neighbor_List(ns->graph, gw);
  
  for (i = 0; i < gw->nvertex; i++) {
    int vertex = index[i];
    if (vertex >= 0) {
      circle = 
	NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, vertex));
      int id1 = i + 1;
      if (gw->status[vertex] == 1){
	Geo3d_Circle_Swc_Fprint_Z(fp, circle, id1, -1, z_scale);
      } else {
	for (j = 1; j <= GRAPH_NUMBER_OF_NEIGHBORS(vertex, neighbor_list); j++) {
	  int id2 = id_array[GRAPH_NEIGHBOR_OF(vertex, j, neighbor_list)];
	  if (id1 > id2) {
	    Geo3d_Circle_Swc_Fprint_Z(fp, circle, id1, id2, z_scale);
	  }
	}
      }
    }
  }

#if 0
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    int id1 = GRAPH_EDGE_NODE(ns->graph, i, 0);
    int id2 = GRAPH_EDGE_NODE(ns->graph, i, 1);
    if ((index[id1] >= 0) && (index[id2] >= 0)) {
      /* swap id1 and id2 to make sure id1 is the parent of id2 */
      if (index[id1] > index[id2]) {
	int tmp;
	SWAP2(id1, id2, tmp);
      }
      
      circle = NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, id2));
      Geo3d_Circle_Swc_Fprint_Z(fp, circle, id2 + 1, id1 + 1, z_scale);
      gw->status[id2] = 1;
    }
  }

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_CONNECTION, FALSE);

  for (i = 0; i < gw->nvertex; i++) {
    if (gw->status[i] == 0) {
      circle = NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, i));
      Geo3d_Circle_Swc_Fprint_Z(fp, circle, i + 1, -1, z_scale);
    }
  }
#endif

  /* free <index> */
  free(index);

  /* free <id_array> */
  free(id_array);
  
  /* kill <gw> */
  Kill_Graph_Workspace(gw);

  fclose(fp);  
}

Swc_Tree* Neuron_Structure_To_Swc_Tree_Circle_Z(const Neuron_Structure *ns, 
						double z_scale, 
						double *root_pos)
{
  int ncomp = Neuron_Structure_Component_Number(ns);

  Swc_Tree_Node **tn_array = (Swc_Tree_Node**) 
    Guarded_Malloc(sizeof(Swc_Tree_Node*) * ncomp,
		   "Neuron_Structure_To_Swc_Tree_Circle_Z");

  Swc_Tree *tree = New_Swc_Tree();
  tree->root = Make_Virtual_Swc_Tree_Node();

  Swc_Tree_Node *root = NULL;
  double mindist = -1.0;

  int i;
  for (i = 0; i < ncomp; i++) {
    tn_array[i] = New_Swc_Tree_Node();
    //NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, i))->radius = 2.0;

    Geo3d_Circle_To_Swc_Node(NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, i)), 0, -1, z_scale, 2, &(tn_array[i]->node));

    Swc_Tree_Node_Data(tn_array[i])->id = 1;

    Swc_Tree_Node_Set_Parent(tn_array[i], tree->root);
    
    if (root_pos != NULL) {
      double pos[3];
      Swc_Tree_Node_Pos(tn_array[i], pos);
      double dist = Geo3d_Dist_Sqr(root_pos[0], root_pos[1], root_pos[2],
				   pos[0], pos[1], pos[2]);
      if ((mindist < 0.0) || (mindist > dist)) {
	mindist = dist;
	root = tn_array[i];
      }
    }
  }

  int nconn = Neuron_Structure_Link_Number(ns);

  for (i = 0; i < nconn; i++) {
    Swc_Tree_Node_Set_Root(tn_array[ns->graph->edges[i][1]]);
    Swc_Tree_Node_Add_Child(tn_array[ns->graph->edges[i][0]],
			    tn_array[ns->graph->edges[i][1]]);
    tn_array[ns->graph->edges[i][1]]->weight = ns->conn[i].cost;
  }

#ifdef _DEBUG_2
  for (i = 0; i < ncomp; i++) {
    if (tn_array[i]->parent == tree->root) {
      printf("missing node: %d\n", i);
    }
  }
#endif

  free(tn_array);

  if (root != NULL) {
    Swc_Tree_Node_Set_Root(root);
    if (root->parent != NULL) {
      if (root->parent->first_child != root) {
	Swc_Tree_Node *first_child = root->parent->first_child;
	Swc_Tree_Node_Detach_Parent(root);
	Swc_Tree_Node_Insert_Before(first_child, root);
      }
    }
  }

#ifdef _DEBUG_2
  printf("tree size: %d\n", Swc_Tree_Node_Fsize(tree->root));
#endif

  Swc_Tree_Iterator_Start(tree, 1, FALSE);
  Swc_Tree_Node *tn = NULL;
  while ((tn = Swc_Tree_Next(tree)) != NULL) {
    if (Swc_Tree_Node_Data(tn)->d == 0.0) {
      Swc_Tree_Node_Data(tn)->id = -1;
    }
  }

  //Swc_Tree_Resort_Id(tree);
  Swc_Tree_Regularize(tree);
  
#ifdef _DEBUG_2
  Print_Swc_Tree(tree);
#endif

  return tree;
}

static int locseg_chain_ns_circle_number(const Neuron_Structure *ns, 
					 int *start_id)
{
  int n = 0;
  int i;
  for (i = 0; i < NEURON_STRUCTURE_COMPONENT_NUMBER(ns); i++) {
    Neuron_Component *nc = Neuron_Structure_Get_Component(ns, i);
    start_id[i] = n;
    int length = Locseg_Chain_Length(NEUROCOMP_LOCSEG_CHAIN(nc));
    if (length > 0) {
      n += length + 2;
    }
  }

  return n;
}


/* the returned value must the length of \a chain plus 2 */
int Neuron_Structure_Break_Locseg_Chain(Neuron_Structure *ns, 
					Locseg_Chain *chain, int start_id)
{
  int n;
  Locseg_Chain_To_Neuron_Component(chain, NEUROCOMP_TYPE_GEO3D_CIRCLE,
				   ns->comp + start_id, &n);
  int i;
  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 0;
  conn.cost = 0.0;

  for (i = 1; i < n; i++) {
    Neuron_Structure_Add_Conn(ns, start_id + i - 1, start_id + i, &conn);
  }

  return n;
}

/* the returned value must the length of \a chain plus 2 */
int Neuron_Structure_Break_Locseg_Chain_S(Neuron_Structure *ns, 
					  Locseg_Chain *chain, int start_id,
					  double xy_scale, double z_scale)
{
  int n;
  Locseg_Chain_To_Neuron_Component_S(chain, NEUROCOMP_TYPE_GEO3D_CIRCLE,
				     ns->comp + start_id, &n, 
				     xy_scale, z_scale);
  if (n > 0) {
    int i;
    Neurocomp_Conn conn;
    conn.mode = NEUROCOMP_CONN_LINK;
    conn.info[0] = 0;
    conn.info[1] = 0;
    conn.cost = 0.0;
    
    for (i = 1; i < n; i++) {
      Neuron_Structure_Add_Conn(ns, start_id + i - 1, start_id + i, &conn);
    }
  }

  return n;
}

/*
 * 0: index
 * 1: head
 * 2: tail
 */
static void translate_mode(const Neurocomp_Conn *conn, int chain_length0,
			   int chain_length1,
			   int start_id0, int start_id1, int *id)
{
  switch (conn->mode) {
  case NEUROCOMP_CONN_HL:
    if (conn->info[0] == 0) {
      id[0] = start_id0;
    } else {
      id[0] = start_id0 + chain_length0 + 1;
    }
    id[1] = start_id1 + conn->info[1] + 1;
    break;
  case NEUROCOMP_CONN_LINK:
    if (conn->info[0] == 0) {
      id[0] = start_id0;
    } else {
      id[0] = start_id0 + chain_length0 + 1;
    }
    if (conn->info[1] == 0) {
      id[1] = start_id1;
    } else {
      id[1] = start_id1 + chain_length1 + 1;
    }
    break;
  default:
    return;
  }
}

#if 1
Neuron_Structure* 
Neuron_Structure_Locseg_Chain_To_Circle(const Neuron_Structure *ns)
{
  int nchain = Neuron_Structure_Component_Number(ns);
  
  /* alloc <start_id> */
  int *start_id = iarray_malloc(nchain+1);
  start_id[0] = 0;

  /* alloc <chain_length> */
  int *chain_length = iarray_malloc(nchain);

  int ncomp = locseg_chain_ns_circle_number(ns, start_id);

  Neuron_Structure *ns_circle = Make_Neuron_Structure(ncomp);
  
  int i;
  for (i = 0; i < nchain; i++) {
    Locseg_Chain *chain = 
      NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, i));
    int n = Neuron_Structure_Break_Locseg_Chain(ns_circle, chain, start_id[i]);
    chain_length[i] = n - 2;
    //if (i + 1 < nchain) {
      start_id[i + 1] = start_id[i] + n;
      //}
  }

  int id[2];
  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 1;
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    int index1 = GRAPH_EDGE_NODE(ns->graph, i, 0);
    int index2 = GRAPH_EDGE_NODE(ns->graph, i, 1);

    Neurocomp_Conn *conn2  = Neuron_Structure_Get_Conn(ns, i);
    translate_mode(conn2, chain_length[index1], chain_length[index2],
		   start_id[index1], start_id[index2], id);
    conn.cost = conn2->cost;

    Neuron_Structure_Add_Conn(ns_circle, id[0], id[1], &conn);
  }
  

  /* free <start_id> */
  free(start_id);
  
  /* free <chain_length> */
  free(chain_length);

  return ns_circle;
}
#endif

#if 0
static void translate_mode2(const Neurocomp_Conn *conn, int chain_length0,
			    int chain_length1,
			    int start_id0, int start_id1, int *id)
{
  switch (conn->mode) {
  case NEUROCOMP_CONN_HL:
    if (conn->info[0] == 0) {
      id[0] = 0;
    } else {
      id[0] = chain_length0 - 1;
    }
    id[1] = conn->info[1];
    break;
  case NEUROCOMP_CONN_LINK:
    if (conn->info[0] == 0) {
      id[0] = 0;
    } else {
      id[0] = chain_length0 - 1;
    }
    if (conn->info[1] == 0) {
      id[1] = 0;
    } else {
      id[1] = chain_length1 - 1;
    }
    break;
  default:
    return;
  }
}

Neuron_Structure* 
Neuron_Structure_Locseg_Chain_To_Circle(const Neuron_Structure *ns)
{
  int nchain = Neuron_Structure_Component_Number(ns);
  
  /* alloc <start_id> */
  int *start_id = iarray_malloc(nchain + 1);
  start_id[0] = 0;

  /* alloc <chain_length> */
  int *chain_length = iarray_malloc(nchain);

  int ncomp = locseg_chain_ns_circle_number(ns, start_id);

  Neuron_Structure *ns_circle = Make_Neuron_Structure(ncomp);
  
  int *locseg_index = iarray_malloc(ncomp);

  int i;
  int offset = 0;
  ncomp = 0;

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 1;
  int id[2];

  for (i = 0; i < nchain; i++) {
    Locseg_Chain *chain = 
      NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, i));

    Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
    int n = Locseg_Chain_Knot_Array_Length(ka);

    Geo3d_Circle *circle = 
      Locseg_Chain_Knot_Array_To_Circle_Z(ka, 1.0, NULL);

    int j;
    for (j = 0; j < n; j++) {
      Set_Neuron_Component(ns_circle->comp + offset, 
			   NEUROCOMP_TYPE_GEO3D_CIRCLE,
			   Copy_Geo3d_Circle(circle + j));
      locseg_index[offset] = Locseg_Chain_Knot_Array_At(ka, j)->id;
      offset++;

      if (j > 0) {
	id[0] = start_id[i] + j - 1;
	id[1] = start_id[i] + j;
	conn.cost = 0;
	Neuron_Structure_Add_Conn(ns_circle, id[0], id[1], &conn);
      }
    }

    free(circle);
    ka->chain = NULL;
    Kill_Locseg_Chain_Knot_Array(ka);

    ncomp += n;
    chain_length[i] = Locseg_Chain_Length(chain);

    start_id[i + 1] = start_id[i] + n;
  }
  
  int index[2];
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    index[0] = GRAPH_EDGE_NODE(ns->graph, i, 0);
    index[1] = GRAPH_EDGE_NODE(ns->graph, i, 1);

    Neurocomp_Conn *conn2  = Neuron_Structure_Get_Conn(ns, i);
    translate_mode2(conn2, chain_length[index[0]], chain_length[index[1]],
		    start_id[index[0]], start_id[index[1]], id);

    int j;
    int k;

    for (k = 0; k < 2; k++) {
      BOOL found = FALSE;
      if (id[k] > 0) {
	for (j = start_id[index[k]]; j < start_id[index[k]+1]; j++) {
	  if (locseg_index[j] > id[k]) {
	    if (j == start_id[index[k]]) {
	      id[k] = j;
	    } else {
	      id[k] = j - 1;
	    }
	    found = TRUE;
	    break;
	  }
	}
	if (found == FALSE) {
	  id[k] = start_id[index[k]+1] - 1;
	}
      } else {
	id[k] = start_id[index[k]];
      }
    }
    conn.cost = conn2->cost;
    Neuron_Structure_Add_Conn(ns_circle, id[0], id[1], &conn);
  }
  
  /* set component number to real one */
  ns_circle->graph->nvertex = ncomp;

  free(locseg_index);

  /* free <start_id> */
  free(start_id);
  
  /* free <chain_length> */
  free(chain_length);

  return ns_circle;
}
#endif


static int closest_circle(Neuron_Component *nc, int n, double *pt, 
			  const double *ort)
{
  if (n == 1) {
    return 0;
  }

  double dist;
  double min_dist;
  int min_index;
  int i;

  if (ort == NULL) {
    min_index = 0;
    /*
    start_circle = NEUROCOMP_GEO3D_CIRCLE(nc);
    end_circle = NEUROCOMP_GEO3D_CIRCLE(nc + 1);
    */
    min_dist = Geo3d_Dist_Sqr(NEUROCOMP_GEO3D_CIRCLE(nc)->center[0], 
			      NEUROCOMP_GEO3D_CIRCLE(nc)->center[1], 
			      NEUROCOMP_GEO3D_CIRCLE(nc)->center[2], 
			      pt[0], pt[1], pt[2]);
    
    /*
    min_dist = Geo3d_Point_Lineseg_Dist(pt, start_circle->center,
					end_circle->center, &lambda1);
    lambda = lambda1;
    */
    for (i = 1; i < n; i++) {
      /*
      start_circle = end_circle;
      end_circle = NEUROCOMP_GEO3D_CIRCLE(nc + i);
      */
      
      dist = Geo3d_Dist_Sqr(NEUROCOMP_GEO3D_CIRCLE(nc+i)->center[0], 
			    NEUROCOMP_GEO3D_CIRCLE(nc+i)->center[1], 
			    NEUROCOMP_GEO3D_CIRCLE(nc+i)->center[2], 
			    pt[0], pt[1], pt[2]);
      /*
      dist = Geo3d_Point_Lineseg_Dist(pt, start_circle->center,
				      end_circle->center, &lambda1);
      */
      if (dist < min_dist) {
	min_dist = dist;
	min_index = i;
	//lambda = lambda1;
      }
    }
  } else {
    Geo3d_Circle *start_circle, *end_circle;
    double lambda1, lambda2;
    double lambda;
    double start[3], end[3];

    for (i = 0; i < 3; i++) {
      start[i] = pt[i] - ort[i] * 5.0;
      end[i] = pt[i] + ort[i] * 5.0;
    }

    min_index = 0;
    start_circle = NEUROCOMP_GEO3D_CIRCLE(nc);
    end_circle = NEUROCOMP_GEO3D_CIRCLE(nc + 1);

    /*
    min_dist = Geo3d_Point_Lineseg_Dist(NEUROCOMP_GEO3D_CIRCLE(nc)->center,
					start, end, NULL);
    */

    int cond;
    min_dist = Geo3d_Lineseg_Lineseg_Dist(start_circle->center,
					  end_circle->center, 
					  start, end, &lambda1, &lambda2, 
					  &cond);
    
    lambda = lambda1;

    for (i = 2; i < n - 1; i++) {
      start_circle = end_circle;
      end_circle = NEUROCOMP_GEO3D_CIRCLE(nc + i);
      /*
      dist = Geo3d_Point_Lineseg_Dist(NEUROCOMP_GEO3D_CIRCLE(nc+i)->center,
				      start, end, NULL);
      */
      dist = Geo3d_Lineseg_Lineseg_Dist(start_circle->center,
					end_circle->center, 
					start, end, &lambda1, &lambda2, &cond);

      if (dist < min_dist) {
	min_dist = dist;
	min_index = i - 1;
	lambda = lambda1;
      }
    }  

    double pos[3];
    if ((lambda > 0.0) && (lambda < 1.0)) {
      start_circle =  NEUROCOMP_GEO3D_CIRCLE(nc + min_index);
      end_circle =  NEUROCOMP_GEO3D_CIRCLE(nc + min_index + 1);
      Geo3d_Lineseg_Break(start_circle->center, end_circle->center,
			  lambda, pos);
      if (lambda <= 0.5) {
	/*
	start_circle->center[0] = pos[0];
	start_circle->center[1] = pos[1];
	start_circle->center[2] = pos[2];
	*/
      } else {
	/*
	end_circle->center[0] = pos[0];
	end_circle->center[1] = pos[1];
	end_circle->center[2] = pos[2];
	*/
	min_index++;
      }
    } else {
      if (lambda >= 1.0) {
	min_index++;
      }
    }  
  }

  return min_index;
}

static void translate_mode_s(const Neurocomp_Conn *conn, int chain_id0,
			     int chain_id1, int *start_id, 
			     Neuron_Component *nc, double xy_scale, 
			     double z_scale, int *id)
{
  double pos[3];
  pos[0] = conn->pos[0] * xy_scale;
  pos[1] = conn->pos[1] * xy_scale;
  pos[2] = conn->pos[2] * z_scale;
  
  /*
  double ort[3];
  ort[0] = conn->ort[0] * xy_scale;
  ort[1] = conn->ort[1] * xy_scale;
  ort[2] = conn->ort[2] * z_scale;
  */
  
  double *pos2;

  switch (conn->mode) {
  case NEUROCOMP_CONN_LINK:
  case NEUROCOMP_CONN_HL:
    if (conn->info[0] == 0) {
      id[0] = start_id[chain_id0];
    } else {
      id[0] = start_id[chain_id0 + 1] - 1;
    }

    pos2 = NEUROCOMP_GEO3D_CIRCLE(nc+id[0])->center;
    /*
    id[1] = closest_circle(nc + start_id[chain_id1], 
			   start_id[chain_id1+1] - start_id[chain_id1], 
			   pos, conn->ort) + start_id[chain_id1];
    */
    id[1] = closest_circle(nc + start_id[chain_id1], 
			   start_id[chain_id1+1] - start_id[chain_id1], 
			   pos, NULL) + start_id[chain_id1];

    break;
    /*
  case NEUROCOMP_CONN_LINK:
    if (conn->info[0] == 0) {
      id[0] = start_id[chain_id0];
    } else {
      id[0] = start_id[chain_id0 + 1] - 1;
    }
    if (conn->info[1] == 0) {
      id[1] = start_id[chain_id1];
    } else {
      id[1] = start_id[chain_id1 + 1] - 1;
    }
    break;
    */
  default:
    return;
  }
}


Neuron_Structure* 
Neuron_Structure_Locseg_Chain_To_Circle_S(const Neuron_Structure *ns, 
					  double xy_scale, double z_scale)
{
  int nchain = Neuron_Structure_Component_Number(ns);
  
  /* alloc <start_id> */
  int *start_id = iarray_malloc(nchain + 1);
  start_id[0] = 0;

  /* alloc <chain_length> */
  //int *chain_length = iarray_malloc(nchain);

  int ncomp = locseg_chain_ns_circle_number(ns, start_id) * 5;

  Neuron_Structure *ns_circle = Make_Neuron_Structure(ncomp);
  
  int i;
  ncomp = 0;
  for (i = 0; i < nchain; i++) {
    Locseg_Chain *chain = 
      NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, i));
    int n = Neuron_Structure_Break_Locseg_Chain_S(ns_circle, chain, 
						  start_id[i],
						  xy_scale, z_scale);
    ncomp += n;
    //chain_length[i] = Locseg_Chain_Length(chain);
    start_id[i + 1] = start_id[i] + n;
  }

  ns_circle->graph->nvertex = ncomp;

  int id[2];
  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 1;
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    int index1 = GRAPH_EDGE_NODE(ns->graph, i, 0);
    int index2 = GRAPH_EDGE_NODE(ns->graph, i, 1);

    Neurocomp_Conn *conn2  = Neuron_Structure_Get_Conn(ns, i);
    /*
    translate_mode(conn2, chain_length[index1], chain_length[index2],
		   start_id[index1], start_id[index2], id);
    */
    translate_mode_s(conn2, index1, index2, start_id, ns_circle->comp, 
		     xy_scale, z_scale, id);

    conn.cost = conn2->cost;

    Neuron_Structure_Add_Conn(ns_circle, id[0], id[1], &conn);
  }
  

  /* free <start_id> */
  free(start_id);
  
  /* free <chain_length> */
  //free(chain_length);

  return ns_circle;
}

int Neuron_Structure_Conn_Index(const Neuron_Structure *ns, int comp_index)
{
  int i;
  for (i = 0; i < ns->graph->nedge; i++) {
    if ((GRAPH_EDGE_NODE(ns->graph, i, 0) == comp_index) ||
	(GRAPH_EDGE_NODE(ns->graph, i, 0) == comp_index)) {
      return i;
    }
  }

  return -1;
}

void Neuron_Structure_Locseg_Chain_Merge_Terminal(Neuron_Structure *ns)
{
  Graph_Workspace *gw = New_Graph_Workspace();

  int *degree = Graph_Degree(ns->graph, gw);
  int i;
  for (i = 0; i < NEURON_STRUCTURE_COMPONENT_NUMBER(ns); i++) {
    if (degree[i] == 0) {
      int j = Neuron_Structure_Conn_Index(ns, i);
      Locseg_Chain *chain2 = 
	NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, i));
      int chain_index = GRAPH_EDGE_NODE(ns->graph, j, 0);
      if (chain_index == i) {
	chain_index = GRAPH_EDGE_NODE(ns->graph, j, 1);
	int tmp_node = GRAPH_EDGE_NODE(ns->graph, j, 0);
	GRAPH_EDGE_NODE(ns->graph, j, 0) = GRAPH_EDGE_NODE(ns->graph, j, 1);
	GRAPH_EDGE_NODE(ns->graph, j, 1) = tmp_node;
      }
      Locseg_Chain *chain1 = 
	NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, chain_index));
      Locseg_Chain_Conn_Fix(chain1, chain2, Neuron_Structure_Get_Conn(ns, j));
    }
  }

  Kill_Graph_Workspace(gw);
}

void Neuron_Structure_Weight_Graph(Neuron_Structure *ns)
{
  int nedge = Neuron_Structure_Link_Number(ns);
  ns->graph->weights = darray_malloc(nedge);
  int i;
  for (i = 0; i < nedge; i++) {
    ns->graph->weights[i] =  ns->conn[i].cost;
  }
}

void Neuron_Structure_Unweight_Graph(Neuron_Structure *ns)
{
  if (ns->graph->weights != NULL) {
    free(ns->graph->weights);
    ns->graph->weights = NULL;
  }
}

void Neuron_Structure_Main_Graph(Neuron_Structure *ns)
{
  Graph_Workspace *gw = New_Graph_Workspace();
  Graph *subgraph = Graph_Main_Subgraph(ns->graph, gw);
  Graph_Update_Edge_Table(subgraph, gw);
  int i;
  int j = 0;
  for (i = 0; i < ns->graph->nedge; i++) {
    if (Graph_Edge_Index(GRAPH_EDGE_NODE(ns->graph, i, 0), 
	GRAPH_EDGE_NODE(ns->graph, i, 1), gw) >= 0) {
      Neurocomp_Conn_Copy(ns->conn + j, ns->conn + i);
      j++;
    }
  }
 
  Kill_Graph(ns->graph);
  ns->graph = subgraph;
  Kill_Graph_Workspace(gw);
}

void Neuron_Structure_To_Tree(Neuron_Structure *ns)
{
  Graph_Workspace *gw = New_Graph_Workspace();

  Neuron_Structure_Weight_Graph(ns);
  Graph_To_Mst2(ns->graph, gw);
  
  int i;
  int j = 0;
  for (i = 0; i < gw->nedge; i++) {
    if (gw->status[i] == 1) {
      Neurocomp_Conn_Copy(ns->conn + j, ns->conn + i);
      j++;
    }
  }

  Neuron_Structure_Unweight_Graph(ns);

  Kill_Graph_Workspace(gw);
}

Neuron_Component_Arraylist* Neuron_Structure_Branch_Point(Neuron_Structure *ns)
{
  /* alloc <branches> */
  Neuron_Component_Arraylist *branches = Make_Neuron_Component_Arraylist(0, 1);
  
  int ncomp = Neuron_Structure_Component_Number(ns);

  /*alloc <status> */
  int *status = (int*) malloc(sizeof(int) * ncomp * 2);
  int *head_link = status;
  int *tail_link = status + ncomp;

  int i;

  for (i = 0; i < ncomp; i++) {
    head_link[i] = 0;
    tail_link[i] = 0;
  }

  int chain_index[2];
  Neuron_Component nc;

  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    chain_index[0] = GRAPH_EDGE_NODE(ns->graph, i, 0);
    chain_index[1] = GRAPH_EDGE_NODE(ns->graph, i, 1);
    Neurocomp_Conn *conn = Neuron_Structure_Get_Conn(ns, i);
    
    if (conn->mode == NEUROCOMP_CONN_HL) { /* branch point */
      Locseg_Chain *chain = 
	NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, chain_index[1]));

      Local_Neuroseg *locseg = 
	Locseg_Chain_Peek_Seg_At(chain, conn->info[1]);
      Set_Neuron_Component(&nc, NEUROCOMP_TYPE_LOCAL_NEUROSEG,
			   Copy_Local_Neuroseg(locseg));
      Neuron_Component_Arraylist_Add(branches, nc);
    } else if (conn->mode == NEUROCOMP_CONN_LINK) {   
      /* calculate degree of each node */
      if (conn->info[0] == 0) {
	head_link[chain_index[0]]++;
      } else {
	tail_link[chain_index[0]]++;
      }
      if (conn->info[1] == 0) {
	head_link[chain_index[1]]++;
      } else {
	tail_link[chain_index[1]]++;
      }
    } else {
      continue;
    }
  }

  /* */
  for (i = 0; i < ncomp; i++) {
    Local_Neuroseg *locseg = NULL;
    Locseg_Chain *chain = 
      NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, i));
    if (chain != NULL) {
      if (head_link[i] > 1) {
	locseg = Locseg_Chain_Head_Seg(chain);
      }
      if (locseg != NULL) {
	Set_Neuron_Component(&nc, NEUROCOMP_TYPE_LOCAL_NEUROSEG,
			   Copy_Local_Neuroseg(locseg));
	Neuron_Component_Arraylist_Add(branches, nc);
      }
      locseg = NULL;
      if (tail_link[i] > 1) {
	locseg = Locseg_Chain_Tail_Seg(chain);
      }
      if (locseg != NULL) {
	Set_Neuron_Component(&nc, NEUROCOMP_TYPE_LOCAL_NEUROSEG,
			   Copy_Local_Neuroseg(locseg));
	Neuron_Component_Arraylist_Add(branches, nc);
      }
    } else {
      Geo3d_Circle *circle = 
	NEUROCOMP_GEO3D_CIRCLE(Neuron_Structure_Get_Component(ns, i));
      if (circle != NULL) {
	if ((head_link[i] > 2) || (tail_link[i] > 2)) {
	  Set_Neuron_Component(&nc, NEUROCOMP_TYPE_GEO3D_CIRCLE,
			       Copy_Geo3d_Circle(circle));
	  Neuron_Component_Arraylist_Add(branches, nc);
	}
      } else {
	Geo3d_Ball *ball = 
	NEUROCOMP_GEO3D_BALL(Neuron_Structure_Get_Component(ns, i));
	if (ball != NULL) {
	  if ((head_link[i] > 2) || (tail_link[i] > 2)) {
	    Set_Neuron_Component(&nc, NEUROCOMP_TYPE_GEO3D_BALL,
				 Copy_Geo3d_Ball(ball));
	    Neuron_Component_Arraylist_Add(branches, nc);
	  }
	} else {
	  TZ_ERROR(ERROR_DATA_TYPE);
	}
      }
    }
  }

  /* free <status> */
  free(status);

  /* return <branches> */
  return branches;  
}

Neuron_Structure* Neuron_Structure_From_Swc_File(const char *filepath)
{
  int n;
  Swc_Node *neuron = Read_Swc_File(filepath, &n);
  Swc_Normalize(neuron, n);

  Neuron_Structure *ns = Make_Neuron_Structure(n);
  
  Set_Neuron_Component(ns->comp, NEUROCOMP_TYPE_GEO3D_BALL, 
		       Swc_Node_To_Geo3d_Ball(neuron, NULL));

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 0;
  conn.cost = 0.0;

  int i;
  for (i = 1; i < n; i++) {
    Set_Neuron_Component(ns->comp + i, NEUROCOMP_TYPE_GEO3D_BALL, 
			 Swc_Node_To_Geo3d_Ball(neuron + i, NULL));
    Neuron_Structure_Add_Conn(ns, neuron[i].id - 1, neuron[i].parent_id - 1, 
			      &conn);
  }

  free(neuron);

  return ns;
}

void Neuron_Structure_Crossover_Test(Neuron_Structure *ns, double z_scale)
{
  int nconn = Neuron_Structure_Link_Number(ns);
  int ncomp = Neuron_Structure_Component_Number(ns);

  int *status = iarray_calloc(nconn);

  double dist_thre = 20.0;

  int i, j;
  for (i = 0; i < ncomp; i++) {
    /* how many components are connected */
    int head_link_number = 0;
    int tail_link_number = 0;
    /* which components are connected */
    int head_link_index[10]; 
    int tail_link_index[10];
    int link_type[2]; /* {head, tail} 0 for normal, 1 for cross */
    
    Locseg_Chain *chain = 
      NEUROCOMP_LOCSEG_CHAIN(Neuron_Structure_Get_Component(ns, i));
    
    if (chain == NULL) {
      continue;
    }
    
    link_type[0] = 0;
    link_type[1] = 0;

    for (j = 0; j < nconn; j++) {
      if (GRAPH_EDGE_NODE(ns->graph, j, 0) == i) {
	if (ns->conn[j].info[0] == 0) {
	  if (ns->conn[j].mode == NEUROCOMP_CONN_HL) {
	    if (ns->conn[j].sdist < dist_thre) {
	      head_link_index[head_link_number++] = j;
	    } 
	  } else {
	    head_link_index[head_link_number++] = j;
	    if ((Neuron_Structure_Conn_Angle(ns, j, z_scale) < TZ_PI_4) &&
		(ns->conn[j].sdist < dist_thre)) {
	      link_type[0] = 1;
	    }
	  }  
	} else {
	  if (ns->conn[j].mode == NEUROCOMP_CONN_HL) {
	    if (ns->conn[j].sdist < dist_thre) {
	      tail_link_index[tail_link_number++] = j;
	    } 
	  } else {
	    tail_link_index[tail_link_number++] = j;
	    if ((Neuron_Structure_Conn_Angle(ns, j, z_scale) < TZ_PI_4) &&
		(ns->conn[j].sdist < dist_thre)) {
	      link_type[1] = 1;
	    }
	  }
	}
      }

      if ((GRAPH_EDGE_NODE(ns->graph, j, 1) == i) && 
	  (ns->conn[j].mode == NEUROCOMP_CONN_LINK)){
	if (ns->conn[j].info[1] == 0) {
	  head_link_index[head_link_number++] = j;
	  if ((Neuron_Structure_Conn_Angle(ns, j, z_scale) < TZ_PI_4) &&
	      (ns->conn[j].sdist < dist_thre)) {
	    link_type[0] = 1;
	  }
	} else {
	  tail_link_index[tail_link_number++] = j;
	  if ((Neuron_Structure_Conn_Angle(ns, j, z_scale) < TZ_PI_4) &&
	      (ns->conn[j].sdist < dist_thre)) {
	    link_type[1] = 1;
	  }
	}
      }
    }

    if (link_type[0] == 1) {
      if (head_link_number > 1) {
	for (j = 0; j < head_link_number; j++) {
	  if (ns->conn[head_link_index[j]].mode == NEUROCOMP_CONN_HL) {
	    //ns->conn[head_link_index[j]].cost += 3.0;
	    status[head_link_index[j]] = 1;
	  }
	}
      }
    } else if (link_type[1] == 1) {
      if (tail_link_number > 1) {
	for (j = 0; j < tail_link_number; j++) {
	  if (ns->conn[tail_link_index[j]].mode == NEUROCOMP_CONN_HL) {
	    //ns->conn[tail_link_index[j]].cost += 3.0;
	    status[tail_link_index[j]] = 1;
	  }
	}
      }
    }
  }

  for (i = 0; i < nconn; i++) {
    if (status[i] == 1) {
      ns->conn[i].cost += 1.0;
    }
  }

  free(status);
}

void Neuron_Structure_Adjust_Link(Neuron_Structure *ns, double factor)
{
  int i;
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    //if (ns->conn[i].mode == NEUROCOMP_CONN_LINK) {
      if (Neuron_Structure_Conn_Angle(ns, i, 1.0) < TZ_PI_4) {
	ns->conn[i].cost *= factor;
      }
      //}
  }
}

double Neuron_Structure_Conn_Angle(Neuron_Structure *ns, int index, 
				   double z_scale)
{
  if (ns->conn[index].mode == NEUROCOMP_CONN_NONE) {
    return -1.0;
  }

  Locseg_Chain *chain1 = NEUROCOMP_LOCSEG_CHAIN
    (Neuron_Structure_Get_Component(ns, GRAPH_EDGE_NODE(ns->graph, index, 0)));

  if (chain1 == NULL) {
    return -1.0;
  }

  Locseg_Chain *chain2 = NEUROCOMP_LOCSEG_CHAIN
    (Neuron_Structure_Get_Component(ns, GRAPH_EDGE_NODE(ns->graph, index, 1)));

  if (chain2 == NULL) {
    return -1.0;
  }

  Local_Neuroseg *locseg1 = NULL;
  Local_Neuroseg *locseg2 = NULL;
  
  switch (ns->conn[index].mode) {
  case NEUROCOMP_CONN_HL:
    if (ns->conn[index].info[0] == 0) {
      locseg1 = Copy_Local_Neuroseg(Locseg_Chain_Head_Seg(chain1));
      Flip_Local_Neuroseg(locseg1);
    } else {
      locseg1 = Copy_Local_Neuroseg(Locseg_Chain_Tail_Seg(chain1));
    }

    locseg2 = Copy_Local_Neuroseg
      (Locseg_Chain_Peek_Seg_At(chain2, ns->conn[index].info[1]));
    break;
  case NEUROCOMP_CONN_LINK:
    if (ns->conn[index].info[0] == 0) {
      locseg1 = Copy_Local_Neuroseg(Locseg_Chain_Head_Seg(chain1));
      Flip_Local_Neuroseg(locseg1);
    } else {
      locseg1 = Copy_Local_Neuroseg(Locseg_Chain_Tail_Seg(chain1));
    }

    if (ns->conn[index].info[1] == 0) {
      locseg2 = Copy_Local_Neuroseg(Locseg_Chain_Head_Seg(chain2));
    } else {
      locseg2 = Copy_Local_Neuroseg(Locseg_Chain_Tail_Seg(chain2));
      Flip_Local_Neuroseg(locseg2);
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }
  
  Delete_Local_Neuroseg(locseg1);
  Delete_Local_Neuroseg(locseg2);
      
  return Neuroseg_Angle_Between_Z(&(locseg1->seg), &(locseg2->seg), z_scale);
}

#ifdef HAVE_LIBXML2
static void neuron_structure_xml_conn(xmlDocPtr doc, xmlNodePtr node, 
				      int *id, int *spot)
{
  xmlNodePtr child = node->xmlChildrenNode;
  while (child != NULL) {
    if (Xml_Node_Is_Element(child, "filePath")) {
      *id = String_Last_Integer(Xml_Node_String_Value(doc, child));
    } else if (Xml_Node_Is_Element(child, "spot")) {
      *spot = Xml_Node_Int_Value(doc, child);
    }
    child = child->next;
  }
}
#endif

Graph* Neuron_Structure_Import_Xml_Graph(const char *file_path)
{
#ifdef HAVE_LIBXML2
  xmlDocPtr doc;
  xmlNodePtr cur;

  doc = xmlParseFile(file_path);

  if (doc == NULL) {
    return NULL;
  }

  xmlNodePtr root = xmlDocGetRootElement(doc);
  
  if (root == NULL) {
    return NULL;
  } else if (Xml_Node_Is_Element(root, "tubeConn") == FALSE) {
    return NULL;
  }

  int n = 0;
  cur = root->xmlChildrenNode;
  while (cur != NULL) {
    if (Xml_Node_Is_Element(cur, "connection")) {
      n++;
    }
    cur = cur->next;
  }

  Graph *graph = Make_Graph(n*3, n, FALSE);
  //GUARDED_MALLOC_ARRAY(ns->conn, ns->graph->nedge, Neurocomp_Conn);
  cur = root->xmlChildrenNode;
  while (cur != NULL) {
    if (Xml_Node_Is_Element(cur, "connection")) {
      xmlNodePtr child = cur->xmlChildrenNode;
      int hook = -1;
      int loop = -1;
      int hook_spot = -1;
      int loop_spot = -1;
      int mode = NEUROCOMP_CONN_HL;
	  
      while (child != NULL) {
	if (Xml_Node_Is_Element(child, "hook")) {
	  neuron_structure_xml_conn(doc, child, &hook, &hook_spot);
	} else if (Xml_Node_Is_Element(child, "loop")) {
	  neuron_structure_xml_conn(doc, child, &loop, &loop_spot);
	} else if (Xml_Node_Is_Element(child, "mode")) {
	  mode = Xml_Node_Int_Value(doc, child);
	}
	child = child->next;
      }
      
      hook *= 3;
      loop *= 3;
      if (hook_spot == 1) {
	hook += 2;
      }
      if (mode == NEUROCOMP_CONN_LINK) {
	if (loop_spot == 1) {
	  loop += 2;
	}
      } else {
	if (loop_spot > 0) {
	  loop += 1;
	}
      }

      Graph_Add_Edge(graph, hook, loop);
    }
    cur = cur->next;
  }
  
  xmlFreeDoc(doc);

  return graph;
#else
  TZ_ERROR(ERROR_NA_FUNC);
  return NULL;
#endif
}

void Neuron_Structure_Merge_Locseg_Chain(const Neuron_Structure *ns)
{
  int ncomp = Neuron_Structure_Component_Number(ns);
  
  int *end_status[2]; /* -1: available; <=-2; supressed */

  end_status[0] = iarray_malloc(ncomp);
  end_status[1] = iarray_malloc(ncomp);
  
  int i;
  for (i = 0; i < ncomp; i++) {
    end_status[0][i] = -1;
    end_status[1][i] = -1;
  }

  int nlink = Neuron_Structure_Link_Number(ns);
  double *cost = darray_malloc(nlink);
  int *edge_index = iarray_malloc(nlink);

  /* suppress ends */
  for (i = 0; i < nlink; i++) {
    if (ns->conn[i].mode == NEUROCOMP_CONN_HL) {
      end_status[ns->conn[i].info[0]][ns->graph->edges[i][0]] = -2;
    }
  }

  for (i = 0; i < nlink; i++) {
    cost[i] = ns->conn[i].cost;
  }
  
  darray_qsort(cost, edge_index, nlink);
  
  /* for every sorted edge */
  for (i = 0; i < nlink; i++) {
    int idx = edge_index[i];
    int node = ns->graph->edges[idx][0];
    int nbr_node = ns->graph->edges[idx][1];
    /* make pairs if both nodes are available */
    if ((end_status[ns->conn[idx].info[0]][node] == -1) && 
	(end_status[ns->conn[idx].info[1]][nbr_node] == -1)) {
      end_status[ns->conn[idx].info[0]][node] = idx;
      end_status[ns->conn[idx].info[1]][nbr_node] = idx;
    }
    
    /* suppress neighbor nodes of paired nodes */
    int j;
    for (j = i; j < nlink; j++) {
      idx = edge_index[j];
      if (ns->conn[idx].mode == NEUROCOMP_CONN_LINK) {
	int k;
	for (k = 0; k < 2; k++) {
	  int node = ns->graph->edges[idx][k];
	  int nbr_node = ns->graph->edges[idx][!k];
	  int *status = end_status[ns->conn[idx].info[k]];
	  int *nbr_status = end_status[ns->conn[idx].info[!k]];
	  if (status[node] >= 0) {
	    if (nbr_status[nbr_node] < 0) {
	      nbr_status[nbr_node] = -3 - node;
	    }
	  }
	}
      }
    }
  }

#ifdef _DEBUG_2
  iarray_print2(end_status[0], ncomp, 1);
  iarray_print2(end_status[1], ncomp, 1);
#endif
  
  BOOL merged = TRUE;
  
  while (merged) {
    merged = FALSE;
    /* for every chain */
    for (i = 0; i < ncomp; i++) {
      Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(ns->comp + i);
      /* if the chain is not empty */
      if (Locseg_Chain_Is_Empty(chain) == FALSE) {
	/* if only one end is paired */
	if (((end_status[0][i] >= 0) + (end_status[1][i] >= 0)) == 1) {
	  int idx;
	  if (end_status[0][i] >= 0) {
	    idx = end_status[0][i];
	  } else {
	    idx = end_status[1][i];
	  }

	  int nbr_node = ns->graph->edges[idx][1];
	  int end = 1;
	  if (nbr_node == i) {
	    nbr_node = ns->graph->edges[idx][0];
	    end = 0;
	  }

	  Locseg_Chain *master = NEUROCOMP_LOCSEG_CHAIN(ns->comp + nbr_node);
	  if (Locseg_Chain_Is_Empty(master) == FALSE) {
#ifdef _DEBUG_
	    printf("%d ~ %d: ", Locseg_Chain_Length(master), 
		Locseg_Chain_Length(chain));
#endif	  
	    Locseg_Chain_Merge(master, chain, (Dlist_End_e)ns->conn[idx].info[end], 
		(Dlist_End_e)ns->conn[idx].info[!end]);
#ifdef _DEBUG_
	    printf("%d\n", Locseg_Chain_Length(master));
#endif
	    end_status[ns->conn[idx].info[0]][i] = -1;
	    end_status[ns->conn[idx].info[0]][nbr_node] = -1;

	    merged = TRUE;
	  }
	}
      }
    }
  }

#ifdef _DEBUG_
  iarray_print2(end_status[0], ncomp, 1);
  iarray_print2(end_status[1], ncomp, 1);
#endif

  free(edge_index);
  free(cost);
  free(end_status[0]);
  free(end_status[1]);
}

static int nearest_chain(Neuron_Structure *ns, coordinate_3d_t pt, 
			 int *seg_index, double *min_dist)
{
  int id = -1;
  double mindist = -1.0;
  int i;
  for (i = 0; i < NEURON_STRUCTURE_COMPONENT_NUMBER(ns); i++) {
    Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(ns->comp + i);
    if (chain != NULL) {
      int index;
      double dist = Locseg_Chain_Point_Dist(chain, pt, &index, NULL);
      if ((mindist < 0) || (dist < mindist)) {
	mindist = dist;
	id = i;
	if (seg_index != NULL) {
	  *seg_index = index;
	}
      }
    }
  }

  if (min_dist != NULL) {
    *min_dist = mindist;
  }

  return id;
}

static void neuron_structure_add_locseg_chain(Neuron_Structure *ns, 
					      Locseg_Chain *chain)
{
  Neuron_Component nc;
  Set_Neuron_Component(&nc, NEUROCOMP_TYPE_LOCSEG_CHAIN, chain);
  Neuron_Structure_Set_Component(ns, NEURON_STRUCTURE_COMPONENT_NUMBER(ns), 
				 &nc);
				 
}

void Neuron_Structure_Break_Root(Neuron_Structure *ns, coordinate_3d_t *roots,
				 int n)
{
  int *chain_id;
  int *seg_index;
  double *dist;

  /* alloc <chain_id> */
  GUARDED_MALLOC_ARRAY(chain_id, n, int);
  /* alloc <seg_index> */
  GUARDED_MALLOC_ARRAY(seg_index, n, int);
  /* alloc <dist> */
  GUARDED_MALLOC_ARRAY(dist, n, double);

  int i;
  for (i = 0; i < n; i++) {
    chain_id[i] = nearest_chain(ns, roots[i], seg_index + i, dist + i);
  }

  int j;
  for (i = 0; i < n; i++) {
    for (j = i + 1; j < n; j++) {
      if (chain_id[i] == chain_id[j]) {
	if ((dist[i] < 11.0) && (dist[j] < 11.0)) {
	  //TZ_ASSERT(seg_index[i] != seg_index[j], "Common index.");
	  Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(ns->comp + chain_id[i]);
	  Locseg_Chain *new_chain = 
	    Locseg_Chain_Break_Between(chain, seg_index[i], seg_index[j]);
	  neuron_structure_add_locseg_chain(ns, new_chain);
	}
      }
    }
  }
  
  /* free <chain_id> */
  free(chain_id);
  /* free <seg_index> */
  free(seg_index);
  /* free <dist> */
  free(dist);
}

int Neuron_Structure_Load_Root(Neuron_Structure *ns, coordinate_3d_t *roots,
			       int n)
{
  int root_chain_id = -1;
  int nedge = 0;
  /* Find the nearest chain for each root and connect them */
  int i;
  for (i = 0; i < n; i++) {
    int chain_id = nearest_chain(ns, roots[i], NULL, NULL);
    Locseg_Chain_Interpolate_L(NEUROCOMP_LOCSEG_CHAIN(ns->comp+chain_id),
			       roots[i], NULL, NULL);
    TZ_ASSERT(chain_id >= 0, "Invalid chain id");
    if (root_chain_id >= 0) {
      Neurocomp_Conn conn;
      conn.mode = NEUROCOMP_CONN_LINK;
      conn.info[0] = 0;
      conn.info[1] = 1;
      conn.cost = -1;
      Neuron_Structure_Add_Conn(ns, root_chain_id, chain_id, &conn);
      nedge++;
    }
    root_chain_id = chain_id;
  }

  return nedge;
}

void Neuron_Structure_Remove_Negative_Conn(Neuron_Structure *ns)
{
  int current_spot = 0; /* the spot for an edge to move to */
  int nconn = NEURON_STRUCTURE_LINK_NUMBER(ns);
  int i;
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    if (ns->conn[i].cost >= 0.0) {
      if (current_spot != i) {
	Neurocomp_Conn_Copy(ns->conn + current_spot, ns->conn + i);
	Graph_Move_Edge(ns->graph, current_spot, i);
	ns->conn[i].cost = -1.0;
      }
      current_spot++;
    } else {
      nconn--;
    }
  }

  NEURON_STRUCTURE_LINK_NUMBER(ns) = nconn;
}
