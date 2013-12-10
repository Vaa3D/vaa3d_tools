/**@file diadem_d1_data.c
 * @author Ting Zhao
 * @date 07-Apr-2010
 */

#include "tz_utilities.h"
#include "tz_mc_stack.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"
#include "tz_string.h"

static void upsample(const char *path, int id)
{
  static char filepath[500];

  sprintf(filepath, "%s/%03d.xml", path, id);
  printf("Upsampling %s ...\n", filepath);
  Mc_Stack *stack = Read_Mc_Stack(filepath, 0);
  Mc_Stack *out = Mc_Stack_Upsample(stack, 0, 0, 1, NULL);
  sprintf(filepath, "%s/%03d.tif", path, id);
  Write_Mc_Stack(filepath, out, NULL);
  Kill_Mc_Stack(stack);
  Kill_Mc_Stack(out);

  sprintf(filepath, "%s/%03d.xml", path, id);
  FILE *fp = GUARDED_FOPEN(filepath, "w");
  fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(fp, "<trace>\n");
  fprintf(fp, "<data>\n");
  fprintf(fp, "<image type=\"tif\">\n");
  fprintf(fp, "<url>%s/%03d.tif</url>\n", path, id);
  fprintf(fp, "</image>\n");
  fprintf(fp, "<resolution><x>0.0375</x><y>0.0375</y><z>0.2</z></resolution>\n");
  fprintf(fp, "</data>\n");
  fprintf(fp, "</trace>\n");
  fclose(fp);
}

int main(int argc, char *argv[])
{
  static char *Spec[] = {"<input:string> -o <string> -tile_number <int> [-upsample <string>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  char filepath[500];
  int i;
  int n = Get_Int_Arg("-tile_number");
  for (i = 0; i < n; i++) {
    sprintf(filepath, "%s/%03d", Get_String_Arg("input"), i+1);
    int n = dir_fnum(filepath, "tif");
    sprintf(filepath, "%s/%03d.xml", Get_String_Arg("-o"), i+1);
    FILE *fp = GUARDED_FOPEN(filepath, "w");
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(fp, "<trace>\n");
    fprintf(fp, "<data>\n");
    fprintf(fp, "<image type=\"bundle\">\n");
    fprintf(fp, "<prefix>%s/%03d/</prefix>\n", Get_String_Arg("input"), i+1);
    fprintf(fp, "<suffix>.tif</suffix>\n");
    if (n > 100) {
      fprintf(fp, "<num_width>3</num_width>\n");
    } else {
      fprintf(fp, "<num_width>2</num_width>\n");
    }
    fprintf(fp, "<first_num>1</first_num>\n");
    fprintf(fp, "</image>\n");
    fprintf(fp, "<resolution><x>0.0375</x><y>0.0375</y><z>0.2</z></resolution>\n");
    fprintf(fp, "</data>\n");
    fprintf(fp, "</trace>\n");
    fclose(fp);
    printf("%s created\n", filepath);
  }

  if (Is_Arg_Matched("-upsample")) {
    if (fexist(Get_String_Arg("-upsample"))) {
      FILE *fp = fopen(Get_String_Arg("-upsample"), "r");
      String_Workspace *sw = New_String_Workspace();
      char *line = NULL;
      int n;
      line = Read_Line(fp, sw);
      int *array = String_To_Integer_Array(line, NULL, &n);
      int i;
      for (i = 0; i < n; i++) {
	upsample(Get_String_Arg("-o"), array[i]);
      }
      fclose(fp);
    }
  }

  return 0;
}
