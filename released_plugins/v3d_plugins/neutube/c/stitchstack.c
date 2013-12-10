#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utilities.h>
#include "tz_error.h"
#include "tz_farray.h"
#include "tz_darray.h"
#include "tz_iarray.h"
#include "tz_image_lib_defs.h"
#include "tz_fimage_lib.h"
#include "tz_dimage_lib.h"
#include "tz_stack_lib.h"
#include "tz_stack_stat.h"
#include "tz_image_io.h"
#include "tz_utilities.h"
#include "tz_graph_utils.h"
#include "tz_string.h"
#include "tz_stack_neighborhood.h"

/* stitchstack image1 image2 ... -o out
 */

int compare_string_by_num(const void *a, const void *b)
{
  if (strlen(*(char**)a) > strlen(*(char**)b)) {
    return 1;
  } else if (strlen(*(char**)a) < strlen(*(char**)b)) {
    return -1;
  } else {
    char *acopy = *(char**)a;
    char *bcopy = *(char**)b;
    while(*acopy == *bcopy) {
      acopy++;
      bcopy++;
    }
    int a1 = atoi(acopy);
    int b1 = atoi(bcopy);
    if (a1 < b1) {
      return -1;
    } else if (a1 > b1) {
      return 1;
    } else {
      return 0;
    }
  }
}

char** splitstack(Mc_Stack *stack, const char *filepath, int nstack) 
{
  Mc_Stack *tmpstack = Make_Mc_Stack(stack->kind, stack->width, stack->height,
      stack->depth / nstack, stack->nchannel);

  size_t channel_size = stack->kind * stack->width *stack->height
    * stack->depth;
  size_t channel_size2 = tmpstack->kind * tmpstack->width *tmpstack->height
    * tmpstack->depth;

  char **outpath = (char**) malloc(sizeof(char*) * nstack);
  char *prefix = "neurolabi_stitch";
  char cmd[500];
  sprintf(cmd, "rm tmp/%s*.lsm", prefix);
  system(cmd);

  int i;
  int k;
  uint8_t *array = stack->array;
  char filename[100];
  fname(filepath, filename);
  for (k = 0; k < nstack; k++) {
    int offset = 0;
    int offset2 = 0;
    for (i = 0; i < stack->nchannel; i++) {
      memcpy(tmpstack->array + offset2, array + offset, channel_size2);
      offset += channel_size;
      offset2 += channel_size2;
    }
    array += channel_size2;

    outpath[k] = (char*) malloc(sizeof(char) * 500);
    
    sprintf(outpath[k], "tmp/%s_%s_%03d.lsm", prefix, filename, k);
    Write_Mc_Stack(outpath[k], tmpstack, filepath);
  }
  Kill_Mc_Stack(tmpstack);

  return outpath;
}

static Stack *read_stack_b(char *filepath, int bgsub, int channel)
{
  Mc_Stack *mc_stack = Read_Mc_Stack(filepath, channel);

  if (bgsub == 200) { /* exclude blue channel */
    if (mc_stack->nchannel == 3) {
      mc_stack->nchannel = 2;
    }
  }

  int mask[mc_stack->nchannel];
  int i;
  for (i = 0; i < mc_stack->nchannel; i++) {
    if (bgsub == -100) {
      mask[i] = 1;
    } else {
      mask[i] = 0;
    }
  }

  if ((bgsub > 0) && (bgsub < 10)) {
    mask[bgsub - 1] = 1;
    bgsub = 1;
  }
  
  if ((bgsub == 1) || (bgsub == -100)) {
    int i;
    for (i = 0; i < mc_stack->nchannel; i++) {
      if (mask[i] == 1) {
	Stack channel = Mc_Stack_Channel(mc_stack, i);
	Pixel_Range* pr = Stack_Range(&channel,0);
	Stack_Sub_Common(&channel,0,(int)((pr->minval+pr->maxval)/2));
      }
    }
  }

  Stack *stack = Mc_Stack_To_Stack(mc_stack, -1, NULL);
  
  /* It seems there is no good way to reuse the memory block in Mc_Stack
   * due to Gene's memory management */
  Kill_Mc_Stack(mc_stack);

  return stack;

  /*
  if (Is_Lsm(filepath)) {
    return Read_Lsm_Stack(filepath, -1);
  } else {
    return Read_Stack(filepath);
  }
  */
}

static int load_align(const char *filepath, char ***stack_file, int ***offset)
{
  FILE *fp = fopen(filepath, "r");
  
  String_Workspace *sw = New_String_Workspace();
  
  char *line = NULL;
  
  int nstack = 0;
  while ((line = Read_Line(fp, sw)) != NULL) {
    if (Is_Space(line) == FALSE) {
      nstack++;
    }
  }

  if (nstack > 0) {
    GUARDED_MALLOC_ARRAY(*stack_file, nstack, char*);
    GUARDED_MALLOC_ARRAY(*offset, nstack, int*);

    fseek(fp, 0, SEEK_SET);
    int i = 0;
    while ((line = Read_Line(fp, sw)) != NULL) {
      if (Is_Space(line) == FALSE) {
	GUARDED_MALLOC_ARRAY((*stack_file)[i], strlen(line)+1, char);
	strcpy((*stack_file)[i], line);
	strrmspc((*stack_file)[i]);
	strsplit((*stack_file)[i], '(', 1);
	int n;
	(*offset)[i] = String_To_Integer_Array(strsplit(line, '(', 1), NULL, 
					       &n);
	i++;
      }
    }
  }

  Kill_String_Workspace(sw);

  fclose(fp);

  return nstack;
}

static int **get_conn_from_image(char *filepath, int ***config, int numStack) {
  Stack *tile = Read_Stack_U(filepath);
  
  int minvalue = Stack_Min(tile, NULL);
  int maxvalue = Stack_Max(tile, NULL);
  int midvalue = (minvalue+maxvalue)/2;
  int thre1 = (minvalue + midvalue)/2;
  int thre2 = (midvalue + maxvalue)/2;
  int h, w;
  int numTilePerRow = 0;
  int numTilePerCol = 0;
  for (h=0; h<tile->height; h++) {
    int pre = minvalue;
    for (w=0; w<tile->width; w++) {
      int pixelvalue;
      if (tile->kind == GREY) {
        pixelvalue = (*STACK_PIXEL_8(tile,w,h,0,0));
      } else if (tile->kind == GREY16) {
        pixelvalue = (*STACK_PIXEL_16(tile,w,h,0,0));
      }
      if (pixelvalue > thre1 && pixelvalue-pre > 0) {
        numTilePerRow++;
      }
      pre = pixelvalue;
    }
    if (numTilePerRow > 0) {
      //printf("number of tile per row: %d\n", numTilePerRow);
      break;
    }
  }
  for (w=0; w<tile->width; w++) {
    int pre = minvalue;
    for (h=0; h<tile->height; h++) {
      int pixelvalue;
      if (tile->kind == GREY) {
        pixelvalue = (*STACK_PIXEL_8(tile,w,h,0,0));
      } else if (tile->kind == GREY16) {
        pixelvalue = (*STACK_PIXEL_16(tile,w,h,0,0));
      }
      if (pixelvalue > thre1 && pixelvalue-pre > 0) {
        numTilePerCol++;
      }
      pre = pixelvalue;
    }
    if (numTilePerCol > 0) {
      //printf("number of tile per col: %d\n", numTilePerRow);
      break;
    }
  }
  int **matrix;
  int i, j;
  MALLOC_2D_ARRAY(matrix, numTilePerCol, numTilePerRow, int, i);
  for (i = 0; i < numTilePerCol; i++) {
    for (j = 0; j < numTilePerRow; j++) {
      matrix[i][j] = 0;
    }
  }
  int currentrow = 0;
  int currentcol = 0;
  int tileindex = 1;
  for (h=0; h<tile->height; h++) {
    int pre = minvalue;
    for (w=0; w<tile->width; w++) {
      int pixelvalue;
      if (tile->kind == GREY) {
        pixelvalue = (*STACK_PIXEL_8(tile,w,h,0,0));
      } else if (tile->kind == GREY16) {
        pixelvalue = (*STACK_PIXEL_16(tile,w,h,0,0));
      }
      int up = minvalue;
      if (h>0) {
        if (tile->kind == GREY) {
          up = (*STACK_PIXEL_8(tile,w,h-1,0,0));
        } else if (tile->kind == GREY16) {
          up = (*STACK_PIXEL_16(tile,w,h-1,0,0));
        }
      }
      if (pixelvalue > thre1 && pixelvalue > pre && pixelvalue > up) {
        if (pixelvalue > thre2) {
          matrix[currentrow][currentcol] = tileindex++;
        }
        currentcol++;
        if (currentcol >= numTilePerRow) {
          currentcol = 0;
          currentrow++;
        }
      }
      pre = pixelvalue;
    }
  }
  int ntile = tileindex-1;
  printf("number of image tile detected from tile_selection image: %d\n", ntile);
  printf("number of actual stack file: %d\n", numStack);
  if (numStack < ntile) {
    printf("warning: number of stack file inconsist with file detected from tile image!\n");
  } else if (numStack > ntile) {
    return NULL;
  }
  int **conn;
  MALLOC_2D_ARRAY(conn, ntile, ntile, int, i);
  for (i = 0; i < ntile; i++) {
    for (j = 0; j < ntile; j++) {
      conn[i][j] = 0;
    }
  }
  printf("tile selection image:\n");
  for (i = 0; i < numTilePerCol; i++) {
    for (j = 0; j < numTilePerRow; j++) {
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
  for (i = 0; i < numTilePerCol; i++) {
    for (j = 0; j < numTilePerRow; j++) {
      if (matrix[i][j] > 0) {
        if (j+1 < numTilePerRow && matrix[i][j+1] > 0) {  //right
          int idx1 = matrix[i][j]-1;
          int idx2 = matrix[i][j+1]-1;
          conn[idx1][idx2] = 1;
          conn[idx2][idx1] = 1;
          config[idx1][idx2] = iarray_calloc(3);
          config[idx1][idx2][0] = -1;
          config[idx1][idx2][1] = 0;
          config[idx1][idx2][2] = 0;
          config[idx2][idx1] = iarray_calloc(3);
          config[idx2][idx1][0] = 1;
          config[idx2][idx1][1] = 0;
          config[idx2][idx1][2] = 0;
        }
        if (i+1 < numTilePerCol && matrix[i+1][j] > 0) {  //down
          int idx1 = matrix[i][j]-1;
          int idx2 = matrix[i+1][j]-1;
          conn[idx1][idx2] = 1;
          conn[idx2][idx1] = 1;
          config[idx1][idx2] = iarray_calloc(3);
          config[idx1][idx2][0] = 0;
          config[idx1][idx2][1] = -1;
          config[idx1][idx2][2] = 0;
          config[idx2][idx1] = iarray_calloc(3);
          config[idx2][idx1][0] = 0;
          config[idx2][idx1][1] = 1;
          config[idx2][idx1][2] = 0;
        }
      }
    }
  }
  return conn;
}

static int **load_conn(char *filepath, int ***config) {
  int ntile;
  FILE *fp = fopen(filepath, "r");

  if (fp == NULL) {
    fprintf(stderr, "Cannot open the connection file.\n");
    return NULL;
  }

  if (fscanf(fp, "%d", &ntile) != 1) {
    return NULL;
  }

  int **conn;
  int i, j;
  MALLOC_2D_ARRAY(conn, ntile, ntile, int, i);
  for (i = 0; i < ntile; i++) {
    for (j = 0; j < ntile; j++) {
      conn[i][j] = 0;
    }
  }
  
  String_Workspace *sw = New_String_Workspace();

  char *line = NULL;
  while ((line = Read_Line(fp, sw)) != NULL) {
    int n;
    int array[5];
    int idx1, idx2;
    String_To_Integer_Array(line, array, &n);
    if (n >= 2) {
      idx1 = array[0];
      idx2 = array[1];
      
      if (IS_IN_CLOSE_RANGE(idx1, 1, ntile) && 
	  IS_IN_CLOSE_RANGE(idx2, 1, ntile)) {
	idx1--;
	idx2--;
	conn[idx1][idx2] = 1;
	conn[idx2][idx1] = 1;

	if (n >= 5) {
	  config[idx1][idx2] = iarray_calloc(3);
	  config[idx1][idx2][0] = array[2];
	  config[idx1][idx2][1] = array[3];
	  config[idx1][idx2][2] = array[4];
	  
	  config[idx2][idx1] = iarray_calloc(3);
	  config[idx2][idx1][0] = -array[2];
	  config[idx2][idx1][1] = -array[3];
	  config[idx2][idx1][2] = -array[4];
	}
      }
    }
  }

  Kill_String_Workspace(sw);

  /*
    int idx1, idx2;
    while (fscanf(fp, "%d %d", &idx1,  &idx2) == 2) {
    if (IS_IN_CLOSE_RANGE(idx1, 1, ntile) && 
    IS_IN_CLOSE_RANGE(idx2, 1, ntile)) {
    conn[idx1 - 1][idx2 - 1] = 1;
    conn[idx2 - 1][idx1 - 1] = 1;
    } else {
    FREE_2D_ARRAY(conn, ntile);
    break;
    }
    }
  */

  fclose(fp);

  if (conn != NULL) { /* test connectivity */
    if (Is_Adjmat_Connected(conn, ntile) == 0) {
      fprintf(stderr, "Tiles are not fully connected.\n");
      FREE_2D_ARRAY(conn, ntile);
    }
  }

  return conn;
}

#if 0
static int lsm_thumbnail_flag(Tiff_IFD *ifd)
{
  int type = 0;
  int count = 0;
  uint32 *val = Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
  
  if (val == NULL) {
    TZ_ERROR(ERROR_DATA_TYPE);
  }

  return *val & TIFF_VALUE_REDUCED_RESOLUTION;
}
#endif

static int skip_argument(char *argv[])
{
  if (strcmp(argv[0], "-merge")) {
    return 2;
  }

  if (strcmp(argv[0], "-conn")) {
    return 2;
  }

  if (strcmp(argv[0], "-fix")) {
    return 1;
  }

  if (strcmp(argv[0], "-d8")) {
    return 2;
  }

  if (strcmp(argv[0], "-bgsub")) {
    return 2;
  }

  if (strcmp(argv[0], "-ds")) {
    return 4;
  }

  if (strcmp(argv[0], "-o")) {
    return 2;
  }

  if (strcmp(argv[0], "-intv")) {
    return 4;
  }

  if (strcmp(argv[0], "-config")) {
    return 4;
  }

  if (strcmp(argv[0], "-layout")) {
    return 3;
  }

  if (strcmp(argv[0], "-channel")) {
    return 3;
  }

  return 1;
}

static int rearrange_arguments(int argc, char *argv[])
{
  int start = -1;
  int end = -1;
  int status = 0;
  
  int i = 1;
  while (i < argc) {
    switch (status) {
    case 0:
      if (argv[i][0] == '-') {
	i += skip_argument(argv + i);
      } else {
	start = i;
	status = 1;
      }
      break;
    case 1:
      if (argv[i][0] != '-') {
	i++;
      } else {
	end = i - 1;
	status = 2;
      }
      break;
    default:
      break;
    }
    
    if (status == 2) {
      break;
    }
  }

  if ((start < 0) || (end < 0)) {
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  int nstack = end - start + 1;
  char *tmp_argv[nstack];
  
  if (end != argc - 1) {
    memcpy(tmp_argv, argv + start, sizeof(char*) * nstack);
    memmove(argv + start, argv + end + 1, sizeof(char*) * (argc - end - 1));
    memcpy(argv + argc - nstack, tmp_argv, sizeof(char*) * nstack);
  }

  return argc - nstack + 1;
}

int main(int argc, char *argv[])
{
  if (Show_Version(argc, argv, "1.65") == 1) {
    return 0;
  }

  static char *Spec[] = {"[-merge <int>] [-tileimage <string>] [-conn <string>] [-fix] [-d8 <int>]",
			 "[-bgsub <int>] [-ds <int> <int> <int>]",
			 "[-intv <int> <int> <int>]",
			 "[-config <int> <int> <int>]",
			 "[-layout <int> <int>] [-channel <int>]",
			 "-o <string>",
			 "<image:string> ...",
			 NULL};

  int k;
  for (k = 0; k < argc; k++) {
    printf("%s ", argv[k]);
  }
  printf("\n");

  int new_argc = argc;
  if (argc >= 25) {
    new_argc = rearrange_arguments(argc, argv);
  }

  Process_Arguments(new_argc, argv, Spec, 1);

  FILE *out_fp;
  if ((out_fp = fopen(Get_String_Arg("-o"), "w")) == NULL) {
    perror("Cannot create output file. Please make sure the ouput folder exists.");
    exit(1);
  }
  fclose(out_fp);

  if (Is_Arg_Matched("-fix")) {
    if (fexist(Get_String_Arg("-o"))) {
      if (Is_Lsm(Get_String_Arg("-o"))) {
	Fix_Lsm_File(Get_String_Arg("-o"));
	return 0;
      }
    }
  }

  int nstack = 0;

  if (new_argc != argc) {
    nstack = argc - new_argc + 1;
  } else {
    nstack = Get_Repeat_Count("image");  
  }

  char **filepath = NULL;

  if (Is_Arg_Matched("-layout")) {
    int tmp_nstack = Get_Int_Arg("-layout", 1) * Get_Int_Arg("-layout", 2);
    if (nstack == 1) {
      nstack = tmp_nstack;
      Mc_Stack *stack = Read_Mc_Stack(Get_String_Arg("image", 0), -1);
      if (stack->depth % tmp_nstack != 0) {
	printf("Invalid layout. Ignore it\n");
      } else {
	filepath = splitstack(stack, Get_String_Arg("image", 0), nstack); 
      }
      Kill_Mc_Stack(stack);
    } else {
      if (tmp_nstack != nstack) {
	printf("Invalid layout. Ignore it\n");
      }
    }
  }

  printf("Stitching %d images ...\n", nstack);

  int **final_offset = NULL;

  if (filepath == NULL) {
    filepath = (char**) malloc(sizeof(char*) * nstack);

    if (new_argc != argc) {
      for (k = 0; k < nstack; k++) {
	filepath[k] = argv[new_argc + k - 1];
      }
    qsort(filepath, nstack, sizeof(char*), compare_string_by_num);
    } else {
      if (fhasext(Get_String_Arg("image", 0), "txt")) {
	free(filepath);
	nstack = load_align(Get_String_Arg("image", 0), &filepath, 
	    &final_offset);
      } else {
	for (k = 0; k < nstack; k++) {
	  filepath[k] = Get_String_Arg("image", k);
	}
    qsort(filepath, nstack, sizeof(char*), compare_string_by_num);
    for (k=0; k<nstack; k++) {
      printf("%s\n", filepath[k]);
    }
      }
    }
  }

  for (k = 0; k < nstack; k++) {
    if (!fexist(filepath[k])) {
      fprintf(stderr, "%s cannot be found\n", filepath[k]);
      fprintf(stderr, "Abort stitching.\n");
      return 1;
    }
  }

  if (nstack == 1) {
    if (Is_Arg_Matched("-ds")) {
      Mc_Stack *stack = Read_Mc_Stack(filepath[0], -1);
      printf("Downsampling ...\n");
      Mc_Stack_Downsample_Mean(stack, Get_Int_Arg("-ds", 1) - 1, 
			       Get_Int_Arg("-ds", 2) - 1, 
			       Get_Int_Arg("-ds", 3) - 1,
			       stack);
      Write_Mc_Stack(Get_String_Arg("-o"), stack, filepath[0]);
    } else {
      if ((Is_Tiff(filepath[0])) && Is_Tiff(Get_String_Arg("-o"))) {
	fcopy(filepath[0], Get_String_Arg("-o"));
      } else if (Is_Lsm(Get_String_Arg("-o")) && Is_Lsm(filepath[0])) {
	fcopy(filepath[0], Get_String_Arg("-o"));
      } else if (Is_Raw(Get_String_Arg("-o")) && Is_Raw(filepath[0])) {
	fcopy(filepath[0], Get_String_Arg("-o"));
      } else {
	Mc_Stack *stack = Read_Mc_Stack(filepath[0], -1);
	Write_Mc_Stack(Get_String_Arg("-o"), stack, NULL);
      }
    }
    printf("%s saved.\n", Get_String_Arg("-o"));
    return 0;
  }

  int **stackSizes;
  stackSizes = malloc(sizeof(int *)*nstack);
  int i;
  for(i=0;i<nstack;i++) {
    stackSizes[i] = (int *) malloc(3*sizeof(int));
  }

  if (final_offset == NULL) {
    /* number of possible pairs */
    int npair = nstack * (nstack + 1);
  
    /* allocate space for correlation scores */
    double *max_corr;
    max_corr = (double *) malloc(sizeof(double)*npair);
    double *unnorm_maxcorr;
    unnorm_maxcorr = (double *) malloc(sizeof(double)*npair);
  
    int **offset;
    MALLOC_2D_ARRAY(offset, npair, 3, int, i);
    /*
      int **offset = malloc(sizeof(int *)*npair);
      int i;
      for(i=0;i<npair;i++) {
      offset[i] = (int *) malloc(3*sizeof(int));
      }
    */

    int **pairs;
    pairs = malloc(sizeof(int *)*npair);
    for(i=0;i<npair;i++) {
      pairs[i] = (int *) malloc(2*sizeof(int));
    }

    int idx = 0;

    Stack **downstacks;
    downstacks = (Stack**)malloc(sizeof(Stack*)*nstack);

    Stack *stack1 = NULL;
    Stack *stack2 = NULL;

    int intv[3] = {3, 3, 3};

    if (Is_Arg_Matched("-intv")) {
      intv[0] = Get_Int_Arg("-intv", 1);
      intv[1] = Get_Int_Arg("-intv", 2);
      intv[2] = Get_Int_Arg("-intv", 3);
    }

    if (Is_Arg_Matched("-ds")) {
      intv[0] /= Get_Int_Arg("-ds", 1);
      intv[1] /= Get_Int_Arg("-ds", 2);
      intv[2] /= Get_Int_Arg("-ds", 3);
    }
  
    int bgsub = -100;
    if (Is_Arg_Matched("-bgsub")) {
      bgsub = Get_Int_Arg("-bgsub");
    }

    int channel = -1;
    if (Is_Arg_Matched("-channel")) {
      channel = Get_Int_Arg("-channel");
    }

    printf("Load stacks ...\n");
    for(i=0;i<nstack;i++) {
      printf("Stack %d ...\n",i);
      stack1 = read_stack_b(filepath[i], bgsub, channel);
      if (Is_Arg_Matched("-ds")) {
	printf("Downsampling ...\n");
	Downsample_Stack_Mean(stack1, Get_Int_Arg("-ds", 1) - 1, 
			      Get_Int_Arg("-ds", 2) - 1, 
			      Get_Int_Arg("-ds", 3) - 1,
			      stack1);
      }

      if (stack1->kind == COLOR) {
	Translate_Stack(stack1, GREY, 1);
      }

      stackSizes[i][0] = stack1->width;
      stackSizes[i][1] = stack1->height;
      stackSizes[i][2] = stack1->depth;
        
      if (bgsub == 100) {
	/*
	  printf("Subtract background ...\n");
	  Print_Stack_Info(stack1);
	*/
	Pixel_Range* pr = Stack_Range(stack1, 0);
	Stack_Sub_Common(stack1, 0, (int)((pr->minval+pr->maxval)/2));
      }

      printf("Downsample stack ...\n");
      if (stack1->depth > 3) {
	downstacks[i] = Downsample_Stack_Mean(stack1,intv[0],intv[1],intv[2], 
					      NULL);
      } else { /* no z downsampling for thin stacks */
	intv[2] =0;
	downstacks[i] = Downsample_Stack_Mean(stack1,intv[0],intv[1],intv[2], 
					      NULL);
      }
    
      if (bgsub == 101) {
	Pixel_Range* pr = Stack_Range(downstacks[i],0);
	Stack_Sub_Common(downstacks[i],0,(int)((pr->minval+pr->maxval)/2));
      }
    
      Kill_Stack(stack1);
    }
    Reset_Stack();

    int j;
    int ***all_config;
    GUARDED_MALLOC_ARRAY(all_config, nstack, int**);
    for (i = 0; i < nstack; i++) {
      GUARDED_MALLOC_ARRAY(all_config[i], nstack, int*);
      for (j = 0; j < nstack; j++) {
	all_config[i][j] = NULL;
      }
    }

    if (Is_Arg_Matched("-config")) {
      if (nstack == 2) {
	all_config[0][1] = iarray_calloc(3);
	all_config[0][1][0] = Get_Int_Arg("-config", 1);
	all_config[0][1][1] = Get_Int_Arg("-config", 2);
	all_config[0][1][2] = Get_Int_Arg("-config", 3);
      }
    }

    int **conn = NULL;

    /*generate connection file from tile_selection.lsm file*/
    if (Is_Arg_Matched("-tileimage")) {
      printf("Geting connection information from tile select image...\n");
      conn = get_conn_from_image(Get_String_Arg("-tileimage"), all_config, nstack);
      if (conn == NULL) {
	fprintf(stderr, "Failed to detect connection from image: %s\n", 
		Get_String_Arg("-tileimage"));
	fprintf(stderr, "Continue stitching with full connection...\n");
      }
    }
    /* load connection file if it is specified */
    if (Is_Arg_Matched("-conn")) {
      printf("Loading connection file...\n");
      conn = load_conn(Get_String_Arg("-conn"), all_config);
      if (conn == NULL) {
	fprintf(stderr, "Failed to load connection file: %s\n", 
		Get_String_Arg("-conn"));
	fprintf(stderr, "Continue stitching with full connection...\n");
      }
    } else if (Is_Arg_Matched("-layout")) {
      MALLOC_2D_ARRAY(conn, nstack, nstack, int, i);
      for (i = 0; i < nstack; i++) {
	for (j = 0; j < nstack; j++) {
	  conn[i][j] = 0;
	}
      }
      int row = Get_Int_Arg("-layout", 1);
      int col = Get_Int_Arg("-layout", 2);

      int neighbor[4];
      int is_in_bound[4];
      Stack_Neighbor_Offset(4, row, col, neighbor);
      for (i = 0; i < nstack; i++) {
	int nbound = Stack_Neighbor_Bound_Test_I(4, row, col, 1, i,
	    is_in_bound); 
	if (nbound == 4) {
	  for (j = 0; j < 4; j++) {
	    int nbr = i + neighbor[j];
	    if (nbr > i) {
	      conn[i][nbr] = 1;
	    }
	  }
	} else {
	  for (j = 0; j < 4; j++) {
	    int nbr = i + neighbor[j];
	    if (nbr > i) {
	      if (is_in_bound[j]) {
		conn[i][nbr] = 1;
	      }
	    }
	  }
	}
      }
      

#ifdef _DEBUG_2
      for (i = 0; i < nstack; i++) {
	for (j = 0; j < nstack; j++) {
	  printf("%d ", conn[i][j]);
	}
	printf("\n");
      }

      return 1;
#endif
    }

#ifdef _DEBUG_
    tic();
#endif
    
    /* rough estimation */
    for(i=0;i<nstack-1;i++) {
      for(j=i+1;j<nstack;j++) {
	if ((conn == NULL) || (conn[i][j] == 1)) {
	  max_corr[idx] = Align_Stack_MR_D(downstacks[i],downstacks[j],intv,
					   -1, all_config[i][j], offset[idx],
					   unnorm_maxcorr+idx);
	  max_corr[idx] = -max_corr[idx];
      
	  pairs[idx][0] = i;
	  pairs[idx][1] = j;
	
	  printf("(%d,%d) : (%d,%d,%d) : %f\n",pairs[idx][0],pairs[idx][1],
		 offset[idx][0],offset[idx][1],offset[idx][2],max_corr[idx]);

	  idx++;
	}
      }
      Kill_Stack(downstacks[i]);
    }
    Kill_Stack(downstacks[nstack-1]);
    free(downstacks);

    /* actual number of pairs */
    npair = idx;

    int *permidx;
    permidx = (int *) malloc(sizeof(int)*npair);
    for(i=0;i<npair;i++) {
      permidx[i] = i;
    }
    int* labels;
    labels = (int *) malloc(sizeof(int)*nstack);
    for(i=0;i<nstack;i++) {
      labels[i] = 0;
    }
    int **selpairs;
    selpairs = malloc(sizeof(int*) * (nstack-1));
    for(i=0;i<nstack-1;i++)
      selpairs[i] = (int*) malloc(sizeof(int)*2);

    int **seloffset;
    seloffset = malloc(sizeof(int*) * (nstack-1));
    for(i=0;i<nstack-1;i++)
      seloffset[i] = (int*) malloc(sizeof(int)*3);

    darray_qsort(max_corr,permidx,npair);

    idx = 0;
    labels[pairs[permidx[0]][0]] = 1;

    final_offset = malloc(sizeof(int*) * nstack);
    for(i=0;i<nstack;i++) {
      final_offset[i] = iarray_calloc(3);
    }
  
    i = 0;
    while(idx<nstack-1) {
      //one and only one idx has not been added
      if(labels[pairs[permidx[i]][0]] != labels[pairs[permidx[i]][1]]) {
	printf("(%d,%d)\n",pairs[permidx[i]][0],pairs[permidx[i]][1]);

	selpairs[idx][0] = pairs[permidx[i]][0];
	selpairs[idx][1] = pairs[permidx[i]][1];
	seloffset[idx][0] = offset[permidx[i]][0];
	seloffset[idx][1] = offset[permidx[i]][1];
	seloffset[idx][2] = offset[permidx[i]][2];

	int v1 = selpairs[idx][0];
	int v2 = selpairs[idx][1];

	stack1 = read_stack_b(filepath[v1], 0, channel);
	Translate_Stack(stack1, GREY, 1);
	stack2 = read_stack_b(filepath[v2], 0, channel);
	Translate_Stack(stack2, GREY, 1);
	if (Is_Arg_Matched("-ds")) {
	  Downsample_Stack_Mean(stack1, Get_Int_Arg("-ds", 1) - 1, 
				Get_Int_Arg("-ds", 2) - 1, 
				Get_Int_Arg("-ds", 3) - 1,
				stack1);
	  Downsample_Stack_Mean(stack2, Get_Int_Arg("-ds", 1) - 1, 
				Get_Int_Arg("-ds", 2) - 1, 
				Get_Int_Arg("-ds", 3) - 1,
				stack2);
	}
      
	Align_Stack_MR_D(stack1, stack2, intv, 2, NULL, seloffset[idx], NULL);
	/*
	  if (stack1->depth > 3) {
	  Align_Stack_MR_F(stack1,stack2,intv,2,seloffset[idx],NULL);
	  } else {
	  float unnorm_corr;
	  Align_Stack_F(stack1,stack2,seloffset[idx],&unnorm_corr);
	  }
	*/
	if (labels[pairs[permidx[i]][0]] == 1) {
	  final_offset[v2][0] = 
	    final_offset[v1][0] - seloffset[idx][0] + stack1->width - 1;
	  final_offset[v2][1] =
	    final_offset[v1][1] - seloffset[idx][1] + stack1->height - 1;
	  final_offset[v2][2] = 
	    final_offset[v1][2] - seloffset[idx][2] + stack1->depth - 1;
	} else {
	  final_offset[v1][0] = 
	    final_offset[v2][0] + seloffset[idx][0] - stack2->width + 1;
	  final_offset[v1][1] =
	    final_offset[v2][1] + seloffset[idx][1] - stack2->height + 1;
	  final_offset[v1][2] = 
	    final_offset[v2][2] + seloffset[idx][2] - stack1->depth + 1;
	}

	Free_Stack(stack1);
	Free_Stack(stack2);

	labels[pairs[permidx[i]][0]] = 1;
	labels[pairs[permidx[i]][1]] = 1;
	idx++;
	i = 0;
      }
      i++;
    }

#ifdef _DEBUG_
    printf("Time passed: %lld\n", toc());
#endif

  }

  Reset_Stack();

  if (fhasext(Get_String_Arg("-o"), "txt")) {
    FILE *fp = GUARDED_FOPEN(Get_String_Arg("-o"), "w");
    int i;
    for (i = 0; i < nstack; i++) {
      fprintf(fp, "%s ", filepath[i]);
      fprintf(fp, "(%d,%d,%d) (%d,%d,%d)\n", 
	      final_offset[i][0], final_offset[i][1], final_offset[i][2],
	      stackSizes[i][0], stackSizes[i][1], stackSizes[i][2]);
    }
    fclose(fp);
  } else {
    int merge_mode = 4;
    if (Is_Arg_Matched("-merge")) {
      merge_mode = Get_Int_Arg("-merge");
    }

    BOOL large_stack = TRUE;
    Mc_Stack *new_stack = NULL;
  
    for (i = 0; i < nstack; i++) {
      if (Is_Arg_Matched("-ds")) {
	int j;
	for (j = 0; j < 3; j++) {
	  if (final_offset[i][j] < 0) {
	    final_offset[i][j]--;
	  }
	  final_offset[i][j] /= Get_Int_Arg("-ds", j + 1);
	}
      }
      printf("(%d,%d,%d)\n", final_offset[i][0], final_offset[i][1],
	     final_offset[i][2]);
    }

    if (large_stack == FALSE) {
      Mc_Stack **stacks = (Mc_Stack **) malloc(sizeof(Mc_Stack*) * nstack);
    
      for (i = 0; i < nstack; i++) {
	stacks[i] = Read_Mc_Stack(filepath[i], -1);
      }

      new_stack = Mc_Stack_Merge(stacks, nstack, final_offset,
				 merge_mode);
    } else {
      if (Is_Arg_Matched("-ds")) {
	int ds[3];
	ds[0] = Get_Int_Arg("-ds", 1);
	ds[1] = Get_Int_Arg("-ds", 2);
	ds[2] = Get_Int_Arg("-ds", 3);
    
	new_stack = Mc_Stack_Merge_F(filepath, nstack, final_offset,
				     merge_mode, ds);
      } else {
	new_stack = Mc_Stack_Merge_F(filepath, nstack, final_offset,
				     merge_mode, NULL);      
      }
    }    
  
    if (Is_Arg_Matched("-d8")) {
      if (new_stack->kind == 2) {
	Mc_Stack_Grey16_To_8(new_stack, Get_Int_Arg("-d8"));
      }
    }

    Write_Mc_Stack(Get_String_Arg("-o"), new_stack, filepath[0]);
    printf("%s saved.\n", Get_String_Arg("-o"));
  }

				       
#if 0
  Stack **stacks = (Stack **) malloc(sizeof(Stack*) * nstack);

  for (i = 0; i < nstack; i++) {
    Mc_Stack *mc_stack = Read_Mc_Stack(Get_String_Arg("image", i), -1);
    if (mc_stack->nchannel > 1) {
      stacks[i] = Mc_Stack_To_Stack(mc_stack, COLOR, NULL);
    } else {
      stacks[i] = Mc_Stack_To_Stack(mc_stack, -1, NULL);
    }
    Kill_Mc_Stack(mc_stack);
    //stacks[i] = read_stack(Get_String_Arg("image", i));
    printf("(%d,%d,%d)\n", final_offset[i][0], final_offset[i][1],
	   final_offset[i][2]);
  }
  
  int merge_mode = 4;
  if (Is_Arg_Matched("-merge")) {
    merge_mode = Get_Int_Arg("-merge");
  }

  Stack *new_stack = Stack_Merge_M(stacks, nstack, final_offset, merge_mode,
				   NULL);

  /*
    if (Is_Lsm(Get_String_Arg("image", 1))) {
    Tiff_IFD *ifd;
    Tiff_Reader *reader;
    
    reader = Open_Tiff_Reader(Get_String_Arg("image", 1),NULL,1);
    
    while (lsm_thumbnail_flag(ifd = Read_Tiff_IFD(reader)) != 0) {
    Free_Tiff_IFD(ifd);
    if (End_Of_Tiff(reader)) {
    ifd = NULL;
    TZ_ERROR(ERROR_IO_READ);
    break;
    }
    }

    Write_Lsm_Stack(Get_String_Arg("-o"), new_stack, ifd);

    } else {
    Write_Stack(Get_String_Arg("-o"), new_stack);
    }
  */

  Write_Stack_U(Get_String_Arg("-o"), new_stack, Get_String_Arg("image", 0));
#endif

#ifdef _DEBUG_2
  Kill_Stack(new_stack);
  final_offset[1][1] += 1;
  new_stack = Stack_Merge_M(stacks, nstack, final_offset, merge_mode);
  Write_Stack_U("../data/test1.tif", new_stack, Get_String_Arg("image", 0));

  Kill_Stack(new_stack);
  final_offset[1][1] += 1;
  new_stack = Stack_Merge_M(stacks, nstack, final_offset, merge_mode);
  Write_Stack_U("../data/test2.tif", new_stack, Get_String_Arg("image", 0));

  Kill_Stack(new_stack);
  final_offset[1][1] += 1;
  new_stack = Stack_Merge_M(stacks, nstack, final_offset, merge_mode);
  Write_Stack_U("../data/test3.tif", new_stack, Get_String_Arg("image", 0));

  Kill_Stack(new_stack);
  final_offset[1][1] += 1;
  new_stack = Stack_Merge_M(stacks, nstack, final_offset, merge_mode);
  Write_Stack_U("../data/test4.tif", new_stack, Get_String_Arg("image", 0));
#endif

  /*
    if (Is_Lsm(Get_String_Arg("-o"))) {
    Fix_Lsm_File(Get_String_Arg("-o"));
    }
  */
  return 0;
}
