/*****************************************************************************************\
*                                                                                         *
*  Tiff Image Coder and Decoder (Tiff 6.0)                                                *
*    The module allows one to extract the images in a tiff IFD into Tiff_Image that is    *
*    eseentially a set of arrays, one per channel (sample).  The module also allows you   *
*    to build a Tiff_Image and convert it back to a tiff IFD.                             *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  February 2008                                                                 *
*                                                                                         *
\*****************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utilities.h"
#include "tiff_io.h"
#include "tiff_image.h"

float halfp_2_fullp(unsigned short halfp)   // half-precision to float converter
{ static float  fullp;
  unsigned int *number = (unsigned int *) (&fullp);

  unsigned int mantissa = halfp & 0x03ff;
  unsigned int exponent = halfp & 0x7c00;
  unsigned int sign     = halfp & 0x8000;

  if (exponent == 0)
    { if (mantissa != 0)        // denormalized number
        { exponent = 0x1c000; 
          while ((mantissa & 0x200) == 0)   // normalize
            { mantissa <<= 1;
              exponent -=  0x400;
            }
          mantissa = ((mantissa << 1) & 0x3ff);
        }
                                // else 0
    }
  else if (exponent == 0x7c00)  // infinity or NAN
    exponent = 0xcfc00;
  else                          // normalized number
    exponent += 0x1c000;

  *number = (sign << 16) | ((exponent | mantissa) << 13); 
  return (fullp);
}

//  These routines are privately shared between tiff_io and tiff_image

extern unsigned char *Tiff_IFD_Data(Tiff_IFD *ifd);
extern int            Allocate_Tiff_IFD_Data(Tiff_IFD *ifd, int size);
extern int            Tiff_IFD_Data_Flip(Tiff_IFD *ifd);

#undef DEBUG_ENCODE
#undef DEBUG_DECODE

static unsigned char lowbits[8] = { 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01 };
static unsigned char cmpbits[9] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };

#define LZW_CLEAR_CODE 256
#define LZW_EOI_CODE   257


typedef struct __Tiff_Histogram
  { struct __Tiff_Histogram *next;
    Tiff_Histogram           tiff_histogram;
  } _Tiff_Histogram;

static _Tiff_Histogram *Free_Tiff_Histogram_List = NULL;
static size_t    Tiff_Histogram_Offset, Tiff_Histogram_Inuse;

static inline Tiff_Histogram *new_tiff_histogram(char *routine)
{ _Tiff_Histogram *object;

  if (Free_Tiff_Histogram_List == NULL)
    { object = (_Tiff_Histogram *) Guarded_Malloc(sizeof(_Tiff_Histogram),routine);
      Tiff_Histogram_Offset = ((char *) &(object->tiff_histogram)) - ((char *) object);
    }
  else
    { object = Free_Tiff_Histogram_List;
      Free_Tiff_Histogram_List = object->next;
    }
  Tiff_Histogram_Inuse += 1;
  return (&(object->tiff_histogram));
}

static inline Tiff_Histogram *copy_tiff_histogram(Tiff_Histogram *tiff_histogram)
{ Tiff_Histogram *copy = new_tiff_histogram("Copy_Tiff_Histogram");
  *copy = *tiff_histogram;
  return (copy);
}

Tiff_Histogram *Copy_Tiff_Histogram(Tiff_Histogram *tiff_histogram)
{ return (copy_tiff_histogram(tiff_histogram)); }

static inline void free_tiff_histogram(Tiff_Histogram *tiff_histogram)
{ _Tiff_Histogram *object  = (_Tiff_Histogram *) (((char *) tiff_histogram) - Tiff_Histogram_Offset);
  object->next = Free_Tiff_Histogram_List;
  Free_Tiff_Histogram_List = object;
  Tiff_Histogram_Inuse -= 1;
}

void Free_Tiff_Histogram(Tiff_Histogram *tiff_histogram)
{ free_tiff_histogram(tiff_histogram); }

static inline void kill_tiff_histogram(Tiff_Histogram *tiff_histogram)
{
  free(((char *) tiff_histogram) - Tiff_Histogram_Offset);
  Tiff_Histogram_Inuse -= 1;
}

void Kill_Tiff_Histogram(Tiff_Histogram *tiff_histogram)
{ kill_tiff_histogram(tiff_histogram); }

static inline void reset_tiff_histogram()
{ _Tiff_Histogram *object;
  while (Free_Tiff_Histogram_List != NULL)
    { object = Free_Tiff_Histogram_List;
      Free_Tiff_Histogram_List = object->next;
      kill_tiff_histogram(&(object->tiff_histogram));
      Tiff_Histogram_Inuse += 1;
    }
}

void Reset_Tiff_Histogram()
{ reset_tiff_histogram(); }

int Tiff_Histogram_Usage()
{ return (Tiff_Histogram_Inuse); }

static inline int tiff_channel_psize(Tiff_Channel *channel)
{ return (channel->bytes_per_pixel*channel->width*channel->height); }


typedef struct __Tiff_Channel
  { struct __Tiff_Channel *next;
    size_t                    psize;
    Tiff_Channel           tiff_channel;
  } _Tiff_Channel;

static _Tiff_Channel *Free_Tiff_Channel_List = NULL;
static size_t    Tiff_Channel_Offset, Tiff_Channel_Inuse;

static inline void allocate_tiff_channel_plane(Tiff_Channel *tiff_channel, size_t psize, char *routine)
{ _Tiff_Channel *object  = (_Tiff_Channel *) (((char *) tiff_channel) - Tiff_Channel_Offset);
  if (object->psize < psize)
    { if (object->psize == 0)
        object->tiff_channel.plane = NULL;
#ifdef _MSC_VER
      object->tiff_channel.plane  = (decltype(object->tiff_channel.plane))Guarded_Realloc(object->tiff_channel.plane,psize,routine);
#else
      object->tiff_channel.plane  = Guarded_Realloc(object->tiff_channel.plane,psize,routine);
#endif
      object->psize = psize;
    }
}

static inline Tiff_Channel *new_tiff_channel(size_t psize, char *routine)
{ _Tiff_Channel *object;

  if (Free_Tiff_Channel_List == NULL)
    { object = (_Tiff_Channel *) Guarded_Malloc(sizeof(_Tiff_Channel),routine);
      Tiff_Channel_Offset = ((char *) &(object->tiff_channel)) - ((char *) object);
      object->psize = 0;
    }
  else
    { object = Free_Tiff_Channel_List;
      Free_Tiff_Channel_List = object->next;
    }
  Tiff_Channel_Inuse += 1;
  allocate_tiff_channel_plane(&(object->tiff_channel),psize,routine);
  object-> tiff_channel.histogram = NULL;
  return (&(object->tiff_channel));
}

static inline Tiff_Channel *copy_tiff_channel(Tiff_Channel *tiff_channel)
{ Tiff_Channel *copy = new_tiff_channel(tiff_channel_psize(tiff_channel),"Copy_Tiff_Channel");
  Tiff_Channel  temp = *copy;
  *copy = *tiff_channel;
  copy->plane = temp.plane;
  if (tiff_channel_psize(tiff_channel) != 0)
    memcpy(copy->plane,tiff_channel->plane,tiff_channel_psize(tiff_channel));
  if (tiff_channel->histogram != NULL)
    copy->histogram = Copy_Tiff_Histogram(temp.histogram);
  return (copy);
}

Tiff_Channel *Copy_Tiff_Channel(Tiff_Channel *tiff_channel)
{ return (copy_tiff_channel(tiff_channel)); }

static inline void pack_tiff_channel(Tiff_Channel *tiff_channel)
{ _Tiff_Channel *object  = (_Tiff_Channel *) (((char *) tiff_channel) - Tiff_Channel_Offset);
  if (object->psize > tiff_channel_psize(tiff_channel))
    { object->psize = tiff_channel_psize(tiff_channel);
      if (object->psize != 0)
#ifdef _MSC_VER
        object->tiff_channel.plane = (decltype(object->tiff_channel.plane))Guarded_Realloc(object->tiff_channel.plane,
#else
        object->tiff_channel.plane = Guarded_Realloc(object->tiff_channel.plane,
#endif
                                                     object->psize,"Pack_Tiff_Channel");
      else
        { free(object->tiff_channel.plane);
          object->psize = 0;
        }
    }
}

void Pack_Tiff_Channel(Tiff_Channel *tiff_channel)
{ pack_tiff_channel(tiff_channel); }

static inline void free_tiff_channel(Tiff_Channel *tiff_channel)
{ _Tiff_Channel *object  = (_Tiff_Channel *) (((char *) tiff_channel) - Tiff_Channel_Offset);
  object->next = Free_Tiff_Channel_List;
  Free_Tiff_Channel_List = object;
  if (tiff_channel->histogram != NULL)
    Free_Tiff_Histogram(tiff_channel->histogram);
  Tiff_Channel_Inuse -= 1;
}

void Free_Tiff_Channel(Tiff_Channel *tiff_channel)
{ free_tiff_channel(tiff_channel); }

static inline void kill_tiff_channel(Tiff_Channel *tiff_channel)
{ _Tiff_Channel *object  = (_Tiff_Channel *) (((char *) tiff_channel) - Tiff_Channel_Offset);
  if (tiff_channel->histogram != NULL)
    Kill_Tiff_Histogram(tiff_channel->histogram);
  if (object->psize != 0)
    free(tiff_channel->plane);
  free(((char *) tiff_channel) - Tiff_Channel_Offset);
  Tiff_Channel_Inuse -= 1;
}

void Kill_Tiff_Channel(Tiff_Channel *tiff_channel)
{ kill_tiff_channel(tiff_channel); }

static inline void reset_tiff_channel()
{ _Tiff_Channel *object;
  while (Free_Tiff_Channel_List != NULL)
    { object = Free_Tiff_Channel_List;
      Free_Tiff_Channel_List = object->next;
      kill_tiff_channel(&(object->tiff_channel));
      Tiff_Channel_Inuse += 1;
    }
}

void Reset_Tiff_Channel()
{ reset_tiff_channel(); }

int Tiff_Channel_Usage()
{ return (Tiff_Channel_Inuse); }

static unsigned char *get_code_vector(int size, char *routine)
{ static unsigned char *Code_Vector = NULL;
  static int            Code_VecMax = 0;
    
  if (size < 0)       
    { free(Code_Vector);
      Code_VecMax = 0;
      Code_Vector = NULL;    
    }
  else if (size > Code_VecMax)
    { Code_VecMax = 1.1*size + 4096;
      Code_Vector = (unsigned char *) Guarded_Realloc(Code_Vector,Code_VecMax,routine);
    }
  return (Code_Vector);
}

static inline int tiff_image_csize(Tiff_Image *image)
{ return (sizeof(Tiff_Channel *)*image->number_channels); }

static inline int tiff_image_msize(Tiff_Image *image)
{ if (image->channels[0]->interpretation == CHAN_MAPPED)
    return (3*sizeof(unsigned short)*(1 << image->channels[0]->scale));
  else
    return (0);
}


typedef struct __Tiff_Image
  { struct __Tiff_Image *next;
    size_t                  csize;
    size_t                  msize;
    Tiff_Image           tiff_image;
  } _Tiff_Image;

static _Tiff_Image *Free_Tiff_Image_List = NULL;
static size_t    Tiff_Image_Offset, Tiff_Image_Inuse;

static inline void allocate_tiff_image_channels(Tiff_Image *tiff_image, size_t csize, char *routine)
{ _Tiff_Image *object  = (_Tiff_Image *) (((char *) tiff_image) - Tiff_Image_Offset);
  if (object->csize < csize)
    { if (object->csize == 0)
        object->tiff_image.channels = NULL;
#ifdef _MSC_VER
      object->tiff_image.channels  = (decltype(object->tiff_image.channels))Guarded_Realloc(object->tiff_image.channels,csize,routine);
#else
      object->tiff_image.channels  = Guarded_Realloc(object->tiff_image.channels,csize,routine);
#endif
      object->csize = csize;
    }
}

static inline void allocate_tiff_image_map(Tiff_Image *tiff_image, size_t msize, char *routine)
{ _Tiff_Image *object  = (_Tiff_Image *) (((char *) tiff_image) - Tiff_Image_Offset);
  if (object->msize < msize)
    { if (object->msize == 0)
        object->tiff_image.map = NULL;
#ifdef _MSC_VER
      object->tiff_image.map  = (decltype(object->tiff_image.map))Guarded_Realloc(object->tiff_image.map,msize,routine);
#else
      object->tiff_image.map  = Guarded_Realloc(object->tiff_image.map,msize,routine);
#endif
      object->msize = msize;
    }
}

static inline Tiff_Image *new_tiff_image(size_t csize, size_t msize, char *routine)
{ _Tiff_Image *object;

  if (Free_Tiff_Image_List == NULL)
    { object = (_Tiff_Image *) Guarded_Malloc(sizeof(_Tiff_Image),routine);
      Tiff_Image_Offset = ((char *) &(object->tiff_image)) - ((char *) object);
      object->csize = 0;
      object->msize = 0;
    }
  else
    { object = Free_Tiff_Image_List;
      Free_Tiff_Image_List = object->next;
    }
  Tiff_Image_Inuse += 1;
  allocate_tiff_image_channels(&(object->tiff_image),csize,routine);
  allocate_tiff_image_map(&(object->tiff_image),msize,routine);
  return (&(object->tiff_image));
}

static inline Tiff_Image *copy_tiff_image(Tiff_Image *tiff_image)
{ Tiff_Image *copy = new_tiff_image(tiff_image_csize(tiff_image),tiff_image_msize(tiff_image),"Copy_Tiff_Image");
  Tiff_Image  temp = *copy;
  *copy = *tiff_image;
  copy->channels = temp.channels;
  if (tiff_image_csize(tiff_image) != 0)
    memcpy(copy->channels,tiff_image->channels,tiff_image_csize(tiff_image));
  copy->map = temp.map;
  if (tiff_image_msize(tiff_image) != 0)
    memcpy(copy->map,tiff_image->map,tiff_image_msize(tiff_image));
  return (copy);
}

static inline void pack_tiff_image(Tiff_Image *tiff_image)
{ _Tiff_Image *object  = (_Tiff_Image *) (((char *) tiff_image) - Tiff_Image_Offset);
  if (object->csize > tiff_image_csize(tiff_image))
    { object->csize = tiff_image_csize(tiff_image);
      if (object->csize != 0)
#ifdef _MSC_VER
        object->tiff_image.channels = (decltype(object->tiff_image.channels))Guarded_Realloc(object->tiff_image.channels,
#else
        object->tiff_image.channels = Guarded_Realloc(object->tiff_image.channels,
#endif
                                                      object->csize,"Pack_Tiff_Image");
      else
        { free(object->tiff_image.channels);
          object->csize = 0;
        }
    }
  if (object->msize > tiff_image_msize(tiff_image))
    { object->msize = tiff_image_msize(tiff_image);
      if (object->msize != 0)
#ifdef _MSC_VER
        object->tiff_image.map = (decltype(object->tiff_image.map))Guarded_Realloc(object->tiff_image.map,
#else
        object->tiff_image.map = Guarded_Realloc(object->tiff_image.map,
#endif
                                                 object->msize,"Pack_Tiff_Image");
      else
        { free(object->tiff_image.map);
          object->msize = 0;
        }
    }
}

static inline void free_tiff_image(Tiff_Image *tiff_image)
{ _Tiff_Image *object  = (_Tiff_Image *) (((char *) tiff_image) - Tiff_Image_Offset);
  object->next = Free_Tiff_Image_List;
  Free_Tiff_Image_List = object;
  Tiff_Image_Inuse -= 1;
}

static inline void kill_tiff_image(Tiff_Image *tiff_image)
{ _Tiff_Image *object  = (_Tiff_Image *) (((char *) tiff_image) - Tiff_Image_Offset);
  if (object->msize != 0)
    free(tiff_image->map);
  if (object->csize != 0)
    free(tiff_image->channels);
  free(((char *) tiff_image) - Tiff_Image_Offset);
  Tiff_Image_Inuse -= 1;
}

static inline void reset_tiff_image()
{ _Tiff_Image *object;
  while (Free_Tiff_Image_List != NULL)
    { object = Free_Tiff_Image_List;
      Free_Tiff_Image_List = object->next;
      kill_tiff_image(&(object->tiff_image));
      Tiff_Image_Inuse += 1;
    }
}

int Tiff_Image_Usage()
{ return (Tiff_Image_Inuse); }

Tiff_Image *Copy_Tiff_Image(Tiff_Image *image)
{ Tiff_Image *copy = copy_tiff_image(image);
  int         i;

  for (i = 0; i < image->number_channels; i++)
    copy->channels[i] = Copy_Tiff_Channel(image->channels[i]);
  return (copy);
}

void Pack_Tiff_Image(Tiff_Image *image)
{ int   i;

  pack_tiff_image(image);
  for (i = 1; i < image->number_channels; i++)
    Pack_Tiff_Channel(image->channels[i]);
}

void Free_Tiff_Image(Tiff_Image *image)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i] != NULL)
      Free_Tiff_Channel(image->channels[i]);
  free_tiff_image(image);
}

void Kill_Tiff_Image(Tiff_Image *image)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i] != NULL)
      Kill_Tiff_Channel(image->channels[i]);
  kill_tiff_image(image);
}

void Reset_Tiff_Image()
{ reset_tiff_image();
  get_code_vector(-1,NULL);
}

#if defined(DEBUG_ENCODE) || defined(DEBUG_DECODE)

static void Print_Plane(int width, int height, int bytes, void *array, int hex, int diff)
{ int x, y;

  if (hex)
    { unsigned char *plane = array;
      if (bytes == 1)
        { for (y = 0; y < height; y++)
            { printf("Row %2d:",y);
              for (x = 0; x < width; x++)
                printf(" %02x",plane[y*width + x]);
              printf("\n");
            }
        }
      else if (bytes == 2)
        { for (y = 0; y < height; y++)
            { printf("Row %2d:",y);
              for (x = 0; x < width; x++)
                printf(" %02x%02x",plane[(y*width + x)*2],plane[(y*width + x)*2+1]);
              printf("\n");
            }
        }
      else //  bytes == 4
        { for (y = 0; y < height; y++)
            { printf("Row %2d:",y);
              for (x = 0; x < width; x++)
                printf(" %02x%02x%02x%02x",plane[(y*width + x)*4],plane[(y*width + x)*4+1],
                                           plane[(y*width + x)*4+2],plane[(y*width + x)*4+3]);
              printf("\n");
            }
        }
    }
  else
    { if (bytes == 1)
        { unsigned char *plane = array;
          signed char   *slane = array;
          for (y = 0; y < height; y++)
            { printf("Row %2d: %u",y,plane[y*width]);
              if (diff)
                for (x = 1; x < width; x++)
                  printf(" %d",slane[y*width + x]);
              else
                for (x = 1; x < width; x++)
                  printf(" %u",plane[y*width + x]);
              printf("\n");
            }
        }
      else if (bytes == 2)
        { unsigned short *plane = array;
          signed short   *slane = array;
          for (y = 0; y < height; y++)
            { printf("Row %2d: %u",y,plane[y*width]);
              if (diff)
                for (x = 1; x < width; x++)
                  printf(" %d",slane[y*width + x]);
              else
                for (x = 1; x < width; x++)
                  printf(" %u",plane[y*width + x]);
              printf("\n");
            }
        }
      else //  bytes == 4
        { unsigned int *plane = array;
          signed int   *slane = array;
          for (y = 0; y < height; y++)
            { printf("Row %2d: %u",y,plane[y*width]);
              if (diff)
                for (x = 1; x < width; x++)
                  printf(" %d",slane[y*width + x]);
              else
                for (x = 1; x < width; x++)
                  printf(" %u",plane[y*width + x]);
              printf("\n");
            }
        }
    }
}

#endif

int PackBits_Decoder(unsigned char *stream, unsigned char *decode, int decode_len)
{ int           i, j, n;
  unsigned char d, *p;

  p = stream;
  i = 0;
  while (i < decode_len)
    { n = *((signed char *) (p++));
      if (n >= 0)
        { for (j = 0; j <= n; j++)
            decode[i++] = *p++;
        }
      else if (n > -128)
        { n = -n;
          d = *p++;
          for (j = 0; j <= n; j++)
            decode[i++] = d;
        }
    }
  return (p-stream);
}


int PackBits_Encoder(unsigned char *stream, int row_bytes, int stream_len, unsigned char *encode)
{ int           i, j, k, r;
  unsigned char d, *p;

#define MOVE_UNIQUE_STRETCH	\
  while (i-k > 128)		\
    { *p++ = 127;		\
      for (j = 0; j < 128; j++)	\
        *p++ = stream[k++];	\
    }				\
  if (i-k > 0)			\
    { *p++ = (i-k)-1;		\
      while (k < i)		\
        *p++ = stream[k++];	\
    }

  p = encode;
  for (r = 0; r < stream_len; r += row_bytes)   // Pack each row separately as per spec.
    { k = i = 0;
      while (i < row_bytes)
        { d = stream[i];
          for (j = i+1; j < stream_len; j++)
            if (d != stream[j])
              break;
          if (j-i > 2)
            { MOVE_UNIQUE_STRETCH
              *p++ = ((i-j)+1) & 0xff;
              *p++ = d;
              k = j;
            }
          i = j;
        }
      MOVE_UNIQUE_STRETCH
      stream += row_bytes;
    }

  return (p - encode);
}

int LZW_Decoder(unsigned char *stream, unsigned char *decode)
{ static int            firstime = 1;
  static int            symlen[4096];
  static unsigned char *strbeg[4096];
  static unsigned char  string[256];

  int            bit     = 0;
  unsigned char *byte    = stream;

  int            tabtop  = LZW_EOI_CODE+1;
  int            codelen = 9;
  int            ratchet = 511;

  unsigned char *dbyte   = decode;
  unsigned char *lastPut = decode-1;

  if (firstime)
    { int i;

      firstime = 0;
      for (i = 0; i < LZW_CLEAR_CODE; i++)
        { symlen[i] = 1;
          strbeg[i] = string+i;
          string[i] = i;
        }
    }

  while (1)
    { int code;

      // get next codelen bits in code for codelen in [9,12]

#define READ_CODE				\
      { int lo;					\
						\
        lo   = bit + codelen - 8;		\
        code = (*byte++ & lowbits[bit]) << lo;	\
        if (lo < 8)				\
          bit = lo;				\
        else					\
          { bit = lo-8;				\
            code |= ((*byte++) << bit);		\
          }					\
        code |= ((*byte) >> (8-bit));		\
      }

      READ_CODE

      if (code == LZW_EOI_CODE) break;

      if (code == LZW_CLEAR_CODE)
        { tabtop  = LZW_EOI_CODE+1;
          codelen = 9;
          ratchet = 511;

          READ_CODE

          if (code == LZW_CLEAR_CODE) break;
        }

      else
        { symlen[tabtop] = (dbyte-lastPut) + 1;
          strbeg[tabtop] = lastPut;

          tabtop += 1;
          if (tabtop == ratchet)
            { codelen += 1;
              ratchet  = (ratchet << 1) + 1;
            }
        }

      lastPut = dbyte;

      { int            i;
        unsigned char *sbyte;

        sbyte = strbeg[code];
        for (i = 0; i < symlen[code]; i++)
          *dbyte++ = *sbyte++;
      }
    }

  return ((byte-stream) + (bit != 0));
}

int LZW_Encoder(unsigned char *stream, int stream_len, unsigned char *encode, int max_avail)
{ static int            firstime = 1;
  static int            lt_fork[4096];
  static int            gt_fork[4096];
  static int            forward[4096];
  static int            symbol[4096];

  int            bit     = 0;
  unsigned char *byte    = encode;
  unsigned char *enend   = encode + max_avail;

  int            tabtop  = LZW_EOI_CODE+1;
  int            codelen = 9;
  int            ratchet = 512;

  { int i;

    if (firstime)
      { firstime = 0;
        for (i = 0; i < LZW_CLEAR_CODE; i++)
          symbol[i] = i;
      }
    for (i = 0; i < LZW_CLEAR_CODE; i++)
      forward[i] = -1;
  }

  // write next codelen bits in code for codelen in [9,12]

#define WRITE_CODE(value)			\
  { int lo;					\
						\
    lo = bit + codelen - 8;			\
    *byte++ |= ((value >> lo) & lowbits[bit]);	\
    if (lo < 8)					\
      bit = lo;					\
    else					\
      { bit = lo-8;				\
        *byte++ = (value >> bit) & 0xff;	\
      }						\
    if (byte >= enend)				\
      return (-1);				\
    *byte = (value << (8-bit)) & 0xff;		\
  }

  // *byte = 0;
  // WRITE_CODE(LZW_CLEAR_CODE)

  { int i, code;

  *byte = 0;
  WRITE_CODE(LZW_CLEAR_CODE)

    code = stream[0];
    for (i = 1; i < stream_len; i++)
      { int *lptr, last;
        int  csym, esym;
  
        csym = stream[i];
        last = code;
        code = *(lptr = forward + code);
        while (code >= 0 && csym != (esym = symbol[code]))
          if (csym < esym)
            code = *(lptr = lt_fork + code);
          else
            code = *(lptr = gt_fork + code);
  
        if (code < 0)
          { code = *lptr = tabtop++;
  
            WRITE_CODE(last)
  
            symbol[code]  = csym;
            lt_fork[code] = gt_fork[code] = forward[code] = -1;
  
            if (tabtop >= ratchet)
              { int k;

                if (ratchet == 4095)
                  { WRITE_CODE(LZW_CLEAR_CODE)
                    tabtop  = LZW_EOI_CODE+1;
                    codelen = 9;
                    ratchet = 512;
                    for (k = 0; k < LZW_CLEAR_CODE; k++)
                      forward[k] = -1;
                  }
                else
                  { codelen  += 1;
                    ratchet <<= 1;
                    if (ratchet == 4096)
                      ratchet = 4095;
                  }
              }
  
            code = csym;
          }
      }
  
    WRITE_CODE(code)
    if (++tabtop >= ratchet && ratchet != 4095)
      codelen += 1;
    WRITE_CODE(LZW_EOI_CODE)
  }

  return ((byte-encode) + (bit != 0));
}

static void flip_rows(Tiff_Channel *channel, int width, int height)
{ unsigned char *p, *q, *r;
  int            b, y, k, t;

  b = (channel->scale-1)/8 + 1;
  width *= b; 

  p = (unsigned char *) channel->plane;
  for (y = 0; y < height; y++)
    { q = p;
      p = p+width;
      r = p-b;
      while (q < r)
        { for (k = 0; k < b; k++)
            { t    = q[k];
              q[k] = r[k];
              r[k] = t;
            }
          q += b;
          r -= b;
        }
    }
}

static void flip_columns(Tiff_Channel *channel, int width, int height)
{ unsigned char *p, *q, *r, *s;
  int            b, x, t;

  b = (channel->scale-1)/8 + 1;
  width *= b; 

  p = (unsigned char *) channel->plane;
  s = p + width * (height-1);
  for (x = 0; x < width; x++)
    { q = p++;
      r = s++;
      while (q < r)
        { t  = *q;
          *q = *r;
          *r = t;
          q += width;
          r -= width;
        }
    }
}

static inline unsigned int get_integer_tag(Tiff_IFD *ifd, int label, int *error)
{ void     *p;
  int       count;
  Tiff_Type type;

  *error = 0;
  p = Get_Tiff_Tag(ifd,label,&type,&count);
  if (p == NULL)
    { *error = 1;
      return (0);
    }
  if (count != 1)
    { *error = 2;
      return (0);
    }
  if (type == TIFF_SHORT)
    return (*((unsigned short *) p));
  else if (type == TIFF_LONG)
    { unsigned int value = *((unsigned int *) p);
      if (value >= 0x80000000)
        return (0x7fffffff);
      else
        return (value);
    }
  else
    { *error = 3;
      return (0);
    }
}

static char Error_String[1000];

static inline void *error(char *text)
{ strcpy(Error_String,text);
  return (NULL);
}

char *Tiff_Image_Error()
{ return (Error_String); }

Tiff_Image *Extract_Image_From_IFD(Tiff_IFD *ifd)
{ static int             Max_Samples_Per_Pixel = 0;
  static int            *Bytes_Per_Sample = NULL;
  static unsigned short *Default_Bps = NULL;

  int             width, height;
  int             samples_per_pixel;
  unsigned short *bits_per_sample;
  int             bytes_per_chunk;
  int             bits_per_chunk;

  unsigned short *sample_format;
  unsigned short *color_map;
  int             photometric;
  int             photo_samples;
  unsigned char  *extra_samples;

  unsigned char  *decode;
  int             compression;
  int             prediction;

  unsigned int   *countI;
  unsigned short *countS;
  int             cshort;
  int             twidth;
  int             theight;
  int             strips;
  int             orient;
  int             chunk;

  Tiff_Image    *image;
  Tiff_Channel **channel;
  int            map_size;

  int eflag;

  //  Get all the tags and information required to build the image and check that all
  //    tags and values make sense (before starting to build the image)

  //  Get and check width and height

  width  = get_integer_tag(ifd,TIFF_IMAGE_WIDTH,&eflag);
  if (eflag != 0)
    {error("IFD does not contain a valid tag for image width");
	 return NULL;}
  height = get_integer_tag(ifd,TIFF_IMAGE_LENGTH,&eflag);
  if (eflag != 0)
    {error("IFD does not contain a valid tag for image height");
	return NULL;}

  //  Get and check samples_per_pixel and bits_per_sample array
  
  { int       count;
    Tiff_Type type;

    samples_per_pixel = get_integer_tag(ifd,TIFF_SAMPLES_PER_PIXEL,&eflag);
    if (eflag != 0)
      samples_per_pixel = 1;

    bits_per_sample = (unsigned short *) Get_Tiff_Tag(ifd,TIFF_BITS_PER_SAMPLE,&type,&count);
  
    if (bits_per_sample != NULL)
      { if (type != TIFF_SHORT)
          {error("Bits_per_sample tag is not of type SHORT");
		  return NULL;}
        if (samples_per_pixel != count)
          {error("Length of bits_per_sample array not equal to samples_per_pixel tag");
		  return NULL;}
      }

    { int i, c;

      if (samples_per_pixel > Max_Samples_Per_Pixel)
        { Max_Samples_Per_Pixel = samples_per_pixel+4;
          Bytes_Per_Sample = (int *) Guarded_Realloc(Bytes_Per_Sample,
                                        sizeof(int)*Max_Samples_Per_Pixel,"Extract_Image_From_IFD");
          Default_Bps = (unsigned short *) Guarded_Realloc(Default_Bps,
                            sizeof(unsigned short)*Max_Samples_Per_Pixel,"Extract_Image_From_IFD");
        }

      if (bits_per_sample == NULL)
        { bits_per_sample = Default_Bps;
          for (i = 0; i < samples_per_pixel; i++)
            Default_Bps[i] = 1;
        }

      bytes_per_chunk = 0;
      bits_per_chunk  = 0;
      for (i = 0; i < samples_per_pixel; i++)
        { c = bits_per_sample[i];
          if (c > 32)
            {error("# of bits in a sample is more than 32");
			return NULL;}

          Bytes_Per_Sample[i] = (c-1)/8+1;
          if (Bytes_Per_Sample[i] == 3)
            Bytes_Per_Sample[i] = 4;
          bytes_per_chunk += Bytes_Per_Sample[i];
          bits_per_chunk  += bits_per_sample[i];
        }
    }
  }

  //  If the sample_format tag is present, make sure its size and type are correct

  { Tiff_Type type;
    int       count;

    sample_format = (unsigned short *) Get_Tiff_Tag(ifd,TIFF_SAMPLE_FORMAT,&type,&count);

    if (sample_format != NULL)
      { int i;

        if (type != TIFF_SHORT)
          {error("Sample_format tag is not of type SHORT");
		  return NULL;}
        if (count != samples_per_pixel)
          {error("Sample_format tag is not of length samples_per_pixel");
		  return NULL;}
        for (i = 0; i < samples_per_pixel; i++)
          if (sample_format[i] == TIFF_VALUE_IEEE_FLOATING_POINT_DATA)
            if (bits_per_sample[i] != 32)
              {error("Floating point pixels must be 32-bits");
			  return NULL;}
      }
  }

  //  Get and check the photometric information including extra_samples if color

  { Tiff_Type ctype, etype;
    int       ccount, ecount;

    map_size = 0;
    photometric   = get_integer_tag(ifd,TIFF_PHOTOMETRIC_INTERPRETATION,&eflag);
    color_map     = (unsigned short *) Get_Tiff_Tag(ifd,TIFF_COLOR_MAP,&ctype,&ccount);
    extra_samples = (unsigned char *) Get_Tiff_Tag(ifd,TIFF_EXTRA_SAMPLES,&etype,&ecount);

    if (eflag != 0)
      {error("Photometric interpretation tag is not present");
	  return NULL;}

    photo_samples = 1;
    if (photometric == TIFF_VALUE_RGB)
      { if (samples_per_pixel < 3)
          {error("RGB image has less than 3 samples_per_pixel");
		  return NULL;}
        photo_samples = 3;
      }
    else if (photometric == TIFF_VALUE_RGB_PALETTE)
      { if (color_map == NULL)
          {error("Color map table is missing");
		  return NULL;}
        if (ctype != TIFF_SHORT)
          {error("Color_map tag is not of type SHORT");
		  return NULL;}
        if (bits_per_sample[0] > 16)
          {error("Color map over values with more than 16-bits");
		  return NULL;}
        if (ccount != 3*(1 << bits_per_sample[0]))
          {error("Color map size doesn not match value range");
		  return NULL;}
        map_size = ccount*sizeof(unsigned short);
      }
    else if (photometric < 0 || photometric > TIFF_VALUE_TRANSPARENCY_MASK)
      {error("Do not understand photometric_interpretation tag");
	  return NULL;}

    if (extra_samples != NULL)
      { if (ecount != samples_per_pixel - photo_samples)
          {error("Extra_samples tag has wrong number of samples");
		  return NULL;} 
        if (etype != TIFF_BYTE)
          {error("Extra_samples tag is not of type BYTE");
		  return NULL;}
      }
  }

  //  Get and check the compression status

  compression = get_integer_tag(ifd,TIFF_COMPRESSION,&eflag);
  if (eflag != 0)
    compression = TIFF_VALUE_UNCOMPRESSED;
  else if (compression != TIFF_VALUE_UNCOMPRESSED && compression != TIFF_VALUE_LZW &&
           compression != TIFF_VALUE_PACK_BITS)
    {error("Support only uncompressed and LZW compressed images");
	return NULL;}

  prediction = get_integer_tag(ifd,TIFF_PREDICTOR,&eflag);
  if (eflag != 0)
    prediction = TIFF_NO_PREDICTION;

  //  Get and check the image encoding parameters, offsets, and byte counts

  { int       count;
    Tiff_Type type;

    countS  = (unsigned short *) Get_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,&type,&count);
    if (countS == NULL)
      { countS  = (unsigned short *) Get_Tiff_Tag(ifd,TIFF_TILE_BYTE_COUNTS,&type,&count);
        if (countS == NULL)
          {error("IFD does not contain a strip or tile image encoding");
		  return NULL;}
        theight = get_integer_tag(ifd,TIFF_TILE_LENGTH,&eflag);
        if (eflag != 0)
          {error("Tile encoding does not contain a tile_height tag");
		  return NULL;}
        twidth  = get_integer_tag(ifd,TIFF_TILE_WIDTH,&eflag);
        if (eflag != 0)
          {error("Tile encoding does not contain a tile_width tag");
		  return NULL;}
        strips = 0;
      }
    else
      { theight = get_integer_tag(ifd,TIFF_ROWS_PER_STRIP,&eflag);
        twidth  = width;
        if (eflag != 0)
          theight = height;
        strips = 1;
      }
    countI = (unsigned int *) countS;
    cshort = (type == TIFF_SHORT);
    if (theight > height)
      theight = height;
    if (twidth > width)
      twidth = width;

    chunk = get_integer_tag(ifd,TIFF_PLANAR_CONFIGURATION,&eflag);
    chunk = ((eflag != 0) || (chunk == TIFF_VALUE_CHUNKY)) && (samples_per_pixel > 1);

    orient = get_integer_tag(ifd,TIFF_ORIENTATION,&eflag);
    if (eflag != 0)
      orient = TIFF_VALUE_TOP_N_LEFT;
  }

  if (chunk)
    decode = get_code_vector(twidth*theight*bytes_per_chunk,"Extract_Image_From_IFD");
  else
    decode = get_code_vector(twidth*theight*((int) sizeof(int)),"Extract_Image_From_IFD");

  //  Now begin building the image, first by filling out the descriptions of all the channels

  image = new_tiff_image(samples_per_pixel*sizeof(Tiff_Channel *),map_size,
                         "Extract_Image_From_IFD");
  image->width           = width;
  image->height          = height;
  image->number_channels = samples_per_pixel;

  if (map_size > 0)
    memcpy(image->map,color_map,map_size);

  channel = image->channels;

  { int i;

    for (i = 0; i < samples_per_pixel; i++)
      { channel[i] = new_tiff_channel(width*height*Bytes_Per_Sample[i],"Extract_Image_From_IFD");

        channel[i]->width  = width;
        channel[i]->height = height;
        channel[i]->scale  = bits_per_sample[i];

        if (sample_format == NULL)
          channel[i]->type = CHAN_UNSIGNED;
        else if (sample_format[i] == TIFF_VALUE_TWOS_COMPLEMENT_DATA)
          channel[i]->type = CHAN_SIGNED;
        else if (sample_format[i] == TIFF_VALUE_IEEE_FLOATING_POINT_DATA)
          channel[i]->type = CHAN_FLOAT;
        else
          channel[i]->type = CHAN_UNSIGNED;

        channel[i]->bytes_per_pixel = Bytes_Per_Sample[i];
      }
  }

  if (photometric == TIFF_VALUE_WHITE_IS_ZERO)
    channel[0]->interpretation = CHAN_WHITE;
  else if (photometric == TIFF_VALUE_BLACK_IS_ZERO)
    channel[0]->interpretation = CHAN_BLACK;
  else if (photometric == TIFF_VALUE_RGB)
    { channel[0]->interpretation = CHAN_RED;
      channel[1]->interpretation = CHAN_GREEN;
      channel[2]->interpretation = CHAN_BLUE;
    }
  else if (photometric == TIFF_VALUE_RGB_PALETTE)
    channel[0]->interpretation = CHAN_MAPPED;
  else //  photometric == TIFF_VALUE_TRANSPARENCY_MASK
    channel[0]->interpretation = CHAN_MASK;
 
  { int i;

    for (i = photo_samples; i < samples_per_pixel; i++)
      if (extra_samples == NULL)
        channel[i]->interpretation = CHAN_OTHER;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_ALPHA_DATA)
        channel[i]->interpretation = CHAN_ALPHA;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_SOFT_MATTE)
        channel[i]->interpretation = CHAN_MATTE;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_EXTRA_RED)
        channel[i]->interpretation = CHAN_RED;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_EXTRA_GREEN)
        channel[i]->interpretation = CHAN_GREEN;
      else if (extra_samples[i-photo_samples] == TIFF_VALUE_EXTRA_BLUE)
        channel[i]->interpretation = CHAN_BLUE;
      else
        channel[i]->interpretation = CHAN_OTHER;
  }

#ifdef DEBUG_DECODE

  { int i;

    static char *Meaning_String[] = { "White", "Black", "Mapped", "Red", "Green", "Blue",
                                      "Alpha", "Matte", "Mask", "Other" };

    static char *Type_String[] = { "Unsigned", "Signed", "Float" };

    printf("\nIFD Image: %d x %d, %d channels\n",image->width,image->height,image->number_channels);
    for (i = 0; i < samples_per_pixel; i++)
      printf("  %d: %s %d(%d) %s\n",i,Meaning_String[channel[i]->interpretation],
                                    channel[i]->scale,channel[i]->bytes_per_pixel,
                                    Type_String[channel[i]->type]);

    printf("  map_size = %d\n",map_size);
    printf("\n  bits_per_sample =");
    for (i = 0; i < samples_per_pixel; i++)
      printf(" %d",bits_per_sample[i]);
    printf(" += %d\n",bits_per_chunk);
    printf("  bytes_per_sample =");
    for (i = 0; i < samples_per_pixel; i++)
      printf(" %d",Bytes_Per_Sample[i]);
    printf(" += %d\n",bytes_per_chunk);
    printf("  tiling      = %d x %d\n",twidth,theight);
    printf("  chunking    = %d\n",chunk);
    printf("  compression = %d\n",compression);
    printf("  prediction  = %d\n",prediction);
    printf("  orientation = %d\n",orient);
  }

#endif

//  Move 8*nbytes + nbits bits from bit stream (source,bitpos) to byte stream target.

#define UNPACK_BITS(source,bitpos,target,nbits,nbytes)				\
{ unsigned char t;								\
  int           b;								\
										\
  if (nbits != 0)								\
    { bitpos += nbits;								\
      if (bitpos <= 8)								\
        { *target++ = ((*source >> (8-bitpos)) & cmpbits[nbits]);		\
          if (bitpos == 8)							\
            { source += 1;							\
              bitpos  = 0;							\
            }									\
        }									\
      else   									\
        { bitpos -= 8;								\
          t = ((*source++ & cmpbits[nbits-bitpos]) << bitpos);			\
          *target++ = (t | ((*source >> (8-bitpos)) & cmpbits[bitpos]));	\
        }									\
    }										\
  if (bitpos == 0)								\
    for (b = 0; b < nbytes; b++)						\
      *target++ = *source++;							\
  else										\
    for (b = 0; b < nbytes; b++)						\
      { t = ((*source++ & lowbits[bitpos]) << bitpos);				\
        *target++ = (t | ((*source >> (8-bitpos)) & cmpbits[bitpos]));		\
      }										\
}

  //  Read each image chunk, decompress it if necessary, and place in the image
  //    Must pay attention to the reversal of rows and columns that may be required by orient.
  //    Distinct inner loops for chunk vs. planar, and byte vs. bit movement.

  { int            ocorner, icorner;
    int            segment, offset;
    int            xinc, yinc;
    int            x, y;
    unsigned char *dfeed;

    int bpc = bits_per_chunk;
    int bc8 = (bpc & 0x7);
    int bc3 = (bpc >> 3);

    if (orient > 4)
      { xinc = height;
        yinc = 1;
      }
    else
      { xinc = 1;
        yinc = width;
      }

    if (chunk)

      for (y       = 0,
           ocorner = 0,
           offset  = 0,
           segment = 0; y < height; y       += theight,
                                    ocorner += yinc*theight)
        for (x       = 0,
             icorner = ocorner; x < width; x       += twidth,
                                           icorner += xinc*twidth,
                                           offset  += (cshort ? countS[segment] : countI[segment]),
                                           segment += 1)

          { int s, off;
            int maxi, maxj;

            dfeed = Tiff_IFD_Data(ifd) + offset;
            if (compression == TIFF_VALUE_LZW)
              { LZW_Decoder(dfeed,decode);
                dfeed = decode;
              }
            else if (compression == TIFF_VALUE_PACK_BITS)
              { if (strips && y+theight > height)
                  PackBits_Decoder(dfeed,decode,bytes_per_chunk*twidth*(height-y));
                else
                  PackBits_Decoder(dfeed,decode,bytes_per_chunk*twidth*theight);
                dfeed = decode;
              }

            maxj = height-y;
            if (maxj > theight)
              maxj = theight;

            maxi = width-x;
            if (maxi > twidth)
              maxi = twidth;

#ifdef DEBUG_DECODE
            printf("  Tile Corner (%d,%d) -> %d\n",x,y,icorner);
#endif

            for (s   = 0,
                 off = 0; s < samples_per_pixel; s   += 1,
                                                 off += bits_per_sample[s])

              { int BPS = Bytes_Per_Sample[s];
                int bps = bits_per_sample[s];
                int bp3 = (bps >> 3);
                int bp8 = (bps & 0x7);
                int pad = (BPS == 4 && bps <= 24);

                int uinc   = yinc * BPS;
                int tinc   = (xinc-1) * BPS + pad;

                unsigned char *opixel, *ipixel;
                unsigned char *stream;
                int            spos;

                int i, j;

                opixel = ((unsigned char *) (channel[s]->plane)) + icorner * BPS + pad;
                stream = dfeed + (off >> 3);
                spos   = (off & 0x7);

                if (bc8 == 0 && spos == 0 && bp8 == 0)

                  { int bs3 = bc3 - bp3;

                    for (j = 0; j < maxj; j      += 1,
                                          opixel += uinc,
                                          stream += bc3*(twidth-i))
                      for (i      = 0,
                           ipixel = opixel; i < maxi; i      += 1,
                                                      ipixel += tinc,
                                                      stream += bs3)
                        { int b;
                          for (b = 0; b < bp3; b++)
                            *ipixel++ = *stream++;
                        }
                  }

                else   //  Must perform bit ops

                  { int bs8 = bpc - bps;

                    for (j = 0; j < maxj; j      += 1,
                                          opixel += uinc,
                                          spos   += bpc*(twidth-i))
                      for (i      = 0,
                           ipixel = opixel; i < maxi; i      += 1,
                                                      ipixel += tinc,
                                                      spos   += bs8)
                        { stream += (spos >> 3);
                          spos   &= 0x7;
                          UNPACK_BITS(stream,spos,ipixel,bp8,bp3)
                        }
                  }
              }
          }

    else  //  ! chunk

      { int s;
  
        segment = 0;
        offset  = 0;
        for (s = 0; s < samples_per_pixel; s++)
          { int BPS = Bytes_Per_Sample[s];
            int bps = bits_per_sample[s];
            int bp3 = (bps >> 3);
            int bp8 = (bps & 0x7);
            int pad = (BPS == 4 && bps <= 24);
    
            int uinc = yinc * BPS;
            int tinc = (xinc-1) * BPS + pad;

            for (y       = 0,
                 ocorner = 0; y < height; y       += theight,
                                          ocorner += theight*yinc)
              for (x       = 0,
                   icorner = ocorner; x < width; x += twidth,
                                                 icorner += twidth*xinc,
                                                 offset  += (cshort ? countS[segment]
                                                                    : countI[segment]),
                                                 segment += 1)

                { unsigned char *opixel, *ipixel;
                  unsigned char *stream;
                  int            spos;
 
                  int i, j;
                  int maxi, maxj;

                  dfeed = Tiff_IFD_Data(ifd) + offset;
                  if (compression == TIFF_VALUE_LZW)
                    { LZW_Decoder(dfeed,decode);
                      dfeed = decode;
                    }
                  else if (compression == TIFF_VALUE_PACK_BITS)
                    { if (strips && y+theight > height)
                        PackBits_Decoder(dfeed,decode,BPS*twidth*(height-y));
                      else
                        PackBits_Decoder(dfeed,decode,BPS*twidth*theight);
                      dfeed = decode;
                    }

                  maxj = height-y;
                  if (maxj > theight)
                    maxj = theight;
 
                  maxi = width-x;
                  if (maxi > twidth)
                    maxi = twidth;

#ifdef DEBUG_DECODE
                  printf("  Plane %d: Tile Corner (%d,%d) -> %d %d\n",s,x,y,icorner,offset);
                  printf("    Bit case: bytes = %d bits = %d\n",bp3,bp8);
#endif
  
                  opixel  = ((unsigned char *) channel[s]->plane) + icorner * BPS + pad;
                  stream  = dfeed;
                  spos    = 0;
  
                  if (bp8 == 0)
  
                    for (j = 0; j < maxj; j      += 1,
                                          opixel += uinc)
                      for (i      = 0,
                           ipixel = opixel; i < maxi; i      += 1,
                                                      ipixel += tinc)
                        { int b;
                          for (b = 0; b < bp3; b++)
                            *ipixel++ = *stream++;
                        }
  
                  else
  
                    for (j = 0; j < maxj; j      += 1,
                                          opixel += uinc,
                                          spos   += bps*(twidth-i))
                      { stream += (spos >> 3);
                        spos   &= 0x7;
                        for (i      = 0,
                             ipixel = opixel; i < maxi; i      += 1,
                                                        ipixel += tinc)
                          UNPACK_BITS(stream,spos,ipixel,bp8,bp3)
                      }
                }
          }
      }
  }

  if (orient > 4)
    { image->width  = height;
      image->height = width;
      width  = image->width;
      height = image->height;
    }

  //  Endian flip multi-byte data if needed

  { int            a = width*height;
    int            i, s;

    for (s = 0; s < samples_per_pixel; s++)
      { unsigned char *w = (unsigned char *) (channel[s]->plane);
        unsigned char  x;

        if (Bytes_Per_Sample[s] == 2)

          { if ((bits_per_sample[s] == 16 && Tiff_IFD_Data_Flip(ifd)) ||
                (bits_per_sample[s] < 16 && ! Native_Endian()))

              for (i = 0; i < a; i++)
                { x = w[0];
                  w[0] = w[1];
                  w[1] = x;
                  w += 2;
                }
          }

        else if (Bytes_Per_Sample[s] == 4)

          { if ((bits_per_sample[s] == 32 && Tiff_IFD_Data_Flip(ifd)) ||
                (bits_per_sample[s] < 32 && ! Native_Endian()))

              for (i = 0; i < a; i++)
                { x = w[0];
                  w[0] = w[3];
                  w[3] = x;
                  x = w[1];
                  w[1] = w[2];
                  w[2] = x;
		  w += 4;
		}
	  }
      }
  }

  //  Difference Predictor: note carefully that differences are sign-extended
  //    in twos' complement representation (if the architecture happens to support
  //    another representation of signed numbers then this code doesn't work!)

  if (prediction == TIFF_HORIZONTAL_DIFFERENCING)
    { int s, x, y;

      for (s = 0; s < samples_per_pixel; s++)

        if (Bytes_Per_Sample[s] == 1)
          { unsigned char  last;
            unsigned char *base = (unsigned char *) (channel[s]->plane);
            unsigned char  sign, mask;

            mask = cmpbits[bits_per_sample[s]];
            sign = ~mask;

            for (y = 0; y < height; y++)
              { last  = *base;
                base += 1;
                for (x = 1; x < width; x++)
                  { last = ((last + (signed char) (*base | sign)) & mask);
                    *base++ = last;
                  }
              }
          }

        else if (Bytes_Per_Sample[s] == 2)
          { unsigned short  last;
            unsigned short *base = (unsigned short *) (channel[s]->plane);
            unsigned short  sign, mask;

            mask = (cmpbits[bits_per_sample[s]-8] << 8) | 0xff;
            sign = ~mask;

            for (y = 0; y < height; y++)
              { last  = *base;
                base += 1;
                for (x = 1; x < width; x++)
                  { last = ((last + (signed short) (*base | sign)) & mask);
                    *base++ = last;
                  }
              }
          }

        else //  Bytes_Per_Sample[s] == 4
          { unsigned int  last;
            unsigned int *base = (unsigned int *) (channel[s]->plane);
            unsigned int  sign, mask;

            if (bits_per_sample[s] > 24)
              mask = (cmpbits[bits_per_sample[s]-24] << 24) | 0xffffff;
            else
              mask = (cmpbits[bits_per_sample[s]-16] << 16) | 0xffff;
            sign = ~mask;

            for (y = 0; y < height; y++)
              { last  = *base;
                base += 1;
                for (x = 1; x < width; x++)
                  { last = ((last + (signed int) (*base | sign)) & mask);
                    *base++ = last;
                  }
              }
          }
    }

  //  Signed data requires extension !!!!

  //  Flip rows, columns, and dimensions as per orient directive

  { int i;

    for (i = 0; i < samples_per_pixel; i++)
      { if (orient%4 >= 2)
          flip_rows(channel[i],width,height);
        if ((orient-1)%4 >= 2)
          flip_columns(channel[i],width,height);
      }
  }

  return (image);
}

Tiff_Image *Create_Tiff_Image(int width, int height)
{ Tiff_Image *image;

  image = new_tiff_image(10*sizeof(Tiff_Channel *),0,"Begin_Tiff_Image_Creation");
  image->width  = width;
  image->height = height;
  image->number_channels = 0;
  return (image);
}

int Add_Tiff_Image_Channel(Tiff_Image *image, Channel_Meaning meaning, int scale, Channel_Type type)
{ Tiff_Channel *channel;
  int           n;

  if (scale > 32)
    { error("Scale cannot be more than 32 bits\n");
      return (1);
    }

  n = image->number_channels;

  int csize = ((_Tiff_Image *) (((char *) image) - Tiff_Image_Offset))->csize;
  if ((n+1)*((int) sizeof(Tiff_Channel *)) > csize) 
    allocate_tiff_image_channels(image,(n+10)*sizeof(Tiff_Channel *),"Add_Tiff_Image_Channel");

  image->number_channels = n+1;
  image->channels[n] = channel = new_tiff_channel(0,"Add_Tiff_Image_Channel");
  channel->width          = image->width;
  channel->height         = image->height;
  channel->interpretation = meaning;
  channel->scale          = scale;
  channel->type           = type;
  channel->histogram      = NULL;

  scale = (scale-1)/8+1;
  if (scale == 3)
    scale = 4;
  channel->bytes_per_pixel = scale;

  allocate_tiff_channel_plane(channel,tiff_channel_psize(channel),"Add_Tiff_Image_Channel");

  if (n == 0 && meaning == CHAN_MAPPED)
    allocate_tiff_image_map(image,tiff_image_msize(image),"Add_Tiff_Image_Channel");

  return (0);
}

Tiff_IFD *Make_IFD_For_Image(Tiff_Image *image, int compression)
{ static int             Max_Samples_Per_Pixel = 0;

  static unsigned short *Bits_Per_Sample = NULL;
  static unsigned short *Sample_Format = NULL;
  static unsigned char  *Extra_Samples = NULL;
  static unsigned int   *Offsets = NULL;
  static unsigned int   *Byte_Counts = NULL;

  static unsigned short  Samples_Per_Pixel;
  static unsigned short  Planar_Configuration;
  static unsigned short  Compression;
  static unsigned short  Predictor;
  static unsigned short  Photometric;

  Tiff_IFD      *ifd;
  int            photo_samples;
  int            i, data_size, remain;
  unsigned char *data, *stream;
  unsigned char *encode;

  encode = get_code_vector(1.1*image->width*image->height*sizeof(int) + 4096,"Make_IFD_For_Image");

  if (image->number_channels > Max_Samples_Per_Pixel)
    { Max_Samples_Per_Pixel = image->number_channels+4;

      Bits_Per_Sample = (unsigned short *) Guarded_Realloc(Bits_Per_Sample,
                            2*sizeof(unsigned short)*Max_Samples_Per_Pixel,"Make_IFD_For_Image");
      Sample_Format   = Bits_Per_Sample + Max_Samples_Per_Pixel;

      Offsets = (unsigned int *) Guarded_Realloc(Offsets,
                         2*sizeof(unsigned int)*Max_Samples_Per_Pixel,"Make_IFD_For_Image");
      Byte_Counts = Offsets + Max_Samples_Per_Pixel;

      Extra_Samples = (unsigned char *) Guarded_Realloc(Extra_Samples,
                         sizeof(unsigned char)*Max_Samples_Per_Pixel,"Make_IFD_For_Image");
    }

  ifd = Create_Tiff_IFD(12);

  Set_Tiff_Tag(ifd,TIFF_IMAGE_WIDTH,TIFF_LONG,1,&(image->width));
  Set_Tiff_Tag(ifd,TIFF_IMAGE_LENGTH,TIFF_LONG,1,&(image->height));

  Samples_Per_Pixel = image->number_channels;
  Set_Tiff_Tag(ifd,TIFF_SAMPLES_PER_PIXEL,TIFF_SHORT,1,&(Samples_Per_Pixel));

  data_size = 0;
  for (i = 0; i < image->number_channels; i++)
    { Bits_Per_Sample[i] = image->channels[i]->scale;
      data_size += tiff_channel_psize(image->channels[i]);
    }
  Set_Tiff_Tag(ifd,TIFF_BITS_PER_SAMPLE,TIFF_SHORT,Samples_Per_Pixel,Bits_Per_Sample);

  Set_Tiff_Tag(ifd,TIFF_ROWS_PER_STRIP,TIFF_LONG,1,&(image->height));

  Planar_Configuration = TIFF_VALUE_PLANAR;
  Set_Tiff_Tag(ifd,TIFF_PLANAR_CONFIGURATION,TIFF_SHORT,1,&Planar_Configuration);

  if (compression)
    { Predictor   = TIFF_HORIZONTAL_DIFFERENCING;
      Compression = TIFF_VALUE_LZW;
    }
  else
    { Predictor   = TIFF_NO_PREDICTION;
      Compression = TIFF_VALUE_UNCOMPRESSED;
    }

  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i]->type == CHAN_UNSIGNED)
      Sample_Format[i] = TIFF_VALUE_UNSIGNED_INTEGER_DATA;
    else if (image->channels[i]->type == CHAN_SIGNED)
      Sample_Format[i] = TIFF_VALUE_TWOS_COMPLEMENT_DATA;
    else //  image->channels[i]->type == CHAN_FLOAT
      Sample_Format[i] = TIFF_VALUE_IEEE_FLOATING_POINT_DATA;
  Set_Tiff_Tag(ifd,TIFF_SAMPLE_FORMAT,TIFF_SHORT,Samples_Per_Pixel,Sample_Format);

  photo_samples = 1;
  if (image->channels[0]->interpretation == CHAN_WHITE)
    Photometric = TIFF_VALUE_WHITE_IS_ZERO;
  else if (image->channels[0]->interpretation == CHAN_BLACK)
    Photometric = TIFF_VALUE_BLACK_IS_ZERO;
  else if (image->channels[0]->interpretation == CHAN_MAPPED)
    { Photometric = TIFF_VALUE_RGB_PALETTE;
      Set_Tiff_Tag(ifd,TIFF_COLOR_MAP,TIFF_SHORT,3*(1 << image->channels[0]->scale),
                       image->map);
    }
  else if (image->channels[0]->interpretation == CHAN_MASK)
    Photometric = TIFF_VALUE_TRANSPARENCY_MASK;
  else //  image->channels[0]->interpretation == CHAN_RED & 1 == CHAN_GREEN & 2 == CHAN_BLUE
    { Photometric = TIFF_VALUE_RGB;
      photo_samples = 3;
    }
  Set_Tiff_Tag(ifd,TIFF_PHOTOMETRIC_INTERPRETATION,TIFF_SHORT,1,&Photometric);

  if (photo_samples < image->number_channels)
    { for (i = photo_samples; i < image->number_channels; i++)
        if (image->channels[i]->interpretation == CHAN_ALPHA)
          Extra_Samples[i-photo_samples] = TIFF_VALUE_ALPHA_DATA;  
        else if (image->channels[i]->interpretation == CHAN_MATTE)
          Extra_Samples[i-photo_samples] = TIFF_VALUE_SOFT_MATTE;
        else if (image->channels[i]->interpretation == CHAN_RED)
          Extra_Samples[i-photo_samples] = TIFF_VALUE_EXTRA_RED;
        else if (image->channels[i]->interpretation == CHAN_GREEN)
          Extra_Samples[i-photo_samples] = TIFF_VALUE_EXTRA_GREEN;
        else if (image->channels[i]->interpretation == CHAN_BLUE)
          Extra_Samples[i-photo_samples] = TIFF_VALUE_EXTRA_BLUE;
        else //  image->channels[i]->interpretation == CHAN_OTHER
          Extra_Samples[i-photo_samples] = TIFF_VALUE_UNSPECIFIED_DATA;
      Set_Tiff_Tag(ifd,TIFF_EXTRA_SAMPLES,TIFF_BYTE,
                   image->number_channels-photo_samples,Extra_Samples);
    }

//  Move an 8*nbytes + nbits bits from byte stream source to bit stream (target,bitpos).

#define PACK_BITS(source,nbits,nbytes,target,bitpos)				\
{ int           b;								\
										\
  if (nbits != 0)								\
    { bitpos += nbits;								\
      if (bitpos == nbits)							\
        *target = ((*source & cmpbits[nbits]) << (8-bitpos));			\
      else if (bitpos <= 8)							\
        { *target |= ((*source & cmpbits[nbits]) << (8-bitpos));		\
          if (bitpos == 8)							\
            { target += 1;							\
              bitpos  = 0;							\
            }									\
        }									\
      else   									\
        { bitpos -= 8;								\
          *target++ |= ((*source & cmpbits[nbits]) >> bitpos);			\
          *target    = ((*source & cmpbits[bitpos]) << (8-bitpos));		\
        }									\
      source += 1;								\
    }										\
  if (bitpos == 0)								\
    for (b = 0; b < nbytes; b++)						\
      *target++ = *source++;							\
  else										\
    for (b = 0; b < nbytes; b++)						\
      { *target++ |= (*source >> bitpos);					\
        *target    = (*source++ << (8-bitpos));					\
      }										\
}

  Allocate_Tiff_IFD_Data(ifd,data_size+2);

redo:
  data = stream = Tiff_IFD_Data(ifd);
  remain = data_size;
  //printf("compression = %d\n",compression);

  for (i = 0; i < image->number_channels; i++)
    { int scale = image->channels[i]->scale;
      int bytes = image->channels[i]->bytes_per_pixel;
      int a     = image->width*image->height;
      int x, y;
      int byte_count;
      unsigned char *source;

      source = (unsigned char *) (image->channels[i]->plane);

      if (compression)    //  Difference data if required

        { if (bytes == 1)
            { unsigned char  last, next;
              unsigned char *base = source;
              signed char   *diff = (signed char *) encode;
              for (y = 0; y < image->height; y++)
                { last  = *base;
                  *((unsigned char *) diff) = last;
                  base += 1;
                  diff += 1;
                  for (x = 1; x < image->width; x++)
                    { next    = *base++;
                      *diff++ = next - last;
                      last    = next;
                    }
                }
            }

          else if (bytes == 2)

            { unsigned short  last, next;
              unsigned short *base = (unsigned short *) source;
              signed short   *diff = (signed short *) encode;
              for (y = 0; y < image->height; y++)
                { last  = *base;
                  *((unsigned short *) diff) = last;
                  base += 1;
                  diff += 1;
                  for (x = 1; x < image->width; x++)
                    { next    = *base++;
                      *diff++ = next - last;
		      last    = next;
		    }
                }
            }

          else //  bytes == 4

            { unsigned int  last, next;
              unsigned int *base = (unsigned int *) source;
              signed int   *diff = (signed int *) encode;
              for (y = 0; y < image->height; y++)
                { last  = *base;
                  *((unsigned int *) diff) = last;
                  base += 1;
                  diff += 1;
                  for (x = 1; x < image->width; x++)
                    { next    = *base++;
                      *diff++ = next - last;
                      last    = next;
                    }
                }
            }

          source = encode;

#ifdef DEBUG_ENCODE
          printf("\nDifference predicted:\n");
          Print_Plane(image->width,image->height,bytes,source,0,1);
#endif
        }

      //  Endian flip multi-byte data if required

      if (bytes == 2)

        { if (scale < 16 && ! Native_Endian())

            { unsigned char *w;

              if (compression || scale%8 != 0)
                w = encode;
              else
                w = stream;

              if (w == source)
                { unsigned char  t;
                  for (x = 0; x < a; x++)
                    { t = w[0];
                      w[0] = w[1];
                      w[1] = t;
                      w += 2;
                    }
                }
              else
                { unsigned char *v = source;
                  for (x = 0; x < a; x++)
                    { w[0] = v[1];
                      w[1] = v[0];
                      w += 2;
                      v += 2;
                    }
                }
              source = w - bytes*a;

#ifdef DEBUG_ENCODE
              printf("\nEndian flipped:\n");
              Print_Plane(image->width,image->height,bytes,source,1,0);
#endif
            }
        }

      else if (bytes == 4)

        { if (scale < 32 && ! Native_Endian())

            { unsigned char *w;

              if (compression || scale%8 != 0)
                w = encode;
              else
                w = stream;

              if (w == source)
                { unsigned char  t;
                  for (x = 0; x < a; x++)
                    { t = w[0];
                      w[0] = w[3];
                      w[3] = t;
                      t = w[1];
                      w[1] = w[2];
                      w[2] = t;
	              w += 4;
                    }
                }
              else
                { unsigned char *v = source;
                  for (x = 0; x < a; x++)
                    { w[0] = v[3];
                      w[1] = v[2];
                      w[2] = v[1];
                      w[3] = v[0];
	              w += 4;
                      v += 4;
                    }
                }
              source = w - bytes*a;

#ifdef DEBUG_ENCODE
              printf("\nEndian flipped:\n");
              Print_Plane(image->width,image->height,bytes,source,1,0);
#endif
            }
        }

      //  In-place bit-packing if non-byte boundary sample size

      if (scale % 8 != 0 || scale == 24)
        { int bp8 = (scale & 0x7);
          int bp3 = (scale >> 3);
          int pos;
          unsigned char *src;
          unsigned char *trg;
          unsigned char *org;

          src = source;
          if (compression)
            trg = org = encode;
          else
            trg = org = stream;

          pos = 0;
          if (16 < scale && scale <= 24)
            for (x = 0; x < a; x++)
              { src += 1;
                PACK_BITS(src,bp8,bp3,trg,pos)
              }
          else
            for (x = 0; x < a; x++)
              PACK_BITS(src,bp8,bp3,trg,pos)
          if (pos != 0)
            trg++;
          byte_count = trg - org;

          source = org;

#ifdef DEBUG_ENCODE
          printf("\nBit packed:\n");
          Print_Plane(byte_count,1,1,source,1,0);
#endif
        }
      else
        byte_count = a*bytes;

      //  Compress if requested

      if (compression)
        { byte_count = LZW_Encoder(source,byte_count,stream,remain);
          if (byte_count < 0)
            { Predictor   = TIFF_NO_PREDICTION;
              Compression = TIFF_VALUE_UNCOMPRESSED;   //  Compressed result longer than
              compression = 0;                         //    uncompressed result !  Turn it
	      //printf("Compression failed\n");
              goto redo;                               //    off and redo it.
            }
        }
      else
        { if (source != stream)
            memcpy(stream,source,byte_count);
        }

#ifdef DEBUG_ENCODE
      printf("\nFinale:\n");
      Print_Plane(byte_count,1,1,stream,1,0);
#endif

      Byte_Counts[i] = byte_count;
      Offsets[i]     = stream-data;
      stream += byte_count;
      remain -= byte_count;
    }
 
  Set_Tiff_Tag(ifd,TIFF_PREDICTOR,TIFF_SHORT,1,&Predictor);
  Set_Tiff_Tag(ifd,TIFF_COMPRESSION,TIFF_SHORT,1,&Compression);

  Set_Tiff_Tag(ifd,TIFF_STRIP_OFFSETS,TIFF_LONG,image->number_channels,Offsets);
  Set_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,TIFF_LONG,image->number_channels,Byte_Counts);

  return (ifd);
}

#define UPSIZE_AREA(S_TYPE,T_TYPE)		\
{ S_TYPE *source = (S_TYPE *) (channel->plane);	\
  T_TYPE *target = (T_TYPE *) (channel->plane);	\
  for (i = area-1; i >= 0; i--)			\
    target[i] = source[i];			\
}

#define DOWNSIZE_AREA(S_TYPE,T_TYPE)		\
{ S_TYPE *source = (S_TYPE *) (channel->plane);	\
  T_TYPE *target = (T_TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    target[i] = source[i];			\
}

#define SHIFTUP_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    target[i] <<= delta;			\
}

#define SHIFTDOWN_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    target[i] >>= delta;			\
}

void Scale_Tiff_Channel(Tiff_Channel *channel, int scale)
{ int i, area, bps, delta;

  if (scale == channel->scale || channel->type == CHAN_FLOAT) return;

  bps = (scale-1)/8+1;
  if (bps == 3)
    bps = 4;

  area = channel->width * channel->height;

  if (scale > channel->scale)

    { delta = scale - channel->scale;

      if (bps > channel->bytes_per_pixel)
        { allocate_tiff_channel_plane(channel,channel->width*channel->height*bps,
                                              "Scale_Tiff_Channel");
          if (channel->bytes_per_pixel == 1)
            if (bps == 2)
              UPSIZE_AREA(unsigned char, unsigned short)
            else // bps == 4
              UPSIZE_AREA(unsigned char, unsigned int)
          else // channel->bytes_per_pixel == 2 && bps == 4
            UPSIZE_AREA(unsigned short, unsigned int)
        }

      if (channel->type == CHAN_UNSIGNED)
        if (bps == 1)
          SHIFTUP_AREA(unsigned char)
        else if (bps == 2)
          SHIFTUP_AREA(unsigned short)
        else //  bps == 4
          SHIFTUP_AREA(unsigned int)
      else // channel->type == CHAN_SIGNED
        if (bps == 1)
          SHIFTUP_AREA(signed char)
        else if (bps == 2)
          SHIFTUP_AREA(short)
        else //  bps == 4
          SHIFTUP_AREA(int)
    }

  else

    { delta = channel->scale - scale;

      if (channel->type == CHAN_UNSIGNED)
        if (channel->bytes_per_pixel == 1)
          SHIFTDOWN_AREA(unsigned char)
        else if (channel->bytes_per_pixel == 2)
          SHIFTDOWN_AREA(unsigned short)
        else //  channel->bytes_per_pixel == 4
          SHIFTDOWN_AREA(unsigned int)
      else // channel->type == CHAN_SIGNED
        if (channel->bytes_per_pixel == 1)
          SHIFTDOWN_AREA(signed char)
        else if (channel->bytes_per_pixel == 2)
          SHIFTDOWN_AREA(short)
        else //  channel->bytes_per_pixel == 4
          SHIFTDOWN_AREA(int)

      if (bps < channel->bytes_per_pixel)
        { if (channel->bytes_per_pixel == 4)
            if (bps == 2)
              DOWNSIZE_AREA(unsigned int, unsigned short)
            else // bps == 1
              DOWNSIZE_AREA(unsigned int, unsigned char)
          else // channel->bytes_per_pixel == 2 && bps == 1
            DOWNSIZE_AREA(unsigned short, unsigned char)
        }
    }

  channel->scale = scale;
  channel->bytes_per_pixel = bps;
}

void Scale_Tiff_Image(Tiff_Image *image, int scale)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    Scale_Tiff_Channel(image->channels[i],scale);
}

#define RANGE_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  for (i = 0; i < area; i++)			\
    { TYPE v = target[i];			\
      if (v < mn)				\
        mn = v;					\
      else if (v > mx)				\
        mx = v;					\
    }						\
  *minval = mn;					\
  *maxval = mx;					\
}

void Range_Tiff_Channel(Tiff_Channel *channel, double *minval, double *maxval)
{ int area = channel->width * channel->height;
  int i;

  if (channel->type == CHAN_FLOAT)
    { float mn = 0.;
      float mx = 0.;
      RANGE_AREA(float)
    }
  else if (channel->type == CHAN_UNSIGNED)
    { unsigned int mn = 0;
      unsigned int mx = 0;
      if (channel->bytes_per_pixel == 1)
        RANGE_AREA(unsigned char)
      else if (channel->bytes_per_pixel == 2)
        RANGE_AREA(unsigned short)
      else //  channel->bytes_per_pixel == 4
        RANGE_AREA(unsigned int)
    }
  else //  channel->type == CHAN_SIGNED
    { int mn = 0;
      int mx = 0;
      if (channel->bytes_per_pixel == 1)
        RANGE_AREA(signed char)
      else if (channel->bytes_per_pixel == 2)
        RANGE_AREA(short)
      else //  channel->bytes_per_pixel == 4
        RANGE_AREA(int)
    }
}

#define SHIFT_AREA(TYPE)			\
{ TYPE *target = (TYPE *) (channel->plane);	\
  if (shift > 0)				\
    for (i = 0; i < area; i++)			\
      target[i] <<= shift;			\
  else						\
    for (i = 0; i < area; i++)			\
      target[i] >>= chift;			\
}

void Shift_Tiff_Channel(Tiff_Channel *channel, int shift)
{ int          area = channel->width * channel->height;
  int          i, chift;

  if (shift == 0 || channel->type == CHAN_FLOAT) return;

  chift = -shift;
  if (channel->type == CHAN_UNSIGNED)
    if (channel->bytes_per_pixel == 1)
      SHIFT_AREA(unsigned char)
    else if (channel->bytes_per_pixel == 2)
      SHIFT_AREA(unsigned short)
    else //  channel->bytes_per_pixel == 4
      SHIFT_AREA(unsigned int)
  else
    if (channel->bytes_per_pixel == 1)
      SHIFT_AREA(signed char)
    else if (channel->bytes_per_pixel == 2)
      SHIFT_AREA(short)
    else //  channel->bytes_per_pixel == 4
      SHIFT_AREA(int)
}

void Shift_Tiff_Image(Tiff_Image *image, int shift)
{ int i;
  for (i = 0; i < image->number_channels; i++)
    Shift_Tiff_Channel(image->channels[i],shift);
}

Tiff_Histogram *Histogram_Tiff_Channel(Tiff_Channel *channel)
{ int i, bit;                                                                               

  Tiff_Histogram *histogram = new_tiff_histogram("Histogram_Tiff_Channel");

  unsigned int *array = histogram->counts;
  int           area  = channel->width * channel->height; 
  int           bpp   = channel->bytes_per_pixel;

  for (i = 0; i < 512; i++)
    array[i] = 0;

  histogram->total = area;

  if (bpp == 1)
    { unsigned char *target = (unsigned char *) (channel->plane);
      for (i = 0; i < area; i++)
        array[target[i]] += 1;
      histogram->bitshift = 0;
      return (histogram);
    }

  { unsigned int *target = (unsigned int *) (channel->plane);                          
    int           words  = (area * bpp) / 4;
    unsigned int  chord;
    int           profile;

    chord = 0;
    for (i = 0; i < words; i ++)
      chord |= target[i]; 
  
    profile = 0;
    if (bpp == 1) 
      for (i = 0; i < 4; i++)
        profile |= ((unsigned char *) &chord)[i];
    else if (bpp == 2) 
      for (i = 0; i < 2; i++)
        profile |= ((unsigned short *) &chord)[i];
    else //  bpp == 4
      profile = chord;

    for (bit = 0; bit < 32; bit++)
      { if (profile == 0)
          break;
        profile >>= 1;
      }
  }

  if (bpp == 2)
    { unsigned short *target = (unsigned short *) (channel->plane);
      if (bit <= 9)
        { for (i = 0; i < area; i++)
            array[target[i]] += 1;
          bit = 9;
        }
      else
        { int shift = bit-9;
          for (i = 0; i < area; i++)
            array[target[i] >> shift] += 1;
        }
    }
  else //  bpp == 4
    { unsigned int *target = (unsigned int *) (channel->plane);
      if (bit <= 9)
        { for (i = 0; i < area; i++)
            array[target[i]] += 1;
          bit = 9;
        }
      else
        { int shift = bit-9;
          for (i = 0; i < area; i++)
            array[target[i] >> shift] += 1;
        }
    }

  histogram->bitshift = bit-9;

  return (histogram);
}

void Histogram_Tiff_Image_Channels(Tiff_Image *image)
{ int i;

  for (i = 0; i < image->number_channels; i++)
    if (image->channels[i]->histogram == NULL)
      image->channels[i]->histogram = Histogram_Tiff_Channel(image->channels[i]);
}

void Tiff_Histogram_Merge(Tiff_Histogram *h1, Tiff_Histogram *h2)
{ unsigned int *array1, *array2;
  int           bit1,    bit2;
  int           block;
  int           j, k, b;
 
  array1 = h1->counts;
  array2 = h2->counts;

  bit1 = h1->bitshift;
  bit2 = h2->bitshift;

  if (bit2 > bit1)
    { block = (1 << (bit2-bit1));
      for (j = 0, b = 0; j < 512; j += block, b += 1)
        { array1[b] = array1[j] + array2[b];
          for (k = j+1; k < j+block; k++)
            array1[b] += array1[k];
        }
      for (j = b; j < 512; j++)
        array1[j] = array2[j];
      h1->bitshift = bit2;
    }
  else if (bit2 < bit1)
    { block = (1 << (bit1-bit2)); 
      for (j = 0, b = 0; j < 512; j += block, b += 1)
        { array1[b] += array2[j];
          for (k = j+1; k < j+block; k++)
            array1[b] += array2[k];
        }
    }
  else
    for (j = 0; j < 512; j++)
      array1[j] += array2[j];

  h1->total += h2->total;
}
