/* testarraylist.c
 *
 * 15-Feb-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_int_arraylist.h"
#include "tz_iarray.h"
#include "tz_arrayqueue.h"

INIT_EXCEPTION_MAIN(e)


int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    /* New an arraylist */
    Int_Arraylist *a = New_Int_Arraylist();

    /* Add an element */
    Int_Arraylist_Add(a, 1);
    
    if ((a->length != 1) || (a->array[0] != 1)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }

    int value = Int_Arraylist_Take(a, 0);

    if ((a->length != 0) || (value != 1)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }

    Int_Arraylist_Set(a, 10, 5);
    if ((a->length != 11) || (a->array[10] != 5)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }

    int i;
    for (i = 0; i < a->length; i++) {
      Int_Arraylist_Set(a, i, i);
    }

    if ((a->length != 11) || (a->array[5] != 5)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }
    
    Int_Arraylist_Insert(a, 5, -1);
    if ((a->length != 12) || (a->array[5] != -1) || (a->array[4] != 4) ||
	(a->array[7] != 6)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }

    Int_Arraylist_Insert(a, 20, -2);
    if ((a->length != 21) || (a->array[20] != -2)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }

    Int_Arraylist_Swap(a, 1, 3);
    if ((a->array[1] != 3) || (a->array[3] != 1)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }
    
    Kill_Int_Arraylist(a);

    /* An arraylist can be used as a heap */
    a = New_Int_Arraylist();
    Int_Heap_Add(a, 1);
    if ((a->length != 2) || (a->array[1] != 1)) {
      PRINT_EXCEPTION("Bug?", "Unexpected array.");
      return 1;
    }

    Int_Heap_Add(a, 3);
    Int_Heap_Add(a, 5);
    Int_Heap_Add(a, 2);
    Int_Heap_Add(a, 1);

    if (Verify_Int_Heap(a) != 0) {
      PRINT_EXCEPTION("Bug?", "Not a heap.");
      return 1;
    }

    if (Int_Heap_Remove(a) != 1) {
      PRINT_EXCEPTION("Bug?", "Unexpected value.");
      return 1;
    }

    if (Verify_Int_Heap(a) != 0) {
      PRINT_EXCEPTION("Bug?", "Not a heap.");
      return 1;
    }

    if (a->length != 5) {
      PRINT_EXCEPTION("Bug?", "Unexpected size.");
      return 1;
    }

    Kill_Int_Arraylist(a);

    /* An associated heap */
    a = New_Int_Arraylist();
    double d[] = {2.0, 1.4, 10.0, 11, 11, 12.0, 3.0, 6.0, 4.0, 13};
    int checked[100];
    for (i = 0; i < 100; i++) {
      checked[i] = 0;
    }
  
    for (i = 0; i < sizeof(d)/sizeof(double) - 1; i++) {
      Int_Heap_Add_I(a, i + 1, d, checked);
    }

    if (Verify_Int_Heap_I(a, d) != 0) {
      PRINT_EXCEPTION("Bug?", "Not a heap.");
      return 1;
    }

    int index = Int_Heap_Remove_I(a, d, checked);
    
    if ((Verify_Int_Heap_I(a, d) != 0) || (checked[index] != 0)){
      PRINT_EXCEPTION("Bug?", "Not a heap.");
      return 1;
    }

    d[4] = 1.0;
    if (checked[a->array[Verify_Int_Heap_I(a, d)]] - 1 != 8){
      printf("%d\n", checked[a->array[Verify_Int_Heap_I(a, d)]]);
      PRINT_EXCEPTION("Bug?", "Unexpected heap.");
      return 1;
    }

    Int_Heap_Update_I(a, 4, d, checked);
    if ((Verify_Int_Heap_I(a, d) != 0) || (checked[4] == 0)){
      PRINT_EXCEPTION("Bug?", "Not a heap.");
      return 1;
    }

    Int_Heap_Remove_I_At(a, 4, d, checked);
    if ((Verify_Int_Heap_I(a, d) != 0) || (checked[4] != 0)){
      PRINT_EXCEPTION("Bug?", "Not a heap.");
      return 1;
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0 /* test Int_Heap_Sort */
  Int_Arraylist *a = Int_Arraylist_New(0, 0);
  int i;
  for (i = 0; i < 20; i++) {
    Int_Arraylist_Add(a, i);
  }

  //a->array[10] = 100;
  //a->array[2] = 100;

  Print_Int_Arraylist(a, "%d");

  Print_Int_Heap(a, "%d");

  Int_Heap_Sort(a);
  Print_Int_Arraylist(a, "%d");
#endif

#if 0 /* test array_binsearch */
  int x[] = {1, 2, 3, 4, 5, 7, 8, 9};
  printf("%d\n", iarray_binsearch(x, 8, 7));
#endif

#if 0 /* test array queue */
  Arrayqueue *aq = Make_Arrayqueue(100);
  Arrayqueue_Add_Last(aq, 1);
  Arrayqueue_Add_Last(aq, 2);
  Arrayqueue_Add_First(aq, 10);
  Arrayqueue_Add_Last(aq, 4);

  Arrayqueue_Add_First(aq, 20);

  Print_Arrayqueue(aq);
  
  printf("%d\n", Arrayqueue_Size(aq));

  int *index = iarray_malloc(Arrayqueue_Max(aq) + 1);
  int i;
  for (i = 0; i <= Arrayqueue_Max(aq); i++) {
    index[i] = -1;
  }

  Arrayqueue_To_Index(aq, index);
  iarray_print2(index, Arrayqueue_Max(aq) + 1, 1);
  
  Arrayqueue_Enqueue(aq, 11);
  Arrayqueue_Enqueue(aq, 12);
  while (Arrayqueue_Is_Empty(aq) == FALSE) {
    printf("%d ", Arrayqueue_Dequeue(aq));
  }
  printf("\n");
#endif

#if 0
  Int_Arraylist *a = Int_Arraylist_New(1, 0);
  double d[] = {2.0, 1.4, 10.0, 11, 11, 12.0, 3.0, 6.0, 4.0, 13};
  
  int i;
  for (i = 0; i < sizeof(d)/sizeof(double) - 1; i++) {
    Int_Heap_Add_I(a, i + 1, d);
  }

  Print_Int_Heap_I(a, d);

  while (a->length > 1) {
    printf("%g ", d[Int_Heap_Remove_I(a, d)]);
  }
  printf("\n");
#endif

#if 0
  int x[] = {1, 2, 3, 4, 5, 6, 7, 8};
  iarray_reverse(x, 8);
  iarray_print2(x, 8, 1);
#endif

#if 0 /* test array operation */
  Int_Arraylist *a = Int_Arraylist_New(1, 0);
  Int_Arraylist_Set(a, 0, 1);
  Int_Arraylist_Add(a, 2);
  Int_Arraylist_Insert(a, 1, 3);
  Int_Arraylist_Insert(a, 0, 4);
  Int_Arraylist_Insert(a, 2, 5);
  Int_Arraylist_Insert(a, 6, 6);
  Print_Int_Arraylist(a, "%d");

  int data = Int_Arraylist_Take(a, 0);
  printf("%d\n", data);
  Print_Int_Arraylist(a, "%d");
  
  Kill_Int_Arraylist(a);
#endif


#if 1
  double values[] = {3, 2, 1, 6, 9, 5, 4, 7, 8, 0};
  Int_Arraylist *h = Make_Int_Arraylist(0, 10);
  int checked[10];
  int i;
  for (i = 0; i < 10; i++) {
    Int_Heap_Add_I(h, i, values, checked);
  }
  Print_Int_Heap_I(h, values);
  Verify_Int_Heap_I(h, values);

  Int_Heap_Remove_I_At(h, 8, values, checked);

  Print_Int_Heap_I(h, values);
  Verify_Int_Heap_I(h, values);

  Kill_Int_Arraylist(h);
#endif

#if 0
  Verify_Int_Heap_I(NULL, NULL);
#endif

#if 0
  Int_Arraylist *h = Int_Arraylist_New(0, 0);
  Int_Arraylist_Insert(h, 0, 0);
  Int_Arraylist_Insert(h, 0, 1);
  Print_Int_Arraylist(h, "%d");
  Kill_Int_Arraylist(h);
#endif

  return 0;
}
