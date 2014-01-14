/**@file merge_neuron.c
 * @author Ting Zhao
 * @date 12-Apr-2013
 */

#include <string.h>
#include "tz_utilities.h"
#include "tz_stdint.h"
#include "tz_error.h"
#include "tz_stack_utils.h"
#include "tz_iarray.h"
#include "tz_objdetect.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_draw.h"
#include "tz_stack_relation.h"
#include "tz_stack_bwmorph.h"
#include "tz_voxel_graphics.h"
#include "tz_stack_attribute.h"
#include "tz_graph.h"
#include "tz_voxel_linked_list_ext.h"
#include "tz_stack_math.h"
#include "tz_stack_threshold.h"
#include "tz_stack_stat.h"
#include "tz_stack_code.h"
#include "tz_image_io.h"
#include "tz_int_histogram.h"
#include "tz_object_3d_linked_list_ext.h"
#include "tz_image_trans.h"
#include "tz_unipointer_linked_list.h"

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.2") == 1) {
    return 0;
  }

  static char *Spec[] = {"<input:string> -o <string> ",
    "[--dist_thre <int(10)>]",
    "[--hue_diff <double(0.5)>] [--channel_number<int>] [--verbose]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (!fexist(Get_String_Arg("input"))) {
    printf("Cannot open %s\n", Get_String_Arg("input"));

    return 1;
  }

  int channel_number;
  if (Is_Arg_Matched("--channel_number")) {
    channel_number = Get_Int_Arg("--channel_number");
  } else {
    channel_number = Infer_Neuron_File_Channel_Number(Get_String_Arg("input"));
  }

  FILE *fp = Guarded_Fopen(Get_String_Arg("input"), "rb", "Read_Neurons");
  if (fp == NULL) {
    fprintf(stderr, "Cannot open %s", Get_String_Arg("input"));
    return 1;
  }

  mylib_Prolog *prolog = Read_Prolog(fp, channel_number, 1, NULL);

  Neurons *neu = read_neurons(fp, channel_number);
  printf("%d neurons loaded\n", neu->numneu); 
  printf("Overall size: %d\n", (int) Neuron_Size(neu)); 

  fclose(fp);

  int width, height, depth;
  Region_Stack_Size(neu->neuron[0], &width, &height, &depth);

  Stack *stack = Make_Stack(GREY16, width, height, depth);

  int i;
  for (i = 0; i < neu->numneu; ++i) {
    Object_3d *obj = Region_To_Object_3d(neu->neuron[i]);
    Stack_Draw_Object_Bw(stack, obj, i+1);

    Kill_Object_3d(obj);
  }

  Stack *out = stack;

  Objlabel_Workspace *ow = New_Objlabel_Workspace();
  Default_Objlabel_Workspace(ow);

  int threshold = Get_Int_Arg("--dist_thre");

  if (Stack_Voxel_Number(out) > STACK_OBJLABLE_MAX_SIZE) {
    printf("Big stack. Downsampling ...\n");
    Stack *out2 = Downsample_Stack_Max(out, 1, 1, 0, NULL);
    Kill_Stack(out);
    out = out2;
    threshold /= 2;
  }

  if (Stack_Voxel_Number(out) > STACK_OBJLABLE_MAX_SIZE) {
    printf("Too big stack. Abort.\n");
    return 1;
  }

  Graph *graph = Stack_Label_Field_Neighbor_Graph(
      out, threshold, ow);

  /* Normalized the indices */
  for (i = 0; i < graph->nedge; ++i) {
    graph->edges[i][0]--;
    graph->edges[i][1]--;
  }
  graph->nvertex--;

  int *merge_status = iarray_calloc(graph->nvertex);

  int verbose = Is_Arg_Matched("--verbose");

  int nedge = 0;
  int *edge_index = (int*) malloc(sizeof(int) * graph->nedge);

  double hue_diff_thre = Get_Double_Arg("--hue_diff");

  char *data_directory = dname(Get_String_Arg("input"), NULL);
  char *vfile = fullpath(data_directory, prolog->vfile, NULL);

  printf("Loading %s ...\n", vfile);
  Mc_Stack *mstack = Read_Mc_Stack(vfile, -1);

  Rgb_Color all_color[neu->numneu];

  for (i = 0; i < neu->numneu; ++i) {
    Rgb_Color color = Region_Average_Color(neu->neuron[i], mstack);
    all_color[i] = color;
  }

  Kill_Mc_Stack(mstack);

  for (i = 0; i < graph->nedge; ++i) {
    int v1 = graph->edges[i][0];
    int v2 = graph->edges[i][1];
    if (Rgb_Color_Hue_Diff(all_color + v1, all_color + v2) < hue_diff_thre) {
      if (verbose) {
        printf("%d - %d: (%u, %u, %u), (%u, %u, %u)\n", v1, v2, 
            all_color[v1].r, all_color[v1].g, all_color[v1].b,
            all_color[v2].r, all_color[v2].g, all_color[v2].b);
      }
    } else {
      edge_index[nedge++] = i;
    }
  }

  Graph_Remove_Edge_List(graph, edge_index, nedge);

  if (verbose) {
    Print_Graph(graph);
  }

  if (graph->nedge == 0) {
    printf("No neurons merged.\n");
    Write_Neurons(Get_String_Arg("-o"), neu, prolog, channel_number);

    return 0;
  }


  Graph_Workspace *gw = New_Graph_Workspace();
  Graph *subgraph = Graph_Connected_Subgraph(graph, gw, 
      GRAPH_EDGE_NODE(graph, 0, 0));

  if (subgraph == NULL) {
    printf("Potential bug: no merging group found. Abort merging.\n"); 
    return 1;
  }

  int j;
  for (j = 0; j < subgraph->nedge; ++j) {
    merge_status[subgraph->edges[j][0]] = 1;
    merge_status[subgraph->edges[j][1]] = 1;
  }
  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);

  Unipointer_List *graph_list = NULL;
  Unipointer_List_Add(&graph_list, subgraph);

  int new_neuron_number = 1;

  for (i = 0; i < graph->nedge; i++) {
    if (gw->elist[i] == 0) {
      subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
      int j;
      for (j = 0; j < subgraph->nedge; ++j) {
        merge_status[subgraph->edges[j][0]] = 1;
        merge_status[subgraph->edges[j][1]] = 1;
      }
      Unipointer_List_Add_Last(graph_list, subgraph);
      ++new_neuron_number;
    }
  }
  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, FALSE);

  Unipointer_List *graph_list_node = graph_list;

  for (i = 0; i < graph->nvertex; ++i) {
    if (merge_status[i] == 0) {
      ++new_neuron_number;
    }
  }

  Neurons *new_neu = Make_Neurons(new_neuron_number, channel_number);

  int neuron_index = 0;
  int *index = (int*) malloc(sizeof(int) * graph->nvertex);
  while (graph_list_node != NULL) {
    subgraph = (Graph*) graph_list_node->data;

    if (verbose) {
      Print_Graph(subgraph);
    }

    Graph_Workspace *gw2 = New_Graph_Workspace();
    Arrayqueue array = Graph_Traverse_B(subgraph, 
        GRAPH_EDGE_NODE(subgraph, 0, 0), gw2);
    int nvertex = Arrayqueue_To_Array(&array, index);

    mylib_Region **merge_region = 
      (mylib_Region**) malloc(sizeof(mylib_Region*) * graph->nvertex);

    for (i = 0; i < nvertex; ++i) {
      merge_region[i] = neu->neuron[index[i]];
    }

    Region_Merge_M(merge_region[0], merge_region + 1, nvertex - 1);

    new_neu->neuron[neuron_index] = merge_region[0];
    memcpy(&(new_neu->color[neuron_index]), &(neu->color[index[0]]),
        sizeof(int) * channel_number);
    new_neu->cluster[neuron_index] = neu->cluster[index[0]];
    new_neu->status[neuron_index] = neu->status[index[0]];
    ++neuron_index;


    free(merge_region);
    merge_region = NULL;

    graph_list_node = graph_list_node->next;
  }

  for (i = 0; i < graph->nvertex; ++i) {
    if (merge_status[i] == 0) {
      new_neu->neuron[neuron_index] = neu->neuron[i];
      memcpy(&(new_neu->color[neuron_index]), &(neu->color[i]),
          sizeof(int) * channel_number);
      new_neu->cluster[neuron_index] = neu->cluster[i];
      new_neu->status[neuron_index] = neu->status[i];
      ++neuron_index;
      merge_status[i] = 1;
    }
  }

  printf("%d neurons after merge\n", new_neu->numneu);
  //printf("Overall size: %d\n", (int) Neuron_Size(new_neu)); 

  Write_Neurons(Get_String_Arg("-o"), new_neu, prolog, channel_number);

  return 0;
}
