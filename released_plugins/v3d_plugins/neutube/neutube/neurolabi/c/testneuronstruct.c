#include "tz_neuron_structure.h"
#include "tz_locseg_chain.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "tz_darray.h"
#include "tz_stack_attribute.h"

int main()
{
#if 0
  char *filepath = "../data/fly_neuron_n2/graph_d.swc";
  
  Neuron_Structure *ns = Neuron_Structure_From_Swc_File(filepath);
  
  Neuron_Component_Arraylist *comp_array =
    Neuron_Structure_Branch_Point(ns);

  filepath = "../data/fly_neuron_n2.tif";
  Stack *stack = Read_Stack(filepath);
  Translate_Stack(stack, COLOR, 1);

  int i;
  Stack_Draw_Workspace *ws = New_Stack_Draw_Workspace();
  for (i = 0; i < comp_array->length; i++) {
    Neuron_Component_Draw_Stack(comp_array->array + i, stack, ws);
  }
  Kill_Stack_Draw_Workspace(ws);

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  Stack *stack = NULL;

  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/fly_neuron_n3/chain0.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/fly_neuron_n3/chain10.tb");

  Connection_Test_Workspace *ws = New_Connection_Test_Workspace();
  Connection_Test_Workspace_Read_Resolution(ws, "../data/fly_neuron_n1.res");

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  Locseg_Chain_Connection_Test(chain1, chain2, stack, 1.0, &conn, ws);

  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  Locseg_Chain **chain = (Locseg_Chain**) malloc(sizeof(Locseg_Chain*) * 3);
  chain[0] = Read_Locseg_Chain("../data/mouse_single_org/chain4.tb");
  chain[1] = Read_Locseg_Chain("../data/mouse_single_org/chain19.tb");
  chain[2] = Read_Locseg_Chain("../data/mouse_single_org/chain64.tb");

  Stack *signal = Read_Stack("../data/mouse_single_org.tif");

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
    
  FILE *fp = fopen("../data/mouse_single_org.res", "r");

  darray_fscanf(fp, ctw->resolution, 3);

  Neuron_Component *chain_array = Make_Neuron_Component_Array(3);

  int i;
  for (i = 0; i < 3; i++) {
    Set_Neuron_Component(chain_array + i, 
			 NEUROCOMP_TYPE_LOCSEG_CHAIN, chain[i]);
  }

  Neuron_Structure *ns = Locseg_Chain_Comp_Neurostruct(chain_array, 
						       3, signal, 1.0, ctw);
  Graph *graph = ns->graph;

  Process_Neuron_Structure(ns);

  Print_Neuron_Structure(ns);

  Neuron_Structure_Crossover_Test(ns, 0.5375);

  printf("\ncross over changed: \n");
  Print_Neuron_Structure(ns);
#endif

#if 0
  Neuron_Structure *ns = Make_Neuron_Structure(5);

  Set_Neuron_Component(ns->comp, NEUROCOMP_TYPE_GEO3D_CIRCLE, 
		       New_Geo3d_Circle());
  Set_Neuron_Component(ns->comp + 1, NEUROCOMP_TYPE_GEO3D_CIRCLE, 
		       New_Geo3d_Circle());
  Set_Neuron_Component(ns->comp + 2, NEUROCOMP_TYPE_GEO3D_CIRCLE, 
		       New_Geo3d_Circle());
  Set_Neuron_Component(ns->comp + 3, NEUROCOMP_TYPE_GEO3D_CIRCLE, 
		       New_Geo3d_Circle());
  Set_Neuron_Component(ns->comp + 4, NEUROCOMP_TYPE_GEO3D_CIRCLE, 
		       New_Geo3d_Circle());

  NEUROCOMP_GEO3D_CIRCLE(ns->comp)->radius = 1.5;
  NEUROCOMP_GEO3D_CIRCLE(ns->comp + 1)->radius = 2.5;
  NEUROCOMP_GEO3D_CIRCLE(ns->comp + 2)->radius = 3.5;
  NEUROCOMP_GEO3D_CIRCLE(ns->comp + 3)->radius = 4.5;
  NEUROCOMP_GEO3D_CIRCLE(ns->comp + 4)->radius = 5.5;
 
  ns->graph = Make_Graph(5, 4, 0);
  //Graph_Add_Edge(ns->graph, 0, 1);
  Graph_Add_Edge(ns->graph, 1, 3);
  Graph_Add_Edge(ns->graph, 1, 4);
  Graph_Add_Edge(ns->graph, 0, 2);

  Graph_Set_Directed(ns->graph, TRUE);

  Print_Graph(ns->graph);

  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns, 1.0, NULL);
  Print_Swc_Tree(tree);

  Swc_Tree_To_Dot_File(tree, "../data/test2.dot");
#endif

#if 0
  int n;
  Neuron_Component *chain_array = Dir_Locseg_Chain_Nc("../data/fly_neuron_n22", 
						      "^chain.*\\.tb", 
						      &n, NULL);
  
  Neuron_Structure *ns =
    Locseg_Chain_Comp_Neurostruct(chain_array, n, NULL, 1.0, NULL);
  
  Process_Neuron_Structure(ns);
  Neuron_Structure* ns2=
      Neuron_Structure_Locseg_Chain_To_Circle(ns);

  Neuron_Structure_To_Tree(ns2);

  Graph_To_Dot_File(ns2->graph, "../data/test.dot");

  Swc_Tree *tree = 
    Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);

  Swc_Tree_Remove_Zigzag(tree);
  //Swc_Tree_Tune_Fork(tree);

  //Print_Swc_Tree(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Graph *graph = Neuron_Structure_Import_Xml_Graph("../data/mouse_single_org/trueconn2.xml");
  Graph_Normalize_Edge(graph);
  Graph_Remove_Duplicated_Edge(graph);

  Graph *graph2 = Neuron_Structure_Import_Xml_Graph("../data/mouse_single_org/conn.xml");
  Graph_Normalize_Edge(graph2);
  Graph_Remove_Duplicated_Edge(graph2);

  Graph_Workspace *gw = New_Graph_Workspace();
  int n = Graph_Edge_Count(graph, graph2->edges, graph2->nedge, gw);

  printf("fp: %d\n", graph2->nedge - n);
  printf("tp: %d\n", n);
  printf("fn: %d\n", graph->nedge - n);

  double p = (double) n / graph2->nedge;
  double r = (double) n / graph->nedge;
  printf("precision: %g\n", p);
  printf("recall: %g\n", r);
  printf("F-measure: %g\n", 2.0 * (p * r) / (p + r));
#endif

#if 0
  Neuron_Structure *ns = Make_Neuron_Structure(2);
  
  Local_Neuroseg *locseg = New_Local_Neuroseg();

  Locseg_Chain *chain1 = New_Locseg_Chain();
  Locseg_Chain_Add(chain1, locseg, NULL, DL_TAIL);

  Set_Neuron_Component(ns->comp, NEUROCOMP_TYPE_LOCSEG_CHAIN, chain1);

  Locseg_Chain *chain2 = New_Locseg_Chain();
  locseg = New_Local_Neuroseg();
  double bottom[3] = {10, 10, 5};
  double top[3] = {5, 5, 5};
  Local_Neuroseg_Set_Bottom_Top(locseg, bottom, top);
  Locseg_Chain_Add(chain2, locseg, NULL, DL_TAIL);

  Set_Neuron_Component(ns->comp + 1, NEUROCOMP_TYPE_LOCSEG_CHAIN, chain2);
  
  Neurocomp_Conn *conn = New_Neurocomp_Conn();
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  Locseg_Chain_Connection_Test(chain2, chain1, NULL, 1.0, conn, ctw);

  Neuron_Structure_Add_Conn(ns, 1, 0, conn);

  Print_Neuron_Structure(ns);

  Neuron_Structure *ns2 = 
    Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);

  Neuron_Structure_To_Swc_File(ns2, "../data/test.swc");
#endif

#if 0
  int n;
  
  Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd("../data/diadem_a1_part3", 
						   "^chain.*\\.tb", &n, NULL);
  /*
  Locseg_Chain **chain_array = 
    Locseg_Chain_Import_List("../data/diadem_a1_part2/good_tube.txt", &n);
  */
  //n = 100;
  /*
  Locseg_Chain **chain_array = 
    (Locseg_Chain**) malloc(sizeof(Locseg_Chain*) * 2);
  n = 2;
  chain_array[0] = Read_Locseg_Chain("../data/diadem_a1_part2/chain58.tb");
  chain_array[1] = Read_Locseg_Chain("../data/diadem_a1_part2/chain154.tb");
  */

  Stack *stack = Read_Stack("../data/diadem_a1_part3.tif");

  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(mask);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.0375 * 2.0;
  sgw->resolution[1] = 0.0375 * 2.0;
  sgw->resolution[2] = 0.33;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);

  Neuron_Structure *ns = 
    Locseg_Chain_Sp_Grow_Reconstruct(chain_array, n, stack, 1.0, sgw);
  
  Print_Neuron_Structure(ns);
  
  Graph_To_Dot_File(ns->graph, "../data/test.dot");

  //Neuron_Structure_To_Swc_File(ns, "../data/test.swc");
  Neuron_Structure *ns2 = 
    Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
  //double root[3] = {31, 430, 0};
  double root[3] = {1221, 449, 8.5};
  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, root);
  Swc_Tree_Clean_Root(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  int n;
  Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd("../data/diadem_e1", 
						   "^chain.*\\.tb", &n, NULL);
  //n = 100;
  /*
  Locseg_Chain **chain_array = 
    (Locseg_Chain**) malloc(sizeof(Locseg_Chain*) * 2);
  n = 2;
  chain_array[0] = Read_Locseg_Chain("../data/diadem_a1_part2/chain58.tb");
  chain_array[1] = Read_Locseg_Chain("../data/diadem_a1_part2/chain154.tb");
  */

  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(mask);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.3296485;
  sgw->resolution[1] = 0.3296485;
  sgw->resolution[2] = 1.0;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);

  Neuron_Structure *ns = 
    Locseg_Chain_Sp_Grow_Reconstruct(chain_array, n, stack, 1.0, sgw);
  
  Print_Neuron_Structure(ns);
  
  //Neuron_Structure_To_Swc_File(ns, "../data/test.swc");
  Neuron_Structure *ns2 = 
    Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);

  Graph_To_Dot_File(ns2->graph, "../data/test.dot");

  double root[3] = {31, 430, 0};
  //double root[3] = {4882, 1797, 19};
  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, root);
  //Swc_Tree_Clean_Root(tree);
  Swc_Tree_Resort_Id(tree);

  Write_Swc_Tree("../data/test2.swc", tree);
#endif

#if 0
  int n;
  Locseg_Chain **chain_array = 
    Dir_Locseg_Chain_Nd("../data/benchmark/stack_graph/fork", "^chain.*\\.tb", 
			&n, NULL);

  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(mask);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);
  Neuron_Structure *ns = 
    Locseg_Chain_Sp_Grow_Reconstruct(chain_array, n, stack, 1.0, sgw);
  
  Neuron_Structure *ns2 = 
    Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);
  //Swc_Tree_Clean_Root(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  int n = 3;
  Locseg_Chain **chain_array = 
    (Locseg_Chain**) malloc(sizeof(Locseg_Chain) * n);
  chain_array[0] = Read_Locseg_Chain("/Users/zhaot/Work/neurolabi/data/benchmark/stack_graph/fork/chain0.tb");
  chain_array[1] = Read_Locseg_Chain("/Users/zhaot/Work/neurolabi/data/benchmark/stack_graph/fork/chain1.tb");
  chain_array[2] = New_Locseg_Chain();
  
  printf("%d\n", Locseg_Chain_Is_Empty(chain_array[2]));

  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(mask);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);
  Neuron_Structure *ns = 
    Locseg_Chain_Sp_Grow_Reconstruct(chain_array, n, stack, 1.0, sgw);
  
  Neuron_Structure *ns2 = 
    Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);
  //Swc_Tree_Clean_Root(tree);
  Swc_Tree_Remove_Zigzag(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test2.swc", tree);
#endif

#if 0
  Neuron_Structure *ns = New_Neuron_Structure();
  ns->graph = New_Graph();

  Graph_Add_Edge(ns->graph, 0, 1);
  Graph_Add_Edge(ns->graph, 0, 2);
  Graph_Add_Edge(ns->graph, 2, 3);
  Graph_Add_Edge(ns->graph, 2, 4);
  Graph_Add_Edge(ns->graph, 4, 5);
  Graph_Add_Edge(ns->graph, 5, 6);
  
  ns->conn = (Neurocomp_Conn*) malloc(sizeof(Neurocomp_Conn) * ns->graph->nedge);

  ns->conn[0].info[0] = 0;
  ns->conn[0].info[1] = 0;
  ns->conn[0].cost = 0.0;
  ns->conn[0].mode = NEUROCOMP_CONN_LINK;

  ns->conn[1].info[0] = 1;
  ns->conn[1].info[1] = 1;
  ns->conn[1].cost = 0.0;
  ns->conn[1].mode = NEUROCOMP_CONN_LINK;

  ns->conn[2].info[0] = 0;
  ns->conn[2].info[1] = 0;
  ns->conn[2].cost = 0.0;
  ns->conn[2].mode = NEUROCOMP_CONN_LINK;

  ns->conn[3].info[0] = 1;
  ns->conn[3].info[1] = 0;
  ns->conn[3].cost = 1.0;
  ns->conn[3].mode = NEUROCOMP_CONN_HL;

  ns->conn[4].info[0] = 0;
  ns->conn[4].info[1] = 1;
  ns->conn[4].cost = 0.0;
  ns->conn[4].mode = NEUROCOMP_CONN_LINK;

  ns->conn[5].info[0] = 1;
  ns->conn[5].info[1] = 1;
  ns->conn[5].cost = 1.0;
  ns->conn[5].mode = NEUROCOMP_CONN_LINK;

  Neuron_Structure_Merge_Locseg_Chain(ns);
  
#endif

#if 0
  Neuron_Structure *ns = New_Neuron_Structure();
  ns->graph = New_Graph();
  ns->comp = Dir_Locseg_Chain_Nc("../data/diadem_e3", "^chain.*\\.tb", 
				 &(ns->graph->nvertex), NULL);

  Graph_Add_Edge(ns->graph, 0, 1);
  Graph_Add_Edge(ns->graph, 0, 2);
  Graph_Add_Edge(ns->graph, 2, 3);
  Graph_Add_Edge(ns->graph, 3, 4);
  Graph_Add_Edge(ns->graph, 4, 5);
  
  ns->conn = (Neurocomp_Conn*) malloc(sizeof(Neurocomp_Conn) * ns->graph->nedge);

  ns->conn[0].info[0] = 0;
  ns->conn[0].info[1] = 0;
  ns->conn[0].cost = 0.0;
  ns->conn[0].mode = NEUROCOMP_CONN_LINK;

  ns->conn[1].info[0] = 0;
  ns->conn[1].info[1] = 0;
  ns->conn[1].cost = 0.0;
  ns->conn[1].mode = NEUROCOMP_CONN_LINK;

  ns->conn[2].info[0] = 0;
  ns->conn[2].info[1] = 0;
  ns->conn[2].cost = 1.0;
  ns->conn[2].mode = NEUROCOMP_CONN_LINK;

  ns->conn[3].info[0] = 0;
  ns->conn[3].info[1] = 0;
  ns->conn[3].cost = 2.0;
  ns->conn[3].mode = NEUROCOMP_CONN_LINK;

  ns->conn[4].info[0] = 0;
  ns->conn[4].info[1] = 0;
  ns->conn[4].cost = 0.0;
  ns->conn[4].mode = NEUROCOMP_CONN_LINK;

  int i;
  for (i = 0; i < ns->graph->nvertex; i++) {
    printf("%d ", Locseg_Chain_Length(NEUROCOMP_LOCSEG_CHAIN(ns->comp+i)));
  }
  printf("\n");

  Neuron_Structure_Merge_Locseg_Chain(ns);  

  for (i = 0; i < ns->graph->nvertex; i++) {
    printf("%d ", Locseg_Chain_Length(NEUROCOMP_LOCSEG_CHAIN(ns->comp+i)));
  }
  printf("\n");

#endif

#if 0
  int n;
  Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd("../data/diadem_e1", 
						   "^chain.*\\.tb", &n, NULL);
  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(mask);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.3296485;
  sgw->resolution[1] = 0.3296485;
  sgw->resolution[2] = 1.0;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;

  Stack_Sp_Grow_Infer_Parameter(sgw, stack);

  Neuron_Structure *ns = 
    Locseg_Chain_Sp_Grow_Reconstruct(chain_array, n, stack, 1.0, sgw);  
  
  Neuron_Structure_Merge_Locseg_Chain(ns);
  int i;
  char filepath[100];
  for (i = 0; i < ns->graph->nvertex; i++) {
    Locseg_Chain_Regularize(NEUROCOMP_LOCSEG_CHAIN(ns->comp+i));
    if (Locseg_Chain_Is_Empty(NEUROCOMP_LOCSEG_CHAIN(ns->comp+i)) == FALSE) {
      sprintf(filepath, "../data/tmp/chain%d.tb", i);
      Write_Locseg_Chain(filepath, NEUROCOMP_LOCSEG_CHAIN(ns->comp+i));
    }
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/fork2/fork2.tif");

  Neuron_Structure *ns = New_Neuron_Structure();
  ns->graph = New_Graph();
  ns->comp = Dir_Locseg_Chain_Nc("../data/benchmark/fork2/tubes",
				 "^chain.*\\.tb", &(ns->graph->nvertex), NULL);

  coordinate_3d_t roots[3];
  roots[0][0] = 51;
  roots[0][1] = 23;
  roots[0][2] = 60;

  roots[1][0] = 51;
  roots[1][1] = 23;
  roots[1][2] = 40;

  roots[2][0] = 25;
  roots[2][1] = 76;
  roots[2][2] = 60;

  Neuron_Structure_Break_Root(ns, roots, 3);
  Neuron_Structure_Load_Root(ns, roots, 3);

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->dist_thre = 100.0;
  ctw->sp_test = FALSE;
  
  Locseg_Chain_Comp_Neurostruct_W(ns, stack, 1.0, ctw);

  Process_Neuron_Structure(ns);
  Neuron_Structure_To_Tree(ns);
  
  /*
  Neuron_Structure_Remove_Conn(ns, 0, 2);
  Neuron_Structure_Remove_Conn(ns, 2, 0);
  */

  Neuron_Structure_Remove_Negative_Conn(ns);

  Neuron_Structure* ns2= NULL;
  
  ns2 = Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
    
  Neuron_Structure_To_Tree(ns2);
  
  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);
  
  Swc_Tree_Resort_Id(tree);

  Write_Swc_Tree("../data/test3.swc", tree);  
#endif

#if 1
  Stack *stack = NULL;

  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/chain22.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/benchmark/diadem/diadem_e1/chain0.tb");

  Connection_Test_Workspace *ws = New_Connection_Test_Workspace();
  Connection_Test_Workspace_Read_Resolution(ws, "../data/diadem_e3.res");

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  Locseg_Chain_Connection_Test(chain1, chain2, stack, 1.0, &conn, ws);

  Print_Neurocomp_Conn(&conn);
#endif

  return 0;
}
