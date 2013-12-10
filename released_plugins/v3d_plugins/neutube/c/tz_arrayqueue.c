/* tz_arrayqueue.c
 *
 * 22-May-2008 Initial write: Ting Zhao
 */
#include <stdlib.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_arrayqueue.h"

Arrayqueue* New_Arrayqueue()
{
  Arrayqueue *aq = (Arrayqueue *) 
    Guarded_Malloc(sizeof(Arrayqueue), "New_Arrayqueue");

  Default_Arrayqueue(aq);

  return aq;
}

void Free_Arrayqueue(Arrayqueue *aq)
{
  free(aq);
}

void Construct_Arrayqueue(Arrayqueue *aq, int capacity)
{
  aq->capacity = capacity;
  if (aq->array != NULL) {
    free(aq->array);
  }
  aq->array = (int *) Guarded_Malloc(sizeof(int) * capacity,
				     "Construct_Arrayqueue");
}

void Clean_Arrayqueue(Arrayqueue *aq)
{
  if (aq->array != NULL) {
    free(aq->array);
  }

  Default_Arrayqueue(aq);
}

Arrayqueue* Make_Arrayqueue(int capacity)
{
  Arrayqueue *aq = New_Arrayqueue();
  Construct_Arrayqueue(aq, capacity);
  return aq;
}

void Kill_Arrayqueue(Arrayqueue *aq)
{
  Clean_Arrayqueue(aq);
  free(aq);
}

void Default_Arrayqueue(Arrayqueue *aq)
{
  aq->head = -1;
  aq->tail = -1;
  aq->capacity = 0;
  aq->array = NULL;
}

void Print_Arrayqueue(Arrayqueue *aq)
{
  int next = aq->head;
  if (next < 0) {
    printf("Empty queue");
  } else {
    do {
      printf("%d ", next);
      if (next == aq->array[next]) {
	printf("\nself loop");
	break;
      } else {
	next = aq->array[next];
      }
    } while (next >= 0);
  }

  printf("\n");
}

void Initialize_Arrayqueue(Arrayqueue *aq)
{
  int i;
  for (i = 0; i < aq->capacity; i++) {
    aq->array[i] = -1;
  }
  aq->head = -1;
  aq->tail = -1;
}

int Arrayqueue_Size(Arrayqueue *aq)
{
  int next = aq->head;
  int n = 0;

  if (next >= 0) {
    do {
      if (next == aq->array[next]) {
	n = -n;
	break;
      } else {
	n++;
	next = aq->array[next];
      }
    } while (next >= 0);
  }

  return n;
}

int Arrayqueue_Max(Arrayqueue *aq)
{
  int next = aq->head;
  int max = next;

  if (next >= 0) {
    do {
      if (next == aq->array[next]) {
	break;
      } else {
	if (next > max) {
	  max = next;
	}
	next = aq->array[next];	
      }
    } while (next >= 0);
  }

  return max;
}

void Arrayqueue_Attach(Arrayqueue *aq, int *array, int n)
{
  aq->array = array;
  aq->capacity = n;
}

void Arrayqueue_Add_First(Arrayqueue *aq, int index)
{
  if ((index >= aq->capacity) || (index < 0)) {
    PRINT_EXCEPTION("Index out of range", "Nothing is done");
    return;
  }
 
  if (aq->tail < 0) {
    aq->tail = index;
  }
  aq->array[index] = aq->head;
  aq->head = index;
}

void Arrayqueue_Add_Last(Arrayqueue *aq, int index)
{
  if ((index >= aq->capacity) || (index < 0)) {
    PRINT_EXCEPTION("Index out of range", "Nothing is done");
    return;
  }
  
  if (aq->head < 0) {
    aq->head = index;
  } else {
    aq->array[aq->tail] = index;
  }

  aq->tail = index;
  aq->array[index] = -1;
}

void Arrayqueue_Update_Tail(Arrayqueue *aq)
{
  int next = aq->head;
  aq->tail = aq->head;
  while (next >= 0) {
    aq->tail = next;
    next = aq->array[next];
  }
}

int Arrayqueue_To_Index(Arrayqueue *aq, int *index)
{
  int next = aq->head;
  int n = 0;
  TZ_ASSERT(next >= 0, "Empty queue");
  do {
    if (next == aq->array[next]) {
      break;
    } else {
      index[next] = n++;
      next = aq->array[next];
    }
  } while (next >= 0);

  return n;
}

int Arrayqueue_To_Array(Arrayqueue *aq, int *array)
{
  int next = aq->head;
  int n = 0;
  TZ_ASSERT(next >= 0, "Empty queue");
  do {
    if (next == aq->array[next]) {
      break;
    } else {
      array[n++] = next;
      next = aq->array[next];
    }
  } while (next >= 0);

  return n;  
}

int Arrayqueue_Dequeue(Arrayqueue *aq)
{
  int old_head = aq->head;

  if (aq->head >= 0) {
    aq->head = aq->array[aq->head];
  }

  return old_head;
}

void Arrayqueue_Enqueue(Arrayqueue *aq, int index)
{
  Arrayqueue_Add_Last(aq, index);
}

BOOL Arrayqueue_Is_Empty(Arrayqueue *aq)
{
  if (aq->head < 0) {
    return TRUE;
  }

  return FALSE;
}
