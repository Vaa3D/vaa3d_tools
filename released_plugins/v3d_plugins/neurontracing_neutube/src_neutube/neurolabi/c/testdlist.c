/* testdlist.c
 * 
 * 06-May-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <utilities.h>
#include "tz_iarray.h"
#include "tz_int_doubly_linked_list.h"
#include "tz_error.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-t")) {
    /* Example Test */

    /* New a int dlist. */
    Int_Dlist *list = Int_Dlist_New();

    /* The number of nodes should be 1 now. */
    if (Int_Dlist_Length(list, DL_BOTHDIR) != 1) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }

    /* Set the value of the current node to 1. */
    Int_Dlist_Set_Data(list, 1);

    /* Add a node to the tail. */
    Int_Dlist *tail = Int_Dlist_Add(list, 2, DL_TAIL);
    
    /* Add a node to the head. */
    Int_Dlist *head = Int_Dlist_Add(list, 3, DL_HEAD);

    if (Int_Dlist_Length(list, DL_BOTHDIR) != 3) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }

    if (Int_Dlist_Length(list, DL_FORWARD) != 2) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }

    if (Int_Dlist_Length(list, DL_BACKWARD) != 2) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }

    if (Int_Dlist_Length(head, DL_FORWARD) != 3) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }
    
    /* Insert a node to a list. */
    Int_Dlist_Insert(list, 5);

    if (Int_Dlist_Length(list, DL_BACKWARD) != 3) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }
    
    /* Get the head of a list. */
    if (Int_Dlist_Head(list) != head) {
      PRINT_EXCEPTION(":( Bug?", "Unmatched heads\n");
      return 1;      
    }

    /* Get the tail of a list. */
    if (Int_Dlist_Tail(list) != tail) {
      PRINT_EXCEPTION(":( Bug?", "Unmatched tails\n");
      return 1;      
    }

    /* Locate a certain node in the list. */
    if (Locate_Int_Dlist(list, 1) != Int_Dlist_Next(list, DL_FORWARD)) {
      PRINT_EXCEPTION(":( Bug?", "Unmatched nodes\n");
      return 1;      
    }

    if (Locate_Int_Dlist(list, -1) != Int_Dlist_Next(list, DL_BACKWARD)) {
      PRINT_EXCEPTION(":( Bug?", "Unmatched nodes\n");
      return 1;      
    }
    
    /* Append two lists. */
    Int_Dlist *list2 = Int_Dlist_New();
    Int_Dlist_Set_Data(list2, 4);
    
    head = Append_Int_Dlist(list, list2);

    if (Int_Dlist_Length(head, DL_FORWARD) != 5) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }

    /* Remove the head of a list */
    head = Int_Dlist_Remove_End(head, DL_HEAD);
    
    if (Int_Dlist_Length(head, DL_FORWARD) != 4) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }
    
    /* Remove the tail of a list */
    tail = Int_Dlist_Remove_End(head, DL_TAIL);
    
    if (Int_Dlist_Length(tail, DL_BACKWARD) != 3) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }
    
    /* Turn a list to an array. */
    int length;
    int *array = Int_Dlist_To_Array(head, 1, NULL, &length);

    if (length != 3) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected length\n");
      return 1;      
    }
    
    if ((array[0] != 5) || (array[1] != 1) || (array[2] != 2)) {
      PRINT_EXCEPTION(":( Bug?", "Unexpected array value\n");
      return 1;      
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 1
  Int_Dlist *list = Int_Dlist_New();

  int i;
  for (i = 1; i < 10; i++) {
    Int_Dlist_Add(list, i, DL_TAIL);
  }

  list = Int_Dlist_Remove_Node(list->next->next);

  Int_Dlist_Insert(list->next->next, 2);

  list = Int_Dlist_Head(list);
  printf("%d\n", Int_Dlist_Length(list, DL_FORWARD));
  Print_Int_Dlist(list);

  int array[100];
  int length;
  Int_Dlist_To_Array(list, 1, array, &length);
  iarray_print2(array, length, 1);

  Int_Dlist_Remove_All(list);
#endif

  return 0;
}
