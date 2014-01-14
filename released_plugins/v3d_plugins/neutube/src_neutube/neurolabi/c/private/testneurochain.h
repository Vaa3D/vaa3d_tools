/* @file testneurochain.h
 * @author Ting Zhao
 * @date 15-Nov-2009
 */

#ifndef _TESTNEUROCHAIN_H_
#define _TESTNEUROCHAIN_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

const static double eps = 1e-5;

static int test_locseg_chain_flip(Locseg_Chain *chain, Locseg_Chain *chain2)
{
  if (Locseg_Chain_Length(chain) != Locseg_Chain_Length(chain2)) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;      
  }
  
  Locseg_Chain_Iterator_Start(chain, DL_TAIL);
  Locseg_Chain_Iterator_Start(chain2, DL_HEAD);
    
  Locseg_Node *node1 = NULL;
  Locseg_Node *node2 = NULL;

  while ((node1 = Locseg_Chain_Prev(chain)) != NULL) {
    node2 = Locseg_Chain_Next(chain2);
    if ((Trace_Record_Direction(node1->tr) == DL_FORWARD) && 
	((Trace_Record_Direction(node2->tr) != DL_BACKWARD))) {
      PRINT_EXCEPTION("Bug?", "Unexpected direction.");
      return 1;
    }
    if ((Trace_Record_Direction(node1->tr) == DL_BACKWARD) && 
	((Trace_Record_Direction(node2->tr) != DL_FORWARD))) {
      PRINT_EXCEPTION("Bug?", "Unexpected direction.");
      return 1;
    }
    if ((Trace_Record_Direction(node2->tr) == DL_FORWARD) && 
	((Trace_Record_Direction(node1->tr) != DL_BACKWARD))) {
      PRINT_EXCEPTION("Bug?", "Unexpected direction.");
      return 1;
    }
    if ((Trace_Record_Direction(node2->tr) == DL_BACKWARD) && 
	((Trace_Record_Direction(node1->tr) != DL_FORWARD))) {
      PRINT_EXCEPTION("Bug?", "Unexpected direction.");
      return 1;
    }
    if ((Trace_Record_Direction(node1->tr) == DL_BOTHDIR) && 
	((Trace_Record_Direction(node2->tr) != DL_BOTHDIR))) {
      PRINT_EXCEPTION("Bug?", "Unexpected direction.");
      return 1;
    }
    if ((Trace_Record_Direction(node1->tr) == DL_BOTHDIR) && 
	((Trace_Record_Direction(node2->tr) != DL_BOTHDIR))) {
      PRINT_EXCEPTION("Bug?", "Unexpected direction.");
      return 1;
    }
    if ((Trace_Record_Direction(node2->tr) == DL_BOTHDIR) && 
	((Trace_Record_Direction(node1->tr) != DL_BOTHDIR))) {
      PRINT_EXCEPTION("Bug?", "Unexpected direction.");
      return 1;
    }
    if ((Trace_Record_Fix_Point(node1->tr) >= 0.0) ||
	(Trace_Record_Fix_Point(node2->tr) >= 0.0)){
      if (fabs(Trace_Record_Fix_Point(node1->tr) + 
	       Trace_Record_Fix_Point(node2->tr) - 1.0) > eps) {
	PRINT_EXCEPTION("Bug?", "Inconsistent fix point.");
	return 1;
      }
    }
  }

  Locseg_Chain_Knot_Array *ka1 = Locseg_Chain_To_Knot_Array(chain, NULL);
  Locseg_Chain_Knot_Array *ka2 = Locseg_Chain_To_Knot_Array(chain2, NULL);
  
  if (Locseg_Chain_Knot_Array_Length(ka1) != 
      Locseg_Chain_Knot_Array_Length(ka2)) {
    PRINT_EXCEPTION("Bug?", "Inconsistent length.");
    return 1;
  }
  
  int i;
  int n = Locseg_Chain_Knot_Array_Length(ka1);
  for (i = 0; i < n; i++) {
    double pos1[3];
    double pos2[3];
    Locseg_Chain_Knot_Pos(ka1, i, pos1);
    Locseg_Chain_Knot_Pos(ka2, n - i -1, pos2);
    if (Coordinate_3d_Distance(pos1, pos2) > eps) {
      PRINT_EXCEPTION("Bug?", "Inconsistent point.");
      return 1;
    }
  }

  Kill_Locseg_Chain_Knot_Array(ka1);
  Kill_Locseg_Chain_Knot_Array(ka2);

  return 0;
}

static int test_locseg_chain_iterator(Locseg_Chain *chain)
{
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  if (chain->iterator->prev != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_TAIL);
  if (chain->iterator->next != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }
      
  if (old_iter->prev  != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, 0);
  if (chain->iterator->prev != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, 1);
  if (chain->iterator->next != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }      

  Locseg_Chain_Iterator_Locate(chain, -1);
  if (chain->iterator->prev != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, 2);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, -2);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }
      
  if (Locseg_Chain_Next(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Next(NULL) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Peek(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Peek(NULL) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Prev(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Prev(NULL) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Next_D(chain, DL_HEAD) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Next_D(chain, DL_UNDEFINED) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Head_Seg(NULL) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Tail_Seg(NULL) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }  

  return 0;
}

static int test_locseg_chain_empty_iterator(Locseg_Chain *chain)
{
  if (Locseg_Chain_Is_Empty(chain) == FALSE) {
    PRINT_EXCEPTION("Bug?", "Non-empty chain.");
    return 1;
  }

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Node_Dlist *old_iter = Locseg_Chain_Iterator_Start(chain, DL_TAIL);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }
      
  if (old_iter  != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, 0);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, 1);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }      

  Locseg_Chain_Iterator_Locate(chain, -1);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, 2);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }

  Locseg_Chain_Iterator_Locate(chain, -2);
  if (chain->iterator != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected iterator.");
    return 1;	
  }
      
  if (Locseg_Chain_Next(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Peek(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Prev(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Next_D(chain, DL_HEAD) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (Locseg_Chain_Next_D(chain, DL_UNDEFINED) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  return 0;
}

static int test_locseg_chain_intensity_profile()
{
  Stack *stack = Make_Stack(GREY, 200, 200, 20);

  Zero_Stack(stack);

  Locseg_Chain *chain = New_Locseg_Chain();
  if (Locseg_Chain_Intensity_Profile(chain, stack, 1.0, NULL) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected profile.");
    return 1;
  }

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Set_Local_Neuroseg(locseg, 1.0, 0.0, NEUROSEG_DEFAULT_H, -TZ_PI_2, 0.0, 0.0,
     0.0, 1.0, 50.0, 10.0, 10.0);
  Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);

  int i;
  for (i = 0; i < 5; i++) {
    locseg = Next_Local_Neuroseg(locseg, NULL, 0.8);
    Locseg_Chain_Add(chain, locseg, NULL, DL_TAIL);
  }
  
  double pos[3];
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  int k = 1;
  while ((locseg = Locseg_Chain_Next_Seg(chain)) != NULL) {
    Local_Neuroseg_Center(locseg, pos);
    Set_Stack_Pixel(stack, iround(pos[0]), iround(pos[1]), iround(pos[2]), 
	0, k++);
  }

  double *profile = Locseg_Chain_Intensity_Profile(chain, stack, 1.0, NULL);

  for (i = 1; i < 5; i++) {
    if (iround(profile[i] / profile[0]) != i+1) {
      PRINT_EXCEPTION("Bug?", "Unexpected profile.");
      return 1;
    }
  }

  return 0;
}

static int unit_test()
{
  Locseg_Chain *chain2 = New_Locseg_Chain();
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  Locseg_Chain_Add(chain2, locseg, NULL, DL_TAIL);
  Locseg_Chain_Add(chain2, Next_Local_Neuroseg(locseg, NULL, 0.5), NULL,
		   DL_TAIL);
  Locseg_Chain *chain = Copy_Locseg_Chain(chain2);

  if (Locseg_Chain_Is_Identical(chain, chain2) == FALSE) {
    PRINT_EXCEPTION("Bug?", "Unequal chains.");
    return 1;	
  }

  Kill_Locseg_Chain(NULL);
      
  if (Locseg_Chain_Length(NULL) != 0) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;
  }

  if (Locseg_Chain_Is_Empty(NULL) == FALSE) {
    PRINT_EXCEPTION("Bug?", "Non-empty chain.");
    return 1;
  }

  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;
  }

  if (test_locseg_chain_iterator(chain)) {
    PRINT_EXCEPTION("Bug?", "Iteration failed.");
    return 1;
  } 

  Clean_Locseg_Chain(chain2);

  if (test_locseg_chain_empty_iterator(chain2)) {
    PRINT_EXCEPTION("Bug?", "Iteration failed.");
    return 1;
  } 

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Node *node = Locseg_Chain_Next(chain);
  if (node != chain->iterator->prev->data) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  if (node->locseg != Locseg_Chain_Head_Seg(chain)) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  Locseg_Chain_Iterator_Start(chain, DL_TAIL);
  node = Locseg_Chain_Prev(chain);
  if (node != chain->iterator->next->data) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }
      
  if (node->locseg != Locseg_Chain_Tail_Seg(chain)) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }
      
  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  node = Locseg_Chain_Next_D(chain, DL_HEAD);
  if (node != chain->iterator->prev->data) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  Locseg_Chain_Iterator_Start(chain, DL_TAIL);
  node = Locseg_Chain_Next_D(chain, DL_TAIL);
  if (node != chain->iterator->next->data) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  Locseg_Chain_Cat(chain, NULL);
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;	
  }

  Locseg_Chain_Cat(chain2, NULL);
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;	
  }

  locseg = Locseg_Chain_Tail_Seg(chain);
  Locseg_Chain_Add(chain2, Next_Local_Neuroseg(locseg, NULL, 0.5), NULL, 
		   DL_HEAD);

  Locseg_Chain *chain3 = Copy_Locseg_Chain(chain);
  Locseg_Chain_Cat(chain, chain2);
  if (Locseg_Chain_Length(chain) != 3) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;	
  }
      
  Locseg_Chain_Remove_End(chain, DL_TAIL);
  if (Locseg_Chain_Is_Identical(chain, chain3) == FALSE) {
    PRINT_EXCEPTION("Bug?", "Unequal chains.");
    return 1;	
  }

  Locseg_Chain_Remove_End(chain, DL_TAIL);
  Locseg_Chain_Remove_End(chain, DL_TAIL);

  if (Locseg_Chain_Is_Empty(chain) == FALSE) {
    PRINT_EXCEPTION("Bug?", "Non-empty chain.");
    return 1;	
  }
      
  Locseg_Chain_Remove_End(chain, DL_TAIL);
  if (Locseg_Chain_Is_Empty(chain) == FALSE) {
    PRINT_EXCEPTION("Bug?", "Non-empty chain.");
    return 1;	
  }

  Locseg_Chain_Remove_Current(chain);
      
  Kill_Locseg_Chain(chain);

  chain = Copy_Locseg_Chain(chain3);
  chain->iterator = NULL;
  Locseg_Chain_Remove_Current(chain);
      
  if (Locseg_Chain_Is_Identical(chain, chain3) == FALSE) {
    PRINT_EXCEPTION("Bug?", "Unequal chains.");
    return 1;	
  }

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  Locseg_Chain_Remove_Current(chain);
  if (Locseg_Chain_Length(chain) != 1) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;	
  }
  Locseg_Chain_Remove_Current(chain);
  if (Locseg_Chain_Length(chain) != 0) {
    PRINT_EXCEPTION("Bug?", "Unexpected length.");
    return 1;	
  }

  if (Locseg_Chain_Take_Current(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  chain = Copy_Locseg_Chain(chain3);
  chain->iterator = NULL;

  if (Locseg_Chain_Take_Current(chain) != NULL) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }

  Locseg_Chain_Iterator_Start(chain, DL_HEAD);
  node = Locseg_Chain_Peek(chain);
  if (node != Locseg_Chain_Take_Current(chain)) {
    PRINT_EXCEPTION("Bug?", "Unexpected node.");
    return 1;
  }
      
  Kill_Locseg_Node(node);
      
  int chain_number;
      
  Locseg_Chain *chain_array = 
    Dir_Locseg_Chain_N("../data/benchmark/"
		       "mouse_neuron_single/chain", 
		       "^chain[0-9][0-9]*\\.txt",
		       &chain_number, NULL);
  if (chain_number != 0) {
    printf("%d\n", chain_number);
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;	
  }
      
  chain_array = 
    Dir_Locseg_Chain_N("../data/benchmark/"
		       "mouse_neuron_single/chain", 
		       "^chain[0-9][0-9]*\\.tb",
		       &chain_number, NULL);
  if (chain_number != 162) {
    printf("%d\n", chain_number);
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;	
  }

  chain = 
    Read_Locseg_Chain("../data/benchmark/"
		      "mouse_neuron_single/chain/chain0.tb");
  chain2 = Copy_Locseg_Chain(chain);
  Locseg_Chain_Flip(chain2);
  if (test_locseg_chain_flip(chain, chain2) != 0) {
    PRINT_EXCEPTION("Bug?", "Flip error.");
    return 1;
  }

  Clean_Locseg_Chain(chain);
  Clean_Locseg_Chain(chain2);

  locseg = New_Local_Neuroseg();
  Trace_Record *tr = New_Trace_Record();
  Trace_Record_Set_Direction(tr, DL_FORWARD);
      
  Locseg_Chain_Add(chain, locseg, Copy_Trace_Record(tr), DL_TAIL);
      
  locseg = Next_Local_Neuroseg(locseg, NULL, 0.5);
  Locseg_Chain_Add(chain, locseg, Copy_Trace_Record(tr), DL_TAIL);

  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;	
  }

  locseg = Next_Local_Neuroseg(locseg, NULL, 0.6);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
      
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

  if (Locseg_Chain_Length(chain) != 3) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }

  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);      
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_TAIL);

  if (Locseg_Chain_Length(chain) != 3) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }

  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Merge(chain, chain2, DL_HEAD, DL_TAIL);

  if (Locseg_Chain_Length(chain) != 3) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }

  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Merge(chain, chain2, DL_HEAD, DL_HEAD);

  if (Locseg_Chain_Length(chain) != 3) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }

  Locseg_Chain_Remove_End(chain, DL_TAIL);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Next_Local_Neuroseg(locseg, NULL, 0.5), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

  if (Locseg_Chain_Length(chain) != 4) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }

  Locseg_Chain_Remove_End(chain, DL_TAIL);
  Locseg_Chain_Remove_End(chain, DL_TAIL);
      
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Next_Local_Neuroseg(locseg, NULL, 0.2), 
		   Copy_Trace_Record(tr), DL_TAIL);
      
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

  if (Locseg_Chain_Length(chain) != 3) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }

  Locseg_Chain_Remove_End(chain, DL_TAIL);
  Local_Neuroseg *locseg2 = Locseg_Chain_Head_Seg(chain);
  locseg2 = Next_Local_Neuroseg(locseg2, NULL, 0.2);
  Neuroseg_Set_Model_Height(&(locseg2->seg), 20.0);
  Locseg_Chain_Add(chain2, locseg2, 
		   Copy_Trace_Record(tr), DL_TAIL);

  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }      
      
  if (Neuroseg_Model_Height(&(Locseg_Chain_Tail_Seg(chain)->seg)) != 20.0) {
    PRINT_EXCEPTION("Bug?", "Unexpected segment.");
    return 1;
  }
      
  if (Trace_Record_Direction(Locseg_Chain_Tail(chain)->tr) != DL_BOTHDIR) {
    PRINT_EXCEPTION("Bug?", "Unexpected direction.");
    return 1;	
  }

  Locseg_Chain_Remove_End(chain, DL_TAIL);
  locseg = Locseg_Chain_Head_Seg(chain);
  locseg2 = Next_Local_Neuroseg(locseg, NULL, 1.5);
  Neuroseg_Set_Model_Height(&(locseg2->seg), 20.0);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg2), 
		   Copy_Trace_Record(tr), DL_TAIL);
      
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }      
      
  if (Trace_Record_Direction(Locseg_Chain_Tail(chain)->tr) != DL_FORWARD) {
    PRINT_EXCEPTION("Bug?", "Unexpected direction.");
    return 1;	
  }

  Locseg_Chain_Remove_End(chain, DL_TAIL);
  Trace_Record_Set_Direction(tr, DL_BOTHDIR);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg2), 
		   Copy_Trace_Record(tr), DL_TAIL);
   
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain number.");
    return 1;
  }   
      
  Local_Neuroseg *locseg3 = 
    Copy_Local_Neuroseg(Locseg_Chain_Tail_Seg(chain));
      
  Locseg_Chain_Remove_End(chain, DL_TAIL);
  Flip_Local_Neuroseg(locseg3);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg3), 
		   Copy_Trace_Record(tr), DL_TAIL);
   
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

  Local_Neuroseg *locseg4 = 
    Copy_Local_Neuroseg(Locseg_Chain_Tail_Seg(chain));
      
  if (Compare_Float(Neuroseg_Angle_Between(&(locseg3->seg), 
					   &(locseg4->seg)), TZ_PI, eps)
      != 0) {
    printf("%g\n", Neuroseg_Angle_Between(&(locseg3->seg), 
					  &(locseg4->seg)));
    PRINT_EXCEPTION("Bug?", "Unexpected seg orientation.");
    return 1;
  }

  Clean_Locseg_Chain(chain);
  locseg = New_Local_Neuroseg();
  locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.5);
  locseg3 = Next_Local_Neuroseg(locseg2, NULL, 0.5);
      
  Locseg_Chain_Add(chain, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg2), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg3), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);
  locseg4 = 
    Copy_Local_Neuroseg(Locseg_Chain_Head_Seg(chain));

  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain length.");
    return 1;
  } 
     
  if (Compare_Float(Neuroseg_Angle_Between(&(locseg->seg), 
					   &(locseg4->seg)), 0.0, eps)
      != 0) {
    PRINT_EXCEPTION("Bug?", "Unexpected seg orientation.");
    return 1;
  }

  Clean_Locseg_Chain(chain);
  locseg = New_Local_Neuroseg();
  locseg2 = Next_Local_Neuroseg(locseg, NULL, 0.5);
  locseg3 = Next_Local_Neuroseg(locseg2, NULL, 0.5);
      
  Flip_Local_Neuroseg(locseg);
  Locseg_Chain_Add(chain, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg2), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg3), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);
  locseg4 = 
    Copy_Local_Neuroseg(Locseg_Chain_Head_Seg(chain));
      
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain length.");
    return 1;
  } 
     
  if (Compare_Float(Neuroseg_Angle_Between(&(locseg->seg), 
					   &(locseg4->seg)), TZ_PI, eps)
      != 0) {
    PRINT_EXCEPTION("Bug?", "Unexpected seg orientation.");
    return 1;
  }

  Clean_Locseg_Chain(chain);
  locseg = New_Local_Neuroseg();
  locseg2 = Next_Local_Neuroseg(locseg, NULL, 1.5);
  Flip_Local_Neuroseg(locseg2);
      
  Locseg_Chain_Add(chain, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg2), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

  locseg3 = Locseg_Chain_Head_Seg(chain);
  locseg4 = Locseg_Chain_Tail_Seg(chain);
      
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain length.");
    return 1;
  } 
     
  if (fabs(Neuroseg_Angle_Between(&(locseg3->seg), 
				  &(locseg4->seg))) > eps) {
    printf("%g\n", Neuroseg_Angle_Between(&(locseg3->seg), 
					  &(locseg4->seg)));
    PRINT_EXCEPTION("Bug?", "Unexpected seg orientation.");
    return 1;
  }

  Clean_Locseg_Chain(chain);
  locseg = New_Local_Neuroseg();
  locseg2 = Next_Local_Neuroseg(locseg, NULL, 1.5);
  Flip_Local_Neuroseg(locseg);
      
  Locseg_Chain_Add(chain, Copy_Local_Neuroseg(locseg), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Add(chain2, Copy_Local_Neuroseg(locseg2), 
		   Copy_Trace_Record(tr), DL_TAIL);
  Locseg_Chain_Merge(chain, chain2, DL_TAIL, DL_HEAD);

  locseg3 = Locseg_Chain_Head_Seg(chain);
  locseg4 = Locseg_Chain_Tail_Seg(chain);
      
  if (Locseg_Chain_Length(chain) != 2) {
    PRINT_EXCEPTION("Bug?", "Unexpected chain length.");
    return 1;
  } 
     
  if (fabs(Neuroseg_Angle_Between(&(locseg3->seg), 
				  &(locseg4->seg))) > eps) {
    PRINT_EXCEPTION("Bug?", "Unexpected seg orientation.");
    return 1;
  }

  if (test_locseg_chain_intensity_profile()) {
    PRINT_EXCEPTION("Bug?", "Unexpected intensity profile.");
    return 1;
  }
  return 0;
}

__END_DECLS

#endif
