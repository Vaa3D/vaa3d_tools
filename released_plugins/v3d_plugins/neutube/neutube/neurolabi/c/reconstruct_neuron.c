/* reconstruct_neuron
 *
 * 12-Jul-2008
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "tz_error.h"
#include "tz_constant.h"
#include "tz_stack_math.h"
#include "tz_stack_utils.h"
#include "tz_stack_threshold.h"
#include "tz_stack_draw.h"
#include "tz_vrml_io.h"
#include "tz_geo3d_vector.h"
#include "tz_stack_bwmorph.h"
//#include "tz_stack_bwdist.h"
#include "tz_vrml_material.h"
#include "tz_string.h"
#include "tz_iarray.h"
#include "tz_trace_utils.h"
#include "tz_stack_attribute.h"
#include "tz_locseg_chain.h"
#include "tz_geo3d_utils.h"
#include "tz_locseg_chain_network.h"
#include "tz_darray.h"
#include "tz_image_io.h"
#include "tz_xml_utils.h"

/*
 * reconstruct_neuron - reconstruct neurons from individual tubes
 *
 * reconstruct_neuron [-R<string>] [-T<string>] -D<string> [-minlen <int>]
 *                    [-I<string>] -o <string> [-root <double> <double> <double>]
 *
 * -R: region label stack (>1 for regions of interest)
 * -T: region type stack (2: arbor, 3: soma)
 * -M: region map file (from label to type)
 */

void normalize_dir(char *dir)
{
  int len = strlen(dir);
  if (len > 1) {
    if (dir[len - 1] == '/') {
      dir[len - 1] = '\0';
    }
  }
}

char* full_path(const char *dir, const char *file) 
{
  static char file_path[MAX_PATH_LENGTH];
  strcpy(file_path, dir);
  normalize_dir(file_path);
  strcat(file_path, "/");
  strcat(file_path, file);

  return file_path;
}

char *get_chain_path(const char *dir, int n)
{
  static char file_path[MAX_PATH_LENGTH];
  strcpy(file_path, dir);
  normalize_dir(file_path);
  sprintf(file_path, "%s/chain%d.bn", file_path, n);

  return file_path;
}

char *get_new_chain_path(const char *dir, int n)
{
  static char file_path[MAX_PATH_LENGTH];
  strcpy(file_path, dir);
  normalize_dir(file_path);
  sprintf(file_path, "%s/newchain%d.bn", file_path, n);

  return file_path;
}

char *get_new_chain_vrml_path(const char *dir, int n)
{
  static char file_path[MAX_PATH_LENGTH];
  strcpy(file_path, dir);
  normalize_dir(file_path);
  sprintf(file_path, "%s/newchain%d.wrl", file_path, n);

  return file_path;
}

/*
char *get_chain_map_path(const char *dir, const int *chain_map, int n) 
{
  return get_chain_path(dir, chain_map[n]);
}

char *get_new_chain_map_path(const char *dir, const int *chain_map, int n) 
{
  return get_new_chain_path(dir, chain_map[n]);
}
*/
Locseg_Chain *get_chain(const char *dir, int n)
{
  if (fexist(get_new_chain_path(dir, n))) {
    return Read_Locseg_Chain(get_new_chain_path(dir, n));
  } else {
    return Read_Locseg_Chain(get_chain_path(dir, n));
  }
}

#if 0
static void neuron_structure_suppress(Neuron_Structure *ns, 
				      coordinate_3d_t *roots, int n)
{
  int i, j;
  int supp_idx;
  int supp_end;
  for (i = 0; i < Neuron_Structure_Component_Number(ns); i++) {
    supp_idx = -1;
    for (j = 0; j < n; j++) {
      Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(ns->comp + i);
      /*
      int seg_index = Locseg_Chain_Hit_Test(chain, DL_HEAD, roots[j][0],
					    roots[j][1], roots[j][2]);
      */
      int seg_index = 0;
      double d = Locseg_Chain_Point_Dist(chain, roots[j], &seg_index, NULL);
      if (d < 5.0) {
	supp_idx = i;
	if (seg_index < Locseg_Chain_Length(chain) / 2) {
	  supp_end = DL_HEAD;
	} else {
	  supp_end = DL_TAIL;
	}
	break;
      }
    }
    if (supp_idx >= 0) {
      for (j = 0; j < Neuron_Structure_Link_Number(ns); j++) {
	if (((supp_idx == ns->graph->edges[j][0]) && 
	     (supp_end == ns->conn[j].info[0])) ||
	    ((supp_idx == ns->graph->edges[j][1]) && 
	     (supp_end == ns->conn[j].info[1]) &&
	     (ns->conn[j].mode = NEUROCOMP_CONN_LINK))) {
	  if (ns->conn[j].cost >= 0.0) {
	    ns->conn[j].mode = NEUROCOMP_CONN_NONE;
	  }
	}
      }
    }
  }
}
#endif

static void neuron_structure_suppress(Neuron_Structure *ns, 
				      coordinate_3d_t *roots, int n)
{
  int i, j;
  int supp_idx;
  int supp_end;
  for (j = 0; j < n; j++) {
    supp_idx = -1;
    double min_dist = Infinity;
    for (i = 0; i < Neuron_Structure_Component_Number(ns); i++) {
      Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(ns->comp + i);
      int seg_index = 0;
      double d = Locseg_Chain_Point_Dist(chain, roots[j], &seg_index, NULL);
      if (min_dist > d) {
	min_dist = d;
	if (d < 5.0) {
	  supp_idx = i;
	  if (seg_index < Locseg_Chain_Length(chain) / 2) {
	    supp_end = DL_HEAD;
	  } else {
	    supp_end = DL_TAIL;
	  }
	}
      }
    }

    if (supp_idx >= 0) {
      int k;
      for (k = 0; k < Neuron_Structure_Link_Number(ns); k++) {
	if (((supp_idx == ns->graph->edges[k][0]) && 
	     (supp_end == ns->conn[k].info[0])) ||
	    ((supp_idx == ns->graph->edges[k][1]) && 
	     (supp_end == ns->conn[k].info[1]) &&
	     (ns->conn[k].mode = NEUROCOMP_CONN_LINK))) {
	  if (ns->conn[k].cost >= 0.0) {
	    ns->conn[k].mode = NEUROCOMP_CONN_NONE;
	  }
	}
      }
    }
  }
}

/*
static double local_neuroseg_zscore(const Local_Neuroseg *locseg)
{
  double pos[3];
  Local_Neuroseg_Bottom(locseg, pos);
  double zbottom = pos[2];
  Local_Neuroseg_Top(locseg, pos);
  double ztop = pos[2];

  return fabs(zbottom - ztop) / locseg->seg.h;
}
*/
static void kill_locseg_chain_array(Locseg_Chain **chain, int n)
{
  int i;
  for (i = 0; i < n; i++) {
    Kill_Locseg_Chain(chain[i]);
  }
  free(chain);
}

#if 0
static void swc_tree_remove_zjump(Swc_Tree *tree, double thre)
{
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
}
#endif

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.0") == 1) {
    return 0;
  }

  static char *Spec[] = {
    "[-R<string> -T<string> -M<string>] -D<string> [-minlen <double>]",
    "[-root <double> <double> <double>] [-trans <double> <double> <double>]",
    "[-rtlist <string>] [-sup_root] [-dist <double>]",
    "[-C<string>] [-I<string>] [-z <double>] -o <string> [-b] [-res <string>]",
    "[-screen] [-sp] [-intp] [-sl] [-rb] [-rz] [-rs] [-ct] [-al <double>]",
    "[-screenz <double>] [-force_merge <double>] [-ct_break <double>]",
    "[-jumpz <double>] [-single_break]",
    NULL};

  Print_Arguments(argc, argv);

  Process_Arguments(argc, argv, Spec, 1);
  
  char *dir = Get_String_Arg("-D");

  Stack_Document *stack_doc = NULL;
  if (Is_Arg_Matched("-I")) {
    if (!fexist(Get_String_Arg("-I"))) {
      PRINT_EXCEPTION("File does not exist", "");
      fprintf(stderr, "%s cannot be found.\n", Get_String_Arg("-I"));
      return 1;
    }
    if (fhasext(Get_String_Arg("-I"), "xml")) {
      stack_doc = Xml_Read_Stack_Document(Get_String_Arg("-I"));
    }
  }

  /* Get number of chains */
  int chain_number2 = dir_fnum_p(dir, "^chain.*\\.tb");

  if (chain_number2 == 0) {
    printf("No tube found.\n");
    printf("Quit reconstruction.\n");
    return 1;
  }

  int i;
  int *chain_map = iarray_malloc(chain_number2);
  int chain_number;
  Locseg_Chain **chain_array =
    Dir_Locseg_Chain_Nd(dir, "chain.*\\.tb", &chain_number, chain_map);

  if (Is_Arg_Matched("-screenz")) {
    Locseg_Chain_Array_Screen_Z(chain_array, chain_number,
	Get_Double_Arg("-screenz"));
  }

  if (Is_Arg_Matched("-single_break")) {
    int i;
    for (i = 0; i < chain_number; i++) {
      if (Locseg_Chain_Length(chain_array[i]) == 1) {
	/* break the segment into two parts */
	Locseg_Chain_Break_Node(chain_array[i], 0, 0.5);
      }
    }
  }

  if (Is_Arg_Matched("-ct_break")) {
    int tmp_chain_number;
    Locseg_Chain **tmp_chain_array = 
      Locseg_Chain_Array_Break_Jump(chain_array, chain_number,
	  Get_Double_Arg("-ct_break"), &tmp_chain_number);
    kill_locseg_chain_array(chain_array, chain_number);
    chain_array = tmp_chain_array;
    chain_number = tmp_chain_number;
  }

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  if (Is_Arg_Matched("-res")) {
    FILE *fp = fopen(Get_String_Arg("-res"), "r");
    if (fp != NULL) {
      if (darray_fscanf(fp, ctw->resolution, 3) != 3) {
	fprintf(stderr, "Failed to load %s\n", Get_String_Arg("-res"));
	ctw->resolution[0] = 1.0;
	ctw->resolution[1] = 1.0;
	ctw->resolution[2] = 1.0;
      } else {
	ctw->unit = 'u';
      }
      fclose(fp);
    } else {
      fprintf(stderr, "Failed to load %s. The file may not exist.\n", 
	      Get_String_Arg("-res"));
    }
  } else if (stack_doc != NULL) {
    ctw->resolution[0] = stack_doc->resolution[0];
    ctw->resolution[1] = stack_doc->resolution[1];
    ctw->resolution[2] = stack_doc->resolution[2];
  }

  if (Is_Arg_Matched("-force_merge")) {
    Connection_Test_Workspace *ws = New_Connection_Test_Workspace();
    ws->dist_thre = Get_Double_Arg("-force_merge");
    ws->interpolate = FALSE;
    ws->resolution[2] = ctw->resolution[2] / ctw->resolution[0];
    for (i = 0; i < chain_number; i++) {
      //Locseg_Chain_Correct_Ends(chain_array[i]); 
    }
    Locseg_Chain_Array_Force_Merge(chain_array, chain_number, ws); 
    Kill_Connection_Test_Workspace(ws);
  }

  chain_number2 = 0;
  Neuron_Component *chain_array2;
  GUARDED_MALLOC_ARRAY(chain_array2, chain_number, Neuron_Component); 
  for (i = 0; i < chain_number; i++) {
    if (Locseg_Chain_Is_Empty(chain_array[i]) == FALSE) {
      chain_map[chain_number2] = chain_map[i];
      Set_Neuron_Component(chain_array2+(chain_number2++), 
	  NEUROCOMP_TYPE_LOCSEG_CHAIN, chain_array[i]);
    } else {
      printf("chain_%d is empty.\n", chain_map[i]);
    }
  }
    /*
    Dir_Locseg_Chain_Nc(dir, "^chain.*\\.tb", &chain_number2, chain_map);
*/
  Stack *signal = NULL;
  //Stack *canvas = NULL;
  if (Is_Arg_Matched("-I")) {
    signal = Read_Stack_U(Get_String_Arg("-I"));
    //canvas = Translate_Stack(signal, COLOR, 0);
  } else {
    if (Is_Arg_Matched("-screen")) {
      perror("The -screen option requires -I option to be supplied.\n");
      return 1;
    }
  }

  /* Minimal tube length. */
  double minlen = 25.0;
  if (Is_Arg_Matched("-minlen")) {
    minlen = Get_Double_Arg("-minlen");
  }

  chain_number = 0;
  //int i;


  if (signal != NULL) {
    ctw->mask = Make_Stack(GREY, signal->width, signal->height, signal->depth);
    One_Stack(ctw->mask);
  }

  FILE *result_file = fopen(full_path(dir, Get_String_Arg("-o")), "w");


  double z_scale = 1.0;
  if (Is_Arg_Matched("-z")) {
    z_scale = Get_Double_Arg("-z");
  }


  /* Array to store corrected chains */
  Neuron_Component *chain_array_c = Make_Neuron_Component_Array(chain_number2);

  int screen = 0;

  double average_intensity = 0.0;

  if (Is_Arg_Matched("-screen")) {
    int good_chain_number = 0;
    int bad_chain_number = 0;
    for (i = 0; i < chain_number2; i++) {
      Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(chain_array2 + i);

      average_intensity += Locseg_Chain_Average_Score(chain, signal, z_scale, 
						      STACK_FIT_MEAN_SIGNAL);

      if ((Locseg_Chain_Geolen(chain) > 55) || 
	  (Locseg_Chain_Average_Score(chain, signal, z_scale, 
				      STACK_FIT_CORRCOEF) > 0.6)) {
	good_chain_number++;
      } else {
	bad_chain_number++;
      }
    }
    
    printf("good %d bad %d\n", good_chain_number, bad_chain_number);

    if (good_chain_number + bad_chain_number > 50) {
      if (bad_chain_number > good_chain_number) {
	screen = 1;
      }
    } else {
      screen = 3;
      /*
      if (bad_chain_number > good_chain_number * 2) {
	screen = 2;
      }
      */
    }
  }

  average_intensity /= chain_number2;

  /* build chain map */
  for (i = 0; i < chain_number2; i++) {
    Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(chain_array2 + i);
    BOOL good = FALSE;
    
    switch (screen) {
    case 1:
    case 2:
      if ((Locseg_Chain_Geolen(chain) > 100) || 
	  (Locseg_Chain_Average_Score(chain, signal, z_scale, 
				      STACK_FIT_CORRCOEF)
	   > 0.6)) {
	good = TRUE;
      } else {
	if (Locseg_Chain_Geolen(chain) < 100) {
	  if ((Locseg_Chain_Average_Score(chain, signal, z_scale, 
					 STACK_FIT_CORRCOEF) > 0.5) ||
	      (Locseg_Chain_Average_Score(chain, signal, z_scale, 
					  STACK_FIT_MEAN_SIGNAL) > 
	       average_intensity)) {
	    good = TRUE;
	  }
	}
      }
      break;
    case 3:
      if ((Locseg_Chain_Average_Score(chain, signal, z_scale, 
				      STACK_FIT_CORRCOEF) > 0.50) ||
	  (Locseg_Chain_Average_Score(chain, signal, z_scale, 
				      STACK_FIT_MEAN_SIGNAL) > 
	   average_intensity)) {
	good = TRUE;
      }
      break;
    default:
      good = TRUE;
    }

    if (good == TRUE) {
      if (Locseg_Chain_Geolen(chain) < minlen) {
	good = FALSE;
      }
    }

    if (good == TRUE) {
      Locseg_Chain *tmpchain = chain;
      if (signal != NULL) {
	//Locseg_Chain_Trace_Np(signal, 1.0, tmpchain, tw);
	Locseg_Chain_Erase(chain, ctw->mask, 1.0);
      }
      fprintf(result_file, "%d %d\n", chain_number, chain_map[i]);
      chain_map[chain_number] = chain_map[i];
      if (z_scale != 1.0) {
	Locseg_Chain_Scale_Z(chain, z_scale);
      }
      Set_Neuron_Component(chain_array_c + chain_number, 
			   NEUROCOMP_TYPE_LOCSEG_CHAIN, tmpchain);
      chain_number++;
    } else {
#ifdef _DEBUG_
      printf("chain%d is excluded.\n", i);
      /*
      char tmpfile[500];
      sprintf(tmpfile, "../data/diadem_c1/bad_chain/chain%d.tb", i);
      Write_Locseg_Chain(tmpfile, chain);
      */
#endif
    }
  }

  z_scale = 1.0;

  fprintf(result_file, "#\n");

  //Int_Arraylist *hit_spots = Int_Arraylist_New(0, chain_number);
  /* reconstruct neuron */

  if (Is_Arg_Matched("-res")) {
    FILE *fp = fopen(Get_String_Arg("-res"), "r");
    if (fp != NULL) {
      if (darray_fscanf(fp, ctw->resolution, 3) != 3) {
	fprintf(stderr, "Failed to load %s\n", Get_String_Arg("-res"));
	ctw->resolution[0] = 1.0;
	ctw->resolution[1] = 1.0;
	ctw->resolution[2] = 1.0;
      } else {
	ctw->unit = 'u';
      }
      fclose(fp);
    } else {
      fprintf(stderr, "Failed to load %s. The file may not exist.\n", 
	      Get_String_Arg("-res"));
    }
  } else if (stack_doc != NULL) {
    ctw->resolution[0] = stack_doc->resolution[0];
    ctw->resolution[1] = stack_doc->resolution[1];
    ctw->resolution[2] = stack_doc->resolution[2];
  }

  if (!Is_Arg_Matched("-sp")) {
    ctw->sp_test = FALSE;
    if (ctw->sp_test == FALSE) {
      ctw->dist_thre = NEUROSEG_DEFAULT_H / 2.0;
    }
  } else {
    ctw->dist_thre = NEUROSEG_DEFAULT_H * 1.5;
  }
  
  if (Is_Arg_Matched("-dist")) {
    ctw->dist_thre = Get_Double_Arg("-dist");
  }

  if (!Is_Arg_Matched("-intp")) {
    ctw->interpolate = FALSE;
  }
  //ctw->dist_thre = 100.0;

  double *tube_offset = NULL;
  if (Is_Arg_Matched("-trans")) {
    tube_offset = darray_malloc(3);
    tube_offset[0] = Get_Double_Arg("-trans", 1);
    tube_offset[1] = Get_Double_Arg("-trans", 2);
    tube_offset[2] = Get_Double_Arg("-trans", 3);
  } else {
    if (stack_doc != NULL) {
      tube_offset = darray_malloc(3);
      tube_offset[0] = stack_doc->offset[0];
      tube_offset[1] = stack_doc->offset[1];
      tube_offset[2] = stack_doc->offset[2];
    }
  }

  Neuron_Structure *ns = New_Neuron_Structure();
  ns->comp = chain_array_c;
  ns->graph = New_Graph();
  ns->graph->nvertex = chain_number;
  
  if (Is_Arg_Matched("-rtlist")) {
    int m, n;
    double *d = darray_load_matrix(Get_String_Arg("-rtlist"), NULL, &m, &n);

    if (n > 0) {
      coordinate_3d_t *roots = GUARDED_MALLOC_ARRAY(roots, n, coordinate_3d_t);
      int i;
      for (i = 0; i < n; i++) {
	if (Is_Arg_Matched("-trans")) {
	  roots[i][0] = d[i*3] - tube_offset[0];
	  roots[i][1] = d[i*3 + 1] - tube_offset[1];
	  roots[i][2] = d[i*3 + 2] - tube_offset[2];
	} else {
	  roots[i][0] = d[i*3];
	  roots[i][1] = d[i*3 + 1];
	  roots[i][2] = d[i*3 + 2];
	}
      }

      Neuron_Structure_Break_Root(ns, roots, n);
      Neuron_Structure_Load_Root(ns, roots, n);
    }
  }
  
  Locseg_Chain_Comp_Neurostruct_W(ns, signal, z_scale, ctw);

  if (tube_offset != NULL) {
    for (i = 0; i < chain_number; i++) {
      Locseg_Chain_Translate(NEUROCOMP_LOCSEG_CHAIN(chain_array_c + i), 
			     tube_offset);
    }
  }

  /*  
  Neuron_Structure *ns = Locseg_Chain_Comp_Neurostruct(chain_array, 
						       chain_number,
						       signal, z_scale, ctw);
  */

  FILE *tube_fp = fopen(full_path(dir, "tube.swc"), "w");
  int start_id = 1;

  for (i = 0; i < chain_number; i++) {
    int node_type = i % 10;
    int n = Locseg_Chain_Swc_Fprint_T(tube_fp, 
				      NEUROCOMP_LOCSEG_CHAIN(chain_array_c + i), 
				      node_type, start_id, 
				      -1, DL_FORWARD, 1.0, NULL);
    start_id += n;
  }
  fclose(tube_fp);

  //Neuron_Structure_To_Swc_File(ns, full_path(dir, "tube.swc"));
  /*
  Graph *testgraph = New_Graph(0, 0, FALSE);
  Int_Arraylist *cidx = Make_Int_Arraylist(0, 2);
  Int_Arraylist *sidx = Make_Int_Arraylist(0, 2);
  
  Locseg_Chain_Network_Simlify(&net, testgraph, cidx, sidx);
  */

  /* Find branch points */
  //Locseg_Chain *branches = Locseg_Chain_Network_Find_Branch(ns);

  //Graph *graph = Locseg_Chain_Graph(chain_array, chain_number, hit_spots);
  //Graph *graph = ns->graph;

  if (Is_Arg_Matched("-sup_root")) {
    if (Is_Arg_Matched("-rtlist")) {
      int m, n;
      double *d = darray_load_matrix(Get_String_Arg("-rtlist"), NULL, &m, &n);
      
      if (n > 0) {
	coordinate_3d_t *roots = 
	  GUARDED_MALLOC_ARRAY(roots, n, coordinate_3d_t);
	int i;
	for (i = 0; i < n; i++) {
	  roots[i][0] = d[i*3];
	  roots[i][1] = d[i*3 + 1];
	  roots[i][2] = d[i*3 + 2];
	  /*
	  if (tube_offset != NULL) {
	    roots[i][0] += tube_offset[0];
	    roots[i][1] += tube_offset[1];
	    roots[i][2] += tube_offset[2];
	  }
	  */
	}
	neuron_structure_suppress(ns, roots, n);
	free(roots);
      }
    }
  }

  Process_Neuron_Structure(ns);

  Print_Neuron_Structure(ns);

#ifdef _DEBUG_
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    printf("chain_%d (%d) -- chain_%d (%d) ", 
	chain_map[ns->graph->edges[i][0]], 
	ns->graph->edges[i][0], 
	chain_map[ns->graph->edges[i][1]],
	ns->graph->edges[i][1]);
    Print_Neurocomp_Conn(ns->conn + i);
  }
#endif

  if (Is_Arg_Matched("-ct")) {
    Neuron_Structure_Crossover_Test(ns, 
				    ctw->resolution[0] / ctw->resolution[2]);
  }

  if (Is_Arg_Matched("-al")) {
    Neuron_Structure_Adjust_Link(ns, Get_Double_Arg("-al"));
  }

  Neuron_Structure_To_Tree(ns);
  Neuron_Structure_Remove_Negative_Conn(ns);

#ifdef _DEBUG_
  printf("\nTree:\n");
  for (i = 0; i < NEURON_STRUCTURE_LINK_NUMBER(ns); i++) {
    printf("chain_%d (%d) -- chain_%d (%d) ", 
	chain_map[ns->graph->edges[i][0]], 
	ns->graph->edges[i][0], 
	chain_map[ns->graph->edges[i][1]],
	ns->graph->edges[i][1]);
    Print_Neurocomp_Conn(ns->conn + i);
  }
#endif
  /*
  printf("\ncross over changed: \n");
  Print_Neuron_Structure(ns);
  */

#ifdef _DEBUG_2
  ns->graph->nedge = 0;
  Neuron_Structure_To_Swc_File(ns, "../data/test.swc"); 
  return 1;
#endif
  
  //Print_Neuron_Structure(ns);

  
  Neuron_Structure* ns2= NULL;
  
  if (Is_Arg_Matched("-intp")) {
    ns2 = Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
  } else {
    ns2 = Neuron_Structure_Locseg_Chain_To_Circle(ns);
  }
    
  /*
  Neuron_Structure* ns2=
    Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
  */
  Graph_To_Dot_File(ns2->graph, full_path(dir, "graph_d.dot"));

  //Neuron_Structure_Main_Graph(ns2);
  Neuron_Structure_To_Tree(ns2);
  
  double root[3];

  if (Is_Arg_Matched("-root")) {
    root[0] = Get_Double_Arg("-root", 1);
    root[1] = Get_Double_Arg("-root", 2);
    root[2] = Get_Double_Arg("-root", 3);
  }

  Swc_Tree *tree = NULL;

  if (Is_Arg_Matched("-root")) {
    /*
    int root_index = Neuron_Structure_Find_Root_Circle(ns2, root);
    Graph_Workspace *gw2 = New_Graph_Workspace();
    Graph_Clean_Root(ns2->graph, root_index, gw2);

    Neuron_Structure_To_Swc_File_Circle_Z(ns2, full_path(dir, "graph_d.swc"),
					  z_scale, root);
    */
    tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, z_scale, root);
    if (Swc_Tree_Node_Is_Virtual(tree->root) == TRUE) {
      tree->root->first_child->next_sibling = NULL;
    }
    Swc_Tree_Clean_Root(tree);
  } else {
    /*
    Neuron_Structure_To_Swc_File_Circle_Z(ns2, full_path(dir, "graph_d.swc"),
					  z_scale, NULL);
    */
    tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, z_scale, NULL);
  }

  ns->graph->nedge = 0;
  //Neuron_Structure_To_Swc_File(ns, full_path(dir, "tube.swc"));


  if (Is_Arg_Matched("-rb")) {
    //Swc_Tree_Tune_Branch(tree);
    Swc_Tree_Tune_Fork(tree);
  }

  if (Is_Arg_Matched("-sl")) {
    Swc_Tree_Leaf_Shrink(tree);
  }

  if (Is_Arg_Matched("-rz")) {
    Swc_Tree_Remove_Zigzag(tree);
  }

  if (Is_Arg_Matched("-rs")) {
    Swc_Tree_Remove_Spur(tree);
  }
  
  Swc_Tree_Resort_Id(tree);

  Write_Swc_Tree(full_path(dir, "graph_d.swc"), tree);

  if (Is_Arg_Matched("-rtlist")) {
    int m, n;
    double *d = darray_load_matrix(Get_String_Arg("-rtlist"), NULL, &m, &n);

    if (n > 0) {
      coordinate_3d_t *roots = GUARDED_MALLOC_ARRAY(roots, n, coordinate_3d_t);
      int i;
      for (i = 0; i < n; i++) {
	roots[i][0] = d[i*3];
	roots[i][1] = d[i*3 + 1];
	roots[i][2] = d[i*3 + 2];

	/*
	if (tube_offset != NULL) {
	  roots[i][0] += tube_offset[0];
	  roots[i][1] += tube_offset[1];
	  roots[i][2] += tube_offset[2];
	}
	*/

	Swc_Tree *subtree = Swc_Tree_Pull_R(tree, roots[i]);
	char filename[MAX_PATH_LENGTH];
	if (subtree->root != NULL) {
	  //Swc_Tree_Clean_Root(subtree);
	  Swc_Tree_Clean_Root(subtree);
	  Swc_Tree_Node_Set_Pos(subtree->root, roots[i]);
	  if (Is_Arg_Matched("-jumpz")) {
	    //swc_tree_remove_zjump(subtree, Get_Double_Arg("-jumpz"));
	  }
	  Swc_Tree_Resort_Id(subtree);
	  sprintf(filename, "graph%d.swc", i + 1);
	  Write_Swc_Tree(full_path(dir, filename), subtree);
	}
      }
    }
  }

  printf("%d chains\n", chain_number);

  return 0;
}
