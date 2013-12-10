/* fly_neuron_chain_feat.c
 *
 * 09-Mar-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tz_error.h"
#include "tz_neurochain.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
  char neuron_name[100];
  if (argc == 1) {
    strcpy(neuron_name, "fly_neuron");
  } else {
    strcpy(neuron_name, argv[1]);
  }

  char file_path[100];
  sprintf(file_path, "../data/%s.tif", neuron_name);
  Stack *stack = Read_Stack(file_path);
  double z_scale = 1.0;

  sprintf(file_path, "../data/%s/chainlist.txt", neuron_name);
  char cmd[100];
  sprintf(cmd, "ls ../data/%s/*.bn > %s", neuron_name, file_path);
  system(cmd);

  
  FILE *fp = fopen(file_path, "r");

  while (!feof(fp)) {
    fscanf(fp, "%s", file_path);
    printf("%s\n", file_path);

    Neuroseg_Fit_Score fs;
    fs.n = 1;
    fs.options[0] = 1;
    double score = 0.0; 
    if (fexist(file_path)) {      
      Neurochain *chain = Read_Neurochain(file_path);
      int length = Neurochain_Length(chain, FORWARD);
      double thick = Neurochain_Thickness_Mean(chain);
      double thick_var = Neurochain_Thickness_Var(chain);
      double dthick = Neurochain_Thickness_Change_Mean(chain);
      double angle = Neurochain_Angle_Mean(chain);
      score = Neurochain_Score(chain, stack, z_scale, &fs);
      
      printf("length: %d; score: %g; thick: %g; thick var: %g\n; thick change: %g; angle: %g\n", 
	     length, score / length, thick, thick_var, dthick, angle);

      Free_Neurochain(chain);
    }
  }

  fclose(fp);

  Kill_Stack(stack);

  return 1;
}
