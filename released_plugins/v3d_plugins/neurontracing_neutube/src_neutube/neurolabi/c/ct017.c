/* @file ct017.c
 * @author Ting Zhao
 * @date 14-Mar-2009
 */

#include <utilities.h>
#include "tz_locseg_chain.h"
#include "tz_darray.h"
#include "tz_trace_utils.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-z <double>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

#if 0
  Locseg_Chain *chain1 = Read_Locseg_Chain("../data/ct017/test5/chain2.bn");
  Locseg_Chain *chain2 = Read_Locseg_Chain("../data/ct017/test5/chain1.bn");

  if (Is_Arg_Matched("-z")) {
    double z_scale = Get_Double_Arg("-z");
    Locseg_Chain_Scale_Z(chain1, z_scale);
    Locseg_Chain_Scale_Z(chain2, z_scale);
  }

  Neurocomp_Conn conn;
  Locseg_Chain_Connection_Test_P(chain1, chain2, &conn);
  Print_Neurocomp_Conn(&conn);

  double scale = 2.0;
  double offset = 5.0;

  Local_Neuroseg *locseg1;
  if (conn.info[0] == 0) {
    locseg1 = Locseg_Chain_Head_Seg(chain1);
    Local_Neuroseg_Stretch(locseg1, scale, offset, -1);
  } else {
    locseg1 = Locseg_Chain_Tail_Seg(chain1);
    Local_Neuroseg_Stretch(locseg1, scale, offset, 1);
  }

  Local_Neuroseg *locseg2 = Locseg_Chain_Peek_Seg_At(chain2, conn.info[1]);

  Local_Neuroseg_Stretch(locseg2, scale, offset, 0);

  printf("%g\n", Local_Neuroseg_Planar_Dist_L(locseg1, locseg2));

  FILE *fp = fopen("../data/ct017/test5/test.swc", "w");
  
  /* to avoid v3d bug */
  Local_Neuroseg *tmp_locseg = Copy_Local_Neuroseg(locseg1);
  tmp_locseg->seg.r1 = 0.1;
  Local_Neuroseg_Swc_Fprint(fp, tmp_locseg, 0, -1);

  Local_Neuroseg_Swc_Fprint(fp, locseg1, 2, -1);
  Local_Neuroseg_Swc_Fprint(fp, locseg2, 4, -1);
  
  fclose(fp);

  Locseg_Chain_Connection_Test(chain1, chain2, NULL, 1.0, &conn, NULL);
  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  double z_scale = 0.5375;

  Locseg_Chain *chain[11];
  char file_path[100];
  
  int i, j;
  for (i = 0; i < 11; i++) {
    sprintf(file_path, "../data/mouse_single_org/chain%d.bn", i);
    chain[i] = Read_Locseg_Chain(file_path);
    Locseg_Chain_Scale_Z(chain[i], z_scale);
  }

  Neurocomp_Conn conn;
  for (i = 0; i < 11; i++) {
    for (j = 0; j < 11; j++) {
      if (i != j) {
	if (Locseg_Chain_Connection_Test_P(chain[i], chain[j], &conn) < 5.0) {
	  printf("%d -> %d \n", i, j);
	  Print_Neurocomp_Conn(&conn);
	}
      }
    }
  }
#endif

#if 0
  double z_scale = 0.5375;

  int n = 278;
  Locseg_Chain *chain[n];
  char file_path[100];
  
  int i, j;
  for (i = 0; i < n; i++) {
    sprintf(file_path, "../data/mouse_single_org/chain%d.bn", i);
    chain[i] = Read_Locseg_Chain(file_path);
    Locseg_Chain_Scale_Z(chain[i], z_scale);
  }

  Locseg_Chain *hook = chain[19];

  Neurocomp_Conn conn;
  for (i = 0; i < n; i++) {
    if (i != 19) {
      if (Locseg_Chain_Connection_Test_P(hook, chain[i], &conn) < 5.0) {
	printf("19 -> %d \n", i);
	Print_Neurocomp_Conn(&conn);
      }
    }
  }
#endif
  
#if 0
  double z_scale = 0.32;
  Locseg_Chain *chain[17];
  char file_path[100];
  
  int i, j;
  for (i = 0; i < 17; i++) {
    sprintf(file_path, "../data/fly_neuron_n1/chain%d.bn", i);
    chain[i] = Read_Locseg_Chain(file_path);
    Locseg_Chain_Scale_Z(chain[i], z_scale);
  }

  Neurocomp_Conn conn;
  for (i = 0; i < 17; i++) {
    for (j = 0; j < 17; j++) {
      if (i != j) {
	if (Locseg_Chain_Connection_Test_P(chain[i], chain[j], &conn) < 5.0) {
	  printf("%d -> %d \n", i, j);
	  Print_Neurocomp_Conn(&conn);
	}
      }
    }
  }
#endif

#if 0
  double z_scale = 0.1400;
  //double z_scale = 1.0;

  int n = 12;
  Locseg_Chain *chain[n];
  char file_path[100];
  
  int i, j;
  for (i = 0; i < n; i++) {
    sprintf(file_path, "../data/ct017/test6/chain%d.bn", i);
    chain[i] = Read_Locseg_Chain(file_path);
    Locseg_Chain_Scale_Z(chain[i], z_scale);
  }

  Neurocomp_Conn conn;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (i != j) {
	if (Locseg_Chain_Connection_Test_P(chain[i], chain[j], &conn) < 10.0) {
	  printf("%d -> %d \n", i, j);
	  Print_Neurocomp_Conn(&conn);
	}
      }
    }
  }
#endif

#if 0
  //double z_scale = 0.1400;
  double z_scale = 1.20;

  int n = 5;
  Locseg_Chain *chain[n];
  char file_path[100];
  
  int i, j;
  for (i = 0; i < n; i++) {
    sprintf(file_path, "../data/ct017/test7/chain%d.bn", i);
    chain[i] = Read_Locseg_Chain(file_path);
    Locseg_Chain_Scale_Z(chain[i], z_scale);
  }

  Neurocomp_Conn conn;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (i != j) {
	if (Locseg_Chain_Connection_Test_P(chain[i], chain[j], &conn) < 10.0) {
	  printf("%d -> %d \n", i, j);
	  Print_Neurocomp_Conn(&conn);
	}
      }
    }
  }
#endif

#if 0
  double z_scale = 1.20;
  //double z_scale = 1.0;
  Stack *stack = Read_Stack("../data/lobster_neuron_org.tif");

  int n = 5;
  Locseg_Chain *chain[n];
  char file_path[100];
  
  int i, j;
  
  for (i = 0; i < n; i++) {
    sprintf(file_path, "../data/ct017/test7/chain%d.bn", i);
    chain[i] = Read_Locseg_Chain(file_path);
    //Locseg_Chain_Scale_Z(chain[i], z_scale);
  }
  
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->z_scale = z_scale;

  Neurocomp_Conn conn;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (i != j) {
	if (Locseg_Chain_Connection_Test(chain[i], chain[j], stack, 1.0,
					 &conn, ctw)) {
	  printf("%d -> %d \n", i, j);
	  Print_Neurocomp_Conn(&conn);
	}
      }
    }
  }
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/ct017/test6/chain7.bn");
  Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(chain);
  Print_Local_Neuroseg(locseg);

  Stack *stack = Read_Stack("../data/mouse_neuron_sp2.tif");
  double *profile = Local_Neuroseg_Height_Profile(locseg, stack, 1.0, 
						  11, STACK_FIT_CORRCOEF,
						  NULL);
  darray_write("../data/profile1.bn", profile, 11);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/ct017/test6/chain7.bn");
  Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(chain);
  Print_Local_Neuroseg(locseg);

  Stack *stack = Read_Stack("../data/mouse_neuron_sp2.tif");

  int n;
  double record[11];
  Locseg_Chain *chain2 = Local_Neuroseg_Push(locseg, stack, 
					     1.0, STACK_FIT_OUTER_SIGNAL, 
					     record, &n);

  Print_Locseg_Chain(chain2);

  Write_Locseg_Chain("../data/test.tb", chain2);

  return 1;

  Locseg_Chain *chain3 = Read_Locseg_Chain("../data/ct017/test6/chain6.bn");
  
  Locseg_Chain_Scale_Z(chain2, 0.14);
  Locseg_Chain_Scale_Z(chain3, 0.14);
  Neurocomp_Conn conn;
  Locseg_Chain_Connection_Test(chain2, chain3, NULL, 1.0, &conn, NULL);
  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/ct017/test6/chain9.bn");
  Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(chain);
  //Local_Neuroseg *locseg = Locseg_Chain_Head_Seg(chain);
  //Flip_Local_Neuroseg(locseg);

  Print_Local_Neuroseg(locseg);

  Stack *stack = Read_Stack("../data/mouse_neuron_sp2.tif");

  int n;
  double record[11];
  Locseg_Chain *chain2 = Local_Neuroseg_Push(locseg, stack, 
					     1.0, STACK_FIT_OUTER_SIGNAL, 
					     record, &n);

  Print_Locseg_Chain(chain2);

  Write_Locseg_Chain("../data/test.bn", chain2);

  return 1;

  Locseg_Chain *chain3 = Read_Locseg_Chain("../data/ct017/test6/chain9.bn");
  
  Locseg_Chain_Scale_Z(chain2, 0.14);
  Locseg_Chain_Scale_Z(chain3, 0.14);
  Neurocomp_Conn conn;
  Locseg_Chain_Connection_Test(chain3, chain2, stack, 1.0, &conn, NULL);
  Print_Neurocomp_Conn(&conn);
#endif

#if 0
  double z_scale = 0.1400;
  //double z_scale = 1.0;
  Stack *stack = Read_Stack("../data/mouse_neuron_sp2.tif");

  int n = 12;
  Locseg_Chain *chain[n];
  char file_path[100];
  
  int i, j;
  
  for (i = 0; i < n; i++) {
    sprintf(file_path, "../data/ct017/test6/chain%d.bn", i);
    chain[i] = Read_Locseg_Chain(file_path);
    //Locseg_Chain_Scale_Z(chain[i], z_scale);
  }
  
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->z_scale = 0.14;

  Neurocomp_Conn conn;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (i != j) {
	if (Locseg_Chain_Connection_Test(chain[i], chain[j], stack, 1.0,
					 &conn, ctw)) {
	  printf("%d -> %d \n", i, j);
	  Print_Neurocomp_Conn(&conn);
	}
      }
    }
  }
#endif

#if 0
  //double z_scale = 0.32;
  //double z_scale = 1.0;
  Stack *stack = Read_Stack("../data/fly_neuron_n1.tif");

  int n = 17;
  Locseg_Chain *chain[n];
  char file_path[100];
  
  int i, j;
  
  for (i = 0; i < n; i++) {
    sprintf(file_path, "../data/fly_neuron_n1/chain%d.tb", i);
    chain[i] = Read_Locseg_Chain(file_path);
    //Locseg_Chain_Scale_Z(chain[i], z_scale);
  }
  
  Connection_Test_Workspace *ctw = New_Connection_Test_Workspace();
  ctw->resolution[0] = 0.32;
  ctw->resolution[1] = 0.32;
  ctw->resolution[2] = 1.0;

  Neurocomp_Conn conn;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      if (i != j) {
	if (Locseg_Chain_Connection_Test(chain[i], chain[j], stack, 1.0,
					 &conn, ctw)) {
	  printf("%d -> %d \n", i, j);
	  Print_Neurocomp_Conn(&conn);
	}
      }
    }
  }
#endif

#if 0
  Locseg_Chain *chain = Read_Locseg_Chain("../data/fly_neuron_n1/chain3.bn");
  Local_Neuroseg *locseg = Locseg_Chain_Tail_Seg(chain);
  //Flip_Local_Neuroseg(locseg);
  Print_Local_Neuroseg(locseg);

  Stack *stack = Read_Stack("../data/fly_neuron_n1.tif");

  int n;
  double record[11];
  Locseg_Chain *chain2 = Local_Neuroseg_Push(locseg, stack, 
					     1.0, STACK_FIT_CORRCOEF, 
					     record, &n);

  Print_Locseg_Chain(chain2);

  Write_Locseg_Chain("../data/test.bn", chain2);

  return 1;

  Locseg_Chain *chain3 = Read_Locseg_Chain("../data/fly_neuron_n1/chain0.bn");
  
  Locseg_Chain_Scale_Z(chain2, 0.32);
  Locseg_Chain_Scale_Z(chain3, 0.32);
  Neurocomp_Conn conn;
  Locseg_Chain_Connection_Test(chain2, chain3, stack, 1.0, &conn, NULL);
  Print_Neurocomp_Conn(&conn);
#endif

  return 0;
}
