/**@file diadem_b1.c
 * @author Ting Zhao
 * @date 01-Apr-2010
 */

#include "tz_utilities.h"
#include "tz_string.h"
#include "tz_neuron_structure.h"
#include "tz_locseg_chain.h"
#include "tz_trace_defs.h"
#include "tz_trace_utils.h"
#include "tz_darray.h"
#include "tz_stack_attribute.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 0
  FILE *fp = GUARDED_FOPEN("/Users/zhaot/Data/diadem/diadem_b1/pos.txt", "r");
  char *line = NULL;
  String_Workspace *sw = New_String_Workspace();
  int array[8];
  int n;
  while ((line = Read_Line(fp, sw)) != NULL) {
    String_To_Integer_Array(line, array, &n);
    strsplit(line, ' ', 1);
    printf("/Users/zhaot/Data/diadem/diadem_b1/%s (%d, %d, 0) (%d, %d, %d)\n", 
	   line, array[7], array[6], array[4], array[3], array[2]);
  }
  fclose(fp);
#endif
  
#if 0
  int n;
  
  Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd("../data/diadem_b1_ds", 
						   "^chain.*\\.tb", &n, NULL);
  //n = 10;

  Stack *stack = Read_Stack("../data/diadem_b1_ds.tif");

  Stack *mask = Make_Stack(GREY, stack->width, stack->height, stack->depth);
  Zero_Stack(mask);
  Sp_Grow_Workspace *sgw = New_Sp_Grow_Workspace();
  sgw->size = Stack_Voxel_Number(stack);
  sgw->resolution[0] = 0.217391 * 2.0;
  sgw->resolution[1] = 0.217391 * 2.0;
  sgw->resolution[2] = 0.33 * 8.0;
  Sp_Grow_Workspace_Set_Mask(sgw, mask->array);
  sgw->wf = Stack_Voxel_Weight_S;
  sgw->fgratio = 0.3;
  Stack_Sp_Grow_Infer_Parameter(sgw, stack);

  Neuron_Structure *ns = 
    Locseg_Chain_Sp_Grow_Reconstruct(chain_array, n, stack, 1.0, sgw);
  
  Print_Neuron_Structure(ns);

  int i;
  int nchain = Neuron_Structure_Component_Number(ns);
  char filepath[100];
  for (i = 0; i < nchain; i++) {
    Locseg_Chain *chain = NEUROCOMP_LOCSEG_CHAIN(ns->comp + i);
    if (Locseg_Chain_Length(chain) > 0) {
      sprintf(filepath, "../data/diadem_b1_ds/newchain/chain%d.tb", i);
      Write_Locseg_Chain(filepath, chain);
    }
  }
  
  Graph_To_Dot_File(ns->graph, "../data/test.dot");

  //Neuron_Structure_To_Swc_File(ns, "../data/test.swc");
  Neuron_Structure *ns2 = 
    Neuron_Structure_Locseg_Chain_To_Circle_S(ns, 1.0, 1.0);
  //double root[3] = {31, 430, 0};
  //double root[3] = {1221, 449, 8.5};
  double *root = NULL;
  Swc_Tree *tree = Neuron_Structure_To_Swc_Tree_Circle_Z(ns2, 1.0, root);
  //Swc_Tree_Clean_Root(tree);
  Swc_Tree_Resort_Id(tree);
  Write_Swc_Tree("../data/test.swc", tree);  
#endif

#if 0
  int i;
  char filepath[100];
  for (i = 1; i <= 5; i++) {
    sprintf(filepath, "../data/diadem_b%d_root.txt", i);
    FILE *fp = GUARDED_FOPEN(filepath, "r");
    sprintf(filepath, "../data/diadem_b%d_ds_root.txt", i);
    FILE *fp2 = GUARDED_FOPEN(filepath, "w");
    sprintf(filepath, "../data/diadem_b%d_ds_root.swc", i);
    FILE *fp3 = GUARDED_FOPEN(filepath, "w");
    
    String_Workspace *sw = New_String_Workspace();
    char *line;
    int array[4];
    int n;
    int index = 1;
    while ((line = Read_Line(fp, sw)) != NULL) {
      String_To_Integer_Array(line, array, &n);
      if (n == 4) {
	fprintf(fp2, "%d %d %d\n", array[1] / 2, array[2] / 2, array[3] / 8);
	fprintf(fp3, "%d 2 %d %d %d 3 -1\n", index++, array[1] / 2, array[2] / 2, array[3] / 8);
      }
    }
    
    fclose(fp2);
    fclose(fp3);
    fclose(fp);
  }
#endif

#if 1 /* for final */
  int i;
  char filepath[100];
  for (i = 1; i <= 4; i++) {
    sprintf(filepath, "/DIADEM/shell/config_final/diadem_b%d_root.txt", i);
    FILE *fp = GUARDED_FOPEN(filepath, "r");
    sprintf(filepath, "/DIADEM/shell/config_final/diadem_b%d_ds_root.txt", i);
    FILE *fp2 = GUARDED_FOPEN(filepath, "w");
    sprintf(filepath, "/DIADEM/shell/config_final/diadem_b%d_ds_root.swc", i);
    FILE *fp3 = GUARDED_FOPEN(filepath, "w");
    
    String_Workspace *sw = New_String_Workspace();
    char *line;
    int array[4];
    int n;
    int index = 1;
    while ((line = Read_Line(fp, sw)) != NULL) {
      String_To_Integer_Array(line, array, &n);
      if (n == 3) {
	fprintf(fp2, "%d %d %d\n", array[0] / 2, array[1] / 2, array[2] / 8);
	fprintf(fp3, "%d 2 %d %d %d 3 -1\n", index++, array[0] / 2, array[1] / 2, array[2] / 8);
      }
    }
    
    fclose(fp2);
    fclose(fp3);
    fclose(fp);
  }
#endif
  return 0;
}
