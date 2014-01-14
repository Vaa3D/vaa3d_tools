/* teststring.c
 *
 * 17-Jun-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <regex.h>
#include "tz_string.h"
#include "tz_utilities.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_stack_lib.h"
#include "tz_math.h"
#include "tz_error.h"

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    if (tz_isletter('a') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('A') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('z') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('Z') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('9') == 1) {
      PRINT_EXCEPTION("Bug?", "should not be a letter.");
      return 1;
    }

    if (tz_isletter('&') == 1) {
      PRINT_EXCEPTION("Bug?", "should not be a letter.");
      return 1;
    }

    int n;
    int *array = String_To_Integer_Array("100 - - -101 frea-w2 13", NULL, &n);

    if (n != 4) {
      PRINT_EXCEPTION("Bug?", "unexpected number.");
      return 1;
    }

    if (array[0] != 100 || array[1] != -101 || array[2] != 2 || array[3] != 13) {
      PRINT_EXCEPTION("Bug?", "unexpected number.");
      return 1;
    }


    printf(":) Testing passed.\n");

    return 0;
  }

#if 0 /* test regular expression */
  regex_t preg;
  const char *pattern = ".*\\.tif";
  regcomp(&preg, pattern, REG_BASIC);
  int result = regexec(&preg, "hello.tif", 0, NULL, REG_BASIC);
  printf("%d\n", result);
  regfree(&preg);
#endif

#if 0 /* test Read_Word */
  FILE *fp = fopen("teststring.c", "r");
  char str[100];
  int n = 0;
  while ((n = Read_Word_D(fp, str, 0, tz_issemicolon)) > 0) {
    printf("%s\n", str);
  }
  fclose(fp);
#endif

#if 0
  String_Workspace *sw = New_String_Workspace();
  FILE *fp = fopen("tz_darray.c", "r");
  char *line;
  int i = 1;
  while((line = Read_Line(fp, sw)) != NULL) {
    printf("%d: %s\n", i++, line);
  };
  fclose(fp);
  
#endif

#if 0
  String_Workspace *sw = New_String_Workspace();
  FILE *fp = fopen("../data/stringtest.txt", "r");
  printf("%s\n", Read_Param(fp, "var3", sw));
  fclose(fp);
  
#endif

#if 0
  if (Is_Integer("23")) {
    printf("An integer\n");
  } else {
    printf("Not an integer\n");
  }
#endif

#if 0
  printf("%s\n", fullpath("../data/fly_neuron_n11", "mask.tif", NULL));
#endif

#if 0
  printf("%d\n", Count_Word_D(",", tz_isdlm));
  printf("%d\n", Count_Number_D("345, 23, 2324, nubmer, hello -30, 10 ,", tz_iscoma));
#endif

#if 0
  int n;
  int *array = String_To_Integer_Array(" 345,-23,-2324, nubmer, hello -30 10 ,",
				       NULL, &n);
  iarray_print2(array, n, 1);
#endif

#if 0
  int n;
  double *array = 
    String_To_Double_Array(" -345.4, -.23, --2.324, - nubmer, hello - 3.0, .10 ,", NULL, &n);
  darray_print2(array, n, 1);
#endif

#if 0 /* test strsplit */
  char str[] = {"/This/is/a/test/"};

  char *tail = strsplit(str, '/', -6);

  printf("%s\n", str);
  if (tail != NULL) {
    printf("tail: %s\n", tail);
  }
#endif

#if 0
  printf("%d\n", File_Line_Number("../data/test.xml", FALSE));
  printf("%d\n", File_Line_Number("../data/test.xml", TRUE));
#endif

#if 0
  printf("%d\n", String_Ends_With("This is a test", "test2"));
#endif

#if 0
  String_Workspace *sw = New_String_Workspace();
  FILE *fp = fopen("../data/score.txt", "r");
  char *line;
  int i = 0;
  double score_array[300];
  while((line = Read_Line(fp, sw)) != NULL) {
    //printf("%d: %s\n", i++, line);
    int n;
    String_To_Double_Array(line, score_array + i, &n);
    i++;
  }
  
  fclose(fp);

  //int neuron_id[] = {209,285743,211940,189938,181639,196,285714,215,446263,29565,194027,24070,170689,5809,106054,1172,1513,277709,386464,280303,2341,278848,545716,3453,210};

  fp = fopen("../data/test.csv", "w");

  int j;
  int index = 0;

  double score_matrix[25][25];

  double max_score = 0.0;
  double min_score = 10000.0;

  double row_max[25];

  for (i = 0; i < 25; i++) {
    for (j = 0; j < 25; j++) {
      if (i < j) {
        score_matrix[i][j] = score_array[index++];
        score_matrix[j][i] = score_matrix[i][j];
        if (max_score < score_matrix[i][j]) {
          max_score = score_matrix[i][j];
        }

        if (min_score > score_matrix[i][j]) {
          min_score = score_matrix[i][j];
        }
      }
      if (i == j) {
        score_matrix[i][i] = 0.0;
      }
    }
  }


  for (i = 0; i < 25; i++) {
    row_max[i] = 0.0;
    for (j = 0; j < 25; j++) {
      if (row_max[i] < score_matrix[i][j]) {
        row_max[i] = score_matrix[i][j];
      }
    }
  }

  for (i = 0; i < 25; i++) {
    for (j = 0; j < 25; j++) {
      if (i < j) {
        score_matrix[i][j] /= row_max[i];
      } else{
        score_matrix[i][j] = score_matrix[j][i];
      }
    }
  }

  Stack *stack = Make_Stack(GREY, 25, 25, 1);

  int offset = 0;
  for (i = 0; i < 25; i++) {
    for (j = 0; j < 25; j++) {
      fprintf(fp, "%g, ", score_matrix[i][j]);
      if (i == j) {
        stack->array[offset++] = 255;
      } else {
        /*
        int value = iround(255.0 / (max_score - min_score) * 
            (score_matrix[i][j] - min_score));
            */
        int value = iround(score_matrix[i][j] * 255.0);

        if (value > 255) {
          value = 255;
        }
        stack->array[offset++] = value;
      }
    }
    fprintf(fp, "\n");
  }

  Write_Stack("../data/test.tif", stack);

  fclose(fp);
#endif

#if 1
  printf("double number: %d\n", count_double(".9 3.4 e +3.3e-3 4"));
  printf("double number: %d\n", count_double(".9-.9.9.9.9"));

  char *str = ".9 3.4 e +3.3e-3 4";
  //char *str = ".9-.9.9.9.9E-41.23.4e0r2.1";
  int n;
  double *array = 
    String_To_Double_Array(str, NULL, &n);
    //String_To_Double_Array(" -345.4e-.23, --2.324e-05, - nubmer, hello - 3.0, .10 ,", NULL, &n);
  darray_printf(array, n, "%g");

#endif

  
  return 1;
}
