/* tz_arraylist.c.t
 *
 * 15-Feb-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_<1t>_arraylist.h"

INIT_EXCEPTION

const static int Growth_Coeff = 2; /* growth factor */

/* <1t>_arraylist_grow(): Grow an arraylist.
 *
 * Args: a - the arraylist to grow.
 *
 * Return: void.
 */
static void <1t>_arraylist_grow(<1T>_Arraylist *a)
{
  if (a->capacity == 0) {
    a->capacity = Growth_Coeff;
    a->array = (<2T> *) Guarded_Malloc(sizeof(<2T>) * a->capacity, 
				       "<1t>_arraylist_grow");
  } else {
    int new_capacity = Growth_Coeff * a->capacity;
  
    <2T> *new_array = (<2T> *) Guarded_Calloc(new_capacity, sizeof(<2T>),
					      "<1t>_arraylist_grow");
    memcpy(new_array, a->array, sizeof(<2T>) * a->capacity);
    free(a->array);
    a->array = new_array;
    a->capacity = new_capacity;
  }
}

/* Print_<1T>_Arraylist(): Prints an array list.
 * 
 * Args: a - the arraylist to print.
 *       format - printing format. This is mainly for priliminary data elements.
 *
 * Return: void.
 */
void Print_<1T>_Arraylist(const <1T>_Arraylist *a<3T=basic>, const char *format</T>)
{
  int i;
  printf("%d elements: ", a->length);
  for  (i = 0; i < a->length; i++) {
<3T=object,struct>
    Print_<2T>(a->array + i);
</T>
<3T=basic>
    printf(format, a->array[i]);
    printf(" ");
</T>
  }
  
  printf("\n");
}

<1T>_Arraylist* New_<1T>_Arraylist()
{
  return <1T>_Arraylist_New(0, 1);
}

/* <1T>_Arraylist_New(): New an arraylist.
 * 
 * Args: length - length of the arraylist;
 *       margin - margin of the arraylist. This means the capacity of the
 *                arraylist will be <length> + <margin>.
 * 
 * Return: a new arraylist.
 */
<1T>_Arraylist* <1T>_Arraylist_New(int length, int margin)
{
  if ((length < 0) || margin < 0) {
    TZ_WARN(ERROR_DATA_VALUE);
    return NULL;
  }

  <1T>_Arraylist *a = (<1T>_Arraylist *) Guarded_Malloc(sizeof(<1T>_Arraylist), 
							"<1T>_Arraylist_New");
  a->length = length;
  a->capacity = length + margin;
  if (a->capacity > 0) {
    a->array = (<2T> *) Guarded_Malloc(sizeof(<2T>) * a->capacity, 
				       "<1T>_Arraylist_New");
  } else {
    a->array = NULL;
  }

  return a;
}

/* <1T>_Arraylist_Free(): Free an array list.
 *
 * Args: a - the array list to free.
 *
 * Return: void.
 */
void <1T>_Arraylist_Free(<1T>_Arraylist *a)
{
  free(a->array);
  free(a);
}

<1T>_Arraylist* Make_<1T>_Arraylist(int length, int margin)
{
  if ((length < 0) || margin < 0) {
    TZ_WARN(ERROR_DATA_VALUE);
    return NULL;
  }

  <1T>_Arraylist *a = (<1T>_Arraylist *) Guarded_Malloc(sizeof(<1T>_Arraylist), 
							"<1T>_Arraylist_New");
  a->length = length;
  a->capacity = length + margin;
  if (a->capacity > 0) {
    a->array = (<2T> *) Guarded_Malloc(sizeof(<2T>) * a->capacity, 
				       "<1T>_Arraylist_New");
  } else {
    a->array = NULL;
  }

  return a;
}

void Kill_<1T>_Arraylist(<1T>_Arraylist *a)
{
  <3t=object>
  int i;
  for (i = 0; i < a->length; i++) {
    Clean_<1T>(a->array + i);
  }
  </t>
  <3t=compact_pointer>
  int i;
  for (i = 0; i < a->length; i++) {
    free(a->array[i]);
  }
  </t>
  free(a->array);
  free(a);
}

<1T>_Arraylist* <1T>_Arraylist_Wrap(<2T> *array, int n)
{
  if (array == NULL) {
    return NULL;
  }

  <1T>_Arraylist *a = (<1T>_Arraylist *) Guarded_Malloc(sizeof(<1T>_Arraylist), 
							"<1T>_Arraylist_New");
  a->length = n;
  a->capacity = n;
  a->array = array;

  return a;
}

<2T> <1T>_Arraylist_At(const <1T>_Arraylist *a, int index)
{
  return a->array[index];
}

<2T> <1T>_Arraylist_Last(const <1T>_Arraylist *a)
{
  return a->array[a->length - 1];
}

<2T>* <1T>_Arraylist_At_R(const <1T>_Arraylist *a, int index)
{
  return a->array + index;
}

/* <1T>_Arraylist_Add(): Add an element to an array list.
 *
 * Args: <a> - the arraylist to manipulate;
 *       <item> - the element to add.
 *
 * Return: void.
 */
void <1T>_Arraylist_Add(<1T>_Arraylist *a, <2T> item)
{
  a->length++;

  if (a->length >= a->capacity) {
    <1t>_arraylist_grow(a);
  }

  a->array[a->length - 1] = item;
}

void <1T>_Arraylist_Insert(<1T>_Arraylist *a, int index, <2T> item)
{
  if (index < a->length) {
    a->length++;
    if (a->length >= a->capacity) {
      <1t>_arraylist_grow(a);
    }

    memmove(a->array + index + 1, a->array + index, 
	    sizeof(<2T>) * (a->length - index - 1));
  }

  <1T>_Arraylist_Set(a, index, item);
}

<2T> <1T>_Arraylist_Take(<1T>_Arraylist *a, int index)
{
  <2T> data = a->array[index];
  a->length--;
  memmove(a->array + index, a->array + index + 1, 
	  sizeof(<2T>) * (a->length - index));

  return data;
}

<2T> <1T>_Arraylist_Take_Last(<1T>_Arraylist *a)
{
  <2T> data = a->array[a->length - 1];
  a->length--;

  return data;
}

void <1T>_Arraylist_Set_Length(<1T>_Arraylist *a, int length)
{
  TZ_ASSERT(length >= 0, "Negative length");

  a->length = length;

  while (a->length > a->capacity) {
    <1t>_arraylist_grow(a);
  }
}

void <1T>_Arraylist_Set(<1T>_Arraylist *a, int index, <2T> item)
{
  if (index >= a->length) {
    a->length = index + 1;
  }

  while (a->length > a->capacity) {
    <1t>_arraylist_grow(a);
  }
  <2T=int,double,unipointer_t>
  int i;
  for (i = a->length; i < a->capacity; ++i) {
    a->array[i] = 0;
  }
  </t>

  a->array[index] = item;
}

/* <1T>_Arraylist_Swap(): Swap two elements
 * 
 * Args: a - the arraylist to manipulate;
 *       idx1 - index of the first element;
 *       idx2 - index of the second element.
 *
 * Return: void.
 */
void <1T>_Arraylist_Swap(<1T>_Arraylist *a, int idx1, int idx2)
{
  if ((idx1 < 0) || (idx1 >= a->length) || (idx2 < 0) || (idx2 >= a->length)) {
    TZ_WARN(ERROR_DATA_VALUE);
    return;
  }
  
  <2T> tmp;
  tmp = a->array[idx1];
  a->array[idx1] = a->array[idx2];
  a->array[idx2] = tmp;
}

<3T=basic>
/* <1T>_Heap_Depth(): Get the depth of a heap.
 *
 * Args: h - the heap.
 *
 * Return: the depth of the heap. It returns 0 if h is NULL.
 */
int <1T>_Heap_Depth(const <1T>_Arraylist *h)
{
  int depth = 0;
  int length = h->length - 1;
  while (length > 0) {
    length >>= 1;
    depth++;
  }

  return depth;
}

/* <1T>_Heap_Node_At(): Get a certain node from a heap.
 *
 * Args: h - input heap;
 *       depth - depth of the node;
 *       rank - rank of the node at its depth. It's counted from left to right
 *              and the first sibling has rank 0.
 *
 * Return: the value of the node.
 */
<2T> <1T>_Heap_Node_At(const <1T>_Arraylist *h, int depth, int rank)
{
  int index = 1;
  index <<= (depth - 1);
  index += rank;

  if (index >= h->length) {
    THROW(ERROR_DATA_VALUE;);
  }

  return h->array[index];
}

/* Print_<1T>_Heap(): Print a heap.
 *
 * Args: h - the heap to print.
 *
 * Return: void.
 */
void Print_<1T>_Heap(const <1T>_Arraylist *h<3T=basic>, const char *format</T>)
{
  printf("A heap with depth %d:\n", <1T>_Heap_Depth(h));
<3T=basic>
  int i;
  for (i = 1; i < h->length; i++) {
    printf(format, h->array[i]);
    printf(" ");
    if ((i & (i + 1)) == 0) { /* i = 2^k - 1*/
      printf("\n");
    }
  }
  printf("\n");
</T>
}

/* <1T>_Heap_Parent(): Get the parent of a node.
 * 
 * Args: h - the heap to query;
 *       child_index - index of the child node.
 *
 * Return: the index of the parent node. It returns 0 when there is no valid
 *         parent.
 */
int <1T>_Heap_Parent(const <1T>_Arraylist *h, int child_index)
{
  if ((child_index <= 1) || (child_index >= h->length)) {
    return 0;
  } else {
    return child_index / 2;
  }
}

/* <1T>_Heap_Child(): Get the left child of a node.
 * 
 * Args: h - the heap to query;
 *       parent_index - index of the parent node.
 *
 * Return: the index of the parent node. It returns 0 when there is no valid
 *         left child.
 */
int <1T>_Heap_Leftchild(const <1T>_Arraylist *h, int parent_index)
{
  if ((parent_index < 1) || (parent_index >= h->length)) {
    return 0;
  }

  int child_index = parent_index * 2;
  if (child_index >= h->length) {
    return 0;
  }

  return child_index;
}

/* <1T>_Heap_Small_Child(): Get the index of the smaller child of a node.
 *
 * Args: h - the heap to query;
 *       parent_index - index of the node whose children are examined.
 *
 * Return: index of the smaller child.
 */
int <1T>_Heap_Small_Child(const <1T>_Arraylist *h, int parent_index)
{
  int index = <1T>_Heap_Leftchild(h, parent_index);
  if ((index > 0) && (index + 1 < h->length)) {
    if (h->array[index] > h->array[index + 1]) {
      index++;
    }
  }

  return index;
}

/* <1T>_Heap_Add(): Add a node to a heap.
 *
 * Args: h - the heap to manipulate;
 *       item - the added node.
 *
 * Return: void.
 */
void <1T>_Heap_Add(<1T>_Arraylist *h, <2T> item)
{
  if (h->length == 0) { 
    /* ensure the preserved element */
    <1T>_Arraylist_Add(h, 0);
  }

  <1T>_Arraylist_Add(h, item);

  int child_index = h->length - 1;
  int parent_index = <1T>_Heap_Parent(h, child_index);
  while (parent_index > 0) {
    if (h->array[child_index] < h->array[parent_index]) {
      <1T>_Arraylist_Swap(h, child_index, parent_index);
      child_index = parent_index;
      parent_index = <1T>_Heap_Parent(h, child_index);
    } else {
      break;
    }
  }
}

/* <1T>_Heap_Remove(): Remove the root of a heap.
 *
 * Args: h - the heap to manipulate.
 *
 * Return: the value of the removed node.
 */
<2T> <1T>_Heap_Remove(<1T>_Arraylist *h)
{
  <2T> root = h->array[1];
  h->array[1] = h->array[h->length - 1];
  h->length--;
  
  int parent_index = 1;
  int child_index = <1T>_Heap_Small_Child(h, parent_index);
  while (child_index > 0) {
    if (h->array[parent_index] > h->array[child_index]) {
      <1T>_Arraylist_Swap(h, parent_index, child_index);
      parent_index = child_index;
      child_index = <1T>_Heap_Small_Child(h, parent_index);
    } else {
      break;
    }
  }

  return root;
}

#define PARENT_INDEX(index) (index / 2)

/*
static int parent_index(int index)
{
  return index / 2;
}
*/

BOOL Verify_<1T>_Heap(<1T>_Arraylist *h)
{
  int i;
  for (i = h->length - 1; i > 1; i--) {
    int pid = PARENT_INDEX(i);
    if (h->array[i] < h->array[pid]) {
      //printf("%g -> %g\n", value[h->array[i]], value[h->array[pid]]);
      return i;
    }
  }
  
  return 0;
}

<2T=int>
int <1T>_Heap_Small_Child_I(const <1T>_Arraylist *h, int parent_index, double *value)
{
  int index = <1T>_Heap_Leftchild(h, parent_index);
  if (index > 0) {
    if (index + 1 < h->length) {
      if (value[h->array[index]] > value[h->array[index + 1]]) {
	index++;
      }
    }
  }

  return index;
}

void <1T>_Heap_Add_I(<1T>_Arraylist *h, <2T> item, double *value, int *checked)
{
  if (h->length == 0) { 
    /* ensure the preserved element */
    <1T>_Arraylist_Add(h, 0);
  }

  <1T>_Arraylist_Add(h, item);

  checked[item] = h->length;
  int child_index = checked[item] - 1;
  int parent_index = <1T>_Heap_Parent(h, child_index);
  while (parent_index > 0) {
    if (value[h->array[child_index]] < value[h->array[parent_index]]) {
      <1T>_Arraylist_Swap(h, child_index, parent_index);
      checked[h->array[parent_index]] = parent_index + 1;
      checked[h->array[child_index]] = child_index + 1;
      child_index = parent_index;
      parent_index = <1T>_Heap_Parent(h, child_index);
    } else {
      break;
    }
  }
}

void <1T>_Heap_Update_I(<1T>_Arraylist *h, <2T> item, double *value, 
			int *checked)
{
  int child_index = checked[item] - 1;
  int parent_index = <1T>_Heap_Parent(h, child_index);
  while (parent_index > 0) {
    if (value[h->array[child_index]] < value[h->array[parent_index]]) {
      <1T>_Arraylist_Swap(h, child_index, parent_index);
      checked[h->array[parent_index]] = parent_index + 1;
      checked[h->array[child_index]] = child_index + 1;
      child_index = parent_index;
      parent_index = <1T>_Heap_Parent(h, child_index);
    } else {
      break;
    }
  }
}

<2T> <1T>_Heap_Remove_I(<1T>_Arraylist *h, double *value, int *checked)
{
  <2T> root = h->array[1];
  checked[root] = 0;
  h->array[1] = h->array[h->length - 1];
  h->length--;
  
  int parent_index = 1;
  checked[h->array[parent_index]] = parent_index + 1;
  int child_index = <1T>_Heap_Small_Child_I(h, parent_index, value);

  while (child_index > 0) {
    if (value[h->array[parent_index]] > value[h->array[child_index]]) {
      <1T>_Arraylist_Swap(h, parent_index, child_index);
      checked[h->array[parent_index]] = parent_index + 1;
      checked[h->array[child_index]] = child_index + 1;
      parent_index = child_index;
      child_index = <1T>_Heap_Small_Child_I(h, parent_index, value);
    } else {
      break;
    }
  }

  return root;
}

<2T> <1T>_Heap_Remove_I_At(<1T>_Arraylist *h, int index,
			   double *value, int *checked)
{
  int remove_index = checked[index] - 1;
  if (remove_index > 0) {
    checked[h->array[remove_index]] = 0;
    <2T> removed = h->array[remove_index];
    h->array[remove_index] = h->array[h->length - 1];
    h->length--;
  
    int parent_index = remove_index;
    checked[h->array[parent_index]] = parent_index + 1;
    int child_index = <1T>_Heap_Small_Child_I(h, parent_index, value);

    while (child_index > 0) {
      if (value[h->array[parent_index]] > value[h->array[child_index]]) {
	<1T>_Arraylist_Swap(h, parent_index, child_index);
	checked[h->array[parent_index]] = parent_index + 1;
	checked[h->array[child_index]] = child_index + 1;
	parent_index = child_index;
	child_index = <1T>_Heap_Small_Child_I(h, parent_index, value);
      } else {
	break;
      }
    }

    return removed;
  }

  return 0;
}

void Print_<1T>_Heap_I(const <1T>_Arraylist *h, double *value)
{
  printf("A heap with depth %d:\n", <1T>_Heap_Depth(h));

  int i;
  for (i = 1; i < h->length; i++) {
    printf("%g", value[h->array[i]]);
    printf(" ");
    if ((i & (i + 1)) == 0) { /* i = 2^k - 1*/
      printf("\n");
    }
  }
  printf("\n");

}

int Verify_<1T>_Heap_I(const <1T>_Arraylist *h, double *value)
{
#ifdef _DEBUG_2
  int k = 3, m = 1;
  for (m = 0; m < 7; m++) {
    printf("2^%d + %d - 1 = %d\n", k, m, pow2sum(k, m));
    
    int n = pow2sum(k, m);
    pow2decomp(n, &k, &m);
    printf("2^%d + %d - 1 = %d\n", k, m, pow2sum(k, m));
  }
#endif

  int i;
  for (i = h->length - 1; i > 1; i--) {
    int pid = PARENT_INDEX(i);
    if (value[h->array[i]] < value[h->array[pid]]) {
      //printf("%g -> %g\n", value[h->array[i]], value[h->array[pid]]);
      return i;
    }
  }
  
  return 0;
}

</t>

/* <1T>_Heap_Sort(): sort an array list using heap sort.
 *
 * Args: a - the array list to sort
 *
 * Return: void.
 */
void <1T>_Heap_Sort(<1T>_Arraylist *a)
{
  <1T>_Arraylist *h = <1T>_Arraylist_New(1, a->length);
  
  <1T>_Arraylist_Add(h, a->array[0]);
  
  int scan_pos = 1;
  while (scan_pos < a->length) {
    <1T>_Heap_Add(h, a->array[scan_pos++]);
  }

  int i;
  int length = a->length;
  a->length = 0;
  for (i = 0; i < length; i++) {
    <1T>_Arraylist_Add(a, <1T>_Heap_Remove(h));
  }
}

</T>
