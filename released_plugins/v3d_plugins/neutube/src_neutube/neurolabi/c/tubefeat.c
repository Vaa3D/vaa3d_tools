/**@file tubefeat.c
 **@brief >> tube features for connection test
 * @author Ting Zhao
 * @date 29-Mar-2009
 */

#include <utilities.h>
#include "tz_locseg_chain.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_stack_attribute.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<neuron:string> ...", 
			 "-o <string> [-p]",NULL};

  Process_Arguments(argc, argv, Spec, 1);

  int nn = Get_Repeat_Count("neuron");
  
  int i, j;
  int m, n;

  char filepath[100];
  
  Stack *signal = NULL;
  Stack *mask = NULL;
  
  double record[16];
  int ntrial;

  FILE *feat_fp = fopen(Get_String_Arg("-o"), "w");

  for (i = 0; i < nn; i++) {    
    char *neuron = Get_String_Arg("neuron", i);

    if (Is_Arg_Matched("-p")) {
      sprintf(filepath, "../data/%s.tif", neuron);
      signal = Read_Stack(filepath);
      mask = Make_Stack(GREY, signal->width, signal->height, signal->depth);
      Zero_Stack(mask);
      sprintf(filepath, "../data/%s/traced.tif", neuron);
      Stack *traced = Read_Stack(filepath);
      Image_Array ima;
      ima.array = traced->array;
      int k;
      int nvoxel = Stack_Voxel_Number(mask);
      for (k = 0; k < nvoxel; k++) {
	if ((ima.arrayc[k][0] != ima.arrayc[k][1]) || 
	    (ima.arrayc[k][1] != ima.arrayc[k][2])) {
	  mask->array[k] = 1;
	}
      }
      Kill_Stack(traced);
      //Write_Stack("../data/test.tif", mask);
    }

    sprintf(filepath, "../data/%s_conn.txt", neuron);
    int *conn = iarray_load_matrix(filepath, NULL,
				   &m, &n);
    
    double res[3];
    sprintf(filepath, "../data/%s.res", neuron);
    FILE *fp = fopen(filepath, "r");
    darray_fscanf(fp, res, 3);
    fclose(fp);
    for (j = 0; j < n; j++) {
      sprintf(filepath, "../data/%s/chain%d.tb", neuron, conn[j*m]);
      Locseg_Chain *chain1 = Read_Locseg_Chain(filepath);
      sprintf(filepath, "../data/%s/chain%d.tb", neuron, conn[j*m+1]);
      Locseg_Chain *chain2 = Read_Locseg_Chain(filepath);

      Local_Neuroseg *locseg1 = NULL;
      if (conn[j*m+2] == 0) {
	locseg1 = Locseg_Chain_Head_Seg(chain1);
	Flip_Local_Neuroseg(locseg1);
      } else {
	locseg1 = Locseg_Chain_Tail_Seg(chain1);
      }

      Local_Neuroseg *locseg2 = NULL;
    
      if (m > 4) {
	if (conn[j*m+4] == 2) {
	  if (conn[j*m+3] == 0) {
	    locseg2 = Locseg_Chain_Head_Seg(chain2);
	  } else {
	    locseg2 = Locseg_Chain_Tail_Seg(chain2);
	  }
	}
      } 
    
      if (locseg2 == NULL) {
	locseg2 = Locseg_Chain_Peek_Seg_At(chain2, conn[j*m+3]);
      }

      int nf;
      double *feat = 
	Locseg_Conn_Feature(locseg1, locseg2, NULL, res, NULL, &nf);

      fprintf(feat_fp, "%d ", conn[j*m+m-1]);

      int k;
      for (k = 0; k < nf; k++) {
	fprintf(feat_fp, "%g ", feat[k]);
      }

      if (signal != NULL) {
	/*
	if (Locseg_Chain_Length(chain1) > 2) {
	  if (conn[j*m+2] == 0) {
	    locseg1 = Locseg_Chain_Peek_Seg_At(chain1, 0);
	    Flip_Local_Neuroseg(locseg1);
	  } else {
	    locseg1 = 
	      Locseg_Chain_Peek_Seg_At(chain1, 
				       Locseg_Chain_Length(chain1) - 2);
	  }
	}
	*/
	Locseg_Chain_Label_G(chain1, mask, 1.0, 0, Locseg_Chain_Length(chain1),
			     1.0, 0.0, 1, 0);

	sprintf(filepath, "../data/%s/pushed_chain_%d_%d_%d_%d.tb", neuron, 
		conn[j*m], conn[j*m+1], conn[j*m+2], conn[j*m+3]);

	Locseg_Chain *chain = Local_Neuroseg_Push(locseg1, signal, 1.0, mask, 
						  STACK_FIT_VALID_SIGNAL_RATIO,
						  record, &ntrial);
	printf("%s saved\n", filepath);

	Write_Locseg_Chain(filepath, chain);

	if (chain != NULL) {
	  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
	  for (k = 1; k < ntrial; k++) {
	    if (record[k] > 0.5) {
	      locseg1 = Locseg_Chain_Peek_Seg(chain);
	    } else {
	      break;
	    }
	    Locseg_Chain_Next(chain);
	  }
	}
	//Locseg_Conn_Feature(locseg1, locseg2, NULL, res, feat, &nf);

	//Locseg_Chain_Conn_Feature(chain2, locseg1, NULL, res, feat, &nf);

	Kill_Locseg_Chain(chain);
	Locseg_Chain_Label_G(chain1, mask, 1.0, 0, Locseg_Chain_Length(chain1),
			     1.0, 0.0, 0, 1);

	for (k = 0; k < nf; k++) {
	  fprintf(feat_fp, "%g ", feat[k]);
	}
      }

      fprintf(feat_fp, "%d %d", i, j);
      fprintf(feat_fp, "\n");
      Kill_Locseg_Chain(chain1);
      Kill_Locseg_Chain(chain2);
    }

    if (signal != NULL) {
      Kill_Stack(signal);
      signal = NULL;
      Kill_Stack(mask);
      mask = NULL;
    }
  }

  fclose(feat_fp);

  return 0;
}
