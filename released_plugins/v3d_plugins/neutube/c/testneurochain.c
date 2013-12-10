#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include "tz_error.h"
#if EXTLIB_EXIST(GSL)
#  include <gsl/gsl_vector.h>
#  include <gsl/gsl_statistics_double.h>
#endif
#include "tz_constant.h"
#include "tz_voxel_linked_list.h"
#include "tz_stack_sampling.h"
#include "tz_voxel_graphics.h"
#include "tz_neurotrace.h"
#include "tz_stack_math.h"
#include "tz_stack_utils.h"
#include "tz_objdetect.h"
#include "tz_voxeltrans.h"
#include "tz_stack_stat.h"
#include "tz_stack_draw.h"
#include "tz_vrml_io.h"
#include "tz_geo3d_vector.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_bwdist.h"
#include "tz_vrml_material.h"
#include "tz_string.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_stack_attribute.h"
#include "tz_locseg_chain.h"
#include "tz_trace_utils.h"
#include "tz_fimage_lib.h"
#include "tz_arrayview.h"
#include "tz_geoangle_utils.h"
#include "tz_workspace.h"
#include "tz_math.h"
#include "tz_geo3d_point_array.h"
#include "private/testneurochain.h"
#include "tz_geo3d_ball.h"
#include "tz_stack_threshold.h"

INIT_EXCEPTION_MAIN(e)

char **Find_Files_Full(char *dir_name, char *ext, int *fn)
{
  int source_num = dir_fnum(dir_name, ext);
  *fn = source_num;
  char **files = (char
      **)malloc(sizeof(char *)*source_num);

  DIR *dir = opendir(dir_name);

  struct dirent *ent = readdir(dir);

  char *source_file_path = (char
      *) malloc(sizeof(char)*(strlen(dir_name) + 100));

  strcpy(source_file_path, dir_name);
  strcat(source_file_path, "/");


  int fid =0;

  while (ent != NULL) {
    if (fextn(ent->d_name) !=
	NULL){
      if (strcmp(fextn(ent->d_name),
	    ext) == 0) {

	strcpy(source_file_path +
	    strlen(dir_name) + 1,
	    ent->d_name);

	files[fid] = (char
	    *)malloc(sizeof(char)*(strlen(dir_name)
		+ 100));

	strcpy(files[fid],
	    source_file_path);
	fid++;
      }

      ent = readdir(dir);
    }
    else{
      ent = readdir(dir);
    }
  }
  closedir(dir);

  return files;
}

double Locseg_Chain_Find_Segment(Locseg_Chain *chain, double *pos,
    Local_Neuroseg **hit_seg, int *seg_index){
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg* seg = NULL;
  double min_dist = 1000.0;
  int seg_ind = 0;

  while((seg=Locseg_Chain_Next_Seg(chain))!=NULL){
    double cen_pos[3];

    Local_Neuroseg_Center(seg, cen_pos);
    double dist =
      Coordinate_3d_Distance(pos, cen_pos);
    if(dist<min_dist){
      min_dist = dist;
      *seg_index = seg_ind;
      *hit_seg =
	seg;
    }
    seg_ind++;
  }

  return min_dist;
}

void Compute_Orientation(Object_3d *ps_obj, int pn, coordinate_3d_t vec){

  coordinate_3d_t lvec, temp_sum, sum_vec;
  sum_vec[0] = 0.0;

  sum_vec[1] = 0.0;
  sum_vec[2] = 0.0;
  Coordinate_3d_Copy(temp_sum,
      sum_vec);
  for(int i=1; i<pn; i++){
    lvec[0] =
      ps_obj->voxels[i][0] - ps_obj->voxels[0][0];
    lvec[1] =
      ps_obj->voxels[i][1] -
      ps_obj->voxels[0][1];
    lvec[2] =
      ps_obj->voxels[i][2] -
      ps_obj->voxels[0][2];
    double lvec_len =
      Coordinate_3d_Norm(lvec);
    if(lvec_len>2){

      Coordinate_3d_Unitize(lvec);

      Coordinate_3d_Add(lvec,temp_sum,sum_vec);

      Coordinate_3d_Copy(temp_sum,
	  sum_vec);
    }
  }

  Coordinate_3d_Scale(sum_vec, 1.0/pn);
  Coordinate_3d_Copy(vec,
      sum_vec);

}

BOOL Locseg_Chain_Orientation(Locseg_Chain *chain, Local_Neuroseg
    *terminal_seg, double orien_seg_len, double *orientation){
  double len = 0.0;
  Local_Neuroseg *lseg;
  BOOL is_head = FALSE;

  Object_3d *ps_obj = Make_Object_3d(40, 0);
  int pn = 0;
  double head_pos[3], tail_pos[3];

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *head_seg =
    Locseg_Chain_Peek_Seg(chain);
  lseg = terminal_seg;

  if(lseg != head_seg){
    is_head = FALSE;

    Locseg_Chain_Iterator_Start(chain, DL_TAIL);    


    while(len<orien_seg_len &&
	lseg!=NULL){
      len +=
	lseg->seg.h;

      Local_Neuroseg_Top(lseg,
	  head_pos);
      Local_Neuroseg_Bottom(lseg,
	  tail_pos);
      ps_obj->voxels[pn][0]
	=
	head_pos[0];

      ps_obj->voxels[pn][1]
	=
	head_pos[1];
      ps_obj->voxels[pn][2]
	=
	head_pos[2];
      pn++;

      ps_obj->voxels[pn][0]
	=
	tail_pos[0];
      ps_obj->voxels[pn][1]
	=
	tail_pos[1];

      ps_obj->voxels[pn][2]
	=
	tail_pos[2];
      pn++;
      lseg
	=
	Locseg_Chain_Prev_Seg(chain);
    }
  }else{

    is_head = TRUE;

    Locseg_Chain_Iterator_Start(chain,
	DL_HEAD);


    while(len<orien_seg_len
	&&
	lseg!=NULL){
      len
	+=
	lseg->seg.h;

      Local_Neuroseg_Top(lseg,
	  head_pos);
      Local_Neuroseg_Bottom(lseg,
	  tail_pos);
      ps_obj->voxels[pn][0]
	=
	tail_pos[0];

      ps_obj->voxels[pn][1]
	=
	tail_pos[1];
      ps_obj->voxels[pn][2]
	=
	tail_pos[2];
      pn++;

      ps_obj->voxels[pn][0]
	=
	head_pos[0];
      ps_obj->voxels[pn][1]
	=
	head_pos[1];

      ps_obj->voxels[pn][2]
	=
	head_pos[2];
      pn++;
      lseg
	=
	Locseg_Chain_Next_Seg(chain);
    }
  }

  ps_obj->size = pn;

  Compute_Orientation(ps_obj, pn, orientation);
  Kill_Object_3d(ps_obj);

  return is_head;
}

double Locseg_Chains_Dist(Locseg_Chain *chain, Locseg_Chain *c_chain, BOOL
    *is_head, int *seg_index, coordinate_3d_t hit_orien,
    Local_Neuroseg **hit_seg)
{
  double dist_head, dist_tail,
  min_dist=10000.0;
  double pos_head[3], pos_tail[3];
  int
    head_hit_index, tail_hit_index;
  Local_Neuroseg* head_seg, *tail_seg;

  Locseg_Chain_Iterator_Start(c_chain, DL_HEAD);
  head_seg =
    Locseg_Chain_Peek_Seg(c_chain);
  Local_Neuroseg_Bottom(head_seg,
      pos_head);
  Locseg_Chain_Iterator_Start(c_chain, DL_TAIL);
  tail_seg =
    Locseg_Chain_Peek_Seg(c_chain);
  Local_Neuroseg_Top(tail_seg, pos_tail);

  double head_skel_pos[3], tail_skel_pos[3];
  dist_head =
    Locseg_Chain_Point_Dist(chain,
	pos_head, &head_hit_index,
	head_skel_pos);

  dist_tail =
    Locseg_Chain_Point_Dist(chain,
	pos_tail, &tail_hit_index,
	tail_skel_pos);
  if(dist_head < dist_tail){
    min_dist = dist_head;
    /*
    double dist = Locseg_Chain_Find_Segment(chain, head_skel_pos, hit_seg,
	  seg_index);
	  */
    hit_orien[0] = head_skel_pos[0] - pos_head[0];
    hit_orien[1] = head_skel_pos[1] - pos_head[1];
    hit_orien[2] = head_skel_pos[2] - pos_head[2];
    *is_head = TRUE;
  }else{
    min_dist = dist_tail;
    /*
    double dist = Locseg_Chain_Find_Segment(chain, tail_skel_pos, hit_seg,
	  seg_index);
*/
    hit_orien[0] = tail_skel_pos[0] - pos_tail[0];
    hit_orien[1] = tail_skel_pos[1] - pos_tail[1];
    hit_orien[2] = tail_skel_pos[2] - pos_tail[2];
    *is_head = FALSE;
    Locseg_Chain_Orientation(c_chain, tail_seg, 15, hit_orien);
  }
  return min_dist;
}

double Find_Overlapping(Swc_Tree *tree, Locseg_Chain *chain)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Local_Neuroseg *lseg =
    Locseg_Chain_Next_Seg(chain);

  double len = 0;
  while(lseg != NULL){
    double pos[3];
    Local_Neuroseg_Center(lseg,  pos);
    double
      dist = Swc_Tree_Point_Dist(tree, pos[0], pos[1], pos[2], NULL, NULL);

    if(dist < 2*lseg->seg.r1)
      len += lseg->seg.h;
    lseg =
      Locseg_Chain_Next_Seg(chain);
  }
  return len;
}

int is_chains_connected(Locseg_Chain *chain1, Locseg_Chain *chain2, char
    *gt_folder, double dist_thre)
{
  char *ext = "swc";

  int seg_index;

  Local_Neuroseg *hit_seg;
  double *hit_orien = (double*)malloc(sizeof(double)*3);
  BOOL is_head;
  double chain_dist =
    Locseg_Chains_Dist(chain1, chain2, &is_head, &seg_index, hit_orien,
	&hit_seg);
  free(hit_orien);

  double len1 = Locseg_Chain_Geolen(chain1);
  double len2 = Locseg_Chain_Geolen(chain2);


  if(chain_dist<dist_thre){

    char *swc_fold_path = (char *) malloc(sizeof(char)*(strlen(gt_folder)
	    + 100));

    strcpy(swc_fold_path, gt_folder);

    int swc_file_n;
    char **swc_files = Find_Files_Full(gt_folder, ext, &swc_file_n);

    double max_over1 = 0.0;
    double max_over2 = 0.0;
    int max_over_swc_id1 = -1;
    int max_over_swc_id2 = -1;

    for(int i=0; i<swc_file_n; i++){
      char *swc_name = swc_files[i];

      Swc_Tree *tree = Read_Swc_Tree(swc_name);
      double overlen1 = Find_Overlapping(tree, chain1);
      double overlen2 = Find_Overlapping(tree, chain2);
      if(max_over1<overlen1 && overlen1 > len1/2.0){
	max_over1 = overlen1;
	max_over_swc_id1 = i;
      }

      if(max_over2 < overlen2 && overlen2 > len2/2.0){
	max_over2 = overlen2;

	max_over_swc_id2 = i;
      }
    }

    if(max_over_swc_id2>-1 && max_over_swc_id1>-1){
      if(max_over_swc_id2==max_over_swc_id1)

	return 1;
      // should
      // be
      // connected.
      else
	return 0;
      // not
      // connected.
    }else{
      return -1;
      // some
      // tubes
      // are not on the ground_truth.
    }
  }else{
    return -2;
    // too // far away.
  }
}

int main(int argc, char* argv[])
{
  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-t")) {
    /* New a locseg chain */
    Locseg_Chain *chain = New_Locseg_Chain();

    /* The new chain should be empty */
    if (Locseg_Chain_Is_Empty(chain) == FALSE) {
      PRINT_EXCEPTION("Bug?", "Non-empty chain.");
      return 1;
    }

    /* Add a node to the chain */
    Local_Neuroseg *locseg = New_Local_Neuroseg();
    Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);

    if (Locseg_Chain_Is_Empty(chain) == TRUE) {
      PRINT_EXCEPTION("Bug?", "Empty chain.");
      return 1;
    }

    /* Now the length of the chain is 1 */
    if (Locseg_Chain_Length(chain) != 1) {
      PRINT_EXCEPTION("Bug?", "Unexpected length.");
      return 1;
    }

    /* We can also calculate the geometric length of the chain */
    if (Compare_Float(Locseg_Chain_Geolen(chain), locseg->seg.h - 1.0, 0.01)
	!= 0) {
      PRINT_EXCEPTION("Bug?", "Unexpected length.");
      return 1;      
    }

    /* We can add a segment, with tracing record */
    Trace_Record *tr = New_Trace_Record();
    Trace_Record_Set_Direction(tr, DL_FORWARD);
    Locseg_Chain_Add(chain, Next_Local_Neuroseg(locseg, NULL, 0.5), tr,
	DL_TAIL);

    /* Write the chain to the disk */
    Write_Locseg_Chain("../data/test/neurochain/test.tb", chain);

    /* Read the chain */
    Locseg_Chain *chain2 = Read_Locseg_Chain("../data/test/neurochain/test.tb");

    if (Locseg_Chain_Is_Identical(chain, chain2) == FALSE) {
      PRINT_EXCEPTION("Bug?", "Unequal chains.");
      return 1; 
    }

    /* Flip the chain */
    Locseg_Chain_Flip(chain2);

    /* A flipped chain has a different orientation */
    if (test_locseg_chain_flip(chain, chain2) != 0) {
      PRINT_EXCEPTION("Bug?", "Flipping error.");
      return 1;
    }

    /* We can merge two chains */
    locseg = Locseg_Chain_Tail_Seg(chain);

    Local_Neuroseg *locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.5);
    chain2 = New_Locseg_Chain();
    Locseg_Chain_Add(chain2, locseg2, NULL, DL_TAIL);

    int n = Locseg_Chain_Length(chain);

    Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

    Kill_Locseg_Chain(chain2);
    if (Locseg_Chain_Length(chain) != n + 1) {
      PRINT_EXCEPTION("Bug?", "Unexpected length.");
      return 1;
    }

    /* Break a chain */
    chain2 = Locseg_Chain_Break_At(chain, 1);

    if ((Locseg_Chain_Length(chain) != 2) || 
	(Locseg_Chain_Length(chain2) != n-1)) {
      PRINT_EXCEPTION("Bug?", "Unexpected length.");
      return 1;
    }


    {           /* unit test */
      if (unit_test() != 0) {
	PRINT_EXCEPTION("Bug?", "Unit test failed.");
	return 1;
      }
    }

    printf(":) Testing passed.\n");
    return 0;
  }

#if 0
  Stack *stack = Read_Stack("../data/testneurotrace.tif");

  Set_Zscale(0.8);

  Neurochain *chain = New_Neurochain();
  Set_Neuroseg(&(chain->seg), 5.0, 6.0, 12.0, -TZ_PI / 3, 0.9);
  Set_Position(chain->position, 143, 99, 107*0.8);

  //Set_Position(chain->position, 127, 68, 34*0.8);
  //Set_Position(chain->position, 60, 136, 91*0.8);
  //Set_Position(chain->position, 176, 221, 184*0.8);

#if 0
  Fit_Neuroseg(stack, &(chain->seg), chain->position, TRUE); 
  Trace_Neuron(stack, chain, FORWARD, NULL);

  Stack *canvas = Translate_Stack(stack, COLOR, 0);
  Neurochain_Label(canvas, Neurochain_Head(chain));

  //Print_Neurochain(Neurochain_Head(chain));

  Stack *traced = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  One_Stack(traced);
  Neurochain_Erase(traced, Neurochain_Head(chain));

  Free_Neurochain(chain);

  chain = New_Neurochain();
  Set_Neuroseg(&(chain->seg), 5.0, 6.0, 12.0, -TZ_PI / 3, 0.9);
  Set_Position(chain->position, 143, 99, 107*0.8);
  Fit_Neuroseg(stack, &(chain->seg), chain->position, TRUE); 
  Trace_Neuron(stack, chain, FORWARD, traced);

  Print_Neurochain(Neurochain_Head(chain));

  Write_Stack("../data/test2.tif", canvas);
  Write_Stack("../data/test3.tif", traced);
#endif

  /*
     Extend_Neurochain(chain);
     Extend_Neurochain(chain);

     Print_Neurochain(chain);

     chain = Neurochain_Tail(chain);

     Neurochain *removed = Shrink_Neurochain(&chain, NEUROCHAIN_BOTH_END);

     Print_Neurochain(removed);
     Print_Neurochain(chain);
   */

  Kill_Stack(stack);
#endif

#if 0 /* test io */
  FILE *fp = fopen("chain.bn", "w+");
  Neurochain *chain = New_Neurochain();
  Set_Neuroseg(&(chain->locseg.seg), 2.21442, 2.09664, 12, 1.55113, 1.99907);
  Set_Neuropos(chain->locseg.pos, 306.794, 50.0325, 134.385);

  Neurochain_Fwrite(chain, fp);
  Print_Neurochain(chain);
  fclose(fp);

  printf("Written.\n");

  fp = fopen("chain.bn", "r");
  Neurochain *chain2 = New_Neurochain();
  Neurochain_Fread(chain2, fp);
  fclose(fp);

  Print_Neurochain(chain2);
#endif

#if 0 /* test reading */
  FILE *fp  = fopen ("../data/fly_neuron_chain.bn", "r");
  Neurochain *chain = New_Neurochain();
  Neurochain_Fread(chain, fp);

  fclose(fp);

  int i;
  int length = Neurochain_Length(chain, FORWARD);
  Neurochain *chain2 = chain;
  for (i = 0; i < length; i++) {
    chain->locseg.seg.r1 = i + 1;
    chain = chain->next;
  }

  chain = Neurochain_Cut_At(chain2, 10);


  //Print_Neurochain(chain2);

  chain2->prev = NULL;
  chain2 = Neurochain_Flip_Link(chain2, FORWARD);

  Neurochain_Remove_Last(chain2, 1);
  chain = Neurochain_Tail(Append_Neurochain(chain2, chain));

  Print_Neurochain(Neurochain_Head(chain));

  Write_Neurochain_Vrml("../data/neurochain.wrl", chain);

  Free_Neurochain(chain);
  //Free_Neurochain(chain2);
  /*
     fp = fopen("../data/neurochain.wrl", "w");
     fprintf(fp, "#VRML V2.0 utf8\n");

     fprintf(fp, "Transform {\n");
  //fprintf(fp, "scaleFactor 0.1 0.1 0.1\n");
  //fprintf(fp, "translation %.4f, %.4f, %.4f\n", -chain->locseg.pos[0], 
  //	  -chain->locseg.pos[1], -chain->locseg.pos[2]);
  fprintf(fp, "children [\n");
  while(chain != NULL) {
  Local_Neuroseg_Fprint_Vrml(&(chain->locseg), fp);
  chain = chain->next;
  }
  fprintf(fp, "]\n");
  fprintf(fp, "}\n");

  fclose(fp);
   */
#endif

#if 0 /* test reading neuron chains from a directory*/
  int chain_number;
  Neurochain **chain_array = Dir_Neurochain("../data/mouse_neuron", 
      "bn", &chain_number);
  printf("%d\n", chain_number);

  FILE *fp = fopen("../data/mouse_neuron.wrl", "w");

  Vrml_Head_Fprint(fp, "V2.0", "utf8");

  int i;
  for (i = 0; i < chain_number; i++) {
    if (Neurochain_Length(chain_array[i], FORWARD) > 5) {
      Vrml_Neurochain_Fprint(fp, chain_array[i], 0);
      //Neurochain_Fprint_Vrml(chain_array[i], fp);
    }

    Free_Neurochain(chain_array[i]);
  }

  fclose(fp);

  free(chain_array);
#endif

#if 0
  Neurochain *chain = Read_Neurochain("/Volumes/myerslab/zhaot/result/chain13.bn");
  Print_Neurochain(chain);

  FILE *fp = fopen("../data/test.ar", "w");
  int length = Neurochain_Length(chain, FORWARD);
  int dlength = length * 3;
  fwrite(&dlength, sizeof(int), 1, fp);
  int i;
  neuropos_t cpos;
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  for (i = 0; i < length; i++) {
    //Neurochain_Get_Locseg(chain, locseg);
    Copy_Local_Neuroseg(locseg, &(chain->locseg));
    Local_Neuroseg_Center(locseg, cpos);
    fwrite(cpos, sizeof(neuropos_t), 1, fp);
    if (chain->next != NULL) {
      chain = chain->next;
    }
  }
  fclose(fp);
  Free_Local_Neuroseg(locseg);
  Free_Neurochain(Neurochain_Head(chain));
#endif

#if 0 /* test neuronchain hit test*/
  Neurochain *chain = Read_Neurochain("../data/mouse_neuron/chain48.bn");
  Print_Neurochain(chain);
  printf("%d\n", Neurochain_Length(chain, FORWARD));
  printf("%d\n", Neurochain_Length(chain, BOTH));

  Print_Local_Neuroseg(&(chain->locseg));

  printf("%d\n", Neurochain_Hit_Test_Seg(chain, FORWARD, 0, 
	&(chain->next->next->locseg)));

  FILE *fp = fopen("../data/test.wrl", "w");

  Vrml_Head_Fprint(fp, "V2.0", "utf8");

  Neurochain_Fprint_Vrml(chain->next, fp);

  fclose(fp);

  Write_Neurochain_Vrml("../data/test2.wrl", chain->next);


#endif

#if 0
  Neurochain *chain = Read_Neurochain("../data/fly_neuron/chain10.bn");
  Print_Neurochain(chain);

  Neurochain_Normalize_Direction(chain, BOTH);
  Print_Neurochain(chain);
  /*
     while (chain->next != NULL) {
     printf("%g ", Neuroseg_Angle_Between(&(chain->locseg.seg), 
     &(chain->next->locseg.seg)));
     chain = chain->next;
     }

     printf("\n");
   */
#endif

#if 0
  int chain_number2;
  /* load chains from the directory */
  Neurochain **chain_array2 = Dir_Neurochain_P("../data/mouse_neuron3_org", 
      "chain.*\\.bn",
      &chain_number2);
  Neurochain **chain_array =
    (Neurochain **) Guarded_Malloc(sizeof(Neurochain *) * chain_number2, 
	"testneurochain");
  int chain_number = 0;
  int i;

  for (i = 0; i < chain_number2; i++) {
    if (Neurochain_Length(chain_array2[i], BOTH) > 5) {
      chain_array[chain_number++] = chain_array2[i];
    } else {
      Free_Neurochain(chain_array2[i]);
    }
  }

  Int_Arraylist *hit_spots = Int_Arraylist_New(0, chain_number);
  Graph *graph = Neurochain_Graph(chain_array, chain_number, hit_spots);

  Graph_Workspace *gw = New_Graph_Workspace();

  Unipointer_List *graph_list = NULL;


  Graph *subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[0][0]);

  Unipointer_List_Add(&graph_list, subgraph);

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);


  for (i = 0; i < graph->nedge; i++) {
    if (gw->elist[i] == 0) {
      subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
      Print_Graph(subgraph);
      Unipointer_List_Add(&graph_list, subgraph);
    }
  }

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, FALSE);

  printf("%d\n", Unipointer_List_Length(graph_list));  



  Graph_Update_Edge_Table(graph, gw);

  Stack *stack = Read_Stack("../data/mouse_neuron3_org.tif"); 
  Stretch_Stack_Value_Q(stack, 0.999);
  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  for (i = 0; i < gw->nvertex; i++) {
    gw->vlist[i] = 0;
  }

  while (graph_list != NULL) {
    subgraph = (Graph *) graph_list->data;
    int v;
    for (i = 0; i < subgraph->nedge; i++) {
      v = subgraph->edges[i][0];
      if (gw->vlist[v] == 0) {
	printf("chain %d labeled\n", v);
	Neurochain_Label(canvas, chain_array[v], 1.0);
	gw->vlist[v] = 1;
      }

      v = subgraph->edges[i][1];
      if (gw->vlist[v] == 0) {
	printf("chain %d labeled\n", v);
	Neurochain_Label(canvas, chain_array[v], 1.0);
	gw->vlist[v] = 1;
      }

      int e = Graph_Edge_Index(subgraph->edges[i][0], subgraph->edges[i][1], gw);

      if ((hit_spots->array[e] > 1) || 
	  (hit_spots->array[e] < Neurochain_Length(chain_array[v], FORWARD))) {
	printf("Hit: %d of %d\n", hit_spots->array[e], 
	    Neurochain_Length(chain_array[v], FORWARD));
	Neurochain *chain = Locate_Neurochain(chain_array[v], 
	    hit_spots->array[e] - 1);
	Local_Neuroseg_Label(&(chain->locseg), canvas, 2, 1.0);
      }

    }
    graph_list = graph_list->next;
  }

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  int nregion = 0;
  FILE *region_fp = fopen("../data/fly_neuron2/region_id.txt", "r");
  int tmpid, tmptype;
  fscanf(region_fp, "%d %d", &tmpid, &tmptype);
  while (tmpid >= 0) {
    nregion = imax2(nregion, tmpid);
    fscanf(region_fp, "%d %d", &tmpid, &tmptype);
  }
  printf("%d\n", nregion);
  fclose(region_fp);


  int *region_map = iarray_malloc(nregion);

  int i;
  for (i = 0; i < nregion; i++) {
    region_map[i] = -1;
  }

  region_fp = fopen("../data/fly_neuron2/region_id.txt", "r");

  fscanf(region_fp, "%d %d", &tmpid, &tmptype);
  while (tmpid >= 0) {
    region_map[tmpid] = tmptype;
    fscanf(region_fp, "%d %d", &tmpid, &tmptype);
  }
  fclose(region_fp);

  iarray_print2(region_map, nregion, 1);

  int chain_number2;
  /* load chains from the directory */
  Neurochain **chain_array2 = Dir_Neurochain_P("../data/fly_neuron2", 
      "chain.*\\.bn",
      &chain_number2);
  Stack *stack = Read_Stack("../data/fly_neuron2.tif");


  Neurochain **chain_array =
    (Neurochain **) Guarded_Malloc(sizeof(Neurochain *) * chain_number2, 
	"testneurochain");
  int chain_number = 0;

  Trace_Workspace tw;
  tw.length = 20;
  tw.fit_first = FALSE;
  tw.tscore_option = 1;
  tw.min_score = 0.15;
  tw.trace_direction = DL_BOTHDIR;
  tw.trace_mask = Read_Stack("../data/fly_neuron2/blobmask.tif");
  if (tw.trace_mask != NULL) {
    Stack_Not(tw.trace_mask, tw.trace_mask);
    for (i = 0; i < chain_number2; i++) {
      if (Neurochain_Length(chain_array2[i], BOTH) > 4) {
	Neurochain_Erase_E(tw.trace_mask, chain_array2[i]->next, 1.0, 0,
	    Neurochain_Length(chain_array2[i], FORWARD) - 2,
	    1.0, 0.0);
      }
    }


    Stack_Not(tw.trace_mask, tw.trace_mask);
  }
  tw.dyvar[0] = 5.0;
  tw.test_func = Locnp_Chain_Trace_Test;

  for (i = 0; i < 6; i++) {
    tw.trace_range[i] = -1.0;
  }

  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  for (i = 0; i < chain_number2; i++) {
    if (Neurochain_Length(chain_array2[i], BOTH) > 4) {
      Locseg_Chain *tmpchain = Neurochain_To_Locseg_Chain(chain_array2[i]);
      Locseg_Chain_Trace_Np(stack, 1.0, tmpchain, &tw);

      chain_array[chain_number++] = Neurochain_From_Locseg_Chain(tmpchain);

      Kill_Locseg_Chain(tmpchain);
    }
  }


  Int_Arraylist *hit_spots = Int_Arraylist_New(0, chain_number);
  Graph *graph = Neurochain_Graph(chain_array, chain_number, hit_spots);

  /* Add region label */
  Stack *region = Read_Stack("../data/fly_neuron2/region_label.tif");
  graph->nvertex += 20;
  int hit_region[2];
  printf("chain number: %d\n", chain_number);
  for (i = 0; i < chain_number; i++) {
    Neurochain_Hit_Stack(chain_array[i], region, hit_region);
    if (hit_region[0] > 1) {
      Graph_Add_Edge(graph, i, hit_region[0] + chain_number);
      if ((hit_region[0] + chain_number) >= graph->nvertex) {
	graph->nvertex = hit_region[0] + chain_number + 1;
      }
    }
    if (hit_region[1] > 1) {
      Graph_Add_Edge(graph, i, hit_region[1] + chain_number);
      if ((hit_region[1] + chain_number) >= graph->nvertex) {
	graph->nvertex = hit_region[1] + chain_number + 1;
      }
    }
  }

  Print_Graph(graph);

  Graph_Workspace *gw = New_Graph_Workspace();

  Unipointer_List *graph_list = NULL;


  Graph *subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[0][0]);

  Print_Graph(subgraph);

  Unipointer_List_Add(&graph_list, subgraph);

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, TRUE);


  for (i = 0; i < graph->nedge; i++) {
    if (gw->elist[i] == 0) {
      subgraph = Graph_Connected_Subgraph(graph, gw, graph->edges[i][0]);
      Print_Graph(subgraph);
      Unipointer_List_Add(&graph_list, subgraph);
    }
  }

  Graph_Workspace_Set_Readiness(gw, GRAPH_WORKSPACE_ELIST, FALSE);

  printf("%d\n", Unipointer_List_Length(graph_list));  

  double hue_step = 1.3; //6.0 / (Unipointer_List_Length(graph_list) + 1);

  for (i = 0; i < gw->nvertex; i++) {
    gw->vlist[i] = 0;
  }

  FILE *fp = fopen("../data/fly_neuron2/graph.wrl", "w");

  Vrml_Head_Fprint(fp, "V2.0", "utf8");

  double hue = 0.0;

  int *connected_blob_mask = iarray_malloc(nregion);
  for (i = 0; i < nregion; i++) {
    connected_blob_mask[i] = 0;
  }

  while (graph_list != NULL) {
    subgraph = (Graph *) graph_list->data;
    graph_list = graph_list->next;

    Print_Graph(subgraph);


    BOOL isolated  = TRUE;
    int region_id;
    int j;
    for (i = 0; i < subgraph->nedge; i++) {
      for (j = 0; j < 2; j++) {
	region_id = subgraph->edges[i][j] - chain_number;
	if (region_id > 0) {
	  if (region_map[region_id] == 3) {
	    isolated = FALSE;
	    break;
	  }
	}
      }
      if (isolated == FALSE) {
	break;
      }
    }

    if (isolated == TRUE) {
      continue;
    }

    for (i = 0; i < subgraph->nedge; i++) {
      for (j = 0; j < 2; j++) {
	region_id = subgraph->edges[i][j] - chain_number;
	if (region_id > 0) {
	  if (region_map[region_id] >= 2) {
	    connected_blob_mask[region_id] = 1;
	  }
	}
      }
    }

    Graph_Update_Edge_Table(graph, gw);

    Rgb_Color color;
    Set_Color_Hsv(&color, hue, 1.0, 1.0);
    hue += hue_step;

    if (hue > 6.0) {
      hue -= 6.0;
    }

    Vrml_Material *material = New_Vrml_Material();
    Bitmask_Set_Bit(DIFFUSE_COLOR, FALSE, &(material->default_mask));
    Vrml_SFVec3f_Set(material->diffuse_color, (double)color.r / 255.0,
	(double)color.g / 255.0, (double)color.b / 255.0);

    Vrml_Sphere sphere;
    Bitmask_Set_Bit(0, FALSE, &(sphere.default_mask));
    sphere.radius = 3;

    int v;
    for (i = 0; i < subgraph->nedge; i++) {
      v = subgraph->edges[i][0];
      if (gw->vlist[v] == 0) {
	printf("chain %d labeled\n", v);
	Vrml_Neurochain_Fprint_C(fp, chain_array[v], material, 0);
	Neurochain_Label(canvas, chain_array[v], 1.0);
	gw->vlist[v] = 1;
      }

      if (subgraph->edges[i][1] >= chain_number) {
	Stack_Label_Color_L(canvas, region, subgraph->edges[i][1] - chain_number,
	    1.0, 1.0, stack);
      } else {
	v = subgraph->edges[i][1];
	if (gw->vlist[v] == 0) {
	  printf("chain %d labeled\n", v);
	  Neurochain_Label(canvas, chain_array[v], 1.0);
	  Vrml_Neurochain_Fprint_C(fp, chain_array[v], material, 0);

	  gw->vlist[v] = 1;
	}

	int e = Graph_Edge_Index(subgraph->edges[i][0], subgraph->edges[i][1], gw);

	if ((hit_spots->array[e] > 1) || 
	    (hit_spots->array[e] < Neurochain_Length(chain_array[v], FORWARD))) {
	  printf("Hit: %d of %d\n", hit_spots->array[e], 
	      Neurochain_Length(chain_array[v], FORWARD));
	  if (hit_spots->array[e] < 
	      Neurochain_Length(chain_array[v], FORWARD) - 1) {
	    Neurochain *chain = Locate_Neurochain(chain_array[v], 
		hit_spots->array[e] - 1);
	    Local_Neuroseg_Label(&(chain->locseg), canvas, 2, 1.0);
	    double center[3];
	    Local_Neuroseg_Center(&(chain->locseg), center);
	    Vrml_Node_Begin_Fprint(fp, "Transform", 0);
	    fprintf(fp, "translation %g %g %g\n",	center[0], center[1], center[2]);
	    fprintf(fp, "children\n");
	    sphere.radius = (chain->locseg.seg.r1 + chain->locseg.seg.r2) / 2 +2.0;
	    Vrml_Node_Begin_Fprint(fp, "Shape", 0);
	    fprint_space(fp, 2);
	    fprintf(fp, "geometry\n");
	    Vrml_Sphere_Fprint(fp, &sphere, 2);
	    Vrml_Node_End_Fprint(fp, "Shape", 0);
	    Vrml_Node_End_Fprint(fp, "Transform", 0);
	  }
	}
      }
    }
  }
  fclose(fp);
  Write_Stack("../data/test.tif", canvas);

  Kill_Stack(canvas);
  Kill_Stack(stack);

  Stack *soma = Read_Stack("../data/fly_neuron2/soma.tif");
  int n = Stack_Voxel_Number(soma);
  for (i = 0; i < n; i++) {
    if ((soma->array[i] == 3) && 
	(connected_blob_mask[region->array[i]] == 1)) {
      soma->array[i] = 1;
    } else {
      soma->array[i] = 0;
    }
  }

  Write_Stack("../data/fly_neuron2/good_soma.tif", soma);

  Stack *arbor = Read_Stack("../data/fly_neuron2/soma.tif");

  for (i = 0; i < n; i++) {
    if ((arbor->array[i] == 2) && 
	(connected_blob_mask[region->array[i]] == 1)) {
      arbor->array[i] = 1;
    } else {
      arbor->array[i] = 0;
    }
  }

  Write_Stack("../data/fly_neuron2/good_arbor.tif", arbor);

  //Print_Graph(graph);
#endif

#if 0
  Locne_Chain *chain = New_Locne_Chain();
  int length = 2;
  int i;
  Local_Neuroseg_Ellipse *locne;
  for (i = 0; i < length; i++) {
    locne = New_Local_Neuroseg_Ellipse();
    chain->nplist = Local_Neuroseg_Ellipse_Dlist_Add(chain->nplist, locne, 
	DL_TAIL);
  }


  Print_Locne_Chain(chain);

  /*
     free(chain->nplist->next->data);
     free(chain->nplist->next);
     free(chain->nplist->data);
     free(chain->nplist);
   */
  //Read_Locne_Chain("../data/mouse_neuron/soma0.bn");
  Kill_Locne_Chain(chain);
#endif

#if 0
  Neurochain *chain = Read_Neurochain("../data/fly_neuron2/chain1101.bn");
  //chain = Neurochain_Remove_Overlap_Segs(chain);
  //Neurochain_Normalize_Direction(chain, FORWARD);
  Print_Neurochain(chain);


  Stack *signal = Read_Stack("../data/fly_neuron2.tif");
  double *value = Neurochain_Average_Signal(chain, signal, 1.0, NULL);
  darray_print2(value, Neurochain_Length(chain, FORWARD), 1);
  darray_write("../data/test1.ar", value, Neurochain_Length(chain, FORWARD));

  printf("%g\n", Value_Trend(value, Neurochain_Length(chain, FORWARD)));


  //printf("%d, %g\n", Neurochain_Length(chain, FORWARD), Neurochain_Geolen(chain));
#endif


#if 0
  Stack *signal = Read_Stack("../data/fly_neuron.tif");
  Stack *mask = Read_Stack("../data/fly_neuron/threshold.tif");
  double *value = darray_malloc(signal->depth);
  int i, j, offset;
  offset = 0;
  for(i = 0; i < signal->depth; i++) {
    value[i] = 0.0;
    int length = 0;
    for (j = 0; j < signal->width * signal->height; j++) {
      if (mask->array[offset] == 1) {
	value[i] += signal->array[offset];
	length++;
      }

      offset++;
    }
    value[i] /= length;
  }

  darray_write("../data/test2.ar", value, signal->depth);
#endif

#if 0
  Locseg_Chain *chain = New_Locseg_Chain();
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Locseg_Chain_Add_Node(chain, Make_Locseg_Node(locseg, NULL), 
      DL_TAIL);
  Trace_Record *tr = New_Trace_Record();
  tr->mask = 4;
  Locseg_Chain_Add_Node(chain, 
      Make_Locseg_Node(Next_Local_Neuroseg(locseg, NULL, 0.5), 
	tr), 
      DL_TAIL);
  Print_Locseg_Chain(chain);

  Write_Locseg_Chain("../data/test.tb", chain);

  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/test.tb");
  Print_Locseg_Chain(chain2);
#endif

#if 0
  FILE *fp = fopen("../data/test.bn", "w");
  int value = 1;
  fwrite(&value, sizeof(int), 1, fp);
  fclose(fp);

  fp = fopen("../data/test.bn", "r");
  //fread(&value, sizeof(int), 1, fp);
  fseek(fp, 4, SEEK_END);
  printf("%lu\n", ftell(fp));
  fread(&value, sizeof(int), 1, fp);
  if (feof(fp)) {
    printf("end of file\n");
  }
  fclose(fp);

#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("/Users/zhaot/Work/V3D/neurolabi/data/fly_neuron2/chain1179.bn");
  Locseg_Chain *loop = Read_Locseg_Chain("/Users/zhaot/Work/V3D/neurolabi/data/fly_neuron2/chain989.bn");

  neurocomp_conn_t conn;
  conn[0] = NEUROCOMP_CONN_HL;
  conn[1] = 0;
  conn[2] = 31;

  Print_Neurocomp_Conn(conn);
  Locseg_Chain_Reach_Conn(hook, loop, conn);

  FILE *fp = fopen("../data/test.wrl", "w");
  Vrml_Head_Fprint(fp, "V2.0", "utf8");
  Vrml_Material *material = New_Vrml_Material();
  Bitmask_Set_Bit(DIFFUSE_COLOR, FALSE, &(material->default_mask));
  Vrml_SFVec3f_Set(material->diffuse_color, 255.0 / 255.0,
      0.0, 0.0);
  Locseg_Chain_Vrml_Fprint(fp, hook, material, 0);
  Locseg_Chain_Vrml_Fprint(fp, loop, NULL, 0);

  fclose(fp);
#endif

#if 0
  Locseg_Chain *loop = Read_Locseg_Chain("/Users/zhaot/Work/V3D/neurolabi/data/fly_neuron/chain88.bn");
  Print_Locseg_Chain(loop);
  FILE *fp = fopen("../data/test.swc", "w");
  Locseg_Chain_Swc_Fprint(fp, loop);
  fclose(fp);
#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("../data/fly_neuron_b1/chain9.bn");
  Locseg_Chain *loop = Read_Locseg_Chain("../data/fly_neuron_b1/chain15.bn");
  Stack *stack = Read_Stack("../data/fly_neuron_b1.tif");
  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  conn.info[0] = 1;
  conn.info[1] = 1;
  Locseg_Chain_Connection_Test(hook, loop, stack, 1.0, &conn, NULL);

  Locseg_Chain_Reach_Conn(hook, loop, &conn);

  Locseg_Chain_To_Vrml_File(hook, "../data/test.wrl");
  Locseg_Chain_To_Vrml_File(loop, "../data/test2.wrl");

  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/mouse_neuron3_org/chain1.bn");
  Locseg_Chain_Vrml_Fprint(stdout, chain, NULL, 0);
  Print_Locseg_Chain(chain);

#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/mouse_neuron3_org/chain12.bn");

  int length;
  Local_Neuroseg_Ellipse *locne = Locseg_Chain_To_Ellipse_Array(chain, NULL,
      &length);
  printf("%d\n", length);
  Print_Local_Neuroseg_Ellipse(locne);

  coordinate_3d_t coord[100];
  Local_Neuroseg_Ellipse_Array_Points(locne, length, 20, coord);

  darray_write("../data/test.bn", Coordinate_3d_Double_Array(coord), 
      sizeof(coord) / sizeof(double));
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/mouse_neuron_org/chain10063.bn");
  Print_Locseg_Chain(chain);

  Stack *stack = Read_Stack("../data/mouse_neuron_org.tif");
  Stack *canvas = Translate_Stack(stack, COLOR, 0);

  Locseg_Chain_Label(chain, canvas, 1.0);

  Write_Stack("../data/test.tif", canvas);
#endif

#if 0
  int chain_map[10];
  int chain_number;
  Neuron_Component *chain_array = 
    Dir_Locseg_Chain_Nc("../data/rn003/cross_45_4", "^chain.*\\.bn", 
	&chain_number, chain_map);

  Stack *signal = Read_Stack("../data/rn003/cross_45_4.tif");

  Connection_Test_Workspace ctw;

  /* reconstruct neuron */
  Neuron_Structure *ns = Locseg_Chain_Comp_Neurostruct(chain_array, 
      chain_number,
      signal, 1.0, &ctw);

  Neuron_Structure* ns2=
    Neuron_Structure_Locseg_Chain_To_Circle(ns);

  Neuron_Structure_To_Tree(ns2);

  Neuron_Structure_To_Swc_File_Circle_Z(ns2, "../data/test.swc", 1.0);


  //ns->graph->nedge = 0;
  //Neuron_Structure_To_Swc_File(ns, "../data/test.swc");

  /*
     Print_Neuron_Structure(ns);

     Neuron_Structure* ns2=
     Neuron_Structure_Locseg_Chain_To_Circle(ns);

     Neuron_Structure_Main_Graph(ns2);
     Neuron_Structure_To_Tree(ns2);
     Neuron_Structure_To_Swc_File_Circle(ns2, "../data/test.swc");
   */

  /*
     Print_Neuron_Structure(ns2);

     Neuron_Structure_To_Tree(ns2);

     Graph_To_Dot_File(ns2->graph, "../data/test.dot");
   */
#endif

#if 0
  int chain_number;
  Neuron_Component *chain_array = 
    Dir_Locseg_Chain_Nc("../data/fly_neuron_n3", "^chain.*\\.bn", 
	&chain_number, NULL);

  Stack *signal = Read_Stack("../data/fly_neuron_n3.tif");

  /* reconstruct neuron */
  Neuron_Structure *ns = Locseg_Chain_Comp_Neurostruct(chain_array, 
      chain_number,
      signal, 1.0, NULL);
  ns->graph->nedge = 0;
  Neuron_Structure_To_Swc_File(ns, "../data/test.swc");  
#endif

#if 0
  int chain_number;
  Locseg_Chain *chain_array = 
    Dir_Locseg_Chain_N("../data/rn003/cross_45_2", "^chain.*\\.bn", 
	&chain_number, NULL);

  /*
     int n;
     coordinate_3d_t* knot1 = 
     Locseg_Chain_Knot_Array(chain_array + 1, 0, -1, NULL, &n);

     darray_write("../data/test1.bn", Coordinate_3d_Double_Array(knot1),
     n * 3);

     coordinate_3d_t* knot2 = 
     Locseg_Chain_Knot_Array(chain_array + 2, 0, -1, NULL, &n);

     darray_write("../data/test2.bn", Coordinate_3d_Double_Array(knot2),
     n * 3);
     return 1;
   */
  Stack *stack = Read_Stack("../data/rn003/cross_45_2.tif");
  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  Connection_Test_Workspace ctw;

  printf("%d\n", Locseg_Chain_Length(chain_array + 1));
  printf("%d\n", Locseg_Chain_Length(chain_array + 0));

  Locseg_Chain_Connection_Test(chain_array + 1, chain_array + 0,
      stack, 1.0, &conn, &ctw);

  Print_Neurocomp_Conn(&conn);

  /*
     int i;
     for (i = 0; i< chain_number; i++) {
     Print_Locseg_Chain(chain_array + i);
     }
   */
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/fly_neuron_n12/chain13.bn");
  Stack *stack = Read_Stack("../data/fly_neuron_n12.tif");
  //Print_Locseg_Chain(chain);
  double *value = Locseg_Chain_Intensity_Profile(chain, stack, 1.0, NULL);
  int n = Locseg_Chain_Length(chain);
  darray_print2(value, n, 1);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/fly_neuron_n12/chain13.bn");
  //Print_Locseg_Chain(chain);
  int n = 0;
  coordinate_3d_t *coord = Locseg_Chain_Knot_Array(chain, 3, 0, NULL, &n);

  printf("%d knots\n", n);
  int i;
  for (i = 0; i < n; i++) {
    Print_Coordinate_3d(coord[i]);
  }
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/fly_neuron_n12/chain13.bn");
  Print_Locseg_Chain(chain);
  return 1;

  int n = 0;
  Local_Neuroseg_Ellipse *locne = 
    Locseg_Chain_Ellipse_Knot_Array(chain, 0, 3, NULL, &n);

  printf("%d knots\n", n);
  int i;
  for (i = 0; i < n; i++) {
    Print_Local_Neuroseg_Ellipse(locne + i);
  }
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/test0.bn");
  Print_Locseg_Chain(chain);

  int n;
  Locseg_Chain *chain_array = Locseg_Chain_Refit_Break(chain, NULL, &n);

  printf("%d\n", n);

  int i;
  for (i = 0; i < n; i++) {
    //Print_Locseg_Chain(chain_array + i);
    printf("%d\n", Locseg_Chain_Length(chain_array + i));
  }
#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("../data/test0.bn");
  Locseg_Chain *loop = Read_Locseg_Chain("../data/test1.bn");
  Print_Locseg_Chain(hook);
  Print_Locseg_Chain(loop);

  Stack *stack = Read_Stack("../data/mouse_neuron4_org_crop.tif");
  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  conn.info[0] = 1;
  conn.info[1] = 1;
  Locseg_Chain_Connection_Test(hook, loop, stack, 1.0, &conn, NULL);

  Print_Neurocomp_Conn(&conn);

  conn.mode = NEUROCOMP_CONN_HL;
  Locseg_Chain_Connection_Test(loop, hook, stack, 1.0, &conn, NULL);

  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/test.bn");
  Print_Locseg_Chain(chain);
#endif

#if 0
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/fly_neuron_n1/chain14.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/fly_neuron_n1/chain7.tb");
  Local_Neuroseg *locseg1 = Locseg_Chain_Head_Seg(chain1);
  Local_Neuroseg *locseg2 = Locseg_Chain_Peek_Seg_At(chain2, 3);

  Stack *stack = Read_Stack("../data/fly_neuron_n1.tif");
  double res[3];
  FILE *fp = fopen("../data/fly_neuron_n1.res", "r");  
  darray_fscanf(fp, res, 3);
  fclose(fp);

  int n;
  double *feat = Locseg_Conn_Feature(locseg1, locseg2, stack, res, NULL, &n);

  darray_print2(feat, n, 1);
#endif

#if 0
  int m, n;
  int *conn = iarray_load_matrix("../data/fly_neuron_n1_conn.txt", NULL,
      &m, &n);
  iarray_print2(conn, m, n);

  char filepath[100];
  Stack *stack = Read_Stack("../data/fly_neuron_n1.tif");
  double res[3];
  FILE *fp = fopen("../data/fly_neuron_n1.res", "r");
  darray_fscanf(fp, res, 3);
  darray_print2(res, 3, 1);
  fclose(fp);

  int i, j;
  for (i = 0; i < n; i++) {
    sprintf(filepath, "../data/fly_neuron_n1/chain%d.tb", conn[i*m]);
    Locseg_Chain *chain1 = Read_Locseg_Chain(filepath);
    sprintf(filepath, "../data/fly_neuron_n1/chain%d.tb", conn[i*m+1]);
    Locseg_Chain *chain2 = Read_Locseg_Chain(filepath);
    Local_Neuroseg *locseg1 = NULL;
    if (conn[i*m+2] == 0) {
      locseg1 = Locseg_Chain_Head_Seg(chain1);
    } else {
      locseg1 = Locseg_Chain_Tail_Seg(chain1);
    }

    Local_Neuroseg *locseg2 = NULL;

    if (m > 4) {
      if (conn[i*m+4] == 2) {
	if (conn[i*m+3] == 0) {
	  locseg2 = Locseg_Chain_Head_Seg(chain2);
	} else {
	  locseg2 = Locseg_Chain_Tail_Seg(chain2);
	}
      }
    } 

    if (locseg2 == NULL) {
      locseg2 = Locseg_Chain_Peek_Seg_At(chain2, conn[i*m+3]);
    }
    int nf;
    double *feat = Locseg_Conn_Feature(locseg1, locseg2, stack, res, NULL, &nf);
    darray_print2(feat, nf, 1);
  }
#endif

#if 0
  int m, n;
  int *conn = iarray_load_matrix("../data/mouse_single_org_conn.txt", NULL,
      &m, &n);
  iarray_print2(conn, m, n);

  char filepath[100];
  Stack *stack = Read_Stack("../data/mouse_single_org.tif");
  double res[3];
  FILE *fp = fopen("../data/mouse_single_org.res", "r");
  darray_fscanf(fp, res, 3);
  darray_print2(res, 3, 1);
  fclose(fp);

  int i, j;
  for (i = 0; i < n; i++) {
    sprintf(filepath, "../data/mouse_single_org/chain%d.tb", conn[i*m]);
    Locseg_Chain *chain1 = Read_Locseg_Chain(filepath);
    sprintf(filepath, "../data/mouse_single_org/chain%d.tb", conn[i*m+1]);
    Locseg_Chain *chain2 = Read_Locseg_Chain(filepath);
    Local_Neuroseg *locseg1 = NULL;
    if (conn[i*m+2] == 0) {
      locseg1 = Locseg_Chain_Head_Seg(chain1);
    } else {
      locseg1 = Locseg_Chain_Tail_Seg(chain1);
    }

    Local_Neuroseg *locseg2 = NULL;

    if (m > 4) {
      if (conn[i*m+4] == 2) {
	if (conn[i*m+3] == 0) {
	  locseg2 = Locseg_Chain_Head_Seg(chain2);
	} else {
	  locseg2 = Locseg_Chain_Tail_Seg(chain2);
	}
      }
    } 

    if (locseg2 == NULL) {
      locseg2 = Locseg_Chain_Peek_Seg_At(chain2, conn[i*m+3]);
    }

    int nf;
    double *feat = Locseg_Conn_Feature(locseg1, locseg2, stack, res, NULL, &nf);
    darray_print2(feat, nf, 1);
  }
#endif

#if 0
  int m, n;
  int *conn = iarray_load_matrix("../data/mouse_neuron_sp2_conn.txt", NULL,
      &m, &n);
  iarray_print2(conn, m, n);

  char filepath[100];
  Stack *stack = Read_Stack("../data/mouse_neuron_sp2.tif");
  double res[3];
  FILE *fp = fopen("../data/mouse_neuron_sp2.res", "r");
  darray_fscanf(fp, res, 3);
  darray_print2(res, 3, 1);
  fclose(fp);

  int i, j;
  for (i = 0; i < n; i++) {
    sprintf(filepath, "../data/mouse_neuron_sp2/chain%d.tb", conn[i*m]);
    Locseg_Chain *chain1 = Read_Locseg_Chain(filepath);
    sprintf(filepath, "../data/mouse_neuron_sp2/chain%d.tb", conn[i*m+1]);
    Locseg_Chain *chain2 = Read_Locseg_Chain(filepath);
    Local_Neuroseg *locseg1 = NULL;
    if (conn[i*m+2] == 0) {
      locseg1 = Locseg_Chain_Head_Seg(chain1);
    } else {
      locseg1 = Locseg_Chain_Tail_Seg(chain1);
    }

    Local_Neuroseg *locseg2 = Locseg_Chain_Peek_Seg_At(chain2, conn[i*m+3]);
    int nf;
    double *feat = Locseg_Conn_Feature(locseg1, locseg2, stack, res, NULL, &nf);
    darray_print2(feat, nf, 1);
  }
#endif

#if 0
  Stack *stack = Read_Stack("../data/mouse_single_org.tif");
  Locseg_Chain *chain = Read_Locseg_Chain("../data/mouse_single_org/chain0.tb");
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->signal = Copy_Stack(stack);
  double sigma[3] = {1.0, 1.0, 1.0};
  FMatrix *filter = Gaussian_3D_Filter_F(sigma, NULL);
  FMatrix *f = Filter_Stack_Fast_F(ws->signal, filter, NULL, 0);
  Correct_Filter_Stack_F(filter, f);
  Kill_Stack(ws->signal);
  Stack_View sv = Stack_View_FMatrix(f);
  ws->signal = &(sv.stack);

  ws->sratio = 1.5;
  ws->sdiff = 1.0;
  ws->option = 2;
  ws->value = 1;
  ws->flag = 0;

  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Locseg_Chain_Label_W(chain, mask, 1.0, 0, Locseg_Chain_Length(chain) - 1, ws);

  Stack *mask2 = Stack_Z_Dilate(mask, 3, ws->signal, NULL);
  //Stack *mask2 = mask;
  Stack_Not(mask2, mask2);
  Stack_Mul(mask2, stack, stack);

  Write_Stack("../data/test.tif", stack);
#endif

#if 0
  int length;
  Local_Neuroseg *locseg = Read_Local_Neuroseg_Array("../data/test.cd", &length);
  Print_Local_Neuroseg(locseg);

  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add_Node(chain, Make_Locseg_Node(locseg, NULL), DL_TAIL);
  Write_Locseg_Chain("../data/test.tb", chain);

#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/mouse_single_org/chain0.tb");
  Print_Locseg_Chain(chain);

  Write_Locseg_Chain("../data/test.tb", chain);
  chain = Read_Locseg_Chain("../data/test.tb");

  Stack *signal = Read_Stack("../data/mouse_single_org.tif");

  double score = 
    Locseg_Chain_Average_Score(chain, signal, 1.0, STACK_FIT_CORRCOEF);
  printf("%d\n", Locseg_Chain_Length(chain));
  printf("%g\n", score);
#endif

#if 0
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/mouse_neuron_sp2/chain179.tb");
  Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/mouse_neuron_sp2/chain20.tb");

  printf("%d\n", Locseg_Chain_Length(chain1));
  printf("%d\n", Locseg_Chain_Length(chain2));

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  double res[3] = {0.14, 0.14, 0.8};
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->resolution[0] = res[0];
  ctw->resolution[1] = res[1];
  ctw->resolution[2] = res[2];
  Locseg_Chain_Connection_Test(chain1, chain2, NULL, 1.0, &conn, ctw);  
  Print_Neurocomp_Conn(&conn);

  Local_Neuroseg *locseg1 = Locseg_Chain_Tail_Seg(chain1);
  Local_Neuroseg *locseg2 = Locseg_Chain_Tail_Seg(chain2);
  printf("%g\n", 
      Neuroseg_Angle_Between(&(locseg1->seg), 
	&(locseg2->seg)) * 180.0 / TZ_PI);

#endif

#if 0
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/fly_neuron_n2/chain0.tb");
  printf("%d\n", Locseg_Chain_Length(chain1));
  Write_Locseg_Chain("../data/test.tb", chain1);
  //Print_Locseg_Chain(chain1);
#endif

#if 0
  printf("%d\n", Locseg_Chain_File_Trace_Record_Size
      ("../data/mouse_single_org/chain0.tb"));

  printf("%lu\n", sizeof(Trace_Record));
#endif

#if 0
  FILE *fp = fopen("../data/crossover.txt", "r");
  String_Workspace *sw = New_String_Workspace();

  char *line = NULL;
  char data[100];
  char filepath[100];
  int state = 0;
  int array[10];
  int n;
  Locseg_Chain *chain1, *chain2;
  Local_Neuroseg *locseg1, *locseg2;
  chain1 = NULL;
  chain2 = NULL;

  while ((line = Read_Line(fp, sw)) != NULL) {
    switch (state) {
      case 0:
	if (line[0] == '#') {
	  strcpy(data, line + 1);
	  state = 1;
	} 
	break;
      case 1:
	if (line[0] == '#') {
	  state = 0;
	} else {
	  String_To_Integer_Array(line, array, &n);
	  if (n == 5) {
	    sprintf(filepath, "../data/%s/chain%d.tb", data, array[0]);
	    chain1 = Read_Locseg_Chain(filepath);
	    sprintf(filepath, "../data/%s/chain%d.tb", data, array[1]);
	    chain2 = Read_Locseg_Chain(filepath);
	    if (array[2] == 0) {
	      locseg1 = Locseg_Chain_Head_Seg(chain1);
	      Flip_Local_Neuroseg(locseg1);
	    } else {
	      locseg1 = Locseg_Chain_Tail_Seg(chain1);
	    }
	    if (array[3] == 0) {
	      locseg2 = Locseg_Chain_Head_Seg(chain2);
	    } else {
	      locseg2 = Locseg_Chain_Tail_Seg(chain2);
	      Flip_Local_Neuroseg(locseg2);
	    }
	    printf("%g\n", Neuroseg_Angle_Between(&(locseg1->seg), 
		  &(locseg2->seg)) * 180.0 / TZ_PI);
	  }
	}
	break;
    }
    if (chain1 != NULL) {
      Kill_Locseg_Chain(chain1);
      chain1 = NULL;
    }
    if (chain2 != NULL) {
      Kill_Locseg_Chain(chain2);
      chain2 = NULL;
    }
  }

  fclose(fp);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/fly_neuron_n1/chain10.tb");
  printf("%d\n", Locseg_Chain_Length(chain));

  Print_Locseg_Chain(chain);

  Locseg_Chain_Iterator_Start(chain, DL_TAIL);
  Locseg_Chain_Prev(chain); 
  Locseg_Chain_Remove_Current(chain);
  printf("%d\n", Locseg_Chain_Length(chain));
  Print_Locseg_Chain(chain);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e1/chain30.tb");
  printf("%d\n", Locseg_Chain_Length(chain));
  Locseg_Chain_Down_Sample(chain);
  printf("%d\n", Locseg_Chain_Length(chain));
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e1/chain0.tb");
  printf("%d\n", Locseg_Chain_Length(chain));
  int n;
  Neuron_Component *nc =
    Locseg_Chain_To_Neuron_Component(chain, NEUROCOMP_TYPE_GEO3D_CIRCLE, 
	NULL, &n);
  printf("%d\n", n);
#endif

#if 0
  Local_Neuroseg *locseg = Locseg_Neuroseg_Import_Xml("../data/locseg.xml", 						      NULL);

  locseg->seg.theta = Normalize_Radian(locseg->seg.theta);
  locseg->seg.psi = Normalize_Radian(locseg->seg.psi);

  Local_Neuroseg *locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.5);
  Local_Neuroseg *locseg3 = Next_Local_Neuroseg(locseg2, NULL, 0.5);

  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
  Locseg_Chain_Add(chain, locseg3, NULL, DL_TAIL);

  Flip_Local_Neuroseg(locseg2);

  printf("%d\n", Locseg_Chain_Connect_Seg(chain, locseg2));

  Locseg_Chain_Flip(chain);

  Print_Locseg_Chain(chain);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain0.tb");
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Chain_Next_Seg(chain);
  Locseg_Chain_Next_Seg(chain);
  Local_Neuroseg *locseg = Locseg_Chain_Next_Seg(chain);
  Stack *signal = Read_Stack("../data/diadem_e3.tif");
  double profile[11];
  Local_Neuroseg_Height_Profile(locseg, signal, 1.0, 11, 
      STACK_FIT_MEAN_SIGNAL, profile);
  darray_print2(profile, 11, 1);
#endif

#if 0
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/diadem_e3/chain12.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/diadem_e3/chain14.tb");
  Locseg_Chain_Merge(chain1, chain2, DL_TAIL, DL_HEAD);

  FILE *fp = fopen("../data/test.swc", "w");
  Locseg_Chain_Swc_Fprint_Z(fp, chain1, 2, 1, -1, DL_FORWARD, 1.0);
  fclose(fp);
#endif

#if 0 /* test Locseg_Chain_Label_W */
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain0.tb");
  Stack *mask = Read_Stack("../data/diadem_e3.tif");
  Zero_Stack(mask);
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->option = 6;
  ws->flag = 0;
  ws->value = 255;
  Locseg_Chain_Label_W(chain, mask, 1.0, 0, Locseg_Chain_Length(chain) - 1, ws);
  Write_Stack("../data/test.tif", mask);
#endif

#if 0
  int n;
  Locseg_Chain *chain_array = Dir_Locseg_Chain_N("../data/diadem_e2", 
      "^chain.*\\.tb", &n, NULL);
  Stack *mask = Read_Stack("../data/diadem_e2.tif");
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();

  int i;
  tic();
  for (i = 0; i < n; i++) {
    Locseg_Chain_Label_W(chain_array + i, mask, 1.0, 0, 
	Locseg_Chain_Length(chain_array + i) - 1, ws);
  }
  printf("time: %lld\n", toc());
  Write_Stack("../data/test.tif", mask);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e2/chain16.tb");
  Stack *mask = Read_Stack("../data/diadem_e2.tif");
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  Locseg_Chain_Label_W(chain, mask, 1.0, 0, 
      Locseg_Chain_Length(chain) - 1, ws);
  Write_Stack("../data/test.tif", mask);
#endif

#if 0
  Local_Neuroseg *locseg = Local_Neuroseg_Import_Xml("../data/locseg.xml", 
      NULL);
  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
  Print_Locseg_Chain(chain);

  Stack *mask = Make_Stack(GREY, 200, 200, 50);
  Zero_Stack(mask);
  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->option = 6;
  ws->flag = 0;
  ws->value = 255;
  Locseg_Chain_Label_W(chain, mask, 1.0, 0, Locseg_Chain_Length(chain) - 1, 
      ws);

  /*
     Local_Neuroseg *locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.5);
     Locseg_Chain_Remove_End(chain, DL_TAIL);
     Locseg_Chain_Add(chain, locseg2, NULL, DL_TAIL);
     Locseg_Chain_Label_W(chain, mask, 1.0, 0, Locseg_Chain_Length(chain) - 1, 
     ws);

     ws->option = 7;
     Locseg_Chain_Label_W(chain, mask, 1.0, 0, Locseg_Chain_Length(chain) - 1, 
     ws);
   */
  Write_Stack("../data/test.tif", mask);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain0.tb");
  Stack *stack = Read_Stack("../data/diadem_e3.tif");

  Local_Neuroseg *locseg = Locseg_Chain_Peek_Seg_At(chain, 6);

  Stack_Fit_Score fs;
  fs.n = 1;
  fs.options[0] = STACK_FIT_DOT;
  Print_Local_Neuroseg(locseg);

  printf("score: %g\n",  Local_Neuroseg_Score(locseg, stack, 1.0, &fs));

  double max_score = 
    Local_Neuroseg_R_Scale_Search(locseg, stack, 1.0, 1.0, 2.0, 0.1,
	0.5, 2.0, 0.05, NULL);

  Print_Local_Neuroseg(locseg);

  /*
     double r, s;

     int n = 0;
     double array[5000];
     double max_score = 0.0;

     for (r = 1.0; r <= 3.001; r += 0.1) {
     for (s = 0.5; s <= 2.001; s += 0.05) {
     locseg->seg.r1 = r;
     locseg->seg.scale = s;
     double score = Local_Neuroseg_Score(locseg, stack, 1.0, &fs);
     array[n++] = score;
     if (max_score < score) {
     max_score = score;
     }
     }
     }
   */

  printf("best score: %g\n", max_score);

  //darray_write("../data/test.ar", array, n);


  /*
     Stack *out = Local_Neuroseg_Stack(locseg, stack);

     Write_Stack("../data/test.tif", out);  
   */
#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("../data/diadem_e2/chain0.tb");
  Locseg_Chain *loop = Read_Locseg_Chain("../data/diadem_e2/chain1.tb");

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  conn.info[0] = 1;
  conn.info[1] = 1;

  double res[3] = {0.3296485, 0.3296485, 1.08};
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->resolution[0] = res[0];
  ctw->resolution[1] = res[1];
  ctw->resolution[2] = res[2];
  Locseg_Chain_Connection_Test(hook, loop, NULL, 1.0, &conn, ctw);  
  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("../data/diadem_e1/chain15.tb");
  Locseg_Chain *loop = Read_Locseg_Chain("../data/diadem_e1/chain0.tb");

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  conn.info[0] = 1;
  conn.info[1] = 1;

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  Locseg_Chain_Connection_Test(hook, loop, NULL, 1.0, &conn, ctw);  
  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e1/chain35.tb");
  Stack *mask = Read_Stack("../data/diadem_e1/trace_mask.tif");
  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  Locseg_Label_Workspace *ws = New_Locseg_Label_Workspace();
  ws->sratio = 1.0;
  ws->sdiff = 0.0;
  ws->flag = -1;
  ws->value = 0;
  Locseg_Chain_Label_W(chain, mask, 1.0, 10, 
      Locseg_Chain_Length(chain) - 1, ws);

  Print_Locseg_Chain(chain);

  tic();
  Locseg_Chain_Tune_End(chain, stack, 1.0, NULL, DL_HEAD);
  printf("time passed: %lld\n", toc());
  Print_Locseg_Chain(chain);
#endif

#if 0
  typedef struct _Test_Struct {
    int n;                                  /* number of options */
    int scores;  /* array to store results */  
  } Test_Struct;

  Test_Struct haha;
  printf("%zd\n", sizeof(haha));
  printf("%p %p\n", &(haha.n), &(haha.scores));
  /*
     Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain0.tb");
     Stack_Fit_Score fs;
     printf("%zd\n", sizeof(fs));
     printf("%zd\n", sizeof(Bitmask_t));
   */
  //Print_Locseg_Chain(chain);
#endif

#if 0
  /*
     Locseg_Chain *chain = 
     Read_Locseg_Chain("../data/adult_brain_Nrg_oblique/chain0.tb");
     Print_Locseg_Chain(chain);

     Write_Locseg_Chain("../data/test.tb", chain);
   */

  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e2/chain0.tb");
  Print_Locseg_Chain(chain);

  FILE *fp = fopen("../data/test2.swc", "w");
  Locseg_Chain_Swc_Fprint(fp, chain, 3, 1, -1, DL_FORWARD);
  fclose(fp);

  Locne_Chain *chain2 = Locseg_Chain_To_Locne_Chain(chain);
  Print_Locne_Chain(chain2);

  fp = fopen("../data/test.swc", "w");
  Locne_Chain_Swc_Fprint(fp, chain2, 2, 1, -1, DL_FORWARD);
  fclose(fp);
#endif

#if 0
  Local_Neuroseg *locseg1 = Local_Neuroseg_Import_Xml("../data/locseg.xml",
      NULL);
  int id = 1;

  FILE *fp = fopen("../data/test.swc", "w");

  //Local_Neuroseg_Swc_Fprint_T(fp, locseg1, id, -1, 2);
  Local_Neuroseg *locseg2 = Copy_Local_Neuroseg(locseg1);
  locseg2->pos[0] += 5;
  locseg2->pos[1] += 5;
  locseg2->pos[2] += 5;

  Flip_Local_Neuroseg(locseg2);

  id += 2;
  //Local_Neuroseg_Swc_Fprint_T(fp, locseg2, id, -1, 2);

  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add(chain, locseg1, NULL, DL_HEAD);
  Locseg_Chain_Add(chain, locseg2, NULL, DL_HEAD);

  //Locseg_Chain_Swc_Fprint(fp, chain, 3, 1, -1, DL_FORWARD);

  Locne_Chain *chain2 = Locseg_Chain_To_Locne_Chain(chain);
  Locne_Chain_Swc_Fprint(fp, chain2, 2, 1, -1, DL_FORWARD);

  fclose(fp);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/mouse_single_org/chain93.tb");
  /*
     Locseg_Node *node = Locseg_Chain_Head(chain);
     node->tr = NULL;
     Write_Locseg_Chain("../data/test.tb", chain);
     Locseg_Chain *chain2 = Read_Locseg_Chain("../data/test.tb");
     Print_Locseg_Chain(chain2);
   */
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e1/chain0.tb");
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  Print_Locseg_Chain_Knot_Array(ka);

  Print_Locseg_Chain(chain);

  Geo3d_Circle *circle = Locseg_Chain_Knot_Array_To_Circle_Z(ka, 1.0, NULL);
  int i;
  FILE *fp = fopen("../data/test.swc", "w");

  int pid = -1;
  for (i = 0; i < Locseg_Chain_Knot_Array_Length(ka); i++) {
    //Print_Geo3d_Circle(circle + i);
    Geo3d_Circle_Swc_Fprint(fp, circle + i, i + 1, pid);
    pid = i + 1;
  }

  fclose(fp);
#endif

#if 0
  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/diadem_e1/chain1.tb");

  /*
     FILE *fp = fopen("../data/test.swc", "w");
     Local_Neuroseg_Swc_Fprint(fp, Locseg_Chain_Peek_At(chain2, 15)->locseg, 1, -1);
     Local_Neuroseg_Swc_Fprint_T(fp, Locseg_Chain_Peek_At(chain2, 16)->locseg, 3, -1, 3);
     Local_Neuroseg_Swc_Fprint(fp, Locseg_Chain_Peek_At(chain2, 17)->locseg, 5, -1);
     fclose(fp);
   */

  /*
     Locseg_Chain_Add_Node(chain, Locseg_Chain_Peek_At(chain2, 15), DL_TAIL);
     Locseg_Node *node = Locseg_Chain_Peek_At(chain2, 16);
  //node->tr = New_Trace_Record();
  //Trace_Record_Set_Fix_Point(node->tr, 0.5);
  //Trace_Record_Set_Direction(node->tr, DL_FORWARD);
  Locseg_Chain_Add_Node(chain, node, DL_TAIL);
  Locseg_Chain_Add_Node(chain, Locseg_Chain_Peek_At(chain2, 17), DL_TAIL);
   */

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Print_Local_Neuroseg(locseg);

  Trace_Record *tr = New_Trace_Record();
  Trace_Record_Set_Direction(tr, DL_BOTHDIR);
  Locseg_Chain_Add_Node(chain, Make_Locseg_Node(locseg, tr), DL_TAIL);

  Local_Neuroseg *locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.5);
  tr = New_Trace_Record();
  Trace_Record_Set_Direction(tr, DL_FORWARD);
  locseg2->seg.theta += 0.2;
  //Locseg_Chain_Add_Node(chain, Make_Locseg_Node(locseg2, tr), DL_TAIL);

  Flip_Local_Neuroseg(locseg);
  locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.5);
  Flip_Local_Neuroseg(locseg);
  tr = New_Trace_Record();
  Trace_Record_Set_Direction(tr, DL_BACKWARD);
  locseg2->seg.theta -= 0.2;
  Flip_Local_Neuroseg(locseg2);
  Locseg_Chain_Add_Node(chain, Make_Locseg_Node(locseg2, tr), DL_HEAD);

  Print_Locseg_Chain(chain);

  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  Print_Locseg_Chain_Knot_Array(ka);

  Geo3d_Circle *circle = Locseg_Chain_Knot_Array_To_Circle_Z(ka, 1.0, NULL);
  int i;
  FILE *fp = fopen("../data/test.swc", "w");

  int pid = -1;
  for (i = 0; i < Locseg_Chain_Knot_Array_Length(ka); i++) {
    //Print_Geo3d_Circle(circle + i);
    Geo3d_Circle_Swc_Fprint(fp, circle + i, i + 1, pid);
    pid = i + 1;
  }

  fclose(fp);

  printf("%g\n", Locseg_Chain_Geolen(chain));

#endif

#if 0
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/diadem_e1/chain0.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/diadem_e1/chain1.tb");
  int n;
  double res[3] = {0.3296485, 0.3296485, 1.0};
  double *feat = Locseg_Chain_Conn_Feature(chain1, chain2, NULL, res, NULL, &n);

  darray_print2(feat, n, 1);
#endif

#if 0
  Locseg_Chain *chain = 
    Read_Locseg_Chain("../data/benchmark/mouse_neuron_single/chain/chain0.tb");

  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  Geo3d_Ellipse *ellipse = Locseg_Chain_Knot_Array_To_Ellipse_Z(ka, 1.0, NULL);

  int i;
  int n = Locseg_Chain_Knot_Array_Length(ka);
  Swc_Node node;
  FILE *fp = fopen("../data/test.swc", "w");
  for (i = 0; i < n; i++) {
    Print_Geo3d_Ellipse(ellipse + i);
    if (i == 0) {
      Geo3d_Ellipse_To_Swc_Node(ellipse + i, 1, -1, 1.0, 2, &node);
    } else {
      Geo3d_Ellipse_To_Swc_Node(ellipse + i, i + 1, i, 1.0, 2, &node);
    }
    Print_Swc_Node(&node);
    Swc_Node_Fprint(fp, &node);
  }
  fclose(fp);
#endif

#if 0
  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/benchmark/mouse_neuron_single/chain/chain1.tb");   Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/benchmark/mouse_neuron_single/chain/chain28.tb");  
  printf("%d\n", Locseg_Chain_Length(chain1));
  printf("%g\n", Locseg_Chain_Dist(chain2, chain1, DL_HEAD));
#endif

#if 0
  int chain_number;
  Locseg_Chain *chain_array = 
    Dir_Locseg_Chain_N("../data/diadem_e1", "^chain.*\\.tb",
	&chain_number, NULL);

  printf("%d\n", chain_number);

  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  double *value = darray_malloc(chain_number);

  int i;
  for (i = 0; i < chain_number; i++) {
    value[i] = Locseg_Chain_Average_Signal(chain_array + i, stack, 1.0);
  }

  printf("%g\n", darray_mean(value, chain_number));
  darray_write("../data/presentation9/diadem_e1/chain_signal.bn", 
      value, chain_number);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Print_Local_Neuroseg(locseg);

  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
  locseg = Copy_Local_Neuroseg(locseg);
  //locseg = Next_Local_Neuroseg(locseg, NULL, 0.5);
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  Print_Locseg_Chain_Knot_Array(ka);

  double pt[3];
  double ort[3] = {1, 0, 1};
  Local_Neuroseg_Axis_Coord_N(locseg, 0.7, pt);

  BOOL changed = Locseg_Chain_Interpolate_L(chain, pt, ort);

  ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  Print_Locseg_Chain_Knot_Array(ka);

  Locseg_Chain_To_Swc_File(chain, "../data/test.swc");

  printf("%d\n", changed);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  locseg->seg.c = 0.1;

  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);


  double pt[3] = {5, 5, 5};
  double ort[3] = {1, 1, 0};
  Coordinate_3d_Unitize(ort);
  Locseg_Chain_Interpolate_L(chain, pt, ort, NULL);

  Locseg_Chain_To_Swc_File(chain, "../data/test.swc");
  Print_Locseg_Chain(chain);
#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("../data/diadem_e3/chain2.tb");
  Locseg_Chain *loop = Read_Locseg_Chain("../data/diadem_e3/chain3.tb");

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  conn.info[0] = 1;
  conn.info[1] = 1;

  Stack *stack = Read_Stack("../data/diadem_e3.tif");

  double res[3] = {0.3296485, 0.3296485, 1.08};
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->resolution[0] = res[0];
  ctw->resolution[1] = res[1];
  ctw->resolution[2] = res[2];
  Locseg_Chain_Connection_Test(hook, loop, stack, 1.0, &conn, ctw);  
  Print_Neurocomp_Conn(&conn);  
  //Print_Locseg_Chain(loop);

  hook = Read_Locseg_Chain("../data/diadem_e3/chain4.tb");
  Locseg_Chain_Connection_Test(hook, loop, stack, 1.0, &conn, ctw);  

  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(loop, NULL);
  int i;
  double pos[3];
  for (i = 0; i < Locseg_Chain_Knot_Array_Length(ka); i++) {
    Locseg_Chain_Knot_Pos(ka, i, pos);
    printf("%g, %g, %g\n", pos[0], pos[1], pos[2]);
  }
#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("../data/diadem_e3/chain1.tb");
  Locseg_Chain *loop = Read_Locseg_Chain("../data/diadem_e3/chain0.tb");

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  conn.info[0] = 1;
  conn.info[1] = 1;

  Stack *stack = Read_Stack("../data/diadem_e3.tif");

  double res[3] = {0.3296485, 0.3296485, 1.08};
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->resolution[0] = res[0];
  ctw->resolution[1] = res[1];
  ctw->resolution[2] = res[2];
  ctw->dist_thre = 25.0;
  ctw->mask = Make_Stack(GREY, stack->width, stack->height, 
      stack->depth);
  One_Stack(ctw->mask);
  Locseg_Chain_Erase(hook, stack, 1.0);
  Locseg_Chain_Erase(loop, stack, 1.0);

  //Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e2/chain1.tb");
  //Locseg_Chain_Erase(chain, stack, 1.0);

  Locseg_Chain_Connection_Test(hook, loop, stack, 1.0, &conn, ctw);  

  Locseg_Chain_To_Swc_File(loop, "../data/test.swc");

  Print_Neurocomp_Conn(&conn);  


  //Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(loop, NULL);
  //Print_Locseg_Chain_Knot_Array(ka);
  //Print_Locseg_Chain(loop);
#endif

#if 0
  Locseg_Chain *hook = 
    Read_Locseg_Chain("/Users/zhaot/Data/jinny/110309_12L/tracing/tb/BasalDenB_new/chain7.tb");
  Locseg_Chain *loop = 
    Read_Locseg_Chain("/Users/zhaot/Data/jinny/110309_12L/tracing/tb/BasalDenB_new/chain12.tb");

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_HL;
  conn.info[0] = 1;
  conn.info[1] = 1;

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();

  Locseg_Chain *chain = Copy_Locseg_Chain(loop);

  Locseg_Chain_To_Swc_File(loop, "../data/test.swc");

  Print_Locseg_Chain(loop);
  Locseg_Chain_Connection_Test(hook, loop, NULL, 1.0, &conn, ctw);  

  printf("%d\n", Locseg_Chain_Is_Identical(loop,chain)); 

  Print_Neurocomp_Conn(&conn);  


  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(loop, NULL);
  Print_Locseg_Chain_Knot_Array(ka);
  //Print_Locseg_Chain(loop);
#endif

#if 0
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  locseg->seg.c = 0.1;

  Locseg_Chain *chain = New_Locseg_Chain();
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);

  Locseg_Chain_Break_Node(chain, 0, 0.5);
  Locseg_Chain_Break_Node(chain, 0, 0.2);
  Locseg_Chain_Remove_Seed(chain);
  Print_Locseg_Chain(chain);

#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain14.tb");
  Print_Locseg_Chain(chain);

  Locseg_Chain_Knot_Array *ka = Locseg_Chain_To_Knot_Array(chain, NULL);
  Print_Locseg_Chain_Knot_Array(ka);

  Locseg_Chain_To_Swc_File(chain, "../data/test.swc");
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain14.tb");
  Print_Locseg_Chain(chain);

  Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain);
  printf("%d\n", Locseg_Chain_Form_Loop(chain, locseg, DL_BACKWARD));
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain0.tb");
  Print_Locseg_Chain(chain);
  Local_Neuroseg *locseg = New_Local_Neuroseg(); 
  //Locseg_Chain_Add(chain, locseg, NULL, DL_HEAD);

  Locseg_Chain_To_Vrml_File(chain, "../data/test.wrl");
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e3/chain0.tb");

  Locseg_Chain *chain2 = New_Locseg_Chain();
  Locseg_Chain_Add(chain2, Locseg_Chain_Head_Seg(chain), NULL, DL_TAIL);

  Print_Locseg_Chain(chain2);

  int n;
  Geo3d_Ellipse *ep_array = 
    Locseg_Chain_To_Ellipse_Array_Z(chain2, 1.0, NULL, &n);

  FILE *fp = fopen("../data/test.swc", "w");
  coordinate_3d_t *pts = Geo3d_Ellipse_Array_Sampling(ep_array, n, 20, NULL);
  Geo3d_Point_Array_Swc_Fprint(fp, pts, n * 20, 1, -1, 1.0, 2);
  fclose(fp);
#endif

#if 0
  Locseg_Chain *chain = 
    Read_Locseg_Chain("../data/benchmark/mouse_neuron_single/chain/chain38.tb");
  Stack *stack = Read_Stack("../data/benchmark/mouse_neuron_single/stack.tif");

  double pos[3];
  Locseg_Chain_Bright_End(chain, DL_HEAD, stack, 1.0, pos);
  /* 296.298, 304.877, 90.0623 */
  printf("%g, %g, %g\n", pos[0], pos[1], pos[2]);
  Locseg_Chain_Bright_End(chain, DL_TAIL, stack, 1.0, pos);
  /* 429.715, 348.662, 108.275 */
  printf("%g, %g, %g\n", pos[0], pos[1], pos[2]);

#endif

#if 0
  Locseg_Chain *hook = Read_Locseg_Chain("../data/diadem_e1/chain7.tb");
  Locseg_Chain *loop = Read_Locseg_Chain("../data/diadem_e1/chain0.tb");

  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  Neurocomp_Conn conn;
  Connection_Test_Workspace *ws = New_Connection_Test_Workspace();
  Locseg_Chain_Connection_Test(hook, loop, stack, 1.0, &conn, ws);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  int path[100];
  int n = 0;
  int y;
  for (y = 23; y < 49; y++) {
    path[n++] = Stack_Util_Offset(50, y, 50, stack->width, stack->height,
	stack->depth);
  }

  Locseg_Chain_Skel_Workspace *ws = New_Locseg_Chain_Skel_Workspace();
  ws->fw = New_Locseg_Fit_Workspace();

  Locseg_Chain *chain = 
    Locseg_Chain_From_Skel(path, n, 1.0, 1.0, stack, 1.0, ws->fw);  

  Translate_Stack(stack, COLOR, 1);
  Locseg_Chain_Label(chain, stack, 1.0);

  Write_Stack("../data/test2.tif", stack);
#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  Stack *mask = Make_Stack(GREY, Stack_Width(stack), Stack_Height(stack),
      Stack_Depth(stack));
  Zero_Stack(mask);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.3296;
  sgw->resolution[1] = 0.3296;
  sgw->resolution[2] = 1.0;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;
  Stack_Sp_Grow_Infer_Parameter(sgw, stack);

  Locseg_Chain *chain1 = 
    Read_Locseg_Chain("../data/diadem_e1/chain0.tb");
  Locseg_Chain *chain2 = 
    Read_Locseg_Chain("../data/diadem_e1/chain24.tb");

  Locseg_Chain *chain = Locseg_Chain_Bridge_Sp_G(chain1, chain2, stack, 1.0,
      sgw);

  Stack *canvas = Translate_Stack(stack, COLOR, 1);

  Stack_Sp_Grow_Draw_Result(canvas, NULL, sgw);
  Locseg_Chain_Label(chain, stack, 1.0);

  Write_Stack("../data/test2.tif", canvas);   
#endif

#if 0
  Stack *stack = Read_Stack("../data/diadem_e1.tif");
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_e1/chain0.tb");
  Locseg_Chain_Label_G(chain, stack, 1.0, 0, -1, 1.0, 0.0, -1, 0.0);
  printf("%d\n", Locseg_Chain_Has_Stack_Value(chain, stack, 1.0, 1.0));
  Locseg_Chain_Label_G(chain, stack, 1.0, 0, -1, 1.0, 0.0, -1, 1.0);
  printf("%d\n", Locseg_Chain_Has_Stack_Value(chain, stack, 1.0, 1.0));

#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/tmp/chain61.tb");
  Print_Locseg_Chain(chain);
#endif

#if 0
  int n;
  Locseg_Chain **chain_array = 
    Locseg_Chain_Import_List("../data/diadem_a1_part2/good_tube.txt", &n);
  printf("%d chains\n", n);
#endif

#if 0
  /* load data */
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  Neuron_Structure *ns = New_Neuron_Structure();
  ns->graph = New_Graph();
  ns->comp = Dir_Locseg_Chain_Nc("../data/benchmark/stack_graph/fork",
      "^chain.*\\.tb", &(ns->graph->nvertex), NULL);

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 1;
  conn.cost = 0;
  Neuron_Structure_Add_Conn(ns, 0, 1, &conn);

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  Locseg_Chain_Comp_Neurostruct_W(ns, stack, 1.0, ctw);

  Print_Neuron_Structure(ns);

  Process_Neuron_Structure(ns);
  Neuron_Structure_To_Tree(ns);

  Neuron_Structure_Remove_Conn(ns, 0, 1);

  Neuron_Structure* ns2= NULL;

  ns2 = Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);

  Neuron_Structure_To_Tree(ns2);

  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);

  Swc_Tree_Resort_Id(tree);

  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  /* load data */
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");
  Neuron_Structure *ns = New_Neuron_Structure();
  ns->graph = New_Graph();
  ns->comp = Dir_Locseg_Chain_Nc("../data/benchmark/stack_graph/fork",
      "^chain.*\\.tb", &(ns->graph->nvertex), NULL);

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 1;
  conn.cost = 0;
  Neuron_Structure_Add_Conn(ns, 0, 1, &conn);

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  Locseg_Chain_Comp_Neurostruct_W(ns, stack, 1.0, ctw);

  Print_Neuron_Structure(ns);

  Process_Neuron_Structure(ns);
  Neuron_Structure_To_Tree(ns);

  Neuron_Structure_Remove_Conn(ns, 0, 1);

  Neuron_Structure* ns2= NULL;

  ns2 = Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);

  Neuron_Structure_To_Tree(ns2);

  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);

  Swc_Tree_Resort_Id(tree);

  Write_Swc_Tree("../data/test.swc", tree);
#endif

#if 0
  Stack *stack = Read_Stack("../data/benchmark/fork2/fork2.tif");

  Neuron_Structure *ns = New_Neuron_Structure();
  ns->graph = New_Graph();
  ns->comp = Dir_Locseg_Chain_Nc("../data/benchmark/fork2/tubes",
      "^chain.*\\.tb", &(ns->graph->nvertex), NULL);

  Neurocomp_Conn conn;
  conn.mode = NEUROCOMP_CONN_LINK;
  conn.info[0] = 0;
  conn.info[1] = 1;
  conn.cost = -1;
  Neuron_Structure_Add_Conn(ns, 0, 2, &conn);

  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->dist_thre = 100.0;
  ctw->sp_test = FALSE;

  Locseg_Chain_Comp_Neurostruct_W(ns, stack, 1.0, ctw);

  Process_Neuron_Structure(ns);
  Neuron_Structure_To_Tree(ns);

  Neuron_Structure_Remove_Conn(ns, 0, 2);

  Neuron_Structure* ns2= NULL;

  ns2 = Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);

  Neuron_Structure_To_Tree(ns2);

  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, NULL);

  Swc_Tree_Resort_Id(tree);

  Write_Swc_Tree("../data/test3.swc", tree);  
#endif

#if 0
  Locseg_Chain *chain = 
    Read_Locseg_Chain("../data/benchmark/fork2/tubes/chain0.tb");
  Print_Locseg_Chain(chain);
  Locseg_Chain_Iterator_Locate(chain, 1);
  Locseg_Chain_Remove_Current(chain);
  Locseg_Chain_Remove_Current(chain);
  Print_Locseg_Chain(chain);
#endif

#if 0
  Swc_Tree *tree = Read_Swc_Tree("../data/tmp/diadem_e10000.swc");
  Stack *stack = Read_Stack("../data/diadem_e1.tif");

  int n = Swc_Tree_Iterator_Start(tree, SWC_TREE_ITERATOR_DEPTH_FIRST, TRUE);
  Locseg_Fit_Workspace *fw = New_Locseg_Fit_Workspace();

  Geo3d_Ball *skel;
  GUARDED_MALLOC_ARRAY(skel, n, Geo3d_Ball);
  int i;

  if (Swc_Tree_Node_Is_Virtual(tree->root)) {
    n--;
    Swc_Tree_Next(tree);
  }

  for (i = 0; i < n; i++) {  
    Swc_Tree_Node *tn = Swc_Tree_Next(tree);
    skel[i].center[0] = Swc_Tree_Node_Data(tn)->x;
    skel[i].center[1] = Swc_Tree_Node_Data(tn)->y;
    skel[i].center[2] = Swc_Tree_Node_Data(tn)->z;
    skel[i].r = Swc_Tree_Node_Data(tn)->d;
  }
  Locseg_Chain *chain = Locseg_Chain_From_Ball_Array(skel, n, stack, 1.0, fw);

  Write_Locseg_Chain("../data/test.tb", chain);

  Kill_Swc_Tree(tree);
  Kill_Stack(stack);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/diadem_c1/chain180.tb");
  Print_Locseg_Chain(chain);
  Locseg_Chain *remain = Locseg_Chain_Break_At(chain, 5);
  Print_Locseg_Chain(remain);
#endif

#if 0
  Locseg_Chain *source = 
    Read_Locseg_Chain("../data/test0.tb");
  Locseg_Chain *target = 
    Read_Locseg_Chain("../data/benchmark/fork2/tubes/chain0.tb");

  printf("%g\n", Locseg_Chain_Hit_Ratio(source, target));
#endif

#if 0
  Locseg_Chain *chain1 =
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain1.tb");
  Print_Locseg_Chain(chain1);
  Locseg_Chain *chain2 =
    Read_Locseg_Chain("../data/benchmark/stack_graph/fork/chain0.tb");
  Stack *stack = Read_Stack("../data/benchmark/stack_graph/fork/fork.tif");

  double feat[100];
  int n = 0;
  Locseg_Chain_Conn_Feature(chain1, chain2, stack, NULL, feat, &n);
  darray_print2(feat, n, 1);

#endif

#if 0
  char *tube_file1 = "../data/diadem_c1/chain_199.tb";
  char *tube_file2 = "../data/diadem_c1/chain_304.tb";
  Locseg_Chain *chain1 = Read_Locseg_Chain(tube_file1);
  Locseg_Chain *chain2 = Read_Locseg_Chain(tube_file2);

  char *gt_folder = "/Users/zhaot/Data/diadem/Neocortical Layer 6 "
    "Axons/translated_manual";

  double dist_thre = 30.0;
  int is_connected = is_chains_connected(chain1, chain2, gt_folder, dist_thre);
  switch(is_connected){
    case 1: 
      printf("chain1 (%s) and chain2 (%s) is connected.\n", tube_file1,
	  tube_file2);
      break;
    case 0:
      printf("both are on manual traces but not connected.\n");
      break;
    case -1:
      printf("some tube is not on the manual trace. \n");
      break;
    case -2: 
      printf("the tubes are too far away.\n");
      break;
    default:   
      break;
  }

#endif

#if 0
  int n;
  int file_number[5000];
  Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd("../data/diadem_c1",
      "chain.*\\.tb", &n, file_number);
  //printf("%d\n", n);
  char *gt_folder = "/Users/zhaot/Data/diadem/Neocortical Layer 6 "
    "Axons/translated_manual";
  double dist_thre = 15.0;
  int is_connected = 0;
  int i, j;
  double feat[100];
  int nfeat;
  double res[3] = { 0.2941, 0.2941, 1 };
  Stack *stack = Read_Stack("../data/diadem_c1.tif");
  FILE *fp = fopen("../data/diadem_c1/conn_feat.csv", "w");
  char folder[500];
  char filepath[500];

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      printf("%d, %d\n", i, j);
      if (i != j) {
	is_connected = is_chains_connected(chain_array[j], chain_array[i], 
	    gt_folder, dist_thre);
	if (is_connected >= 0) {
	  fprintf(fp, "%d, %d, %d", file_number[i], file_number[j], 
	      is_connected);
	  Locseg_Chain_Conn_Feature(chain_array[i], chain_array[j], stack, res,
	      feat, &nfeat);
	  int k;
	  for (k = 0; k < nfeat; k++) {
	    fprintf(fp, ", %g", feat[k]); 
	  }
	  fprintf(fp, "\n");
	  sprintf(folder, "../data/diadem_c1/connections/conn%d_%d_%d.trace",
	      is_connected, file_number[i], file_number[j]);
	  char cmd[500];
	  sprintf(cmd, "mkdir -p %s/traced", folder);
	  system(cmd);
	  sprintf(filepath, "%s/project.xml", folder);
	  FILE *fp2 = fopen(filepath, "w");
	  fprintf(fp2, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");	  
	  fprintf(fp2, "<trace version = \"1.0\">\n");
	  fprintf(fp2, "<data>\n");
	  fprintf(fp2, "<image type=\"tif\">\n");
	  fprintf(fp2, "<url>../../../diadem_c1.tif</url>\n");
	  fprintf(fp2, "</image>\n");
	  fprintf(fp2, "</data>\n");
	  fprintf(fp2, "<output>\n");
	  fprintf(fp2, "<workdir>traced</workdir>\n");
	  fprintf(fp2, "<tube>chain</tube>\n");
	  fprintf(fp2, "</output>\n");
	  fprintf(fp2, "<object>\n");
	  for (k = 1; k <= 34; k++) {
	    fprintf(fp2,
		"<swc>../../translated_manual/NC_%02d.swc.translated.swc</swc>\n", 
		k);
	  }
	  fprintf(fp2, "</object>\n");
	  fprintf(fp2, "</trace>\n");
	  fclose(fp2);
	  sprintf(filepath, "%s/traced/chain%d.tb", folder, file_number[i]); 
	  Write_Locseg_Chain(filepath, chain_array[i]);
	  sprintf(filepath, "%s/traced/chain%d.tb", folder, file_number[j]); 
	  Write_Locseg_Chain(filepath, chain_array[j]);
	}
      }
    }
  }
  fclose(fp);
#endif

#if 0
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/diadem_c1/chain_199.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/diadem_c1/chain_304.tb");

  double feat[100];
  int nfeat;
  double res[3] = { 0.2941, 0.2941, 1 };
  Stack *stack = Read_Stack("../data/diadem_c1.tif");
  Locseg_Chain_Conn_Feature(chain1, chain2, stack, res, feat, &nfeat);
  int k;
  for (k = 0; k < nfeat; k++) {
    printf(", %g", feat[k]); 
  }
  printf("\n");
#endif

#if 0
  Locseg_Chain *chain =
    Read_Locseg_Chain("../data/diadem_c1/bad_chain/chain527.tb");
  Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain);
  printf("%g\n", Local_Neuroseg_Zscore(locseg));
  printf("%g\n", Locseg_Chain_Geolen(chain));
#endif

#if 0
  int n;
  int file_number[5000];
  Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd("../data/benchmark/diadem/diadem_e4",
      "chain.*\\.tb", &n, file_number);
  Stack *stack = Read_Stack("../data/benchmark/diadem/diadem_e4.tif");
  double thre = 0.0;
  int i;
  for (i = 0; i < n; i++) {
    double min_signal = Locseg_Chain_Min_Seg_Signal(chain_array[i], stack,
	1.0);
    if (thre < min_signal) {
      thre = min_signal;
    }
  } 
  printf("Threshold: %g\n", thre);

  Stack_Threshold_Binarize(stack, iround(thre)-1);
  Stack *onmask = Copy_Stack(stack);
  for (i = 0; i < n; i++) {
    Locseg_Chain_Erase_E(stack, chain_array[i],  1.0, 0,
	Locseg_Chain_Length(chain_array[i]) - 1, 1.0, 1.5);
    Locseg_Chain_Erase_E(onmask, chain_array[i],  1.0, 0,
	Locseg_Chain_Length(chain_array[i]) - 1, 1.0, 0.0);
  }
  stack = Stack_Remove_Small_Object(stack, NULL, 10, 26);
  //Struct_Element *se1 = Make_Disc_Se(1);
  //stack = Stack_Erode_Fast(stack, NULL, se1);
  Struct_Element *se2 = Make_Ball_Se(3);
  stack = Stack_Dilate(stack, NULL, se2);
  Stack_And(stack, onmask, stack);

  stack = Stack_Remove_Small_Object(stack, NULL, 10, 26);
  Write_Stack("../data/leftover.tif", stack);


  Object_3d_List *objs = Stack_Find_Object(stack, 1, 10);
  Object_3d_List *iterator = objs;
  char filepath[500];
  int soma_id = 0;
  //iterator = iterator->next->next->next;
  while (iterator != NULL) {
    Object_3d *obj = iterator->data;
    Swc_Tree *tree = Swc_Tree_From_Object_3d(obj);

    Swc_Tree_Node *tn = tree->root;
    int n1 = 0;
    while (tn != NULL)  {
      n1++;
      tn = tn->first_child;
    }
    int n2 = 0;
    if (n1 > 1) {
      tn = tree->root->first_child->next_sibling;
      while (tn != NULL)  {
	n2++;
	tn = tn->first_child;
      }
    }
    int n = n1+n2;

    int *skel;
    GUARDED_MALLOC_ARRAY(skel, n, int);
    Geo3d_Ball *ball;
    GUARDED_MALLOC_ARRAY(ball, n, Geo3d_Ball);
    tn = tree->root;
    int k = n1;
    while (tn != NULL)  {
      skel[--k] = Stack_Util_Offset(tn->node.x, tn->node.y, tn->node.z, 
	  Stack_Width(stack), Stack_Height(stack), Stack_Depth(stack));
      //Swc_Node_To_Geo3d_Ball(Swc_Tree_Node_Data(tn), ball + (--k));
      tn = tn->first_child;
    }
    k = n1;
    if (n1 > 1) {
      tn = tree->root->first_child->next_sibling;
      while (tn != NULL)  {
	//Swc_Node_To_Geo3d_Ball(Swc_Tree_Node_Data(tn), ball + (k++));
	skel[k++] = Stack_Util_Offset(tn->node.x, tn->node.y, tn->node.z, 
	    Stack_Width(stack), Stack_Height(stack), Stack_Depth(stack));
	tn = tn->first_child;
      }
    }
    /*
    Locseg_Chain *chain = 
      Locseg_Chain_From_Ball_Array(ball, n, NULL, 1.0, NULL);
      */
    Locseg_Chain *chain =
      Locseg_Chain_From_Skel(skel, n, 1.0, 1.0, stack, 1.0, NULL);
    sprintf(filepath, "%s/left%d.tb", "../data/test", soma_id++);
    Write_Locseg_Chain(filepath, chain);
    free(ball);

    Kill_Swc_Tree(tree);
    iterator = iterator->next;
  }
  Kill_Object_3d_List(objs);
#endif

#if 0
  int n;
  int file_number[5000];
  Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd("../data/benchmark/diadem/diadem_e4",
      "chain.*\\.tb", &n, file_number);
  Stack *stack = Read_Stack("../data/benchmark/diadem/diadem_e4.tif");
  Stack *signal = Copy_Stack(stack);
  double thre = 0.0;
  int i;
  for (i = 0; i < n; i++) {
    double min_signal = Locseg_Chain_Min_Seg_Signal(chain_array[i], stack,
	1.0);
    if (thre < min_signal) {
      thre = min_signal;
    }
  } 
  printf("Threshold: %g\n", thre);

  Stack_Threshold_Binarize(stack, iround(thre)-1);
  Stack *onmask = Copy_Stack(stack);
  for (i = 0; i < n; i++) {
    Locseg_Chain_Erase_E(stack, chain_array[i],  1.0, 0,
	Locseg_Chain_Length(chain_array[i]) - 1, 1.0, 1.5);
    Locseg_Chain_Erase_E(onmask, chain_array[i],  1.0, 0,
	Locseg_Chain_Length(chain_array[i]) - 1, 1.0, 0.0);
  }
  stack = Stack_Remove_Small_Object(stack, NULL, 10, 26);
  //Struct_Element *se1 = Make_Disc_Se(1);
  //stack = Stack_Erode_Fast(stack, NULL, se1);
  Struct_Element *se2 = Make_Ball_Se(3);
  stack = Stack_Dilate(stack, NULL, se2);
  Stack_And(stack, onmask, stack);

  stack = Stack_Remove_Small_Object(stack, NULL, 10, 26);
  Write_Stack("../data/leftover.tif", stack);


  Object_3d_List *objs = Stack_Find_Object(stack, 1, 10);
  Object_3d_List *iterator = objs;
  char filepath[500];
  int soma_id = 0;
  //iterator = iterator->next->next->next;
  while (iterator != NULL) {
    Object_3d *obj = iterator->data;
    Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(obj->size);
    int i;
    for (i = 0; i < obj->size; i++) {
      field->points[i][0] = obj->voxels[i][0];
      field->points[i][1] = obj->voxels[i][1];
      field->points[i][2] = obj->voxels[i][2];
    }
    Geo3d_Scalar_Field_Stack_Sampling(field, signal, 1.0, field->values);
    Local_Neuroseg *locseg = Local_Neuroseg_From_Field(field, NULL);
    Locseg_Chain *chain = New_Locseg_Chain();
    Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL); 
    sprintf(filepath, "%s/left%d.tb", "../data/test", soma_id++);
    Write_Locseg_Chain(filepath, chain);
    iterator = iterator->next;
  }
  
  Kill_Object_3d_List(objs);
#endif

#if 1
  int n;
  /*
  Locseg_Chain **chain_array =
    Dir_Locseg_Chain_Nd("../data/benchmark/fork2/fork.trace/traced",
      "chain.*\\.tb", &n, NULL);
      */
  /*
  Locseg_Chain **chain_array =
    Dir_Locseg_Chain_Nd("../data/rn003/cross_30_2.trace/traced",
      "chain.*\\.tb", &n, NULL);
      */
  int file_num[5000];
  Locseg_Chain **chain_array =
    Dir_Locseg_Chain_Nd("../data/diadem_c1", "chain.*\\.tb", &n, file_num);
  /*
  Locseg_Chain **chain_array =
    Dir_Locseg_Chain_Nd("/Users/zhaot/Work/diadem_final/tmpdata/diadem_c1", "chain.*\\.tb", &n, file_num);
    */
  printf("%d chains\n", n);
  
  int n2;
  Locseg_Chain **chain_array2 = 
    Locseg_Chain_Array_Break_Jump(chain_array, n, 15.0, &n2);
  printf("%d\n", n2);

  int i;
  for (i = 0; i < n2; i++) {
    //printf("index %d -> fnum %d -> conn %d\n", i, file_num[i], i * 2);
    Locseg_Chain_Correct_Ends(chain_array2[i]); 
  }
  Connection_Test_Workspace *ws = New_Connection_Test_Workspace();
  ws->dist_thre = 20.0;
  ws->resolution[2] = 3.4;

  Locseg_Chain_Array_Force_Merge(chain_array2, n2, ws);
  char filepath[500];
  system("rm ../data/tmp/*.tb");
  for (i = 0; i < n2; i++) {
    sprintf(filepath, "../data/tmp/chain%d.tb", i);
    if (Locseg_Chain_Is_Empty(chain_array2[i]) == FALSE) {
      Write_Locseg_Chain(filepath, chain_array2[i]); 
    }
  }
#endif

#if 0
  double res[3] = {1, 1, 1};
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/tmp/chain0.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/tmp/chain1.tb");
  Locseg_Chain *chain3 = Read_Locseg_Chain("../data/tmp/chain2.tb");
  Locseg_Chain *chain4 = Read_Locseg_Chain("../data/tmp/chain3.tb");
  double w = Locseg_Chain_Turning_Angle(chain1, chain2, 1, 1, res); 
  printf("%g\n", w);
  w = Locseg_Chain_Turning_Angle(chain1, chain4, 1, 0, res); 
  printf("%g\n", w);
  w = Locseg_Chain_Turning_Angle(chain3, chain2, 0, 1, res); 
  printf("%g\n", w);
  w = Locseg_Chain_Turning_Angle(chain3, chain4, 0, 0, res); 
  printf("%g\n", w);
#endif

#if 0
  int n;
  int file_num[5000];
  Locseg_Chain **chain_array =
    Dir_Locseg_Chain_Nd("../data/diadem_c1", "chain.*\\.tb", &n, file_num);
  int i;
  for (i = 0; i < n; i++) {
    if (file_num[i] == 27) {
      printf("index %d -> fnum %d\n", i, file_num[i]);
    }
  }

  int n2;
  Locseg_Chain **chain_array2 = 
    Locseg_Chain_Array_Break_Jump(chain_array, n, 20.0, &n2);
  printf("%d\n", n2);

  system("rm ../data/tmp/*.tb");
  char filepath[500];
  for (i = 0; i < n2; i++) {
    sprintf(filepath, "../data/tmp/chain%d.tb", i);
    if (Locseg_Chain_Is_Empty(chain_array2[i]) == FALSE) {
      Write_Locseg_Chain(filepath, chain_array2[i]); 
    }
  }
#endif

#if 0
  double res[3] = {1, 1, 3.4};
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/diadem_c1/chain237.tb");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/diadem_c1/chain299.tb");
  double w = Locseg_Chain_Turning_Angle(chain1, chain2, 1, 1, res); 
  printf("angle: %g\n", w);
#endif

  return 0;
}
