/* tz_linked_list_c.t
 *
 * 27-Nov-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "tz_error.h"
#include "utilities.h"
#include "tz_<1t>_linked_list.h"

INIT_EXCEPTION

/* <1T>_List_New(): New a <1T> list.
 *
 * Return: A <1T> list with length 1.
 */
<1T>_List* <1T>_List_New()
{
  <1T>_List *new_list = (<1T>_List *) Guarded_Malloc(sizeof(<1T>_List), 
						     "<1T>_List_New");
  new_list->next = NULL;

  return new_list;
}

void <1T>_List_Set_Data(<1T>_List *p, <2T> data)
{
  p->data = data;
}

/* <1T>_List_Removeall(): Remove all nodes in a list.
 *
 * Args: p - a pointer to the list. It becomes NULL after calling this function.
 *
 * Return: void.
 */
void <1T>_List_Removeall(<1T>_List **p)
{
  if (p != NULL) {
    while (*p != NULL) {
      <1T>_List_Remove(p);
    }
  }
}

/* <1T>_List_Unlinkall(): Unlink all nodes in a list.
 *
 * Args: p - a pointer to the list. It becomes NULL after calling this function.
 *
 * Return: void.
 */
void <1T>_List_Unlinkall(<1T>_List **p)
{
  if (p != NULL) {
    while (*p != NULL) {
      <1T>_List_Unlink(p);
    }
  }
}

void Kill_<1T>_List(<1T>_List *p)
{
  <1T>_List_Removeall(&p);
}

/* <1T>_List_Add(): Add a node to the beginning of the list.
 * 
 * Args: p - pointer to the list;
 *       data - data node.
 *
 * Return: the old head.
 * 
 * Note: <data> (not its copy) will be added to the list, so be careful about
 *       freeing the object.
 */
<1T>_List* <1T>_List_Add(<1T>_List **p, <2T> data)
{
  <1T>_List *new_node = <1T>_List_New();
  new_node->next = *p;
  <1T>_List_Set_Data(new_node, data);
  *p = new_node;
  
  return (*p)->next;
}

/* <1T>_List_Add_Last(): Add a new node to the end of the list.
 *
 * Return: the last node of the new list.
 */
<1T>_List* <1T>_List_Add_Last(<1T>_List *p, <2T> data)
{
  p = <1T>_List_Last(p);
  <1T>_List *new_node = <1T>_List_New();
  <1T>_List_Set_Data(new_node, data);
  p->next = new_node;

  return new_node;
}

/* <1T>_List_Remove(): Remove the first node.
 *
 * Args: p - a pointer to the list. It becomes the new list after calling this
 *           function.
 *
 * Return: void.
 */
void <1T>_List_Remove(<1T>_List **p)
{
  if (*p != NULL) {
    <1T>_List *tmp_head = *p;
    *p = tmp_head->next;
    <3T=object>
    Kill_<1T>(tmp_head->data);
    </T>
    <3T=compact_pointer>
    free(tmp_head->data);
    </T>
    free(tmp_head);
  }
}

void <1T>_List_Remove_Last(<1T>_List *p)
{
  if (p == NULL) {
    return;
  }

  while (p->next != NULL) {
    p = <1T>_List_Next(p);
  }

  <1T>_List_Remove(&p);
}

/* <1T>_List_Unlink(): Unlink the head of a list.
 *
 * Args: p - input list, which will be changed.
 *
 * Return: the unlinked data.
 */
<2T> <1T>_List_Unlink(<1T>_List **p)
{
  if (*p != NULL) {
    <1T>_List *tmp_head = *p;
    *p = tmp_head->next;

    <2T> data = tmp_head->data;
    free(tmp_head);
    
    return data;
  }

  <3T=object,compact_pointer>
  return NULL;
  </t>
  <3T=basic>
  return 0;
  </t>
}

/* <1T>_List_Next(): Get next node.
 *
 * Return: the next node of p.
 */
<1T>_List* <1T>_List_Next(<1T>_List *p)
{
  return p->next;
}

/* <1T>_List_Last(): Get the last node.
 *
 * Return: the last node of <p>.
 */
<1T>_List* <1T>_List_Last(<1T>_List *p)
{
  <1T>_List* prev = NULL;

  while (p != NULL) {
    prev = p;
    p = <1T>_List_Next(p);
  }

  return prev;
}

BOOL <1T>_List_Is_Empty(const <1T>_List *p)
{
  if (p == NULL) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/* <1T>_List_Length(): Length of the list.
 * 
 * Args: p - input list.
 *
 * Return: the length of the list.
 */
int <1T>_List_Length(const <1T>_List *p)
{
  int n = 0;
  while (p != NULL) {
    n++;
    p = p->next;
  }

  return n;
}

/* <1T>_List_To_Array(): Turn a list to an array.
 *
 * Args: p - input list;
 *       array - returned array. If it is NULL, a new array will be created,
 *               otherwise it must be large enough to hold the result.
 *       step - The offset between an element and its right neighbor. If it is
 *              negative, the array has a reversed order.
 * Return: the resulted array.
 *
 * Note: The caller is responsible for freeing the returned array.
 */
<2T>* <1T>_List_To_Array(const <1T>_List *p, int step, <2T> *array, int *length)
{
  if (step == 0) {
    THROW(ERROR_DATA_VALUE);
  }

  int n = <1T>_List_Length(p);
  int array_length = (n - 1) * abs(step) + 1;

  if (array == NULL) {
    array = (<2T> *) Guarded_Malloc(sizeof(<2T>) * array_length, 
				    "<1T>_List_To_Array");
  }

  int begin;
  int end;
  
  if (step > 0) {
    begin = 0;
    end = array_length - 1;
  } else {
    begin = array_length - 1;
    end = 0;
  }
  
  int i;
  for (i = begin; p != NULL; i += step, p = p->next) {
    array[i] = p->data;
  }

  if (length != NULL) {
    *length = array_length;
  }

  return array;
}

/* Print_<1T>_List(): Print <1T> list.
 *
 * Args: p - list to print.
 *
 * Return: void.
 *
 * Note: This function works only when Print_<1T> is available.
 */
void Print_<1T>_List(const <1T>_List *p)
{
  if (p == NULL) {
    printf("Null list.\n");
    return;
  }

  printf("Total length: %d\n", <1T>_List_Length(p));

  <3T=object,compact_pointer>
  while (p != NULL) {
    Print_<1T>(p->data);
    p = p->next;
  }
  </T>

}

/* Print_<1T>_List_Compact(): Print <1T> list briefly.
 *
 * Args: p - list to print.
 *
 * Return: void.
 *
 * Note: This function works only when Print_<1T>_Info is available.
 */
void Print_<1T>_List_Compact(const <1T>_List *p)
{
  if (p == NULL) {
    printf("Null list.\n");
    return;
  }

  <3T=object>
  int i = 1;
  while (p != NULL) {
    printf("Item %d: ", i);
    Print_<1T>_Info(p->data);
    i++;
    p = p->next;
  }
  </T>
}

void Print_<1T>_List_F(const <1T>_List *p, const char *format)
{
  if (p == NULL) {
    printf("Null list.\n");
    return;
  }

  printf("Total length: %d\n", <1T>_List_Length(p));
  while (p != NULL) {
    printf(format, p->data);
    printf(" ");
    p = p->next;
  }
  printf("\n");
}

/*
 * The following routines implement a queue based on a list.
 */

/* <1T>_Queue_En(): Add an item to the end of the queue.
 */
<1T>_List* <1T>_Queue_En(<1T>_List **p, <2T> data)
{
  if (*p == NULL) {
    *p = <1T>_List_New();
    <1T>_List_Set_Data(*p, data);
    return *p;
  }

  return <1T>_List_Add_Last(*p, data);
}

/* <1T>_Queue_De(): Removes the item from the front of the queue.
 */
<2T> <1T>_Queue_De(<1T>_List **p)
{
  return <1T>_List_Unlink(p);
}

/* <1T>_Queue_Peek(): Get the iterm from the front of the queue without removing
 *                    it.
 */
<2T> <1T>_Queue_Peek(const <1T>_List *p)
{
  return p->data;
}


/*
 * The following routines implement a stack based on a list.
 */
void <1T>_Stack_Push(<1T>_List **p, <2T> data)
{
  if (*p == NULL) {
    *p = <1T>_List_New();
    <1T>_List_Set_Data(*p, data);
  } else {
    <1T>_List_Add(p, data);  
  }
}

<2T> <1T>_Stack_Pop(<1T>_List **p)
{
  return <1T>_List_Unlink(p);
}

<2T> <1T>_Stack_Peek(const <1T>_List *p)
{
  return p->data;
}


