
#define STACK_ZPROFILE(stack_array)\
  for (k = 0; k < stack->depth; k++) {\
    x[k] = 0.0;\
    for (j = 0; j < stack->height; j++) {\
      for (i = 0; i < stack->width; i++) {\
	x[k] += stack_array[offset++];\
	if (y != NULL) {\
	  y[k] = k;\
	}\
      }\
    }\
  }

static void stack_zprofile(Stack *stack, double *x, double *y)
{
  int i, j, k;
  int offset = 0;
  
  DEFINE_SCALAR_ARRAY_ALL(array, stack);
 
  switch(stack->kind) {
  case GREY:
    STACK_ZPROFILE(array_grey);
    break;
  case GREY16:
    STACK_ZPROFILE(array_grey16);
    break;
  case FLOAT32:
    STACK_ZPROFILE(array_float32);
    break;
  case FLOAT64:
    STACK_ZPROFILE(array_float64);
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }
}

static int estimate_chopoff(Stack *stack)
{
  double *x = darray_malloc(stack->depth);
  double *y = darray_malloc(stack->depth);

  stack_zprofile(stack, x, y);
  double c0, c1, cov00, cov01, cov11, sumsq;
  gsl_fit_linear (x, 1, y, 1, 20, &c0, &c1, &cov00, &cov01, &cov11, &sumsq);

  free(x);
  free(y);

  return (int) (c0 - 1);
}
