/* fly_neuron_export.c
 *
 * 26-Mar-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include "tz_error.h"
#include "tz_neurochain.h"
#include "tz_neurotrace.h"

INIT_EXCEPTION_MAIN(e)

int main(int argc, char *argv[])
{
  const char *file_path = 
    "../data/fly_neuron/chain4.bn";

  Neurochain *chain = Read_Neurochain(file_path);
  
  FILE *fp = fopen("../data/export/chain.txt", "w");
  FILE *fp2 = fopen("../data/export/chain.dat", "w");
  fprintf(fp2, "r1 r2 height theta psi x y z\n");

  int n = 1;
  while (chain != NULL) {
    fprintf(fp, "Node: %d\n", n);
    if (chain->hist != NULL) {
      if (Bitmask_Get_Bit(chain->hist->mask, TRACE_HIST_DIRECTION_MASK) 
	  == TRUE) {
	if (chain->hist->direction == BACKWARD) {
	  Flip_Local_Neuroseg(&(chain->locseg));
	}
      }
    }
    Fprint_Local_Neuroseg(fp, &(chain->locseg));
\
    fprintf(fp2, "%g %g %g %g %g %g %g %g\n", chain->locseg.seg.r1, 
	    chain->locseg.seg.r2,
	    chain->locseg.seg.h, chain->locseg.seg.theta, chain->locseg.seg.psi,
	    chain->locseg.pos[0], chain->locseg.pos[1], chain->locseg.pos[2]);

    chain = chain->next;
    n++;
  } 

  fclose(fp);
  fclose(fp2);

  return 0;
}
