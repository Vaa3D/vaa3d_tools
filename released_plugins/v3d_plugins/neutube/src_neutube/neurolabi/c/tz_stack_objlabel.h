/**@file tz_stack_objlabel.h
 * @brief label objects in a stack
 * @author Ting Zhao
 * @date 31-Jan-2008
 */

#ifndef _TZ_STACK_OBJLABEL_
#define _TZ_STACK_OBJLABEL_

#include "tz_cdefs.h"
#include "tz_imatrix.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

#define STACK_OBJLABLE_MAX_SIZE 2147483647

/**@brief Workspace for labeling objects.
 */
typedef struct _Objlabel_Workspace {
  int conn;             /**< connnectivity */
  int seed;             /**< an integer for passing or storing seed */
  BOOL init_chord;      /**< initialize \a chord (TRUE) or not (FALSE). */
  BOOL recover_chord;   /**< recover \a chord (TRUE) or not (FALSE). */
  IMatrix *chord;       /**< space for intermediate result */
  void *u;              /**< undefined space */
} Objlabel_Workspace;


#define STACK_OBJLABEL_OPEN_WORKSPACE(stack, ow)			\
  BOOL is_chord_owner = FALSE;						\
  if (ow->chord == NULL) {						\
    ow->chord = Make_3d_IMatrix(stack->width, stack->height, stack->depth); \
    is_chord_owner = TRUE;						\
    ow->init_chord = TRUE;						\
  } else {								\
    if (ow->chord->ndim != 3) {						\
      THROW(ERROR_DATA_TYPE);						\
    }									\
    if ((stack->width != ow->chord->dim[0]) ||				\
	(stack->height != ow->chord->dim[1]) ||				\
	(stack->depth != ow->chord->dim[2])) {				\
      THROW(ERROR_DATA_COMPTB);						\
    }									\
  }

#define STACK_OBJLABEL_CLOSE_WORKSPACE(ow)			\
  if (is_chord_owner == TRUE) {					\
    Kill_IMatrix(ow->chord);					\
    ow->chord = NULL;						\
  } else {							\
    if (ow->recover_chord == TRUE) {				\
      stack_label_object_recover_chord(ow->chord, ow->seed);	\
    }								\
  }

Objlabel_Workspace *New_Objlabel_Workspace();
void Default_Objlabel_Workspace(Objlabel_Workspace *ow);
void Delete_Objlabel_Workspace(Objlabel_Workspace *ow);
void Kill_Objlabel_Workspace(Objlabel_Workspace *ow);

/* 
 * Stack_Label_Object_Level_N() lables an object with the level code
 * constraint <code>. It is the same as Stack_Label_Object_N() is <code> is 
 * NULL. Otherwise it labels no voxels greater than <max>.
 *
 * Stack_Label_Object_Dist_N() labels an object in a stack to a certain 
 * distance. 
 */

/**@brief Label an object (Obsolete).
 *
 * Stack_Label_Object_N() labels an object in a stack. <chord> is used for
 * storing intermediate result. It starts from the stack index <seed> and
 * labels all contigous voxles with value <flag> to the value <label>. <n_nbr>
 * specifies the neighborhood system.
 */
int Stack_Label_Object_N(Stack *stack, IMatrix *chord, int seed, int flag,
			 int label, int n_nbr);

/**@brief Label an object.
 *
 * Stack_Label_Object_W() labels an object in a stack. \ow is used for
 * storing intermediate result and parameters. It starts from the stack index 
 * \a seed and labels all contigous voxles with value \a flag to the value 
 * \a label.
 */
int Stack_Label_Object_W(Stack *stack, int seed, int flag, int label, 
			 Objlabel_Workspace *ow);

/**@brief Label an object to a certain level (Obsolete).
 * 
 * Stack_Label_Object_Level_N() lables an object with the level code
 * constraint <code>. It is the same as Stack_Label_Object_N() is <code> is 
 * NULL. Otherwise it labels no voxels greater than <max>.
 */
int Stack_Label_Object_Level_N(Stack *stack, IMatrix *chord, int seed, 
			       int flag, int label, Stack *code, int max,
			       int n_nbr);

/**@brief Label an object to a certain level.
 * 
 * Stack_Label_Object_Level_Nw() lables an object with the level code
 * constraint \a code. It is the same as Stack_Label_Object_W() if \a code is 
 * NULL. Otherwise it labels no voxels greater than level \a max.
 */
int Stack_Label_Object_Level_Nw(Stack *stack, int seed, 
				int flag, int label, Stack *code, int max,
				Objlabel_Workspace *ow);

/**@brief Label an object to a certain distance.
 *
 * Stack_Label_Object_Dist_N() labels an object within a certain range, i.e.
 * only voxels no further than \a dist away from \seed are labeled. \a flag 
 * could be -1 for this function and it means all voxels, from both foreground
 * and background within the distance will be labeled.
 */
int Stack_Label_Object_Dist_N(Stack *stack, IMatrix *chord, int seed, 
			      int flag, int label, double dist,
			      int n_nbr);

/**@brief Label objects.
 *
 * Stack_Label_Objects_N() labels all objects in <stack> and each object has a
 * unique value. It returns the number of labeled objects. <label> is the
 * minimal value of the labels. <chord> is for storing temporary results. It
 * can be NULL, which means that the function itself will handle it.
 */
int Stack_Label_Objects_N(Stack *stack, IMatrix *chord, 
			  int flag, int label, int n_nbr);

/**@brief Label objects with a certain value.
 *
 * Stack_Label_Objects_Ns() is similar to Stack_Label_Objects_N(). But it will
 * set a pixel of each object to slabel.
 */
int Stack_Label_Objects_Ns(Stack *stack, IMatrix *chord, 
			   int flag, int label, int slabel, int n_nbr);

/**@brief Label large objects
 *
 * Stack_Label_Large_Objects_N() labels objects that have voxel number no 
 * smaller than <minsize> by the value <label> + 1. All other objects are 
 * labeled as <lablel>. It returns the number of large objects labeled.
 * <chord> is an integer matrix to store intermediate results. It can be NULL.
 */
int Stack_Label_Large_Objects_N(Stack *stack, IMatrix *chord, 
				int flag, int label, int minsize,
				int n_nbr);

int Stack_Label_Large_Objects_W(Stack *stack, int flag, int label, int minsize,
				Objlabel_Workspace *ow);

/*
 * Stack_Label_Largest_Object_W() labels the largest object in <stack> by the
 * value <label> + 1 and all other objects are labeled as <label>. It returns
 * the size of the largest object. If there are more than one object that has
 * the largest size, only one of them is labeled as the largest object. It
 * returns the size of the largest object.
 */
int Stack_Label_Largest_Object_W(Stack *stack, int flag, int label,
				 Objlabel_Workspace *ow);

int Stack_Label_Largest_Object_N(Stack *stack, IMatrix *chord, 
				 int flag, int label, int n_nbr);

void Stack_Grow_Object_S(Stack *seed, Stack *mask, Objlabel_Workspace *ow);

/*
 * Note: Stack_Build_Seed_Graph_G() is preferred than Stack_Build_Seed_Graph().
 *
 * Stack_Build_Seed_Graph() builds a graph of <seed> based on the binary
 * stack <stack>. The result is stored in the connection matrix <connmat>.*
 * 
 * Stack_Build_Seed_Graph_G() does the same job but with a different interface.
 * The result is returned as a graph, which takes much less memory than a
 * connection matrix does when there are many seeds.
 */
void Stack_Build_Seed_Graph(Stack *stack, int *seed, int nseed,
			    uint8_t **connmat, Objlabel_Workspace *ow);

#include "tz_graph.h"
Graph* Stack_Build_Seed_Graph_G(Stack *stack, int *seed, int nseed, 
				BOOL weighted, Objlabel_Workspace *ow);


Objlabel_Workspace* Make_Objlabel_Workspace_Gg(int width, int height, 
					       int depth);

Graph* Stack_Build_Seed_Graph_Gg(Stack *stack, int *seed, int nseed,
				 BOOL weighted, Objlabel_Workspace *ow);


Graph* Stack_Label_Field_Neighbor_Graph(Stack *stack, int threshold,
    Objlabel_Workspace *ow);

__END_DECLS

#endif
