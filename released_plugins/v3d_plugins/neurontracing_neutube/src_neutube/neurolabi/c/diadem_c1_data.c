/**@file diadem_c1_data.c
 * @author Ting Zhao
 * @date 05-Apr-2010
 */

#include <stdio.h>
#include "tz_utilities.h"
#include "tz_string.h"
#include "tz_image_io.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string> -tile_number <int>", 
    "-align <string>", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  char filepath[500];
  int i;
  int ntile = Get_Int_Arg("-tile_number");
  for (i = 1; i <= ntile; i++) {
    sprintf(filepath, "%s/diadem_c1_%02d.xml", Get_String_Arg("-o"), i);
    FILE *fp = fopen(filepath, "w");
    
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<trace>\n");
    fprintf(fp, "<data>\n");
    fprintf(fp, "<image type=\"bundle\">\n");
    //fprintf(fp, "<prefix>%s/Image Stacks/0%d/</prefix>\n", Get_String_Arg("input"), i);
    fprintf(fp, "<prefix>%s/%02d/</prefix>\n", Get_String_Arg("input"), i);
    fprintf(fp, "<suffix>.tif</suffix>\n");
    fprintf(fp, "<num_width>2</num_width>\n");
    fprintf(fp, "<first_num>1</first_num>\n");
    fprintf(fp, "</image>\n");
    //fprintf(fp, "<offset><x>73</x><y>507</y><z>-5</z></offset>\n");
    fprintf(fp, "<resolution><x>0.2941</x><y>0.2941</y><z>1</z></resolution>\n");
    fprintf(fp, "</data>\n");
    /*
    fprintf(fp, "<output>\n");
    fprintf(fp, "<workdir>/Users/zhaot/Work/neurolabi/data</workdir>\n");
    fprintf(fp, "<name>diadem_c1_01</name>\n");
    fprintf(fp, "</output>\n");
    */
    fprintf(fp, "</trace>\n");
  
    fclose(fp);
  }

  //char offset_file[500];
  //sprintf(offset_file, "%s/diadem_c1_offset.txt", Get_String_Arg("-o"));
  char *offset_file = Get_String_Arg("-align");
  FILE *offset_fp = fopen(offset_file, "r");
  String_Workspace *sw = New_String_Workspace();
  char *line = NULL;
  int n = 1;
  
  sprintf(filepath, "%s/diadem_c1_align.txt", Get_String_Arg("-o"));
  FILE *fp = fopen(filepath, "w");
  int array[10];

  while ((line = Read_Line(offset_fp, sw)) != NULL) {
    int m;
    String_To_Integer_Array(line, array, &m); 
    char stack_file[500];
    if (m == 3) {
      sprintf(stack_file, "%s/diadem_c1_%02d.xml", Get_String_Arg("-o"), n);
      Stack *stack = Read_Stack_U(stack_file);
      if (stack == NULL) {
	fprintf(stderr, "failed to read %s\n", stack_file);
	return 1;
      }
      fprintf(fp, "%s (%d,%d,%d) (%d,%d,%d)\n", 
	  stack_file, array[0], array[1], array[2], 
	  stack->width, stack->height, stack->depth);
      n++;
    }
  }
  /*
  fprintf(fp, "%s/diadem_c1_01.xml (73,507,-5) (512,512,60)\n", Get_String_Arg("-o"));
  fprintf(fp, "%s/diadem_c1_02.xml (526,484,11) (512,512,33)\n", Get_String_Arg("-o"));
  fprintf(fp, "%s/diadem_c1_03.xml (952,462,-21) (512,512,44)\n", Get_String_Arg("-o"));
  fprintf(fp, "%s/diadem_c1_04.xml (924,3,-19) (512,512,51)\n", Get_String_Arg("-o"));
  fprintf(fp, "%s/diadem_c1_05.xml (468,-14,-1) (512,512,50)\n", Get_String_Arg("-o"));
  fprintf(fp, "%s/diadem_c1_06.xml (0,0,0) (512,512,46)\n", Get_String_Arg("-o"));
*/
  Kill_String_Workspace(sw);
  fclose(offset_fp);
  fclose(fp);

  return 0;
}
