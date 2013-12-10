/* imbinarize.c
 *
 * 14-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <stdio.h>
#include <utilities.h>
#include <string.h>
#include "tz_error.h"
#include "tz_int_histogram.h"
#include "tz_stack_threshold.h"
#include "tz_stack_lib.h"
#include "tz_stack_stat.h"
#include "tz_stack_attribute.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack.h"
#include "tz_stack_attribute.h"
#include "tz_stack_objlabel.h"
#include "tz_iarray.h"
#include "tz_image_io.h"

/*
 * imbinarize - binarize a tif image or stack
 *
 * imbinarize [-m method] ([-t thre] | [-l low] [-h high]) infile -o outfile
 */
int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-M<string>] ([-t <int>] | [-l <int>] [-h <int>])", 
			 "[-R<string>] [-retry <int>] [-rsobj <int>]",
			 " <image:string>",
			 " -o <string>",
			 NULL};

  Process_Arguments(argc, argv, Spec, 1);

  char *image_file = Get_String_Arg("image");
  
  Stack *stack = Read_Stack_U(image_file);
  Stack *refstack = stack;
  if (Is_Arg_Matched("-R")) {
    if (strcmp(Get_String_Arg("-R"), image_file) != 0) {
      refstack = Read_Stack(Get_String_Arg("-R"));
    }
  }

  const char *method = "hist_rc";
  if (Is_Arg_Matched("-M")) {
    method = Get_String_Arg("-M");
  }

  int thre = 0;
  int *hist = NULL;

  printf("Thresholding %s ...\n", image_file);

  Stack *locmax = NULL;  
  int low, high;

  if (strlen(method) >= 4) {
    if (strncmp(method, "hist", 4) == 0) {
      if (method[4] == '\0') { /* manual thresholding */
	thre = Get_Int_Arg("-t");
      } else {
	if (method[4] == '_') { /* auto thresholding */
	  hist = Stack_Hist(refstack);
	  int low, high;
	  Int_Histogram_Range(hist, &low, &high);
	  if (Is_Arg_Matched("-l")) {
	    low = Get_Int_Arg("-l");
	  }
	  if (Is_Arg_Matched("-h")) {
	    high = Get_Int_Arg("-h");
	  }
	  
	  if (strcmp(method + 5, "rc") == 0) { /* RC threshold */
	    thre = Int_Histogram_Rc_Threshold(hist, low, high);
	  } else if (strcmp(method + 5, "sp") == 0) { /* stable point */
	    thre = Int_Histogram_Stable_Point(hist, low, high);
	  } else if (strcmp(method + 5, "tr") == 0) { /* triangle */
	    thre = Int_Histogram_Triangle_Threshold(hist, low, high);
	  } else {
	    Kill_Stack(stack);
	    free(hist);
	    fprintf(stderr, "Invalid argument.");
	    return 1;
	  }
	}
      }
      Stack_Threshold(stack, thre);
    } else if (strcmp(method, "locmax") == 0) {
      if (stack == refstack) {
	stack = Copy_Stack(refstack);
      }

#if 0
      locmax = Stack_Local_Max(refstack, NULL, STACK_LOCMAX_SINGLE);
#else      
      int conn = 18;
      locmax = Stack_Locmax_Region(refstack, conn);
      Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
      int nvoxel = Stack_Voxel_Number(locmax);
      int i;

      for (i = 0; i < nvoxel; i++) {
	if (locmax->array[i] < 3) {
	  locmax->array[i] = 0;
	} else {
	  locmax->array[i] = 1;
	}
      }
#endif      
      hist = Stack_Hist_M(refstack, locmax);

#ifdef _DEBUG_
      printf("%d %d\n", hist[0], hist[1]);
      iarray_write("../data/test.bn", hist, Int_Histogram_Length(hist) + 2);
#endif

      Kill_Stack(locmax);

      Int_Histogram_Range(hist, &low, &high);
      if (Is_Arg_Matched("-l")) {
	low = Get_Int_Arg("-l");
      }
      if (Is_Arg_Matched("-h")) {
	high = Get_Int_Arg("-h");
      }

      thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);
      printf("Threshold: %d\n", thre);
      //Stack_Threshold(stack, thre);
    } else if (strcmp(method, "level") == 0) {
	int low = Get_Int_Arg("-t");
     
        Stack_Binarize_Level(stack, low);
        char *out_file = Get_String_Arg("-o");
        Write_Stack(out_file, stack);
        printf("%s created.\n", out_file);

        return 0;
    }
  }
 

  double fgratio = (double) Stack_Fgarea_T(stack, thre) /
    Stack_Voxel_Number(stack);
  printf("Foreground: %g\n", fgratio);

  int succ = 1;


  double ratio_low_thre = 0.01;
  double ratio_thre = 0.05;
  int thre2;
  double fgratio2 = fgratio;
  double prev_fgratio = fgratio;

  if ((Is_Arg_Matched("-retry")) && (strcmp(method, "locmax") == 0)) {
    int nretry = Get_Int_Arg("-retry");
    if ((fgratio > ratio_low_thre) && (fgratio <= ratio_thre)) {
      thre2 = Int_Histogram_Triangle_Threshold(hist, thre + 1, high - 1);
      fgratio2 = (double) Stack_Fgarea_T(stack, thre2) /
	Stack_Voxel_Number(stack);
      printf("Threshold: %d\n", thre2);
      printf("Foreground: %g\n", fgratio2);
      if (fgratio2 / fgratio <= 0.3) {
	thre = thre2;
      }
    } else {
      thre2 = thre;
      while (fgratio2 > ratio_thre) {      
	//ASSERT(locmax != NULL, "bug found");
	printf("Bad threshold, retrying ...\n");
	
	thre2 = Int_Histogram_Triangle_Threshold(hist, thre2 + 1, high - 1);
	printf("Threshold: %d\n", thre2);
	
	//stack = Read_Stack(image_file);
	//Stack_Threshold(stack, thre);
	fgratio2 = (double) Stack_Fgarea_T(stack, thre2) /
	  Stack_Voxel_Number(stack);
	if (fgratio2 / prev_fgratio <= 0.5) {
	  thre = thre2;
	}
	prev_fgratio = fgratio2;

	printf("Foreground: %g\n", fgratio2);
	
	nretry--;
	
	if (nretry == 0) {
	  break;
	}
      }

      if (fgratio2 > ratio_thre * 4.0) {
	succ = 0;
      }/* else if (fgratio2 / fgratio <= 0.5) {
	thre = thre2;
	}  */    
    }
  }

 
  if (succ == 0) {
    PRINT_EXCEPTION("Thresholding error", "The threshold seems wrong.");
    printf("Try it anyway.\n");
  } 
  {
    succ = 1;
    printf("Binarizing with threshold %d ...\n", thre);
    Stack_Threshold(stack, thre);
    Stack_Binarize(stack);
    
    if (stack->kind != GREY) {
      Translate_Stack(stack, GREY, 1);
    }
    
    if (Is_Arg_Matched("-rsobj")) {
      stack = Stack_Remove_Small_Object(Copy_Stack(stack), stack, 
					Get_Int_Arg("-rsobj"), 26);
    }
    
    if (stack->kind != GREY) {
      Translate_Stack(stack, GREY, 1);
    }
    
    char *out_file = Get_String_Arg("-o");
    Write_Stack(out_file, stack);
    printf("%s created.\n", out_file);
  }

  if (refstack != stack) {
    Kill_Stack(refstack);
  }

  Kill_Stack(stack);
  if (hist != NULL) {
    free(hist);
  }

  return (!succ);
}
