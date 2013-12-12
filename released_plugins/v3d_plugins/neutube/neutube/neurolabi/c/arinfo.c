/* arinfo.c
 *
 * 28-Oct-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <sys/stat.h>
#include <utilities.h>
#include "tz_utilities.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<file:string>", "[-u <int>]", NULL};

  Process_Arguments(argc, argv, Spec, 1);

  if (!fexist(Get_String_Arg("file"))) {
    printf("%s does not exist.\n", Get_String_Arg("file"));
    return 1;
  }

  struct stat buf;

  stat(Get_String_Arg("file"), &buf);

  int file_size = buf.st_size;
  printf("file size: %d\n", file_size);

  FILE *fp = fopen(Get_String_Arg("file"), "r");

  int length;

  if (fp != NULL) {
    if (fread(&length, sizeof(int), 1, fp) != 1) {
      printf("Wrong file format.\n");
      return 1;
    } else {
      if (((file_size - sizeof(int)) % length) != 0) {
	printf("Bad array file.\n");
	return 1;
      } else {
	if (Is_Arg_Matched("-u")) {
	  int unit = Get_Int_Arg("-u");
	  if (length * unit != file_size - sizeof(int)) {
	    printf("Bad array file.\n");
	    return 1;
	  }
	}
      }
    }

    fclose(fp);
  }

  printf("%d elements (%lu)\n", length, (file_size - sizeof(int)) / length);

  return 0;
}
