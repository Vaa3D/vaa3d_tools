/* tz_stack_math.c
 *
 * 04-Nov-2007 Initial write: Ting Zhao
 */

#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_stack_relation.h"
#include "tz_stack_attribute.h"
#include "tz_stack_io.h"
#include "tz_stack_math.h"
#include "tz_stack_utils.h"

INIT_EXCEPTION

/* Stack_And(): And operation between two stacks.
 * 
 * Args: stack1 - the first input stack
 *       stack2 - the second input stack
 *       out - output stack.
 *
 * Return: output stack, which points to the same address as out does if out is
 *         not NULL.
 *
 * Note: If out is NULL, a new stack will be made and the caller is resposible
 *       to free the returned stack.
 */
Stack *Stack_And(Stack *stack1, Stack *stack2, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    Print_Stack_Info(stack1);
    Print_Stack_Info(stack2);
    THROW(ERROR_DATA_COMPTB);
  }

  if ((stack1->kind != GREY) ||  (stack2->kind != GREY)) {
    THROW(ERROR_DATA_TYPE);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  int length = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < length; i++) {
    out->array[i] = ((stack1->array[i] > 0) && (stack2->array[i] > 0));
  }

  return out;
}

/* Stack_Or(): Or operation between two stacks.
 * 
 * Args: stack1 - the first input stack
 *       stack2 - the second input stack
 *       out - output stack, which could point to stack1 or stack2.
 *
 * Return: output stack, which points to the same address as out does if out is
 *         not NULL.
 *
 * Note: If out is NULL, a new stack will be made and the caller is resposible
 *       to free the returned stack.
 */
Stack *Stack_Or(Stack *stack1, Stack *stack2, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if ((stack1->kind != GREY) ||  (stack2->kind != GREY)) {
    THROW(ERROR_DATA_TYPE);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  int length = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < length; i++) {
    out->array[i] = ((stack1->array[i] > 0) || (stack2->array[i] > 0));
  }

  return out;
}

/* Stack_Xor(): Xor operation between two stacks.
 * 
 * Args: stack1 - the first input stack
 *       stack2 - the second input stack
 *       out - output stack, which could point to stack1 or stack2.
 *
 * Return: output stack, which points to the same address as out does if out is
 *         not NULL.
 *
 * Note: If out is NULL, a new stack will be made and the caller is resposible
 *       to free the returned stack.
 */
Stack *Stack_Xor(Stack *stack1, Stack *stack2, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if ((stack1->kind != GREY) ||  (stack2->kind != GREY)) {
    THROW(ERROR_DATA_TYPE);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  int length = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < length; i++) {
    if (((stack1->array[i] > 0) && (stack2->array[i] > 0)) ||
	((stack1->array[i] == 0) && (stack2->array[i] == 0))) {
      out->array[i] = 0;
    } else {
      out->array[i] = 1;
    }
  }

  return out;
}

/* Stack_Not(): Not operation of a stack.
 * 
 * Args: stack - the first input stack
 *       out - output stack, which could point to stack (in place operation).
 *
 * Return: output stack, which points to the same address as out does if out is
 *         not NULL.
 *
 * Note: If out is NULL, a new stack will be made and the caller is resposible
 *       to free the returned stack.
 */
Stack *Stack_Not(Stack *stack, Stack *out)
{
  if (stack == NULL) {
    THROW(ERROR_POINTER_NULL);
  }

  if (stack->kind != GREY) {
    THROW(ERROR_DATA_TYPE);
  }

  if (out == NULL) {
    out = Make_Stack(stack->kind, stack->width, stack->height, 
		     stack->depth);
  }

  int length = Stack_Voxel_Number(stack);
  int i;
  for (i = 0; i < length; i++) {
    out->array[i] = (stack->array[i] == 0);
  }

  return out;
}

Stack* Stack_Bsub(Stack *stack1, Stack *stack2, Stack *out)
{
  TZ_ASSERT((stack1->kind == GREY) && (stack2->kind == GREY),
	    "uint8 stack supported only");

  if (out == NULL) {
    out = Make_Stack(GREY, stack1->width, stack1->height, stack1->depth);
  }

  int nvoxel = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < nvoxel; i++) {
    if (stack1->array[i] > stack2->array[i]) {
      out->array[i] = 1;
    } else {
      out->array[i] = 0;
    }
  }
  
  return out;
}

#define STACK_AND_E(array1, array2, array_out)			\
  for (offset = 0; offset < voxel_number; offset++) {		\
    array_out[offset] = MIN2(array1[offset], array2[offset]);	\
  }

Stack *Stack_And_E(Stack *stack1, Stack *stack2, Stack *out)
{
  ASSERT((stack1 != NULL) && (stack2 != NULL), "null stack");
  ASSERT(Stack_Same_Attribute(stack1, stack2) == TRUE, "unmatched stack");
  
  if (out == NULL) {
    out = Make_Stack(stack1->kind, 
		     stack1->width, stack1->height, stack1->depth);
  }

  ASSERT(Stack_Same_Attribute(stack1, out) == TRUE, "unmatched stack");
  
  Image_Array ima1;
  Image_Array ima2;
  Image_Array imaout;

  ima1.array = stack1->array;
  ima2.array = stack2->array;
  imaout.array = out->array;
  
  size_t offset;
  size_t voxel_number = (size_t) Stack_Voxel_Number(stack1);

  switch (out->kind) {
  case GREY:
    STACK_AND_E(ima1.array, ima2.array, imaout.array);
    break;
  case GREY16:
    STACK_AND_E(ima1.array16, ima2.array16, imaout.array16);
    break;
  case FLOAT32:
    STACK_AND_E(ima1.array32, ima2.array32, imaout.array32);
    break;
  case FLOAT64:
    STACK_AND_E(ima1.array64, ima2.array64, imaout.array64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return out;
}

#define STACK_OR_E(array1, array2, array_out)			\
  for (offset = 0; offset < voxel_number; offset++) {		\
    array_out[offset] = MAX2(array1[offset], array2[offset]);	\
  }

Stack *Stack_Or_E(Stack *stack1, Stack *stack2, Stack *out)
{
  ASSERT((stack1 != NULL) && (stack2 != NULL), "null stack");
  ASSERT(Stack_Same_Attribute(stack1, stack2) == TRUE, "unmatched stack");
  
  if (out == NULL) {
    out = Make_Stack(stack1->kind, 
		     stack1->width, stack1->height, stack1->depth);
  }

  ASSERT(Stack_Same_Attribute(stack1, out) == TRUE, "unmatched stack");
  
  Image_Array ima1;
  Image_Array ima2;
  Image_Array imaout;

  ima1.array = stack1->array;
  ima2.array = stack2->array;
  imaout.array = out->array;
  
  size_t offset;
  size_t voxel_number = (size_t) Stack_Voxel_Number(stack1);

  switch (out->kind) {
  case GREY:
    STACK_OR_E(ima1.array, ima2.array, imaout.array);
    break;
  case GREY16:
    STACK_OR_E(ima1.array16, ima2.array16, imaout.array16);
    break;
  case FLOAT32:
    STACK_OR_E(ima1.array32, ima2.array32, imaout.array32);
    break;
  case FLOAT64:
    STACK_OR_E(ima1.array64, ima2.array64, imaout.array64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return out;  
}

#define STACK_MASK(array1, array2, array_out, suffix, length)		\
  for (i = 0; i < length; i++) {					\
    if (array2[i] > 0) {						\
      TZ_CONCATU(array_out, suffix)[i] = TZ_CONCATU(array1, suffix)[i];	\
    }									\
  }

/* Stack_Mask(): Mask a stack.
 *
 */
Stack *Stack_Mask(Stack *stack1, const Stack *stack2, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)){
    THROW(ERROR_POINTER_NULL);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  if (Stack_Same_Attribute(stack1, out) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (Stack_Kind(stack2) != GREY) {
    THROW(ERROR_DATA_TYPE);
  }

  int length = Stack_Voxel_Number(stack1);
  int i;
  
  DEFINE_SCALAR_ARRAY_ALL(array1, stack1);
  DEFINE_SCALAR_ARRAY_ALL(array_out, out);

  switch (stack1->kind) {
  case GREY:
    STACK_MASK(array1, stack2->array, array_out, grey, length);
    break;
  case GREY16:
    STACK_MASK(array1, stack2->array, array_out, grey16, length);
    break;
  case FLOAT32:
    STACK_MASK(array1, stack2->array, array_out, float32, length);
    break;
  case FLOAT64:
    STACK_MASK(array1, stack2->array, array_out, float64, length);
    break;
  default:
    THROW(ERROR_DATA_TYPE);
    break;
  }

  return out;
}

#undef STACK_MASK

#define STACK_ADD(stack1_array, stack2_array, out_array, validate)	\
  for (i = 0; i < nvoxel; i++) {					\
    double result = (double) (stack1_array[i]) + (double) (stack2_array[i]); \
    validate(result);							\
    out_array[i] = result;						\
  }

#define STACK_ADD_T(t1, t2, t3)			\
  if ((stack1->kind == TZ_CONCAT(GREY, t1)) && (stack2->kind == TZ_CONCAT(GREY, t2)) && (out->kind == TZ_CONCAT(GREY, t3))) { \
    STACK_ADD(TZ_CONCAT(ima1.array, t1), TZ_CONCAT(ima2.array, t2), TZ_CONCAT(ima_out.array, t3), TZ_CONCAT(VALIDATE_INTENSITY_GREY, t3)); \
  }  

Stack *Stack_Add(Stack *stack1, Stack *stack2, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  Image_Array ima1, ima2, ima_out;
  ima1.array = stack1->array;
  ima2.array = stack2->array;
  ima_out.array = out->array;
  
  int nvoxel = Stack_Voxel_Number(stack1);
  int i;

  STACK_ADD_T(8, 8, 8);
  STACK_ADD_T(8, 8, 16);
  STACK_ADD_T(8, 8, 32);
  STACK_ADD_T(8, 8, 64);

  STACK_ADD_T(8, 16, 8);
  STACK_ADD_T(8, 16, 16);
  STACK_ADD_T(8, 16, 32);
  STACK_ADD_T(8, 16, 64);
  
  STACK_ADD_T(8, 32, 8);
  STACK_ADD_T(8, 32, 16);
  STACK_ADD_T(8, 32, 32);
  STACK_ADD_T(8, 32, 64);

  STACK_ADD_T(8, 64, 8);
  STACK_ADD_T(8, 64, 16);
  STACK_ADD_T(8, 64, 32);
  STACK_ADD_T(8, 64, 64);

  STACK_ADD_T(16, 8, 8);
  STACK_ADD_T(16, 8, 16);
  STACK_ADD_T(16, 8, 32);
  STACK_ADD_T(16, 8, 64);

  STACK_ADD_T(16, 16, 8);
  STACK_ADD_T(16, 16, 16);
  STACK_ADD_T(16, 16, 32);
  STACK_ADD_T(16, 16, 64);
  
  STACK_ADD_T(16, 32, 8);
  STACK_ADD_T(16, 32, 16);
  STACK_ADD_T(16, 32, 32);
  STACK_ADD_T(16, 32, 64);

  STACK_ADD_T(16, 64, 8);
  STACK_ADD_T(16, 64, 16);
  STACK_ADD_T(16, 64, 32);
  STACK_ADD_T(16, 64, 64);

  STACK_ADD_T(32, 8, 8);
  STACK_ADD_T(32, 8, 16);
  STACK_ADD_T(32, 8, 32);
  STACK_ADD_T(32, 8, 64);

  STACK_ADD_T(32, 16, 8);
  STACK_ADD_T(32, 16, 16);
  STACK_ADD_T(32, 16, 32);
  STACK_ADD_T(32, 16, 64);
  
  STACK_ADD_T(32, 32, 8);
  STACK_ADD_T(32, 32, 16);
  STACK_ADD_T(32, 32, 32);
  STACK_ADD_T(32, 32, 64);

  STACK_ADD_T(32, 64, 8);
  STACK_ADD_T(32, 64, 16);
  STACK_ADD_T(32, 64, 32);
  STACK_ADD_T(32, 64, 64);

  STACK_ADD_T(64, 8, 8);
  STACK_ADD_T(64, 8, 16);
  STACK_ADD_T(64, 8, 32);
  STACK_ADD_T(64, 8, 64);

  STACK_ADD_T(64, 16, 8);
  STACK_ADD_T(64, 16, 16);
  STACK_ADD_T(64, 16, 32);
  STACK_ADD_T(64, 16, 64);
  
  STACK_ADD_T(64, 32, 8);
  STACK_ADD_T(64, 32, 16);
  STACK_ADD_T(64, 32, 32);
  STACK_ADD_T(64, 32, 64);

  STACK_ADD_T(64, 64, 8);
  STACK_ADD_T(64, 64, 16);
  STACK_ADD_T(64, 64, 32);
  STACK_ADD_T(64, 64, 64);

  /*
  int length = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < length; i++) {
    out->array[i] = stack1->array[i] + stack2->array[i];
  }
  */

  return out;
}

#define STACK_SUB(stack1_array, stack2_array, out_array, validate)	\
  for (i = 0; i < nvoxel; i++) {					\
    double result = (double) (stack1_array[i]) - (double) (stack2_array[i]); \
    validate(result);							\
    out_array[i] = result;						\
  }

#define STACK_SUB_T(t1, t2, t3)			\
  if ((stack1->kind == TZ_CONCAT(GREY, t1)) && (stack2->kind == TZ_CONCAT(GREY, t2)) && (out->kind == TZ_CONCAT(GREY, t3))) { \
    STACK_SUB(TZ_CONCAT(ima1.array, t1), TZ_CONCAT(ima2.array, t2), TZ_CONCAT(ima_out.array, t3), TZ_CONCAT(VALIDATE_INTENSITY_GREY, t3)); \
  } 

Stack *Stack_Sub(Stack *stack1, Stack *stack2, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  Image_Array ima1, ima2, ima_out;
  ima1.array = stack1->array;
  ima2.array = stack2->array;
  ima_out.array = out->array;
  
  int nvoxel = Stack_Voxel_Number(stack1);
  int i;

  STACK_SUB_T(8, 8, 8);
  STACK_SUB_T(8, 8, 16);
  STACK_SUB_T(8, 8, 32);
  STACK_SUB_T(8, 8, 64);

  STACK_SUB_T(8, 16, 8);
  STACK_SUB_T(8, 16, 16);
  STACK_SUB_T(8, 16, 32);
  STACK_SUB_T(8, 16, 64);
  
  STACK_SUB_T(8, 32, 8);
  STACK_SUB_T(8, 32, 16);
  STACK_SUB_T(8, 32, 32);
  STACK_SUB_T(8, 32, 64);

  STACK_SUB_T(8, 64, 8);
  STACK_SUB_T(8, 64, 16);
  STACK_SUB_T(8, 64, 32);
  STACK_SUB_T(8, 64, 64);

  STACK_SUB_T(16, 8, 8);
  STACK_SUB_T(16, 8, 16);
  STACK_SUB_T(16, 8, 32);
  STACK_SUB_T(16, 8, 64);

  STACK_SUB_T(16, 16, 8);
  STACK_SUB_T(16, 16, 16);
  STACK_SUB_T(16, 16, 32);
  STACK_SUB_T(16, 16, 64);
  
  STACK_SUB_T(16, 32, 8);
  STACK_SUB_T(16, 32, 16);
  STACK_SUB_T(16, 32, 32);
  STACK_SUB_T(16, 32, 64);

  STACK_SUB_T(16, 64, 8);
  STACK_SUB_T(16, 64, 16);
  STACK_SUB_T(16, 64, 32);
  STACK_SUB_T(16, 64, 64);

  STACK_SUB_T(32, 8, 8);
  STACK_SUB_T(32, 8, 16);
  STACK_SUB_T(32, 8, 32);
  STACK_SUB_T(32, 8, 64);

  STACK_SUB_T(32, 16, 8);
  STACK_SUB_T(32, 16, 16);
  STACK_SUB_T(32, 16, 32);
  STACK_SUB_T(32, 16, 64);
  
  STACK_SUB_T(32, 32, 8);
  STACK_SUB_T(32, 32, 16);
  STACK_SUB_T(32, 32, 32);
  STACK_SUB_T(32, 32, 64);

  STACK_SUB_T(32, 64, 8);
  STACK_SUB_T(32, 64, 16);
  STACK_SUB_T(32, 64, 32);
  STACK_SUB_T(32, 64, 64);

  STACK_SUB_T(64, 8, 8);
  STACK_SUB_T(64, 8, 16);
  STACK_SUB_T(64, 8, 32);
  STACK_SUB_T(64, 8, 64);

  STACK_SUB_T(64, 16, 8);
  STACK_SUB_T(64, 16, 16);
  STACK_SUB_T(64, 16, 32);
  STACK_SUB_T(64, 16, 64);
  
  STACK_SUB_T(64, 32, 8);
  STACK_SUB_T(64, 32, 16);
  STACK_SUB_T(64, 32, 32);
  STACK_SUB_T(64, 32, 64);

  STACK_SUB_T(64, 64, 8);
  STACK_SUB_T(64, 64, 16);
  STACK_SUB_T(64, 64, 32);
  STACK_SUB_T(64, 64, 64);

  /*
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if ((stack1->kind != GREY) ||  (stack2->kind != GREY)) {
    THROW(ERROR_DATA_TYPE);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  int length = Stack_Voxel_Number(stack1);
  int i;
  for (i = 0; i < length; i++) {
    if (stack1->array[i] <= stack2->array[i]) {
      out->array[i] = 0;
    } else {
      out->array[i] = stack1->array[i] - stack2->array[i];
    }
  }
  */

  return out;  
}

#define STACK_MUL(stack1_array, stack2_array, out_array)	\
  for (i = 0; i < nvoxel; i++) {				\
    out_array[i] = (double) (stack1_array[i]) * (double) (stack2_array[i]); \
  }

#define STACK_MUL_T(t1, t2, t3)			\
  if ((stack1->kind == TZ_CONCAT(GREY, t1)) && (stack2->kind == TZ_CONCAT(GREY, t2)) && (out->kind == TZ_CONCAT(GREY, t3))) { \
    STACK_MUL(TZ_CONCAT(ima1.array, t1), TZ_CONCAT(ima2.array, t2), TZ_CONCAT(ima_out.array, t3)); \
  }  

Stack *Stack_Mul(Stack *stack1, Stack *stack2, Stack *out)
{
  Image_Array ima1, ima2, ima_out;
  ima1.array = stack1->array;
  ima2.array = stack2->array;
  ima_out.array = out->array;
  
  int nvoxel = Stack_Voxel_Number(stack1);
  int i;

  STACK_MUL_T(8, 8, 8);
  STACK_MUL_T(8, 8, 16);
  STACK_MUL_T(8, 8, 32);
  STACK_MUL_T(8, 8, 64);

  STACK_MUL_T(8, 16, 8);
  STACK_MUL_T(8, 16, 16);
  STACK_MUL_T(8, 16, 32);
  STACK_MUL_T(8, 16, 64);
  
  STACK_MUL_T(8, 32, 8);
  STACK_MUL_T(8, 32, 16);
  STACK_MUL_T(8, 32, 32);
  STACK_MUL_T(8, 32, 64);

  STACK_MUL_T(8, 64, 8);
  STACK_MUL_T(8, 64, 16);
  STACK_MUL_T(8, 64, 32);
  STACK_MUL_T(8, 64, 64);

  STACK_MUL_T(16, 8, 8);
  STACK_MUL_T(16, 8, 16);
  STACK_MUL_T(16, 8, 32);
  STACK_MUL_T(16, 8, 64);

  STACK_MUL_T(16, 16, 8);
  STACK_MUL_T(16, 16, 16);
  STACK_MUL_T(16, 16, 32);
  STACK_MUL_T(16, 16, 64);
  
  STACK_MUL_T(16, 32, 8);
  STACK_MUL_T(16, 32, 16);
  STACK_MUL_T(16, 32, 32);
  STACK_MUL_T(16, 32, 64);

  STACK_MUL_T(16, 64, 8);
  STACK_MUL_T(16, 64, 16);
  STACK_MUL_T(16, 64, 32);
  STACK_MUL_T(16, 64, 64);

  STACK_MUL_T(32, 8, 8);
  STACK_MUL_T(32, 8, 16);
  STACK_MUL_T(32, 8, 32);
  STACK_MUL_T(32, 8, 64);

  STACK_MUL_T(32, 16, 8);
  STACK_MUL_T(32, 16, 16);
  STACK_MUL_T(32, 16, 32);
  STACK_MUL_T(32, 16, 64);
  
  STACK_MUL_T(32, 32, 8);
  STACK_MUL_T(32, 32, 16);
  STACK_MUL_T(32, 32, 32);
  STACK_MUL_T(32, 32, 64);

  STACK_MUL_T(32, 64, 8);
  STACK_MUL_T(32, 64, 16);
  STACK_MUL_T(32, 64, 32);
  STACK_MUL_T(32, 64, 64);

  STACK_MUL_T(64, 8, 8);
  STACK_MUL_T(64, 8, 16);
  STACK_MUL_T(64, 8, 32);
  STACK_MUL_T(64, 8, 64);

  STACK_MUL_T(64, 16, 8);
  STACK_MUL_T(64, 16, 16);
  STACK_MUL_T(64, 16, 32);
  STACK_MUL_T(64, 16, 64);
  
  STACK_MUL_T(64, 32, 8);
  STACK_MUL_T(64, 32, 16);
  STACK_MUL_T(64, 32, 32);
  STACK_MUL_T(64, 32, 64);

  STACK_MUL_T(64, 64, 8);
  STACK_MUL_T(64, 64, 16);
  STACK_MUL_T(64, 64, 32);
  STACK_MUL_T(64, 64, 64);

  return out;
}

/**
 * Subtract a value from a stack. The stack will be kept unchaged if
 * subtr is less than 0.
 */
void Stack_Subc(Stack *stack,int subtr)
{
  if(subtr<=0)
    return;

  int pixelNumber = Get_Stack_Size(stack);
  uint16 *array16 = (uint16 *)stack->array;
  int i;

  switch(stack->kind) {
  case GREY:
    if(subtr>=MAXVALUE_GREY)
      Zero_Stack(stack);
    else {
      for(i=0;i<pixelNumber;i++) {
	//printf("%d\n", i);
	if(stack->array[i]<=subtr)
	  stack->array[i] = 0;
	else
	  stack->array[i] -= subtr;
      }
    }
    break;
  case GREY16:
    if(subtr>=MAXVALUE_GREY16)
      Zero_Stack(stack);
    else {
      for(i=0;i<pixelNumber;i++) {
	if(array16[i]<=subtr)
	  array16[i] = 0;
	else
	  array16[i] -= subtr;
      }
    }
    break;
  default:
    fprintf(stderr,"Image_Hist: Unsupported image kind.\n");
  }
}

void Stack_Csub(Stack *stack, int subtr)
{
  int pixelNumber = Get_Stack_Size(stack);
  uint16 *array16 = (uint16 *)stack->array;
  int i;

  switch(stack->kind) {
  case GREY:
    if(subtr <= 0)
      Zero_Stack(stack);
    else {
      for(i=0;i<pixelNumber;i++) {
	//printf("%d\n", i);
	if(stack->array[i] >= subtr)
	  stack->array[i] = 0;
	else
	  stack->array[i] = subtr - stack->array[i];
      }
    }
    break;
  case GREY16:
    if(subtr <= 0)
      Zero_Stack(stack);
    else {
      for(i=0;i<pixelNumber;i++) {
	if(array16[i] >= subtr)
	  array16[i] = 0;
	else
	  array16[i] = subtr - array16[i];
      }
    }
    break;
  default:
    fprintf(stderr,"Image_Hist: Unsupported image kind.\n");
  }
}

void Stack_Addc_M(Stack *stack,int value)
{
  int pixelNumber = Get_Stack_Size(stack);
  uint16 *array16 = (uint16 *)stack->array;
  int i;
  int tmp_value;

  switch(stack->kind) {
    case GREY:
      for(i=0;i<pixelNumber;i++) {
	if (stack->array[i] > 0) {
	  tmp_value = value + stack->array[i];
	  VALIDATE_INTENSITY_GREY(tmp_value)	
	  stack->array[i] = tmp_value;
	}
      }
      break;
    case GREY16:
      for(i=0;i<pixelNumber;i++) {
	if (array16[i] > 0) {
	  tmp_value = value + array16[i];
	  VALIDATE_INTENSITY_GREY16(tmp_value)	
	  array16[i] = tmp_value;
	}
      }
      break;
    default:
      fprintf(stderr,"Image_Hist: Unsupported image kind.\n");
  }
}

#define STACK_ADD_R(stack1_array, stack2_array, out_array, validate)	\
  for (k = range[2]; k <= range[5]; k++) {				\
    for (j = range[1]; j <= range[4]; j++) {				\
      for (i = range[0]; i <= range[3]; i++) {				\
	double result = (double) (stack1_array[offset]) + (double) (stack2_array[offset]); \
	validate(result);						\
	out_array[offset] = result;					\
	offset++;							\
      }									\
      offset += height_offset;						\
    }									\
    offset += depth_offset;						\
  }

#define STACK_ADD_R_T(t1, t2, t3)			\
  if ((stack1->kind == TZ_CONCAT(GREY, t1)) && (stack2->kind == TZ_CONCAT(GREY, t2)) && (out->kind == TZ_CONCAT(GREY, t3))) { \
    STACK_ADD_R(TZ_CONCAT(ima1.array, t1), TZ_CONCAT(ima2.array, t2), TZ_CONCAT(ima_out.array, t3), TZ_CONCAT(VALIDATE_INTENSITY_GREY, t3)); \
  }  

Stack *Stack_Add_R(Stack *stack1, Stack *stack2, const int *range, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  Image_Array ima1, ima2, ima_out;
  ima1.array = stack1->array;
  ima2.array = stack2->array;
  ima_out.array = out->array;
  
  int i, j, k;
  int offset = Stack_Util_Offset(range[0], range[1], range[2],
				 stack1->width, stack1->height, stack1->depth);
  int height_offset = stack1->width - (range[3] - range[0] + 1);
  int depth_offset = stack1->height * stack1->width 
    - stack1->width * (range[4] - range[1] + 1);

  STACK_ADD_R_T(8, 8, 8);
  STACK_ADD_R_T(8, 8, 16);
  STACK_ADD_R_T(8, 8, 32);
  STACK_ADD_R_T(8, 8, 64);

  STACK_ADD_R_T(8, 16, 8);
  STACK_ADD_R_T(8, 16, 16);
  STACK_ADD_R_T(8, 16, 32);
  STACK_ADD_R_T(8, 16, 64);
  
  STACK_ADD_R_T(8, 32, 8);
  STACK_ADD_R_T(8, 32, 16);
  STACK_ADD_R_T(8, 32, 32);
  STACK_ADD_R_T(8, 32, 64);

  STACK_ADD_R_T(8, 64, 8);
  STACK_ADD_R_T(8, 64, 16);
  STACK_ADD_R_T(8, 64, 32);
  STACK_ADD_R_T(8, 64, 64);

  STACK_ADD_R_T(16, 8, 8);
  STACK_ADD_R_T(16, 8, 16);
  STACK_ADD_R_T(16, 8, 32);
  STACK_ADD_R_T(16, 8, 64);

  STACK_ADD_R_T(16, 16, 8);
  STACK_ADD_R_T(16, 16, 16);
  STACK_ADD_R_T(16, 16, 32);
  STACK_ADD_R_T(16, 16, 64);
  
  STACK_ADD_R_T(16, 32, 8);
  STACK_ADD_R_T(16, 32, 16);
  STACK_ADD_R_T(16, 32, 32);
  STACK_ADD_R_T(16, 32, 64);

  STACK_ADD_R_T(16, 64, 8);
  STACK_ADD_R_T(16, 64, 16);
  STACK_ADD_R_T(16, 64, 32);
  STACK_ADD_R_T(16, 64, 64);

  STACK_ADD_R_T(32, 8, 8);
  STACK_ADD_R_T(32, 8, 16);
  STACK_ADD_R_T(32, 8, 32);
  STACK_ADD_R_T(32, 8, 64);

  STACK_ADD_R_T(32, 16, 8);
  STACK_ADD_R_T(32, 16, 16);
  STACK_ADD_R_T(32, 16, 32);
  STACK_ADD_R_T(32, 16, 64);
  
  STACK_ADD_R_T(32, 32, 8);
  STACK_ADD_R_T(32, 32, 16);
  STACK_ADD_R_T(32, 32, 32);
  STACK_ADD_R_T(32, 32, 64);

  STACK_ADD_R_T(32, 64, 8);
  STACK_ADD_R_T(32, 64, 16);
  STACK_ADD_R_T(32, 64, 32);
  STACK_ADD_R_T(32, 64, 64);

  STACK_ADD_R_T(64, 8, 8);
  STACK_ADD_R_T(64, 8, 16);
  STACK_ADD_R_T(64, 8, 32);
  STACK_ADD_R_T(64, 8, 64);

  STACK_ADD_R_T(64, 16, 8);
  STACK_ADD_R_T(64, 16, 16);
  STACK_ADD_R_T(64, 16, 32);
  STACK_ADD_R_T(64, 16, 64);
  
  STACK_ADD_R_T(64, 32, 8);
  STACK_ADD_R_T(64, 32, 16);
  STACK_ADD_R_T(64, 32, 32);
  STACK_ADD_R_T(64, 32, 64);

  STACK_ADD_R_T(64, 64, 8);
  STACK_ADD_R_T(64, 64, 16);
  STACK_ADD_R_T(64, 64, 32);
  STACK_ADD_R_T(64, 64, 64);

  return out;
}

#define STACK_SUB_R(stack1_array, stack2_array, out_array, validate)	\
  for (k = range[2]; k <= range[5]; k++) {				\
    for (j = range[1]; j <= range[4]; j++) {				\
      for (i = range[0]; i <= range[3]; i++) {				\
	double result = (double) (stack1_array[offset]) - (double) (stack2_array[offset]); \
	validate(result);						\
	out_array[offset] = result;					\
	offset++;							\
      }									\
      offset += height_offset;						\
    }									\
    offset += depth_offset;						\
  }

#define STACK_SUB_R_T(t1, t2, t3)			\
  if ((stack1->kind == TZ_CONCAT(GREY, t1)) && (stack2->kind == TZ_CONCAT(GREY, t2)) && (out->kind == TZ_CONCAT(GREY, t3))) { \
    STACK_SUB_R(TZ_CONCAT(ima1.array, t1), TZ_CONCAT(ima2.array, t2), TZ_CONCAT(ima_out.array, t3), TZ_CONCAT(VALIDATE_INTENSITY_GREY, t3)); \
  }  

Stack *Stack_Sub_R(Stack *stack1, Stack *stack2, const int *range, Stack *out)
{
  if ((stack1 == NULL) || (stack2 == NULL)) {
    THROW(ERROR_POINTER_NULL);
  }

  if (Stack_Same_Size(stack1, stack2) == FALSE) {
    THROW(ERROR_DATA_COMPTB);
  }

  if (out == NULL) {
    out = Make_Stack(stack1->kind, stack1->width, stack1->height, 
		     stack1->depth);
  }

  Image_Array ima1, ima2, ima_out;
  ima1.array = stack1->array;
  ima2.array = stack2->array;
  ima_out.array = out->array;
  
  int i, j, k;
  int offset = Stack_Util_Offset(range[0], range[1], range[2],
				 stack1->width, stack1->height, stack1->depth);
  int height_offset = stack1->width - (range[3] - range[0] + 1);
  int depth_offset = stack1->height * stack1->width 
    - stack1->width * (range[4] - range[1] + 1);

  STACK_SUB_R_T(8, 8, 8);
  STACK_SUB_R_T(8, 8, 16);
  STACK_SUB_R_T(8, 8, 32);
  STACK_SUB_R_T(8, 8, 64);

  STACK_SUB_R_T(8, 16, 8);
  STACK_SUB_R_T(8, 16, 16);
  STACK_SUB_R_T(8, 16, 32);
  STACK_SUB_R_T(8, 16, 64);
  
  STACK_SUB_R_T(8, 32, 8);
  STACK_SUB_R_T(8, 32, 16);
  STACK_SUB_R_T(8, 32, 32);
  STACK_SUB_R_T(8, 32, 64);

  STACK_SUB_R_T(8, 64, 8);
  STACK_SUB_R_T(8, 64, 16);
  STACK_SUB_R_T(8, 64, 32);
  STACK_SUB_R_T(8, 64, 64);

  STACK_SUB_R_T(16, 8, 8);
  STACK_SUB_R_T(16, 8, 16);
  STACK_SUB_R_T(16, 8, 32);
  STACK_SUB_R_T(16, 8, 64);

  STACK_SUB_R_T(16, 16, 8);
  STACK_SUB_R_T(16, 16, 16);
  STACK_SUB_R_T(16, 16, 32);
  STACK_SUB_R_T(16, 16, 64);
  
  STACK_SUB_R_T(16, 32, 8);
  STACK_SUB_R_T(16, 32, 16);
  STACK_SUB_R_T(16, 32, 32);
  STACK_SUB_R_T(16, 32, 64);

  STACK_SUB_R_T(16, 64, 8);
  STACK_SUB_R_T(16, 64, 16);
  STACK_SUB_R_T(16, 64, 32);
  STACK_SUB_R_T(16, 64, 64);

  STACK_SUB_R_T(32, 8, 8);
  STACK_SUB_R_T(32, 8, 16);
  STACK_SUB_R_T(32, 8, 32);
  STACK_SUB_R_T(32, 8, 64);

  STACK_SUB_R_T(32, 16, 8);
  STACK_SUB_R_T(32, 16, 16);
  STACK_SUB_R_T(32, 16, 32);
  STACK_SUB_R_T(32, 16, 64);
  
  STACK_SUB_R_T(32, 32, 8);
  STACK_SUB_R_T(32, 32, 16);
  STACK_SUB_R_T(32, 32, 32);
  STACK_SUB_R_T(32, 32, 64);

  STACK_SUB_R_T(32, 64, 8);
  STACK_SUB_R_T(32, 64, 16);
  STACK_SUB_R_T(32, 64, 32);
  STACK_SUB_R_T(32, 64, 64);

  STACK_SUB_R_T(64, 8, 8);
  STACK_SUB_R_T(64, 8, 16);
  STACK_SUB_R_T(64, 8, 32);
  STACK_SUB_R_T(64, 8, 64);

  STACK_SUB_R_T(64, 16, 8);
  STACK_SUB_R_T(64, 16, 16);
  STACK_SUB_R_T(64, 16, 32);
  STACK_SUB_R_T(64, 16, 64);
  
  STACK_SUB_R_T(64, 32, 8);
  STACK_SUB_R_T(64, 32, 16);
  STACK_SUB_R_T(64, 32, 32);
  STACK_SUB_R_T(64, 32, 64);

  STACK_SUB_R_T(64, 64, 8);
  STACK_SUB_R_T(64, 64, 16);
  STACK_SUB_R_T(64, 64, 32);
  STACK_SUB_R_T(64, 64, 64);

  return out;
}
