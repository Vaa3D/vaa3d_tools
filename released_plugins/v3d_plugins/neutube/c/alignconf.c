#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <image_lib.h>
#include "tz_darray.h"
#include "tz_stack_stat.h"
#include "tz_fimage_lib.h"
#include "tz_utilities.h"
#include "tz_string.h"

#include "private/alignconf.c"

int main(int argc, const char *argv[])
{
  const char *dir_name = "/Users/zhaot/Data/nathan/2008-04-18";

  if (argc > 1) {
    dir_name = argv[1];
  }

  const char *result_dir = "/Users/zhaot/Work/V3D/neurolabi/data/align";

  char filelist[100];

  fullpath(result_dir, "filelist.txt", filelist);
  
  printf("%s\n", filelist);

  char file_path[150];
  char image_path[150];

  /* Read all files from the directory */
  FILE *fp = fopen(filelist, "r");
  char id[100];
  int idx = 0;
  int idx2 = 0;
  int row = 22;
  int col = 15;

#define MAX_FILE_NUMBER 364
  char all_file[MAX_FILE_NUMBER][100];
  double stack_mean[MAX_FILE_NUMBER];
  int available[row * col];

  while (Read_Word(fp, all_file[idx++], 0) > 0) {
    fullpath(dir_name, all_file[idx - 1], image_path);
    printf("%d, %s\n", idx, image_path);
  }
 
  fullpath(result_dir, "stack_mean.bn", file_path);
  if (!fexist(file_path)) {
    for (idx = 1; idx <= MAX_FILE_NUMBER; idx++) {
      fullpath(dir_name, all_file[idx - 1], image_path);
      Stack *stack = Read_Stack(image_path);
      stack_mean[idx - 1] = Stack_Mean(stack);
      printf("%d, %s: %g\n", idx, image_path, stack_mean[idx - 1]);
      Kill_Stack(stack);
    } 
    darray_write(file_path, stack_mean, MAX_FILE_NUMBER);
  } else {
    int array_length;
    darray_read(file_path, stack_mean, &array_length);
  }
  
  if (idx - 1 != MAX_FILE_NUMBER) {
    fprintf(stderr, "Wrong file number: %d.\n", idx);
  }

  const double threshold = 115.0;

  for (idx = 0; idx < MAX_FILE_NUMBER; idx++) {
    if (stack_mean[idx] > threshold) {
      available[idx] = 1;
    } else {
      available[idx] = 0;
    }
  }

  for (idx = MAX_FILE_NUMBER; idx < row * col; idx++) {
    available[idx] = 0;
  }

  fullpath(result_dir, "align_input.txt", file_path);
  
  fclose(fp);


  int neighbor[] = {-1, 1, -22, 22};

  fp = fopen(file_path, "w");

  for (idx = 1; idx < MAX_FILE_NUMBER; idx++) {
    if (idx % row != 0) {
      if (available[idx - 1] && available[idx]) {
	fprintf(fp, "%s %s\n", all_file[idx - 1], all_file[idx]);
	align_id(all_file[idx - 1], all_file[idx], id);
	printf("%s\n", id);
      }
    }
  }

  int i, j;
  int offset = 0;
  for (j = 0; j < col - 1; j++) {
    offset = row * j;
    for (i = 0; i < row; i++) {
      idx = offset + i;
      idx2 = idx + row;
      if ((available[idx]) && (available[idx2])) {
	fprintf(fp, "%s %s\n", all_file[idx], all_file[idx2]);
	align_id(all_file[idx], all_file[idx2], id);
	printf("%s\n", id);
	break;
      }
    }
  }

  fclose(fp);

  return 1;
}
