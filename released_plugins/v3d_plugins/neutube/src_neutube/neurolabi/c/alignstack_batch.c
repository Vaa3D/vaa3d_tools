#include <stdio.h>
#include <string.h>
#include <image_lib.h>
#include <gsl/gsl_fit.h>
#include "tz_image_lib.h"
#include "tz_string.h"
#include "tz_fimage_lib.h"
#include "tz_error.h"
#include "tz_darray.h"

#include "private/alignstack.c"
#include "private/alignconf.c"

int main(int argc, const char *argv[])
{
  const char *result_dir = "/Users/zhaot/Work/V3D/neurolabi/data/align";
  const char *image_dir = "/Users/zhaot/Data/nathan/2008-04-18";

  char file_path[150];
  char image_path[150];
  char result_path[150];
  char file1[100], file2[100];
  char id[100];

  fullpath(result_dir, "align_input.txt", file_path);  

  FILE *fp = fopen(file_path, "r");

  while ((Read_Word(fp, file1, 0) > 0) && 
	 (Read_Word(fp, file2, 0) > 0)) {
    align_id(file1, file2, id);
    strcat(id, ".txt");
    fullpath(result_dir, id, result_path);
    if (!fexist(result_path)) {
      printf("%s, %s\n", file1, file2);

      fullpath(image_dir, file1, image_path);
      Stack *stack1 = Read_Stack(image_path);
      fullpath(image_dir, file2, image_path);
      Stack *stack2 = Read_Stack(image_path);

      // Stack_Threshold_Tp4(stack1, 0, 65535);
      //Stack_Threshold_Tp4(stack2, 0, 65535);
      int chopoff1 = estimate_chopoff(stack1);
      printf("%d\n", chopoff1);
      Stack *substack1 = Crop_Stack(stack1, 0, 0, chopoff1, 
				    stack1->width, stack1->height,
				    stack1->depth - chopoff1, NULL);
      int chopoff2 = estimate_chopoff(stack2);
      printf("%d\n", chopoff2);
      Stack *substack2 = Crop_Stack(stack2, 0, 0, chopoff2, 
				    stack2->width, stack2->height,
				    stack2->depth - chopoff2, NULL);

      float unnorm_maxcorr;
      int offset[3];
      
      int intv[] = {3, 3, 3};
      float score = Align_Stack_MR_F(substack1, substack2, intv, 1, offset, 
				     &unnorm_maxcorr);
  
      if (score <= 0.0) {
	printf("failed\n");
	write_align_result(result_dir, file1, file2, NULL);
      } else {
	offset[2] += chopoff2;
	printf("(%d, %d, %d): %g\n", offset[0], offset[1], offset[2], score);
	write_align_result(result_dir, file1, file2, offset);	
      }

      Kill_Stack(substack1);
      Kill_Stack(substack2);
      Kill_Stack(stack1);
      Kill_Stack(stack2);
    }
  }

  fclose(fp);

  return 0;
}
