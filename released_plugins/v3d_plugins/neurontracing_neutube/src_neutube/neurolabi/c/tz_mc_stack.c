#include <string.h>
#include "tz_image_lib_defs.h"
#include "tz_objdef.h"
#include "tz_mc_stack.h"
#include "tz_error.h"
#include "tz_image_array.h"
#include "tz_int_histogram.h"
#include "tz_stack_lib.h"
#include "tz_iarray.h"
#include "tz_stack_attribute.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"
#include "tz_image_trans.h"
#include "tz_stack_lib.h"
#include "tz_stack_math.h"

size_t Mc_Stack_Asize(size_t size)
{
  return size;
}

void Default_Mc_Stack(Mc_Stack *mc_stack)
{
  if (mc_stack->array != NULL) {
    return;
  }
  
  mc_stack->kind = 1;
  mc_stack->width = 0;
  mc_stack->height = 0;
  mc_stack->depth = 0;
  mc_stack->nchannel = 0;
}

static inline size_t mc_stack_asize(const Mc_Stack *mc_stack)
{ return Mc_Stack_Asize((size_t) mc_stack->kind * mc_stack->width * mc_stack->height *
			mc_stack->depth * mc_stack->nchannel); }

DEFINE_OBJECT_MANAGEMENT(Mc_Stack, array, asize, mc_stack)

Mc_Stack *Mc_Stack_Frame(int kind, int width, int height, int depth, 
			 int nchannel)
{
  size_t asize = Mc_Stack_Asize((size_t) kind * width * height * depth * 
				nchannel);

  Mc_Stack *mc_stack = new_mc_stack_empty(asize, "Make_Mc_Stack");
  mc_stack->kind = kind;
  mc_stack->width = width;
  mc_stack->height = height;
  mc_stack->depth = depth;
  mc_stack->nchannel = nchannel;
  mc_stack->array = NULL;

  return mc_stack;
  
}

Mc_Stack *Make_Mc_Stack(int kind, int width, int height, int depth, 
			int nchannel)
{
  Mc_Stack *mc_stack = Mc_Stack_Frame(kind, width, height, depth, nchannel);
  mc_stack->array = (uint8*) 
    Guarded_Malloc(OBJECT_HANDLE(Mc_Stack, mc_stack)->asize,
		   "Make_Mc_Stack");
  memset(mc_stack->array, 0, OBJECT_HANDLE(Mc_Stack, mc_stack)->asize);

  return mc_stack;
}

Stack Mc_Stack_Channel(const Mc_Stack *mc_stack, int chan)
{
  Stack stack;
  stack.kind = mc_stack->kind;
  stack.width = mc_stack->width;
  stack.height = mc_stack->height;
  stack.depth = mc_stack->depth;
  stack.text = NULL;
  stack.array = NULL;
  if (chan < mc_stack->nchannel) {
    stack.array = mc_stack->array + 
      (size_t) mc_stack->kind * mc_stack->width * mc_stack->height * 
      mc_stack->depth * chan;
  }

  return stack;
}

void Mc_Stack_Copy_Channel(Mc_Stack *mc_stack, int chan, const Stack *stack)
{
  TZ_ASSERT((chan >= 0) && (chan < mc_stack->nchannel), "Invalid channel");
  TZ_ASSERT((mc_stack->kind == stack->kind) && 
	    (mc_stack->width == stack->width) &&
	    (mc_stack->height == stack->height) &&
	    (mc_stack->depth == stack->depth),
	    "Unmatched stack");
  
  size_t bsize = Stack_Array_Bsize(stack);
  memcpy(mc_stack->array + bsize * chan, stack->array, bsize);
}

Mc_Stack* Combine_Mc_Stack(const Mc_Stack *mc_stack1, const Mc_Stack *mc_stack2)
{
  TZ_ASSERT((mc_stack1->kind == mc_stack2->kind) &&
      (mc_stack1->width == mc_stack2->width) &&
      (mc_stack1->height == mc_stack2->height) &&
      (mc_stack1->depth == mc_stack2->depth),
      "Unmatched stacks");

  Mc_Stack *mc_stack = Make_Mc_Stack(mc_stack1->kind, mc_stack1->width,
      mc_stack1->height, mc_stack1->depth, mc_stack1->nchannel+mc_stack2->nchannel);

  size_t mc_stack1_bsize = ((size_t) mc_stack1->width) * ((size_t) mc_stack1->height) *
      ((size_t) mc_stack1->depth) * ((size_t) mc_stack1->nchannel) *
      ((size_t) mc_stack1->kind);

  size_t mc_stack2_bsize = ((size_t) mc_stack2->width) * ((size_t) mc_stack2->height) *
        ((size_t) mc_stack2->depth) * ((size_t) mc_stack2->nchannel) *
        ((size_t) mc_stack2->kind);

  memcpy(mc_stack->array, mc_stack1->array, mc_stack1_bsize);
  memcpy(mc_stack->array + mc_stack1_bsize, mc_stack2->array, mc_stack2_bsize);
  return mc_stack;
}

void Print_Mc_Stack_Info(const Mc_Stack *mc_stack)
{
  if (mc_stack == NULL) {
    printf("Null stack\n");
  } else {
    printf("Multichannel stack: \n");
    printf("%d channels, %d bytes/pixel\n", mc_stack->nchannel, mc_stack->kind);
    printf("Plane size: %d x %d x %d\n", mc_stack->width, mc_stack->height,
        mc_stack->depth);
  }
}

#define MC_STACK_TO_STACK(mc_stack_array, stack_array, validate_value)	\
  for (i = 0; i < nvoxel; i++) {					\
    value = 0.0;							\
    for (j = 0; j < nchannel; j++) {					\
      value += mc_stack_array[j * channel_offset + i];			\
    }									\
    value /= nchannel;							\
    validate_value(value);						\
    stack_array[i] = value;						\
  }
  


Stack* Mc_Stack_To_Stack(const Mc_Stack *mc_stack, int kind, Stack *stack)
{
  TZ_ASSERT(mc_stack != NULL, "Null pointer.");
  TZ_ASSERT(kind > -2, "Invalid kind.");

  if (kind == -1) {
    kind = mc_stack->kind;
  }

  if (stack == NULL) {
    stack = Make_Stack(kind, mc_stack->width, mc_stack->height,
		       mc_stack->depth);
  }

  if (mc_stack->nchannel == 1) {
    memcpy(stack->array, mc_stack->array, Stack_Array_Bsize(stack));
  } else {
    Image_Array mc_ima;
    Image_Array ima;
    mc_ima.array = mc_stack->array;
    ima.array = stack->array;
    double value = 0.0;
    int nvoxel = Stack_Voxel_Number(stack);
    int nchannel = mc_stack->nchannel;
    int channel_offset = nvoxel;
    int i, j;
    if (mc_stack->kind == stack->kind) {
      switch (stack->kind) {
      case GREY:
	MC_STACK_TO_STACK(mc_ima.array8, ima.array8, VALIDATE_INTENSITY_GREY);
	break;
      case GREY16:
	MC_STACK_TO_STACK(mc_ima.array16, ima.array16,
			  VALIDATE_INTENSITY_GREY16);
	break;
      case FLOAT32:
	MC_STACK_TO_STACK(mc_ima.array32, ima.array32,
			  VALIDATE_INTENSITY_FLOAT32);
	break;
      case FLOAT64:
	MC_STACK_TO_STACK(mc_ima.array64, ima.array64,
			  VALIDATE_INTENSITY_FLOAT32);
	break;
      case COLOR:
	for (i = 0; i < nvoxel; i++) {
	  int k;
	  for (k = 0; k < 3; k++) {
	    value = 0.0;		
	    for (j = 0; j < nchannel; j++) {
	      value += mc_ima.arrayc[j * channel_offset + i][k];
	    }						
	    value /= nchannel;				
	    VALIDATE_INTENSITY_COLOR(value);			
	    ima.arrayc[i][k] = value;			
	  }
	}
	break;
      default:
	TZ_ERROR(ERROR_DATA_TYPE);
	break;
      }
    } else {
      if (kind != COLOR) {
	stack = Mc_Stack_To_Stack(mc_stack, mc_stack->kind, stack);
	Translate_Stack(stack, kind, 1);
      } else {
	if (mc_stack->nchannel == 3) {
	  switch (mc_stack->kind) {
	  case GREY:
	    {
	      uint8 *channel_array[3];
	      channel_array[0] = mc_ima.array8;
	      channel_array[1] = channel_array[0] + channel_offset;
	      channel_array[2] = channel_array[1] + channel_offset;
	      for (i = 0; i < nvoxel; i++) {
		ima.arrayc[i][0] = channel_array[0][i];
		ima.arrayc[i][1] = channel_array[1][i];
		ima.arrayc[i][2] = channel_array[2][i];
	      }
	    }
	    break;
	  case GREY16:
	    {
	      uint16 *channel_array[3];
	      channel_array[0] = mc_ima.array16;
	      channel_array[1] = channel_array[0] + channel_offset;
	      channel_array[2] = channel_array[1] + channel_offset;
	      int *map[3];
	      int min[3];
	      for (i = 0; i < 3; i++) {
		int *hist = Image_Array_Hist(channel_array[i], GREY16, nvoxel,
					     NULL);
		int j;
		int *hist_array = Int_Histogram_Array(hist);
		for (j = 1; j <= Int_Histogram_Length(hist); j++) {
		  hist_array[j] *= j;
		}

		map[i] = Int_Histogram_Equal_Info_Map(hist, 256, NULL);
#ifdef _DEBUG_2
		iarray_print2(map[i], Int_Histogram_Length(hist), 1);
#endif
		min[i] = Int_Histogram_Min(hist);
		free(hist);
	      }
	      
	      for (i = 0; i < nvoxel; i++) {
		ima.arrayc[i][0] = map[0][channel_array[0][i]-min[0]];
		ima.arrayc[i][1] = map[1][channel_array[1][i]-min[1]];
		ima.arrayc[i][2] = map[2][channel_array[2][i]-min[2]];
	      }
	      free(map[0]);
	      free(map[1]);
	      free(map[2]);
	    }
	    break;
	  default:
	    TZ_ERROR(ERROR_DATA_TYPE);
	    break;
	  }
	} else {
	  TZ_ERROR(ERROR_DATA_TYPE);
	}
      }
    }
  }

  return stack;
}

Mc_Stack* Mc_Stack_Rewrap_Stack(Stack *stack)
{
  Mc_Stack *mc_stack = Mc_Stack_Frame(stack->kind, stack->width, 
				      stack->height, stack->depth, 1);
  mc_stack->array = stack->array;
  stack->array = NULL;
  Kill_Stack(stack);

  OBJECT_HANDLE(Mc_Stack, mc_stack)->asize = mc_stack_asize(mc_stack);

  return mc_stack;
}

Mc_Stack *Mc_Stack_Merge(Mc_Stack **mc_stack, int n, int **offset, int option)
{
  Stack **stack = (Stack**) malloc(sizeof(Stack*) * n);
#ifdef _MSC_VER
  Stack *stack_array = (Stack*)malloc(sizeof(Stack)*n);
#else
  Stack stack_array[n];
#endif

  int i, j;
  Mc_Stack *result = NULL;

  for (j = 0; j < mc_stack[0]->nchannel; j++) {
    for (i = 0; i < n; i++) {
      stack_array[i] = Mc_Stack_Channel(mc_stack[i], j);
      stack[i] = stack_array + i;
    }
    
    Stack *merge_stack = Stack_Merge_M(stack, n, offset, option, NULL);
    if (result == NULL) {
      result = Make_Mc_Stack(merge_stack->kind, merge_stack->width, 
			     merge_stack->height, merge_stack->depth,
			     mc_stack[0]->nchannel);
    }

    Mc_Stack_Copy_Channel(result, j, merge_stack);
    
    Kill_Stack(merge_stack);
  }

#ifdef _MSC_VER
  free(stack_array);
#endif

  free(stack);

  return result;
}

Mc_Stack *Mc_Stack_Merge_F(char **filepath, int n, int **offset, int option,
			   const int *ds)
{
  /* determine final image size */
  int *start[3];
  int *end[3];
  int size[3];
  int i, j;
  /* alloc <start> <end> */
  for (i = 0; i < 3; i++) {
    start[i] = iarray_malloc(n);
    end[i] = iarray_malloc(n);
  }
  
  for (i = 0; i < n; i++) {
    for (j = 0; j < 3; j++) {
      start[j][i] = offset[i][j];
    }
  }

  int corner_offset[3];
  size_t min_idx[3];
  for (i = 0; i < 3; i++) {
    corner_offset[i] = iarray_min(start[i], n, min_idx + i);
  }

  for (i = 0; i < n; i++) {
    for (j = 0; j < 3; j++) {
      start[j][i] -= corner_offset[j];
    }

    /*
    if (Is_Lsm(filepath[i])) {
      Lsm_Size(filepath[i], size);
    } else if (Is_Tiff(filepath[i])) {
      Tiff_Size(filepath[i], size);
    } else {
      TZ_ERROR(ERROR_IO_FORMAT);
    }
    */
    
    Stack_Size_F(filepath[i], size);

    if (ds != NULL) {
      for (j = 0; j < 3; j++) {
	size[j] = size[j] / ds[j] + ((size[j] % ds[j]) > 0);
      }
    }

    end[0][i] = start[0][i] + size[0] - 1;
    end[1][i] = start[1][i] + size[1] - 1;
    end[2][i] = start[2][i] + size[2] - 1;
  }

  int width = iarray_max(end[0], n, NULL) - start[0][min_idx[0]] + 1;
  int height = iarray_max(end[1], n, NULL) - start[1][min_idx[1]] + 1;
  int depth = iarray_max(end[2], n, NULL) - start[2][min_idx[2]] + 1;

  /* free <start> <end> */
  for (i = 0; i < 3; i++) {
    free(start[i]);
    free(end[i]);
  }
  /***************************************************/

  Stack **stack = (Stack**) malloc(sizeof(Stack*) * n);
#ifdef _MSC_VER
  Stack *stack_array = (Stack*)malloc(sizeof(Stack)*n);
#else
  Stack stack_array[n];
#endif
  Mc_Stack *tmp_stack = Read_Mc_Stack(filepath[0], -1);
  
  int nchannel = tmp_stack->nchannel;
  int kind = tmp_stack->kind;

  Kill_Mc_Stack(tmp_stack);

  Mc_Stack *result = Make_Mc_Stack(kind, width, height, depth, nchannel);
  Stack result_stack;

  for (j = 0; j < nchannel; j++) {
    for (i = 0; i < n; i++) {
      tmp_stack = Read_Mc_Stack(filepath[i], j);
      if (ds != NULL) {
	Mc_Stack_Downsample_Mean(tmp_stack, ds[0] - 1, ds[1] - 1, ds[2] - 1,
				 tmp_stack);
      }
      stack_array[i] = Mc_Stack_Channel(tmp_stack, 0);
      tmp_stack->array = NULL;
      stack[i] = stack_array + i;
      Kill_Mc_Stack(tmp_stack);
    }

    result_stack = Mc_Stack_Channel(result, j);

    Stack_Merge_M(stack, n, offset, option, &result_stack);

    for (i = 0; i < n; i++) {
      free(stack_array[i].array);
    }
  }

#ifdef _MSC_VER
  free(stack_array);
#endif

  free(stack);

  return result;
}

/* option: 1 - [min, max] -> [0, 255]
 *         2 - [min, q(99.99)] -> [0, 255]
 *         3 - equal info map
 */
void Mc_Stack_Grey16_To_8(Mc_Stack *mc_stack, int option)
{
  Image_Array mc_ima;
  mc_ima.array = mc_stack->array;

  int nvoxel = mc_stack->width * mc_stack->height * mc_stack->depth;

  uint16 *channel_array[3];
  channel_array[0] = mc_ima.array16;
  channel_array[1] = channel_array[0] + nvoxel;
  channel_array[2] = channel_array[1] + nvoxel;
  int *map[3];
  int min[3];
  int max[3];
  int i, j;

  switch (option) {
  case 1:
    for (i = 0; i < mc_stack->nchannel; i++) {
      size_t index[2];
      Image_Array_Minmax_Index(channel_array[i], GREY16, nvoxel, index);
      min[i] = channel_array[i][index[0]];
      max[i] = channel_array[i][index[1]];
      int map_length = max[i] - min[i] + 1;
      map[i] = iarray_malloc(map_length);
      for (j = 0; j < map_length; j++) {
          map[i][j] = (j * 500/ (map_length - 1) + 1) / 2;
      }
    }
    break;
  case 2:
    for (i = 0; i < mc_stack->nchannel; i++) {
      int *hist = Image_Array_Hist(channel_array[i], GREY16, nvoxel,
				   NULL);
      min[i] = Int_Histogram_Min(hist);
      max[i] = Int_Histogram_Max(hist);
      int map_length = max[i] - min[i] + 1;
      int q = Int_Histogram_Quantile(hist, 0.9999) - min[i];

      free(hist);
      
      map[i] = iarray_malloc(map_length);
      for (j = 0; j <= q; j++) {
	map[i][j] = (j * 500 / q + 1) / 2;
      }

      for (j = q + 1; j < map_length; j++) {
	map[i][j] = 255;
      }
    }
    break;
  case 3:
    for (i = 0; i < mc_stack->nchannel; i++) {
      int *hist = Image_Array_Hist(channel_array[i], GREY16, nvoxel,
				   NULL);
      
      map[i] = Int_Histogram_Equal_Info_Map(hist, 256, NULL);
      min[i] = Int_Histogram_Min(hist);
      free(hist);
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }
  
  for (i = 0; i < mc_stack->nchannel; i++) {
    int j;
    for (j = 0; j < nvoxel; j++) {
      *(mc_ima.array8++) =  map[i][channel_array[i][j] - min[i]]; 
    }
    free(map[i]);
  }

  mc_stack->kind = 1;
}

Mc_Stack* Mc_Stack_Downsample_Mean(Mc_Stack *mc_stack, int wintv, 
				   int hintv, int dintv, Mc_Stack *out)
{
  int width = mc_stack->width;
  int height = mc_stack->height;
  int depth = mc_stack->depth;

  int nw = mc_stack->width / (wintv+1) + ((mc_stack->width % (wintv+1)) > 0);
  int nh = mc_stack->height / (hintv+1) + ((mc_stack->height % (hintv+1)) > 0);
  int nd = mc_stack->depth / (dintv+1) + ((mc_stack->depth % (dintv+1)) > 0);  

  if (out == NULL) {
    out = Make_Mc_Stack(mc_stack->kind, nw, nh, nd, mc_stack->nchannel);
  }

  int i;
  for (i = 0; i < mc_stack->nchannel; i++) {
    if (mc_stack == out) {
      out->width = nw;
      out->height = nh;
      out->depth = nd;
    }
    Stack out_stack = Mc_Stack_Channel(out, i);
    if (mc_stack == out) {
      mc_stack->width = width;
      mc_stack->height = height;
      mc_stack->depth = depth;
    }
    Stack stack = Mc_Stack_Channel(mc_stack, i);
    Downsample_Stack_Mean(&stack, wintv, hintv, dintv, &out_stack);
  }
  
  if (mc_stack == out) {
    out->width = nw;
    out->height = nh;
    out->depth = nd;
  }

  return out;
}

size_t  Mc_Stack_Voxel_Number(const Mc_Stack *mc_stack)
{
  return (size_t) mc_stack->width * mc_stack->height * mc_stack->depth;
}

#define MC_STACK_MIP(ima_array, ima_proj_array) \
  /* For each channel */					\
  for (ch = 0; ch < mc_stack->nchannel; ch++) {			\
    /* initialize project array*/				\
    memcpy(ima_proj.array, ima.array, area * mc_stack->kind);	\
    /* for each slice */					\
    for (k = 0; k < mc_stack->depth; k++) {			\
      /* for each voxel on the slice */				\
      for (v = 0; v < area; v++) {				\
	/* assign the maximum */				\
	if (ima.array8[0] > ima_proj.array8[v]) {		\
	  ima_proj.array8[v] = ima.array8[0];			\
	}							\
	ima.array8++;						\
      }								\
    }								\
    /* to next channel */					\
    ima_proj.array8 += area;					\
  }

Mc_Stack* Mc_Stack_Mip(Mc_Stack *mc_stack)
{
  Mc_Stack *proj = Make_Mc_Stack(mc_stack->kind, mc_stack->width, 
				 mc_stack->height, 1, 
				 mc_stack->nchannel);

  Image_Array ima;
  Image_Array ima_proj;
  
  size_t area = mc_stack->width * mc_stack->height;
  int k, ch;
  size_t v;

  ima_proj.array = proj->array;
  ima.array = mc_stack->array;

  switch (mc_stack->kind) {
  case GREY:
    MC_STACK_MIP(ima.array8, ima_proj.array8);
    break;
  case GREY16:
    MC_STACK_MIP(ima.array16, ima_proj.array16);
    break;
  case GREY32:
    MC_STACK_MIP(ima.array32, ima_proj.array32);
    break;
  case GREY64:
    MC_STACK_MIP(ima.array64, ima_proj.array64);
    break;
  case COLOR:
    /* For each channel */
    for (ch = 0; ch < mc_stack->nchannel; ch++) {
      /* initialize project array*/
      memcpy(ima_proj.array, ima.array, area * mc_stack->kind);
      /* for each slice */
      for (k = 0; k < mc_stack->depth; k++) {
	/* for each voxel on the slice */
	for (v = 0; v < area; v++) {
	  /* assign the maximum */
	  if (ima.arrayc[0][0] > ima_proj.arrayc[v][0]) { 
	    ima_proj.arrayc[v][0] = ima.arrayc[0][0];
	  }
	  if (ima.arrayc[0][1] > ima_proj.arrayc[v][1]) { 
	    ima_proj.arrayc[v][1] = ima.arrayc[0][1];
	  }
	  if (ima.arrayc[0][2] > ima_proj.arrayc[v][2]) { 
	    ima_proj.arrayc[v][2] = ima.arrayc[0][2];
	  }
	  ima.array8++;
	}
      }
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
    break;
  }

  return proj;
}

Mc_Stack* Mc_Stack_Upsample(const Mc_Stack *mc_stack, int wintv,
			    int hintv, int dintv, Mc_Stack *out)
{
  Stack stack;
  stack.kind = mc_stack->kind;
  stack.width  = mc_stack->width;
  stack.height  = mc_stack->height;
  stack.depth  = mc_stack->depth;
  stack.array = mc_stack->array;

  if (out == NULL) {
    out = Make_Mc_Stack(mc_stack->kind, mc_stack->width * (wintv + 1),
			mc_stack->height * (hintv + 1), 
			mc_stack->depth * (dintv + 1),
			mc_stack->nchannel);
  }

  Stack out_stack;
  out_stack.kind = out->kind;
  out_stack.width  = out->width;
  out_stack.height  = out->height;
  out_stack.depth  = out->depth;
  out_stack.array = out->array;

  Upsample_Stack(&stack, wintv, hintv, dintv, &out_stack);

  return out;
}

#define MC_STACK_BINARIZE(stack_array) \
  for (i = 0;  i < nvoxel; i++) { \
    stack_array[i] = (stack_array[i] <= high) && \
      (stack_array[i] >= low); \
  }

void Mc_Stack_Binarize(Mc_Stack *mc_stack, double low, double high)
{
  size_t i;
  size_t nvoxel = Mc_Stack_Voxel_Number(mc_stack);

  SCALAR_STACK_OPERATION(mc_stack, MC_STACK_BINARIZE)
}

Mc_Stack* Mc_Stack_Flip_Y(const Mc_Stack *mc_stack, Mc_Stack *out)
{
  if (out == NULL) {
    out = Make_Mc_Stack(mc_stack->kind, mc_stack->width, mc_stack->height,
        mc_stack->depth, mc_stack->nchannel);
  }

  int ch;
  for (ch = 0; ch < mc_stack->nchannel; ++ch) {
    Stack stack = Stack_View_Mc_Stack(mc_stack, ch);
    Stack out_buffer = Stack_View_Mc_Stack(out, ch);
    Flip_Stack_Y(&stack, &out_buffer);
  }

  return out;
}

Mc_Stack *Mc_Stack_Mask(Mc_Stack *stack1, const Stack *stack2, Mc_Stack *out)
{
  if (out == NULL) {
    out = Make_Mc_Stack(stack1->kind, stack1->width, stack1->height,
        stack1->depth, stack1->nchannel);
  }

  int ch;
  for (ch = 0; ch < stack1->nchannel; ++ch) {
    Stack stack = Stack_View_Mc_Stack(stack1, ch);
    Stack out_buffer = Stack_View_Mc_Stack(out, ch);
    Stack_Mask(&stack, stack2, &out_buffer);
  }

  return out;
}

void Mc_Stack_Set_Zero(Mc_Stack *stack)
{
  int ch;
  for (ch = 0; ch < stack->nchannel; ++ch) {
    Stack stack_view = Stack_View_Mc_Stack(stack, ch);
    Zero_Stack(&stack_view);
  }
}
