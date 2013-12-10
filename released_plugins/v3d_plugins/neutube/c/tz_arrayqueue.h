/**@file tz_arrayqueue.h
 * @brief array queue
 * @author Ting Zhao
 * @date 22-May-2008
 */

#ifndef _TZ_ARRAY_QUEUE_
#define _TZ_ARRAY_QUEUE_

#include "tz_cdefs.h"

__BEGIN_DECLS

/**@brief Array queue data structure
 *
 * x_0 = head;
 * x_{n+1} = f(x_n)
 */
typedef struct _Arrayqueue {
  int head; /**< head buffer */
  int tail; /**< tail buffer */
  int capacity; /**< capacity */
  int *array; /**< array data */
} Arrayqueue;

Arrayqueue* New_Arrayqueue();
void Free_Arrayqueue(Arrayqueue *aq);

void Construct_Arrayqueue(Arrayqueue *aq, int capacity);
void Clean_Arrayqueue(Arrayqueue *aq);

Arrayqueue* Make_Arrayqueue(int capacity);
void Kill_Arrayqueue(Arrayqueue *aq);

void Default_Arrayqueue(Arrayqueue *aq);

void Print_Arrayqueue(Arrayqueue *aq);

/**@brief Initialize an array queue.
 *
 * Initialize_Arrayqueue() sets all the elements in the array container of <aq>
 * to -1.
 */
void Initialize_Arrayqueue(Arrayqueue *aq);

/**@brief Size of an array queue.
 *
 * Arrayqueue_Size() returns the number of elements in the array queue.
 */
int Arrayqueue_Size(Arrayqueue *aq);

/**@brief Maximum of the array queue.
 *
 * Arrayqueue_Max() returns the maximum value of <aq>.
 */
int Arrayqueue_Max(Arrayqueue *aq);

/**@brief Attach array data to an array queue.
 *
 * Arrayqueue_Attach() attaches <array> to <aq>. <aq> becomes the owner of
 * <array> after attachment. <n> is the capacity of <array>.
 */
void Arrayqueue_Attach(Arrayqueue *aq, int *array, int n);

#define ARRAYQUEUE_ADD_LAST(aq, index)		\
  {						\
    aq->array[aq->tail] = index;		\
    aq->tail = index;				\
  }

#define ARRAYQUEUE_ADD_FIRST(aq, index)		\
  {						\
    aq->array[index] = aq->head;		\
    aq->head = index;				\
  }

/**@brief Add an element to the tail of an array queue.
 *
 * Arrayqueue_Add_Last() adds <index> to the tail of <aq>.
 */  
void Arrayqueue_Add_Last(Arrayqueue *aq, int index);

/**@brief Add an element to the head of an array queue.
 *
 * Arrayqueue_Add_Last() adds <index> to the head of <aq>.
 */  
void Arrayqueue_Add_First(Arrayqueue *aq, int index);

/**@brief Update the tail buffer.
 *
 * Arrayqueue_Update_Tail() updates the tail buffer of <aq>.
 */
void Arrayqueue_Update_Tail(Arrayqueue *aq);

/**@brief Get the order of the array queue.
 *
 * Arrayqueue_To_Index() checks the order of the elements in <aq> and stores
 * the result in <index>. It returns the number of elements.
 */
int Arrayqueue_To_Index(Arrayqueue *aq, int *index);

/**@brief Turn an array queue to a compact array.
 *
 * Arrayqueue_To_Array() turns <aq> to an array. The position of each element 
 * of <array> corresponds to its position in <aq>. It returns the number of 
 * elements.
 */
int Arrayqueue_To_Array(Arrayqueue *aq, int *array);

int Arrayqueue_Dequeue(Arrayqueue *aq);
void Arrayqueue_Enqueue(Arrayqueue *aq, int index);

BOOL Arrayqueue_Is_Empty(Arrayqueue *aq);

__END_DECLS

#endif
