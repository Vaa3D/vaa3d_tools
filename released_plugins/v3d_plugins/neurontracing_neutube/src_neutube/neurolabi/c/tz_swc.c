#include "tz_error.h"
#include "tz_swc.h"
#include "tz_iarray.h"

int* Swc_Arraylist_Queue(const Swc_Arraylist *sc, int *q)
{
  if (q == NULL) {
    q = iarray_malloc(sc->length);
  }

  int i, j;
  for (i = 0; i < sc->length; i++) {
    q[i] = -1;
    if (sc->array[i].parent_id >= 0) {
      for (j = 0; j < sc->length; j++) {
	if (sc->array[i].parent_id == sc->array[j].id) {
	  q[i] = j;
	  break;
	}
      }
    }
  }

  return q;
}

Graph* Swc_Arraylist_Graph(const Swc_Arraylist *sc, int *q)
{
  BOOL is_owner = TRUE;

  if (q != NULL) {
    is_owner = FALSE;
  }

  q = Swc_Arraylist_Queue(sc, q);
  Graph *graph = Make_Graph(sc->length, sc->length - 1, FALSE);
  Graph_Set_Directed(graph, TRUE);

  int i;
  for (i = 0; i < sc->length; i++) {
    if (q[i] >= 0) {
      Graph_Add_Edge(graph, q[i], i);
    }
  }

  if (is_owner == TRUE) {
    free(q);
  }

  return graph;
}

Swc_Arraylist* Swc_Arraylist_Rebuild(const Swc_Arraylist *sc, 
				     const Graph *graph)
{
  Swc_Arraylist *new_sc = Make_Swc_Arraylist(0, graph->nedge * 2);

  Graph_Workspace *gw = New_Graph_Workspace();
  int *in_degree = Graph_In_Degree(graph, gw);
  int *out_degree = Graph_Out_Degree(graph, gw);

  int i;

  for (i = 0; i < graph->nvertex; i++) {
    if ((in_degree[i] == 0) && (out_degree[i] > 0)) {
      Swc_Node node = sc->array[i];
      node.parent_id = -1;
      Swc_Arraylist_Add(new_sc, node);
    }
  }

  Kill_Graph_Workspace(gw);

  for (i = 0; i < graph->nedge; i++) {
    Swc_Node node = sc->array[graph->edges[i][1]];
    node.parent_id = sc->array[graph->edges[i][0]].id;
    Swc_Arraylist_Add(new_sc, node);
  }

  return new_sc;
}

void Swc_Arraylist_To_Swc_File(const Swc_Arraylist *sc, const char *file_path)
{
  FILE *fp = fopen(file_path, "w");
  
  if (fp == NULL) {
    TZ_ERROR(ERROR_IO_OPEN);
  }

  int i;
  for (i = 0; i < sc->length; i++) {
    Swc_Node_Fprint(fp, sc->array + i);
  }

  fclose(fp);
}
