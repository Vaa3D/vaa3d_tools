/**@file tz_mc_stack.h
 * @brief multi-channel stack
 * @author Ting Zhao
 * @date 20-Nov-2008
 */

#ifndef _TZ_MC_STACK_H_
#define _TZ_MC_STACK_H_

#include "tz_cdefs.h"
#include "tz_image_lib_defs.h"

__BEGIN_DECLS

#ifdef _MSC_VER
typedef struct __Mc_Stack {
#else
typedef struct _Mc_Stack {
#endif
  int kind;
  int width;
  int height;
  int depth;
  int nchannel;
  uint8* array;
} Mc_Stack;

size_t Mc_Stack_Asize(size_t size);

Mc_Stack *Make_Mc_Stack(int kind, int width, int height, int depth, 
			int nchannel);
void Default_Mc_Stack(Mc_Stack *mc_stack);

/* Functions defined in the macros */
Mc_Stack* New_Mc_Stack();
Mc_Stack *Mc_Stack_Frame(int kind, int width, int height, int depth, 
       int nchannel);
Mc_Stack* Copy_Mc_Stack(const Mc_Stack *mc_stack);
void   Pack_Mc_Stack(Mc_Stack *mc_stack);
void   Free_Mc_Stack(Mc_Stack *mc_stack);
void   Kill_Mc_Stack(Mc_Stack *mc_stack);
void   Reset_Mc_Stack();
int    Mc_Stack_Usage();

int Save_Mc_Stack(Mc_Stack *mc_stack, char *file_path);
Mc_Stack* Load_Mc_Stack(char *file_path);
/***********************************/

Stack Mc_Stack_Channel(const Mc_Stack *mc_stack, int chan);  
void Mc_Stack_Copy_Channel(Mc_Stack *mc_stack, int chan, const Stack *stack);

/**@brief combine two multi-channel stack to one
 * Combine_Mc_Stack() appends the channels of mc_stack2 to mc_stack1 and returns
 * the new combined mc_stack
 */
Mc_Stack* Combine_Mc_Stack(const Mc_Stack *mc_stack1, const Mc_Stack *mc_stack2);

void Print_Mc_Stack_Info(const Mc_Stack *mc_stack);

/**@brief multi-channel stack to stack
 *
 * Mc_Stack_To_Stack() returns the stack that is converted from \a mc_stack.
 * It can be the same as \a stack if \a stack is not NULL, but \a stack should
 * preallocated to have sufficient space to hold the result. \a kind specifies
 * the kind of result. It can be -1 for default kind, which is the same as the
 * kind of \a mc_stack.
 */
Stack* Mc_Stack_To_Stack(const Mc_Stack *mc_stack, int kind, Stack *stack);

/**@brief Stack to a multi-channel stack.
 *
 * Mc_Stack_Rewrap_Stack() turns <stack> into a multi-channel stack and 
 * returns the result. <stack> is killed after the function call.
 */
Mc_Stack* Mc_Stack_Rewrap_Stack(Stack *stack);

Mc_Stack *Mc_Stack_Merge(Mc_Stack **stack, int n, int **offset, int option);

Mc_Stack *Mc_Stack_Merge_F(char **filepath, int n, int **offset, int option,
			   const int *ds);

void Mc_Stack_Grey16_To_8(Mc_Stack *mc_stack, int option);

Mc_Stack* Mc_Stack_Downsample_Mean(Mc_Stack *mc_stack, int wintv, 
				   int hintv, int dintv, Mc_Stack *out);

Mc_Stack* Mc_Stack_Upsample(const Mc_Stack *mc_stack, int wintv,
				  int hintv, int dintv, Mc_Stack *out);

/**@brief Maximum intensity projection.
 *
 * Mc_Stack_Mip() returns the maximum intensity projection of <mc_stack>.
 */
Mc_Stack* Mc_Stack_Mip(Mc_Stack *mc_stack);

void Mc_Stack_Binarize(Mc_Stack *mc_stack, double low, double high);

size_t  Mc_Stack_Voxel_Number(const Mc_Stack *mc_stack);

Mc_Stack* Mc_Stack_Flip_Y(const Mc_Stack *stack, Mc_Stack *out);
Mc_Stack *Mc_Stack_Mask(Mc_Stack *stack1, const Stack *stack2, Mc_Stack *out);

void Mc_Stack_Set_Zero(Mc_Stack *stack);
__END_DECLS

#endif
