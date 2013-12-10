/**@file flyem_range.c
 * @author Ting Zhao
 * @date 10-Aug-2012
 */

#include "tz_utilities.h"
#include "tz_string.h"
#include "tz_utilities.h"


int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);


  String_Workspace *sw = New_String_Workspace();
  FILE *fp = fopen(Get_String_Arg("input"), "r");
  char *line;
  int start = 10000;
  int end = 0;

  while((line = Read_Line(fp, sw)) != NULL) {
    int value = String_First_Integer(line);
    if (value < start) {
      start = value;
    }
    if (value > end) {
      end = value;
    }
  }
  fclose(fp);

  printf("%d %d\n", start, end);

  return 0;
}
