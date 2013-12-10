/**@file hex.c
 * @brief >> hex number convertion
 * @author Ting Zhao
 * @date 24-Feb-2009
 */

#include <stdlib.h>
#include <utilities.h>
#include "tz_utilities.h"

/*
 * hex x00AB or hex 234
 * hex add 
 */

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[<number:string> | -a <num1:string> <num2:string>]", 
			 NULL};
  Process_Arguments(argc, argv, Spec, 1);

  if (Is_Arg_Matched("-a")) {
    char *numstr1 = Get_String_Arg("num1");
    char *numstr2 = Get_String_Arg("num2");
    uint32_t num1;
    uint32_t num2;
    if (numstr1[0] == 'x') {
      num1 = Hexstr_To_Uint(numstr1 + 1);
    } else {
      num1 = atoi(numstr1);
    }
    if (numstr2[0] == 'x') {
      num2 = Hexstr_To_Uint(numstr2 + 1);
    } else {
      num2 = atoi(numstr2);
    }
    
    char str[12];
    printf("%s\n", Uint_To_Hexstr(num1 + num2, str));
  } else {
    char *numstr = Get_String_Arg("number");
    if (numstr[0] == 'x') {
      printf("%u\n", Hexstr_To_Uint(numstr + 1));
    } else {
      char str[12];
      printf("%s\n", Uint_To_Hexstr(atoi(numstr), str));
    }
  }

  return 0;
}
