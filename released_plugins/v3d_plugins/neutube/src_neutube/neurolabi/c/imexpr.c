#include <utilities.h>
#include "tz_stack_math.h"
#include "tz_image_io.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {" <image1:string> <opr:string> <image2:string>",
			 "-o <string>",
			 NULL};

  Process_Arguments(argc, argv, Spec, 1);

  char *image1 = Get_String_Arg("image1");
  char *image2 = Get_String_Arg("image2");

  Stack *stack1 = Read_Stack_U(image1);
  Stack *stack2 = Read_Stack_U(image2);
  
  char opr = *Get_String_Arg("opr");

  switch (opr) {
  case '+':
    Stack_Add(stack1, stack2, stack1);
    break;
  case '\\':
    Stack_Sub(stack1, stack2, stack1);
    break;
  case '|':
    Stack_Or_E(stack1, stack2, stack1);
    break;    
  case '&':
    Stack_And_E(stack1, stack2, stack1);
    break;  
  case '^':
    Stack_Xor(stack1, stack2, stack1);
    break;
  default:
    fprintf(stderr, "Invalid operator: %c", opr);
    return 1;
  }

  Write_Stack_U(Get_String_Arg("-o"), stack1, NULL);

  return 0;
}
