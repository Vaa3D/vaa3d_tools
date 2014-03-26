/**@file edtube.c
 * @brief >> edit tube files
 * @author Ting Zhao
 * @date 10-Oct-2009
 */

#include <utilities.h>
#include <string.h>
#include "tz_error.h"
#include "tz_locseg_chain.h"
#include "tz_utilities.h"
#include "tz_u8array.h"
#define FORCE_PROGRESS
#include "tz_interface.h"
#include "tz_stack_threshold.h"
#include "tz_math.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_math.h"
#include "tz_objdetect.h"
#include "tz_iarray.h"

#include "private/tz_main.c"

static int skip_argument(char *argv[])
{
  if (strcmp(argv[0], "-o")) {
    return 2;
  }

  if (strcmp(argv[0], "-xz_ratio")) {
    return 2;
  }

  if (strcmp(argv[0], "-xy_scale")) {
    return 2;
  }

  if (strcmp(argv[0], "-screen")) {
    return 2;
  }

  if (strcmp(argv[0], "-stack")) {
    return 2;
  }

  return 1;
}

static double locseg_chain_confidence(Locseg_Chain *chain, Stack *stack,
				      double z_scale)
{
  double c[3] = {1.1072, 5.3103, -2.0465};

  if (z_scale != 1.0) {
    c[0] = 7.0544;
    c[1] = 5.2073;
    c[2] = -12.8934;
  }

  double x1;
  if (z_scale == 1.0) {
    x1 = Locseg_Chain_Geolen(chain);
  } else {
    x1 = Locseg_Chain_Geolen_Z(chain, z_scale);
  }
  double x2 = Locseg_Chain_Average_Score(chain, stack, 1.0,
                                         STACK_FIT_CORRCOEF);

  return 1.0 / (1.0 + exp((x1 * c[0] + x2 * c[1] + x1 * x2 * c[2])));  
}

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.02") == 1) {
    return 0;
  }

  static char *Spec[] = {"-o <string> [-xz_ratio <double>] <input:string> ...",
			 "[-xy_scale <double>] [-screen <double>]",
			 "[-stack <string>] [-subtract] [-recover]",
			 NULL};

  int k;
  for (k = 0; k < argc; k++) {
    printf("%s ", argv[k]);
  }
  printf("\n");

  int new_argc = argc;
  if (argc >= 25) {
    new_argc = rearrange_arguments(argc, argv);
  }

  Process_Arguments(new_argc, argv, Spec, 1);

  int nfile = 0;

  if (new_argc != argc) {
    nfile = argc - new_argc + 1;
  } else {
    nfile = Get_Repeat_Count("input");  
  }
  
  char **filepath = (char**) malloc(sizeof(char*) * nfile);

  if (new_argc != argc) {
    for (k = 0; k < nfile; k++) {
      filepath[k] = argv[new_argc + k - 1];
    }
  } else {
    for (k = 0; k < nfile; k++) {
      filepath[k] = Get_String_Arg("input", k);
    }
  }

  if (Is_Arg_Matched("-recover")) {
    int n;
    int file_number[50000];
    Locseg_Chain **chain_array = Dir_Locseg_Chain_Nd(filepath[0],
	"chain.*\\.tb", &n, file_number);
    Stack *stack = Read_Stack(Get_String_Arg("-stack"));
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

    Stack_Threshold_Binarize(stack, iround(thre)-1);
    Stack *onmask = Copy_Stack(stack);
    for (i = 0; i < n; i++) {
      Locseg_Chain_Erase_E(stack, chain_array[i],  1.0, 0,
	  Locseg_Chain_Length(chain_array[i]) - 1, 1.0, 1.5);
      Locseg_Chain_Erase_E(onmask, chain_array[i],  1.0, 0,
	  Locseg_Chain_Length(chain_array[i]) - 1, 1.0, 0.0);
    }
    stack = Stack_Remove_Small_Object(stack, NULL, 10, 26);
    Struct_Element *se2 = Make_Ball_Se(3);
    stack = Stack_Dilate(stack, NULL, se2);
    Stack_And(stack, onmask, stack);

    stack = Stack_Remove_Small_Object(stack, NULL, 10, 26);

    Object_3d_List *objs = Stack_Find_Object(stack, 1, 10);
    Object_3d_List *iterator = objs;
    char tmp_filepath[500];
    int chain_id = iarray_max(file_number, n, NULL) + 1;
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
      sprintf(tmp_filepath, "%s/chain%d.tb", Get_String_Arg("-o"), chain_id++);
      Write_Locseg_Chain(tmp_filepath, chain);
      iterator = iterator->next;
    }
    Kill_Object_3d_List(objs);

    return 0;
  }

  if (Is_Arg_Matched("-subtract")) {
    if (nfile < 2) {
      fprintf(stderr, "There must be at least two inputs");
      return 1;
    }

    int n1;
    int n2;
    Locseg_Chain *chain1 = 
      Dir_Locseg_Chain_N(filepath[0], ".*\\.tb", &n1, NULL);

    if (n1 == 0) {
      fprintf(stderr, 
	      "The first input must be a directory containing .tb files");
      return 1;      
    }

    tz_uint8 *sub = u8array_calloc(n1);
    int i, j, k;
    for (k = 1; k < nfile; k++) {
      if (!fhasext(filepath[k], "swc")) {
	Locseg_Chain *chain2 = 
	  Dir_Locseg_Chain_N(filepath[k], ".*\\.tb", &n2, NULL);
       
	for (i = 0; i < n1; i++) {
	  if (sub[i] == FALSE) {
	    for (j = 0; j < n2; j++) {
	      if (Locseg_Chain_Hit_Ratio(chain1+i, chain2+j) > 0.5) {
		sub[i] = TRUE;
		break;
	      }
	    }
	  }
	}
	for (j = 0; j < n2; j++) {
	  Clean_Locseg_Chain(chain2+j);
	  free(chain2);
	}
      } else {
	Swc_Tree *tree = Read_Swc_Tree(filepath[k]);
	PROGRESS_BEGIN("Subtracting ...")
	for (i = 0; i < n1; i++) {
	  PROGRESS_STATUS((i * 100) / n1)
	  if (sub[i] == FALSE) {
	    if (Locseg_Chain_Hit_Ratio_Swc(chain1+i, tree) > 0.5) {
	      sub[i] = TRUE;
	    }
	  }
	  PROGRESS_REFRESH
	}
	Kill_Swc_Tree(tree);
      }
    }

    k = 0;
    char tmppath[MAX_PATH_LENGTH];
    for (i = 0; i < n1; i++) {
      if (sub[i] == FALSE) {
	sprintf(tmppath, "%s/chain%d.tb", Get_String_Arg("-o"), i);
	Write_Locseg_Chain(tmppath, chain1 + i);
      }
    }

    return 0;
  }

  char filename[100];
  char new_filepath[MAX_PATH_LENGTH];

  double z_scale= 1.0;
  if (Is_Arg_Matched("-xz_ratio")) {
    z_scale = Get_Double_Arg("-xz_ratio");
  }

  if (Is_Arg_Matched("-screen")) {
    Stack *stack = Read_Stack(Get_String_Arg("-stack"));
    for (k = 0; k < nfile; k++) {
      Locseg_Chain *chain = Read_Locseg_Chain(filepath[k]);
      double confidence = locseg_chain_confidence(chain, stack, z_scale);
      if (confidence >= Get_Double_Arg("-screen")) {
	fname(filepath[k], filename);
	fullpath_e(Get_String_Arg("-o"), filename, "tb", new_filepath);
	Write_Locseg_Chain(new_filepath, chain);
      }
    }
  } else {
    for (k = 0; k < nfile; k++) {
      Locseg_Chain *chain = Read_Locseg_Chain(filepath[k]);
      
      if (Is_Arg_Matched("-xy_scale")) {
	Locseg_Chain_Scale_XY(chain, Get_Double_Arg("-xy_scale"));
      }
      
      fname(filepath[k], filename);
      
      fullpath_e(Get_String_Arg("-o"), filename, "swc", new_filepath);
      
      FILE *fp = fopen(new_filepath, "w");
      Locseg_Chain_Swc_Fprint_Z(fp, chain, 2, 1, -1, DL_FORWARD, z_scale);
      fclose(fp);
      
      Kill_Locseg_Chain(chain);
    }
  }


  return 0;
}
