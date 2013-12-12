/**@file chainfeat.c
 * @brief >>chain feature calculation
 * @author Ting Zhao
 * @date 02-Apr-2009
 */

#include <utilities.h>
#include "tz_locseg_chain.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_string.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<tube:string> -o <string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  FILE *fp = fopen(Get_String_Arg("tube"), "r");
  
  String_Workspace *sw = New_String_Workspace();

  int j = 0;
  
  FILE *out = fopen(Get_String_Arg("-o"), "w");

  char *neuron;
  while ((neuron = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(neuron)) {
      continue;
    }

    j++;
    char data_dir[100];
    sprintf(data_dir, "../data/%s", neuron);
    
    int n = dir_fnum_p(data_dir, "^chain.*\\.tb");
    int mask[n];
    
    int i;
    for (i = 0; i < n; i++) {
      mask[i] = 0;
    }
    
    char filepath[100];
    sprintf(filepath, "../data/%s.tif", neuron);
    Stack *stack = Read_Stack(filepath);
  
    char *tmp = Read_Line(fp, sw);

    while (tmp != NULL) {
      if (Is_Space(tmp)) {
	tmp = Read_Line(fp, sw);
	continue;
      }
      int n2;
      int *ids = String_To_Integer_Array(tmp, NULL, &n2);
      iarray_print2(ids, n2, 1);
      
      for (i = 0; i < n2; i++) {
	mask[ids[i]] = 1;
      }
      
      free(ids);
      tmp = Read_Line(fp, sw);
      if (tmp != NULL) {
	if (tmp[0] == '#') {
	  tmp = NULL;
	}
      }
    }

    for (i = 0; i < n; i++) {
      sprintf(filepath, "%s/chain%d.tb", data_dir, i);
      printf("%s\n", filepath);
      Locseg_Chain *chain = Read_Locseg_Chain(filepath);
      fprintf(out, "%d %d %d %g %g %g %g\n", j, i, mask[i], 
	      Locseg_Chain_Geolen(chain),
	      Locseg_Chain_Average_Score(chain, stack, 1.0, STACK_FIT_CORRCOEF),
	      Locseg_Chain_Median_Score(chain, stack, 1.0, STACK_FIT_CORRCOEF),
	      Locseg_Chain_Average_Bottom_Radius(chain));
      Kill_Locseg_Chain(chain);
    }
  }

  fclose(fp);
  fclose(out);

  return 0;
}
