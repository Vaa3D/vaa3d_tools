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
*  Image and Image Stack Data Abstraction for TIF-encoded files                           *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  August 2006                                                                   *
*  Mods  :  November 2007: added idea of text description and read/write to               *
*              TIFFTAG_IMAGEDESCRIPTION tag of tif format                                 *
*           May 2008: Replaced libtiff with my own tiff library, necessitating            *
*              the introduction of a Tiff data type, and the addition of Advance_Tiff     *
*              and Tiff_EOF to the abstraction (cleaner than before).                     *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mylib.h"
#include "array.h"
#include "MY_TIFF/tiff.io.h"
#include "MY_TIFF/tiff.image.h"
#include "image.h"

#ifdef __APPLE__

#include <malloc/malloc.h>

#else

#include <malloc.h>

#endif

#ifdef _MSC_VER

#define  malloc_size _msize

#elif defined __APPLE__
// Mac
// no action, Mac is the only platform with an actual malloc_size() method
#else
// Linux
#define  malloc_size malloc_usable_size

#endif

/****************************************************************************************
 *                                                                                      *
 *  SPACE MANAGEMENT AND HELPER ROUTINES                                                *
 *                                                                                      *
 ****************************************************************************************/

static void *Guarded_Realloc(void *p, size_t size, char *routine)  //  Redefine behavior of said
{ (void) routine;
  p = realloc(p,size);
  return (p);
}

typedef struct
  { Tiff_Reader *reader;
    Tiff_Writer *writer;
    Tiff_IFD    *ifd;
    Tiff_Image  *img;
    string       text;
    boolean      eof;
  } Tio;


typedef struct __Tio
  { struct __Tio *next;
    struct __Tio *prev;
    int           refcnt;
    Tio           tio;
  } _Tio;

static _Tio *Free_Tio_List = NULL;
static _Tio *Use_Tio_List  = NULL;

static pthread_mutex_t Tio_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int Tio_Offset = sizeof(_Tio)-sizeof(Tio);
static int Tio_Inuse  = 0;

int Tiff_Refcount(Tiff *tiff)
{ _Tio *object = (_Tio *) (((char *) tiff) - Tio_Offset);
  return (object->refcnt);
}

static inline void kill_tio(Tio *tio);

static inline Tio *new_tio(char *routine)
{ _Tio *object;
  Tio  *tio;

  pthread_mutex_lock(&Tio_Mutex);
  if (Free_Tio_List == NULL)
    { object = (_Tio *) Guarded_Realloc(NULL,sizeof(_Tio),routine);
      if (object == NULL) return (NULL);
      tio = &(object->tio);
    }
  else
    { object = Free_Tio_List;
      Free_Tio_List = object->next;
      tio = &(object->tio);
    }
  Tio_Inuse += 1;
  object->refcnt = 1;
  if (Use_Tio_List != NULL)
    Use_Tio_List->prev = object;
  object->next = Use_Tio_List;
  object->prev = NULL;
  Use_Tio_List = object;
  pthread_mutex_unlock(&Tio_Mutex);
  tio->reader = NULL;
  tio->writer = NULL;
  tio->ifd = NULL;
  tio->img = NULL;
  return (tio);
}

static inline Tio *copy_tio(Tio *tio)
{ Tio *copy = new_tio("Copy_Tiff");
  *copy = *tio;
  if (tio->reader != NULL)
    copy->reader = Copy_Tiff_Reader(tio->reader);
  if (tio->writer != NULL)
    copy->writer = Copy_Tiff_Writer(tio->writer);
  if (tio->ifd != NULL)
    copy->ifd = Copy_Tiff_IFD(tio->ifd);
  if (tio->img != NULL)
    copy->img = Copy_Tiff_Image(tio->img);
  return (copy);
}

Tiff *Copy_Tiff(Tiff *tiff)
{ return ((Tiff *) copy_tio(((Tio *) tiff))); }

static inline int pack_tio(Tio *tio)
{
  if (tio->reader != NULL)
    if (Pack_Tiff_Reader(tio->reader) == NULL) return (1);
  if (tio->writer != NULL)
    if (Pack_Tiff_Writer(tio->writer) == NULL) return (1);
  if (tio->ifd != NULL)
    if (Pack_Tiff_IFD(tio->ifd) == NULL) return (1);
  if (tio->img != NULL)
    if (Pack_Tiff_Image(tio->img) == NULL) return (1);
  return (0);
}

Tiff *Pack_Tiff(Tiff *tiff)
{ if (pack_tio(((Tio *) tiff))) return (NULL);
  return (tiff);
}

Tiff *Inc_Tiff(Tiff *tiff)
{ _Tio *object  = (_Tio *) (((char *) tiff) - Tio_Offset);
  pthread_mutex_lock(&Tio_Mutex);
  object->refcnt += 1;
  pthread_mutex_unlock(&Tio_Mutex);
  return (tiff);
}

static inline void free_tio(Tio *tio)
{ _Tio *object  = (_Tio *) (((char *) tio) - Tio_Offset);
  pthread_mutex_lock(&Tio_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Tio_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Freeing previously released Tiff\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Tio_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  object->next = Free_Tio_List;
  Free_Tio_List = object;
  Tio_Inuse -= 1;
  pthread_mutex_unlock(&Tio_Mutex);
  if (tio->img != NULL)
    { Free_Tiff_Image(tio->img);
      tio->img = NULL;
    }
  if (tio->ifd != NULL)
    { Free_Tiff_IFD(tio->ifd);
      tio->ifd = NULL;
    }
  if (tio->writer != NULL)
    { Free_Tiff_Writer(tio->writer);
      tio->writer = NULL;
    }
  if (tio->reader != NULL)
    { Free_Tiff_Reader(tio->reader);
      tio->reader = NULL;
    }
}

void Free_Tiff(Tiff *tiff)
{ free_tio(((Tio *) tiff)); }

static inline void kill_tio(Tio *tio)
{ _Tio *object  = (_Tio *) (((char *) tio) - Tio_Offset);
  pthread_mutex_lock(&Tio_Mutex);
  if (--object->refcnt > 0)
    { pthread_mutex_unlock(&Tio_Mutex);
      return;
    }
  if (object->refcnt < 0)
    fprintf(stderr,"Warning: Killing previously released Tiff\n");
  if (object->prev != NULL)
    object->prev->next = object->next;
  else
    Use_Tio_List = object->next;
  if (object->next != NULL)
    object->next->prev = object->prev;
  Tio_Inuse -= 1;
  pthread_mutex_unlock(&Tio_Mutex);
  if (tio->img != NULL)
    Kill_Tiff_Image(tio->img);
  if (tio->ifd != NULL)
    Kill_Tiff_IFD(tio->ifd);
  if (tio->writer != NULL)
    Kill_Tiff_Writer(tio->writer);
  if (tio->reader != NULL)
    Kill_Tiff_Reader(tio->reader);
  free(((char *) tio) - Tio_Offset);
}

void Kill_Tiff(Tiff *tiff)
{ kill_tio(((Tio *) tiff)); }

static inline void reset_tio()
{ _Tio *object;
  pthread_mutex_lock(&Tio_Mutex);
  while (Free_Tio_List != NULL)
    { object = Free_Tio_List;
      Free_Tio_List = object->next;
      free(object);
    }
  pthread_mutex_unlock(&Tio_Mutex);
}

void Reset_Tiff()
{ reset_tio(); }

int Tiff_Usage()
{ return (Tio_Inuse); }

void Tiff_List(void (*handler)(Tiff *))
{ _Tio *a, *b;
  for (a = Use_Tio_List; a != NULL; a = b)
    { b = a->next;
      handler((Tiff *) &(a->tio));
    }
}

  //  If an image routine encounters an exception it trys to grab the "error resource" with
  //    grab_message and if successfull then sets the global string Image_Estring
  //    before returning NULL.  A user can subsequently get this string with Image_Error()
  //    and release the "error resource" with a call to Image_Error_Release().

static pthread_mutex_t Image_Err_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int        Image_Error_Flag = 0;
static char       Image_Estring[1000];
static pthread_id Image_Ethread;

static int grab_message()
{ int ret;
  pthread_mutex_lock(&Image_Err_Mutex);
  if (Image_Error_Flag)
    ret = 0;
  else
    { Image_Error_Flag = 1;
      Image_Ethread    = pthread_tag();
      ret = 1;
    }
  pthread_mutex_unlock(&Image_Err_Mutex);
  return (ret);
}

string Image_Error()
{ char *ret;
  pthread_mutex_lock(&Image_Err_Mutex);
  if (Image_Error_Flag && pthread_is_this(Image_Ethread))
    ret = Image_Estring;
  else
    ret = NULL;
  pthread_mutex_unlock(&Image_Err_Mutex);
  return (ret);
}

void Image_Error_Release()
{ pthread_mutex_lock(&Image_Err_Mutex);
  if (Image_Error_Flag && pthread_is_this(Image_Ethread))
    Image_Error_Flag = 0;
  pthread_mutex_unlock(&Image_Err_Mutex);
}

static int type_size[] = { 1, 2, 4, 8, 1, 2, 4, 8, 4, 8 };

static int kind_size[] = { 1, 3, 4, 2 };

static char Empty_String[1] = { 0 };

static Value_Type determine_type(Tiff_Image *img, int cidx) // Determine type of channel cidx of img
{ Tiff_Channel *chan;

  chan = img->channels[cidx];
  if (chan->type == CHAN_FLOAT)
    return (FLOAT32_TYPE);
  else if (chan->type == CHAN_SIGNED)
    { if (chan->scale > 16)
        return (INT32_TYPE);
      else if (chan->scale > 8)
        return (INT16_TYPE);
      else
        return (INT8_TYPE);
    }
  else
    { if (chan->scale > 16)
        return (UINT32_TYPE);
      else if (chan->scale > 8)
        return (UINT16_TYPE);
      else
        return (UINT8_TYPE);
    }
}

static Array_Kind determine_kind(Tiff_Image *img, int cidx)   //  Determine kind of layer of img
{ if (img->number_channels >= cidx+3)                         //    starting at channel cidx
    { int i, n, s, t;

      if (img->number_channels >= cidx+4)
        n = cidx+4;
      else
        n = cidx+3;
      s = t = 0;
      for (i = cidx; i < n; i++)
        { if (img->channels[i]->scale != img->channels[cidx]->scale ||
              img->channels[i]->type  != img->channels[cidx]->type)
            break;
          if (img->channels[i]->interpretation == CHAN_RED)
            s += 1;
          else if (img->channels[i]->interpretation == CHAN_GREEN)
            s += 2;
          else if (img->channels[i]->interpretation == CHAN_BLUE)
            s += 4;
          else if (img->channels[i]->interpretation == CHAN_ALPHA)
            s += 8;
          if (i == cidx+2) t = s;
        }
      if (s == 15)
        return (RGBA_KIND);
      else if (t == 7)
        return (RGB_KIND);
    }
  return (PLAIN_KIND);
}

static int channel_order(Tiff_Channel *chan)   //  Map channel interpretation to array index
{ switch (chan->interpretation)
  { case CHAN_RED:
      return (0);
    case CHAN_GREEN:
      return (1);
    case CHAN_BLUE:
      return (2);
    case CHAN_ALPHA:
      return (3);
    default:
      return (0);
  }
}


/****************************************************************************************
 *                                                                                      *
 *  TIFF ROUTINES: FOR BOTH READ & WRITE                                                *
 *                                                                                      *
 ****************************************************************************************/

Tiff *G(Open_Tiff)(const char * file_name, const char * mode)
{ Tio *tif;

  tif = new_tio("Open_Tiff");
  if (tif == NULL)
    { if (grab_message())
        sprintf(Image_Estring,"Out of memory (Open_Tiff)");
      return (NULL);
    }

  tif->reader = NULL;
  tif->writer = NULL;
  tif->ifd    = NULL;
  tif->img    = NULL;

  if (strcmp(mode,"r") == 0)
    { tif->reader = Open_Tiff_Reader(file_name,NULL,NULL,
                                     strcmp(file_name+(strlen(file_name)-4),".lsm") == 0);
      if (tif->reader == NULL)
        { string es = Tiff_Error_String();
          if (es != NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Error reading Tif: '%s' (Open_Tiff)",es);
              Tiff_Error_Release();
            }
          kill_tio(tif);
          return (NULL);
        }
      tif->eof    = End_Of_Tiff(tif->reader);
    }
  else if (strcmp(mode,"w") == 0)
    { tif->writer = Open_Tiff_Writer(file_name,0,0);
      tif->eof    = 1;
    }
  else
    { if (grab_message())
        sprintf(Image_Estring,"Mode must be either 'r' or 'w' (Open_Tiff)");
      kill_tio(tif);
      return (NULL);
    }

  return ((Tiff *) tif);
}

void Advance_Tiff(RTiff *M(etif))
{ Tio *tif = (Tio *) etif;

  if (tif->reader == NULL)
    { fprintf(stderr,"Trying to read a Tiff open for writing (Advance_Tiff)\n");
      exit (1);
    }

  if (tif->eof) return;
  if (tif->ifd == NULL)
    Advance_Tiff_Reader(tif->reader);
  else
    { Free_Tiff_Image(tif->img);
      Free_Tiff_IFD(tif->ifd);
      tif->ifd = NULL;
      tif->img = NULL;
    }
  tif->eof = End_Of_Tiff(tif->reader);
}

void Update_Tiff(WTiff *M(etif), Image_Compress compress)
{ Tio *tif = (Tio *) etif;

  if (tif->writer == NULL)
    { fprintf(stderr,"Trying to write a Tiff open for reading (Update_Tiff)\n");
      exit (1);
    }

  if (tif->img == NULL) return;
  if (compress == DONT_PRESS)
    tif->ifd = Make_IFD_For_Image(tif->img,DONT_COMPRESS,0,0);
  else if (compress == LZW_PRESS)
    tif->ifd = Make_IFD_For_Image(tif->img,LZW_COMPRESS,0,0);
  else
    tif->ifd = Make_IFD_For_Image(tif->img,PACKBITS_COMPRESS,0,0);
  if (tif->text != NULL)
    { if (Set_Tiff_Tag(tif->ifd,TIFF_JF_ANO_BLOCK,TIFF_BYTE,(int) (strlen(tif->text))+1,tif->text))
        { fprintf(stderr,"Out of memory (Update_Tiff)\n");
          exit (1);
        }
    }
  Write_Tiff_IFD(tif->writer,tif->ifd);
  Free_Tiff_Image(tif->img);
  Free_Tiff_IFD(tif->ifd);
  tif->img = NULL;
  tif->ifd = NULL;
}

void Rewind_Tiff(Tiff *M(etif))
{ Tio *tif = (Tio *) etif;

  if (tif->reader == NULL)
    { fprintf(stderr,"Trying to read a Tiff open for writing (Advance_Tiff)\n");
      exit (1);
    }

  if (tif->ifd != NULL)
    { Free_Tiff_Image(tif->img);
      Free_Tiff_IFD(tif->ifd);
      tif->ifd = NULL;
      tif->img = NULL;
    }
  Rewind_Tiff_Reader(tif->reader);
  tif->eof = End_Of_Tiff(tif->reader);
}

boolean Tiff_EOF(Tiff *tif)
{ return (((Tio *) tif)->eof); }

void Close_Tiff(Tiff *F(etif))
{ Free_Tiff(etif); }


/****************************************************************************************
 *                                                                                      *
 *  TIFF ROUTINES: FOR READING ONLY                                                     *
 *                                                                                      *
 ****************************************************************************************/

boolean load_IFD(Tio *tif, char *routine)
{ if (tif->eof)
    { if (grab_message())
        sprintf(Image_Estring,"Trying to access an IFD when at end of file (%s)",routine);
       return (1);
    }
  if (tif->reader == NULL)
    { fprintf(stderr,"Trying to read a Tiff open for writing (%s)\n",routine);
      exit (1);
    }
  if (tif->ifd == NULL)
    { tif->ifd = Read_Tiff_IFD(tif->reader);
      if (tif->ifd == NULL)
        { string es = Tiff_Error_String();
          if (es != NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Error reading Tif IFD: '%s' (%s)",es,routine);
              Tiff_Error_Release();
            }
          return (1);
        }
      tif->img = Get_Tiff_Image(tif->ifd);
      if (tif->img == NULL)
        { string es = Tiff_Error_String();
          if (es != NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Error reading Tif Image: '%s' (%s)",es,routine);
              Tiff_Error_Release();
            }
          return (1);
        }
    }
  return (0);
}

static Value_Type UnsignType[4] = { UINT8_TYPE, UINT16_TYPE, UINT32_TYPE, UINT32_TYPE };
static Value_Type SignType[4]   = { INT8_TYPE, INT16_TYPE, INT32_TYPE, INT32_TYPE };

static Channel_Kind ChanMap[] = { PLAIN_CHAN, PLAIN_CHAN, MAPPED_CHAN, RED_CHAN, GREEN_CHAN,
                                  BLUE_CHAN, ALPHA_CHAN, PLAIN_CHAN, PLAIN_CHAN, PLAIN_CHAN };

boolean Get_IFD_Shape(Tiff *etif, int *width, int *height, int *channels)
{ Tio *tif = (Tio *) etif;

  if (load_IFD(tif,"Get_IFD_Shape"))
    return (1);
  *width    = tif->img->width;
  *height   = tif->img->height;
  *channels = tif->img->number_channels;
  return (0);
}

Value_Type Get_IFD_Channel_Type(Tiff *etif, int channel)
{ Tio       *tif   = (Tio *) etif;
  Value_Type type;

  if (load_IFD(tif,"Get_IFD_Channel_Types"))
    return (-1);
  if (channel < 0 || channel >= tif->img->number_channels)
    { if (grab_message())
        sprintf(Image_Estring,"Channel number not valid (Get_IFD_Channel_Type)");
       return (-1);
    }

  switch (tif->img->channels[channel]->type)
  { case CHAN_UNSIGNED:
      type = UnsignType[tif->img->channels[channel]->bytes_per_pixel-1];
      break;
    case CHAN_SIGNED:
      type = SignType[tif->img->channels[channel]->bytes_per_pixel-1];
      break;
    case CHAN_FLOAT:
    default:
      type = FLOAT32_TYPE;
      break;
    }
  return (type);
}

Channel_Kind Get_IFD_Channel_Kind(Tiff *etif, int channel)
{ Tio *tif   = (Tio *) etif;

  if (load_IFD(tif,"Get_IFD_Channel_Kind"))
    return (-1);
  if (channel < 0 || channel >= tif->img->number_channels)
    { if (grab_message())
        sprintf(Image_Estring,"Channel number not valid (Get_IFD_Channel_Kind)");
       return (-1);
    }

  return (ChanMap[tif->img->channels[channel]->interpretation]);
}

string Get_IFD_Annotation(Tiff *etif)
{ Tio *tif   = (Tio *) etif;
  Tiff_Type targ;
  string    text;
  int       count;

  if (load_IFD(tif,"Get_IFD_Annotation"))
    return (NULL);

  if ((text = (string) Get_Tiff_Tag(tif->ifd,TIFF_JF_ANO_BLOCK,&targ,&count)) == NULL)
    return (Empty_String);
  return (text);
}

boolean Get_IFD_Channel(Tiff *etif, int channel, Array *M(plane))
{ Tio       *tif = (Tio *) etif;
  Value_Type type;
  int        i;
  void     **planes, *Planes[16];

  if (load_IFD(tif,"Get_IFD_Channel"))
    return (1);

  if (channel < 0 || channel >= tif->img->number_channels)
    { if (grab_message())
        sprintf(Image_Estring,"Channel number not valid (Get_IFD_Channel)");
       return (1);
    }

  switch (tif->img->channels[channel]->type)
  { case CHAN_UNSIGNED:
      type = UnsignType[tif->img->channels[channel]->bytes_per_pixel-1];
      break;
    case CHAN_SIGNED:
      type = SignType[tif->img->channels[channel]->bytes_per_pixel-1];
      break;
    case CHAN_FLOAT:
    default:
      type = FLOAT32_TYPE;
      break;
  }

  if (plane->type != type || plane->kind != PLAIN_KIND || plane->ndims != 2 ||
      plane->dims[0] != tif->img->width || plane->dims[1] != tif->img->height)
    { if (grab_message())
        sprintf(Image_Estring,
               "Plane not a 2D PLAIN array of the right type and shape (Get_IFD_Channel)");
       return (1);
    }

  if (tif->img->number_channels > 16)
    { planes = (void **)
                Guarded_Realloc(NULL,sizeof(void *)*((size_t) tif->img->number_channels),
                                  "Get_IFD_Channel");
      if (planes == NULL)
        { if (grab_message())
            sprintf(Image_Estring,"Out of memory (Get_IFD_Channel)");
          return (1);
        }
    }
  else
    planes = Planes;
  for (i = 0; i < tif->img->number_channels; i++)
    planes[i] = NULL;
  planes[channel] = plane->data;

  if (Load_Tiff_Image_Planes(tif->img,planes))
    { string es = Tiff_Error_String();
      if (es != NULL)
        { if (grab_message())
            sprintf(Image_Estring,"Error reading Tif Image: '%s' (Get_Tiff_Channel)",es);
          Tiff_Error_Release();
        }
      planes[channel] = NULL;
      return (1);
    }

  plane->scale = tif->img->channels[channel]->scale;
  return (0);
}

boolean Get_IFD_Map(Tiff *etif, Array *M(map))
{ Tio *tif = (Tio *) etif;
  int  dom;

  if (load_IFD(tif,"Get_IFD_Channel"))
    return (1);

  if (tif->img->channels[0]->interpretation != CHAN_MAPPED)
    { if (grab_message())
        sprintf(Image_Estring,"1st channel of IFD is not MAPPED (Get_IFD_Map)");
       return (1);
    }

  dom = (1 << tif->img->channels[0]->scale);

  if (map->type != UINT16_TYPE || map->kind != RGB_KIND ||
      map->ndims != 1 || map->dims[0] != dom)
    { if (grab_message())
        sprintf(Image_Estring,"Map not a 1D RGB array of the right length (Get_IFD_Map)");
       return (1);
    }

  memcpy(map->data,tif->img->map,(size_t) (map->size*type_size[UINT16_TYPE]));

  return (0);
}


/****************************************************************************************
 *                                                                                      *
 *  TIFF ROUTINES: FOR WRITING ONLY                                                     *
 *                                                                                      *
 ****************************************************************************************/

static Channel_Meaning MapChan[] = { CHAN_BLACK, CHAN_MAPPED, CHAN_RED, CHAN_GREEN,
                                     CHAN_BLUE, CHAN_ALPHA };

static Channel_Type TypeChan[] = { CHAN_UNSIGNED, CHAN_UNSIGNED, CHAN_UNSIGNED, CHAN_UNSIGNED,
                                   CHAN_SIGNED, CHAN_SIGNED, CHAN_SIGNED, CHAN_SIGNED,
                                   CHAN_FLOAT };

boolean Add_IFD_Channel(WTiff *M(etif), Array *channel, Channel_Kind kind)
{ Tio *tif = (Tio *) etif;

  if (channel->type == FLOAT64_TYPE)
    { if (grab_message())
        sprintf(Image_Estring,"Channel cannot be of type FLOAT64 (Add_IFD_Channel)");
       return (1);
    }
  if (channel->kind != PLAIN_KIND || channel->ndims != 2)
    { if (grab_message())
        sprintf(Image_Estring,"Channel is not a PLAIN 2D array (Add_IFD_Channel)");
       return (1);
    }
   
  if (tif->img == NULL)
    tif->img = Create_Tiff_Image(channel->dims[0],channel->dims[1]);

  if (channel->dims[0] != tif->img->width || channel->dims[1] != tif->img->height)
    { if (grab_message())
        sprintf(Image_Estring,"Channel dimensions don't match IFD's (Add_IFD_Channel)");
       return (1);
    }
  
  Add_Tiff_Image_Channel(tif->img,MapChan[kind],channel->scale,
                                  TypeChan[channel->type],channel->data);

  if (tif->img->number_channels == 1 && channel->text[0] != '\0')
    tif->text = channel->text;
  else
    tif->text = NULL;

  return (0);
}

boolean Set_IFD_Map(WTiff *M(etif), Array *map)
{ Tio *tif = (Tio *) etif;
  int  dom;

  if (tif->img == NULL)
    { if (grab_message())
        sprintf(Image_Estring,"No channels in the current IFD (Set_IFD_Map)");
       return (1);
    }

  if (tif->img->channels[0]->interpretation != CHAN_MAPPED)
    { if (grab_message())
        sprintf(Image_Estring,"First channel is not MAPPED (Set_IFD_Map)");
       return (1);
    }

  dom = (1 << tif->img->channels[0]->scale);

  if (map->type != UINT16_TYPE || map->kind != RGB_KIND ||
      map->ndims != 1 || map->dims[0] != dom)
    { if (grab_message())
        sprintf(Image_Estring,"Map not a 1D RGB array of the right length (Set_IFD_Map)");
       return (1);
    }

  memcpy(tif->img->map,map->data,(size_t) (map->size*type_size[UINT16_TYPE]));

  return (0);
}


/****************************************************************************************
 *                                                                                      *
 *  PARSING A SERIES NAME                                                               *
 *                                                                                      *
 ****************************************************************************************/

Series_Bundle *Parse_Series_Name(Series_Bundle *R(M(bundle)), string file_name)
{ int   len;
  char *ps;
  char *s, *t, *n;

  len = (int) strlen(file_name);
  if (bundle->prefix == NULL)
    bundle->prefix = Guarded_Realloc(NULL,(size_t) (len+1),"Parse_Series_Name");
  else if (malloc_size(bundle->prefix) <= (size_t) len)
    bundle->prefix = (char *) Guarded_Realloc(bundle->prefix,(size_t) (len+1),"Parse_Series_Name");
  ps = bundle->prefix;
  if (ps == NULL)
    { if (grab_message())
        sprintf(Image_Estring,"Out of memory (Parse_Series_Name)");
      return (NULL);
    }

  strcpy(ps,file_name);

  for (t = ps + len; t > ps; t--)
    if (*t == '/')
      break;

  for (; *t != 0 && !isdigit(*t); t++)
    ;
  if (*t == 0)
    { if (grab_message())
        sprintf(Image_Estring,"No number in file name %s (Parse_Series_Name)",file_name);
      return (NULL);
    }
  n = t;
  while (isdigit(*t))
    t += 1;
  s = t;
  while (*t != 0)
    if (isdigit(*t++))
      { if (grab_message())
          sprintf(Image_Estring,
                  "Two distinct number subsegments in name: %s (Parse_Series_Name)",file_name);
        return (NULL);
      }

  bundle->suffix    = s;
  bundle->num_width = (int) (s-n);
  bundle->padded    = (*n == '0');
  bundle->first_num = atoi(n);
  *n = '\0';
  return (bundle);
}


/****************************************************************************************
 *                                                                                      *
 *  CENTRAL READ ROUTINE                                                                *
 *                                                                                      *
 ****************************************************************************************/

  // There are depth planes in the tiff file, and each successive tif IFD is returned by
  //   calling reader_handler with the next plane number.  If layer < 0 then get all layers,
  //   otherwise load only the specified layer.  If there is a color-map and the first
  //   channel is CHAN_MAPPED then allocate and build a map, returning it in *pmap.  The
  //   originating external routine has name 'routine'.  One calls read_handler(source,-1,?) to
  //   take care of any epilogue activity for the reading of successive IFDs.

static void *read_tiff(string routine, Dimn_Type depth,
                       void *source, Tiff_Reader *(*read_handler)(void *, int, string),
                       Array **pmap, int layer, Layer_Bundle *R(M(bundle)))
{ Indx_Type   *area, Area;
  int         *invert, Invert;
  Array      **array, *AVector;
  void       **planes, *Planes[16];

  Array       *map;
  int          cidx, lidx, lnum, nlayers;
  Dimn_Type    width, height;
  Array_Kind   kind;
  Value_Type   type;
  int          scale;
  Tiff_Reader *tif;
  Tiff_IFD    *ifd;
  Tiff_Image  *img;
  Tiff_Type    targ;
  string       text;
  int          count;
  Dimn_Type    dims[4];

  *pmap = NULL;

  tif = read_handler(source,0,routine);
  if (tif == NULL)
    return (NULL);

  ifd = Read_Tiff_IFD(tif);
  if (ifd == NULL)
    { string es = Tiff_Error_String();
      if (es != NULL)
        { if (grab_message())
            sprintf(Image_Estring,"Error reading Tif IFD: '%s' (%s)",es,routine);
          Tiff_Error_Release();
        }
      return (NULL);
    }
  img = Get_Tiff_Image(ifd);
  if (img == NULL)
    { string es = Tiff_Error_String();
      if (es != NULL)
        { if (grab_message())
            sprintf(Image_Estring,"Error reading Tif Image: '%s' (%s)",es,routine);
          Tiff_Error_Release();
        }
      return (NULL);
    }

  nlayers = 0;
  for (cidx = 0; cidx < img->number_channels; cidx += kind_size[kind])
    { kind = determine_kind(img,cidx);
      if (layer == nlayers)
        break;
      nlayers += 1;
    }

  if (layer >= 0)
    { if (cidx >= img->number_channels)
        { if (grab_message())
            sprintf(Image_Estring,"Layer %d does not exit in tiff (%s)",layer,routine);
          return (NULL);
        }
      lidx = cidx;
      nlayers += 1;
      area     = &Area - layer;
      invert   = &Invert - layer;
      array    = &AVector - layer;
    }
  else
    { int i;
      if (bundle->num_layers >= nlayers)
        { for (i = nlayers; i < bundle->num_layers; i++)
            Kill_Array(bundle->layers[i]);
          for (i = nlayers-1; i >= 0; i--)
            Free_Array(bundle->layers[i]);
        }
      else
        { for (i = bundle->num_layers-1; i >= 0; i--)
            Free_Array(bundle->layers[i]);
          bundle->layers =
              (Array **) Guarded_Realloc(bundle->layers,
                               (sizeof(Array *)+sizeof(Size_Type)+sizeof(int))*((size_t) nlayers),
                               routine);
          if (bundle->layers == NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Out of memory (%s)",routine);
              return (NULL);
            }
        }
      bundle->num_layers = nlayers;
      lidx   = 0;
      area   = (Size_Type *) (bundle->layers + nlayers);
      invert = (int *) (area + nlayers);
      array  = bundle->layers;
    }

  { int i;

    if (img->number_channels > 16)
      { planes = (void **)
                   Guarded_Realloc(NULL,sizeof(void *)*((size_t) img->number_channels),routine);
        if (planes == NULL)
          { if (grab_message())
              sprintf(Image_Estring,"Out of memory (%s)",routine);
            return (NULL);
          }
      }
    else
      planes = Planes;
    for (i = 0; i < img->number_channels; i++)
      planes[i] = NULL;
  }

  width  = img->width;
  height = img->height;

  dims[0] = width;
  dims[1] = height;
  dims[2] = depth;

  if (layer >= 0)
    { cidx = lidx; lnum = layer; }
  else
    lnum = cidx = 0;
  for ( ; lnum < nlayers; lnum++, cidx += kind_size[kind])
    { kind         = determine_kind(img,cidx);
      type         = determine_type(img,cidx);
      scale        = img->channels[cidx]->scale;
      invert[lnum] = (img->channels[cidx]->interpretation == CHAN_WHITE);
      area[lnum]   = (((Indx_Type) width) * height) * type_size[type];
      array[lnum]  = Make_Array(kind,type,2 + (depth != 1),dims);
      array[lnum]->scale = scale;
    }

  map = NULL;
  if (img->channels[0]->interpretation == CHAN_MAPPED && layer <= 0)
    { int dom = (1 << img->channels[0]->scale);
 
      dims[0] = dom;
      map = Make_Array(RGB_KIND,UINT16_TYPE,1,dims);

      memcpy(map->data,img->map,(size_t) (map->size*type_size[UINT16_TYPE]));
    }

  if ((text = (string) Get_Tiff_Tag(ifd,TIFF_JF_ANO_BLOCK,&targ,&count)) == NULL)
    text = Empty_String;
  if (layer > 0)
    Set_Array_Text(array[layer],text);
  else
    Set_Array_Text(array[0],text);

  { int       i;
    Dimn_Type d;

    d = 0;
    while (1)
      { Tiff_Channel **chan = img->channels;

        if (layer >= 0)
          { cidx = lidx; lnum = layer; }
        else
          lnum = cidx = 0;
        for ( ; lnum < nlayers; lnum++)
          { Indx_Type base;

            kind = array[lnum]->kind;
            if (kind == RGB_KIND)
              for (i = 0; i < 3; i++)
                { base = channel_order(chan[cidx+i]);
                  planes[cidx+i] = ((char *) array[lnum]->data) + area[lnum]*(d+depth*base);
                }
            else if (kind == RGBA_KIND)
              for (i = 0; i < 4; i++)
                { base = channel_order(chan[cidx+i]);
                  planes[cidx+i] = ((char *) array[lnum]->data) + area[lnum]*(d+depth*base);
                }
            else // kind == PLAIN_KIND
              planes[cidx] = ((char *) array[lnum]->data) + area[lnum]*d;
            cidx += kind_size[kind];
          }
        Load_Tiff_Image_Planes(img,planes);

        Free_Tiff_Image(img);
        Free_Tiff_IFD(ifd);

        d += 1;
        if (d >= depth) break;

        tif = read_handler(source,1,routine);
        if (tif == NULL)
          goto cleanup;

        while (1)
          { int *tag;

            ifd = Read_Tiff_IFD(tif);
            if (ifd == NULL)
              { string es = Tiff_Error_String();
                if (es != NULL)
                  { if (grab_message())
                      sprintf(Image_Estring,"Error reading Tif IFD: '%s' (%s)",es,routine);
                    Tiff_Error_Release();
                  }
                goto cleanup;
              }
            tag = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&targ,&count);
            if (tag == NULL || (*tag & TIFF_VALUE_REDUCED_RESOLUTION) == 0)
              break;
            Free_Tiff_IFD(ifd);
          }
        img = Get_Tiff_Image(ifd);
        if (img == NULL)
          { string es = Tiff_Error_String();
            if (es != NULL)
              { if (grab_message())
                  sprintf(Image_Estring,"Error reading Tif IFD: '%s' (%s)",es,routine);
                Tiff_Error_Release();
              }
            Free_Tiff_IFD(ifd);
            goto cleanup;
          }

        if (img->width != width || img->height != height)
          { if (grab_message())
              sprintf(Image_Estring,
                      "Planes of a stack are not of the same dimensions (%s)!",routine);
            Free_Tiff_Image(img);
            Free_Tiff_IFD(ifd);
            goto cleanup;
          }

        if (layer >= 0)
          { cidx = lidx; lnum = layer; }
        else
          lnum = cidx = 0;
        for ( ; lnum < nlayers; lnum++)
          { kind = array[lnum]->kind;
            if (determine_type(img,cidx) != array[lnum]->type ||
                determine_kind(img,cidx) != kind ||
                img->channels[cidx]->scale != array[lnum]->scale)
              { if (grab_message())
                  sprintf(Image_Estring,
                          "Planes of a stack are not of the same type (%s)!",routine);
                Free_Tiff_Image(img);
                Free_Tiff_IFD(ifd);
                goto cleanup;
              }
            cidx += kind_size[kind];
          }
      }
  }

  read_handler(source,-1,routine);

  if (layer >= 0)
    { cidx = lidx; lnum = layer; }
  else
    lnum = cidx = 0;
  for ( ; lnum < nlayers; lnum++)
    { if (invert[lnum])
        { double max;
          if (array[lnum]->type <= UINT32_TYPE)
            max = (double) ((((uint64) 1) << array[lnum]->scale) - 1);
          else if (array[lnum]->type <= INT32_TYPE)
            max = (double) ((((uint64) 1) << (array[lnum]->scale-1)) - 1);
          else
            max = 1.0;
          Scale_Array(array[lnum], -1., -max);
        }
      cidx += kind_size[array[lnum]->kind];
    }

  if (img->number_channels > 16)
    free(planes);

  *pmap = map;
  if (layer >= 0)
    return (array[layer]);
  else
    return (bundle);

cleanup:
  if (layer >= 0)
    { cidx = lidx; lnum = layer; }
  else
    lnum = cidx = 0;
  for ( ; lnum < nlayers; lnum++)
    Free_Array(array[lnum]);
  if (map != NULL) Free_Array(map);

  if (img->number_channels > 16)
    free(planes);

  *pmap = NULL;
  return (NULL);
}


/****************************************************************************************
 *                                                                                      *
 *  ALL OTHER READER ROUTINES (EXTERNAL INTERFACES)                                     *
 *                                                                                      *
 ***************************************************************************************/

static Dimn_Type image_depth(void *source, string routine)
{ Tiff_Reader *reader;
  Dimn_Type    depth;
  int         *tag, count;
  Tiff_Type    type;
  Tiff_IFD    *ifd;
  string       file_name = (string) source;

  reader = Open_Tiff_Reader(file_name,NULL,NULL,
                            strcmp(file_name+(strlen(file_name)-4),".lsm") == 0);
  if (reader == NULL)
    { string es = Tiff_Error_String();
      if (es != NULL)
        { if (grab_message())
            sprintf(Image_Estring,"Error opening %s: '%s' (%s)",file_name,es,routine);
          Tiff_Error_Release();
        }
      return (0);
    }
  depth = 0;
  while (! End_Of_Tiff(reader))
    { ifd = Read_Tiff_IFD(reader);
      if (ifd == NULL)
        { string es = Tiff_Error_String();
          if (es != NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Error reading Tif IFD: '%s' (%s)",es,routine);
              Tiff_Error_Release();
            }
          return (0);
        }
      tag = (int *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
      if (tag == NULL || (*tag & TIFF_VALUE_REDUCED_RESOLUTION) == 0)
        depth += 1;
      Free_Tiff_IFD(ifd);
    }
  Free_Tiff_Reader(reader);
  if (depth == 0)
    { if (grab_message())
        sprintf(Image_Estring,"Tif file %s has 0 planes within it! (%s)",file_name,routine);
    }
  return (depth);
}

static Dimn_Type series_depth(string sname, void *source, string routine)
{ FILE        *fd;
  Dimn_Type    pdepth, ndepth;
  Series_Bundle *bundle = (Series_Bundle *) source;

  for (pdepth = 0; 1; pdepth += 1) 
    { sprintf(sname,"%s%0*u%s",bundle->prefix,bundle->num_width,
                               bundle->first_num+pdepth,bundle->suffix);
      if ((fd = fopen(sname,"r")) == NULL)
        break;
      fclose(fd);
    }
  for (ndepth = 0; 1; ndepth += 1) 
    { sprintf(sname,"%s%u%s",bundle->prefix,bundle->first_num+ndepth,bundle->suffix);
      if ((fd = fopen(sname,"r")) == NULL)
        break;
      fclose(fd);
    }
  if (pdepth == 0 && ndepth == 0)
    { if (grab_message())
        sprintf(Image_Estring,"Tif series has no files! (%s)",routine);
    }
  if (pdepth > ndepth)
    { ndepth = pdepth;
      bundle->padded = 1;
    }
  else
    bundle->padded = 0;
  return (ndepth);
}

typedef struct
  { Tiff_Reader *tif;
    void        *source;
  } File_Source;

static Tiff_Reader *image_reader(void *source, int state, string routine)
{ File_Source *series    = (File_Source *) source;
  string       file_name = (string) series->source;

  if (state == 0)
    { series->tif = Open_Tiff_Reader(file_name,NULL,NULL,
                             strcmp(file_name+(strlen(file_name)-4),".lsm") == 0);
      if (series->tif == NULL)
        { string es = Tiff_Error_String();
          if (es != NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Error opening %s: '%s' (%s)",file_name,es,routine);
              Tiff_Error_Release();
            }
          return (NULL);
        }
    }
  else if (state < 0)
    Free_Tiff_Reader(series->tif);
  return (series->tif);
}

typedef struct
  { Tiff_Reader *tif;
    string       sname;
    Dimn_Type    index;
    void        *source;
  } Series_Source;

static Tiff_Reader *series_reader(void *source, int state, string routine)
{ Series_Source *series = (Series_Source *) source;
  Series_Bundle *bundle = (Series_Bundle *) series->source;
  string         sname;

  if (state == 0)
    series->index = bundle->first_num;
  else
    Free_Tiff_Reader(series->tif);
  sname = series->sname;
  if (state >= 0)
    { if (bundle->padded)
        sprintf(sname,"%s%0*u%s",bundle->prefix,bundle->num_width,series->index,bundle->suffix);
      else
        sprintf(sname,"%s%u%s",bundle->prefix,series->index,bundle->suffix);
      series->tif = Open_Tiff_Reader(sname,NULL,NULL,strcmp(sname+(strlen(sname)-4),".lsm") == 0);
      if (series->tif == NULL)
        { string es = Tiff_Error_String();
          if (es != NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Error opening %s: '%s' (%s)",sname,es,routine);
              Tiff_Error_Release();
            }
          return (NULL);
        }
      series->index += 1;
    }
  return (series->tif);
}

static void *read_array(Tiff_Reader *(*reader)(void *, int, string),
                        string routine, void *file_source, int layer, Layer_Bundle *bundle)
{ Dimn_Type  depth;
  Array     *map;
  void      *image;

  if (reader == image_reader)
    { File_Source series;
      depth = image_depth(file_source,routine);
      if (depth == 0)
        return (NULL);
      series.source = file_source;
      if ((image = read_tiff(routine,depth,&series,reader,&map,layer,bundle)) == NULL)
        return (NULL);
    }
  else
    { Series_Source  series;
      char           Name[256];
      Series_Bundle *fbun = (Series_Bundle *) file_source;
      int            slen = (int) (strlen(fbun->prefix) + 50 + strlen(fbun->suffix));

      if (slen > 255)
        { series.sname = (string) Guarded_Realloc(NULL,(size_t) (slen+1),routine);
          if (series.sname == NULL)
            { if (grab_message())
                sprintf(Image_Estring,"Out of memory (%s)",routine);
              return (NULL);
            }
        }
      else
        series.sname = Name;
      depth = series_depth(series.sname,file_source,routine);
      if (depth == 0)
        { if (slen > 255)
            free(series.sname);
          return (NULL);
        }
      series.source = file_source;
      image = read_tiff(routine,depth,&series,reader,&map,layer,bundle);
      if (slen > 255)
        free(series.sname);
      if (image == NULL)
        return (NULL);
    }

  if (map != NULL)
    { if (layer < 0)
        { Array *ex = Apply_Map(bundle->layers[0],map);
          Free_Array(bundle->layers[0]);
          bundle->layers[0] = ex;
        }
      else  // layer == 1
        { Array *ex = Apply_Map(image,map);
          Free_Array(image);
          image = ex;
        }
      Free_Array(map);
    }

  return (image);
}

Layer_Bundle *Read_Images(string file_name, Layer_Bundle *R(M(bundle)))
{ return (read_array(image_reader,"Read_Images",file_name,-1,bundle)); }

Array *G(Read_Image)(string file_name, int layer)
{ return (read_array(image_reader,"Read_Image",file_name,layer,NULL)); }

Layer_Bundle *Read_All_Series(Series_Bundle *source, Layer_Bundle *R(M(bundle)))
{ return (read_array(series_reader,"Read_All_Series",source,-1,bundle)); }

Array *G(Read_Series)(Series_Bundle *source, int layer)
{ return (read_array(series_reader,"Read_Series",source,layer,NULL)); }


/****************************************************************************************
 *                                                                                      *
 *  CENTRAL WRITE ROUTINE                                                               *
 *                                                                                      *
 ****************************************************************************************/

  // Write each successive plane of the list of layers in images to the Tiff_Writer returned
  //   by write_handler for each plane number.  If map is not NULL, then the first channel/layer
  //   is coded as being color-mapped in the tiff.  The originating external routine has name
  //   'routine'.  One calls write_handler(-1) to finish the write of the file or series of
  //   files.  The text of the first layer is written to the JF-TAGGER field of the (first) tif.

static Channel_Type atype2class[] = { CHAN_UNSIGNED, CHAN_UNSIGNED, CHAN_UNSIGNED,
                                      CHAN_SIGNED,   CHAN_SIGNED,   CHAN_SIGNED,
                                      CHAN_FLOAT,    CHAN_FLOAT };

static boolean write_tiff(string routine, void *targs, Tiff_Writer *(*write_handler)(void *, int),
                          Layer_Bundle *images, Array *map, Image_Compress compress)
{ int          ndims, color;
  Dimn_Type    depth;
  Channel_Type class;
  int          kind, scale;
  Tiff_Image  *img;
  Tiff_IFD    *ifd = NULL;
  Array      **array;
  int          cidx, lnum, nlayers;

  nlayers = images->num_layers;

  array = images->layers;
  color = (array[0]->kind != PLAIN_KIND);
  ndims = array[0]->ndims - color;
  if (ndims == 2)
    depth = 1;
  else
    depth = array[0]->dims[2];
  if ( ndims != 2 && ndims != 3)
    { if (grab_message())
        sprintf(Image_Estring,"Array is not an image or a stack (%s)\n",routine);
      return (1);
    }

  for (lnum = 0; lnum < nlayers; lnum++)
    { color = (array[lnum]->kind != PLAIN_KIND);
      if (array[lnum]->kind == COMPLEX_KIND)
        { if (grab_message())
            sprintf(Image_Estring,"Cannot write a complex array to tif (%s)\n",routine);
          return (1);
        }
      if (ndims != array[lnum]->ndims - color)
        { if (grab_message())
            sprintf(Image_Estring,"Layers are not all of the same dimensionality (%s)\n",routine);
          return (1);
        }
      if (color && array[lnum]->dims[ndims] != kind_size[array[lnum]->kind])
        { if (grab_message())
            sprintf(Image_Estring,"Kind and outer dimension are inconsistent (%s)\n",routine);
          return (1);
        }
      if (array[lnum]->scale > 32)
        { if (grab_message())
            sprintf(Image_Estring,"Tif format cannot handle values of more than 32 bits (%s)\n",
                    routine);
          return (1);
        }
      if (array[lnum]->dims[0] != array[0]->dims[0] ||
          array[lnum]->dims[1] != array[0]->dims[1] ||
          (ndims > 2 && array[lnum]->dims[2] != array[0]->dims[2]))
        { if (grab_message())
            sprintf(Image_Estring,"Layers do not all have the same dimensions (%s)\n",routine);
          return (1);
        }
    }

  if (map != NULL)
    { if (array[0]->kind != PLAIN_KIND)
        { if (grab_message())
            sprintf(Image_Estring,"Layer 0 is a color image, yet has a color map (%s)\n",routine);
          return (1);
        }
      if (map->ndims != 2)
        { if (grab_message())
            sprintf(Image_Estring,"Color map is not of dimension 2 (%s)\n",routine);
          return (1);
        }
      if (map->kind != RGB_KIND)
        { if (grab_message())
            sprintf(Image_Estring,"Color map is not an RGB_KIND (%s)\n",routine);
          return (1);
        }
      if (map->type != UINT16_TYPE)
        { if (grab_message())
            sprintf(Image_Estring,"Color map is not a UINT16_TYPE (%s)\n",routine);
          return (1);
        }
      if (map->dims[1] != 3)
        { if (grab_message())
            sprintf(Image_Estring,"Color map does not have outer dimension 3 (%s)\n",routine);
          return (1);
        }
      if (map->dims[0] != (1 << array[0]->scale))
        { if (grab_message())
            sprintf(Image_Estring,"Domain of color map doesn't match range of image (%s)\n",
                    routine);
          return (1);
        }
    }

  img  = Create_Tiff_Image(array[0]->dims[0],array[0]->dims[1]);
  if (img == NULL)
    goto error1;
  for (lnum = 0; lnum < nlayers; lnum++)
    { scale = array[lnum]->scale;
      class = atype2class[array[lnum]->type];
      switch (array[lnum]->kind)
        { case RGB_KIND:
          case RGBA_KIND:
            if (Add_Tiff_Image_Channel(img,CHAN_RED,scale,class,NULL)) goto error2;
            if (Add_Tiff_Image_Channel(img,CHAN_GREEN,scale,class,NULL)) goto error2;
            if (Add_Tiff_Image_Channel(img,CHAN_BLUE,scale,class,NULL)) goto error2;
            if (array[lnum]->kind == RGBA_KIND)
              { if (Add_Tiff_Image_Channel(img,CHAN_ALPHA,scale,class,NULL)) goto error2; }
            break;
          case PLAIN_KIND:
            if (map != NULL && lnum == 0)
              { if (Add_Tiff_Image_Channel(img,CHAN_MAPPED,scale,class,NULL)) goto error2; }
            else
              { if (Add_Tiff_Image_Channel(img,CHAN_BLACK,scale,class,NULL)) goto error2; }
            break;
          default:   //  Cannot happen as checked earlier
            break;
        }
    }

  if (map != NULL && img->channels[0]->interpretation == CHAN_MAPPED)
    memcpy(img->map,map->data,(size_t) (map->size*type_size[UINT16_TYPE]));

  { int          i;
    Size_Type    a;
    Dimn_Type    d;
    Tiff_Writer *tif;

    for (d = 0; d < depth; d++)
      { tif = write_handler(targs,d > 0);
        if (tif == NULL)
          goto error2;

        cidx = 0;
        for (lnum = 0; lnum < nlayers; lnum++)
          { kind = array[lnum]->kind;
            a = array[lnum]->dims[0];
            a = (a * array[lnum]->dims[1]) * type_size[array[lnum]->type];
            if (kind == RGB_KIND)
              for (i = 0; i < 3; i++)
                img->channels[cidx+i]->plane = ((char *) array[lnum]->data) + a*(d+i*depth);
            else if (kind == RGBA_KIND)
              for (i = 0; i < 4; i++)
                img->channels[cidx+i]->plane = ((char *) array[lnum]->data) + a*(d+i*depth);
            else // kind == PLAIN_KIND
              img->channels[cidx]->plane = ((char *) array[lnum]->data) + a*d;
            cidx += kind_size[kind];
          }

        switch (compress)
        { case DONT_PRESS:
            ifd = Make_IFD_For_Image(img,DONT_COMPRESS,0,0);
            break;
          case LZW_PRESS:
            ifd = Make_IFD_For_Image(img,LZW_COMPRESS,0,0);
            break;
          case PACKBITS_PRESS:
            ifd = Make_IFD_For_Image(img,PACKBITS_COMPRESS,0,0);
            break;
        }
        if (ifd == NULL)
          goto error3;

        if (d == 0 && array[0]->text[0] != '\0')
          if (Set_Tiff_Tag(ifd,TIFF_JF_ANO_BLOCK,TIFF_BYTE,
                           (int) (strlen(array[0]->text))+1,array[0]->text))
            goto error4;

        if (Write_Tiff_IFD(tif,ifd))
          goto error4;

        Free_Tiff_IFD(ifd);
      }
  }

  Free_Tiff_Image(img);
  write_handler(targs,-1);
  return (0);

error4:
  Free_Tiff_IFD(ifd);
error3:
  write_handler(targs,-1);
error2:
  Free_Tiff_Image(img);
error1:
  { string es = Tiff_Error_String();
    if (es != NULL)
      { if (grab_message())
          sprintf(Image_Estring,"Error reading Tif IFD: '%s' (%s)",es,routine);
        Tiff_Error_Release();
      }
  }
  return (1);
}


/****************************************************************************************
 *                                                                                      *
 *  WRITE ROUTINES (EXTERNAL INTERFACES)                                                *
 *                                                                                      *
 ****************************************************************************************/

typedef struct
  { Tiff_Writer *tif;
    string       file;
  } Image_Target;

static Tiff_Writer *image_writer(void *target, int state)
{ Image_Target *trg = (Image_Target *) target;

  if (state == 0)
    trg->tif = Open_Tiff_Writer(trg->file,0,0);
  else if (state < 0)
    Free_Tiff_Writer(trg->tif);
  return (trg->tif);
}

boolean Write_Image(string file_name, Array *image, Image_Compress compress)
{ Layer_Bundle WList;
  Array       *AList[1];
  Image_Target Target;

  WList.num_layers = 1;
  WList.layers     = AList;
  AList[0]         = image;

  Target.file = file_name;
  return (write_tiff("Write_Image",&Target,image_writer,&WList,NULL,compress));
}

boolean Write_Images(string file_name, Layer_Bundle *images, Image_Compress compress)
{ Image_Target Target;
  Target.file = file_name;
  return (write_tiff("Write_Images",&Target,image_writer,images,NULL,compress));
}

typedef struct
  { Tiff_Writer   *tif;
    string         name;
    Dimn_Type      index;
    Series_Bundle *bundle;
  } Series_Target;

static Tiff_Writer *plane_writer(void *target, int state)
{ Series_Target *trg = (Series_Target *) target;

  Series_Bundle *bundle = trg->bundle;
  string         name   = trg->name;

  if (state == 0)
    trg->index = bundle->first_num;
  else
    Free_Tiff_Writer(trg->tif);
  if (state >= 0)
    { if (bundle->padded)
        sprintf(name,"%s%0*u%s",bundle->prefix,bundle->num_width,
                           trg->index,bundle->suffix);
      else
        sprintf(name,"%s%u%s",bundle->prefix,trg->index,bundle->suffix);
      trg->tif    = Open_Tiff_Writer(name,0,0);
      trg->index += 1;
    }
  return (trg->tif);
}

boolean Write_Series(Series_Bundle *bundle, Array *image, Image_Compress compress)
{ Layer_Bundle  WList;
  Array        *AList[1];
  Series_Target Target;
  char          Name[256];
  int           slen = (int) (strlen(bundle->prefix) + 50 + strlen(bundle->suffix));
  boolean       stat;

  WList.num_layers = 1;
  WList.layers     = AList;
  AList[0]         = image;

  if (slen > 255)
    { Target.name = (string) Guarded_Realloc(NULL,(size_t) (slen+1),"Write_Series");
      if (Target.name == NULL)
        { if (grab_message())
            sprintf(Image_Estring,"Out of memory (Write_Series)");
          return (1);
        }
    }
  else
    Target.name = Name;
  Target.bundle = bundle;
  stat = write_tiff("Write_Series",&Target,plane_writer,&WList,NULL,compress);
  if (slen > 255)
    free(Target.name);
  return (stat);
}

boolean Write_All_Series(Series_Bundle *bundle, Layer_Bundle *images, Image_Compress compress)
{ Series_Target Target;
  char          Name[256];
  int           slen = (int) (strlen(bundle->prefix) + 50 + strlen(bundle->suffix));
  boolean       stat;

  if (slen > 255)
    { Target.name = (string) Guarded_Realloc(NULL,(size_t) (slen+1),"Write_All_Series");
      if (Target.name == NULL)
        { if (grab_message())
            sprintf(Image_Estring,"Out of memory (Write_All_Series)");
          return (1);
        }
    }
  else
    Target.name = Name;
  Target.bundle = bundle;
  stat = write_tiff("Write_All_Series",&Target,plane_writer,images,NULL,compress);
  if (slen > 255)
    free(Target.name);
  return (stat);
}
