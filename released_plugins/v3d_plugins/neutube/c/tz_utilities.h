/**@file tz_utilities.h
 * @brief general utilities
 * @author Ting Zhao
 * @date 05-Aug-2007
 */

#ifndef _TZ_UTILITIES_H_
#define _TZ_UTILITIES_H_

#include <stdio.h>
#include "tz_cdefs.h"
#include "tz_stdint.h"

__BEGIN_DECLS

#include <utilities.h>

typedef uint8_t tz_uint8;
typedef int8_t tz_int8;
typedef uint16_t tz_uint16;
typedef int16_t tz_int16;
typedef uint32_t tz_uint32;
typedef int32_t tz_int32;
typedef uint64_t tz_uint64;
typedef int64_t tz_int64;
typedef float tz_float32;
typedef double tz_float64;

#define BYTE_ARRAY(array) ((byte_t*)(array))

#define DATA_TYPE_NUMBER 12

typedef enum {FLOAT64_TD=0,FLOAT32_TD,INT64_TD,UINT64_TD,INT32_TD,
	      UINT32_TD,INT16_TD,UINT16_TD,INT8_TD,UINT8_TD,
	      DOUBLE_TD=0,FLOAT_TD,LLONG_TD,ULLONG_TD,LONG_TD,ULONG_TD,
	      SHORT_TD,USHORT_TD,CHAR_TD,UCHAR_TD,
	      INT_TD,UINT_TD} Data_Type_t;

const static int gDataSize[] = {8,4,8,8,4,4,2,2,1,1,4,4};

/* some basic functions */
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN2(a, b) ((a) < (b) ? (a) : (b))

#define MAX3(a, b, c) ((a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c))
#define MIN3(a, b, c) ((a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c))
#define MEDIAN3(a, b, c) ((a)<=(b) ? ((b)<=(c)?(b):MAX2(a,c)) : ((c)<=(b)?(b):MIN2(a, c)))

INLINE float sqr(float a) { return a * a; }
INLINE double dsqr(double a) { return a * a; }
INLINE double dmax2(double a,double b) { return (a>b ? a : b); }
INLINE double dmin2(double a,double b) { return (a<b ? a : b); }
INLINE float fmax2(float a,float b) { return (a>b ? a : b); }
INLINE float fmin2(float a,float b) { return (a<b ? a : b); }
INLINE long lmax2(long a,long b) { return (a>b ? a : b); }
INLINE long lmin2(long a,long b) { return (a<b ? a : b); }
INLINE int imax2(int a,int b) { return (a>b ? a : b); }
INLINE int imin2(int a,int b) { return (a<b ? a : b); }

#define DEFINE_MAX3_FUNC(type, prefix)					\
  INLINE double TZ_CONCAT(prefix, max3) (type a, type b, type c) {	\
    return (a)>=(b) ? (a)>=(c)?(a):(c) : (b)>=(c)?(b):(c); }

#define DEFINE_MIN3_FUNC(type, prefix)					\
  INLINE double TZ_CONCAT(prefix, min3) (type a, type b, type c) {	\
    return (a)<=(b) ? (a)<=(c)?(a):(c) : (b)<=(c)?(b):(c); }

DEFINE_MAX3_FUNC(double, d) /* dmax3(a, b, c) */
DEFINE_MIN3_FUNC(double, d) /* dmin3(a, b, c) */
DEFINE_MAX3_FUNC(float, f)  /* fmax3(a, b, c) */
DEFINE_MIN3_FUNC(float, f)  /* dmin3(a, b, c) */
DEFINE_MAX3_FUNC(long, l)   /* lmax3(a, b, c) */
DEFINE_MIN3_FUNC(long, l)   /* lmin3(a, b, c) */
DEFINE_MAX3_FUNC(int, i)    /* imax3(a, b, c) */
DEFINE_MIN3_FUNC(int, i)    /* imin3(a, b, c) */

#define XOR_SWAP(a, b) {a ^= b; b ^= a; a ^= b;}
#define SWAP2(a, b, t) ((t) = (a), (a) = (b), (b) = (t))

#define IS_IN_CLOSE_RANGE(x, minv, maxv) (((x)>=(minv)) && ((x)<=(maxv)))
#define IS_IN_OPEN_RANGE(x, minv, maxv) (((x)>(minv)) && ((x)<(maxv)))

#define IS_IN_CLOSE_RANGE3(x, y, z, minx, maxx, miny, maxy, minz, maxz) \
  (IS_IN_CLOSE_RANGE(x, minx, maxx) && IS_IN_CLOSE_RANGE(y, miny, maxy) && \
   IS_IN_CLOSE_RANGE(z, minz, maxz))

#define IS_IN_OPEN_RANGE3(x, y, z, minx, maxx, miny, maxy, minz, maxz) \
  (IS_IN_OPEN_RANGE(x, minx, maxx) && IS_IN_OPEN_RANGE(y, miny, maxy) && \
   IS_IN_OPEN_RANGE(z, minz, maxz))

#define SHIFT2(a, b, c) {a = (b); b = (c);}
#define SHIFT3(a, b, c, d) {a = (b); b = (c); c = (d);}

/********************************/

/**@addtogroup utilities_ Utilities for general purposes (tz_utilities.h)
 * @{
 */

/* 
 * valid_type() tests if a type is valid. It returns 1 if the type is valid,
 * otherwise it returns 0.
 */
INLINE int valid_type(Data_Type_t type) { return ((int)type>=0 && 
						  (int)type<DATA_TYPE_NUMBER); }

/**@brief Timer starts.
 * 
 * tic() and toc() are timer functions. tic() starts a stopwatch timer. 
 */
void tic();

/**@brief Timer ends.
 *
 * toc() ends the timer and returns how many milliseconds has passed since the
 * the call of tic();
 */
tz_int64 toc();

/**@brief Print ending time.
 *
 * ptoc() print toc() to the standard output.
 */
void ptoc();

/**@brief Print ending time.
 *
 * pmtoc() print toc() to the standard output with the message <msg>.
 */
void pmtoc(const char *msg);

#define CLIP_VALUE(v, lower, upper) (v = ((v) < (lower)) ? (lower) : (((v) > (upper)) ? (upper) : (v)))

static inline double Clip_Value(double v, double lower, double upper) {
  return CLIP_VALUE(v, lower, upper);
}

/**@brief Test if a file exists.
 *
 * fexist() tests if a file exists. It returns 1 if the file exists, otherwise
 * it returns 0. It returns 0 if <filename> is NULL.
 */
int fexist(const char* filename);

/**@brief Test if a directory exists.
 *
 * dexist() tests if a directory exists. It returns 1 if the directory exists, 
 * otherwise it returns 0. It returns 0 if <path> is NULL.
 */
int dexist(const char *path);

/**@brief File copy.
 *
 * fcopy() copies <source> to <target>. It returns 0 if the copy is successful.
 */
int fcopy(const char *source, const char *target);

/**@brief Extract the extsion of a file name.
 * 
 * fextn() returns the extension of a file. It returns NULL if there is no 
 * extension or <filename> is NULL.
 */
const char *fextn(const char* filename);

/**@brief Test if a file name has an extension
 *
 * fhasext() returns 1 if <filename> has the extension <ext> or <filename> has
 * no extension and <ext> is NULL. Otherwise it returns 0.
 */
int fhasext(const char* filename, const char *ext);

/**@brief Extract filename without extension.
 * 
 * fname() returns the name of a file. It returns NULL if there is no 
 * file found. If <name> is NULL, it returns a new pointer, which is the user's
 * responsibility to free it.
 */
char* fname(const char *path, char *name);

/**@brief Extract directory name.
 * 
 * dname() returns the name of a directory. It returns NULL if there is no 
 * file found. If <name> is NULL, it returns a new pointer, which is the user's
 * responsibility to free it.
 */
char* dname(const char *path, char *name);

/**@brief The number of certain files in a directory.
 *
 * dir_fnum() returns the number of files that have the extension <ext> in the
 * directory <dir_name>.
 */
int dir_fnum(const char *dir_name, const char *ext);


/**@brief The number of certain files in a directory.
 *
 * dir_fnum() returns the number of files that match the regular expression
 * <pattern> in the directory <dir_name>.
 */
int dir_fnum_p(const char *dir_name, const char *pattern);

/**@brief The maximum number label of certain files in a directory.
 *
 * dir_fnum() returns the maximum label of the files that match the regular 
 * expression <pattern> in the directory <dir_name>. Here the label of a file
 * is defined as the last number of the file name.
 */
int dir_fnum_s(const char *dir_name, const char *pattern);

/**@brief Get the start and end number of the files in a directory.
 *
 */
int dir_fnum_pair(const char *dir_name, const char *pattern, int *start,
    int *end);

/**@brief Size of a file.
 *
 * fsize() returns the size of the file <filename> in the number of bytes.
 */
size_t fsize(const char *filename);

/**@brief Compare two files.
 *
 * fcmp() returns 0 if the two files <filepath1> and <filepath2> have the same
 * content. It returns a non-zero value if they are different.
 */
int fcmp(const char *filepath1, const char *filepath2);

/**@brief Test if two strings are equal.
 * It returns TRUE iff the two strings are equal or both are NULL.
 */
BOOL eqstr(const char *str1, const char *str2);

/**@brief Remove a file or directory.
 *
 * rmpath() attepmts to delete <path>
 */
void rmpath(const char *path);

/**@brief Get full path.
 *
 * fullpath() combine two strings into a file path. The returned pointer is
 * the same as <path> if it is not NULL.
 */
char* fullpath(const char *path1, const char *path2, char *path);

/**@brief  Get full path with an extension.
 *
 * fullpath_e() return a path that is the combination of <path1>, <path2> and
 * <ext>. The returned pointer is same as <path> if it is not NULL.
 */
char* fullpath_e(const char *path1, const char *path2, const char *ext, 
		 char *path);

/**@brief Free a pointer.
 *
 * safe_free() frees a pointer variable in safe way. *<ptr> becomes NULL after
 * the return of the function. <free_fun> is the function pointer to free 
 * <*ptr>.
 */
void safe_free(void **ptr, void (*free_fun) (void *ptr));

#define OBJECT_SAFE_FREE(obj, free_fun)		\
  if (obj != NULL) {				\
    free_fun(obj);				\
    obj = NULL;					\
  }

#if defined GUARDED_MALLOC_ARRAY
#  undef GUARDED_MALLOC_ARRAY
#endif

#define GUARDED_MALLOC_ARRAY(array, n, type)			\
  array = (type*) Guarded_Malloc(sizeof(type) * (n), (char*) __func__)

#define GUARDED_CALLOC_ARRAY(array, n, type)			\
  array = (type*) Guarded_Calloc(n, sizeof(type), (char*) __func__)

#define GUARDED_REALLOC_ARRAY(array, n, type)			\
  array = (type*) Guarded_Realloc(array, sizeof(type) * (n), (char*) __func__)

#define GUARDED_FOPEN(path, mode)		\
  Guarded_Fopen((char*) path, mode, (char*) __func__)

#if defined MALLOC_2D_ARRAY
#  undef MALLOC_2D_ARRAY
#endif

#if defined FREE_2D_ARRAY
#  undef FREE_2D_ARRAY
#endif

#define MALLOC_2D_ARRAY(array, m, n, type, i)		\
  array = (type **) malloc(sizeof(type*) * m);		\
  for (i = 0; i < m; i++) {				\
    array[i] = (type *) malloc(sizeof(type) * n);	\
  }

#define FREE_2D_ARRAY(array, n)			\
  if (array != NULL) {				\
    int i;					\
    for (i = 0; i < n; i++) {			\
      if (array[i] != NULL) {			\
	free(array[i]);				\
	array[i] = NULL;			\
      }						\
    }						\
    free(array);				\
    array = NULL;				\
  }

/**@brief double value to a string of its binary representation
 *
 * double_binstr() turns a double into a string of its binary representation in
 * the current operating system. 
 * The result is stored in str. As a result of the definition of a double, the
 * resulted string will have length 64. */
char* double_binstr(double x, char str[]);

/**@brief Test if a value is NaN.
 */
int tz_isnan(double x);

/**@brief Test if a value is infinity.
 *
 * tz_isinf() returns 1 if <x> is Infinity of -Infinity.
 */
int tz_isinf(double x);

/* in case fgetln is not defined externally */
extern char* fgetln(FILE *stream, size_t *len);

/**@brief Check the increasing or decreasing trend of an array of values.
 */
double Value_Trend(double *value, int n);

/**@brief Show the version of a program.
 *
 * Show_Version() prints the version string <ver> to the screen if <argv[1]>
 * is "-ver". 
 */
int Show_Version(int argc, char *argv[], const char *ver);

/**@brief Print arguments to standard output.
 */
void Print_Arguments(int argc, char *argv[]);

/**@}*/

/**@brief Hex string to uint32.
 */
uint32_t Hexstr_To_Uint(char *str);

/**@brief uint32 to hex string.
 */
char* Uint_To_Hexstr(uint32_t num, char *str);

/**@brief Print a file in the hex format.
 */
void Fprint_File_Binary(FILE *fp, size_t n, FILE *out);

void *Guarded_Calloc(size_t count, size_t size, const char *routine);

/* n = 2^k + m - 1
 * k: starts from 1
 * m: starts from 0
 * n: starts from 1
 */
void pow2decomp(int n, int *k, int *m);
int pow2sum(int k, int m);

void Print_Argument_Spec(const char * const spec[]);

/**@brief 4-byte-wise memset.
 *
 * This is a backup for memset_pattern4 in case it is not available.
 */
void Memset_Pattern4(void *b, const void *pattern4, size_t len);

#ifndef HAVE_MEMSET_PATTERN4
#define memset_pattern4 Memset_Pattern4
#endif

/**@brief Test if an index value is valid.
 *
 * In this library the maximum index value is defined as an invalid index.
 * Is_Valid_Array_Index() returns TRUE iff <index> is valid.
 */
BOOL Is_Valid_Array_Index(size_t index);

__END_DECLS

#endif

/******************************************************************************/
