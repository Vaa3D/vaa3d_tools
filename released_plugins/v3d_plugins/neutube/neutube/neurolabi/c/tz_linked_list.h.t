/**@file tz_<1t>_linked_list.h
 * @brief Linked list of <2T>
 * @author Ting Zhao
 */

/* tz_linked_list.h.t
 *
 * 27-Nov-2007 Initial write: Ting Zhao
 */

/* <1T> prefix <2T> type <3T> free type*/

#ifndef _TZ_<1T>_LINKED_LIST_H_
#define _TZ_<1T>_LINKED_LIST_H_

#include "tz_cdefs.h"
<1T=Object_3d>
#include "tz_object_3d.h"
</t>
<1T=Voxel>
#include "tz_voxel.h"
</t>
<2T=unipointer_t>
#include "tz_unipointer.h"
</t>

__BEGIN_DECLS

typedef struct _<1T>_List {
  <2T> data;
  struct _<1T>_List *next;
} <1T>_List;

/*
 * <1T>_List_New() news a list with one node.
 */
<1T>_List* <1T>_List_New();

/*
 * <1T>_List_Set_Data() sets data for the current node of <p>.
 */
void <1T>_List_Set_Data(<1T>_List *p, <2T> data);

/*
 * <1T>_List_Removeall() removes all nodes, including their data, from a list.
 * <1T>_List_Unlinkall() also removes all nodes but keeps the associated data
 * intact if they are references.
 */
void <1T>_List_Removeall(<1T>_List **p);
void <1T>_List_Unlinkall(<1T>_List **p);

void Kill_<1T>_List(<1T>_List *p);

/*
 * <1T>_List_Add() adds data as a new node at the beginning of the list.
 * It returns the new node. <p> will also point to the new head. 
 * <1T>_List_Add_Last() adds data to the end of the list and returns the new
 * tail. <p> should not be NULL for this function.
 */
<1T>_List* <1T>_List_Add(<1T>_List **p, <2T> data);
<1T>_List* <1T>_List_Add_Last(<1T>_List *p, <2T> data);

/*
 * <1T>_List_Remove() removes the first node from p. <1T>_List_Unlink() also
 * removes the first node from p, but it will return the data in the removed
 * node.
 */
void <1T>_List_Remove(<1T>_List **p);
<2T> <1T>_List_Unlink(<1T>_List **p);

/*
 * <1T>_List_Remove_Last() removes the last node of the list <p>. Its behavior
 * is not defined if <p> is the last node itself. 
 */
void <1T>_List_Remove_Last(<1T>_List *p);


<1T>_List* <1T>_List_Next(<1T>_List *p);
<1T>_List* <1T>_List_Last(<1T>_List *p);

BOOL <1T>_List_Is_Empty(const <1T>_List *p);
int <1T>_List_Length(const <1T>_List *p);

/*
 * <1T>_List_To_Array() copies a list to an array. 
 */
<2T>* <1T>_List_To_Array(const <1T>_List *p, int step, <2T> *array, 
			 int *length);

void Print_<1T>_List(const <1T>_List *p);
void Print_<1T>_List_Compact(const <1T>_List *p);
void Print_<1T>_List_F(const <1T>_List *p, const char *format);

/*
 * The following routines implement a queue by a list.
 */

/**@brief Enqueue an element.
 *
 * <1T>_Queue_En() adds <data> to the end of the queue <p> and returns the
 * new end. <*p> can be NULL, but after the function call, <*p> will be 
 * changed to the head position.
 */
<1T>_List* <1T>_Queue_En(<1T>_List **p, <2T> data);

/**@brief Dequeue an element.
 *
 * <1T>_Queue_De() removes the element at the head of the queue and returns
 * the removed element. <*p> is changed to the new head position or NULL if
 * it is empty.
 */
<2T> <1T>_Queue_De(<1T>_List **p);

<2T> <1T>_Queue_Peek(const <1T>_List *p); 

/*
 * The following routines implement a stack by a list.
 */
void <1T>_Stack_Push(<1T>_List **p, <2T> data);
<2T> <1T>_Stack_Pop(<1T>_List **p);
<2T> <1T>_Stack_Peek(const <1T>_List *p); 


__END_DECLS

#endif
