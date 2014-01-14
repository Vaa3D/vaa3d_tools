/**@file tz_<1t>_doubly_linked_list.h
 * @brief doubly linked list of <2T>
 * @author Ting Zhao
 */

/* tz_doubly_linked_list.h.t
 *
 * 27-Nov-2007 Initial write: Ting Zhao
 */

/* <1T> prefix <2T> type <3T> free type*/

#ifndef _TZ_<1T>_DOUBLY_LINKED_LIST_H_
#define _TZ_<1T>_DOUBLY_LINKED_LIST_H_

#include "tz_cdefs.h"
#include "tz_doubly_linked_list_defs.h"
<3T=object,compact_pointer>
#include "tz_<1t>.h"
</t>

__BEGIN_DECLS

typedef struct _<1T>_Dlist {
  <2T> data;                    /* data */
  struct _<1T>_Dlist *prev;     /* previous node */
  struct _<1T>_Dlist *next;     /* next node */
} <1T>_Dlist;

/*
 * <1T>_Dlist_New() news a list with one node.
 */
<1T>_Dlist* <1T>_Dlist_New();

/*
 * <1T>_Dlist_Removeall() removes all nodes of <p>. All memories associated with
 * <p> will be freed.
 */
void <1T>_Dlist_Remove_All(<1T>_Dlist *p);

/*
 * <1T>_Dlist_Set_Data() sets data for the current node of <p>.
 */
void <1T>_Dlist_Set_Data(<1T>_Dlist *p, <2T> data);


/*
 * <1T>_Dlist_Head() return the head of <p> and <1T>_Dlist_Tail() returns the
 * tail of p.
 */
<1T>_Dlist* <1T>_Dlist_Head(const <1T>_Dlist *p);
<1T>_Dlist* <1T>_Dlist_Tail(const <1T>_Dlist *p);

/*
 * <1T>_Dlist_Next() returns the next node of <p> along the direction <d>.
 */
<1T>_Dlist* <1T>_Dlist_Next(const <1T>_Dlist *p, Dlist_Direction_e d);

/*
 * <1T>_Dlist_Length() returns the length of <p> in the direction <d>, which
 * can be forward (DL_FORWARD), backward (DL_BACKWARD) or both ways 
* (DL_BOTHDIR).
 * When the length is achieved in both ways, it is the total length of the list.
 * It returns 0 when the list is NULL and a negative number when something is 
 * wrong in the function (e.g. invalid direction option).
 */
int <1T>_Dlist_Length(const <1T>_Dlist *p, Dlist_Direction_e d);

/*
 * Locate_<1T>_Dlist locates a certain node that has offset <n> to <p>. <n> can
 * be negative, which means the located node is ahead of <p>. It returns NULL
 * if the location is out of range.
 */
<1T>_Dlist* Locate_<1T>_Dlist(const <1T>_Dlist *p, int n);

/*
 * Append_<1T>_Dlist() appends <p1> and <p2> and returns the head of the new
 * list.
 */
<1T>_Dlist* Append_<1T>_Dlist(<1T>_Dlist *p1, <1T>_Dlist *p2);

/*
 * <1T>_Dlist_Remove_End() removes one of the ends of <p> according to <e>:
 * DL_HEAD for head and DL_TAIL for tail. It returns the new head or tail if
 * the new list is not empty; otherwise it returns NULL.
 */
<1T>_Dlist* <1T>_Dlist_Remove_End(<1T>_Dlist *p, Dlist_End_e e);


/*
 * <1T>_Dlist_Remove_Node() removes the node <p> from its list.
 * It returns the new head if the new list is not empty; otherwise it 
 * returns NULL.
 */
<1T>_Dlist* <1T>_Dlist_Remove_Node(<1T>_Dlist *p);

/*
 * <1T>_Dlist_Unlinkall() removes all nodes but keeps the associated data
 * intact if they are references.
 */
void <1T>_Dlist_Unlinkall(<1T>_Dlist *p);

/*
 * <1T>_Dlist_Add() adds data as a new node to the list. <e> speficies where
 * where the new node is added. It can noly be only DL_HEAD or DL_TAIL. DL_HEAD
 * means the new node is added as the head and DL_TAIL means the new node is
 * added as the tail. It returns the new node.
 */
<1T>_Dlist* <1T>_Dlist_Add(<1T>_Dlist *p, <2T> data, Dlist_End_e e);

/*
 * <1T>_Dlist_Insert() inserts <data> into <p> right before its current 
 * position.
 */
void <1T>_Dlist_Insert(<1T>_Dlist *p, <2T> data);

/*
 * <1T>_Dlist_Is_Empty() returns TRUE if <p> is empty; otherwise it returns 
 * FALSE.
 */
BOOL <1T>_Dlist_Is_Empty(const <1T>_Dlist *p);

/**@brief Turn a list to an array.
 *
 * <1T>_Dlist_To_Array() copies a list to an array. 
 */
<2T>* <1T>_Dlist_To_Array(const <1T>_Dlist *p, int step, <2T> *array, 
			 int *length);

/*
 * Printing functions.
 */
void Print_<1T>_Dlist(const <1T>_Dlist *p);
void Print_<1T>_Dlist_Compact(const <1T>_Dlist *p);
void Print_<1T>_Dlist_F(const <1T>_Dlist *p, const char *format);


__END_DECLS

#endif
