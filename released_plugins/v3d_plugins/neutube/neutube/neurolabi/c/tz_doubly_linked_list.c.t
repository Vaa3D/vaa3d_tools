/* tz_linked_list_c.t
 *
 * 27-Nov-2007 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include "tz_error.h"
#include "utilities.h"
#include "tz_<1t>_doubly_linked_list.h"

INIT_EXCEPTION

#undef _DEBUG_

/* <1T>_Dlist_New(): New a <1T> list.
 *
 * Return: A <1T> list with length 1.
 */
<1T>_Dlist* <1T>_Dlist_New()
{
  <1T>_Dlist *new_list = (<1T>_Dlist *) Guarded_Malloc(sizeof(<1T>_Dlist), 
						     "<1T>_Dlist_New");
  
  new_list->data = 0x0;
  
  new_list->next = NULL;
  new_list->prev = NULL;

  return new_list;
}

void <1T>_Dlist_Remove_All(<1T>_Dlist *p)
{
  <1T>_Dlist *head = <1T>_Dlist_Head(p);
  while (head != NULL) {
    p = head;
    <3T=object>
    if (p->data != NULL) {
      Kill_<1T>(p->data);
    }
    </T>
	
    <3T=compact_pointer>
    if (p->data != NULL) {
      free(p->data);
    }
    </T>

    head = head->next;
#ifdef _DEBUG_
    printf("%p freed\n", p);
#endif
    free(p);
  }
}

void <1T>_Dlist_Set_Data(<1T>_Dlist *p, <2T> data)
{
  p->data = data;
}

<1T>_Dlist *<1T>_Dlist_Head(const <1T>_Dlist *p)
{
  if (p == NULL) {
    return NULL;
  }

  while (p->prev != NULL) {
    p = p->prev;
  }

  return (<1T>_Dlist *) p;
}

<1T>_Dlist *<1T>_Dlist_Tail(const <1T>_Dlist *p)
{
  if (p == NULL) {
    return NULL;
  }

  while (p->next != NULL) {
    p = p->next;
  }

  return (<1T>_Dlist *) p;
}

<1T>_Dlist* <1T>_Dlist_Next(const <1T>_Dlist *p, Dlist_Direction_e d)
{
  if (p != NULL) {
    if (d == DL_FORWARD) {
      return p->next;
    } else if (d == DL_BACKWARD) {
      return p->prev;
    } else {
      TZ_WARN(ERROR_DATA_VALUE);
      return NULL;
    }
  }

  return NULL;
}

int <1T>_Dlist_Length(const <1T>_Dlist *p, Dlist_Direction_e d)
{
  if (p == NULL) {
    return 0;
  }

  int length = 0;

  switch (d) {
  case DL_FORWARD:
    for(; p != NULL; p = p->next) {
      length++;
    }
    break;
  case DL_BACKWARD:
    for(; p != NULL; p = p->prev) {
      length++;
    }
    break;
  case DL_BOTHDIR:
    length = <1T>_Dlist_Length(p, DL_BACKWARD) + 
      <1T>_Dlist_Length(p->next, DL_FORWARD);
    break;
  default:
    TZ_WARN(ERROR_DATA_VALUE);
    length = -1;
  }

  return length;  
}

<1T>_Dlist* Locate_<1T>_Dlist(const <1T>_Dlist *p, int n)
{
  if (p == NULL) {
    return NULL;
  }

  int i;
  
  if (n > 0) {
    for (i = 0; i < n; i++) {
      p = p->next;
      if (p == NULL) {
	return NULL;
      }
    }
  } else if (n < 0){ /* going back */
    for (i = 0; i < -n;  i++) {
      p = p->prev;
      if (p == NULL) {
	return NULL;
      }
    }
  }

  return (<1T>_Dlist *)  p;
}

<1T>_Dlist* Append_<1T>_Dlist(<1T>_Dlist *p1, <1T>_Dlist *p2)
{
  if (p1 == NULL) {
    return p2;
  }

  if (p2 == NULL) {
    return p1;
  }

  <1T>_Dlist *head = <1T>_Dlist_Head(p1);

  p1 = <1T>_Dlist_Tail(p1);
  p2 = <1T>_Dlist_Head(p2);

  p1->next = p2;
  p2->prev = p1;

  return head;
}


<1T>_Dlist* <1T>_Dlist_Remove_End(<1T>_Dlist *p, Dlist_End_e e)
{
  ASSERT(p != NULL, "null pointer");

  <1T>_Dlist *end;

  switch (e) {
  case DL_HEAD:
    end = <1T>_Dlist_Head(p);
    p = end->next;
    if (p != NULL) {
      p->prev = NULL;
    }
    end->next = NULL;
    <1T>_Dlist_Remove_All(end);
    break;
  case DL_TAIL:
    end = <1T>_Dlist_Tail(p);
    p = end->prev;
    if (p != NULL) {
      p->next = NULL;
    }
    end->prev = NULL;
    <1T>_Dlist_Remove_All(end);
    break;
  default:
    TZ_WARN(ERROR_DATA_TYPE);
    return NULL;
  }

  return p;
}


<1T>_Dlist* <1T>_Dlist_Remove_Node(<1T>_Dlist *p)
{
  if (p == NULL) {
    return NULL;
  }

  <1T>_Dlist *head = <1T>_Dlist_Head(p);
  <1T>_Dlist *prev = p->prev;

  if (prev == NULL) {
    head = p->next;
  } else {
    prev->next = p->next;
    p->prev = NULL;
  }

  if (p->next != NULL) {
    p->next->prev = prev;
    p->next = NULL;
  }

  <1T>_Dlist_Remove_All(p);

  return head;
}

/* <1T>_Dlist_Add(): Add a node to the beginning of the list.
 * 
 * Args: p - pointer to the list;
 *       data - data node.
 *
 * Return: the old head.
 * 
 * Note: <data> (not its copy) will be added to the list, so be careful about
 *       freeing the object.
 */
<1T>_Dlist* <1T>_Dlist_Add(<1T>_Dlist *p, <2T> data, Dlist_End_e e)
{
  <1T>_Dlist *new_node = <1T>_Dlist_New();
  <1T>_Dlist_Set_Data(new_node, data);

  if (p == NULL) {
#ifdef _DEBUG_2
    printf("%p->%p added\n", new_node, data);
#endif    
    return new_node;
  }
  
  switch (e) {
  case DL_HEAD:
    p = <1T>_Dlist_Head(p);
    new_node->next = p;
    p->prev = new_node;
    p = new_node;
    break;
  case DL_TAIL:
    p = <1T>_Dlist_Tail(p);
    new_node->prev = p;
    p->next = new_node;
#ifdef _DEBUG_2
    printf("%p->%p added\n", new_node, data);
#endif
    p = new_node;
    break;
  default:
    TZ_WARN(ERROR_DATA_VALUE);
    <1T>_Dlist_Remove_All(new_node);
  }
      
  return p;
}

void <1T>_Dlist_Insert(<1T>_Dlist *p, <2T> data)
{
  if (p == NULL) {
    return;
  }

  <1T>_Dlist *new_node = <1T>_Dlist_New();
  <1T>_Dlist_Set_Data(new_node, data);
  
  <1T>_Dlist *prev = p->prev;
  p->prev = new_node;
  new_node->next = p;
  new_node->prev = prev;
  if (prev != NULL) {
    prev->next = new_node;
  }
}

/* <1T>_Dlist_Unlink(): Unlink the head of a list.
 *
 * Args: p - input list, which will be changed.
 *
 * Return: the unlinked data.
 */
<2T> <1T>_Dlist_Unlink(<1T>_Dlist **p)
{
  if (*p != NULL) {
    <1T>_Dlist *tmp_head = *p;
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

BOOL <1T>_Dlist_Is_Empty(const <1T>_Dlist *p)
{
  if (p == NULL) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/* <1T>_Dlist_To_Array(): Turn a list to an array.
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
<2T>* <1T>_Dlist_To_Array(const <1T>_Dlist *p, int step, <2T> *array, int *length)
{
  if (step == 0) {
    THROW(ERROR_DATA_VALUE);
  }

  int n = <1T>_Dlist_Length(p, DL_FORWARD);
  int array_length = (n - 1) * abs(step) + 1;

  if (array == NULL) {
    array = (<2T> *) Guarded_Malloc(sizeof(<2T>) * array_length, 
				    "<1T>_Dlist_To_Array");
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

/* Print_<1T>_Dlist(): Print <1T> list.
 *
 * Args: p - list to print.
 *
 * Return: void.
 *
 * Note: This function works only when Print_<1T> is available.
 */
void Print_<1T>_Dlist(const <1T>_Dlist *p)
{
  if (p == NULL) {
    printf("Null list.\n");
    return;
  }

  printf("Total length: %d\n", <1T>_Dlist_Length(p, DL_BOTHDIR));

  <3T=object,compact_pointer>
  while (p != NULL) {
    Print_<1T>(p->data);
    p = p->next;
  }
  </T>

}

/* Print_<1T>_Dlist_Compact(): Print <1T> list briefly.
 *
 * Args: p - list to print.
 *
 * Return: void.
 *
 * Note: This function works only when Print_<1T>_Info is available.
 */
void Print_<1T>_Dlist_Compact(const <1T>_Dlist *p)
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

void Print_<1T>_Dlist_F(const <1T>_Dlist *p, const char *format)
{
  if (p == NULL) {
    printf("Null list.\n");
    return;
  }

  printf("Total length: %d\n", <1T>_Dlist_Length(p, DL_FORWARD));
  while (p != NULL) {
    printf(format, p->data);
    printf(" ");
    p = p->next;
  }
  printf("\n");
}
