/**@file detection_score.c
 * @brief >> compare detection result with ground truth, give precision and recall
 * @author Linqing Feng
 * @date 21-July-2011
 */

#include <stdio.h>
#include "tz_utilities.h"
#include "tz_swc_cell.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> [-o <string>] [-gt <string>] [-range <int(5)>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int ngt, ntest;
  Swc_Node *gtpuncta = Read_Swc_File(Get_String_Arg("-gt"), &ngt);
  Swc_Node *testpuncta = Read_Swc_File(Get_String_Arg("input"), &ntest);

  FILE *fp;
  if (Is_Arg_Matched("-o")) {
    fp = GUARDED_FOPEN(Get_String_Arg("-o"), "w");
  }
  double ncorrect = 0;
  int i,j;
/*
  double range = 5;               
  if (Is_Arg_Matched("-range")) { 
    range = Get_Int_Arg("-range");
  }                               
*/
  double range = Get_Int_Arg("-range");
  for (i=0; i<ngt; i++) {
    BOOL isRecalled = FALSE;
    for (j=0; j<ntest; j++) {
      if (IS_IN_CLOSE_RANGE3(testpuncta[j].x, testpuncta[j].y, testpuncta[j].z,
                             gtpuncta[i].x-range, gtpuncta[i].x+range,
                             gtpuncta[i].y-range, gtpuncta[i].y+range,
                             gtpuncta[i].z-range, gtpuncta[i].z+range)) {
        isRecalled = TRUE;
        ncorrect++;
        break;
      }
    }
    if (!isRecalled) {
      Print_Swc_Node(gtpuncta+i);
      if (Is_Arg_Matched("-o")) {
        Swc_Node_Fprint(fp, gtpuncta+i);
      }
    }
  }
  if (Is_Arg_Matched("-o")) {
    fclose(fp);
  }

  printf("precision: %f\n", ncorrect/ntest);
  printf("recall: %f\n", ncorrect/ngt);


  return 0;
}
