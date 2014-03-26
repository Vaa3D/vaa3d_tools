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

/*****************************************************************************************\
*                                                                                         *
*  FFT algorithms including convolution and correlation                                   *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2007                                                                   *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "utilities.h"
#include "array.h"
#include "fft.h"

#include "MY_FFT/fft.F.h"   //  All the real functionality is in the fft.F.[ch], fft.D.[ch] codes
#include "MY_FFT/fft.D.h"   //  This file just interfaces to the array data type of our library

#define ACOMPLEX32(x) ((Complex_f *) x->data)
#define ACOMPLEX64(x) ((Complex_d *) x->data)

Dimn_Type Power_Of_2_Pad(Dimn_Type m)
{ return (Next_Power_Of_2(m)); }

#define E(R) { if (R == NULL) goto error; }

void check_power(Array *data, char *routine)
{ int i;

  for (i = 0; i < data->ndims; i++)
    if (Next_Power_Of_2(data->dims[i]) != data->dims[i])
      { fprintf(stderr,"dims[%d] = %u is not a power of 2 (%s)\n",i,data->dims[i],routine);
        exit (1);
      }
}

void check_even(Array *data, char *routine)
{ int i;

  for (i = 0; i < data->ndims; i++)
    if (data->dims[i] % 2)
      { fprintf(stderr,"dims[%d] = %u must be even (%s)\n",i,data->dims[i],routine);
        exit (1);
      }
}

Complex_Array *FFT(Complex_Array *R(M(data)), boolean invert)
{ char *s;

  if (data->type < FLOAT32_TYPE)
    { fprintf(stderr,"FFT: Array must be float or double\n");
      exit (1);
    }
  if (data->kind == PLAIN_KIND)
    { check_power(data,"FFT");
      if (data->size == 1)
        return (data);
      if (invert)
        { if (data->type == FLOAT32_TYPE)
            if (data->ndims == 1)
              E(Real_FFT_Inverse_1f(data->dims[0],ACOMPLEX32(data)))
            else
              E(Real_FFT_Inverse_nf(data->ndims,data->dims,ACOMPLEX32(data)))
          else
            if (data->ndims == 1)
              E(Real_FFT_Inverse_1d(data->dims[0],ACOMPLEX64(data)))
            else
              E(Real_FFT_Inverse_nd(data->ndims,data->dims,ACOMPLEX64(data)))
          return (data);
        }
      else
        { if (data->type == FLOAT32_TYPE)
            if (data->ndims == 1)
              E(Real_FFT_1f(data->dims[0],AFLOAT32(data)))
            else
              E(Real_FFT_nf(data->ndims,data->dims,AFLOAT32(data)))
          else
            if (data->ndims == 1)
              E(Real_FFT_1d(data->dims[0],AFLOAT64(data)))
            else
              E(Real_FFT_nd(data->ndims,data->dims,AFLOAT64(data)))
          return (data);
        }
    }
  if (data->kind != COMPLEX_KIND)
    { fprintf(stderr,"FFT: Array is not complex or numeric\n");
      exit (1);
    }
  check_power(data,"FFT");
  if (data->size == 2)
    return (data);
  else if (data->type == FLOAT32_TYPE)
    if (data->ndims == 2)
      E(FFT_1f(data->dims[1],ACOMPLEX32(data),invert))
    else
      E(FFT_nf(data->ndims-1,data->dims+1,ACOMPLEX32(data),invert))
  else
    if (data->ndims == 2)
      E(FFT_1d(data->dims[1],ACOMPLEX64(data),invert))
    else
      E(FFT_nd(data->ndims-1,data->dims+1,ACOMPLEX64(data),invert))
  return (data);

error:
  s = FFT_Error_String();
  if (s == NULL)
    fprintf(stderr,"Limit exceeded (FFT)\n");
  else
    { fprintf(stderr,"%s (FFT)\n",s);
      FFT_Error_Release();
    }
  exit (1);
}

Complex_Array *FFT_Convolution(Complex_Array *R(M(fft1)), Complex_Array *fft2)
{ char *s;
 
  if ( ! Same_Type(fft1,fft2))
    { fprintf(stderr,"FFT_Convolution: Arrays must have the same shape and type\n");
      exit (1);
    }
  if (fft1->type < FLOAT32_TYPE)
    { fprintf(stderr,"FFT_Convolution: Arrays must be float or double\n");
      exit (1);
    }

  if (fft1->size == 1)
    { if (fft1->type == FLOAT32_TYPE)
        AFLOAT32(fft1)[0] *= AFLOAT32(fft2)[0];
      else
        AFLOAT64(fft1)[0] *= AFLOAT64(fft2)[0];
      return (fft1);
    }

  if ((fft1->kind != COMPLEX_KIND && fft1->kind != PLAIN_KIND) ||
      (fft2->kind != COMPLEX_KIND && fft2->kind != PLAIN_KIND))
    { fprintf(stderr,"FFT_Convolution: Arrays must be complex or real\n");
      exit (1);
    }
  if (fft1->kind != fft2->kind)
    { fprintf(stderr,"FFF_Convolution: Arrays must both be FFT's of real or of complex data\n");
      exit (1);
    }

  if (fft1->kind == PLAIN_KIND)
    { check_even(fft1,"FFT_Convolution");
      if (fft1->type == FLOAT32_TYPE)
        if (fft1->ndims == 1)
          E(Real_Convolution_1f(fft1->dims[0],ACOMPLEX32(fft1),ACOMPLEX32(fft2)))
        else
          E(Real_Convolution_nf(fft1->ndims,fft1->dims,ACOMPLEX32(fft1),ACOMPLEX32(fft2)))
      else
        if (fft1->ndims == 1)
          E(Real_Convolution_1d(fft1->dims[0],ACOMPLEX64(fft1),ACOMPLEX64(fft2)))
        else
          E(Real_Convolution_nd(fft1->ndims,fft1->dims,ACOMPLEX64(fft1),ACOMPLEX64(fft2)))
    }
  else
    { if (fft1->type == FLOAT32_TYPE)
        if (fft1->ndims == 2)
          Complex_Convolution_1f(fft1->dims[1],ACOMPLEX32(fft1),ACOMPLEX32(fft2));
        else
          Complex_Convolution_nf(fft1->ndims-1,fft1->dims+1,ACOMPLEX32(fft1),ACOMPLEX32(fft2));
      else
        if (fft1->ndims == 2)
          Complex_Convolution_1d(fft1->dims[1],ACOMPLEX64(fft1),ACOMPLEX64(fft2));
        else
          Complex_Convolution_nd(fft1->ndims-1,fft1->dims+1,ACOMPLEX64(fft1),ACOMPLEX64(fft2));
    }
  return (fft1);

error:
  s = FFT_Error_String();
  if (s == NULL)
    fprintf(stderr,"Limit exceeded (FFT_Convolution)\n");
  else
    { fprintf(stderr,"%s (FFT_Convolution)\n",s);
      FFT_Error_Release();
    }
  exit (1);
}

Complex_Array *FFT_Correlation(Complex_Array *R(M(fft1)), Complex_Array *fft2)
{ char *s;

  if ( ! Same_Type(fft1,fft2))
    { fprintf(stderr,"FFT_Correlation: Arrays must have the same shape and type\n");
      exit (1);
    }
  if (fft1->type < FLOAT32_TYPE)
    { fprintf(stderr,"FFT_Correlation: Arrays must be float or double\n");
      exit (1);
    }

  if (fft1->size == 1)
    { if (fft1->type == FLOAT32_TYPE)
        AFLOAT32(fft1)[0] *= AFLOAT32(fft2)[0];
      else
        AFLOAT64(fft1)[0] *= AFLOAT64(fft2)[0];
      return (fft1);
    }

  if ((fft1->kind != COMPLEX_KIND && fft1->kind != PLAIN_KIND) ||
      (fft2->kind != COMPLEX_KIND && fft2->kind != PLAIN_KIND))
    { fprintf(stderr,"FFT_Convolution: Arrays must be complex or real\n");
      exit (1);
    }
  if (fft1->kind != fft2->kind)
    { fprintf(stderr,"FFF_Correlation: Arrays must both be FFT's of real or of complex data\n");
      exit (1);
    }

  if (fft1->kind == PLAIN_KIND)
    { check_even(fft1,"FFT_Correlation");
      if (fft1->type == FLOAT32_TYPE)
        if (fft1->ndims == 1)
          E(Real_Correlation_1f(fft1->dims[0],ACOMPLEX32(fft1),ACOMPLEX32(fft2)))
        else
          E(Real_Correlation_nf(fft1->ndims,fft1->dims,ACOMPLEX32(fft1),ACOMPLEX32(fft2)))
      else
        if (fft1->ndims == 1)
          E(Real_Correlation_1d(fft1->dims[0],ACOMPLEX64(fft1),ACOMPLEX64(fft2)))
        else
          E(Real_Correlation_nd(fft1->ndims,fft1->dims,ACOMPLEX64(fft1),ACOMPLEX64(fft2)))
    }
  else
    { if (fft1->type == FLOAT32_TYPE)
        if (fft1->ndims == 2)
          Complex_Correlation_1f(fft1->dims[1],ACOMPLEX32(fft1),ACOMPLEX32(fft2));
        else
          Complex_Correlation_nf(fft1->ndims-1,fft1->dims+1,ACOMPLEX32(fft1),ACOMPLEX32(fft2));
      else
        if (fft1->ndims == 2)
          Complex_Correlation_1d(fft1->dims[1],ACOMPLEX64(fft1),ACOMPLEX64(fft2));
        else
          Complex_Correlation_nd(fft1->ndims-1,fft1->dims+1,ACOMPLEX64(fft1),ACOMPLEX64(fft2));
    }
  return (fft1);

error:
  s = FFT_Error_String();
  if (s == NULL)
    fprintf(stderr,"Limit exceeded (FFT_Correlation)\n");
  else
    { fprintf(stderr,"%s (FFT_Correlation)\n",s);
      FFT_Error_Release();
    }
  exit (1);
}

Numeric_Array *Normalize_FFT_Correlation(Numeric_Array *ref1, Numeric_Array *ref2,
                                         Numeric_Array *R(M(cor)))
{ int   i;
  char *s;

  if (ref1->kind != PLAIN_KIND || ref1->type < FLOAT32_TYPE)
    { fprintf(stderr,"Normalize_Correlation: ref1 is not a numeric array\n");
      exit (1);
    }
  if (ref2->kind != PLAIN_KIND || ref2->type < FLOAT32_TYPE)
    { fprintf(stderr,"Normalize_Correlation: ref2 is not a numeric array\n");
      exit (1);
    }
  if (cor->kind != PLAIN_KIND || cor->type < FLOAT32_TYPE)
    { fprintf(stderr,"Normalize_Correlation: correlation matrix is not a numeric array\n");
      exit (1);
    }
  if (ref1->ndims != ref2->ndims || ref1->ndims != cor->ndims)
    { fprintf(stderr,"Normalize_Correlation: arrays do not have the same dimensionality\n");
      exit (1);
    }
  if (ref1->type != ref2->type || ref1->type != cor->type)
    { fprintf(stderr,"Normalize_Correlation: All inputs must be float or double and not a mix\n");
      exit (1);
    }
  for (i = 0; i < ref1->ndims; i++)
    if (ref1->dims[i] + ref2->dims[i] > cor->dims[i])
      { fprintf(stderr,"Normalize_Correlation: Correlation matrix is not properly padded\n");
        exit (1);
      }
  if (ref1->type == FLOAT32_TYPE)
    E(Normalize_nf(ref1->ndims,ref1->dims,ref1->data,ref2->dims,ref2->data,cor->dims,cor->data))
  else
    E(Normalize_nd(ref1->ndims,ref1->dims,ref1->data,ref2->dims,ref2->data,cor->dims,cor->data))
  return (cor);

error:
  s = FFT_Error_String();
  if (s == NULL)
    fprintf(stderr,"Limit exceeded (Normalize_FFT_Correlation)\n");
  else
    { fprintf(stderr,"%s (Normalize_FFT_Correlation)\n",s);
      FFT_Error_Release();
    }
  exit (1);
}
