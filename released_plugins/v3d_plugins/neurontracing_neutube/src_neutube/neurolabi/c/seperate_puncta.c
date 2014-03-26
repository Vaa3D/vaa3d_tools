/**@file assign_puncta.c
 * @brief >> use distance to decide which tree puncta belong to, also mask
 * @author Linqing Feng
 * @date 21-July-2011
 */

#include <stdio.h>
#include <string.h>
#include "utilities.h"
#include "tz_string.h"
#include "tz_swc_tree.h"
#include "tz_workspace.h"
#include "tz_stack_graph.h"
#include "tz_stack_threshold.h"
#include "tz_stack_attribute.h"
#include "tz_stack_utils.h"
#include "tz_int_histogram.h"
#include "tz_graph.h"
#include "tz_math.h"
#include "tz_local_neuroseg.h"
#include "tz_locseg_chain_com.h"
#include "tz_locseg_chain.h"
#include "image_lib.h"
#include "tz_stack_stat.h"
#include "tz_swc_cell.h"
#include "tz_stack_neighborhood.h"
#include "tz_geo3d_ball.h"
#include "tz_apo.h"


int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> ...", "[-volsizethreshold <double>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int nswc = Get_Repeat_Count("input");
  char **bigoutpath = (char**) malloc(sizeof(char*) * nswc);
  FILE **bigfps = (FILE**) malloc(sizeof(FILE*) * nswc);
  char **smalloutpath = (char**) malloc(sizeof(char*) * nswc);
  FILE **smallfps = (FILE**) malloc(sizeof(FILE*) * nswc);
  
  int i,j;
  for (i=0; i<nswc; i++) {
    bigoutpath[i] = (char *) malloc(sizeof(char) * 500);
    sprintf(bigoutpath[i], "%s_big.apo", Get_String_Arg("input", i));
    bigfps[i] = Guarded_Fopen(bigoutpath[i], "w", "open output big puncta files");
    smalloutpath[i] = (char *) malloc(sizeof(char) * 500);
    sprintf(smalloutpath[i], "%s_small.apo", Get_String_Arg("input", i));
    smallfps[i] = Guarded_Fopen(smalloutpath[i], "w", "open output small puncta files");
  }

  double thre = Get_Double_Arg("-volsizethreshold");
  for (j=0; j<nswc; j++) {
    int npuncta;
    Apo_Node *punctas = Read_Apo_File(Get_String_Arg("input", j), &npuncta);
    for (i=0; i<npuncta; i++) {
      if (punctas[i].volsize >= thre) {
        Apo_Node_Fprint(bigfps[j], &(punctas[i]));
      } else {
        Apo_Node_Fprint(smallfps[j], &(punctas[i]));
      }
    }
    free(punctas);
  }

  for (i=0; i<nswc; i++) {
    fclose(bigfps[i]);
    fclose(smallfps[i]);
  }
  for (i=0; i<nswc; i++) {
    free(bigoutpath[i]);
    free(smalloutpath[i]);
  }
  return 0;
}

