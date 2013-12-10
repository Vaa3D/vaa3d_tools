/* testutilities.c
 *
 * 24-May-2008 Initial write: Ting Zhao
 */

#include <stdlib.h>
#include <string.h>
#include <utilities.h>
#include <math.h>
#include "tz_utilities.h"
#include "tz_error.h"
#include "tz_file_list.h"
#include "tz_image_lib_defs.h"
#include "tz_stack_utils.h"

/*
#include "tz_matlabio.h"
#include "tz_rastergeom.h"
#include <gsl/gsl_math.h>
*/
int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};
 
  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    /* example test */
    
    /* extract file name without extension from a path */
    const char *path = "../data/test.tif";
    char *name = fname(path, NULL);
    if (strcmp(name, "test") != 0) {
      PRINT_EXCEPTION("Bug?", "Unexpected file name.");
      return 1;
    }

    /* The user is responsible for freeing the returned value */
    free(name);

    /* The routine returns NULL when no name can be extracted */
    path = ".";
    if (fname(path, NULL) != NULL) {
      PRINT_EXCEPTION("Bug?", "Unexpected file name.");
      return 1;
    }

    path = "../data/";
    if (fname(path, NULL) != NULL) {
      PRINT_EXCEPTION("Bug?", "Unexpected file name.");
      return 1;
    }

    /* Different parts can be put together to form a path */
    char new_path[500];
    fullpath_e("../data/", "test", "tif", new_path);
    if (strcmp(new_path, "../data/test.tif") != 0) {
      printf("%s\n", new_path);
      PRINT_EXCEPTION("Bug?", "Unexpected file path.");
      return 1;      
    }
    
    fullpath_e("", "test", "tif", new_path);
    if (strcmp(new_path, "test.tif") != 0) {
      printf("%s\n", new_path);
      PRINT_EXCEPTION("Bug?", "Unexpected file path.");
      return 1;      
    }

    { /* unit test */
      char buffer[500];

      if (fname(NULL, buffer) != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (fname("\0", buffer) != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (fname(".", buffer) != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }
      
      if (strcmp(fname("t", buffer), "t") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (fname("/", buffer) != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }
      
      if (fname("test/", buffer) != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (strcmp(fname("test", buffer), "test") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (strcmp(fname("./test", buffer), "test") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (strcmp(fname("/test", buffer), "test") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (strcmp(fname("/.test", buffer), ".test") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (strcmp(fname("t.est", buffer), "t") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (strcmp(fname("/t.est", buffer), "t") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (fname("/.", buffer) != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }

      if (fname("data/.", buffer) != NULL) {
	PRINT_EXCEPTION("Bug?", "Unexpected file name.");
	return 1;
      }
      
      if (strcmp(fullpath_e("dir", "file", "ext", buffer), 
		 "dir/file.ext") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected path name.");
	return 1;	
      }

      if (strcmp(fullpath_e("dir/", "file", "ext", buffer), 
		 "dir/file.ext") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected path name.");
	return 1;	
      }

      if (strcmp(fullpath_e("dir", "file", NULL, buffer), 
		 "dir/file") != 0) {
	PRINT_EXCEPTION("Bug?", "Unexpected path name.");
	return 1;	
      }

      if (strcmp(fullpath_e("dir", "file", ".", buffer), 
		 "dir/file.") != 0) {
	printf("%s\n", buffer);
	PRINT_EXCEPTION("Bug?", "Unexpected path name.");
	return 1;	
      }

      if (strcmp(fullpath_e("dir", "file", ".ext", buffer), 
		 "dir/file.ext") != 0) {
	printf("%s\n", buffer);
	PRINT_EXCEPTION("Bug?", "Unexpected path name.");
	return 1;	
      }

    }

    printf(":) Testing passed.\n");
    return 0;
  }

#if 0
  FILE *fp = fopen("../data/test.fbd", "r");

  size_t len;
  char *line;
  while (!feof(fp)) {
    if ((line = fgetln(fp, &len)) != NULL) {
      printf("read %lu: ", len);
      line[len-1] = '\0';
      printf("%s\n", line);
    }
  }

  fclose(fp);
#endif

#if 0
  char path[100];
  Find_Matlab_Binpath(path);
  printf("%s : %lu\n", path, strlen(path));
#endif

#if 0
  int a = 10000;
  int b = 11;
  XOR_SWAP(a, b);

  printf("%d %d\n", a, b);
#endif

#if 0
  printf("%d\n", 2 << 2);
  printf("%d\n", Compare_Float(1.01, 1.002, 0.0005));
  printf("%d\n", gsl_fcmp(1.01, 1.002, 0.0005));
#endif

#if 0
  char *s1 = "hello";
  printf("%p\n", s1);
  s1[0] = 't';
  printf("%s\n", s1);
#endif

#if 0
  double x = -100.4350;
  printf("%g\n", Cube_Root(x) * Cube_Root(x) * Cube_Root(x));
#endif

#if 0
  printf("%d\n", Raster_Line_Map(852, 451, 296));
  return 1;

  int m = 33;
  int n = 10;
  int i;
  for (i = 0; i < m; i++) {
    printf("%d ", Raster_Line_Map(m, n, i));
  }
  printf("\n");
#endif

#if 0
  int w1 = 302;
  int h1 = 586;
  int nw2;
  int nh2;
  Raster_Ratio_Scale(w1, h1, 425, 236, &nw2, &nh2);
  printf("%d x %d\n", nw2, nh2);
  printf("%g => %g\n", (double) w1 / h1, (double) nw2 / nh2);
#endif

#if 0
  //printf("%u\n", Hexstr_To_Uint("BA9B"));
  char str[9];
  printf("%s\n", Uint_To_Hexstr(262, str));
#endif

#if 0
  FILE *fp = fopen("/Users/zhaot/Work/neurolabi/data/movie/a0.BMP", "r");
  Fprint_File_Binary(fp, 20, stdout);
  fclose(fp);
#endif

#if 0
  printf("%d\n", Round_Div(110, -19));
#endif

#if 0
  printf("%d\n", Raster_Linear_Map(31, 10, 100, 0, 231));
#endif

#if 0
  printf("%d\n", Raster_Point_Zoom_Offset(0, 2, 100, 512, 500, 1));
#endif

#if 0
  fcopy("../data/test.tif", "../data/test2.tif");
#endif

#if 0
  printf("%d bytes\n", fsize("../data/test.tif"));
#endif

#if 0
  printf("%g %g %g\n", Infinity, -Infinity, NaN);
#endif

#if 0
  size_t x = 1000;
  printf("%zd\n", (size_t) 1000 * 1000 * 1000 * 4);
#endif

#if 0
  size_t index = Stack_Util_Offset(1000, 1000, 1000, 2000, 2000, 2000);
  printf("%zd\n", index);
  int x, y, z;
  Stack_Util_Coord(index, 2000, 2000, &x, &y, &z);
  printf("%d %d %d\n", x, y, z);
#endif

#if 0
  printf("%s\n", fname("../data/test.dot", NULL));
  printf("%s\n", dname("../data/test.dot", NULL));
  printf("%s\n", dname("/test.dot", NULL));
  printf("%s\n", dname("test.dot", NULL));
#endif

#if 0
  char *line = malloc(2048 * 2048 * 4);
  char *data = malloc(1024 * 1024 * 4);
  int i;
  int j;
  tic();
  for (i = 0; i < 2048; i++) {
    for (j = 0; j < 2048; j++) {
      if (*line != *data++) {
	*line++ = *data;
	*line++ = *data;
	*line = *data;
	line += 2;
      }
      /*
      memset(line, *data++, 3);
      line += 4;
      */
    }
  }
  printf("%lld\n", toc());
#endif

#if 0
  char buf[11];
  Memset_Pattern4( buf, "1234", 10 );
  buf[10] = '\0';
  printf("%s\n", buf);

  Memset_Pattern4( buf, "4321", 8 );
  buf[8] = '\0';
  printf("%s\n", buf);

  Memset_Pattern4( buf, "4321", 2);
  buf[2] = '\0';
  printf("%s\n", buf);
#endif

#if 0
  char *block[1000000];
  int i;

  tic();
  for (i = 0; i < 1000000; i++) {
    block[i] = (char*) malloc(i % 991 + 10);
  }
  printf("%lld\n", toc());

  int index[1000000];
  int j;
  int offset = 0;
  for (i = 0; i < 100; i++) {
    for (j = 0; j < 10000; j++) {
      index[offset++] = j * 100 + i;
    }
  }

  tic();
  for (i = 999999; i >= 0; i--) {
    free(block[index[i]]);
  }
  printf("%lld\n", toc());
#endif

#if 0
  int (*test)[10];
  int test2[10];
  test = &test2;
  printf("%p\n", test);
  printf("%p\n", *test);
  printf("%p\n", test2);
#endif

#if 0
  int a[100];
  int i = 0;
  i[a] = 0;
  printf("%d\n", i[a]);
#endif

#if 0
  free(NULL);
#endif

#if 0
  int i;
  for (i = 0; i < 10; i++) {
    int n = 0;
    n++;
    printf("%d\n", n);
  }
#endif

#if 0
  printf("%g\n", round(0.5));
  printf("%g\n", floor(0.5));
#endif

#if 0
  if (fcmp("../data/test2.tb", "../data/test3.tb")) {
    printf("The files are different.\n");
  } else {
    printf("The files are the same.\n");
  }
#endif

#if 0
  char str[65];
  printf("%s\n", double_binstr(0.0, str));
#endif

#if 0
  printf("%d\n", MEDIAN3(0, 1, 2));
  printf("%d\n", MEDIAN3(0, 2, 1));
  printf("%d\n", MEDIAN3(1, 0, 2));
  printf("%d\n", MEDIAN3(2, 1, 0));
  printf("%d\n", MEDIAN3(1, 2, 0));
  printf("%d\n", MEDIAN3(2, 0, 1));
  printf("%d\n", MEDIAN3(0, 1, 1));
  printf("%d\n", MEDIAN3(1, 0, 1));
  printf("%d\n", MEDIAN3(1, 1, 0));
  printf("%d\n", MEDIAN3(2, 1, 1));
  printf("%d\n", MEDIAN3(1, 2, 1));
  printf("%d\n", MEDIAN3(1, 1, 2));
#endif

#if 0
  unsigned char ch1 = 255;
  char ch = ch1;
  printf("%x\n", ch);

  ch1 = -1;
  printf("%x\n", ch1);
#endif

#if 0
  //size_t fs = fsize("../data/spikes/pm002.nev");
  FILE *fp = fopen("../data/spikes/pm002.nev", "r");
  /*
  fseek(fp, fs - 104*5, SEEK_SET);
  uint32_t tmp1;
  uint16_t tmp2;
  uint8_t tmp3;
  fread(&tmp1, 4, 1, fp);
  fread(&tmp2, 2, 1, fp);
  fread(&tmp3, 1, 1, fp);
  printf("%u, %u, %u\n", tmp1, tmp2, tmp3);
  */

  uint32_t head_size;
  fseek(fp, 12, SEEK_SET);
  fread(&head_size, 4, 1, fp);
  fseek(fp, head_size, SEEK_SET);
  int packetcnt = 4325366;
  char *buffer = (char*) malloc(packetcnt * 104);
  fread(buffer, 1, packetcnt * 104, fp);
  int offset = 0;
  uint32_t tmp1;
  uint16_t tmp2;
  uint8_t tmp3;
  
  int i;
  for (i = 0; i < packetcnt; i++) {
    tmp1 = *((uint32_t*) (buffer + offset));
    offset += 4;
    tmp2 = *((uint16_t*) (buffer + offset));
    offset += 2;
    tmp3 = *((uint8_t*) (buffer + offset));
    if (i > 4325360) {
      printf("%u %u %u\n", tmp1, tmp2, tmp3);
    }
    offset += 1;
    offset += 97;
  }

  fclose(fp);
#endif

#if 0
  File_List *list = New_File_List();

  //File_List_Load_Dir("/home/zhaot/Work/neutube/neurolabi/data/test", "tif", list);
  File_List_Load_Dir("../data", "tif", list);

  File_List_Sort_By_Number(list);

  Print_File_List(list);
#endif

#if 0
  int start, end;
  dir_fnum_pair("../data/ting_example_stack/superpixel_maps", ".*\\.png", 
      &start, &end);
  printf("%d, %d\n", start, end);
#endif

#if 0
  color_t color;
  uint8_t overflow = Value_To_Color(0, color);

  printf("%u %u %u %u\n", overflow, color[2], color[1], color[0]);
#endif

#if 0
  printf("%*sD\n", 3, "t");
#endif

#if 1
#endif


  return 0;
}
