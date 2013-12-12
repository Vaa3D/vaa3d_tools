#include <stdio.h>
#include <string.h>
#include <image_lib.h>
#include "tz_string.h"
#include "tz_fimage_lib.h"

#include "private/alignconf.c"

int main(int argc, const char *argv[])
{
  const char *result_dir = "/Users/zhaot/Work/V3D/neurolabi/data/align";
  const char *image_dir = "/Users/zhaot/Data/nathan/2008-04-18";

  char file_path[150];
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
      printf("Unaligned stacks: %s, %s", file1, file2);
    } else {
      FILE *result_fp = fopen(result_path, "r");
      char first_word[100];
      Read_Word(result_fp, first_word, 0);
      if (strcmp(first_word, "failed") == 0) {
	printf("Failed stacks: %s, %s", file1, file2);
      } else {
	printf("%s\n", first_word);
      }
      fclose(result_fp);
    }
  }

  fclose(fp);

  return 0;
}
