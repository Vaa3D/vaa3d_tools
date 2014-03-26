/* private/tz_stack_objlabel.c
 * 
 * 02-Jun-2008 Initial write: Ting Zhao
 */

#ifdef STACK_LABEL_OBJECT_RECOVER_CHORD
static void stack_label_object_recover_chord(const IMatrix *chord, int seed)
{
  int index = seed;
  int next = index;

  while (index >= 0) {
    next = chord->array[index];
    chord->array[index] = -1;
    index = next;
  }
}
#endif

#ifdef STACK_LABEL_OBJECT_BY_CHORD
static void stack_label_object_by_chord(Stack *stack, const IMatrix *chord,
					int label, int seed)
{
  int index = seed;

  while (index >= 0) {
    stack->array[index] = label;
    index = chord->array[index];
  }
}
#endif
