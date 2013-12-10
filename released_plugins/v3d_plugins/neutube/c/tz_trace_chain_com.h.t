/**@file tz_<1t>_chain_com.h
 * @brief chain of <2T>
 * @author Ting Zhao
 * @date 23-Oct-2008
 */

/* 1T Locseg 2T Local_Neuroseg */

#ifndef _TZ_<1T>_CHAIN_COM_H_
#define _TZ_<1T>_CHAIN_COM_H_

#include "tz_trace_defs.h"
#include "tz_<1t>_node_doubly_linked_list.h"
#include "tz_int_arraylist.h"
#include "tz_neuron_structure.h"
#include "tz_geo3d_circle.h"
#include "tz_vrml_defs.h"
#include "tz_stack_draw.h"

__BEGIN_DECLS

/**@addtogroup <1t>_chain <1T> chain
 *
 * @{
 */

/* note for possible future revision: list should hold all the data */
/**@struct _<1T>_Chain tz_<1t>_chain.h
 *
 * @brief data structure of neuron chain
 */
typedef struct _<1T>_Chain {
  <1T>_Node_Dlist *list;     /**< where the data are stored */
  <1T>_Node_Dlist *iterator; /**< be careful about nesting.*/
} <1T>_Chain;


/**@addtogroup <1t>_chain_obj general neurochain object routines
 *
 * @{
 */
<1T>_Chain* New_<1T>_Chain();
<1T>_Chain* Free_<1T>_Chain();

void Delete_<1T>_Chain(<1T>_Chain *chain);

void Construct_<1T>_Chain(<1T>_Chain *chain, <1T>_Node *p);
void Clean_<1T>_Chain(<1T>_Chain *chain);

<1T>_Chain* Make_<1T>_Chain(<1T>_Node *p);
void Kill_<1T>_Chain(<1T>_Chain *chain);

/**@}
 */

/**@addtogroup <1t>_chain_io_ I/O routines for <1t> chain
 *
 * @{
 */
void Print_<1T>_Chain(<1T>_Chain *chain);
void Write_<1T>_Chain(const char *file_path, <1T>_Chain *chain);
<1T>_Chain* Read_<1T>_Chain(const char *file_path);

/*@brief Load chain files under a directory
 *
 * Dir_<1T>_Chain_N() reads all files with a certain pattern under 
 * \a dir_name. The pattern is specified by the regular expression \a pattern,
 * i.e. any name that matches \a pattern in terms of regular expression will be
 * read. The results are stored in the returned array, in which each element is
 * a chain from a file. The number of chains is stored in \a n and the 
 * corresponding file numbers is stored in \a file_num. Set \a file_num to NULL
 * if you want to ignore it.
 */
<1T>_Chain* Dir_<1T>_Chain_N(const char *dir_name, const char *pattern,
				 int *n, int *file_num);

/*@brief Load chain files under a directory
 *
 * Dir_<1T>_Chain_Nd() is similar to Dir_<1T>_Chain_N, except that the return 
 * value is a an array of chain pointers.
 */
<1T>_Chain** Dir_<1T>_Chain_Nd(const char *dir_name, const char *pattern,
			       int *n, int *file_num);

/**@}*/

/**@addtogroup <1t>_chain_attr_ Chain attributes
 *
 * @{
 */

/*
 * <1T>_Chain_Length() return the number of nodes of <chain>.
 */
int <1T>_Chain_Length(<1T>_Chain *chain);
double <1T>_Chain_Geolen(<1T>_Chain *chain);

BOOL <1T>_Chain_Is_Empty(<1T>_Chain *chain);

/**@}*/

/**@addtogroup <1t>_chain_trace_ Trace routines
 * @{
 */

/**@brief Initialize a chain for tracing.
 *
 * <1T>_Chain_Trace_Init() retruns a chain for tracing. The starting point
 * is <<1t>>, which will be fit to <stack> after initialization. <fs> is used
 * to store fitting score.
 *
 * Note: <<1t>> will become a part of the returned chain.
 */
<1T>_Chain* <1T>_Chain_Trace_Init(const Stack *stack, double z_scale, 
				  <2T> *<1t>, void *ws);

/**@brief Test tracing status.
 *
 * Test if tracing should stop.
 * Return code: See tz_trace_defs.h for more details.
 */
int <1T>_Chain_Trace_Test(void *argv[]);

Trace_Workspace* 
<1T>_Chain_Default_Trace_Workspace(Trace_Workspace *tw, 
				   const Stack *stack);

/*
 * Trace neuron. 
 * <tw->dyvar> has the following meaning:
 *    tw->dyvar[0] : max radius
 *    tw->dyvar[1] : min radius
<1t=locseg>
 *    tw->dyvar[2] : first-level score 
</t>
 */
void Trace_<1T>(const Stack *stack, double z_scale, <1T>_Chain *chain, 
		Trace_Workspace *tw);

/*
 * <1T>_Chain_Trace_Np() 
 */
void <1T>_Chain_Trace_Np(const Stack *stack, double z_scale, 
			 <1T>_Chain *chain, Trace_Workspace *tw);

/**@}*/

/**@addtogroup <1t>_chain_iter_ Traversing routines
 * @{
 */
/**@brief Start the iterator.
 *
 * <1T>_Chain_Iterator_Locate() moves the iterator of <chain> to one of the end
 * positions of <chain>. If <d> is DL_HEAD, it moves to the head. If <d> is 
 * DL_TAIL, it moves to the tail. It returns the old iterator.
 */
<1T>_Node_Dlist* <1T>_Chain_Iterator_Start(<1T>_Chain *chain, 
					       Dlist_End_e d);

/**@brief Locate the iterator.
 *
 * <1T>_Chain_Iterator_Locate() moves the iterator of <chain> to the position
 * <index>. It returns the old iterator.
 */
<1T>_Node_Dlist* <1T>_Chain_Iterator_Locate(<1T>_Chain *chain, 
					    int index);
/*
 * <1T>_Chain_Next() makes the iterator forward by 1. It returns the node
 * before forwarding.
 */
<1T>_Node* <1T>_Chain_Next(<1T>_Chain *chain);
<1T>_Node* <1T>_Chain_Prev(<1T>_Chain *chain);
<1T>_Node* <1T>_Chain_Next_D(<1T>_Chain *chain, Dlist_End_e d);

<2T>* <1T>_Chain_Next_Seg(<1T>_Chain *chain);
<2T>* <1T>_Chain_Prev_Seg(<1T>_Chain *chain);
<2T>* <1T>_Chain_Next_Seg_D(<1T>_Chain *chain, Dlist_End_e d);

<1T>_Node* <1T>_Chain_Peek(const <1T>_Chain *chain);
<2T>* <1T>_Chain_Peek_Seg(const <1T>_Chain *chain);

<1T>_Node* <1T>_Chain_Peek_Next(const <1T>_Chain *chain);
<1T>_Node* <1T>_Chain_Peek_Prev(const <1T>_Chain *chain);


BOOL <1T>_Chain_Has_More(const <1T>_Chain *chain);

<1T>_Node* <1T>_Chain_Head(<1T>_Chain *chain);
<1T>_Node* <1T>_Chain_Tail(<1T>_Chain *chain);

<2T>* <1T>_Chain_Head_Seg(<1T>_Chain *chain);
<2T>* <1T>_Chain_Tail_Seg(<1T>_Chain *chain);

<1T>_Node* <1T>_Chain_Peek_At(<1T>_Chain *chain, int index);
<2T>* <1T>_Chain_Peek_Seg_At(<1T>_Chain *chain, int index);

/**@}*/


/**@addtogroup <1t>_chain_opr_ Chain manipulation
 * @{
 */

/*
 * <1T>_Chain_Add() adds a node to the end of the chain.
 * <1T>_Chain_Cat() appends <chain2> to <chain1>. Notice that the nodes in
 * chain2 are appended in <chain1> by reference, so use Free_<1T>_Chain()
 * only to free memory allocated to chain2.
 */
void <1T>_Chain_Add(<1T>_Chain *chain, <2T> *<1t>, 
		      Trace_Record *tr, Dlist_End_e end);
void <1T>_Chain_Add_Node(<1T>_Chain *chain, <1T>_Node *p, Dlist_End_e end);
void <1T>_Chain_Cat(<1T>_Chain *chain1, const <1T>_Chain *chain2);

void <1T>_Chain_Insert(<1T>_Chain *chain, <2T> *<1t>, 
		       Trace_Record *tr, int index);

/**@brief Remove current node.
 *
 * <1T>_Chain_Remove_Current() removes the node at the current position of the
 * iterator of <chain>. The iterator is moved to the next node.
 */
void <1T>_Chain_Remove_Current(<1T>_Chain *chain);

/**@brief Take current node.
 *
 * <1T>_Chain_Take_Current() removes the node at the current position of the
 * iterator of <chain> and returns it. The iterator is moved to the next node.
 */
<1T>_Node* <1T>_Chain_Take_Current(<1T>_Chain *chain);

void <1T>_Chain_Remove_End(<1T>_Chain *chain, Dlist_End_e end);
int <1T>_Chain_Remove_Overlap_Ends(<1T>_Chain *chain);
void <1T>_Chain_Remove_Turn_Ends(<1T>_Chain *chain, double max_angle);
/**@}*/

void <1T>_Chain_Label(<1T>_Chain *chain, Stack *stack, double z_scale);
void <1T>_Chain_Erase(<1T>_Chain *chain, Stack *stack, double z_scale);

void <1T>_Chain_Label_E(Stack *stack, <1T>_Chain *chain, double z_scale,
			  int begin, int end, double ratio, double diff);
void <1T>_Chain_Erase_E(Stack *stack, <1T>_Chain *chain, double z_scale,
			  int begin, int end, double ratio, double diff);
			
void <1T>_Chain_Label_G(<1T>_Chain *chain, Stack *stack, double z_scale,
			int begin, int end, double ratio, double diff, 
			int flag, int value);

void Set_<1T>_Trace_Record(<2T> *<1t>, const Trace_Record *tr);

/**@brief Connection test for two <1t> chains.
 *
 * <1T>_Chain_Connection_Test() tests if two chains are connected in \a stack.
 * It returns TRUE if the two chains are connected. More information is stored 
 * in \a conn, which specifies which mode is tested too. When the connection 
 * mode is hook-loop mode (set \a conn->mode to NEUROCOMP_CONN_HL), it tests if 
 * \a chain1 hooks to \a chain2. \a conn->info[0] stores the position of hook 
 * of \a chain1 (0 for head and 1 for tail). \a conn->info[1] stores the 
 * position of loop of \a chain2 and its value is the index of the hooked 
 * segment in \a chain2. The strenth of connection is \a conn->strength. 
 * \a ws is the workspace, which should be NULL in the current version.
 */
BOOL <1T>_Chain_Connection_Test(<1T>_Chain *chain1, <1T>_Chain *chain2,
				const Stack *stack, double z_scale, 
				Neurocomp_Conn *conn, 
				Connection_Test_Workspace *ws);

int <1T>_Chain_Hit_Test(<1T>_Chain *chain, Dlist_Direction_e d,
			  double x, double y, double z);
int <1T>_Chain_Hit_Test_Seg(<1T>_Chain *chain,
			    const <2T> *<1t>);

BOOL <1T>_Chain_Touched(<1T>_Chain *chain1, <1T>_Chain *chain2);

Geo3d_Circle* <1T>_Chain_To_Geo3d_Circle_Array(<1T>_Chain *chain,
						 Geo3d_Circle *circle, int *n);
void <1T>_Chain_Vrml_Fprint(FILE *fp, <1T>_Chain *chain,
			    const Vrml_Material *material, int indent);
void <1T>_Chain_To_Vrml_File(<1T>_Chain *chain, const char *file_path);


Graph* <1T>_Chain_Graph(<1T>_Chain *chain, int n, Int_Arraylist *hit_spots);

void Neuron_Structure_Set_<1T>_Chain_Array(Neuron_Structure *ns,
					   <1T>_Chain *chain_array);

/**@brief Reconstruct neuron structure.
 *
 * <1T>_Chain_Neurostruct() reconstructs neuron structure from \a chain and
 * returns the result. \a chain will be owned by \a Neuron_Structure.
 */
Neuron_Structure* <1T>_Chain_Neurostruct(<1T>_Chain *chain, int n, 
					   const Stack *stack, double z_scale,
					   void *ws);
 
int <1T>_Chain_End_Points(<1T>_Chain *chain, int n, coordinate_3d_t pts[]);
void <1T>_Chain_Hit_Stack(<1T>_Chain *chain, Stack *region, int hit[]);

int <1T>_Chain_Swc_Fprint(FILE *fp, <1T>_Chain *chain, int type,
  		          int start_id,
			  int parent_id, Dlist_Direction_e direction);

/*
 * <1T>_Chain_Reach_Conn() changes one of the ends of \a chain1 so that it
 * can touch \a chain2. How it reaches \a chain2 is defined in \a conn.
 */
void <1T>_Chain_Reach_Conn(<1T>_Chain *hook, <1T>_Chain *loop,
			     const Neurocomp_Conn *conn);

void <1T>_Chain_Draw_Stack(<1T>_Chain *chain, Stack *stack, 
			   const Stack_Draw_Workspace *ws);

/**@}*/

__END_DECLS

#endif
