/*
 * @author Ting Zhao
 * @date 05-Aug-2007
 */
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef _MSC_VER
#include <dirent.h>
#else
#include "tz_dirent.h"
#endif
#if defined(_WIN32) || defined(_WIN64)
  #define PCRE_STATIC
  #include <pcreposix.h>
#else
  #include <regex.h>
#endif
#include <errno.h>
#include "tz_utilities.h"
#include "tz_string.h"
#ifdef HAVE_SYS_STAT_H
  #include <sys/stat.h>
#endif
#include "tz_error.h"


static struct timeval start_time,end_time;

void tic()
{
#ifdef HAVE_SYS_TIME_H
  gettimeofday(&start_time,NULL);
#endif
}

tz_int64 toc()
{
#ifdef HAVE_SYS_TIME_H
  gettimeofday(&end_time,NULL);
  return (end_time.tv_sec-start_time.tv_sec) * 1000 +
    (end_time.tv_usec-start_time.tv_usec)/1000;
#else
  return 0;
#endif
}

void ptoc()
{
  printf("Time elapsed: %llu ms\n", (long long unsigned) toc());
}

void pmtoc(const char *msg)
{
  if (msg == NULL) {
    ptoc();
  } else {
    printf("%llu : %s\n", (long long unsigned) toc(), msg);
  }
}

int fexist(const char* filename)
{
  if (filename == NULL) {
    return 0;
  }

  FILE* fp = fopen(filename,"r");

  if(fp==NULL)
    return 0;
  else {
    fclose(fp);
    return 1;
  }
}

int dexist(const char *path)
{
#ifdef HAVE_SYS_STAT_H
  if (path == NULL) {
    return 0;
  }

  struct stat st;
  if ((stat(path, &st) == 0) && (S_ISDIR(st.st_mode))) {
    return 1;
  }
#else
  TZ_ERROR(ERROR_NA_FUNC);
#endif

  return 0;
}
int fcopy(const char *source, const char *target)
{
  if (source == NULL || target == NULL) {
    return -2;
  }

  FILE *fp = fopen(source, "rb");
  if (fp == NULL) {
    return -1;
  }

  FILE *fp2 = fopen(target, "wb");
  if (fp2 == NULL) {
    fclose(fp);
    return -1;
  }

  while (!feof(fp)) {
    char ch = fgetc(fp);
    if (!feof(fp)) {
      fputc(ch, fp2);
    }
  }

  fclose(fp2);
  
  return 0;
}

const char *fextn(const char* filename)
{
  if (filename == NULL) {
    return NULL;
  }

  int len = strlen(filename);
  int i;
  for (i = len - 1; i >=0; i--) {
    if (filename[i] == '.') {
      return filename + i + 1;
    }
  }

  return NULL;
}

char* fname(const char *path, char *name)
{
  if (path == NULL) {
    return NULL;
  }

  int len = strlen(path);

  if (len == 0) {
    return NULL;
  }

  if (len == 1) {
    if (path[0] == '.') {
      return NULL;
    }
  }

  char pathsep = '/';
  if (path[len-1] == pathsep || path[len-1] == '\\') {
    return NULL;
  }

  int start = -1;
  int end = -1;
  int dot_pos = len;
  int pathsep_pos = -1;
  int i;
  for (i = len - 1; i >=0; i--) {
    if (dot_pos == len) {
      if (path[i] == '.') {
	dot_pos = i;
      }
    }
    if (pathsep_pos < 0) {
      if (path[i] == pathsep || path[i] == '\\') {
	pathsep_pos = i;
      }
    }
  }

  if (pathsep_pos >= 0) {
    if (dot_pos == pathsep_pos + 1)  { 
      if (dot_pos == len - 1) { /* dot only */
	return NULL;
      }
      dot_pos = len;
    }    
  }

  if (dot_pos < pathsep_pos) {
    dot_pos = len;
  }

  start = pathsep_pos + 1;
  end = dot_pos - 1;

  int new_len = end - start + 1;

  if (name == NULL) {
    name = (char*) malloc(sizeof(char) * (new_len + 1));
  }

  memcpy(name, path + start, new_len); 
  name[new_len] = '\0';

  return name;
}

char* dname(const char *path, char *name)
{
  if (path == NULL) {
    return NULL;
  }

  int len = strlen(path);

  if (len == 0) {
    return NULL;
  }

  if (len == 1) {
    if (path[0] == '.') {
      return NULL;
    }
  }

  char pathsep = '/';
  if (path[len-1] == pathsep || path[len-1] == '\\') {
    return NULL;
  }

  int start = -1;
  //int end = -1;
  int dot_pos = len;
  int pathsep_pos = -1;
  int i;
  for (i = len - 1; i >=0; i--) {
    if (dot_pos == len) {
      if (path[i] == '.') {
	dot_pos = i;
      }
    }
    if (pathsep_pos < 0) {
      if (path[i] == pathsep || path[i] == '\\') {
	pathsep_pos = i;
      }
    }
  }

  if (pathsep_pos >= 0) {
    if (dot_pos == pathsep_pos + 1)  { 
      if (dot_pos == len - 1) { /* dot only */
	return NULL;
      }
      dot_pos = len;
    }    
  }

  if (dot_pos < pathsep_pos) {
    dot_pos = len;
  }

  start = pathsep_pos + 1;
  //end = dot_pos - 1;

  int new_len = start;

  if (name == NULL) {
    name = (char*) malloc(sizeof(char) * (new_len + 1));
  }

  if (new_len > 0) {
    memcpy(name, path, new_len); 
  }

  name[new_len] = '\0';

  return name;
}

int fhasext(const char* filename, const char *ext)
{
  if (filename == NULL) {
    return 0;
  }

  const char *sext = fextn(filename);
  if ((sext == NULL) && (ext == NULL)) {
    return 1;
  } else if ((sext != NULL) && (ext != NULL)) {
    if (strcmp(sext, ext) == 0) {
      return 1;
    }
  }

  return 0;
}

int dir_fnum(const char *dir_name, const char *ext)
{
  if (dir_name == NULL) {
    return 0;
  }

  int n = 0;
  DIR *dir = opendir(dir_name);
  if (dir == NULL) {
    perror(strerror(errno));
    return 0;
  }

  struct dirent *ent = readdir(dir);
  while (ent != NULL) {
    const char *file_ext = fextn(ent->d_name);
    if (file_ext != NULL) {
      if (strcmp(file_ext, ext) ==0) {
	n++;
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  return n;
}

int dir_fnum_p(const char *dir_name, const char *pattern)
{
  if (dir_name == NULL) {
    return 0;
  }

  int n = 0;
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);

  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      n++;
    }
    ent = readdir(dir);
  }
  closedir(dir);

  regfree(&preg);

  return n;
}

int dir_fnum_s(const char *dir_name, const char *pattern)
{
  if (dir_name == NULL) {
    return 0;
  }

  int n = 0;
  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);

  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int id = String_Last_Integer(ent->d_name); 
      if (n < id) {
	n = id;
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  regfree(&preg);

  return n;
}

int dir_fnum_pair(const char *dir_name, const char *pattern, int *start,
    int *end)
{
  if (dir_name == NULL) {
    return 0;
  }

  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);

  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  *start = -1;
  *end = -1;

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      int id = String_Last_Integer(ent->d_name); 
      if (*start < 0) {
        *start = id;
        *end = id;
      } else {
        if (*start > id) {
          *start = id;
        }
        if (*end < id) {
          *end = id;
        }
      }
    }
    ent = readdir(dir);
  }
  closedir(dir);

  regfree(&preg);

  return ((*start) >= 0);
}

char* fullpath(const char *path1, const char *path2, char *path)
{
  return fullpath_e(path1, path2, NULL, path);
}

char* fullpath_e(const char *path1, const char *path2, const char *ext,
		 char *path)
{
  if (path == NULL) {
    path = (char*) malloc(sizeof(char) * (strlen(path1) + strlen(path2) + 2));
  }

  int len = strlen(path1);
  if (len > 0) {
    strcpy(path, path1);
    if (path1[len - 1] != '/') {
      path[len] = '/';
      path[len + 1] = '\0';
    }
    strcat(path, path2);
  } else {
    strcpy(path, path2);
  }

  if (ext != NULL) {
    if (ext[0] == '.') {
      ext++;
    }
    len = strlen(path);
    path[len] = '.';
    path[len+1] = '\0';
    strcat(path, ext);
  }

  return path;
}

void safe_free(void **ptr, void (*free_fun) (void *)) 
{
  if (free_fun == NULL) {
    free(*ptr);
  } else {
    free_fun(*ptr);
  }

  *ptr = NULL;
}


char* double_binstr(double x, char str[])
{
  union {
    long long c;
    double d;
  } tmp_x;

  tmp_x.d = x;

  long long y = 1;
  int nbit = sizeof(double) * 8;
  int i;
  for (i = nbit - 1; i >= 0; i--) {
    if ((y & tmp_x.c) == 0) {
      str[i] = '0';
    } else {
      str[i] = '1';
    }
    y <<= 1;
  }
  str[nbit] = '\0';

  return str;
}

int tz_isnan(double x)
{
  return isnan(x);
}

int tz_isinf(double x)
{
  return (x == Infinity) || (x == -Infinity);
}

#ifndef HAVE_FGETLN
char *fgetln(FILE *stream, size_t *len)
{
  #if defined FGETLN_MAX_LENGTH
  #  undef FGETLN_MAX_LENGTH
  #endif

  #define FGETLN_MAX_LENGTH 1000

  static char line[FGETLN_MAX_LENGTH];

  if (fgets(line, FGETLN_MAX_LENGTH, stream) == NULL) {
    return NULL;
  }

  *len = strlen(line);

  return line;
}
#endif

double Value_Trend(double *scores, int n)
{
  int i;
  double sxi = 0.0;
  double sx2 = 0.0;
  double sx = 0.0;
  for (i = 0; i < n; i++) {
    sx += scores[i];
    sxi += scores[i] * (double) i;
    sx2 += scores[i] * scores[i];
  }

  return (sxi - sx * (n-1) / 2.0) / 
    (sqrt((sx2 * n - sx * sx) * (n - 1) * (n + 1) / 12.0));
}

int Show_Version(int argc, char *argv[], const char *ver)
{
  int show_ver = 0;
  if (argc == 2) {
    if (strcmp(argv[1], "-ver") == 0) {
      show_ver = 1;
    }
  }

  if (show_ver == 1) {
    printf("%s Version %s\n", argv[0], ver);
#if ARCH_BIT == 64
    printf("64 bit\n");
#endif
    printf("Author: Ting Zhao\n");
    printf("Janelia Farm Research Campus\n"); 
  }

  return show_ver;
}

void Print_Arguments(int argc, char *argv[])
{
  int i;
  for (i = 0; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
}

static inline uint8_t hexchar2num(char c)
{
  if ((c >= '0') && (c <= '9')) {
    return c - '0';
  } else if ((c >= 'a') && (c <= 'f')){
    return c - 'a' + 10;
  } else if ((c >= 'A') && (c <= 'F')){
    return c - 'A' + 10;
  } else {
    return 255;
  }
}

uint32_t Hexstr_To_Uint(char *str)
{
  int len = strlen(str);

  TZ_ASSERT((len > 0) && (len % 2 == 0), "invalid string");

  uint32_t num;
  uint8_t *mem = (uint8_t*) &num;
  
  len /= 2;
  int i;
  for (i = 0; i < len; i++) {
    mem[i] = (hexchar2num(str[i*2]) << 4) + hexchar2num(str[i*2+1]);
  }

  for (i = len; i < 4; i++) {
    mem[i] = 0;
  }

  return num;
}

static inline char num2hexchar(uint8_t num)
{
  if (num < 10) {
    return num + '0';
  } else if (num < 16) {
    return num - 10 + 'A';
  } else {
    return 'n';
  }
}

char* Uint_To_Hexstr(uint32_t num, char *str)
{
  uint8_t *mem = (uint8_t*) &num;
  int i;
  for (i = 0; i < 4; i++) {
    uint8_t b = mem[i];
    str[3*i] = num2hexchar((b & 0xF0)>>4);
    str[3*i+1] = num2hexchar(b & 0x0F);
    str[3*i+2] = ' ';
  }
  str[11] = '\0';

  return str;
}

inline static char* uint8_to_hexstr(uint8_t value, char *str)
{
  str[0] = num2hexchar((value & 0xF0)>>4);
  str[1] = num2hexchar(value & 0x0F);

  return str;
}

void Fprint_File_Binary(FILE *fp, size_t n, FILE *out)
{
  if (fp == NULL) {
    return;
  }

  uint8_t value;
  char str[3];					
  str[2] = '\0';
  size_t i;
  for (i = 0; i < n; i++) {
    fread(&value, 1, 1, fp);
    if (!feof(fp)) {
      fprintf(out, "%s ", uint8_to_hexstr(value, str));
    } else {
      fprintf(out, "##");
      break;
    }
  }
  fprintf(out, "\n");
}

size_t fsize(const char *filename)
{
#ifdef HAVE_SYS_STAT_H
  struct stat buf;
  stat(filename, &buf);
  return buf.st_size;
#else
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    return -1;
  }

  fseek(fp, 0L, SEEK_END);
  size_t n = ftell(fp);
  fclose(fp);
  
  return n;
#endif
}

int fcmp(const char *filepath1, const char *filepath2)
{
  if (strcmp(filepath1, filepath2) == 0) {
    return 0;
  }

  FILE *fp1 = GUARDED_FOPEN(filepath1, "rb");
  FILE *fp2 = GUARDED_FOPEN(filepath2, "rb");

  int diff = 0;

  size_t n1, n2;
  char ch1, ch2;

  while (!feof(fp1) && !feof(fp2)) {
    ch1 = '\0';
    ch2 = '\0';
    n1 = fread(&ch1, 1, 1, fp1);
    n2 = fread(&ch2, 1, 1, fp2);
    if ((n1 == 1) && (n2 == 1)) {
      if (ch1 != ch2) {
        diff = 1;
        break;
      }
    } else {
      if (feof(fp1) != feof(fp2)) {
        diff = 2;
      }
      break;
    }
  }

  fclose(fp1);
  fclose(fp2);

  return diff;
}

void *Guarded_Calloc(size_t count, size_t size, const char *routine)
{ void *p;

  p = calloc(count, size);
  if (p == NULL)
    { fprintf(stderr,"\nError in %s:\n",routine);
      fprintf(stderr,"   Out of memory\n");
      exit (1);
    }
  return (p);
}

/* n = 2^k + m - 1
 * k: starts from 1
 * m: starts from 0
 * n: starts from 1
 */
void pow2decomp(int n, int *k, int *m)
{
  *k = 0;
  int pn = 2;
  n += 1;
  while (pn <= n) {
    (*k)++;
    pn *= 2;
  }

  pn /= 2;

  *m = n - pn;
}


int pow2sum(int k, int m)
{
  int result = 1;
  int i;
  for (i = 0; i < k; i++) {
    result *= 2;
  }

  return result - 1 + m;
}

void Print_Argument_Spec(const char* const spec[])
{
  while (*spec) {
    printf("%s\n", *(spec++));
  }
}

void Memset_Pattern4(void *b, const void *pattern4, size_t len)
{
  size_t count = len / 4;
  size_t i;
  char *cb = (char*) b;
  for (i = 0; i < count; i++) {
    memcpy(cb, pattern4, 4);
    cb += 4;
  }

  count = len % 4;
  if (count > 0) {
    memcpy(cb, pattern4, count);
  }
}

void rmpath(const char *path)
{
  char cmd[500];
  sprintf(cmd, "rm -rf %s", path);
  system(cmd);
}


BOOL eqstr(const char *str1, const char *str2)
{
  if ((str1 == NULL) && (str2 == NULL)) {
    return TRUE;
  } else if ((str1 == NULL) && (str2 == NULL)) {
    return FALSE;
  } else {
    if (strcmp(str1, str2) == 0) {
      return TRUE;
    }
  }
   
  return FALSE;
}

BOOL Is_Valid_Array_Index(size_t index)
{
  return (index != INVALID_ARRAY_INDEX);
}
