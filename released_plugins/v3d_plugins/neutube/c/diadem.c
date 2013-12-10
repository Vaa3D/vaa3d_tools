/**@file diadem.c
 * @brief >> Process DIADEM data.
 * @author Ting Zhao
 * @date 10-Dec-2009
 */

#include "tz_utilities.h"
#include "tz_string.h"
#include "tz_geo3d_point_array.h"
#include "tz_graph_utils.h"
#include "tz_graph.h"
#include "tz_iarray.h"
#include "tz_image_io.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[<input:string>] [-score]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-score")) {
    FILE *fp = fopen(Get_String_Arg("input"), "r");
    char *line = NULL;
    String_Workspace *sw = New_String_Workspace();
    double average = 0;
    int nscore = 0;
    while ((line = Read_Line(fp, sw)) != NULL) {
      double score[10];
      if (String_Starts_With(line, "Score")) {
	int n;
	String_To_Double_Array(line, score, &n);
	average += score[0];
	nscore++;
      }
    }

    fclose(fp);
    printf("%g\n", average / nscore);
  }
 
#if 0 /* check data */
  FILE *fp = GUARDED_FOPEN("../data/diadem_d1.txt", "r");
  String_Workspace *sw = New_String_Workspace();

  int depth[200];
  int value[100];
  int n = 0;
  char *line;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int nvalue = 0;
    String_To_Integer_Array(line, value, &nvalue);
    if (nvalue > 0) {
      depth[n++] = value[1];
    }
  }

  iarray_print2(depth, n, 1);

  int i = 0;
  int size[3];
  char filepath[100];
  for (i = 0; i < n; i++) {
    sprintf(filepath, "../data/diadem_d1_%03d.xml", i+1);
    Stack_Size_F(filepath, size);
    if (size[2] != depth[i]) {
      printf("Corrupted file: %s (%d -> %d)\n", filepath, size[2], depth[i]);
    }
  }
#endif

#if 0
  FILE *fp = GUARDED_FOPEN("../data/diadem_d1.txt", "r");
  String_Workspace *sw = New_String_Workspace();

  int depth[200];
  int value[100];
  int n = 0;
  char *line;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int nvalue = 0;
    String_To_Integer_Array(line, value, &nvalue);
    if (nvalue > 0) {
      depth[n++] = value[1];
    }
  }
  fclose(fp);

  iarray_print2(depth, n, 1);

  int i = 0;
  int size[3];
  char filepath[100];
  char filepath2[100];
  for (i = 0; i < n; i++) {
    sprintf(filepath, "../data/diadem_d1_%03d.tif", i+1);
    if (!fexist(filepath)) {
      sprintf(filepath, "../data/diadem_d1_%03d.xml", i+1);
    }
    Stack_Size_F(filepath, size);
    
    sprintf(filepath2, "../data/diadem_d1_%03d_pos.txt", i+1);
    fp = fopen(filepath2, "w");
    fprintf(fp, "%s (0, 0, 0) (%d, %d, %d)", filepath, 
	    size[0], size[1], size[2]);
    fclose(fp);
  }
#endif

#if 0 /* check data */
  FILE *fp = GUARDED_FOPEN("../data/diadem_c1.txt", "r");
  String_Workspace *sw = New_String_Workspace();

  int depth[200];
  int value[100];
  int n = 0;
  char *line;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int nvalue = 0;
    String_To_Integer_Array(line, value, &nvalue);
    if (nvalue > 0) {
      depth[n++] = value[1];
    }
  }

  iarray_print2(depth, n, 1);

  int i = 0;
  int size[3];
  char filepath[100];
  for (i = 0; i < n; i++) {
    sprintf(filepath, "../data/diadem_c1_%02d.xml", i+1);
    Stack_Size_F(filepath, size);
    if (size[2] != depth[i]) {
      printf("Corrupted file: %s (%d -> %d)\n", filepath, size[2], depth[i]);
    }
  }
#endif

#if 0
  FILE *fp = GUARDED_FOPEN("../data/diadem_d1.txt", "r");
  
  String_Workspace *sw = New_String_Workspace();

  char *line;
  double value[100];
  coordinate_3d_t points[200];
  int n = 0;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int nvalue = 0;
    String_To_Double_Array(line, value, &nvalue);
    if (nvalue > 0) {
      printf("%d, %g, %g\n", n, value[nvalue-2], value[nvalue-1]);
      points[n][0] = value[nvalue-2];
      points[n][1] = value[nvalue-1];
      points[n++][2] = 0.0;
    }
  }

  double **dist = Geo3d_Point_Array_Distmat(points, n, NULL);

  Graph_Edge_t edges[200];
  int nedge;
  double sum;
  Graph_Mst_From_Adjmat(dist, edges, n, &nedge, &sum);
  
  Graph *graph = New_Graph();
  graph->edges = edges;
  graph->nedge = nedge;
  graph->nvertex = nedge + 1;

  fp = GUARDED_FOPEN("../data/diadem_d1_conn.txt", "w");
  fprintf(fp, "%d\n", graph->nvertex);
  int i;
  for (i = 0; i < graph->nedge; i++) {
    fprintf(fp, "%d %d\n", graph->edges[i][0] + 1, graph->edges[i][1] + 1);
  }
  fclose(fp);

  Kill_String_Workspace(sw);

  fclose(fp);
#endif

#if 0
  FILE *fp = GUARDED_FOPEN("../data/diadem_c1.txt", "r");
  
  String_Workspace *sw = New_String_Workspace();

  char *line;
  double value[100];
  coordinate_3d_t points[200];
  int n = 0;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int nvalue = 0;
    String_To_Double_Array(line, value, &nvalue);
    if (nvalue > 0) {
      printf("%d, %g, %g, %g\n", n, value[nvalue-3], value[nvalue-2], 
	     value[nvalue-1]);
      points[n][0] = value[nvalue-3];
      points[n][1] = value[nvalue-2];
      points[n++][2] = value[nvalue-1];
    }
  }

  double **dist = Geo3d_Point_Array_Distmat(points, n, NULL);

  Graph_Edge_t edges[200];
  int nedge;
  double sum;
  Graph_Mst_From_Adjmat(dist, edges, n, &nedge, &sum);
  
  Graph *graph = New_Graph();
  graph->edges = edges;
  graph->nedge = nedge;
  graph->nvertex = nedge + 1;

  fp = GUARDED_FOPEN("../data/diadem_c1_conn.txt", "w");
  fprintf(fp, "%d\n", graph->nvertex);
  int i;
  for (i = 0; i < graph->nedge; i++) {
    fprintf(fp, "%d %d\n", graph->edges[i][0] + 1, graph->edges[i][1] + 1);
  }
  fclose(fp);

  Kill_String_Workspace(sw);

  fclose(fp);
#endif

  return 0;
}
