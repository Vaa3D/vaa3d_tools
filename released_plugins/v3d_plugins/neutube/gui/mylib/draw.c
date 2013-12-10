#ifdef _MSC_VER

#pragma warning( disable:4996 )

#ifndef __cplusplus
#define inline __inline
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

//  WINDOW pthreads "LIBRARY"

  //  Mutex macros

typedef SRWLOCK pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER RTL_SRWLOCK_INIT

#define pthread_mutex_lock(m)  AcquireSRWLockExclusive(m)

#define pthread_mutex_unlock(m) ReleaseSRWLockExclusive(m)

  //  Condition variable macros

typedef CONDITION_VARIABLE pthread_cond_t;

#define PTHREAD_COND_INITIALIZER RTL_CONDITION_VARIABLE_INIT

#define pthread_cond_signal(c) WakeConditionVariable(c)

#define pthread_cond_broadcast(c) WakeAllConditionVariable(c)

#define pthread_cond_wait(c,m) SleepConditionVariableSRW(c,m,INFINITE,0)

  //  Simple thread support

typedef struct
  { HANDLE *handle;
    void   *(*fct)(void *);
    void   *arg;
    void   *retval;
    int     id;
  } Mythread;

typedef Mythread *pthread_t;

static DWORD WINAPI MyStart(void *arg)
{ Mythread *tv = (Mythread *) arg;

  tv->retval = tv->fct(tv->arg);
  return (0);
}

static int pthread_create(pthread_t *thread, void *attr,
                          void *(*fct)(void *), void *arg)
{ Mythread *tv;
  if (attr != NULL)
    { fprintf(stderr,"Do not support thread attributes\n");
      exit (1);
    }
  tv = (Mythread *) malloc(sizeof(Mythread));
  if (tv == NULL)
    { fprintf(stderr,"pthread_create: Out of memory.\n");
      exit (1);
    };
  tv->fct    = fct;
  tv->arg    = arg;
  tv->handle = CreateThread(NULL,0,MyStart,tv,0,&tv->id);
  if (tv->handle == NULL)
    return (EAGAIN);
  else
    return (0);
}

static int pthread_join(pthread_t t, void **ret)
{ Mythread *tv = (Mythread *) t;

  WaitForSingleObject(tv->handle,INFINITE);
  if (ret != NULL)
    *ret = tv->retval;

  CloseHandle(tv->handle);
  free(tv);
  return (0);
}

typedef int pthread_id;

static pthread_id pthread_tag()
{ return (GetCurrentThreadId()); }

static int pthread_is_this(pthread_id id)
{ return (GetCurrentThreadId() == id); }

#else   //  Small extension to pthreads!

#include <pthread.h>

typedef pthread_t pthread_id;

#define pthread_tag() pthread_self()

static inline int pthread_is_this(pthread_id id)
{ return (pthread_equal(pthread_self(),id)); }

#endif

/******************************************************************************************\
*                                                                                         *
*  Drawing Level Sets, Regions, and Basic Shapes                                          *
*                                                                                         *
*  Author  :  Gene Myers                                                                  *
*  Date    :  June 2007                                                                   *
*  Last Mod:  Aug 2008                                                                    *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <stdint.h>
#include <math.h>

#include "utilities.h"
#include "connectivity.h"
#include "draw.h"
#include "level.set.h"

//  A Brush_Bundle determines the setting of the following globals that are then
//    used by the various painter routines below to set pixels.



typedef struct        //  Generic container for a "Paint bundle"
  { void   *red;
    void   *green;
    void   *blue;
    void   *alpha;
    double  redI;
    double  greenI;
    double  blueI;
    double  alphaI;
  } PAINT_VOID;

 //  Paint brush bundle for each type & operator combo

  typedef struct
    { uint8    *red;       //  Typed paint bundle for non-multiplicative ops
      uint8    *green;
      uint8    *blue;
      uint8    *alpha;
      uint8     redI;
      uint8     greenI;
      uint8     blueI;
      uint8     alphaI;
    } PAINT_UINT8;

  typedef struct
    { uint8    *red;       //  Typed paint bundle for multiplicative ops
      uint8    *green;
      uint8    *blue;
      uint8    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_UINT8;


  #define CPUINT8(a)  ((PAINT_UINT8 *) (a))
  #define CRUINT8(a)  ((MUPNT_UINT8 *) (a))

  typedef struct
    { uint16    *red;       //  Typed paint bundle for non-multiplicative ops
      uint16    *green;
      uint16    *blue;
      uint16    *alpha;
      uint16     redI;
      uint16     greenI;
      uint16     blueI;
      uint16     alphaI;
    } PAINT_UINT16;

  typedef struct
    { uint16    *red;       //  Typed paint bundle for multiplicative ops
      uint16    *green;
      uint16    *blue;
      uint16    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_UINT16;


  #define CPUINT16(a)  ((PAINT_UINT16 *) (a))
  #define CRUINT16(a)  ((MUPNT_UINT16 *) (a))

  typedef struct
    { uint32    *red;       //  Typed paint bundle for non-multiplicative ops
      uint32    *green;
      uint32    *blue;
      uint32    *alpha;
      uint32     redI;
      uint32     greenI;
      uint32     blueI;
      uint32     alphaI;
    } PAINT_UINT32;

  typedef struct
    { uint32    *red;       //  Typed paint bundle for multiplicative ops
      uint32    *green;
      uint32    *blue;
      uint32    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_UINT32;


  #define CPUINT32(a)  ((PAINT_UINT32 *) (a))
  #define CRUINT32(a)  ((MUPNT_UINT32 *) (a))

  typedef struct
    { uint64    *red;       //  Typed paint bundle for non-multiplicative ops
      uint64    *green;
      uint64    *blue;
      uint64    *alpha;
      uint64     redI;
      uint64     greenI;
      uint64     blueI;
      uint64     alphaI;
    } PAINT_UINT64;

  typedef struct
    { uint64    *red;       //  Typed paint bundle for multiplicative ops
      uint64    *green;
      uint64    *blue;
      uint64    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_UINT64;


  #define CPUINT64(a)  ((PAINT_UINT64 *) (a))
  #define CRUINT64(a)  ((MUPNT_UINT64 *) (a))

  typedef struct
    { int8    *red;       //  Typed paint bundle for non-multiplicative ops
      int8    *green;
      int8    *blue;
      int8    *alpha;
      int8     redI;
      int8     greenI;
      int8     blueI;
      int8     alphaI;
    } PAINT_INT8;

  typedef struct
    { int8    *red;       //  Typed paint bundle for multiplicative ops
      int8    *green;
      int8    *blue;
      int8    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_INT8;


  #define CPINT8(a)  ((PAINT_INT8 *) (a))
  #define CRINT8(a)  ((MUPNT_INT8 *) (a))

  typedef struct
    { int16    *red;       //  Typed paint bundle for non-multiplicative ops
      int16    *green;
      int16    *blue;
      int16    *alpha;
      int16     redI;
      int16     greenI;
      int16     blueI;
      int16     alphaI;
    } PAINT_INT16;

  typedef struct
    { int16    *red;       //  Typed paint bundle for multiplicative ops
      int16    *green;
      int16    *blue;
      int16    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_INT16;


  #define CPINT16(a)  ((PAINT_INT16 *) (a))
  #define CRINT16(a)  ((MUPNT_INT16 *) (a))

  typedef struct
    { int32    *red;       //  Typed paint bundle for non-multiplicative ops
      int32    *green;
      int32    *blue;
      int32    *alpha;
      int32     redI;
      int32     greenI;
      int32     blueI;
      int32     alphaI;
    } PAINT_INT32;

  typedef struct
    { int32    *red;       //  Typed paint bundle for multiplicative ops
      int32    *green;
      int32    *blue;
      int32    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_INT32;


  #define CPINT32(a)  ((PAINT_INT32 *) (a))
  #define CRINT32(a)  ((MUPNT_INT32 *) (a))

  typedef struct
    { int64    *red;       //  Typed paint bundle for non-multiplicative ops
      int64    *green;
      int64    *blue;
      int64    *alpha;
      int64     redI;
      int64     greenI;
      int64     blueI;
      int64     alphaI;
    } PAINT_INT64;

  typedef struct
    { int64    *red;       //  Typed paint bundle for multiplicative ops
      int64    *green;
      int64    *blue;
      int64    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_INT64;


  #define CPINT64(a)  ((PAINT_INT64 *) (a))
  #define CRINT64(a)  ((MUPNT_INT64 *) (a))

  typedef struct
    { float32    *red;       //  Typed paint bundle for non-multiplicative ops
      float32    *green;
      float32    *blue;
      float32    *alpha;
      float32     redI;
      float32     greenI;
      float32     blueI;
      float32     alphaI;
    } PAINT_FLOAT32;

  typedef struct
    { float32    *red;       //  Typed paint bundle for multiplicative ops
      float32    *green;
      float32    *blue;
      float32    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_FLOAT32;


  #define CPFLOAT32(a)  ((PAINT_FLOAT32 *) (a))
  #define CRFLOAT32(a)  ((MUPNT_FLOAT32 *) (a))

  typedef struct
    { float64    *red;       //  Typed paint bundle for non-multiplicative ops
      float64    *green;
      float64    *blue;
      float64    *alpha;
      float64     redI;
      float64     greenI;
      float64     blueI;
      float64     alphaI;
    } PAINT_FLOAT64;

  typedef struct
    { float64    *red;       //  Typed paint bundle for multiplicative ops
      float64    *green;
      float64    *blue;
      float64    *alpha;
      float64 redI;
      float64 greenI;
      float64 blueI;
      float64 alphaI;
    } MUPNT_FLOAT64;


  #define CPFLOAT64(a)  ((PAINT_FLOAT64 *) (a))
  #define CRFLOAT64(a)  ((MUPNT_FLOAT64 *) (a))


  //  Generate a painter for each image kind, array type, and paint operator combination!


      static void PAINT_UINT8_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPUINT8(a)->red[p] = CPUINT8(a)->redI; }

      static void PAINT_UINT8_RGB_SET_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
          CPUINT8(a)->blue[p] = CPUINT8(a)->blueI;
      }

      static void PAINT_UINT8_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
          CPUINT8(a)->blue[p] = CPUINT8(a)->blueI;
          CPUINT8(a)->alpha[p] = CPUINT8(a)->alphaI;
      }

      static void PAINT_UINT8_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
      }


      static void PAINT_UINT8_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPUINT8(a)->red[p] += CPUINT8(a)->redI; }

      static void PAINT_UINT8_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] += CPUINT8(a)->redI;
          CPUINT8(a)->green[p] += CPUINT8(a)->greenI;
          CPUINT8(a)->blue[p] += CPUINT8(a)->blueI;
      }

      static void PAINT_UINT8_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] += CPUINT8(a)->redI;
          CPUINT8(a)->green[p] += CPUINT8(a)->greenI;
          CPUINT8(a)->blue[p] += CPUINT8(a)->blueI;
          CPUINT8(a)->alpha[p] += CPUINT8(a)->alphaI;
      }

      static void PAINT_UINT8_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] += CPUINT8(a)->redI;
          CPUINT8(a)->green[p] += CPUINT8(a)->greenI;
      }


      static void PAINT_UINT8_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPUINT8(a)->red[p] -= CPUINT8(a)->redI; }

      static void PAINT_UINT8_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] -= CPUINT8(a)->redI;
          CPUINT8(a)->green[p] -= CPUINT8(a)->greenI;
          CPUINT8(a)->blue[p] -= CPUINT8(a)->blueI;
      }

      static void PAINT_UINT8_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] -= CPUINT8(a)->redI;
          CPUINT8(a)->green[p] -= CPUINT8(a)->greenI;
          CPUINT8(a)->blue[p] -= CPUINT8(a)->blueI;
          CPUINT8(a)->alpha[p] -= CPUINT8(a)->alphaI;
      }

      static void PAINT_UINT8_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPUINT8(a)->red[p] -= CPUINT8(a)->redI;
          CPUINT8(a)->green[p] -= CPUINT8(a)->greenI;
      }


      static void PAINT_UINT8_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] * CRUINT8(a)->redI); }

      static void PAINT_UINT8_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] * CRUINT8(a)->redI);
          CRUINT8(a)->green[p] = (uint8) (CRUINT8(a)->green[p] * CRUINT8(a)->greenI);
          CRUINT8(a)->blue[p] = (uint8) (CRUINT8(a)->blue[p] * CRUINT8(a)->blueI);
      }

      static void PAINT_UINT8_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] * CRUINT8(a)->redI);
          CRUINT8(a)->green[p] = (uint8) (CRUINT8(a)->green[p] * CRUINT8(a)->greenI);
          CRUINT8(a)->blue[p] = (uint8) (CRUINT8(a)->blue[p] * CRUINT8(a)->blueI);
          CRUINT8(a)->alpha[p] = (uint8) (CRUINT8(a)->alpha[p] * CRUINT8(a)->alphaI);
      }

      static void PAINT_UINT8_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] * CRUINT8(a)->redI);
          CRUINT8(a)->green[p] = (uint8) (CRUINT8(a)->green[p] * CRUINT8(a)->greenI);
      }


      static void PAINT_UINT8_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] / CRUINT8(a)->redI); }

      static void PAINT_UINT8_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] / CRUINT8(a)->redI);
          CRUINT8(a)->green[p] = (uint8) (CRUINT8(a)->green[p] / CRUINT8(a)->greenI);
          CRUINT8(a)->blue[p] = (uint8) (CRUINT8(a)->blue[p] / CRUINT8(a)->blueI);
      }

      static void PAINT_UINT8_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] / CRUINT8(a)->redI);
          CRUINT8(a)->green[p] = (uint8) (CRUINT8(a)->green[p] / CRUINT8(a)->greenI);
          CRUINT8(a)->blue[p] = (uint8) (CRUINT8(a)->blue[p] / CRUINT8(a)->blueI);
          CRUINT8(a)->alpha[p] = (uint8) (CRUINT8(a)->alpha[p] / CRUINT8(a)->alphaI);
      }

      static void PAINT_UINT8_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRUINT8(a)->red[p] = (uint8) (CRUINT8(a)->red[p] / CRUINT8(a)->redI);
          CRUINT8(a)->green[p] = (uint8) (CRUINT8(a)->green[p] / CRUINT8(a)->greenI);
      }


      static void PAINT_UINT8_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { uint8 x = CPUINT8(a)->red[p];
        if (CPUINT8(a)->redI < x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
      }

      static void PAINT_UINT8_RGB_MIN_PIX(Indx_Type p, void *a) {
        uint8 x;
          x = CPUINT8(a)->red[p];
          if (CPUINT8(a)->redI < x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          x = CPUINT8(a)->green[p];
          if (CPUINT8(a)->greenI < x) CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
          x = CPUINT8(a)->blue[p];
          if (CPUINT8(a)->blueI < x) CPUINT8(a)->blue[p] = CPUINT8(a)->blueI;
      }

      static void PAINT_UINT8_RGBA_MIN_PIX(Indx_Type p, void *a) {
        uint8 x;
          x = CPUINT8(a)->red[p];
          if (CPUINT8(a)->redI < x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          x = CPUINT8(a)->green[p];
          if (CPUINT8(a)->greenI < x) CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
          x = CPUINT8(a)->blue[p];
          if (CPUINT8(a)->blueI < x) CPUINT8(a)->blue[p] = CPUINT8(a)->blueI;
          x = CPUINT8(a)->alpha[p];
          if (CPUINT8(a)->alphaI < x) CPUINT8(a)->alpha[p] = CPUINT8(a)->alphaI;
      }

      static void PAINT_UINT8_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        uint8 x;
        p <<= 1;
          x = CPUINT8(a)->red[p];
          if (CPUINT8(a)->redI < x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          x = CPUINT8(a)->green[p];
          if (CPUINT8(a)->greenI < x) CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
      }


      static void PAINT_UINT8_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { uint8 x = CPUINT8(a)->red[p];
        if (CPUINT8(a)->redI > x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
      }

      static void PAINT_UINT8_RGB_MAX_PIX(Indx_Type p, void *a) {
        uint8 x;
          x = CPUINT8(a)->red[p];
          if (CPUINT8(a)->redI > x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          x = CPUINT8(a)->green[p];
          if (CPUINT8(a)->greenI > x) CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
          x = CPUINT8(a)->blue[p];
          if (CPUINT8(a)->blueI > x) CPUINT8(a)->blue[p] = CPUINT8(a)->blueI;
      }

      static void PAINT_UINT8_RGBA_MAX_PIX(Indx_Type p, void *a) {
        uint8 x;
          x = CPUINT8(a)->red[p];
          if (CPUINT8(a)->redI > x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          x = CPUINT8(a)->green[p];
          if (CPUINT8(a)->greenI > x) CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
          x = CPUINT8(a)->blue[p];
          if (CPUINT8(a)->blueI > x) CPUINT8(a)->blue[p] = CPUINT8(a)->blueI;
          x = CPUINT8(a)->alpha[p];
          if (CPUINT8(a)->alphaI > x) CPUINT8(a)->alpha[p] = CPUINT8(a)->alphaI;
      }

      static void PAINT_UINT8_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        uint8 x;
        p <<= 1;
          x = CPUINT8(a)->red[p];
          if (CPUINT8(a)->redI > x) CPUINT8(a)->red[p] = CPUINT8(a)->redI;
          x = CPUINT8(a)->green[p];
          if (CPUINT8(a)->greenI > x) CPUINT8(a)->green[p] = CPUINT8(a)->greenI;
      }


      static void PAINT_UINT16_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPUINT16(a)->red[p] = CPUINT16(a)->redI; }

      static void PAINT_UINT16_RGB_SET_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
          CPUINT16(a)->blue[p] = CPUINT16(a)->blueI;
      }

      static void PAINT_UINT16_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
          CPUINT16(a)->blue[p] = CPUINT16(a)->blueI;
          CPUINT16(a)->alpha[p] = CPUINT16(a)->alphaI;
      }

      static void PAINT_UINT16_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
      }


      static void PAINT_UINT16_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPUINT16(a)->red[p] += CPUINT16(a)->redI; }

      static void PAINT_UINT16_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] += CPUINT16(a)->redI;
          CPUINT16(a)->green[p] += CPUINT16(a)->greenI;
          CPUINT16(a)->blue[p] += CPUINT16(a)->blueI;
      }

      static void PAINT_UINT16_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] += CPUINT16(a)->redI;
          CPUINT16(a)->green[p] += CPUINT16(a)->greenI;
          CPUINT16(a)->blue[p] += CPUINT16(a)->blueI;
          CPUINT16(a)->alpha[p] += CPUINT16(a)->alphaI;
      }

      static void PAINT_UINT16_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] += CPUINT16(a)->redI;
          CPUINT16(a)->green[p] += CPUINT16(a)->greenI;
      }


      static void PAINT_UINT16_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPUINT16(a)->red[p] -= CPUINT16(a)->redI; }

      static void PAINT_UINT16_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] -= CPUINT16(a)->redI;
          CPUINT16(a)->green[p] -= CPUINT16(a)->greenI;
          CPUINT16(a)->blue[p] -= CPUINT16(a)->blueI;
      }

      static void PAINT_UINT16_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] -= CPUINT16(a)->redI;
          CPUINT16(a)->green[p] -= CPUINT16(a)->greenI;
          CPUINT16(a)->blue[p] -= CPUINT16(a)->blueI;
          CPUINT16(a)->alpha[p] -= CPUINT16(a)->alphaI;
      }

      static void PAINT_UINT16_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPUINT16(a)->red[p] -= CPUINT16(a)->redI;
          CPUINT16(a)->green[p] -= CPUINT16(a)->greenI;
      }


      static void PAINT_UINT16_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] * CRUINT16(a)->redI); }

      static void PAINT_UINT16_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] * CRUINT16(a)->redI);
          CRUINT16(a)->green[p] = (uint16) (CRUINT16(a)->green[p] * CRUINT16(a)->greenI);
          CRUINT16(a)->blue[p] = (uint16) (CRUINT16(a)->blue[p] * CRUINT16(a)->blueI);
      }

      static void PAINT_UINT16_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] * CRUINT16(a)->redI);
          CRUINT16(a)->green[p] = (uint16) (CRUINT16(a)->green[p] * CRUINT16(a)->greenI);
          CRUINT16(a)->blue[p] = (uint16) (CRUINT16(a)->blue[p] * CRUINT16(a)->blueI);
          CRUINT16(a)->alpha[p] = (uint16) (CRUINT16(a)->alpha[p] * CRUINT16(a)->alphaI);
      }

      static void PAINT_UINT16_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] * CRUINT16(a)->redI);
          CRUINT16(a)->green[p] = (uint16) (CRUINT16(a)->green[p] * CRUINT16(a)->greenI);
      }


      static void PAINT_UINT16_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] / CRUINT16(a)->redI); }

      static void PAINT_UINT16_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] / CRUINT16(a)->redI);
          CRUINT16(a)->green[p] = (uint16) (CRUINT16(a)->green[p] / CRUINT16(a)->greenI);
          CRUINT16(a)->blue[p] = (uint16) (CRUINT16(a)->blue[p] / CRUINT16(a)->blueI);
      }

      static void PAINT_UINT16_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] / CRUINT16(a)->redI);
          CRUINT16(a)->green[p] = (uint16) (CRUINT16(a)->green[p] / CRUINT16(a)->greenI);
          CRUINT16(a)->blue[p] = (uint16) (CRUINT16(a)->blue[p] / CRUINT16(a)->blueI);
          CRUINT16(a)->alpha[p] = (uint16) (CRUINT16(a)->alpha[p] / CRUINT16(a)->alphaI);
      }

      static void PAINT_UINT16_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRUINT16(a)->red[p] = (uint16) (CRUINT16(a)->red[p] / CRUINT16(a)->redI);
          CRUINT16(a)->green[p] = (uint16) (CRUINT16(a)->green[p] / CRUINT16(a)->greenI);
      }


      static void PAINT_UINT16_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { uint16 x = CPUINT16(a)->red[p];
        if (CPUINT16(a)->redI < x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
      }

      static void PAINT_UINT16_RGB_MIN_PIX(Indx_Type p, void *a) {
        uint16 x;
          x = CPUINT16(a)->red[p];
          if (CPUINT16(a)->redI < x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          x = CPUINT16(a)->green[p];
          if (CPUINT16(a)->greenI < x) CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
          x = CPUINT16(a)->blue[p];
          if (CPUINT16(a)->blueI < x) CPUINT16(a)->blue[p] = CPUINT16(a)->blueI;
      }

      static void PAINT_UINT16_RGBA_MIN_PIX(Indx_Type p, void *a) {
        uint16 x;
          x = CPUINT16(a)->red[p];
          if (CPUINT16(a)->redI < x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          x = CPUINT16(a)->green[p];
          if (CPUINT16(a)->greenI < x) CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
          x = CPUINT16(a)->blue[p];
          if (CPUINT16(a)->blueI < x) CPUINT16(a)->blue[p] = CPUINT16(a)->blueI;
          x = CPUINT16(a)->alpha[p];
          if (CPUINT16(a)->alphaI < x) CPUINT16(a)->alpha[p] = CPUINT16(a)->alphaI;
      }

      static void PAINT_UINT16_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        uint16 x;
        p <<= 1;
          x = CPUINT16(a)->red[p];
          if (CPUINT16(a)->redI < x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          x = CPUINT16(a)->green[p];
          if (CPUINT16(a)->greenI < x) CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
      }


      static void PAINT_UINT16_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { uint16 x = CPUINT16(a)->red[p];
        if (CPUINT16(a)->redI > x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
      }

      static void PAINT_UINT16_RGB_MAX_PIX(Indx_Type p, void *a) {
        uint16 x;
          x = CPUINT16(a)->red[p];
          if (CPUINT16(a)->redI > x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          x = CPUINT16(a)->green[p];
          if (CPUINT16(a)->greenI > x) CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
          x = CPUINT16(a)->blue[p];
          if (CPUINT16(a)->blueI > x) CPUINT16(a)->blue[p] = CPUINT16(a)->blueI;
      }

      static void PAINT_UINT16_RGBA_MAX_PIX(Indx_Type p, void *a) {
        uint16 x;
          x = CPUINT16(a)->red[p];
          if (CPUINT16(a)->redI > x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          x = CPUINT16(a)->green[p];
          if (CPUINT16(a)->greenI > x) CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
          x = CPUINT16(a)->blue[p];
          if (CPUINT16(a)->blueI > x) CPUINT16(a)->blue[p] = CPUINT16(a)->blueI;
          x = CPUINT16(a)->alpha[p];
          if (CPUINT16(a)->alphaI > x) CPUINT16(a)->alpha[p] = CPUINT16(a)->alphaI;
      }

      static void PAINT_UINT16_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        uint16 x;
        p <<= 1;
          x = CPUINT16(a)->red[p];
          if (CPUINT16(a)->redI > x) CPUINT16(a)->red[p] = CPUINT16(a)->redI;
          x = CPUINT16(a)->green[p];
          if (CPUINT16(a)->greenI > x) CPUINT16(a)->green[p] = CPUINT16(a)->greenI;
      }


      static void PAINT_UINT32_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPUINT32(a)->red[p] = CPUINT32(a)->redI; }

      static void PAINT_UINT32_RGB_SET_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
          CPUINT32(a)->blue[p] = CPUINT32(a)->blueI;
      }

      static void PAINT_UINT32_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
          CPUINT32(a)->blue[p] = CPUINT32(a)->blueI;
          CPUINT32(a)->alpha[p] = CPUINT32(a)->alphaI;
      }

      static void PAINT_UINT32_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
      }


      static void PAINT_UINT32_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPUINT32(a)->red[p] += CPUINT32(a)->redI; }

      static void PAINT_UINT32_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] += CPUINT32(a)->redI;
          CPUINT32(a)->green[p] += CPUINT32(a)->greenI;
          CPUINT32(a)->blue[p] += CPUINT32(a)->blueI;
      }

      static void PAINT_UINT32_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] += CPUINT32(a)->redI;
          CPUINT32(a)->green[p] += CPUINT32(a)->greenI;
          CPUINT32(a)->blue[p] += CPUINT32(a)->blueI;
          CPUINT32(a)->alpha[p] += CPUINT32(a)->alphaI;
      }

      static void PAINT_UINT32_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] += CPUINT32(a)->redI;
          CPUINT32(a)->green[p] += CPUINT32(a)->greenI;
      }


      static void PAINT_UINT32_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPUINT32(a)->red[p] -= CPUINT32(a)->redI; }

      static void PAINT_UINT32_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] -= CPUINT32(a)->redI;
          CPUINT32(a)->green[p] -= CPUINT32(a)->greenI;
          CPUINT32(a)->blue[p] -= CPUINT32(a)->blueI;
      }

      static void PAINT_UINT32_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] -= CPUINT32(a)->redI;
          CPUINT32(a)->green[p] -= CPUINT32(a)->greenI;
          CPUINT32(a)->blue[p] -= CPUINT32(a)->blueI;
          CPUINT32(a)->alpha[p] -= CPUINT32(a)->alphaI;
      }

      static void PAINT_UINT32_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPUINT32(a)->red[p] -= CPUINT32(a)->redI;
          CPUINT32(a)->green[p] -= CPUINT32(a)->greenI;
      }


      static void PAINT_UINT32_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] * CRUINT32(a)->redI); }

      static void PAINT_UINT32_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] * CRUINT32(a)->redI);
          CRUINT32(a)->green[p] = (uint32) (CRUINT32(a)->green[p] * CRUINT32(a)->greenI);
          CRUINT32(a)->blue[p] = (uint32) (CRUINT32(a)->blue[p] * CRUINT32(a)->blueI);
      }

      static void PAINT_UINT32_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] * CRUINT32(a)->redI);
          CRUINT32(a)->green[p] = (uint32) (CRUINT32(a)->green[p] * CRUINT32(a)->greenI);
          CRUINT32(a)->blue[p] = (uint32) (CRUINT32(a)->blue[p] * CRUINT32(a)->blueI);
          CRUINT32(a)->alpha[p] = (uint32) (CRUINT32(a)->alpha[p] * CRUINT32(a)->alphaI);
      }

      static void PAINT_UINT32_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] * CRUINT32(a)->redI);
          CRUINT32(a)->green[p] = (uint32) (CRUINT32(a)->green[p] * CRUINT32(a)->greenI);
      }


      static void PAINT_UINT32_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] / CRUINT32(a)->redI); }

      static void PAINT_UINT32_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] / CRUINT32(a)->redI);
          CRUINT32(a)->green[p] = (uint32) (CRUINT32(a)->green[p] / CRUINT32(a)->greenI);
          CRUINT32(a)->blue[p] = (uint32) (CRUINT32(a)->blue[p] / CRUINT32(a)->blueI);
      }

      static void PAINT_UINT32_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] / CRUINT32(a)->redI);
          CRUINT32(a)->green[p] = (uint32) (CRUINT32(a)->green[p] / CRUINT32(a)->greenI);
          CRUINT32(a)->blue[p] = (uint32) (CRUINT32(a)->blue[p] / CRUINT32(a)->blueI);
          CRUINT32(a)->alpha[p] = (uint32) (CRUINT32(a)->alpha[p] / CRUINT32(a)->alphaI);
      }

      static void PAINT_UINT32_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRUINT32(a)->red[p] = (uint32) (CRUINT32(a)->red[p] / CRUINT32(a)->redI);
          CRUINT32(a)->green[p] = (uint32) (CRUINT32(a)->green[p] / CRUINT32(a)->greenI);
      }


      static void PAINT_UINT32_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { uint32 x = CPUINT32(a)->red[p];
        if (CPUINT32(a)->redI < x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
      }

      static void PAINT_UINT32_RGB_MIN_PIX(Indx_Type p, void *a) {
        uint32 x;
          x = CPUINT32(a)->red[p];
          if (CPUINT32(a)->redI < x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          x = CPUINT32(a)->green[p];
          if (CPUINT32(a)->greenI < x) CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
          x = CPUINT32(a)->blue[p];
          if (CPUINT32(a)->blueI < x) CPUINT32(a)->blue[p] = CPUINT32(a)->blueI;
      }

      static void PAINT_UINT32_RGBA_MIN_PIX(Indx_Type p, void *a) {
        uint32 x;
          x = CPUINT32(a)->red[p];
          if (CPUINT32(a)->redI < x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          x = CPUINT32(a)->green[p];
          if (CPUINT32(a)->greenI < x) CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
          x = CPUINT32(a)->blue[p];
          if (CPUINT32(a)->blueI < x) CPUINT32(a)->blue[p] = CPUINT32(a)->blueI;
          x = CPUINT32(a)->alpha[p];
          if (CPUINT32(a)->alphaI < x) CPUINT32(a)->alpha[p] = CPUINT32(a)->alphaI;
      }

      static void PAINT_UINT32_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        uint32 x;
        p <<= 1;
          x = CPUINT32(a)->red[p];
          if (CPUINT32(a)->redI < x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          x = CPUINT32(a)->green[p];
          if (CPUINT32(a)->greenI < x) CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
      }


      static void PAINT_UINT32_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { uint32 x = CPUINT32(a)->red[p];
        if (CPUINT32(a)->redI > x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
      }

      static void PAINT_UINT32_RGB_MAX_PIX(Indx_Type p, void *a) {
        uint32 x;
          x = CPUINT32(a)->red[p];
          if (CPUINT32(a)->redI > x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          x = CPUINT32(a)->green[p];
          if (CPUINT32(a)->greenI > x) CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
          x = CPUINT32(a)->blue[p];
          if (CPUINT32(a)->blueI > x) CPUINT32(a)->blue[p] = CPUINT32(a)->blueI;
      }

      static void PAINT_UINT32_RGBA_MAX_PIX(Indx_Type p, void *a) {
        uint32 x;
          x = CPUINT32(a)->red[p];
          if (CPUINT32(a)->redI > x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          x = CPUINT32(a)->green[p];
          if (CPUINT32(a)->greenI > x) CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
          x = CPUINT32(a)->blue[p];
          if (CPUINT32(a)->blueI > x) CPUINT32(a)->blue[p] = CPUINT32(a)->blueI;
          x = CPUINT32(a)->alpha[p];
          if (CPUINT32(a)->alphaI > x) CPUINT32(a)->alpha[p] = CPUINT32(a)->alphaI;
      }

      static void PAINT_UINT32_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        uint32 x;
        p <<= 1;
          x = CPUINT32(a)->red[p];
          if (CPUINT32(a)->redI > x) CPUINT32(a)->red[p] = CPUINT32(a)->redI;
          x = CPUINT32(a)->green[p];
          if (CPUINT32(a)->greenI > x) CPUINT32(a)->green[p] = CPUINT32(a)->greenI;
      }


      static void PAINT_UINT64_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPUINT64(a)->red[p] = CPUINT64(a)->redI; }

      static void PAINT_UINT64_RGB_SET_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
          CPUINT64(a)->blue[p] = CPUINT64(a)->blueI;
      }

      static void PAINT_UINT64_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
          CPUINT64(a)->blue[p] = CPUINT64(a)->blueI;
          CPUINT64(a)->alpha[p] = CPUINT64(a)->alphaI;
      }

      static void PAINT_UINT64_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
      }


      static void PAINT_UINT64_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPUINT64(a)->red[p] += CPUINT64(a)->redI; }

      static void PAINT_UINT64_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] += CPUINT64(a)->redI;
          CPUINT64(a)->green[p] += CPUINT64(a)->greenI;
          CPUINT64(a)->blue[p] += CPUINT64(a)->blueI;
      }

      static void PAINT_UINT64_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] += CPUINT64(a)->redI;
          CPUINT64(a)->green[p] += CPUINT64(a)->greenI;
          CPUINT64(a)->blue[p] += CPUINT64(a)->blueI;
          CPUINT64(a)->alpha[p] += CPUINT64(a)->alphaI;
      }

      static void PAINT_UINT64_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] += CPUINT64(a)->redI;
          CPUINT64(a)->green[p] += CPUINT64(a)->greenI;
      }


      static void PAINT_UINT64_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPUINT64(a)->red[p] -= CPUINT64(a)->redI; }

      static void PAINT_UINT64_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] -= CPUINT64(a)->redI;
          CPUINT64(a)->green[p] -= CPUINT64(a)->greenI;
          CPUINT64(a)->blue[p] -= CPUINT64(a)->blueI;
      }

      static void PAINT_UINT64_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] -= CPUINT64(a)->redI;
          CPUINT64(a)->green[p] -= CPUINT64(a)->greenI;
          CPUINT64(a)->blue[p] -= CPUINT64(a)->blueI;
          CPUINT64(a)->alpha[p] -= CPUINT64(a)->alphaI;
      }

      static void PAINT_UINT64_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPUINT64(a)->red[p] -= CPUINT64(a)->redI;
          CPUINT64(a)->green[p] -= CPUINT64(a)->greenI;
      }


      static void PAINT_UINT64_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] * CRUINT64(a)->redI); }

      static void PAINT_UINT64_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] * CRUINT64(a)->redI);
          CRUINT64(a)->green[p] = (uint64) (CRUINT64(a)->green[p] * CRUINT64(a)->greenI);
          CRUINT64(a)->blue[p] = (uint64) (CRUINT64(a)->blue[p] * CRUINT64(a)->blueI);
      }

      static void PAINT_UINT64_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] * CRUINT64(a)->redI);
          CRUINT64(a)->green[p] = (uint64) (CRUINT64(a)->green[p] * CRUINT64(a)->greenI);
          CRUINT64(a)->blue[p] = (uint64) (CRUINT64(a)->blue[p] * CRUINT64(a)->blueI);
          CRUINT64(a)->alpha[p] = (uint64) (CRUINT64(a)->alpha[p] * CRUINT64(a)->alphaI);
      }

      static void PAINT_UINT64_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] * CRUINT64(a)->redI);
          CRUINT64(a)->green[p] = (uint64) (CRUINT64(a)->green[p] * CRUINT64(a)->greenI);
      }


      static void PAINT_UINT64_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] / CRUINT64(a)->redI); }

      static void PAINT_UINT64_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] / CRUINT64(a)->redI);
          CRUINT64(a)->green[p] = (uint64) (CRUINT64(a)->green[p] / CRUINT64(a)->greenI);
          CRUINT64(a)->blue[p] = (uint64) (CRUINT64(a)->blue[p] / CRUINT64(a)->blueI);
      }

      static void PAINT_UINT64_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] / CRUINT64(a)->redI);
          CRUINT64(a)->green[p] = (uint64) (CRUINT64(a)->green[p] / CRUINT64(a)->greenI);
          CRUINT64(a)->blue[p] = (uint64) (CRUINT64(a)->blue[p] / CRUINT64(a)->blueI);
          CRUINT64(a)->alpha[p] = (uint64) (CRUINT64(a)->alpha[p] / CRUINT64(a)->alphaI);
      }

      static void PAINT_UINT64_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRUINT64(a)->red[p] = (uint64) (CRUINT64(a)->red[p] / CRUINT64(a)->redI);
          CRUINT64(a)->green[p] = (uint64) (CRUINT64(a)->green[p] / CRUINT64(a)->greenI);
      }


      static void PAINT_UINT64_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { uint64 x = CPUINT64(a)->red[p];
        if (CPUINT64(a)->redI < x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
      }

      static void PAINT_UINT64_RGB_MIN_PIX(Indx_Type p, void *a) {
        uint64 x;
          x = CPUINT64(a)->red[p];
          if (CPUINT64(a)->redI < x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          x = CPUINT64(a)->green[p];
          if (CPUINT64(a)->greenI < x) CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
          x = CPUINT64(a)->blue[p];
          if (CPUINT64(a)->blueI < x) CPUINT64(a)->blue[p] = CPUINT64(a)->blueI;
      }

      static void PAINT_UINT64_RGBA_MIN_PIX(Indx_Type p, void *a) {
        uint64 x;
          x = CPUINT64(a)->red[p];
          if (CPUINT64(a)->redI < x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          x = CPUINT64(a)->green[p];
          if (CPUINT64(a)->greenI < x) CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
          x = CPUINT64(a)->blue[p];
          if (CPUINT64(a)->blueI < x) CPUINT64(a)->blue[p] = CPUINT64(a)->blueI;
          x = CPUINT64(a)->alpha[p];
          if (CPUINT64(a)->alphaI < x) CPUINT64(a)->alpha[p] = CPUINT64(a)->alphaI;
      }

      static void PAINT_UINT64_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        uint64 x;
        p <<= 1;
          x = CPUINT64(a)->red[p];
          if (CPUINT64(a)->redI < x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          x = CPUINT64(a)->green[p];
          if (CPUINT64(a)->greenI < x) CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
      }


      static void PAINT_UINT64_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { uint64 x = CPUINT64(a)->red[p];
        if (CPUINT64(a)->redI > x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
      }

      static void PAINT_UINT64_RGB_MAX_PIX(Indx_Type p, void *a) {
        uint64 x;
          x = CPUINT64(a)->red[p];
          if (CPUINT64(a)->redI > x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          x = CPUINT64(a)->green[p];
          if (CPUINT64(a)->greenI > x) CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
          x = CPUINT64(a)->blue[p];
          if (CPUINT64(a)->blueI > x) CPUINT64(a)->blue[p] = CPUINT64(a)->blueI;
      }

      static void PAINT_UINT64_RGBA_MAX_PIX(Indx_Type p, void *a) {
        uint64 x;
          x = CPUINT64(a)->red[p];
          if (CPUINT64(a)->redI > x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          x = CPUINT64(a)->green[p];
          if (CPUINT64(a)->greenI > x) CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
          x = CPUINT64(a)->blue[p];
          if (CPUINT64(a)->blueI > x) CPUINT64(a)->blue[p] = CPUINT64(a)->blueI;
          x = CPUINT64(a)->alpha[p];
          if (CPUINT64(a)->alphaI > x) CPUINT64(a)->alpha[p] = CPUINT64(a)->alphaI;
      }

      static void PAINT_UINT64_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        uint64 x;
        p <<= 1;
          x = CPUINT64(a)->red[p];
          if (CPUINT64(a)->redI > x) CPUINT64(a)->red[p] = CPUINT64(a)->redI;
          x = CPUINT64(a)->green[p];
          if (CPUINT64(a)->greenI > x) CPUINT64(a)->green[p] = CPUINT64(a)->greenI;
      }


      static void PAINT_INT8_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPINT8(a)->red[p] = CPINT8(a)->redI; }

      static void PAINT_INT8_RGB_SET_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] = CPINT8(a)->redI;
          CPINT8(a)->green[p] = CPINT8(a)->greenI;
          CPINT8(a)->blue[p] = CPINT8(a)->blueI;
      }

      static void PAINT_INT8_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] = CPINT8(a)->redI;
          CPINT8(a)->green[p] = CPINT8(a)->greenI;
          CPINT8(a)->blue[p] = CPINT8(a)->blueI;
          CPINT8(a)->alpha[p] = CPINT8(a)->alphaI;
      }

      static void PAINT_INT8_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] = CPINT8(a)->redI;
          CPINT8(a)->green[p] = CPINT8(a)->greenI;
      }


      static void PAINT_INT8_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPINT8(a)->red[p] += CPINT8(a)->redI; }

      static void PAINT_INT8_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] += CPINT8(a)->redI;
          CPINT8(a)->green[p] += CPINT8(a)->greenI;
          CPINT8(a)->blue[p] += CPINT8(a)->blueI;
      }

      static void PAINT_INT8_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] += CPINT8(a)->redI;
          CPINT8(a)->green[p] += CPINT8(a)->greenI;
          CPINT8(a)->blue[p] += CPINT8(a)->blueI;
          CPINT8(a)->alpha[p] += CPINT8(a)->alphaI;
      }

      static void PAINT_INT8_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] += CPINT8(a)->redI;
          CPINT8(a)->green[p] += CPINT8(a)->greenI;
      }


      static void PAINT_INT8_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPINT8(a)->red[p] -= CPINT8(a)->redI; }

      static void PAINT_INT8_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] -= CPINT8(a)->redI;
          CPINT8(a)->green[p] -= CPINT8(a)->greenI;
          CPINT8(a)->blue[p] -= CPINT8(a)->blueI;
      }

      static void PAINT_INT8_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] -= CPINT8(a)->redI;
          CPINT8(a)->green[p] -= CPINT8(a)->greenI;
          CPINT8(a)->blue[p] -= CPINT8(a)->blueI;
          CPINT8(a)->alpha[p] -= CPINT8(a)->alphaI;
      }

      static void PAINT_INT8_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPINT8(a)->red[p] -= CPINT8(a)->redI;
          CPINT8(a)->green[p] -= CPINT8(a)->greenI;
      }


      static void PAINT_INT8_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] * CRINT8(a)->redI); }

      static void PAINT_INT8_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] * CRINT8(a)->redI);
          CRINT8(a)->green[p] = (int8) (CRINT8(a)->green[p] * CRINT8(a)->greenI);
          CRINT8(a)->blue[p] = (int8) (CRINT8(a)->blue[p] * CRINT8(a)->blueI);
      }

      static void PAINT_INT8_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] * CRINT8(a)->redI);
          CRINT8(a)->green[p] = (int8) (CRINT8(a)->green[p] * CRINT8(a)->greenI);
          CRINT8(a)->blue[p] = (int8) (CRINT8(a)->blue[p] * CRINT8(a)->blueI);
          CRINT8(a)->alpha[p] = (int8) (CRINT8(a)->alpha[p] * CRINT8(a)->alphaI);
      }

      static void PAINT_INT8_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] * CRINT8(a)->redI);
          CRINT8(a)->green[p] = (int8) (CRINT8(a)->green[p] * CRINT8(a)->greenI);
      }


      static void PAINT_INT8_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] / CRINT8(a)->redI); }

      static void PAINT_INT8_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] / CRINT8(a)->redI);
          CRINT8(a)->green[p] = (int8) (CRINT8(a)->green[p] / CRINT8(a)->greenI);
          CRINT8(a)->blue[p] = (int8) (CRINT8(a)->blue[p] / CRINT8(a)->blueI);
      }

      static void PAINT_INT8_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] / CRINT8(a)->redI);
          CRINT8(a)->green[p] = (int8) (CRINT8(a)->green[p] / CRINT8(a)->greenI);
          CRINT8(a)->blue[p] = (int8) (CRINT8(a)->blue[p] / CRINT8(a)->blueI);
          CRINT8(a)->alpha[p] = (int8) (CRINT8(a)->alpha[p] / CRINT8(a)->alphaI);
      }

      static void PAINT_INT8_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRINT8(a)->red[p] = (int8) (CRINT8(a)->red[p] / CRINT8(a)->redI);
          CRINT8(a)->green[p] = (int8) (CRINT8(a)->green[p] / CRINT8(a)->greenI);
      }


      static void PAINT_INT8_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { int8 x = CPINT8(a)->red[p];
        if (CPINT8(a)->redI < x) CPINT8(a)->red[p] = CPINT8(a)->redI;
      }

      static void PAINT_INT8_RGB_MIN_PIX(Indx_Type p, void *a) {
        int8 x;
          x = CPINT8(a)->red[p];
          if (CPINT8(a)->redI < x) CPINT8(a)->red[p] = CPINT8(a)->redI;
          x = CPINT8(a)->green[p];
          if (CPINT8(a)->greenI < x) CPINT8(a)->green[p] = CPINT8(a)->greenI;
          x = CPINT8(a)->blue[p];
          if (CPINT8(a)->blueI < x) CPINT8(a)->blue[p] = CPINT8(a)->blueI;
      }

      static void PAINT_INT8_RGBA_MIN_PIX(Indx_Type p, void *a) {
        int8 x;
          x = CPINT8(a)->red[p];
          if (CPINT8(a)->redI < x) CPINT8(a)->red[p] = CPINT8(a)->redI;
          x = CPINT8(a)->green[p];
          if (CPINT8(a)->greenI < x) CPINT8(a)->green[p] = CPINT8(a)->greenI;
          x = CPINT8(a)->blue[p];
          if (CPINT8(a)->blueI < x) CPINT8(a)->blue[p] = CPINT8(a)->blueI;
          x = CPINT8(a)->alpha[p];
          if (CPINT8(a)->alphaI < x) CPINT8(a)->alpha[p] = CPINT8(a)->alphaI;
      }

      static void PAINT_INT8_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        int8 x;
        p <<= 1;
          x = CPINT8(a)->red[p];
          if (CPINT8(a)->redI < x) CPINT8(a)->red[p] = CPINT8(a)->redI;
          x = CPINT8(a)->green[p];
          if (CPINT8(a)->greenI < x) CPINT8(a)->green[p] = CPINT8(a)->greenI;
      }


      static void PAINT_INT8_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { int8 x = CPINT8(a)->red[p];
        if (CPINT8(a)->redI > x) CPINT8(a)->red[p] = CPINT8(a)->redI;
      }

      static void PAINT_INT8_RGB_MAX_PIX(Indx_Type p, void *a) {
        int8 x;
          x = CPINT8(a)->red[p];
          if (CPINT8(a)->redI > x) CPINT8(a)->red[p] = CPINT8(a)->redI;
          x = CPINT8(a)->green[p];
          if (CPINT8(a)->greenI > x) CPINT8(a)->green[p] = CPINT8(a)->greenI;
          x = CPINT8(a)->blue[p];
          if (CPINT8(a)->blueI > x) CPINT8(a)->blue[p] = CPINT8(a)->blueI;
      }

      static void PAINT_INT8_RGBA_MAX_PIX(Indx_Type p, void *a) {
        int8 x;
          x = CPINT8(a)->red[p];
          if (CPINT8(a)->redI > x) CPINT8(a)->red[p] = CPINT8(a)->redI;
          x = CPINT8(a)->green[p];
          if (CPINT8(a)->greenI > x) CPINT8(a)->green[p] = CPINT8(a)->greenI;
          x = CPINT8(a)->blue[p];
          if (CPINT8(a)->blueI > x) CPINT8(a)->blue[p] = CPINT8(a)->blueI;
          x = CPINT8(a)->alpha[p];
          if (CPINT8(a)->alphaI > x) CPINT8(a)->alpha[p] = CPINT8(a)->alphaI;
      }

      static void PAINT_INT8_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        int8 x;
        p <<= 1;
          x = CPINT8(a)->red[p];
          if (CPINT8(a)->redI > x) CPINT8(a)->red[p] = CPINT8(a)->redI;
          x = CPINT8(a)->green[p];
          if (CPINT8(a)->greenI > x) CPINT8(a)->green[p] = CPINT8(a)->greenI;
      }


      static void PAINT_INT16_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPINT16(a)->red[p] = CPINT16(a)->redI; }

      static void PAINT_INT16_RGB_SET_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] = CPINT16(a)->redI;
          CPINT16(a)->green[p] = CPINT16(a)->greenI;
          CPINT16(a)->blue[p] = CPINT16(a)->blueI;
      }

      static void PAINT_INT16_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] = CPINT16(a)->redI;
          CPINT16(a)->green[p] = CPINT16(a)->greenI;
          CPINT16(a)->blue[p] = CPINT16(a)->blueI;
          CPINT16(a)->alpha[p] = CPINT16(a)->alphaI;
      }

      static void PAINT_INT16_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] = CPINT16(a)->redI;
          CPINT16(a)->green[p] = CPINT16(a)->greenI;
      }


      static void PAINT_INT16_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPINT16(a)->red[p] += CPINT16(a)->redI; }

      static void PAINT_INT16_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] += CPINT16(a)->redI;
          CPINT16(a)->green[p] += CPINT16(a)->greenI;
          CPINT16(a)->blue[p] += CPINT16(a)->blueI;
      }

      static void PAINT_INT16_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] += CPINT16(a)->redI;
          CPINT16(a)->green[p] += CPINT16(a)->greenI;
          CPINT16(a)->blue[p] += CPINT16(a)->blueI;
          CPINT16(a)->alpha[p] += CPINT16(a)->alphaI;
      }

      static void PAINT_INT16_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] += CPINT16(a)->redI;
          CPINT16(a)->green[p] += CPINT16(a)->greenI;
      }


      static void PAINT_INT16_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPINT16(a)->red[p] -= CPINT16(a)->redI; }

      static void PAINT_INT16_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] -= CPINT16(a)->redI;
          CPINT16(a)->green[p] -= CPINT16(a)->greenI;
          CPINT16(a)->blue[p] -= CPINT16(a)->blueI;
      }

      static void PAINT_INT16_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] -= CPINT16(a)->redI;
          CPINT16(a)->green[p] -= CPINT16(a)->greenI;
          CPINT16(a)->blue[p] -= CPINT16(a)->blueI;
          CPINT16(a)->alpha[p] -= CPINT16(a)->alphaI;
      }

      static void PAINT_INT16_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPINT16(a)->red[p] -= CPINT16(a)->redI;
          CPINT16(a)->green[p] -= CPINT16(a)->greenI;
      }


      static void PAINT_INT16_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] * CRINT16(a)->redI); }

      static void PAINT_INT16_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] * CRINT16(a)->redI);
          CRINT16(a)->green[p] = (int16) (CRINT16(a)->green[p] * CRINT16(a)->greenI);
          CRINT16(a)->blue[p] = (int16) (CRINT16(a)->blue[p] * CRINT16(a)->blueI);
      }

      static void PAINT_INT16_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] * CRINT16(a)->redI);
          CRINT16(a)->green[p] = (int16) (CRINT16(a)->green[p] * CRINT16(a)->greenI);
          CRINT16(a)->blue[p] = (int16) (CRINT16(a)->blue[p] * CRINT16(a)->blueI);
          CRINT16(a)->alpha[p] = (int16) (CRINT16(a)->alpha[p] * CRINT16(a)->alphaI);
      }

      static void PAINT_INT16_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] * CRINT16(a)->redI);
          CRINT16(a)->green[p] = (int16) (CRINT16(a)->green[p] * CRINT16(a)->greenI);
      }


      static void PAINT_INT16_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] / CRINT16(a)->redI); }

      static void PAINT_INT16_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] / CRINT16(a)->redI);
          CRINT16(a)->green[p] = (int16) (CRINT16(a)->green[p] / CRINT16(a)->greenI);
          CRINT16(a)->blue[p] = (int16) (CRINT16(a)->blue[p] / CRINT16(a)->blueI);
      }

      static void PAINT_INT16_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] / CRINT16(a)->redI);
          CRINT16(a)->green[p] = (int16) (CRINT16(a)->green[p] / CRINT16(a)->greenI);
          CRINT16(a)->blue[p] = (int16) (CRINT16(a)->blue[p] / CRINT16(a)->blueI);
          CRINT16(a)->alpha[p] = (int16) (CRINT16(a)->alpha[p] / CRINT16(a)->alphaI);
      }

      static void PAINT_INT16_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRINT16(a)->red[p] = (int16) (CRINT16(a)->red[p] / CRINT16(a)->redI);
          CRINT16(a)->green[p] = (int16) (CRINT16(a)->green[p] / CRINT16(a)->greenI);
      }


      static void PAINT_INT16_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { int16 x = CPINT16(a)->red[p];
        if (CPINT16(a)->redI < x) CPINT16(a)->red[p] = CPINT16(a)->redI;
      }

      static void PAINT_INT16_RGB_MIN_PIX(Indx_Type p, void *a) {
        int16 x;
          x = CPINT16(a)->red[p];
          if (CPINT16(a)->redI < x) CPINT16(a)->red[p] = CPINT16(a)->redI;
          x = CPINT16(a)->green[p];
          if (CPINT16(a)->greenI < x) CPINT16(a)->green[p] = CPINT16(a)->greenI;
          x = CPINT16(a)->blue[p];
          if (CPINT16(a)->blueI < x) CPINT16(a)->blue[p] = CPINT16(a)->blueI;
      }

      static void PAINT_INT16_RGBA_MIN_PIX(Indx_Type p, void *a) {
        int16 x;
          x = CPINT16(a)->red[p];
          if (CPINT16(a)->redI < x) CPINT16(a)->red[p] = CPINT16(a)->redI;
          x = CPINT16(a)->green[p];
          if (CPINT16(a)->greenI < x) CPINT16(a)->green[p] = CPINT16(a)->greenI;
          x = CPINT16(a)->blue[p];
          if (CPINT16(a)->blueI < x) CPINT16(a)->blue[p] = CPINT16(a)->blueI;
          x = CPINT16(a)->alpha[p];
          if (CPINT16(a)->alphaI < x) CPINT16(a)->alpha[p] = CPINT16(a)->alphaI;
      }

      static void PAINT_INT16_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        int16 x;
        p <<= 1;
          x = CPINT16(a)->red[p];
          if (CPINT16(a)->redI < x) CPINT16(a)->red[p] = CPINT16(a)->redI;
          x = CPINT16(a)->green[p];
          if (CPINT16(a)->greenI < x) CPINT16(a)->green[p] = CPINT16(a)->greenI;
      }


      static void PAINT_INT16_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { int16 x = CPINT16(a)->red[p];
        if (CPINT16(a)->redI > x) CPINT16(a)->red[p] = CPINT16(a)->redI;
      }

      static void PAINT_INT16_RGB_MAX_PIX(Indx_Type p, void *a) {
        int16 x;
          x = CPINT16(a)->red[p];
          if (CPINT16(a)->redI > x) CPINT16(a)->red[p] = CPINT16(a)->redI;
          x = CPINT16(a)->green[p];
          if (CPINT16(a)->greenI > x) CPINT16(a)->green[p] = CPINT16(a)->greenI;
          x = CPINT16(a)->blue[p];
          if (CPINT16(a)->blueI > x) CPINT16(a)->blue[p] = CPINT16(a)->blueI;
      }

      static void PAINT_INT16_RGBA_MAX_PIX(Indx_Type p, void *a) {
        int16 x;
          x = CPINT16(a)->red[p];
          if (CPINT16(a)->redI > x) CPINT16(a)->red[p] = CPINT16(a)->redI;
          x = CPINT16(a)->green[p];
          if (CPINT16(a)->greenI > x) CPINT16(a)->green[p] = CPINT16(a)->greenI;
          x = CPINT16(a)->blue[p];
          if (CPINT16(a)->blueI > x) CPINT16(a)->blue[p] = CPINT16(a)->blueI;
          x = CPINT16(a)->alpha[p];
          if (CPINT16(a)->alphaI > x) CPINT16(a)->alpha[p] = CPINT16(a)->alphaI;
      }

      static void PAINT_INT16_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        int16 x;
        p <<= 1;
          x = CPINT16(a)->red[p];
          if (CPINT16(a)->redI > x) CPINT16(a)->red[p] = CPINT16(a)->redI;
          x = CPINT16(a)->green[p];
          if (CPINT16(a)->greenI > x) CPINT16(a)->green[p] = CPINT16(a)->greenI;
      }


      static void PAINT_INT32_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPINT32(a)->red[p] = CPINT32(a)->redI; }

      static void PAINT_INT32_RGB_SET_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] = CPINT32(a)->redI;
          CPINT32(a)->green[p] = CPINT32(a)->greenI;
          CPINT32(a)->blue[p] = CPINT32(a)->blueI;
      }

      static void PAINT_INT32_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] = CPINT32(a)->redI;
          CPINT32(a)->green[p] = CPINT32(a)->greenI;
          CPINT32(a)->blue[p] = CPINT32(a)->blueI;
          CPINT32(a)->alpha[p] = CPINT32(a)->alphaI;
      }

      static void PAINT_INT32_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] = CPINT32(a)->redI;
          CPINT32(a)->green[p] = CPINT32(a)->greenI;
      }


      static void PAINT_INT32_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPINT32(a)->red[p] += CPINT32(a)->redI; }

      static void PAINT_INT32_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] += CPINT32(a)->redI;
          CPINT32(a)->green[p] += CPINT32(a)->greenI;
          CPINT32(a)->blue[p] += CPINT32(a)->blueI;
      }

      static void PAINT_INT32_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] += CPINT32(a)->redI;
          CPINT32(a)->green[p] += CPINT32(a)->greenI;
          CPINT32(a)->blue[p] += CPINT32(a)->blueI;
          CPINT32(a)->alpha[p] += CPINT32(a)->alphaI;
      }

      static void PAINT_INT32_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] += CPINT32(a)->redI;
          CPINT32(a)->green[p] += CPINT32(a)->greenI;
      }


      static void PAINT_INT32_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPINT32(a)->red[p] -= CPINT32(a)->redI; }

      static void PAINT_INT32_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] -= CPINT32(a)->redI;
          CPINT32(a)->green[p] -= CPINT32(a)->greenI;
          CPINT32(a)->blue[p] -= CPINT32(a)->blueI;
      }

      static void PAINT_INT32_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] -= CPINT32(a)->redI;
          CPINT32(a)->green[p] -= CPINT32(a)->greenI;
          CPINT32(a)->blue[p] -= CPINT32(a)->blueI;
          CPINT32(a)->alpha[p] -= CPINT32(a)->alphaI;
      }

      static void PAINT_INT32_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPINT32(a)->red[p] -= CPINT32(a)->redI;
          CPINT32(a)->green[p] -= CPINT32(a)->greenI;
      }


      static void PAINT_INT32_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] * CRINT32(a)->redI); }

      static void PAINT_INT32_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] * CRINT32(a)->redI);
          CRINT32(a)->green[p] = (int32) (CRINT32(a)->green[p] * CRINT32(a)->greenI);
          CRINT32(a)->blue[p] = (int32) (CRINT32(a)->blue[p] * CRINT32(a)->blueI);
      }

      static void PAINT_INT32_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] * CRINT32(a)->redI);
          CRINT32(a)->green[p] = (int32) (CRINT32(a)->green[p] * CRINT32(a)->greenI);
          CRINT32(a)->blue[p] = (int32) (CRINT32(a)->blue[p] * CRINT32(a)->blueI);
          CRINT32(a)->alpha[p] = (int32) (CRINT32(a)->alpha[p] * CRINT32(a)->alphaI);
      }

      static void PAINT_INT32_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] * CRINT32(a)->redI);
          CRINT32(a)->green[p] = (int32) (CRINT32(a)->green[p] * CRINT32(a)->greenI);
      }


      static void PAINT_INT32_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] / CRINT32(a)->redI); }

      static void PAINT_INT32_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] / CRINT32(a)->redI);
          CRINT32(a)->green[p] = (int32) (CRINT32(a)->green[p] / CRINT32(a)->greenI);
          CRINT32(a)->blue[p] = (int32) (CRINT32(a)->blue[p] / CRINT32(a)->blueI);
      }

      static void PAINT_INT32_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] / CRINT32(a)->redI);
          CRINT32(a)->green[p] = (int32) (CRINT32(a)->green[p] / CRINT32(a)->greenI);
          CRINT32(a)->blue[p] = (int32) (CRINT32(a)->blue[p] / CRINT32(a)->blueI);
          CRINT32(a)->alpha[p] = (int32) (CRINT32(a)->alpha[p] / CRINT32(a)->alphaI);
      }

      static void PAINT_INT32_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRINT32(a)->red[p] = (int32) (CRINT32(a)->red[p] / CRINT32(a)->redI);
          CRINT32(a)->green[p] = (int32) (CRINT32(a)->green[p] / CRINT32(a)->greenI);
      }


      static void PAINT_INT32_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { int32 x = CPINT32(a)->red[p];
        if (CPINT32(a)->redI < x) CPINT32(a)->red[p] = CPINT32(a)->redI;
      }

      static void PAINT_INT32_RGB_MIN_PIX(Indx_Type p, void *a) {
        int32 x;
          x = CPINT32(a)->red[p];
          if (CPINT32(a)->redI < x) CPINT32(a)->red[p] = CPINT32(a)->redI;
          x = CPINT32(a)->green[p];
          if (CPINT32(a)->greenI < x) CPINT32(a)->green[p] = CPINT32(a)->greenI;
          x = CPINT32(a)->blue[p];
          if (CPINT32(a)->blueI < x) CPINT32(a)->blue[p] = CPINT32(a)->blueI;
      }

      static void PAINT_INT32_RGBA_MIN_PIX(Indx_Type p, void *a) {
        int32 x;
          x = CPINT32(a)->red[p];
          if (CPINT32(a)->redI < x) CPINT32(a)->red[p] = CPINT32(a)->redI;
          x = CPINT32(a)->green[p];
          if (CPINT32(a)->greenI < x) CPINT32(a)->green[p] = CPINT32(a)->greenI;
          x = CPINT32(a)->blue[p];
          if (CPINT32(a)->blueI < x) CPINT32(a)->blue[p] = CPINT32(a)->blueI;
          x = CPINT32(a)->alpha[p];
          if (CPINT32(a)->alphaI < x) CPINT32(a)->alpha[p] = CPINT32(a)->alphaI;
      }

      static void PAINT_INT32_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        int32 x;
        p <<= 1;
          x = CPINT32(a)->red[p];
          if (CPINT32(a)->redI < x) CPINT32(a)->red[p] = CPINT32(a)->redI;
          x = CPINT32(a)->green[p];
          if (CPINT32(a)->greenI < x) CPINT32(a)->green[p] = CPINT32(a)->greenI;
      }


      static void PAINT_INT32_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { int32 x = CPINT32(a)->red[p];
        if (CPINT32(a)->redI > x) CPINT32(a)->red[p] = CPINT32(a)->redI;
      }

      static void PAINT_INT32_RGB_MAX_PIX(Indx_Type p, void *a) {
        int32 x;
          x = CPINT32(a)->red[p];
          if (CPINT32(a)->redI > x) CPINT32(a)->red[p] = CPINT32(a)->redI;
          x = CPINT32(a)->green[p];
          if (CPINT32(a)->greenI > x) CPINT32(a)->green[p] = CPINT32(a)->greenI;
          x = CPINT32(a)->blue[p];
          if (CPINT32(a)->blueI > x) CPINT32(a)->blue[p] = CPINT32(a)->blueI;
      }

      static void PAINT_INT32_RGBA_MAX_PIX(Indx_Type p, void *a) {
        int32 x;
          x = CPINT32(a)->red[p];
          if (CPINT32(a)->redI > x) CPINT32(a)->red[p] = CPINT32(a)->redI;
          x = CPINT32(a)->green[p];
          if (CPINT32(a)->greenI > x) CPINT32(a)->green[p] = CPINT32(a)->greenI;
          x = CPINT32(a)->blue[p];
          if (CPINT32(a)->blueI > x) CPINT32(a)->blue[p] = CPINT32(a)->blueI;
          x = CPINT32(a)->alpha[p];
          if (CPINT32(a)->alphaI > x) CPINT32(a)->alpha[p] = CPINT32(a)->alphaI;
      }

      static void PAINT_INT32_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        int32 x;
        p <<= 1;
          x = CPINT32(a)->red[p];
          if (CPINT32(a)->redI > x) CPINT32(a)->red[p] = CPINT32(a)->redI;
          x = CPINT32(a)->green[p];
          if (CPINT32(a)->greenI > x) CPINT32(a)->green[p] = CPINT32(a)->greenI;
      }


      static void PAINT_INT64_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPINT64(a)->red[p] = CPINT64(a)->redI; }

      static void PAINT_INT64_RGB_SET_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] = CPINT64(a)->redI;
          CPINT64(a)->green[p] = CPINT64(a)->greenI;
          CPINT64(a)->blue[p] = CPINT64(a)->blueI;
      }

      static void PAINT_INT64_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] = CPINT64(a)->redI;
          CPINT64(a)->green[p] = CPINT64(a)->greenI;
          CPINT64(a)->blue[p] = CPINT64(a)->blueI;
          CPINT64(a)->alpha[p] = CPINT64(a)->alphaI;
      }

      static void PAINT_INT64_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] = CPINT64(a)->redI;
          CPINT64(a)->green[p] = CPINT64(a)->greenI;
      }


      static void PAINT_INT64_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPINT64(a)->red[p] += CPINT64(a)->redI; }

      static void PAINT_INT64_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] += CPINT64(a)->redI;
          CPINT64(a)->green[p] += CPINT64(a)->greenI;
          CPINT64(a)->blue[p] += CPINT64(a)->blueI;
      }

      static void PAINT_INT64_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] += CPINT64(a)->redI;
          CPINT64(a)->green[p] += CPINT64(a)->greenI;
          CPINT64(a)->blue[p] += CPINT64(a)->blueI;
          CPINT64(a)->alpha[p] += CPINT64(a)->alphaI;
      }

      static void PAINT_INT64_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] += CPINT64(a)->redI;
          CPINT64(a)->green[p] += CPINT64(a)->greenI;
      }


      static void PAINT_INT64_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPINT64(a)->red[p] -= CPINT64(a)->redI; }

      static void PAINT_INT64_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] -= CPINT64(a)->redI;
          CPINT64(a)->green[p] -= CPINT64(a)->greenI;
          CPINT64(a)->blue[p] -= CPINT64(a)->blueI;
      }

      static void PAINT_INT64_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] -= CPINT64(a)->redI;
          CPINT64(a)->green[p] -= CPINT64(a)->greenI;
          CPINT64(a)->blue[p] -= CPINT64(a)->blueI;
          CPINT64(a)->alpha[p] -= CPINT64(a)->alphaI;
      }

      static void PAINT_INT64_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPINT64(a)->red[p] -= CPINT64(a)->redI;
          CPINT64(a)->green[p] -= CPINT64(a)->greenI;
      }


      static void PAINT_INT64_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] * CRINT64(a)->redI); }

      static void PAINT_INT64_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] * CRINT64(a)->redI);
          CRINT64(a)->green[p] = (int64) (CRINT64(a)->green[p] * CRINT64(a)->greenI);
          CRINT64(a)->blue[p] = (int64) (CRINT64(a)->blue[p] * CRINT64(a)->blueI);
      }

      static void PAINT_INT64_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] * CRINT64(a)->redI);
          CRINT64(a)->green[p] = (int64) (CRINT64(a)->green[p] * CRINT64(a)->greenI);
          CRINT64(a)->blue[p] = (int64) (CRINT64(a)->blue[p] * CRINT64(a)->blueI);
          CRINT64(a)->alpha[p] = (int64) (CRINT64(a)->alpha[p] * CRINT64(a)->alphaI);
      }

      static void PAINT_INT64_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] * CRINT64(a)->redI);
          CRINT64(a)->green[p] = (int64) (CRINT64(a)->green[p] * CRINT64(a)->greenI);
      }


      static void PAINT_INT64_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] / CRINT64(a)->redI); }

      static void PAINT_INT64_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] / CRINT64(a)->redI);
          CRINT64(a)->green[p] = (int64) (CRINT64(a)->green[p] / CRINT64(a)->greenI);
          CRINT64(a)->blue[p] = (int64) (CRINT64(a)->blue[p] / CRINT64(a)->blueI);
      }

      static void PAINT_INT64_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] / CRINT64(a)->redI);
          CRINT64(a)->green[p] = (int64) (CRINT64(a)->green[p] / CRINT64(a)->greenI);
          CRINT64(a)->blue[p] = (int64) (CRINT64(a)->blue[p] / CRINT64(a)->blueI);
          CRINT64(a)->alpha[p] = (int64) (CRINT64(a)->alpha[p] / CRINT64(a)->alphaI);
      }

      static void PAINT_INT64_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRINT64(a)->red[p] = (int64) (CRINT64(a)->red[p] / CRINT64(a)->redI);
          CRINT64(a)->green[p] = (int64) (CRINT64(a)->green[p] / CRINT64(a)->greenI);
      }


      static void PAINT_INT64_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { int64 x = CPINT64(a)->red[p];
        if (CPINT64(a)->redI < x) CPINT64(a)->red[p] = CPINT64(a)->redI;
      }

      static void PAINT_INT64_RGB_MIN_PIX(Indx_Type p, void *a) {
        int64 x;
          x = CPINT64(a)->red[p];
          if (CPINT64(a)->redI < x) CPINT64(a)->red[p] = CPINT64(a)->redI;
          x = CPINT64(a)->green[p];
          if (CPINT64(a)->greenI < x) CPINT64(a)->green[p] = CPINT64(a)->greenI;
          x = CPINT64(a)->blue[p];
          if (CPINT64(a)->blueI < x) CPINT64(a)->blue[p] = CPINT64(a)->blueI;
      }

      static void PAINT_INT64_RGBA_MIN_PIX(Indx_Type p, void *a) {
        int64 x;
          x = CPINT64(a)->red[p];
          if (CPINT64(a)->redI < x) CPINT64(a)->red[p] = CPINT64(a)->redI;
          x = CPINT64(a)->green[p];
          if (CPINT64(a)->greenI < x) CPINT64(a)->green[p] = CPINT64(a)->greenI;
          x = CPINT64(a)->blue[p];
          if (CPINT64(a)->blueI < x) CPINT64(a)->blue[p] = CPINT64(a)->blueI;
          x = CPINT64(a)->alpha[p];
          if (CPINT64(a)->alphaI < x) CPINT64(a)->alpha[p] = CPINT64(a)->alphaI;
      }

      static void PAINT_INT64_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        int64 x;
        p <<= 1;
          x = CPINT64(a)->red[p];
          if (CPINT64(a)->redI < x) CPINT64(a)->red[p] = CPINT64(a)->redI;
          x = CPINT64(a)->green[p];
          if (CPINT64(a)->greenI < x) CPINT64(a)->green[p] = CPINT64(a)->greenI;
      }


      static void PAINT_INT64_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { int64 x = CPINT64(a)->red[p];
        if (CPINT64(a)->redI > x) CPINT64(a)->red[p] = CPINT64(a)->redI;
      }

      static void PAINT_INT64_RGB_MAX_PIX(Indx_Type p, void *a) {
        int64 x;
          x = CPINT64(a)->red[p];
          if (CPINT64(a)->redI > x) CPINT64(a)->red[p] = CPINT64(a)->redI;
          x = CPINT64(a)->green[p];
          if (CPINT64(a)->greenI > x) CPINT64(a)->green[p] = CPINT64(a)->greenI;
          x = CPINT64(a)->blue[p];
          if (CPINT64(a)->blueI > x) CPINT64(a)->blue[p] = CPINT64(a)->blueI;
      }

      static void PAINT_INT64_RGBA_MAX_PIX(Indx_Type p, void *a) {
        int64 x;
          x = CPINT64(a)->red[p];
          if (CPINT64(a)->redI > x) CPINT64(a)->red[p] = CPINT64(a)->redI;
          x = CPINT64(a)->green[p];
          if (CPINT64(a)->greenI > x) CPINT64(a)->green[p] = CPINT64(a)->greenI;
          x = CPINT64(a)->blue[p];
          if (CPINT64(a)->blueI > x) CPINT64(a)->blue[p] = CPINT64(a)->blueI;
          x = CPINT64(a)->alpha[p];
          if (CPINT64(a)->alphaI > x) CPINT64(a)->alpha[p] = CPINT64(a)->alphaI;
      }

      static void PAINT_INT64_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        int64 x;
        p <<= 1;
          x = CPINT64(a)->red[p];
          if (CPINT64(a)->redI > x) CPINT64(a)->red[p] = CPINT64(a)->redI;
          x = CPINT64(a)->green[p];
          if (CPINT64(a)->greenI > x) CPINT64(a)->green[p] = CPINT64(a)->greenI;
      }


      static void PAINT_FLOAT32_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI; }

      static void PAINT_FLOAT32_RGB_SET_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
          CPFLOAT32(a)->blue[p] = CPFLOAT32(a)->blueI;
      }

      static void PAINT_FLOAT32_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
          CPFLOAT32(a)->blue[p] = CPFLOAT32(a)->blueI;
          CPFLOAT32(a)->alpha[p] = CPFLOAT32(a)->alphaI;
      }

      static void PAINT_FLOAT32_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
      }


      static void PAINT_FLOAT32_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPFLOAT32(a)->red[p] += CPFLOAT32(a)->redI; }

      static void PAINT_FLOAT32_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] += CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] += CPFLOAT32(a)->greenI;
          CPFLOAT32(a)->blue[p] += CPFLOAT32(a)->blueI;
      }

      static void PAINT_FLOAT32_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] += CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] += CPFLOAT32(a)->greenI;
          CPFLOAT32(a)->blue[p] += CPFLOAT32(a)->blueI;
          CPFLOAT32(a)->alpha[p] += CPFLOAT32(a)->alphaI;
      }

      static void PAINT_FLOAT32_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] += CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] += CPFLOAT32(a)->greenI;
      }


      static void PAINT_FLOAT32_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPFLOAT32(a)->red[p] -= CPFLOAT32(a)->redI; }

      static void PAINT_FLOAT32_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] -= CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] -= CPFLOAT32(a)->greenI;
          CPFLOAT32(a)->blue[p] -= CPFLOAT32(a)->blueI;
      }

      static void PAINT_FLOAT32_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] -= CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] -= CPFLOAT32(a)->greenI;
          CPFLOAT32(a)->blue[p] -= CPFLOAT32(a)->blueI;
          CPFLOAT32(a)->alpha[p] -= CPFLOAT32(a)->alphaI;
      }

      static void PAINT_FLOAT32_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPFLOAT32(a)->red[p] -= CPFLOAT32(a)->redI;
          CPFLOAT32(a)->green[p] -= CPFLOAT32(a)->greenI;
      }


      static void PAINT_FLOAT32_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] * CRFLOAT32(a)->redI); }

      static void PAINT_FLOAT32_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] * CRFLOAT32(a)->redI);
          CRFLOAT32(a)->green[p] = (float32) (CRFLOAT32(a)->green[p] * CRFLOAT32(a)->greenI);
          CRFLOAT32(a)->blue[p] = (float32) (CRFLOAT32(a)->blue[p] * CRFLOAT32(a)->blueI);
      }

      static void PAINT_FLOAT32_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] * CRFLOAT32(a)->redI);
          CRFLOAT32(a)->green[p] = (float32) (CRFLOAT32(a)->green[p] * CRFLOAT32(a)->greenI);
          CRFLOAT32(a)->blue[p] = (float32) (CRFLOAT32(a)->blue[p] * CRFLOAT32(a)->blueI);
          CRFLOAT32(a)->alpha[p] = (float32) (CRFLOAT32(a)->alpha[p] * CRFLOAT32(a)->alphaI);
      }

      static void PAINT_FLOAT32_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] * CRFLOAT32(a)->redI);
          CRFLOAT32(a)->green[p] = (float32) (CRFLOAT32(a)->green[p] * CRFLOAT32(a)->greenI);
      }


      static void PAINT_FLOAT32_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] / CRFLOAT32(a)->redI); }

      static void PAINT_FLOAT32_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] / CRFLOAT32(a)->redI);
          CRFLOAT32(a)->green[p] = (float32) (CRFLOAT32(a)->green[p] / CRFLOAT32(a)->greenI);
          CRFLOAT32(a)->blue[p] = (float32) (CRFLOAT32(a)->blue[p] / CRFLOAT32(a)->blueI);
      }

      static void PAINT_FLOAT32_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] / CRFLOAT32(a)->redI);
          CRFLOAT32(a)->green[p] = (float32) (CRFLOAT32(a)->green[p] / CRFLOAT32(a)->greenI);
          CRFLOAT32(a)->blue[p] = (float32) (CRFLOAT32(a)->blue[p] / CRFLOAT32(a)->blueI);
          CRFLOAT32(a)->alpha[p] = (float32) (CRFLOAT32(a)->alpha[p] / CRFLOAT32(a)->alphaI);
      }

      static void PAINT_FLOAT32_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRFLOAT32(a)->red[p] = (float32) (CRFLOAT32(a)->red[p] / CRFLOAT32(a)->redI);
          CRFLOAT32(a)->green[p] = (float32) (CRFLOAT32(a)->green[p] / CRFLOAT32(a)->greenI);
      }


      static void PAINT_FLOAT32_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { float32 x = CPFLOAT32(a)->red[p];
        if (CPFLOAT32(a)->redI < x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
      }

      static void PAINT_FLOAT32_RGB_MIN_PIX(Indx_Type p, void *a) {
        float32 x;
          x = CPFLOAT32(a)->red[p];
          if (CPFLOAT32(a)->redI < x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          x = CPFLOAT32(a)->green[p];
          if (CPFLOAT32(a)->greenI < x) CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
          x = CPFLOAT32(a)->blue[p];
          if (CPFLOAT32(a)->blueI < x) CPFLOAT32(a)->blue[p] = CPFLOAT32(a)->blueI;
      }

      static void PAINT_FLOAT32_RGBA_MIN_PIX(Indx_Type p, void *a) {
        float32 x;
          x = CPFLOAT32(a)->red[p];
          if (CPFLOAT32(a)->redI < x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          x = CPFLOAT32(a)->green[p];
          if (CPFLOAT32(a)->greenI < x) CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
          x = CPFLOAT32(a)->blue[p];
          if (CPFLOAT32(a)->blueI < x) CPFLOAT32(a)->blue[p] = CPFLOAT32(a)->blueI;
          x = CPFLOAT32(a)->alpha[p];
          if (CPFLOAT32(a)->alphaI < x) CPFLOAT32(a)->alpha[p] = CPFLOAT32(a)->alphaI;
      }

      static void PAINT_FLOAT32_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        float32 x;
        p <<= 1;
          x = CPFLOAT32(a)->red[p];
          if (CPFLOAT32(a)->redI < x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          x = CPFLOAT32(a)->green[p];
          if (CPFLOAT32(a)->greenI < x) CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
      }


      static void PAINT_FLOAT32_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { float32 x = CPFLOAT32(a)->red[p];
        if (CPFLOAT32(a)->redI > x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
      }

      static void PAINT_FLOAT32_RGB_MAX_PIX(Indx_Type p, void *a) {
        float32 x;
          x = CPFLOAT32(a)->red[p];
          if (CPFLOAT32(a)->redI > x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          x = CPFLOAT32(a)->green[p];
          if (CPFLOAT32(a)->greenI > x) CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
          x = CPFLOAT32(a)->blue[p];
          if (CPFLOAT32(a)->blueI > x) CPFLOAT32(a)->blue[p] = CPFLOAT32(a)->blueI;
      }

      static void PAINT_FLOAT32_RGBA_MAX_PIX(Indx_Type p, void *a) {
        float32 x;
          x = CPFLOAT32(a)->red[p];
          if (CPFLOAT32(a)->redI > x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          x = CPFLOAT32(a)->green[p];
          if (CPFLOAT32(a)->greenI > x) CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
          x = CPFLOAT32(a)->blue[p];
          if (CPFLOAT32(a)->blueI > x) CPFLOAT32(a)->blue[p] = CPFLOAT32(a)->blueI;
          x = CPFLOAT32(a)->alpha[p];
          if (CPFLOAT32(a)->alphaI > x) CPFLOAT32(a)->alpha[p] = CPFLOAT32(a)->alphaI;
      }

      static void PAINT_FLOAT32_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        float32 x;
        p <<= 1;
          x = CPFLOAT32(a)->red[p];
          if (CPFLOAT32(a)->redI > x) CPFLOAT32(a)->red[p] = CPFLOAT32(a)->redI;
          x = CPFLOAT32(a)->green[p];
          if (CPFLOAT32(a)->greenI > x) CPFLOAT32(a)->green[p] = CPFLOAT32(a)->greenI;
      }


      static void PAINT_FLOAT64_PLAIN_SET_PIX(Indx_Type p, void *a)
      { CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI; }

      static void PAINT_FLOAT64_RGB_SET_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
          CPFLOAT64(a)->blue[p] = CPFLOAT64(a)->blueI;
      }

      static void PAINT_FLOAT64_RGBA_SET_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
          CPFLOAT64(a)->blue[p] = CPFLOAT64(a)->blueI;
          CPFLOAT64(a)->alpha[p] = CPFLOAT64(a)->alphaI;
      }

      static void PAINT_FLOAT64_COMPLEX_SET_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
      }


      static void PAINT_FLOAT64_PLAIN_ADD_PIX(Indx_Type p, void *a)
      { CPFLOAT64(a)->red[p] += CPFLOAT64(a)->redI; }

      static void PAINT_FLOAT64_RGB_ADD_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] += CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] += CPFLOAT64(a)->greenI;
          CPFLOAT64(a)->blue[p] += CPFLOAT64(a)->blueI;
      }

      static void PAINT_FLOAT64_RGBA_ADD_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] += CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] += CPFLOAT64(a)->greenI;
          CPFLOAT64(a)->blue[p] += CPFLOAT64(a)->blueI;
          CPFLOAT64(a)->alpha[p] += CPFLOAT64(a)->alphaI;
      }

      static void PAINT_FLOAT64_COMPLEX_ADD_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] += CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] += CPFLOAT64(a)->greenI;
      }


      static void PAINT_FLOAT64_PLAIN_SUB_PIX(Indx_Type p, void *a)
      { CPFLOAT64(a)->red[p] -= CPFLOAT64(a)->redI; }

      static void PAINT_FLOAT64_RGB_SUB_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] -= CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] -= CPFLOAT64(a)->greenI;
          CPFLOAT64(a)->blue[p] -= CPFLOAT64(a)->blueI;
      }

      static void PAINT_FLOAT64_RGBA_SUB_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] -= CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] -= CPFLOAT64(a)->greenI;
          CPFLOAT64(a)->blue[p] -= CPFLOAT64(a)->blueI;
          CPFLOAT64(a)->alpha[p] -= CPFLOAT64(a)->alphaI;
      }

      static void PAINT_FLOAT64_COMPLEX_SUB_PIX(Indx_Type p, void *a) {
          CPFLOAT64(a)->red[p] -= CPFLOAT64(a)->redI;
          CPFLOAT64(a)->green[p] -= CPFLOAT64(a)->greenI;
      }


      static void PAINT_FLOAT64_PLAIN_MUL_PIX(Indx_Type p, void *a)
      { CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] * CRFLOAT64(a)->redI); }

      static void PAINT_FLOAT64_RGB_MUL_PIX(Indx_Type p, void *a) {
          CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] * CRFLOAT64(a)->redI);
          CRFLOAT64(a)->green[p] = (float64) (CRFLOAT64(a)->green[p] * CRFLOAT64(a)->greenI);
          CRFLOAT64(a)->blue[p] = (float64) (CRFLOAT64(a)->blue[p] * CRFLOAT64(a)->blueI);
      }

      static void PAINT_FLOAT64_RGBA_MUL_PIX(Indx_Type p, void *a) {
          CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] * CRFLOAT64(a)->redI);
          CRFLOAT64(a)->green[p] = (float64) (CRFLOAT64(a)->green[p] * CRFLOAT64(a)->greenI);
          CRFLOAT64(a)->blue[p] = (float64) (CRFLOAT64(a)->blue[p] * CRFLOAT64(a)->blueI);
          CRFLOAT64(a)->alpha[p] = (float64) (CRFLOAT64(a)->alpha[p] * CRFLOAT64(a)->alphaI);
      }

      static void PAINT_FLOAT64_COMPLEX_MUL_PIX(Indx_Type p, void *a) {
          CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] * CRFLOAT64(a)->redI);
          CRFLOAT64(a)->green[p] = (float64) (CRFLOAT64(a)->green[p] * CRFLOAT64(a)->greenI);
      }


      static void PAINT_FLOAT64_PLAIN_DIV_PIX(Indx_Type p, void *a)
      { CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] / CRFLOAT64(a)->redI); }

      static void PAINT_FLOAT64_RGB_DIV_PIX(Indx_Type p, void *a) {
          CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] / CRFLOAT64(a)->redI);
          CRFLOAT64(a)->green[p] = (float64) (CRFLOAT64(a)->green[p] / CRFLOAT64(a)->greenI);
          CRFLOAT64(a)->blue[p] = (float64) (CRFLOAT64(a)->blue[p] / CRFLOAT64(a)->blueI);
      }

      static void PAINT_FLOAT64_RGBA_DIV_PIX(Indx_Type p, void *a) {
          CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] / CRFLOAT64(a)->redI);
          CRFLOAT64(a)->green[p] = (float64) (CRFLOAT64(a)->green[p] / CRFLOAT64(a)->greenI);
          CRFLOAT64(a)->blue[p] = (float64) (CRFLOAT64(a)->blue[p] / CRFLOAT64(a)->blueI);
          CRFLOAT64(a)->alpha[p] = (float64) (CRFLOAT64(a)->alpha[p] / CRFLOAT64(a)->alphaI);
      }

      static void PAINT_FLOAT64_COMPLEX_DIV_PIX(Indx_Type p, void *a) {
          CRFLOAT64(a)->red[p] = (float64) (CRFLOAT64(a)->red[p] / CRFLOAT64(a)->redI);
          CRFLOAT64(a)->green[p] = (float64) (CRFLOAT64(a)->green[p] / CRFLOAT64(a)->greenI);
      }


      static void PAINT_FLOAT64_PLAIN_MIN_PIX(Indx_Type p, void *a)
      { float64 x = CPFLOAT64(a)->red[p];
        if (CPFLOAT64(a)->redI < x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
      }

      static void PAINT_FLOAT64_RGB_MIN_PIX(Indx_Type p, void *a) {
        float64 x;
          x = CPFLOAT64(a)->red[p];
          if (CPFLOAT64(a)->redI < x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          x = CPFLOAT64(a)->green[p];
          if (CPFLOAT64(a)->greenI < x) CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
          x = CPFLOAT64(a)->blue[p];
          if (CPFLOAT64(a)->blueI < x) CPFLOAT64(a)->blue[p] = CPFLOAT64(a)->blueI;
      }

      static void PAINT_FLOAT64_RGBA_MIN_PIX(Indx_Type p, void *a) {
        float64 x;
          x = CPFLOAT64(a)->red[p];
          if (CPFLOAT64(a)->redI < x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          x = CPFLOAT64(a)->green[p];
          if (CPFLOAT64(a)->greenI < x) CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
          x = CPFLOAT64(a)->blue[p];
          if (CPFLOAT64(a)->blueI < x) CPFLOAT64(a)->blue[p] = CPFLOAT64(a)->blueI;
          x = CPFLOAT64(a)->alpha[p];
          if (CPFLOAT64(a)->alphaI < x) CPFLOAT64(a)->alpha[p] = CPFLOAT64(a)->alphaI;
      }

      static void PAINT_FLOAT64_COMPLEX_MIN_PIX(Indx_Type p, void *a) {
        float64 x;
        p <<= 1;
          x = CPFLOAT64(a)->red[p];
          if (CPFLOAT64(a)->redI < x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          x = CPFLOAT64(a)->green[p];
          if (CPFLOAT64(a)->greenI < x) CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
      }


      static void PAINT_FLOAT64_PLAIN_MAX_PIX(Indx_Type p, void *a)
      { float64 x = CPFLOAT64(a)->red[p];
        if (CPFLOAT64(a)->redI > x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
      }

      static void PAINT_FLOAT64_RGB_MAX_PIX(Indx_Type p, void *a) {
        float64 x;
          x = CPFLOAT64(a)->red[p];
          if (CPFLOAT64(a)->redI > x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          x = CPFLOAT64(a)->green[p];
          if (CPFLOAT64(a)->greenI > x) CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
          x = CPFLOAT64(a)->blue[p];
          if (CPFLOAT64(a)->blueI > x) CPFLOAT64(a)->blue[p] = CPFLOAT64(a)->blueI;
      }

      static void PAINT_FLOAT64_RGBA_MAX_PIX(Indx_Type p, void *a) {
        float64 x;
          x = CPFLOAT64(a)->red[p];
          if (CPFLOAT64(a)->redI > x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          x = CPFLOAT64(a)->green[p];
          if (CPFLOAT64(a)->greenI > x) CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
          x = CPFLOAT64(a)->blue[p];
          if (CPFLOAT64(a)->blueI > x) CPFLOAT64(a)->blue[p] = CPFLOAT64(a)->blueI;
          x = CPFLOAT64(a)->alpha[p];
          if (CPFLOAT64(a)->alphaI > x) CPFLOAT64(a)->alpha[p] = CPFLOAT64(a)->alphaI;
      }

      static void PAINT_FLOAT64_COMPLEX_MAX_PIX(Indx_Type p, void *a) {
        float64 x;
        p <<= 1;
          x = CPFLOAT64(a)->red[p];
          if (CPFLOAT64(a)->redI > x) CPFLOAT64(a)->red[p] = CPFLOAT64(a)->redI;
          x = CPFLOAT64(a)->green[p];
          if (CPFLOAT64(a)->greenI > x) CPFLOAT64(a)->green[p] = CPFLOAT64(a)->greenI;
      }


static void (*Painter_Table[])(Indx_Type,void *) = {   //  Make a table of all these puppies
          PAINT_UINT8_PLAIN_SET_PIX,
          PAINT_UINT8_PLAIN_ADD_PIX,
          PAINT_UINT8_PLAIN_SUB_PIX,
          PAINT_UINT8_PLAIN_MUL_PIX,
          PAINT_UINT8_PLAIN_DIV_PIX,
          PAINT_UINT8_PLAIN_MIN_PIX,
          PAINT_UINT8_PLAIN_MAX_PIX,
          PAINT_UINT16_PLAIN_SET_PIX,
          PAINT_UINT16_PLAIN_ADD_PIX,
          PAINT_UINT16_PLAIN_SUB_PIX,
          PAINT_UINT16_PLAIN_MUL_PIX,
          PAINT_UINT16_PLAIN_DIV_PIX,
          PAINT_UINT16_PLAIN_MIN_PIX,
          PAINT_UINT16_PLAIN_MAX_PIX,
          PAINT_UINT32_PLAIN_SET_PIX,
          PAINT_UINT32_PLAIN_ADD_PIX,
          PAINT_UINT32_PLAIN_SUB_PIX,
          PAINT_UINT32_PLAIN_MUL_PIX,
          PAINT_UINT32_PLAIN_DIV_PIX,
          PAINT_UINT32_PLAIN_MIN_PIX,
          PAINT_UINT32_PLAIN_MAX_PIX,
          PAINT_UINT64_PLAIN_SET_PIX,
          PAINT_UINT64_PLAIN_ADD_PIX,
          PAINT_UINT64_PLAIN_SUB_PIX,
          PAINT_UINT64_PLAIN_MUL_PIX,
          PAINT_UINT64_PLAIN_DIV_PIX,
          PAINT_UINT64_PLAIN_MIN_PIX,
          PAINT_UINT64_PLAIN_MAX_PIX,
          PAINT_INT8_PLAIN_SET_PIX,
          PAINT_INT8_PLAIN_ADD_PIX,
          PAINT_INT8_PLAIN_SUB_PIX,
          PAINT_INT8_PLAIN_MUL_PIX,
          PAINT_INT8_PLAIN_DIV_PIX,
          PAINT_INT8_PLAIN_MIN_PIX,
          PAINT_INT8_PLAIN_MAX_PIX,
          PAINT_INT16_PLAIN_SET_PIX,
          PAINT_INT16_PLAIN_ADD_PIX,
          PAINT_INT16_PLAIN_SUB_PIX,
          PAINT_INT16_PLAIN_MUL_PIX,
          PAINT_INT16_PLAIN_DIV_PIX,
          PAINT_INT16_PLAIN_MIN_PIX,
          PAINT_INT16_PLAIN_MAX_PIX,
          PAINT_INT32_PLAIN_SET_PIX,
          PAINT_INT32_PLAIN_ADD_PIX,
          PAINT_INT32_PLAIN_SUB_PIX,
          PAINT_INT32_PLAIN_MUL_PIX,
          PAINT_INT32_PLAIN_DIV_PIX,
          PAINT_INT32_PLAIN_MIN_PIX,
          PAINT_INT32_PLAIN_MAX_PIX,
          PAINT_INT64_PLAIN_SET_PIX,
          PAINT_INT64_PLAIN_ADD_PIX,
          PAINT_INT64_PLAIN_SUB_PIX,
          PAINT_INT64_PLAIN_MUL_PIX,
          PAINT_INT64_PLAIN_DIV_PIX,
          PAINT_INT64_PLAIN_MIN_PIX,
          PAINT_INT64_PLAIN_MAX_PIX,
          PAINT_FLOAT32_PLAIN_SET_PIX,
          PAINT_FLOAT32_PLAIN_ADD_PIX,
          PAINT_FLOAT32_PLAIN_SUB_PIX,
          PAINT_FLOAT32_PLAIN_MUL_PIX,
          PAINT_FLOAT32_PLAIN_DIV_PIX,
          PAINT_FLOAT32_PLAIN_MIN_PIX,
          PAINT_FLOAT32_PLAIN_MAX_PIX,
          PAINT_FLOAT64_PLAIN_SET_PIX,
          PAINT_FLOAT64_PLAIN_ADD_PIX,
          PAINT_FLOAT64_PLAIN_SUB_PIX,
          PAINT_FLOAT64_PLAIN_MUL_PIX,
          PAINT_FLOAT64_PLAIN_DIV_PIX,
          PAINT_FLOAT64_PLAIN_MIN_PIX,
          PAINT_FLOAT64_PLAIN_MAX_PIX,
          PAINT_UINT8_RGB_SET_PIX,
          PAINT_UINT8_RGB_ADD_PIX,
          PAINT_UINT8_RGB_SUB_PIX,
          PAINT_UINT8_RGB_MUL_PIX,
          PAINT_UINT8_RGB_DIV_PIX,
          PAINT_UINT8_RGB_MIN_PIX,
          PAINT_UINT8_RGB_MAX_PIX,
          PAINT_UINT16_RGB_SET_PIX,
          PAINT_UINT16_RGB_ADD_PIX,
          PAINT_UINT16_RGB_SUB_PIX,
          PAINT_UINT16_RGB_MUL_PIX,
          PAINT_UINT16_RGB_DIV_PIX,
          PAINT_UINT16_RGB_MIN_PIX,
          PAINT_UINT16_RGB_MAX_PIX,
          PAINT_UINT32_RGB_SET_PIX,
          PAINT_UINT32_RGB_ADD_PIX,
          PAINT_UINT32_RGB_SUB_PIX,
          PAINT_UINT32_RGB_MUL_PIX,
          PAINT_UINT32_RGB_DIV_PIX,
          PAINT_UINT32_RGB_MIN_PIX,
          PAINT_UINT32_RGB_MAX_PIX,
          PAINT_UINT64_RGB_SET_PIX,
          PAINT_UINT64_RGB_ADD_PIX,
          PAINT_UINT64_RGB_SUB_PIX,
          PAINT_UINT64_RGB_MUL_PIX,
          PAINT_UINT64_RGB_DIV_PIX,
          PAINT_UINT64_RGB_MIN_PIX,
          PAINT_UINT64_RGB_MAX_PIX,
          PAINT_INT8_RGB_SET_PIX,
          PAINT_INT8_RGB_ADD_PIX,
          PAINT_INT8_RGB_SUB_PIX,
          PAINT_INT8_RGB_MUL_PIX,
          PAINT_INT8_RGB_DIV_PIX,
          PAINT_INT8_RGB_MIN_PIX,
          PAINT_INT8_RGB_MAX_PIX,
          PAINT_INT16_RGB_SET_PIX,
          PAINT_INT16_RGB_ADD_PIX,
          PAINT_INT16_RGB_SUB_PIX,
          PAINT_INT16_RGB_MUL_PIX,
          PAINT_INT16_RGB_DIV_PIX,
          PAINT_INT16_RGB_MIN_PIX,
          PAINT_INT16_RGB_MAX_PIX,
          PAINT_INT32_RGB_SET_PIX,
          PAINT_INT32_RGB_ADD_PIX,
          PAINT_INT32_RGB_SUB_PIX,
          PAINT_INT32_RGB_MUL_PIX,
          PAINT_INT32_RGB_DIV_PIX,
          PAINT_INT32_RGB_MIN_PIX,
          PAINT_INT32_RGB_MAX_PIX,
          PAINT_INT64_RGB_SET_PIX,
          PAINT_INT64_RGB_ADD_PIX,
          PAINT_INT64_RGB_SUB_PIX,
          PAINT_INT64_RGB_MUL_PIX,
          PAINT_INT64_RGB_DIV_PIX,
          PAINT_INT64_RGB_MIN_PIX,
          PAINT_INT64_RGB_MAX_PIX,
          PAINT_FLOAT32_RGB_SET_PIX,
          PAINT_FLOAT32_RGB_ADD_PIX,
          PAINT_FLOAT32_RGB_SUB_PIX,
          PAINT_FLOAT32_RGB_MUL_PIX,
          PAINT_FLOAT32_RGB_DIV_PIX,
          PAINT_FLOAT32_RGB_MIN_PIX,
          PAINT_FLOAT32_RGB_MAX_PIX,
          PAINT_FLOAT64_RGB_SET_PIX,
          PAINT_FLOAT64_RGB_ADD_PIX,
          PAINT_FLOAT64_RGB_SUB_PIX,
          PAINT_FLOAT64_RGB_MUL_PIX,
          PAINT_FLOAT64_RGB_DIV_PIX,
          PAINT_FLOAT64_RGB_MIN_PIX,
          PAINT_FLOAT64_RGB_MAX_PIX,
          PAINT_UINT8_RGBA_SET_PIX,
          PAINT_UINT8_RGBA_ADD_PIX,
          PAINT_UINT8_RGBA_SUB_PIX,
          PAINT_UINT8_RGBA_MUL_PIX,
          PAINT_UINT8_RGBA_DIV_PIX,
          PAINT_UINT8_RGBA_MIN_PIX,
          PAINT_UINT8_RGBA_MAX_PIX,
          PAINT_UINT16_RGBA_SET_PIX,
          PAINT_UINT16_RGBA_ADD_PIX,
          PAINT_UINT16_RGBA_SUB_PIX,
          PAINT_UINT16_RGBA_MUL_PIX,
          PAINT_UINT16_RGBA_DIV_PIX,
          PAINT_UINT16_RGBA_MIN_PIX,
          PAINT_UINT16_RGBA_MAX_PIX,
          PAINT_UINT32_RGBA_SET_PIX,
          PAINT_UINT32_RGBA_ADD_PIX,
          PAINT_UINT32_RGBA_SUB_PIX,
          PAINT_UINT32_RGBA_MUL_PIX,
          PAINT_UINT32_RGBA_DIV_PIX,
          PAINT_UINT32_RGBA_MIN_PIX,
          PAINT_UINT32_RGBA_MAX_PIX,
          PAINT_UINT64_RGBA_SET_PIX,
          PAINT_UINT64_RGBA_ADD_PIX,
          PAINT_UINT64_RGBA_SUB_PIX,
          PAINT_UINT64_RGBA_MUL_PIX,
          PAINT_UINT64_RGBA_DIV_PIX,
          PAINT_UINT64_RGBA_MIN_PIX,
          PAINT_UINT64_RGBA_MAX_PIX,
          PAINT_INT8_RGBA_SET_PIX,
          PAINT_INT8_RGBA_ADD_PIX,
          PAINT_INT8_RGBA_SUB_PIX,
          PAINT_INT8_RGBA_MUL_PIX,
          PAINT_INT8_RGBA_DIV_PIX,
          PAINT_INT8_RGBA_MIN_PIX,
          PAINT_INT8_RGBA_MAX_PIX,
          PAINT_INT16_RGBA_SET_PIX,
          PAINT_INT16_RGBA_ADD_PIX,
          PAINT_INT16_RGBA_SUB_PIX,
          PAINT_INT16_RGBA_MUL_PIX,
          PAINT_INT16_RGBA_DIV_PIX,
          PAINT_INT16_RGBA_MIN_PIX,
          PAINT_INT16_RGBA_MAX_PIX,
          PAINT_INT32_RGBA_SET_PIX,
          PAINT_INT32_RGBA_ADD_PIX,
          PAINT_INT32_RGBA_SUB_PIX,
          PAINT_INT32_RGBA_MUL_PIX,
          PAINT_INT32_RGBA_DIV_PIX,
          PAINT_INT32_RGBA_MIN_PIX,
          PAINT_INT32_RGBA_MAX_PIX,
          PAINT_INT64_RGBA_SET_PIX,
          PAINT_INT64_RGBA_ADD_PIX,
          PAINT_INT64_RGBA_SUB_PIX,
          PAINT_INT64_RGBA_MUL_PIX,
          PAINT_INT64_RGBA_DIV_PIX,
          PAINT_INT64_RGBA_MIN_PIX,
          PAINT_INT64_RGBA_MAX_PIX,
          PAINT_FLOAT32_RGBA_SET_PIX,
          PAINT_FLOAT32_RGBA_ADD_PIX,
          PAINT_FLOAT32_RGBA_SUB_PIX,
          PAINT_FLOAT32_RGBA_MUL_PIX,
          PAINT_FLOAT32_RGBA_DIV_PIX,
          PAINT_FLOAT32_RGBA_MIN_PIX,
          PAINT_FLOAT32_RGBA_MAX_PIX,
          PAINT_FLOAT64_RGBA_SET_PIX,
          PAINT_FLOAT64_RGBA_ADD_PIX,
          PAINT_FLOAT64_RGBA_SUB_PIX,
          PAINT_FLOAT64_RGBA_MUL_PIX,
          PAINT_FLOAT64_RGBA_DIV_PIX,
          PAINT_FLOAT64_RGBA_MIN_PIX,
          PAINT_FLOAT64_RGBA_MAX_PIX,
          PAINT_UINT8_COMPLEX_SET_PIX,
          PAINT_UINT8_COMPLEX_ADD_PIX,
          PAINT_UINT8_COMPLEX_SUB_PIX,
          PAINT_UINT8_COMPLEX_MUL_PIX,
          PAINT_UINT8_COMPLEX_DIV_PIX,
          PAINT_UINT8_COMPLEX_MIN_PIX,
          PAINT_UINT8_COMPLEX_MAX_PIX,
          PAINT_UINT16_COMPLEX_SET_PIX,
          PAINT_UINT16_COMPLEX_ADD_PIX,
          PAINT_UINT16_COMPLEX_SUB_PIX,
          PAINT_UINT16_COMPLEX_MUL_PIX,
          PAINT_UINT16_COMPLEX_DIV_PIX,
          PAINT_UINT16_COMPLEX_MIN_PIX,
          PAINT_UINT16_COMPLEX_MAX_PIX,
          PAINT_UINT32_COMPLEX_SET_PIX,
          PAINT_UINT32_COMPLEX_ADD_PIX,
          PAINT_UINT32_COMPLEX_SUB_PIX,
          PAINT_UINT32_COMPLEX_MUL_PIX,
          PAINT_UINT32_COMPLEX_DIV_PIX,
          PAINT_UINT32_COMPLEX_MIN_PIX,
          PAINT_UINT32_COMPLEX_MAX_PIX,
          PAINT_UINT64_COMPLEX_SET_PIX,
          PAINT_UINT64_COMPLEX_ADD_PIX,
          PAINT_UINT64_COMPLEX_SUB_PIX,
          PAINT_UINT64_COMPLEX_MUL_PIX,
          PAINT_UINT64_COMPLEX_DIV_PIX,
          PAINT_UINT64_COMPLEX_MIN_PIX,
          PAINT_UINT64_COMPLEX_MAX_PIX,
          PAINT_INT8_COMPLEX_SET_PIX,
          PAINT_INT8_COMPLEX_ADD_PIX,
          PAINT_INT8_COMPLEX_SUB_PIX,
          PAINT_INT8_COMPLEX_MUL_PIX,
          PAINT_INT8_COMPLEX_DIV_PIX,
          PAINT_INT8_COMPLEX_MIN_PIX,
          PAINT_INT8_COMPLEX_MAX_PIX,
          PAINT_INT16_COMPLEX_SET_PIX,
          PAINT_INT16_COMPLEX_ADD_PIX,
          PAINT_INT16_COMPLEX_SUB_PIX,
          PAINT_INT16_COMPLEX_MUL_PIX,
          PAINT_INT16_COMPLEX_DIV_PIX,
          PAINT_INT16_COMPLEX_MIN_PIX,
          PAINT_INT16_COMPLEX_MAX_PIX,
          PAINT_INT32_COMPLEX_SET_PIX,
          PAINT_INT32_COMPLEX_ADD_PIX,
          PAINT_INT32_COMPLEX_SUB_PIX,
          PAINT_INT32_COMPLEX_MUL_PIX,
          PAINT_INT32_COMPLEX_DIV_PIX,
          PAINT_INT32_COMPLEX_MIN_PIX,
          PAINT_INT32_COMPLEX_MAX_PIX,
          PAINT_INT64_COMPLEX_SET_PIX,
          PAINT_INT64_COMPLEX_ADD_PIX,
          PAINT_INT64_COMPLEX_SUB_PIX,
          PAINT_INT64_COMPLEX_MUL_PIX,
          PAINT_INT64_COMPLEX_DIV_PIX,
          PAINT_INT64_COMPLEX_MIN_PIX,
          PAINT_INT64_COMPLEX_MAX_PIX,
          PAINT_FLOAT32_COMPLEX_SET_PIX,
          PAINT_FLOAT32_COMPLEX_ADD_PIX,
          PAINT_FLOAT32_COMPLEX_SUB_PIX,
          PAINT_FLOAT32_COMPLEX_MUL_PIX,
          PAINT_FLOAT32_COMPLEX_DIV_PIX,
          PAINT_FLOAT32_COMPLEX_MIN_PIX,
          PAINT_FLOAT32_COMPLEX_MAX_PIX,
          PAINT_FLOAT64_COMPLEX_SET_PIX,
          PAINT_FLOAT64_COMPLEX_ADD_PIX,
          PAINT_FLOAT64_COMPLEX_SUB_PIX,
          PAINT_FLOAT64_COMPLEX_MUL_PIX,
          PAINT_FLOAT64_COMPLEX_DIV_PIX,
          PAINT_FLOAT64_COMPLEX_MIN_PIX,
          PAINT_FLOAT64_COMPLEX_MAX_PIX,
  };
  
void *SETUP_PAINTER(Array *canvas, Brush_Bundle *generic, void *argp)
{ Color_Bundle *brush = (Color_Bundle  *) generic;
  int           kind  = canvas->kind;

  if (brush->op == MUL_PIX || brush->op == DIV_PIX)
    switch (canvas->type) {
        case UINT8_TYPE:
          { MUPNT_UINT8 *ap = (MUPNT_UINT8 *) argp;
  
            ap->red  = (uint8 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case UINT16_TYPE:
          { MUPNT_UINT16 *ap = (MUPNT_UINT16 *) argp;
  
            ap->red  = (uint16 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case UINT32_TYPE:
          { MUPNT_UINT32 *ap = (MUPNT_UINT32 *) argp;
  
            ap->red  = (uint32 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case UINT64_TYPE:
          { MUPNT_UINT64 *ap = (MUPNT_UINT64 *) argp;
  
            ap->red  = (uint64 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case INT8_TYPE:
          { MUPNT_INT8 *ap = (MUPNT_INT8 *) argp;
  
            ap->red  = (int8 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case INT16_TYPE:
          { MUPNT_INT16 *ap = (MUPNT_INT16 *) argp;
  
            ap->red  = (int16 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case INT32_TYPE:
          { MUPNT_INT32 *ap = (MUPNT_INT32 *) argp;
  
            ap->red  = (int32 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case INT64_TYPE:
          { MUPNT_INT64 *ap = (MUPNT_INT64 *) argp;
  
            ap->red  = (int64 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case FLOAT32_TYPE:
          { MUPNT_FLOAT32 *ap = (MUPNT_FLOAT32 *) argp;
  
            ap->red  = (float32 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case FLOAT64_TYPE:
          { MUPNT_FLOAT64 *ap = (MUPNT_FLOAT64 *) argp;
  
            ap->red  = (float64 *) (canvas->data);
            ap->redI = brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = brush->green.fval;
                    ap->blueI  = brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = brush->alpha.fval;
                      }
                  }
              }
            break;
          }
    }
  else
    switch (canvas->type) {
        case UINT8_TYPE:
          { PAINT_UINT8 *ap = (PAINT_UINT8 *) argp;

            ap->red  = (uint8 *) (canvas->data);
            ap->redI = (uint8) brush->red.uval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (uint8) brush->green.uval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (uint8) brush->green.uval;
                    ap->blueI  = (uint8) brush->blue.uval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (uint8) brush->alpha.uval;
                      }
                  }
              }
            break;
          }
        case UINT16_TYPE:
          { PAINT_UINT16 *ap = (PAINT_UINT16 *) argp;

            ap->red  = (uint16 *) (canvas->data);
            ap->redI = (uint16) brush->red.uval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (uint16) brush->green.uval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (uint16) brush->green.uval;
                    ap->blueI  = (uint16) brush->blue.uval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (uint16) brush->alpha.uval;
                      }
                  }
              }
            break;
          }
        case UINT32_TYPE:
          { PAINT_UINT32 *ap = (PAINT_UINT32 *) argp;

            ap->red  = (uint32 *) (canvas->data);
            ap->redI = (uint32) brush->red.uval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (uint32) brush->green.uval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (uint32) brush->green.uval;
                    ap->blueI  = (uint32) brush->blue.uval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (uint32) brush->alpha.uval;
                      }
                  }
              }
            break;
          }
        case UINT64_TYPE:
          { PAINT_UINT64 *ap = (PAINT_UINT64 *) argp;

            ap->red  = (uint64 *) (canvas->data);
            ap->redI = (uint64) brush->red.uval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (uint64) brush->green.uval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (uint64) brush->green.uval;
                    ap->blueI  = (uint64) brush->blue.uval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (uint64) brush->alpha.uval;
                      }
                  }
              }
            break;
          }
        case INT8_TYPE:
          { PAINT_INT8 *ap = (PAINT_INT8 *) argp;

            ap->red  = (int8 *) (canvas->data);
            ap->redI = (int8) brush->red.ival;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (int8) brush->green.ival;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (int8) brush->green.ival;
                    ap->blueI  = (int8) brush->blue.ival;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (int8) brush->alpha.ival;
                      }
                  }
              }
            break;
          }
        case INT16_TYPE:
          { PAINT_INT16 *ap = (PAINT_INT16 *) argp;

            ap->red  = (int16 *) (canvas->data);
            ap->redI = (int16) brush->red.ival;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (int16) brush->green.ival;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (int16) brush->green.ival;
                    ap->blueI  = (int16) brush->blue.ival;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (int16) brush->alpha.ival;
                      }
                  }
              }
            break;
          }
        case INT32_TYPE:
          { PAINT_INT32 *ap = (PAINT_INT32 *) argp;

            ap->red  = (int32 *) (canvas->data);
            ap->redI = (int32) brush->red.ival;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (int32) brush->green.ival;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (int32) brush->green.ival;
                    ap->blueI  = (int32) brush->blue.ival;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (int32) brush->alpha.ival;
                      }
                  }
              }
            break;
          }
        case INT64_TYPE:
          { PAINT_INT64 *ap = (PAINT_INT64 *) argp;

            ap->red  = (int64 *) (canvas->data);
            ap->redI = (int64) brush->red.ival;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (int64) brush->green.ival;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (int64) brush->green.ival;
                    ap->blueI  = (int64) brush->blue.ival;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (int64) brush->alpha.ival;
                      }
                  }
              }
            break;
          }
        case FLOAT32_TYPE:
          { PAINT_FLOAT32 *ap = (PAINT_FLOAT32 *) argp;

            ap->red  = (float32 *) (canvas->data);
            ap->redI = (float32) brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (float32) brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (float32) brush->green.fval;
                    ap->blueI  = (float32) brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (float32) brush->alpha.fval;
                      }
                  }
              }
            break;
          }
        case FLOAT64_TYPE:
          { PAINT_FLOAT64 *ap = (PAINT_FLOAT64 *) argp;

            ap->red  = (float64 *) (canvas->data);
            ap->redI = (float64) brush->red.fval;
            if (kind > PLAIN_KIND)
              { if (kind == COMPLEX_KIND)
                  { ap->green  = ap->red + 1;
                    ap->greenI = (float64) brush->green.fval;
                  }
                else
                  { Indx_Type n = canvas->size / canvas->dims[canvas->ndims-1];
                    ap->green  = ap->red + n;
                    ap->blue   = ap->green + n;
                    ap->greenI = (float64) brush->green.fval;
                    ap->blueI  = (float64) brush->blue.fval;
                    if (kind == RGBA_KIND)
                      { ap->alpha  = ap->blue + n;
                        ap->alphaI = (float64) brush->alpha.fval;
                      }
                  }
              }
            break;
          }
    }

  return (Painter_Table[(canvas->kind*10 + canvas->type)*7 + brush->op]);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR REGIONS                                                        *
 *                                                                                      *
 ****************************************************************************************/

void check_shapes(Array *canvas, Region *reg, char *routine)
{ Array_Bundle rarray;

  rarray.kind  = PLAIN_KIND;
  rarray.ndims = Get_Region_Dimensionality(reg);
  rarray.dims  = Get_Region_Dimensions(reg);
  if (canvas->kind != PLAIN_KIND)
    { canvas->ndims -= 1;
      if (canvas->kind == COMPLEX_KIND)
        canvas->dims += 1;
    }
  if ( ! Same_Shape(canvas,&rarray))
    { fprintf(stderr,"Canvas and image from which region was computed do not have the same shape");
      fprintf(stderr," (%s)\n",routine);
      exit (1);
    }
  if (canvas->kind != PLAIN_KIND)
    { if (canvas->kind == COMPLEX_KIND)
        canvas->dims -= 1;
      canvas->ndims += 1;
    }
}

/* Color the pixels of region reg */

void Draw_Region_Outline(Array *M(canvas), Brush_Bundle *brush, Region *reg)
{ void      (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;

  check_shapes(canvas,reg,"Draw_Region_Outline");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Outline(reg,argp,painter);
}

/* Color the region defined by reg */

void Draw_Region(Array *M(canvas), Brush_Bundle *brush, Region *reg)
{ Indx_Type *raster;
  Indx_Type  len;
  void     (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;

  check_shapes(canvas,reg,"Draw_Region");

  painter = SETUP_PAINTER(canvas,brush,argp);

  raster = reg->raster;
  len    = reg->rastlen;

  { Indx_Type i, v, w, p;

    for (i = 0; i < len; i += 2)
      { v = raster[i];
        w = raster[i+1];
        for (p = v; p <= w; p++)
          painter(p,argp);
      }
  }
}

/* Color the complement of the region defined by reg */

void Draw_Region_Exterior(Array *M(canvas), Brush_Bundle *brush, Region *reg)
{ void     (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;

  check_shapes(canvas,reg,"Draw_Region_Exterior");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Exterior(reg,argp,painter);
}

void Draw_Region_Holes(Array *M(canvas), Brush_Bundle *brush, Region *reg)
{ void     (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;

  check_shapes(canvas,reg,"Draw_Region_Holes");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Holes(reg,argp,painter);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR LEVEL SETS                                                     *
 *                                                                                      *
 ****************************************************************************************/

void Draw_Level_Set_Outline(Array *M(canvas), Brush_Bundle *brush,
                            Level_Tree *t, Level_Set *r, int share)
{ void      (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;
  Region    *c;

  c = Record_Level_Set(t,r,share,0);

  check_shapes(canvas,c,"Draw_Level_Set_Outline");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Outline(c,argp,painter);

  Free_Region(c);
}

void Draw_Level_Set_Exterior(Array *M(canvas), Brush_Bundle *brush,
                             Level_Tree *t, Level_Set *r, int share)
{ void      (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;
  Region    *c;

  c = Record_Level_Set(t,r,share,0);

  check_shapes(canvas,c,"Draw_Level_Set_Exterior");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Exterior(c,argp,painter);

  Free_Region(c);
}

void Draw_Level_Set_Holes(Array *M(canvas), Brush_Bundle *brush,
                          Level_Tree *t, Level_Set *r, int share)
{ void      (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;
  Region    *c;

  c = Record_Level_Set(t,r,share,1);

  check_shapes(canvas,c,"Draw_Level_Set_Holes");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Holes(c,argp,painter);

  Free_Region(c);
}


/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR WATERHSEDS                                                     *
 *                                                                                      *
 ****************************************************************************************/

void Draw_P_Vertex_Outline(Array *M(canvas), Brush_Bundle *brush, Partition *w, int cb, int share)
{ void      (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;
  Region    *c;

  if (Get_Partition_Labels(w) == NULL)
    { fprintf(stderr,"Partition does not have a label array (Draw_P_Vertex_Outline)\n");
      exit (1);
    }

  c = Record_P_Vertex(w,cb,share,0);

  check_shapes(canvas,c,"Draw_P_Vertex_Outline");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Outline(c,argp,painter);

  Free_Region(c);
}

void Draw_P_Vertex_Exterior(Array *M(canvas), Brush_Bundle *brush, Partition *w, int cb, int share)
{ void      (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;
  Region    *c;

  if (Get_Partition_Labels(w) == NULL)
    { fprintf(stderr,"Partition does not have a label array (Draw_P_Vertex_Exterior)\n");
      exit (1);
    }

  c = Record_P_Vertex(w,cb,share,0);

  check_shapes(canvas,c,"Draw_P_Vertex_Exterior");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Exterior(c,argp,painter);

  Free_Region(c);
}

void Draw_P_Vertex_Holes(Array *M(canvas), Brush_Bundle *brush, Partition *w, int cb, int share)
{ void      (*painter)(Indx_Type,void *);
  PAINT_VOID arg, *argp = &arg;
  Region    *c;

  if (Get_Partition_Labels(w) == NULL)
    { fprintf(stderr,"Partition does not have a label array (Draw_P_Vertex_Holes)\n");
      exit (1);
    }

  c = Record_P_Vertex(w,cb,share,1);

  check_shapes(canvas,c,"Draw_P_Vertex_Holes");

  painter = SETUP_PAINTER(canvas,brush,argp);

  For_Region_Holes(c,argp,painter);

  Free_Region(c);
}


/****************************************************************************************
 *                                                                                      *
 *  IMAGE DRAWING ROUTINES FOR REGION, LEVEL SETS, & WATERHSEDS                         *
 *                                                                                      *
 ****************************************************************************************/

void draw_region_image(Array *M(canvas), Array *image, Region *reg, string routine)
{ Indx_Type *raster;
  Indx_Type  len;

  check_shapes(canvas,reg,routine);
  if ( ! Same_Type(canvas,image))
    { fprintf(stderr,"Canvas and image must have the same shape and type (routine)\n");
      exit (1);
    }
 
  raster = reg->raster;
  len    = reg->rastlen;

  { Indx_Type i, v, w, p;

    switch (canvas->kind)
    { case PLAIN_KIND:
        switch (canvas->type) {
          case UINT8_TYPE:
            { uint8 *val  = AUINT8(canvas);
              uint8 *valI = AUINT8(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case UINT16_TYPE:
            { uint16 *val  = AUINT16(canvas);
              uint16 *valI = AUINT16(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case UINT32_TYPE:
            { uint32 *val  = AUINT32(canvas);
              uint32 *valI = AUINT32(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case UINT64_TYPE:
            { uint64 *val  = AUINT64(canvas);
              uint64 *valI = AUINT64(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case INT8_TYPE:
            { int8 *val  = AINT8(canvas);
              int8 *valI = AINT8(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case INT16_TYPE:
            { int16 *val  = AINT16(canvas);
              int16 *valI = AINT16(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case INT32_TYPE:
            { int32 *val  = AINT32(canvas);
              int32 *valI = AINT32(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case INT64_TYPE:
            { int64 *val  = AINT64(canvas);
              int64 *valI = AINT64(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32 *val  = AFLOAT32(canvas);
              float32 *valI = AFLOAT32(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64 *val  = AFLOAT64(canvas);
              float64 *valI = AFLOAT64(image);
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    val[p] = valI[p]; 
                }
              break;
            }
        }
        break;
      case RGB_KIND:
        switch (canvas->type) {
          case UINT8_TYPE:
            { Size_Type N = canvas->size / 3;
              uint8 *red    = AUINT8(canvas);
              uint8 *redI   = AUINT8(image);
              uint8 *green  = red + N;
              uint8 *greenI = redI + N;
              uint8 *blue   = green + N;
              uint8 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { Size_Type N = canvas->size / 3;
              uint16 *red    = AUINT16(canvas);
              uint16 *redI   = AUINT16(image);
              uint16 *green  = red + N;
              uint16 *greenI = redI + N;
              uint16 *blue   = green + N;
              uint16 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { Size_Type N = canvas->size / 3;
              uint32 *red    = AUINT32(canvas);
              uint32 *redI   = AUINT32(image);
              uint32 *green  = red + N;
              uint32 *greenI = redI + N;
              uint32 *blue   = green + N;
              uint32 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { Size_Type N = canvas->size / 3;
              uint64 *red    = AUINT64(canvas);
              uint64 *redI   = AUINT64(image);
              uint64 *green  = red + N;
              uint64 *greenI = redI + N;
              uint64 *blue   = green + N;
              uint64 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case INT8_TYPE:
            { Size_Type N = canvas->size / 3;
              int8 *red    = AINT8(canvas);
              int8 *redI   = AINT8(image);
              int8 *green  = red + N;
              int8 *greenI = redI + N;
              int8 *blue   = green + N;
              int8 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case INT16_TYPE:
            { Size_Type N = canvas->size / 3;
              int16 *red    = AINT16(canvas);
              int16 *redI   = AINT16(image);
              int16 *green  = red + N;
              int16 *greenI = redI + N;
              int16 *blue   = green + N;
              int16 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case INT32_TYPE:
            { Size_Type N = canvas->size / 3;
              int32 *red    = AINT32(canvas);
              int32 *redI   = AINT32(image);
              int32 *green  = red + N;
              int32 *greenI = redI + N;
              int32 *blue   = green + N;
              int32 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case INT64_TYPE:
            { Size_Type N = canvas->size / 3;
              int64 *red    = AINT64(canvas);
              int64 *redI   = AINT64(image);
              int64 *green  = red + N;
              int64 *greenI = redI + N;
              int64 *blue   = green + N;
              int64 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { Size_Type N = canvas->size / 3;
              float32 *red    = AFLOAT32(canvas);
              float32 *redI   = AFLOAT32(image);
              float32 *green  = red + N;
              float32 *greenI = redI + N;
              float32 *blue   = green + N;
              float32 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { Size_Type N = canvas->size / 3;
              float64 *red    = AFLOAT64(canvas);
              float64 *redI   = AFLOAT64(image);
              float64 *green  = red + N;
              float64 *greenI = redI + N;
              float64 *blue   = green + N;
              float64 *blueI  = greenI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                    }
                }
              break;
            }
        }
        break;
      case RGBA_KIND:
        switch (canvas->type) {
          case UINT8_TYPE:
            { Size_Type N = canvas->size / 4;
              uint8 *red    = AUINT8(canvas);
              uint8 *redI   = AUINT8(image);
              uint8 *green  = red + N;
              uint8 *greenI = redI + N;
              uint8 *blue   = green + N;
              uint8 *blueI  = greenI + N;
              uint8 *alpha  = blue + N;
              uint8 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { Size_Type N = canvas->size / 4;
              uint16 *red    = AUINT16(canvas);
              uint16 *redI   = AUINT16(image);
              uint16 *green  = red + N;
              uint16 *greenI = redI + N;
              uint16 *blue   = green + N;
              uint16 *blueI  = greenI + N;
              uint16 *alpha  = blue + N;
              uint16 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { Size_Type N = canvas->size / 4;
              uint32 *red    = AUINT32(canvas);
              uint32 *redI   = AUINT32(image);
              uint32 *green  = red + N;
              uint32 *greenI = redI + N;
              uint32 *blue   = green + N;
              uint32 *blueI  = greenI + N;
              uint32 *alpha  = blue + N;
              uint32 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { Size_Type N = canvas->size / 4;
              uint64 *red    = AUINT64(canvas);
              uint64 *redI   = AUINT64(image);
              uint64 *green  = red + N;
              uint64 *greenI = redI + N;
              uint64 *blue   = green + N;
              uint64 *blueI  = greenI + N;
              uint64 *alpha  = blue + N;
              uint64 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case INT8_TYPE:
            { Size_Type N = canvas->size / 4;
              int8 *red    = AINT8(canvas);
              int8 *redI   = AINT8(image);
              int8 *green  = red + N;
              int8 *greenI = redI + N;
              int8 *blue   = green + N;
              int8 *blueI  = greenI + N;
              int8 *alpha  = blue + N;
              int8 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case INT16_TYPE:
            { Size_Type N = canvas->size / 4;
              int16 *red    = AINT16(canvas);
              int16 *redI   = AINT16(image);
              int16 *green  = red + N;
              int16 *greenI = redI + N;
              int16 *blue   = green + N;
              int16 *blueI  = greenI + N;
              int16 *alpha  = blue + N;
              int16 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case INT32_TYPE:
            { Size_Type N = canvas->size / 4;
              int32 *red    = AINT32(canvas);
              int32 *redI   = AINT32(image);
              int32 *green  = red + N;
              int32 *greenI = redI + N;
              int32 *blue   = green + N;
              int32 *blueI  = greenI + N;
              int32 *alpha  = blue + N;
              int32 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case INT64_TYPE:
            { Size_Type N = canvas->size / 4;
              int64 *red    = AINT64(canvas);
              int64 *redI   = AINT64(image);
              int64 *green  = red + N;
              int64 *greenI = redI + N;
              int64 *blue   = green + N;
              int64 *blueI  = greenI + N;
              int64 *alpha  = blue + N;
              int64 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { Size_Type N = canvas->size / 4;
              float32 *red    = AFLOAT32(canvas);
              float32 *redI   = AFLOAT32(image);
              float32 *green  = red + N;
              float32 *greenI = redI + N;
              float32 *blue   = green + N;
              float32 *blueI  = greenI + N;
              float32 *alpha  = blue + N;
              float32 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { Size_Type N = canvas->size / 4;
              float64 *red    = AFLOAT64(canvas);
              float64 *redI   = AFLOAT64(image);
              float64 *green  = red + N;
              float64 *greenI = redI + N;
              float64 *blue   = green + N;
              float64 *blueI  = greenI + N;
              float64 *alpha  = blue + N;
              float64 *alphaI = blueI + N;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { red[p]   = redI[p]; 
                      green[p] = greenI[p]; 
                      blue[p]  = blueI[p]; 
                      alpha[p] = alphaI[p]; 
                    }
                }
              break;
            }
        }
        break;
      case COMPLEX_KIND:
        switch (canvas->type) {
          case UINT8_TYPE:
            { uint8 *real  = AUINT8(canvas);
              uint8 *realI = AUINT8(image);
              uint8 *imag  = real + 1;
              uint8 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case UINT16_TYPE:
            { uint16 *real  = AUINT16(canvas);
              uint16 *realI = AUINT16(image);
              uint16 *imag  = real + 1;
              uint16 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case UINT32_TYPE:
            { uint32 *real  = AUINT32(canvas);
              uint32 *realI = AUINT32(image);
              uint32 *imag  = real + 1;
              uint32 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case UINT64_TYPE:
            { uint64 *real  = AUINT64(canvas);
              uint64 *realI = AUINT64(image);
              uint64 *imag  = real + 1;
              uint64 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case INT8_TYPE:
            { int8 *real  = AINT8(canvas);
              int8 *realI = AINT8(image);
              int8 *imag  = real + 1;
              int8 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case INT16_TYPE:
            { int16 *real  = AINT16(canvas);
              int16 *realI = AINT16(image);
              int16 *imag  = real + 1;
              int16 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case INT32_TYPE:
            { int32 *real  = AINT32(canvas);
              int32 *realI = AINT32(image);
              int32 *imag  = real + 1;
              int32 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case INT64_TYPE:
            { int64 *real  = AINT64(canvas);
              int64 *realI = AINT64(image);
              int64 *imag  = real + 1;
              int64 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case FLOAT32_TYPE:
            { float32 *real  = AFLOAT32(canvas);
              float32 *realI = AFLOAT32(image);
              float32 *imag  = real + 1;
              float32 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
          case FLOAT64_TYPE:
            { float64 *real  = AFLOAT64(canvas);
              float64 *realI = AFLOAT64(image);
              float64 *imag  = real + 1;
              float64 *imagI = realI + 1;
              for (i = 0; i < len; i += 2)
                { v = raster[i];
                  w = raster[i+1];
                  for (p = v; p <= w; p++)
                    { real[p] = realI[p]; 
                      imag[p] = imagI[p]; 
                    }
                }
              break;
            }
        }
        break;
    }
  }
}

void Draw_Region_Image(Array *M(canvas), Array *image, Region *reg)
{ draw_region_image(canvas,image,reg,"Draw_Region_Image"); }

void Draw_Level_Set_Image(Array *M(canvas), Array *image, Level_Tree *t, Level_Set *r, int share)
{ Region *c;
  c = Record_Level_Set(t,r,share,1);
  draw_region_image(canvas,image,c,"Draw_Level_Set_Image");
  Free_Region(c);
}

void Draw_P_Vertex_Image(Array *M(canvas), Array *image, Partition *w, int cb, int share)
{ Region *c;

  if (Get_Partition_Labels(w) == NULL)
    { fprintf(stderr,"Partition does not have a label array (Draw_P_Vertex_Image)\n");
      exit (1);
    }

  c = Record_P_Vertex(w,cb,share,1);
  draw_region_image(canvas,image,c,"Draw_P_Vertex_Image");
  Free_Region(c);
}

/****************************************************************************************
 *                                                                                      *
 *  GENERAL FLOOD-FILL DRAWING ROUTINE                                                  *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { void    *value;
    uint64   level_uval;
    int64    level_ival;
    float64  level_fval;
  } DrawArg;

#define DA(a) ((DrawArg *) (a))

    static boolean is_le_uint8(Indx_Type p, void *a)
    { return (((uint8 *) DA(a)->value)[p] <= DA(a)->level_uval); }
    static boolean is_lt_uint8(Indx_Type p, void *a)
    { return (((uint8 *) DA(a)->value)[p] < DA(a)->level_uval); }
    static boolean is_eq_uint8(Indx_Type p, void *a)
    { return (((uint8 *) DA(a)->value)[p] == DA(a)->level_uval); }
    static boolean is_ne_uint8(Indx_Type p, void *a)
    { return (((uint8 *) DA(a)->value)[p] != DA(a)->level_uval); }
    static boolean is_gt_uint8(Indx_Type p, void *a)
    { return (((uint8 *) DA(a)->value)[p] > DA(a)->level_uval); }
    static boolean is_ge_uint8(Indx_Type p, void *a)
    { return (((uint8 *) DA(a)->value)[p] >= DA(a)->level_uval); }
    static boolean is_le_uint16(Indx_Type p, void *a)
    { return (((uint16 *) DA(a)->value)[p] <= DA(a)->level_uval); }
    static boolean is_lt_uint16(Indx_Type p, void *a)
    { return (((uint16 *) DA(a)->value)[p] < DA(a)->level_uval); }
    static boolean is_eq_uint16(Indx_Type p, void *a)
    { return (((uint16 *) DA(a)->value)[p] == DA(a)->level_uval); }
    static boolean is_ne_uint16(Indx_Type p, void *a)
    { return (((uint16 *) DA(a)->value)[p] != DA(a)->level_uval); }
    static boolean is_gt_uint16(Indx_Type p, void *a)
    { return (((uint16 *) DA(a)->value)[p] > DA(a)->level_uval); }
    static boolean is_ge_uint16(Indx_Type p, void *a)
    { return (((uint16 *) DA(a)->value)[p] >= DA(a)->level_uval); }
    static boolean is_le_uint32(Indx_Type p, void *a)
    { return (((uint32 *) DA(a)->value)[p] <= DA(a)->level_uval); }
    static boolean is_lt_uint32(Indx_Type p, void *a)
    { return (((uint32 *) DA(a)->value)[p] < DA(a)->level_uval); }
    static boolean is_eq_uint32(Indx_Type p, void *a)
    { return (((uint32 *) DA(a)->value)[p] == DA(a)->level_uval); }
    static boolean is_ne_uint32(Indx_Type p, void *a)
    { return (((uint32 *) DA(a)->value)[p] != DA(a)->level_uval); }
    static boolean is_gt_uint32(Indx_Type p, void *a)
    { return (((uint32 *) DA(a)->value)[p] > DA(a)->level_uval); }
    static boolean is_ge_uint32(Indx_Type p, void *a)
    { return (((uint32 *) DA(a)->value)[p] >= DA(a)->level_uval); }
    static boolean is_le_uint64(Indx_Type p, void *a)
    { return (((uint64 *) DA(a)->value)[p] <= DA(a)->level_uval); }
    static boolean is_lt_uint64(Indx_Type p, void *a)
    { return (((uint64 *) DA(a)->value)[p] < DA(a)->level_uval); }
    static boolean is_eq_uint64(Indx_Type p, void *a)
    { return (((uint64 *) DA(a)->value)[p] == DA(a)->level_uval); }
    static boolean is_ne_uint64(Indx_Type p, void *a)
    { return (((uint64 *) DA(a)->value)[p] != DA(a)->level_uval); }
    static boolean is_gt_uint64(Indx_Type p, void *a)
    { return (((uint64 *) DA(a)->value)[p] > DA(a)->level_uval); }
    static boolean is_ge_uint64(Indx_Type p, void *a)
    { return (((uint64 *) DA(a)->value)[p] >= DA(a)->level_uval); }
    static boolean is_le_int8(Indx_Type p, void *a)
    { return (((int8 *) DA(a)->value)[p] <= DA(a)->level_ival); }
    static boolean is_lt_int8(Indx_Type p, void *a)
    { return (((int8 *) DA(a)->value)[p] < DA(a)->level_ival); }
    static boolean is_eq_int8(Indx_Type p, void *a)
    { return (((int8 *) DA(a)->value)[p] == DA(a)->level_ival); }
    static boolean is_ne_int8(Indx_Type p, void *a)
    { return (((int8 *) DA(a)->value)[p] != DA(a)->level_ival); }
    static boolean is_gt_int8(Indx_Type p, void *a)
    { return (((int8 *) DA(a)->value)[p] > DA(a)->level_ival); }
    static boolean is_ge_int8(Indx_Type p, void *a)
    { return (((int8 *) DA(a)->value)[p] >= DA(a)->level_ival); }
    static boolean is_le_int16(Indx_Type p, void *a)
    { return (((int16 *) DA(a)->value)[p] <= DA(a)->level_ival); }
    static boolean is_lt_int16(Indx_Type p, void *a)
    { return (((int16 *) DA(a)->value)[p] < DA(a)->level_ival); }
    static boolean is_eq_int16(Indx_Type p, void *a)
    { return (((int16 *) DA(a)->value)[p] == DA(a)->level_ival); }
    static boolean is_ne_int16(Indx_Type p, void *a)
    { return (((int16 *) DA(a)->value)[p] != DA(a)->level_ival); }
    static boolean is_gt_int16(Indx_Type p, void *a)
    { return (((int16 *) DA(a)->value)[p] > DA(a)->level_ival); }
    static boolean is_ge_int16(Indx_Type p, void *a)
    { return (((int16 *) DA(a)->value)[p] >= DA(a)->level_ival); }
    static boolean is_le_int32(Indx_Type p, void *a)
    { return (((int32 *) DA(a)->value)[p] <= DA(a)->level_ival); }
    static boolean is_lt_int32(Indx_Type p, void *a)
    { return (((int32 *) DA(a)->value)[p] < DA(a)->level_ival); }
    static boolean is_eq_int32(Indx_Type p, void *a)
    { return (((int32 *) DA(a)->value)[p] == DA(a)->level_ival); }
    static boolean is_ne_int32(Indx_Type p, void *a)
    { return (((int32 *) DA(a)->value)[p] != DA(a)->level_ival); }
    static boolean is_gt_int32(Indx_Type p, void *a)
    { return (((int32 *) DA(a)->value)[p] > DA(a)->level_ival); }
    static boolean is_ge_int32(Indx_Type p, void *a)
    { return (((int32 *) DA(a)->value)[p] >= DA(a)->level_ival); }
    static boolean is_le_int64(Indx_Type p, void *a)
    { return (((int64 *) DA(a)->value)[p] <= DA(a)->level_ival); }
    static boolean is_lt_int64(Indx_Type p, void *a)
    { return (((int64 *) DA(a)->value)[p] < DA(a)->level_ival); }
    static boolean is_eq_int64(Indx_Type p, void *a)
    { return (((int64 *) DA(a)->value)[p] == DA(a)->level_ival); }
    static boolean is_ne_int64(Indx_Type p, void *a)
    { return (((int64 *) DA(a)->value)[p] != DA(a)->level_ival); }
    static boolean is_gt_int64(Indx_Type p, void *a)
    { return (((int64 *) DA(a)->value)[p] > DA(a)->level_ival); }
    static boolean is_ge_int64(Indx_Type p, void *a)
    { return (((int64 *) DA(a)->value)[p] >= DA(a)->level_ival); }
    static boolean is_le_float32(Indx_Type p, void *a)
    { return (((float32 *) DA(a)->value)[p] <= DA(a)->level_fval); }
    static boolean is_lt_float32(Indx_Type p, void *a)
    { return (((float32 *) DA(a)->value)[p] < DA(a)->level_fval); }
    static boolean is_eq_float32(Indx_Type p, void *a)
    { return (((float32 *) DA(a)->value)[p] == DA(a)->level_fval); }
    static boolean is_ne_float32(Indx_Type p, void *a)
    { return (((float32 *) DA(a)->value)[p] != DA(a)->level_fval); }
    static boolean is_gt_float32(Indx_Type p, void *a)
    { return (((float32 *) DA(a)->value)[p] > DA(a)->level_fval); }
    static boolean is_ge_float32(Indx_Type p, void *a)
    { return (((float32 *) DA(a)->value)[p] >= DA(a)->level_fval); }
    static boolean is_le_float64(Indx_Type p, void *a)
    { return (((float64 *) DA(a)->value)[p] <= DA(a)->level_fval); }
    static boolean is_lt_float64(Indx_Type p, void *a)
    { return (((float64 *) DA(a)->value)[p] < DA(a)->level_fval); }
    static boolean is_eq_float64(Indx_Type p, void *a)
    { return (((float64 *) DA(a)->value)[p] == DA(a)->level_fval); }
    static boolean is_ne_float64(Indx_Type p, void *a)
    { return (((float64 *) DA(a)->value)[p] != DA(a)->level_fval); }
    static boolean is_gt_float64(Indx_Type p, void *a)
    { return (((float64 *) DA(a)->value)[p] > DA(a)->level_fval); }
    static boolean is_ge_float64(Indx_Type p, void *a)
    { return (((float64 *) DA(a)->value)[p] >= DA(a)->level_fval); }

static boolean (*Comparator_Table[])(Indx_Type,void *) = {
    is_le_uint8,
    is_lt_uint8,
    is_eq_uint8,
    is_ne_uint8,
    is_gt_uint8,
    is_ge_uint8,
    is_le_uint16,
    is_lt_uint16,
    is_eq_uint16,
    is_ne_uint16,
    is_gt_uint16,
    is_ge_uint16,
    is_le_uint32,
    is_lt_uint32,
    is_eq_uint32,
    is_ne_uint32,
    is_gt_uint32,
    is_ge_uint32,
    is_le_uint64,
    is_lt_uint64,
    is_eq_uint64,
    is_ne_uint64,
    is_gt_uint64,
    is_ge_uint64,
    is_le_int8,
    is_lt_int8,
    is_eq_int8,
    is_ne_int8,
    is_gt_int8,
    is_ge_int8,
    is_le_int16,
    is_lt_int16,
    is_eq_int16,
    is_ne_int16,
    is_gt_int16,
    is_ge_int16,
    is_le_int32,
    is_lt_int32,
    is_eq_int32,
    is_ne_int32,
    is_gt_int32,
    is_ge_int32,
    is_le_int64,
    is_lt_int64,
    is_eq_int64,
    is_ne_int64,
    is_gt_int64,
    is_ge_int64,
    is_le_float32,
    is_lt_float32,
    is_eq_float32,
    is_ne_float32,
    is_gt_float32,
    is_ge_float32,
    is_le_float64,
    is_lt_float64,
    is_eq_float64,
    is_ne_float64,
    is_gt_float64,
    is_ge_float64,
  };

static void check_drawing_compatibility(Array *canvas, Array *source, char *routine)
{ if (source->kind != PLAIN_KIND)
    { fprintf(stderr,"Source must be a plain array (%s)\n",routine);
      exit (1);
    }
  if (canvas->kind != PLAIN_KIND)
    { canvas->ndims -= 1;
      if (canvas->kind == COMPLEX_KIND)
        canvas->dims += 1;
    }
  if ( ! Same_Shape(canvas,source))
    { fprintf(stderr,"Canvas and source do not have the same shape! (%s)\n",routine);
      exit (1);
    }
  if (canvas->kind != PLAIN_KIND)
    { if (canvas->kind == COMPLEX_KIND)
        canvas->dims -= 1;
      canvas->ndims += 1;
    }
}

void Draw_Floodfill(Array *M(canvas), Brush_Bundle *brush,
                    APart *source, int share, boolean iscon2n,
                    Indx_Type seed, void *arg, boolean (*test)(Indx_Type p, void *arg))
{ Array      *array = AForm_Array(source);
  void      (*painter)(Indx_Type,void *);
  PAINT_VOID parg;

  check_drawing_compatibility(canvas,array,"Draw_Floodfill");

  painter = SETUP_PAINTER(canvas,brush,&parg);

  Flood_Object(source,share,iscon2n,seed,arg,test,NULL,NULL,NULL,NULL,&parg,painter);
}

void Draw_Basic(Array *M(canvas), Brush_Bundle *brush, APart *source, int share,
                boolean iscon2n, Indx_Type seed, Comparator cmprsn, Value level)
{ Array      *array = AForm_Array(source);
  boolean   (*value)(Indx_Type,void *);
  void      (*painter)(Indx_Type,void *);
  DrawArg    targ;
  PAINT_VOID parg;

  check_drawing_compatibility(canvas,array,"Draw_Basic");

  value   = Comparator_Table[6*array->type + cmprsn];
  painter = SETUP_PAINTER(canvas,brush,&parg);

  switch (array->type) {
      case UINT8_TYPE:
        targ.value = array->data;
        targ.level_uval = level.uval;
        break;
      case UINT16_TYPE:
        targ.value = array->data;
        targ.level_uval = level.uval;
        break;
      case UINT32_TYPE:
        targ.value = array->data;
        targ.level_uval = level.uval;
        break;
      case UINT64_TYPE:
        targ.value = array->data;
        targ.level_uval = level.uval;
        break;
      case INT8_TYPE:
        targ.value = array->data;
        targ.level_ival = level.ival;
        break;
      case INT16_TYPE:
        targ.value = array->data;
        targ.level_ival = level.ival;
        break;
      case INT32_TYPE:
        targ.value = array->data;
        targ.level_ival = level.ival;
        break;
      case INT64_TYPE:
        targ.value = array->data;
        targ.level_ival = level.ival;
        break;
      case FLOAT32_TYPE:
        targ.value = array->data;
        targ.level_fval = level.fval;
        break;
      case FLOAT64_TYPE:
        targ.value = array->data;
        targ.level_fval = level.fval;
        break;
  }

  Flood_Object(source,share,iscon2n,seed,&targ,value,NULL,NULL,NULL,NULL,&parg,painter);
}

void Draw_P_Vertex(Array *M(canvas), Brush_Bundle *brush, Partition *shed, int cb, int share)
{ Array        *source  = Get_Partition_Labels(shed);
  boolean       iscon2n = Is_Partition_2n_Connected(shed);
  Indx_Type     seed    = Get_Partition_Vertex(shed,cb)->seed;
  boolean      (*value)(Indx_Type,void *);
  void         (*painter)(Indx_Type,void *);
  DrawArg       targ;
  PAINT_VOID    parg;

  if (source == NULL)
    { fprintf(stderr,"Partition does not have a label array (Draw_P_Vertex_Image)\n");
      exit (1);
    }

  check_drawing_compatibility(canvas,source,"Draw_P_Vertex");

  value   = Comparator_Table[6*source->type + EQ_COMP];
  painter = SETUP_PAINTER(canvas,brush,&parg);

  switch (source->type) {
      case UINT8_TYPE:
        targ.value = source->data;
        targ.level_uval = AUINT8(source)[seed];
        break;
      case UINT16_TYPE:
        targ.value = source->data;
        targ.level_uval = AUINT16(source)[seed];
        break;
      case UINT32_TYPE:
        targ.value = source->data;
        targ.level_uval = AUINT32(source)[seed];
        break;
      case UINT64_TYPE:
        targ.value = source->data;
        targ.level_uval = AUINT64(source)[seed];
        break;
      case INT8_TYPE:
        targ.value = source->data;
        targ.level_ival = AINT8(source)[seed];
        break;
      case INT16_TYPE:
        targ.value = source->data;
        targ.level_ival = AINT16(source)[seed];
        break;
      case INT32_TYPE:
        targ.value = source->data;
        targ.level_ival = AINT32(source)[seed];
        break;
      case INT64_TYPE:
        targ.value = source->data;
        targ.level_ival = AINT64(source)[seed];
        break;
      case FLOAT32_TYPE:
        targ.value = source->data;
        targ.level_fval = AFLOAT32(source)[seed];
        break;
      case FLOAT64_TYPE:
        targ.value = source->data;
        targ.level_fval = AFLOAT64(source)[seed];
        break;
  }

  Flood_Object(source,share,iscon2n,seed,&targ,value,NULL,NULL,NULL,NULL,&parg,painter);
}

void Draw_Level_Set(Array *M(canvas), Brush_Bundle *brush, Level_Tree *t, Level_Set *r, int share)
{ APart        *source  = Get_Level_Tree_APart(t);
  Array        *array   = AForm_Array(source);
  boolean       iscon2n = Get_Level_Tree_Connectivity(t);
  Indx_Type     seed    = Level_Set_Leftmost(t,r);
  void         (*painter)(Indx_Type,void *);
  boolean      (*value)(Indx_Type,void *);
  DrawArg       targ;
  PAINT_VOID    parg;

  check_drawing_compatibility(canvas,array,"Draw_Level_Set");

  value   = Comparator_Table[6*array->type + 5];
  painter = SETUP_PAINTER(canvas,brush,&parg);

  targ.value      = array->data;
  targ.level_uval = (uint32) Level_Set_Level(t,r);

  Flood_Object(source,share,iscon2n,seed,&targ,value,NULL,NULL,NULL,NULL,&parg,painter);
}

/****************************************************************************************
 *                                                                                      *
 *  DRAWING ROUTINES FOR BASIC SHAPES                                                   *
 *                                                                                      *
 ****************************************************************************************/

static Array_Bundle *base_shape(Array *a, Array_Bundle *base)
{ base->ndims = a->ndims;
  base->dims  = a->dims;
  if (a->kind != PLAIN_KIND)
    { if (a->kind == COMPLEX_KIND)
        base->dims  += 1;
      base->ndims -= 1;
    }
  return (base);
}

void check_ivector(Coordinate *coord, Array *base, char *routine)
{ if (coord->kind != PLAIN_KIND || coord->type != DIMN_TYPE || coord->ndims != 1)
    { fprintf(stderr,"Coordinate is not a Dimn_Type vector (%s)\n",routine);
      exit (1);
    }
  if (coord->dims[0] != (Dimn_Type) base->ndims)
    { fprintf(stderr,"Coordinate is not of the correct dimensionality (%s)\n",routine);
      exit (1);
    }
}

/* Draw a size[0] x size[1] x ... x size[n-1] rectangle with lower left corner
   (corner[0],corner[1],...,corner[n-1]) where n = canvas->ndims-1.              */

typedef struct
  { Dimn_Type  *point1;
    Dimn_Type  *point2;
    Dimn_Type  *dims;
    void      (*painter)(Indx_Type,void *);
    PAINT_VOID *arg;
  } DrawRect; 

static void rectangle(int k, Indx_Type base, DrawRect *argp)
{ Dimn_Type beg = argp->point1[k];
  Dimn_Type end = argp->point2[k];
  Dimn_Type p;

  base *= argp->dims[k];
  if (k == 0)
    { for (p = beg; p <= end; p++)
        argp->painter(base+p,argp->arg);
    }
  else
    { for (p = beg; p <= end; p++)
        rectangle(k-1,base+p,argp);
    }
}

void Draw_Rectangle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(corner1),
                                                           Coordinate *F(corner2))
{ Array_Bundle base;
  int          i,   n;
  Dimn_Type   *c1, *c2, *d;
  PAINT_VOID   darg;
  DrawRect     rarg;
  Dimn_Type    Point1[10], *point1;
  Dimn_Type    Point2[10], *point2;

  base_shape(canvas,&base);

  check_ivector(corner1,&base,"Draw_Rectangle");
  check_ivector(corner2,&base,"Draw_Rectangle");

  n = base.ndims;
  d = base.dims;

  if (n > 10)
    { point1 = (Dimn_Type *) Guarded_Malloc(sizeof(Dimn_Type)*2*((size_t) n),"Draw_Rectangle");
      point2 = point1 + n;
    }
  else
    { point1 = Point1;
      point2 = Point2;
    }

  rarg.point1  = point1;
  rarg.point2  = point2;
  rarg.dims    = d;
  rarg.arg     = &darg;
  rarg.painter = SETUP_PAINTER(canvas,brush,&darg);

  c1 = ADIMN(corner1);
  c2 = ADIMN(corner2);
  for (i = 0; i < n; i++)
    { Dimn_Type beg = c1[i];
      Dimn_Type end = c2[i];
      if (beg < 0)
        beg  = 0;
      if (end >= d[i])
        end = d[i]-1;
      point1[i] = beg;
      point2[i] = end;
    }

  rectangle(n-1,0,&rarg);

  if (n > 10)
    free(point1);
  Free_Array(corner1);
  Free_Array(corner2);
}

/* Reset an entire image */

void Draw_Image(Array *M(canvas), Brush_Bundle *brush)
{ Array_Bundle base;
  int          i, n;
  Dimn_Type   *d;
  PAINT_VOID   darg;
  DrawRect     rarg;
  Dimn_Type    Point1[10], *point1;
  Dimn_Type    Point2[10], *point2;

  base_shape(canvas,&base);

  n = base.ndims;
  d = base.dims;

  if (n > 10)
    { point1 = (Dimn_Type *) Guarded_Malloc(sizeof(Dimn_Type)*2*((size_t) n),"Draw_Image");
      point2 = point1 + n;
    }
  else
    { point1 = Point1;
      point2 = Point2;
    }

  rarg.point1  = point1;
  rarg.point2  = point2;
  rarg.dims    = d;
  rarg.arg     = &darg;
  rarg.painter = SETUP_PAINTER(canvas,brush,&darg);

  for (i = 0; i < n; i++)
    { point1[i] = 0;
      point2[i] = d[i]-1;
    }

  rectangle(n-1,0,&rarg);

  if (n > 10)
    free(point1);
}

/* Draw a point centered a pixel point */

void Draw_Point(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(point))
{ Array_Bundle base;
  Dimn_Type   *p;
  int          i;
  void        (*painter)(Indx_Type,void *);
  PAINT_VOID   arg, *argp = &arg;

  base_shape(canvas,&base);

  check_ivector(point,&base,"Draw_Cross");

  p = ADIMN(point);
  for (i = 0; i < base.ndims; i++)
    if (p[i] >= base.dims[i])
      return;

  painter = SETUP_PAINTER(canvas,brush,argp);
  painter(Coord2IdxA(&base,point),argp);
}

/* Draw a cross centered at pixel center with each arm being radius pixels long */

void Draw_Cross(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius)
{ Array_Bundle base;
  int          i, n;
  Dimn_Type   *d, *c;
  Indx_Type    p, q;
  void         (*painter)(Indx_Type,void *);
  PAINT_VOID   arg, *argp = &arg;

  base_shape(canvas,&base);

  check_ivector(center,&base,"Draw_Cross");

  n = base.ndims;
  d = base.dims;

  painter = SETUP_PAINTER(canvas,brush,argp);

  q = 1;
  c = ADIMN(center);
  p = Coord2IdxA(&base,center);
  for (i = 0; i < n; i++)
    { int64 x = c[i];
      int64 b, e, k;

      if (x < radius)
        b = -x;
      else
        b = -radius;
      if (x+radius >= d[i])
        e = (d[i] - x) - 1;
      else
        e = radius;
      for (k = b; k <= e; k++)
        if (k != 0)
          painter(p+q*k,argp);
      q *= d[i];
    }
  painter(p,argp);
}

/* Draw an n-dimensional circle centered at pixel center with radius radius,
     where n = canvas->ndims                                                  */

typedef struct
  { Dimn_Type *center;
    Dimn_Type *dims;
    void      *arg;
    void     (*painter)(Indx_Type,void *);
  } DrawCirc;

static void circle(int k, Indx_Type base, int64 rem, int64 rad, DrawCirc *argp)
{ int64 i, beg, end, rng;

  while (rad*rad > rem)
    rad -= 1;

  beg = argp->center[k] - rad;
  if (beg < 0)
    beg = 0;
  end = argp->center[k] + rad;
  if (end > argp->dims[k])
    end = argp->dims[k]-1;

  base *= argp->dims[k];
  if (k == 0)
    { for (i = beg; i <= end; i++)
        argp->painter(base+i,argp->arg);
    }
  else
    { rng = beg-argp->center[k];
      for (i = beg; i <= end; i++)
        { circle(k-1,base+i,rem-rng*rng,rad,argp);
          rng += 1;
        }
    }
}

void Draw_Circle(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(center), int radius)
{ Array_Bundle base;
  PAINT_VOID   darg;
  DrawCirc     carg;

  base_shape(canvas,&base);

  check_ivector(center,&base,"Draw_Circle");

  carg.dims    = base.dims;
  carg.center  = ADIMN(center);
  carg.arg     = &darg;
  carg.painter = SETUP_PAINTER(canvas,brush,&darg);

  circle(base.ndims-1,0,radius*radius,(Dimn_Type) radius,&carg);

  Free_Array(center);
}

/*  Draw an n-dimensional line from begp to endp, clipping to the canvas as necessary */

void Draw_Line(Array *M(canvas), Brush_Bundle *brush, Coordinate *F(begp), Coordinate *F(endp))
{ int            ndims;
  Dimn_Type     *beg,  *end, *dims;
  void          (*painter)(Indx_Type,void *);
  int            kmax;
  int64          bgk, enk;
  PAINT_VOID     arg, *argp = &arg;
  double         Val[10], *val;
  double         Inc[10], *inc;

  { Array_Bundle base;

    base_shape(canvas,&base);

    check_ivector(begp,&base,"Draw_Line");
    check_ivector(endp,&base,"Draw_Line");

    painter = SETUP_PAINTER(canvas,brush,argp);

    beg  = ADIMN(begp);
    end  = ADIMN(endp);

    ndims = base.ndims;
    dims  = base.dims;

    if (ndims > 10)
      { val = (double *) Guarded_Malloc(sizeof(double)*2*((size_t) ndims),"Draw_Line");
        inc = val + ndims;
      }
    else
      { val = Val;
        inc = Inc;
      }
  }

  { Dimn_Type d, maxd = 0;           //  kmax = dimension with largest delta
    int       i;

    kmax = 0;
    for (i = 0; i < ndims; i++)
      { if (end[i] > beg[i])
          val[i] = d = end[i] - beg[i];
        else
          { d = beg[i] - end[i];
            val[i] = -1.*d;
          }
        if (d > maxd)
          { maxd = d;
            kmax = i;
          }
      }
    if (maxd == 0) goto exit_line;
  }

  { double ab, ae;
    double dkm, bkm;
    double den, dim;
    double abg, aen;
    int    i;

    ab = 0.;                   //  figure the clip interval [ab,ae] <= [0,1] for the line
    ae = 1.;
    for (i = 0; i < ndims; i++)
      { den = val[i];
        dim = dims[i] - .5;
        if (den == 0.)
          { if (beg[i] < 0 || beg[i] > dim)
              ab = 2.;
          }
        else
          { abg = - (beg[i] + .5) / den;
            aen = (dim - beg[i]) / den;
            if (abg > aen)
              { den = abg; abg = aen; aen = den; }
            if (abg > ab)
              ab = abg;
            if (aen < ae)
              ae = aen;
          }
      }

    bkm = beg[kmax];        //  then further refine to have integral start and end poinnts,
    dkm = val[kmax];        //    bgk & end, for dimension kmax
    if (dkm > 0.)
      { enk = (int64) (bkm + ae * dkm);
        bgk = (int64) ceil(bkm + ab * dkm);
      }
    else
      { bgk = (int64) (bkm + ab * dkm);
        enk = (int64) ceil(bkm + ae * dkm);
      }
    ab = (bgk - bkm) / dkm;
    ae = (enk - bkm) / dkm;

    if (ab > ae)
      goto exit_line;

    dkm = fabs(dkm);
    for (i = 0; i < ndims; i++)   // compute clipped start points and increments for every dimension
      { den = val[i];
        inc[i] = den / dkm;
        if (i == kmax)
          val[i] = (double) bgk;
        else
          val[i] = beg[i] + ab * den + .5;
      }
  }

  { int       k, step;
    int64     i;
    Indx_Type p;

    if (bgk <= enk)
      step = 1;
    else
      step = -1;
    enk += step;
    for (i = bgk; i != enk; i += step)  //  for each integer along dimension kmax,
      { k = ndims-1;                    //    paint the nearest pixel
        p = (Indx_Type) val[k];
        val[k] += inc[k];
        for (k--; k >= 0; k--)
          { p = p * dims[k] + ((int) val[k]);
            val[k] += inc[k];
          }
        painter(p,argp);
      }
  }

exit_line:
  if (ndims > 10)
    free(val);
  Free_Array(begp);
  Free_Array(endp);
}
