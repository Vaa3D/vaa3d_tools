#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <utilities.h>
#include <sys/stat.h>
#if defined(_WIN32) || defined(_WIN64)
  #define PCRE_STATIC
  #include <pcreposix.h>
#else
  #include <regex.h>
#endif
#include <dirent.h>
#include <errno.h>
#include "tz_string.h"
#include "tz_mc_stack.h"
#include "tz_image_io.h"

char** get_image_paths(const char *dir_name, const char *pattern, int *n)
{
  *n = dir_fnum_p(dir_name, pattern);
  if (*n == 0) {
    return NULL;
  }

  char** filepaths = (char**) malloc(sizeof(char*) * (*n));
  int i;
  for (i = 0; i < *n; ++i) {
    filepaths[i] = (char*) Guarded_Malloc(sizeof(char) * 500, "filepath");
  }

  DIR *dir = opendir(dir_name);
  struct dirent *ent = readdir(dir);
  int index = 0;

  /* allocate a string for storing full path. 100 is just an initial guess */
  char *prefix = (char *) Guarded_Malloc(strlen(dir_name) + 100,
      "Dir_Neurochain");
  strcpy(prefix, dir_name);
  if (prefix[strlen(prefix)-1]!='/') {
    strcat(prefix, "/");
  }
  printf("%s\n", prefix);

  regex_t preg;
  regcomp(&preg, pattern, REG_BASIC);

  while (ent != NULL) {
    if (regexec(&preg, ent->d_name, 0, NULL, REG_BASIC) ==0) {
      if (strncasecmp(ent->d_name, "tileselection", 13) != 0) {
        strcpy(filepaths[index], prefix);
        strcat(filepaths[index], ent->d_name);
        printf("%s\n", filepaths[index]);
        index++;
      } else {
        (*n)--;
      }
    }
    ent = readdir(dir);
  }
  free(prefix);
  closedir(dir);

  regfree(&preg);

  return filepaths;
}

int compare_string_by_num(const void *a, const void *b)
{
  if (strlen(*(char**)a) > strlen(*(char**)b)) {
    return 1;
  } else if (strlen(*(char**)a) < strlen(*(char**)b)) {
    return -1;
  } else {
    char *acopy = *(char**)a;
    char *bcopy = *(char**)b;
    while(*acopy == *bcopy) {
      acopy++;
      bcopy++;
    }
    int a1 = atoi(acopy);
    int b1 = atoi(bcopy);
    if (a1 < b1) {
      return -1;
    } else if (a1 > b1) {
      return 1;
    } else {
      return 0;
    }
  }
}

/* same as fopen, but create folder when needed */
FILE *open_file(const char *path, const char *mode)
{
  FILE *fp = fopen(path, mode);
  if (fp == NULL) {
    int errsv = errno;
    /*perror("fopen");*/
    if (errsv == ENOENT) {
      /*printf("The directory does not exist. Creating new directory ...\n");*/
      int pos;
      char *pathcopy = malloc(strlen(path)+1);
      strcpy(pathcopy, path);
      for (pos=1; pos<strlen(pathcopy)-1; pos++) {
        if (pathcopy[pos]=='/') {
          pathcopy[pos] = '\0';
          struct stat sb;
          int e = stat(pathcopy, &sb);
          if (e != 0 && errno == ENOENT) {
            printf("Creating directory %s\n", pathcopy);
      #if defined(_WIN64) || defined(_WIN32)
      e = mkdir(pathcopy);
      #else
            e = mkdir(pathcopy, S_IRWXU);
      #endif
            if (e != 0) {
              perror("mkdir error");
              exit(EXIT_FAILURE);
            }
          }
          pathcopy[pos] = '/';
        }
      }
      free(pathcopy);
      /*try again*/
      fp = fopen(path, mode);
      if (fp == NULL) {
        perror("second try fopen");
        exit(EXIT_FAILURE);
      }
      return fp;
    }

    exit(EXIT_FAILURE);
  }
  return fp;
}

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-outfolder <string>] [-redgreenimagefolder <string>]",
    "[-blueimagefolder <string>]", NULL};
  Process_Arguments(argc, argv, Spec, 1);

  int nredgreenimage;
  int nblueimage;
  char** redgreenfilepath = get_image_paths(Get_String_Arg("-redgreenimagefolder"), ".*\\.lsm", &nredgreenimage);
  char** bluefilepath = get_image_paths(Get_String_Arg("-blueimagefolder"), ".*\\.lsm", &nblueimage);


  if (redgreenfilepath == NULL || bluefilepath == NULL) {
    printf("no image!\n");
    return 1;
  } else if (nredgreenimage != nblueimage) {
    printf("image number don't match, abort! red: %d, blue: %d\n", nredgreenimage, nblueimage);
    return 1;
  } else {
    printf("%d images\n", nblueimage);
  }

  qsort(redgreenfilepath, nblueimage, sizeof(char*), compare_string_by_num);
  qsort(bluefilepath, nblueimage, sizeof(char*), compare_string_by_num);

  char prefix[500];
  char outfilepath[500];
  char filenamenoext[500];
  int k;
  for (k = 0; k < nblueimage; ++k) {
    strcpy(prefix, Get_String_Arg("-outfolder"));
    strcpy(filenamenoext, redgreenfilepath[k]);
    char *end = strrchr(filenamenoext, '.');
    if (end != NULL) {
      *end = '\0';
    }
    else {
      printf("something stange happens..\n");
    }
    char *start = strrchr(filenamenoext, '/');
    if (start != NULL) {
      start++;
    }
    else {
      start = filenamenoext;
    }
    if (prefix[strlen(prefix)-1]=='/') {
      sprintf(outfilepath, "%s%s.tif", prefix, start);
    } else {
      sprintf(outfilepath, "%s/%s.tif", prefix, start);
    }

    FILE* fp = open_file(outfilepath, "wb");
    fclose(fp);

    Mc_Stack* mc_stack1 = Read_Mc_Stack(redgreenfilepath[k], -1);
    Mc_Stack* mc_stack2 = Read_Mc_Stack(bluefilepath[k], -1);

    Mc_Stack* mc_stack = Combine_Mc_Stack(mc_stack1, mc_stack2);
    Write_Mc_Stack(outfilepath, mc_stack, NULL);
    printf("Write %s done!\n", outfilepath);
    Kill_Mc_Stack(mc_stack1);
    Kill_Mc_Stack(mc_stack2);
    Kill_Mc_Stack(mc_stack);
  }

  free(redgreenfilepath);
  free(bluefilepath);

  return 0;
}

