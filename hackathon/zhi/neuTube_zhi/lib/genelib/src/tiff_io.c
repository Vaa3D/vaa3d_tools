/*****************************************************************************************\
*                                                                                         *
*  Basic Tiff Reader and Writer (Tiff 6.0)                                                *
*    This program reads and writes .tif files without really understanding anything more  *
*    than that it is a header that has a linked lists of image file descriptors (ifd's)   *
*    that contain a collection of tags, one of which is an array pointing to blocks of    *
*    image data (TIFF_TILE_OFFSETS or TIFF_STRIP_OFFSETS) and another of which is an      *
*    array of byte counts for each block (TIFF_TILE_BYTE_COUNTS or TIFF_STRIP_BYTE_COUNTS,*
*    respectively).                                                                       *
*                                                                                         *
*    You can add, delete, and modify tags from each image as you wish.                    *
*                                                                                         *
*    Most importantly, we have "Tiff_Annotator" routines that allow one to efficiently    *
*      write  a TIFF_JF_TAGGER tag that points at a string buffer that is always          *
*      guaranteed to be at the end of the file so that changing it just is a matter of    *
*      rewriting the last bit of the file.                                                *
*                                                                                         *
*    The routines can also read and write .lsm files, consolidating them as proper tifs   *
*      once read and minimally reintroducing the essential "quirks" that an .lsm reader   *
*      would expect when writing back out (as a .lsm)                                     *
*                                                                                         *
*  Author:  Gene Myers                                                                    *
*  Date  :  January 2008                                                                  *
*                                                                                         *
\*****************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef _MSC_VER
#include <io.h>
#define ftruncate _chsize
#else
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "utilities.h"
#include "tiff_io.h"

//  This routine in tiff_image.p is needed here for .lsm conversion

extern int LZW_Decoder(unsigned char *source, unsigned char *target);


/****************************************************************************************
 *                                                                                      *
 *  HEADERS AND INTERNAL DATA TYPES                                                     *
 *                                                                                      *
 ****************************************************************************************/

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

typedef struct
  { uint16  label;
    uint16  type;
    uint32  count;
    uint32  value;    //  Can be packed with 2 shorts, 4 bytes, or is word-boundary offset to array
  } Tif_Tag;          //    See Tiff 6.0 documentation

static int type_sizes[13] = //  Sizes in bytes of the tiff types (see tiff_io.h)
                            { 0, 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8 };

#define POINTER(e)    ( & ((e)->value) )

#define VOID_PTR(e)   ((void *) POINTER(e))
#define SHORT_PTR(e)  ((uint16 *) POINTER(e))
#define LONG_PTR(e)   ((uint32 *) POINTER(e))

#define WORD_MULTIPLE(x) ((((x)-1)/4+1)*4)  // Will align values internally on word boundaries

typedef struct
  { int     data_flip;  //  The image data needs to be endian-flipped when interpreted

    int     numtags;    //  Number of tags in the IFD
    int     initags;    //  Tags in the prefix are in sorted order of label
    int     maxtags;    //  Have room for this many (maxtags >= numtags >= initags)

    Tif_Tag *tags;      //  tags[0..maxtags] of tags the first numtags of which are being used

    int      vmax;      //  max size of the value block (vsize <= veof <= vmax)
    int      veof;      //  current highwater mark of value block
    int      vsize;     //  current sum of the size of all values in the block
    uint8   *values;    //  concatenation of all value vectors

    int      dsize;     //  size of data block
    uint8   *data;      //  concatenation of all tiles/strips encoding the image
  } TIFD;

typedef struct
  { int      flip;       //  bytes need to be flipped to get machine endian
    int      ifd_no;     //  # of next ifd to be read (start counting at 1)
    int      lsm;        //  this is an lsm file and you have to do hideous things to read it
    uint32   first_ifd;  //  offset of the first ifd in file
    uint32   ifd_offset; //  offset of ifd to be read next
    int      file_size;  //  size of the file in bytes (needed to guard against lsm counts)
    FILE    *input;
  } Treader;

typedef struct
  { int      flip;        //  bytes need to be flipped to get machine endian
    int      ifd_no;      //  # of next ifd to be written (start counting at 1)
    int      lsm;         //  this is an lsm file and you have to do hideous things as a result
    uint32   eof_offset;  //  current offset to end of what is written.
    uint32   ifd_linko;   //  offset of last ifd link
    uint32   ano_count;   //  length of annotation
    uint32   ano_linko;   //  offset of annotation link (0 => not present)
    char    *annotation;  //  memory block holding annotation
    FILE    *output;
  } Twriter;

typedef struct
  { int      flip;        //  bytes need to be flipped to get machine endian
    uint32   ano_cnto;    //  offset of annotation count
    uint32   ano_offset;  //  offset of annotation block
    uint32   ano_count;   //  length of annotation
    char    *annotation;  //  memory block holding annotation
    FILE    *inout;
  } Tannotator;


/****************************************************************************************
 *                                                                                      *
 *  LSM DECODING INFORMATION                                                            *
 *                                                                                      *
 ****************************************************************************************/

#define LSM_TAG_COUNT           500  // size of LSM information master block

#define LSM_NO_CHANNELS           5  // offset to an int containing the # of channels
#define LSM_SCAN_TYPE            44  // offset to a short containing the scan type

// Offsets within the master block of offsets to secondary blocks and comments on how to
//   compute their sizes

#define LSM_VECTOR_OVERLAY       24  // 2nd-int
#define LSM_INPUT_LUT            25  // 1st-int
#define LSM_OUTPUT_LUT           26  // 1st-int
#define LSM_CHANNEL_COLORS       27  // 1st-int + LSM_NO_CHANNELS  ???
#define LSM_CHANNEL_DATA_TYPES   30  // 4*LSM_NO_CHANNELS
#define LSM_SCAN_INFORMATION     31  // 3rd-int + 12
#define LSM_KS_DATA              32  // not documented, zero  ???
#define LSM_TIME_STAMPS          33  // 1st-int
#define LSM_EVENT_LIST           34  // 1st-int
#define LSM_ROI                  35  // 2nd-int
#define LSM_BLEACH_ROI           36  // 2nd-int
#define LSM_NEXT_RECORDING       37  // another entire tiff, geez, zero
#define LSM_MEAN_OF_ROIS_OVERLAY 45  // 2nd-int (iff SCAN_TYPE = 5), 0 otherwise
#define LSM_TOPO_ISOLINE_OVERLAY 46  // 2nd-int
#define LSM_TOPO_PROFILE_OVERLAY 47  // 2nd-int
#define LSM_LINE_SCAN_OVERLAY    48  // 2nd-int
#define LSM_CHANNEL_WAVELENGTH   50  // 16*1st-int + 4
#define LSM_CHANNEL_FACTORS      51  // 24*LSM_NO_CHANNELS
#define LSM_UNMIX_PARAMETERS     54  // not documented, zero  ???
#define LSM_ASCII_INFORMATION    56  // 1st-int  (reverse engineered)
#define LSM_ASCII_SIGNATURE      57  // 1st-int  (reverse engineered)

static int lsm_offset_list[] = { LSM_VECTOR_OVERLAY, LSM_INPUT_LUT, LSM_OUTPUT_LUT,
                                 LSM_CHANNEL_COLORS, LSM_CHANNEL_DATA_TYPES, LSM_SCAN_INFORMATION,
                                 LSM_KS_DATA, LSM_TIME_STAMPS, LSM_EVENT_LIST, LSM_ROI,
                                 LSM_BLEACH_ROI, LSM_NEXT_RECORDING, LSM_MEAN_OF_ROIS_OVERLAY,
                                 LSM_TOPO_ISOLINE_OVERLAY, LSM_TOPO_PROFILE_OVERLAY,
                                 LSM_LINE_SCAN_OVERLAY, LSM_CHANNEL_WAVELENGTH,
                                 LSM_CHANNEL_FACTORS, LSM_UNMIX_PARAMETERS, LSM_ASCII_INFORMATION,
                                 LSM_ASCII_SIGNATURE, 0
                               };

static int lsm_1st_size[] = { LSM_INPUT_LUT, LSM_OUTPUT_LUT, LSM_TIME_STAMPS, LSM_EVENT_LIST,
                              LSM_ASCII_INFORMATION, LSM_ASCII_SIGNATURE, 0 };

static int lsm_2nd_size[] = { LSM_VECTOR_OVERLAY, LSM_ROI, LSM_BLEACH_ROI,
                              LSM_TOPO_ISOLINE_OVERLAY, LSM_TOPO_PROFILE_OVERLAY,
                              LSM_LINE_SCAN_OVERLAY, 0
                            };

static int lsm_zero_size[] = { LSM_KS_DATA, LSM_NEXT_RECORDING, LSM_UNMIX_PARAMETERS, 0 };


/****************************************************************************************
 *                                                                                      *
 *  BASIC OBJECT MEMORY MANAGEMENT                                                      *
 *                                                                                      *
 ****************************************************************************************/


typedef struct __Treader
  { struct __Treader *next;
    Treader           treader;
  } _Treader;

static _Treader *Free_Treader_List = NULL;
static size_t    Treader_Offset, Treader_Inuse;

static inline Treader *new_treader(char *routine)
{ _Treader *object;

  if (Free_Treader_List == NULL)
    { object = (_Treader *) Guarded_Malloc(sizeof(_Treader),routine);
      Treader_Offset = ((char *) &(object->treader)) - ((char *) object);
    }
  else
    { object = Free_Treader_List;
      Free_Treader_List = object->next;
    }
  Treader_Inuse += 1;
  return (&(object->treader));
}

static inline Treader *copy_treader(Treader *treader)
{ Treader *copy = new_treader("Copy_Tiff_Reader");
  *copy = *treader;
  return (copy);
}

Tiff_Reader *Copy_Tiff_Reader(Tiff_Reader *tiff_reader)
{ return ((Tiff_Reader *) copy_treader((Treader *) tiff_reader)); }

static inline void free_treader(Treader *treader)
{ _Treader *object  = (_Treader *) (((char *) treader) - Treader_Offset);
  object->next = Free_Treader_List;
  Free_Treader_List = object;
  Treader_Inuse -= 1;
}

static inline void kill_treader(Treader *treader)
{
  free(((char *) treader) - Treader_Offset);
  Treader_Inuse -= 1;
}

static inline void reset_treader()
{ _Treader *object;
  while (Free_Treader_List != NULL)
    { object = Free_Treader_List;
      Free_Treader_List = object->next;
      kill_treader(&(object->treader));
      Treader_Inuse += 1;
    }
}

void Reset_Tiff_Reader()
{ reset_treader(); }

int Tiff_Reader_Usage()
{ return (Treader_Inuse); }

void Free_Tiff_Reader(Tiff_Reader *tif)
{ Treader *rtif = (Treader *) tif;
  fclose(rtif->input);
  free_treader(rtif);
}

void Kill_Tiff_Reader(Tiff_Reader *tif)
{ Treader *rtif = (Treader *) tif;
  fclose(rtif->input);
  kill_treader(rtif);
}

static inline int twriter_asize(Twriter *tif)
{ return (tif->ano_count); }


typedef struct __Twriter
  { struct __Twriter *next;
    size_t               asize;
    Twriter           twriter;
  } _Twriter;

static _Twriter *Free_Twriter_List = NULL;
static size_t    Twriter_Offset, Twriter_Inuse;

static inline void allocate_twriter_annotation(Twriter *twriter, size_t asize, char *routine)
{ _Twriter *object  = (_Twriter *) (((char *) twriter) - Twriter_Offset);
  if (object->asize < asize)
    { if (object->asize == 0)
        object->twriter.annotation = NULL;
#ifdef _MSC_VER
      object->twriter.annotation  = (decltype(object->twriter.annotation))Guarded_Realloc(object->twriter.annotation,asize,routine);
#else
      object->twriter.annotation  = Guarded_Realloc(object->twriter.annotation,asize,routine);
#endif
      object->asize = asize;
    }
}

static inline Twriter *new_twriter(size_t asize, char *routine)
{ _Twriter *object;

  if (Free_Twriter_List == NULL)
    { object = (_Twriter *) Guarded_Malloc(sizeof(_Twriter),routine);
      Twriter_Offset = ((char *) &(object->twriter)) - ((char *) object);
      object->asize = 0;
    }
  else
    { object = Free_Twriter_List;
      Free_Twriter_List = object->next;
    }
  Twriter_Inuse += 1;
  allocate_twriter_annotation(&(object->twriter),asize,routine);
  return (&(object->twriter));
}

static inline Twriter *copy_twriter(Twriter *twriter)
{ Twriter *copy = new_twriter(twriter_asize(twriter),"Copy_Tiff_Writer");
  Twriter  temp = *copy;
  *copy = *twriter;
  copy->annotation = temp.annotation;
  if (twriter_asize(twriter) != 0)
    memcpy(copy->annotation,twriter->annotation,twriter_asize(twriter));
  return (copy);
}

Tiff_Writer *Copy_Tiff_Writer(Tiff_Writer *tiff_writer)
{ return ((Tiff_Writer *) copy_twriter((Twriter *) tiff_writer)); }

static inline void pack_twriter(Twriter *twriter)
{ _Twriter *object  = (_Twriter *) (((char *) twriter) - Twriter_Offset);
  if (object->asize > twriter_asize(twriter))
    { object->asize = twriter_asize(twriter);
      if (object->asize != 0)
#ifdef _MSC_VER
        object->twriter.annotation = (decltype(object->twriter.annotation))Guarded_Realloc(object->twriter.annotation,
#else
        object->twriter.annotation = Guarded_Realloc(object->twriter.annotation,
#endif
                                                     object->asize,"Pack_Twriter");
      else
        { free(object->twriter.annotation);
          object->asize = 0;
        }
    }
}

void Pack_Tiff_Writer(Tiff_Writer *tiff_writer)
{ pack_twriter(((Twriter *) tiff_writer)); }

static inline void free_twriter(Twriter *twriter)
{ _Twriter *object  = (_Twriter *) (((char *) twriter) - Twriter_Offset);
  object->next = Free_Twriter_List;
  Free_Twriter_List = object;
  Twriter_Inuse -= 1;
}

void Free_Tiff_Writer(Tiff_Writer *tiff_writer)
{ free_twriter(((Twriter *) tiff_writer)); }

static inline void kill_twriter(Twriter *twriter)
{ _Twriter *object  = (_Twriter *) (((char *) twriter) - Twriter_Offset);
  if (object->asize != 0)
    free(twriter->annotation);
  free(((char *) twriter) - Twriter_Offset);
  Twriter_Inuse -= 1;
}

void Kill_Tiff_Writer(Tiff_Writer *tiff_writer)
{ kill_twriter(((Twriter *) tiff_writer)); }

static inline void reset_twriter()
{ _Twriter *object;
  while (Free_Twriter_List != NULL)
    { object = Free_Twriter_List;
      Free_Twriter_List = object->next;
      kill_twriter(&(object->twriter));
      Twriter_Inuse += 1;
    }
}

void Reset_Tiff_Writer()
{ reset_twriter(); }

int Tiff_Writer_Usage()
{ return (Twriter_Inuse); }

static inline int tannotator_asize(Tannotator *tif)
{ return (tif->ano_count); }


typedef struct __Tannotator
  { struct __Tannotator *next;
    size_t                  asize;
    Tannotator           tannotator;
  } _Tannotator;

static _Tannotator *Free_Tannotator_List = NULL;
static size_t    Tannotator_Offset, Tannotator_Inuse;

static inline void allocate_tannotator_annotation(Tannotator *tannotator, size_t asize, char *routine)
{ _Tannotator *object  = (_Tannotator *) (((char *) tannotator) - Tannotator_Offset);
  if (object->asize < asize)
    { if (object->asize == 0)
        object->tannotator.annotation = NULL;
#ifdef _MSC_VER
      object->tannotator.annotation  = (decltype(object->tannotator.annotation))Guarded_Realloc(object->tannotator.annotation,asize,routine);
#else
      object->tannotator.annotation  = Guarded_Realloc(object->tannotator.annotation,asize,routine);
#endif
      object->asize = asize;
    }
}

static inline Tannotator *new_tannotator(size_t asize, char *routine)
{ _Tannotator *object;

  if (Free_Tannotator_List == NULL)
    { object = (_Tannotator *) Guarded_Malloc(sizeof(_Tannotator),routine);
      Tannotator_Offset = ((char *) &(object->tannotator)) - ((char *) object);
      object->asize = 0;
    }
  else
    { object = Free_Tannotator_List;
      Free_Tannotator_List = object->next;
    }
  Tannotator_Inuse += 1;
  allocate_tannotator_annotation(&(object->tannotator),asize,routine);
  return (&(object->tannotator));
}

static inline Tannotator *copy_tannotator(Tannotator *tannotator)
{ Tannotator *copy = new_tannotator(tannotator_asize(tannotator),"Copy_Tiff_Annotator");
  Tannotator  temp = *copy;
  *copy = *tannotator;
  copy->annotation = temp.annotation;
  if (tannotator_asize(tannotator) != 0)
    memcpy(copy->annotation,tannotator->annotation,tannotator_asize(tannotator));
  return (copy);
}

Tiff_Annotator *Copy_Tiff_Annotator(Tiff_Annotator *tiff_annotator)
{ return ((Tiff_Annotator *) copy_tannotator((Tannotator *) tiff_annotator)); }

static inline void pack_tannotator(Tannotator *tannotator)
{ _Tannotator *object  = (_Tannotator *) (((char *) tannotator) - Tannotator_Offset);
  if (object->asize > tannotator_asize(tannotator))
    { object->asize = tannotator_asize(tannotator);
      if (object->asize != 0)
#ifdef _MSC_VER
        object->tannotator.annotation = (decltype(object->tannotator.annotation))Guarded_Realloc(object->tannotator.annotation,
#else
        object->tannotator.annotation = Guarded_Realloc(object->tannotator.annotation,
#endif
                                                        object->asize,"Pack_Tannotator");
      else
        { free(object->tannotator.annotation);
          object->asize = 0;
        }
    }
}

void Pack_Tiff_Annotator(Tiff_Annotator *tiff_annotator)
{ pack_tannotator(((Tannotator *) tiff_annotator)); }

static inline void free_tannotator(Tannotator *tannotator)
{ _Tannotator *object  = (_Tannotator *) (((char *) tannotator) - Tannotator_Offset);
  object->next = Free_Tannotator_List;
  Free_Tannotator_List = object;
  Tannotator_Inuse -= 1;
}

static inline void kill_tannotator(Tannotator *tannotator)
{ _Tannotator *object  = (_Tannotator *) (((char *) tannotator) - Tannotator_Offset);
  if (object->asize != 0)
    free(tannotator->annotation);
  free(((char *) tannotator) - Tannotator_Offset);
  Tannotator_Inuse -= 1;
}

static inline void reset_tannotator()
{ _Tannotator *object;
  while (Free_Tannotator_List != NULL)
    { object = Free_Tannotator_List;
      Free_Tannotator_List = object->next;
      kill_tannotator(&(object->tannotator));
      Tannotator_Inuse += 1;
    }
}

void Reset_Tiff_Annotator()
{ reset_tannotator(); }

int Tiff_Annotator_Usage()
{ return (Tannotator_Inuse); }

void Free_Tiff_Annotator(Tiff_Annotator *tif)
{ Tannotator *atif = (Tannotator *) tif;
  fclose(atif->inout);
  free_tannotator(atif);
}

void Kill_Tiff_Annotator(Tiff_Annotator *tif)
{ Tannotator *atif = (Tannotator *) tif;
  fclose(atif->inout);
  kill_tannotator(atif);
}

static uint32 *get_ifd_vector(int size, char *routine)
{ static uint32 *IFD_Vector = NULL;
  static int     IFD_VecMax = 0;

  if (size < 0)
    { free(IFD_Vector);
      IFD_VecMax = 0;
      IFD_Vector = NULL;
    }
  else if (size > IFD_VecMax)
    { IFD_VecMax = 1.2*size + 50;
      IFD_Vector = (uint32 *) Guarded_Realloc(IFD_Vector,IFD_VecMax,routine);
    }
  return (IFD_Vector);
}

static uint8 *get_lsm_decode(int size, char *routine)
{ static uint8 *LSM_Decode_Block = NULL;
  static int    LSM_DecodeMax    = 0;

  if (size < 0)
    { free(LSM_Decode_Block);
      LSM_DecodeMax    = 0;
      LSM_Decode_Block = NULL;
    }
  else if (size > LSM_DecodeMax)
    { LSM_DecodeMax    = 1.2*size + 50;
      LSM_Decode_Block = (uint8 *) Guarded_Realloc(LSM_Decode_Block,LSM_DecodeMax,routine);
    }
  return (LSM_Decode_Block);
}

static inline int tifd_tsize(TIFD *tifd)
{ return (sizeof(Tif_Tag)*tifd->maxtags); }

static inline int tifd_dsize(TIFD *tifd)
{ return (tifd->dsize); }

static inline int tifd_vsize(TIFD *tifd)
{ return (tifd->vmax); }


typedef struct __TIFD
  { struct __TIFD *next;
    size_t            tsize;
    size_t            vsize;
    size_t            dsize;
    TIFD           tifd;
  } _TIFD;

static _TIFD *Free_TIFD_List = NULL;
static size_t    TIFD_Offset, TIFD_Inuse;

static inline void allocate_tifd_tags(TIFD *tifd, size_t tsize, char *routine)
{ _TIFD *object  = (_TIFD *) (((char *) tifd) - TIFD_Offset);
  if (object->tsize < tsize)
    { if (object->tsize == 0)
        object->tifd.tags = NULL;
#ifdef _MSC_VER
      object->tifd.tags  = (decltype(object->tifd.tags))Guarded_Realloc(object->tifd.tags,tsize,routine);
#else
      object->tifd.tags  = Guarded_Realloc(object->tifd.tags,tsize,routine);
#endif
      object->tsize = tsize;
    }
}

static inline void allocate_tifd_values(TIFD *tifd, size_t vsize, char *routine)
{ _TIFD *object  = (_TIFD *) (((char *) tifd) - TIFD_Offset);
  if (object->vsize < vsize)
    { if (object->vsize == 0)
        object->tifd.values = NULL;
#ifdef _MSC_VER
      object->tifd.values  = (decltype(object->tifd.values))Guarded_Realloc(object->tifd.values,vsize,routine);
#else
      object->tifd.values  = Guarded_Realloc(object->tifd.values,vsize,routine);
#endif
      object->vsize = vsize;
    }
}

static inline void allocate_tifd_data(TIFD *tifd, size_t dsize, char *routine)
{ _TIFD *object  = (_TIFD *) (((char *) tifd) - TIFD_Offset);
  if (object->dsize < dsize)
    { if (object->dsize == 0)
        object->tifd.data = NULL;
#ifdef _MSC_VER
      object->tifd.data  = (decltype(object->tifd.data))Guarded_Realloc(object->tifd.data,dsize,routine);
#else
      object->tifd.data  = Guarded_Realloc(object->tifd.data,dsize,routine);
#endif
      object->dsize = dsize;
    }
}

static inline TIFD *new_tifd(size_t tsize, size_t vsize, size_t dsize, char *routine)
{ _TIFD *object;

  if (Free_TIFD_List == NULL)
    { object = (_TIFD *) Guarded_Malloc(sizeof(_TIFD),routine);
      TIFD_Offset = ((char *) &(object->tifd)) - ((char *) object);
      object->tsize = 0;
      object->vsize = 0;
      object->dsize = 0;
    }
  else
    { object = Free_TIFD_List;
      Free_TIFD_List = object->next;
    }
  TIFD_Inuse += 1;
  allocate_tifd_tags(&(object->tifd),tsize,routine);
  allocate_tifd_values(&(object->tifd),vsize,routine);
  allocate_tifd_data(&(object->tifd),dsize,routine);
  return (&(object->tifd));
}

static inline TIFD *copy_tifd(TIFD *tifd)
{ TIFD *copy = new_tifd(tifd_tsize(tifd),tifd_vsize(tifd),tifd_dsize(tifd),"Copy_Tiff_IFD");
  TIFD  temp = *copy;
  *copy = *tifd;
  copy->tags = temp.tags;
  if (tifd_tsize(tifd) != 0)
    memcpy(copy->tags,tifd->tags,tifd_tsize(tifd));
  copy->values = temp.values;
  if (tifd_vsize(tifd) != 0)
    memcpy(copy->values,tifd->values,tifd_vsize(tifd));
  copy->data = temp.data;
  if (tifd_dsize(tifd) != 0)
    memcpy(copy->data,tifd->data,tifd_dsize(tifd));
  return (copy);
}

Tiff_IFD *Copy_Tiff_IFD(Tiff_IFD *tiff_ifd)
{ return ((Tiff_IFD *) copy_tifd((TIFD *) tiff_ifd)); }

static inline void pack_tifd(TIFD *tifd)
{ _TIFD *object  = (_TIFD *) (((char *) tifd) - TIFD_Offset);
  if (object->tsize > tifd_tsize(tifd))
    { object->tsize = tifd_tsize(tifd);
      if (object->tsize != 0)
#ifdef _MSC_VER
        object->tifd.tags = (decltype(object->tifd.tags))Guarded_Realloc(object->tifd.tags,
#else
        object->tifd.tags = Guarded_Realloc(object->tifd.tags,
#endif
                                            object->tsize,"Pack_TIFD");
      else
        { free(object->tifd.tags);
          object->tsize = 0;
        }
    }
  if (object->vsize > tifd_vsize(tifd))
    { object->vsize = tifd_vsize(tifd);
      if (object->vsize != 0)
#ifdef _MSC_VER
        object->tifd.values = (decltype(object->tifd.values))Guarded_Realloc(object->tifd.values,
#else
        object->tifd.values = Guarded_Realloc(object->tifd.values,
#endif
                                              object->vsize,"Pack_TIFD");
      else
        { free(object->tifd.values);
          object->vsize = 0;
        }
    }
  if (object->dsize > tifd_dsize(tifd))
    { object->dsize = tifd_dsize(tifd);
      if (object->dsize != 0)
#ifdef _MSC_VER
        object->tifd.data = (decltype(object->tifd.data))Guarded_Realloc(object->tifd.data,
#else
        object->tifd.data = Guarded_Realloc(object->tifd.data,
#endif
                                            object->dsize,"Pack_TIFD");
      else
        { free(object->tifd.data);
          object->dsize = 0;
        }
    }
}

void Pack_Tiff_IFD(Tiff_IFD *tiff_ifd)
{ pack_tifd(((TIFD *) tiff_ifd)); }

static inline void free_tifd(TIFD *tifd)
{ _TIFD *object  = (_TIFD *) (((char *) tifd) - TIFD_Offset);
  object->next = Free_TIFD_List;
  Free_TIFD_List = object;
  TIFD_Inuse -= 1;
}

void Free_Tiff_IFD(Tiff_IFD *tiff_ifd)
{ free_tifd(((TIFD *) tiff_ifd)); }

static inline void kill_tifd(TIFD *tifd)
{ _TIFD *object  = (_TIFD *) (((char *) tifd) - TIFD_Offset);
  if (object->dsize != 0)
    free(tifd->data);
  if (object->vsize != 0)
    free(tifd->values);
  if (object->tsize != 0)
    free(tifd->tags);
  free(((char *) tifd) - TIFD_Offset);
  TIFD_Inuse -= 1;
}

void Kill_Tiff_IFD(Tiff_IFD *tiff_ifd)
{ kill_tifd(((TIFD *) tiff_ifd)); }

static inline void reset_tifd()
{ _TIFD *object;
  while (Free_TIFD_List != NULL)
    { object = Free_TIFD_List;
      Free_TIFD_List = object->next;
      kill_tifd(&(object->tifd));
      TIFD_Inuse += 1;
    }
}

int Tiff_IFD_Usage()
{ return (TIFD_Inuse); }

void Reset_Tiff_IFD()
{ reset_tifd();
  get_ifd_vector(-1,NULL);
  get_lsm_decode(-1,NULL);
}


/****************************************************************************************
 *                                                                                      *
 *  UTILITIES FOR ENDIAN HANDLING                                                       *
 *                                                                                      *
 ****************************************************************************************/

static void flip_short(void *w)
{ uint8 *v = (uint8 *) w;
  uint8  x;
  
  x    = v[0];
  v[0] = v[1];
  v[1] = x;
}

static void flip_long(void *w)
{ uint8 *v = (uint8 *) w;
  uint8  x;

  x    = v[0];
  v[0] = v[3];
  v[3] = x;
  x    = v[1];
  v[1] = v[2];
  v[2] = x;
}

static void flip_quad(void *w)
{ uint8 *v = (uint8 *) w;
  uint8  x;

  x    = v[0];
  v[0] = v[7];
  v[7] = x;
  x    = v[1];
  v[1] = v[6];
  v[6] = x;
  x    = v[2];
  v[2] = v[5];
  v[5] = x;
  x    = v[3];
  v[3] = v[4];
  v[4] = x;
}

int Native_Endian()
{ uint32 x;
  uint8 *b;

  b = (uint8 *) (&x);
  x = 3;
  return (b[0] != 3);
}

static char *Tiff_Estring = NULL;

static void report_error(char *message)
{ 
  /*
  if (Tiff_Estring != NULL)
    free(Tiff_Estring);
  Tiff_Estring = Guarded_Strdup(message,"Tiff Error String");
  */
}

char *Tiff_Error_String()
{ return (Tiff_Estring); }

int Tiff_Is_LSM(void *rtif)
{ Treader *tif = (Treader *) rtif;
  return (tif->lsm);
}

#define INPUT_SHORT(var,escape)			\
  if (fread(&(var),2,1,input) != 1)		\
    { report_error("File ends prematurely");	\
      goto escape;				\
    }						\
  if (flip)					\
    flip_short(&(var));

#define INPUT_LONG(var,escape)			\
  if (fread(&(var),4,1,input) != 1)		\
    { report_error("File ends prematurely");	\
      goto escape;				\
    }						\
  if (flip)					\
    flip_long(&(var));

#define OUTPUT_SHORT(var)		\
  { uint16 _t = (var);			\
    if (flip)				\
      flip_short(&_t);                  \
    fwrite(&_t,2,1,output);		\
  }

#define OUTPUT_LONG(var)		\
  { uint32 _t = (var);			\
    if (flip)				\
      flip_long(&_t);                   \
    fwrite(&_t,4,1,output);		\
  }

/*tz-
static int optseek(FILE *file, int offset, int pos)
tz--*/
/* tz+ */
static int optseek(FILE *file, uint32 offset, int pos)
/* tz++ */
{ if (pos == SEEK_SET)
    { if (offset != ftell(file))
        return (fseek(file,offset,pos));
      else
        return (0);
    }
  else //  pos == SEEK_END or pos == SEEK_CUR
    return (fseek(file,offset,pos));
}

/****************************************************************************************
 *                                                                                      *
 *  TIFF_READER ROUTINES                                                                *
 *                                                                                      *
 ****************************************************************************************/

Tiff_Reader *Open_Tiff_Reader(char *name, int *big_endian, int lsm)
{ static int firstime = 1;
  static int mach_endian;
  static struct stat fdesc;

  Treader *tif;
  uint32   flip, offset;
  uint16   order;
  FILE    *input;

  if (firstime)
    { firstime = 0;
      mach_endian = Native_Endian();
    }

  input = fopen(name,"rb");
  if (input == NULL)
    { report_error("Cannot open file for reading");
      return (NULL);
    }

  if (fread(&order,2,1,input) != 1)
    goto invalid;
  if (order == 0x4949)
    { flip = mach_endian;
      if (big_endian != NULL)
        *big_endian = 0;
    }
  else if (order == 0x4D4D)
    { flip = 1-mach_endian;
      if (big_endian != NULL)
        *big_endian = 1;
    }
  else
    { report_error("Does not contain valid endian value");
      return (NULL);
    }

  INPUT_SHORT(order,invalid)

  if (order != 0x002A)
    { report_error("Does not contain magic key");
      return (NULL);
    }

  INPUT_LONG(offset,invalid)

  tif = new_treader("Open_Tiff_Reader");

  fstat(fileno(input),&fdesc);

  tif->flip       = flip;
  tif->first_ifd  = offset;
  tif->ifd_offset = offset;
  tif->ifd_no     = 1;
  tif->lsm        = lsm;
  tif->file_size  = fdesc.st_size;
  tif->input      = input;

  return ((Tiff_Reader *) tif);

invalid:
  return (NULL);
}

void Rewind_Tiff_Reader(Tiff_Reader *rtif)
{ Treader *tif = (Treader *) rtif;
  tif->ifd_no     = 1;
  tif->ifd_offset = tif->first_ifd;
}

int Advance_Tiff_Reader(Tiff_Reader *rtif)
{ Treader *tif = (Treader *) rtif;
  uint16   ntags;
  uint32   flip, offset;
  FILE    *input;

  if (tif->ifd_offset == 0)
    { report_error("Trying to advance at end-of-ifd-list");
      return (1);
    }

  input  = tif->input;
  flip   = tif->flip;
  offset = tif->ifd_offset;

  if (optseek(input,offset,SEEK_SET) < 0)
    { report_error("Seek for next IFD failed");
      return (1);
    }

  INPUT_SHORT(ntags,invalid)

  offset += 2 + ntags*12;

  if (optseek(input,offset,SEEK_SET) < 0)
    { report_error("Seek for next IFD offset failed");
      return (1);
    }

  INPUT_LONG(offset,invalid);

  tif->ifd_no    += 1;
  tif->ifd_offset = offset;
  return (0);

invalid:
  return (1);
}

int End_Of_Tiff(Tiff_Reader *tif)
{ return (((Treader *) tif)->ifd_offset == 0); }

void Close_Tiff_Reader(Tiff_Reader *etif)
{ Treader *tif = (Treader *) etif;
  fclose(tif->input);
}

/* For .lsm's produced by Zeiss: the ROWS_PER_STRIP tag is missing and equals IMAGE_LENGTH.
   For .lsm's subsequently written by this package, the ROWS_PER_STRIP tag is present, implying
   the file is a proper tif (including byte counts) save for the bloody BITS_PER_PIXEL value
*/

static int get_lsm_size(uint32 offset, int y, int flip, FILE *input)
{ uint32 size;

  if (optseek(input,offset+y,SEEK_SET) < 0)
    { report_error("Seek to lsm sub-block failed");
      return (-1);
    }
  if (fread(&size,4,1,input) != 1)
    { report_error("File ends prematurely");
      return (-1);
    }
  if (flip)
    flip_long(&size);
  return (size);
}

Tiff_IFD *Read_Tiff_IFD(Tiff_Reader *rtif)
{ Treader *tif = (Treader *) rtif;
  uint16   ntags;
  int      flip;
  Tif_Tag *off_tag, *cnt_tag;
  FILE    *input;
  TIFD    *ifd = NULL;

  Tif_Tag *bit_tag, *spp_tag, *map_tag, *lsm_tag;   //  lsm correction variables
  int      lsm_lzw, has_row;

  if (tif->ifd_offset == 0)
    { report_error("Trying to read when at end-of-ifd-list");
      return (NULL);
    }

  input = tif->input;
  flip  = tif->flip;

  // Seek to the next IFD, see how many tags it has,
  //   and allocate the ifd record and its tag array

  if (optseek(input,tif->ifd_offset,SEEK_SET) < 0)
    { report_error("Seek for next IFD failed");
      return (NULL);
    }

  INPUT_SHORT(ntags,invalid)
  
  ifd = new_tifd((ntags+10)*sizeof(Tif_Tag),0,0,"Read_Tiff_IFD");

  ifd->data_flip = flip; 
  ifd->numtags   = ntags;
  ifd->initags   = ntags;
  ifd->maxtags   = ntags+10;

  // Load the tags, endian treating all but the value field

  { int i, vsize, veof;

    bit_tag = NULL;   //  Initially all lsm variables are "off"
    spp_tag = NULL;
    map_tag = NULL;
    lsm_tag = NULL;
    lsm_lzw = 0;
    has_row = 0;

    vsize = 0;
    veof  = 0;
    for (i = 0; i < ntags; i++)

      { uint16 label, type;
        uint32 count, value;

        Tif_Tag *tag = ifd->tags+i;

        INPUT_SHORT(label,invalid);
        INPUT_SHORT(type,invalid);
        INPUT_LONG(count,invalid);
        if (fread(&value,4,1,input) != 1)
          { report_error("File ends prematurely");
            goto invalid;
          }

        if (type < TIFF_BYTE || type > TIFF_DOUBLE)
          { report_error("Illegal type in IFD tag");
            goto invalid;
          }

        //  If lsm then (a) may have to fix BITS_PER_SAMPLE tag if more than 1 channel,
        //    (b) may have to fix color map if present, and (c) will have to bundle lsm's
        //    special tag and its auxiliary blocks

        if (tif->lsm)
          { if (label == TIFF_SAMPLES_PER_PIXEL)
              spp_tag = tag;
            else if (label == TIFF_COLOR_MAP)
              map_tag = tag;
            else if (label == TIFF_CZ_LSMINFO && tif->ifd_no == 1)
              lsm_tag = tag;
            else if (label == TIFF_BITS_PER_SAMPLE && count > 1)
              bit_tag = tag;
            else if (label == TIFF_ROWS_PER_STRIP)
              has_row = 1;
          }

        tag->label       = label;
        tag->type        = type;
        tag->count       = count;
        tag->value       = value;
        //  LONG_PTR(tag)[0] = value; ???

        int size = count * type_sizes[type];
        if (size > 4 || bit_tag == tag)
          { veof  += WORD_MULTIPLE(size);
            vsize += size;
          }
      }

    ifd->vsize = vsize;
    ifd->veof  = veof;
    ifd->vmax  = veof + 1024;

    allocate_tifd_values(ifd,ifd->vmax,"Read_Tiff_IFD");
  }

  // Get the offset of the next IFD

  { uint32 offset;

    INPUT_LONG(offset,invalid);
    tif->ifd_no    += 1;
    tif->ifd_offset = offset;
  }

  // Get the value arrays of tags whose values don't fit in the value field, endian adjust
  //   the values in all cases, and incidentally note the count and offset tags and whether
  //   this is a compressed lsm

  { int i, veof;

    veof = 0;
    off_tag = cnt_tag = NULL;
    for (i = 0; i < ntags; i++)

      { Tif_Tag *tag = ifd->tags+i;
        int      tsize  = type_sizes[tag->type];
        int      esize  = tsize * tag->count;

        if (tag->label == TIFF_STRIP_OFFSETS || tag->label == TIFF_TILE_OFFSETS)
          { off_tag = tag;
            if (tag->type != TIFF_LONG &&
                     (tag->type != TIFF_SHORT || tag->label != TIFF_STRIP_OFFSETS))
              { report_error("Type of image offsets is not correct");
                goto invalid;
              }
          }
        else if (tag->label == TIFF_STRIP_BYTE_COUNTS || tag->label == TIFF_TILE_BYTE_COUNTS)
          { cnt_tag = tag;
            if (tag->type != TIFF_LONG && tag->type != TIFF_SHORT)
              { report_error("Type of image byte counts is not correct");
                goto invalid;
              }
          }
      
        if (esize <= 4 && tag != bit_tag)

          { if (flip)
              { if (tsize == 4)               //  value is in place, endian treat according to type
                  flip_long(LONG_PTR(tag));
                else if (tsize == 2)
                  { uint16 *valS = SHORT_PTR(tag);
                    flip_short(valS);
                    flip_short(valS+1);
                  }
              }
          }

        else
          { uint32 *valI;        //  value is in a block, seek to it, read into the end
            uint16 *valS;        //    of the values block, and endian treat its values
            double *valD;
            uint32     j;

            valI = LONG_PTR(tag);
            if (flip)
              flip_long(valI);
            if (optseek(input,valI[0],SEEK_SET) < 0)
              { report_error("Seek to tag value array failed");
                goto invalid;
              }

            valI = (uint32 *) (ifd->values+veof);
            if (fread(valI,esize,1,input) != 1)
              goto invalid;

            tag->value = veof;
            veof += WORD_MULTIPLE(esize);

            if (flip)
              switch (tsize)
              { case 2:
                  valS = (uint16 *) valI;
                  for (j = 0; j < tag->count; j++)
                    flip_short(valS+j);
                  break;
                case 4:
                  for (j = 0; j < tag->count; j++)
                    flip_long(valI+j);
                  break;
                case 8:
                  if (tag->type == TIFF_DOUBLE)
                    { valD = (double *) valI;
                      for (j = 0; j < tag->count; j++)
                        flip_quad(valD+j);
                    }
                  else  // tag->type in { TIFF_RATIONAL, TIFF_SRATIONAL }
                    { for (j = 0; j < 2*tag->count; j++)
                        flip_long(valI+j);
                    }
                  break;
              }
          }

        //  Set flag if lzw compressed lsm file that was not written by us (sigh)

        if (tif->lsm && tag->label == TIFF_COMPRESSION && *SHORT_PTR(tag) == TIFF_VALUE_LZW)
          lsm_lzw = 1 - has_row;
      }

    if (off_tag == NULL || cnt_tag == NULL)
      { report_error("IFD does not contain offset and/or byte count tags");
        goto invalid;
      }
    if ( ! ((off_tag->label == TIFF_STRIP_OFFSETS && cnt_tag->label == TIFF_STRIP_BYTE_COUNTS) ||
            (off_tag->label == TIFF_TILE_OFFSETS  && cnt_tag->label == TIFF_TILE_BYTE_COUNTS)))
      { report_error("IFD does not contain compatible offset and byte count tags");
        goto invalid;
      }
  }

  //  If this is an lsm file and the info tag is present, gather the blocks into one super block
  //    so that it becomes a proper tif

  /* Notes:
     * space for TOPO_ISOLINE_OVERLAY is twice what it needs to be (200 vs. 400), all other
         overlays are 200 and space for them is the same
     * the size of the CHANNEL_COLORS is coded as 70 but is in fact 72 bytes are needed and 138
         bytes are available to it.  Added NO_CHANNELS as I figure they just forgot to count the
         '\0's at the end of each color name.
     * KS_DATA and UNMIX_PARAMETERS are undocumented
  */

  if (lsm_tag != NULL)
    { int lflip, vsize, slot;
      int lsm_subsizes[LSM_ASCII_SIGNATURE+1];

      uint32 *valI = (uint32 *) (ifd->values + lsm_tag->value);

/*
      if (lsm_tag->count != LSM_TAG_COUNT)
        { report_error("LSM tag is not of the right length");
          goto invalid;
        }
*/

      { uint32 key;   //  Determine if multi-byte values need to be flipped

        key = valI[0];
        if (key == 0x0300494C || key == 0x0400494C)
          lflip = 0;
        else
          { lflip = 1;
            flip_long(&key);
            if (key != 0x0300494C && key != 0x0400494C)
              { report_error("LSM tag does not start with magic key");
                goto invalid;
              }
          }
      }

      if (lflip)       //  Flip all the offset vars in the main block if needed
        { int i, idx;

          for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
            flip_long(valI+idx);
          flip_long(valI+LSM_NO_CHANNELS);
          flip_short(((uint16 *) valI) + LSM_SCAN_TYPE);
        }

      //  Determine the sum of the sizes of all blocks

      vsize = LSM_TAG_COUNT;

      { int i, idx, offset;

#define LSM_FETCH(channel,disp,mul,add)				\
        if ((offset = valI[channel]) != 0)			\
          { int s = get_lsm_size(offset,disp,lflip,input);	\
            if (s < 0) goto invalid;				\
            lsm_subsizes[channel] = mul*s+add;			\
            vsize += lsm_subsizes[channel];			\
          }							\
        else							\
          lsm_subsizes[channel] = 0;

        for (i = 0; (idx = lsm_1st_size[i]) != 0; i++)
          LSM_FETCH(idx,0,1,0);

        for (i = 0; (idx = lsm_2nd_size[i]) != 0; i++)
          LSM_FETCH(idx,4,1,0);
  
        for (i = 0; (idx = lsm_zero_size[i]) != 0; i++)
          { valI[idx] = 0;
            lsm_subsizes[idx] = 0;
          }
      
        LSM_FETCH(LSM_CHANNEL_COLORS,0,1,valI[LSM_NO_CHANNELS])
        LSM_FETCH(LSM_CHANNEL_WAVELENGTH,0,16,4)

        if (((uint16 *) valI)[LSM_SCAN_TYPE] != 5)
          { valI[LSM_MEAN_OF_ROIS_OVERLAY] = 0;
            lsm_subsizes[LSM_MEAN_OF_ROIS_OVERLAY] = 0;
          }
        else
          LSM_FETCH(LSM_MEAN_OF_ROIS_OVERLAY,4,1,0)
      }

      vsize += (lsm_subsizes[LSM_CHANNEL_DATA_TYPES] = valI[LSM_NO_CHANNELS] * 4);
      vsize += (lsm_subsizes[LSM_CHANNEL_FACTORS] = valI[LSM_NO_CHANNELS] * 24);

      { uint32 vecI[3], depth;   //  Get size of SCAN_INFORMATION by traversing it (the only way!)
        uint32 size;
        
        if (optseek(input,valI[LSM_SCAN_INFORMATION],SEEK_SET) < 0)
          { report_error("Seek to lsm sub-block for scan information failed");
            goto invalid;
          }
        depth = 0;
        size  = 0;
        do
          { if (fread(vecI,12,1,input) != 1)
              { report_error("Read within lsm scan information sub-block failed");
                goto invalid;
              }
            if (lflip)
              { flip_long(vecI);
                flip_long(vecI+2);
              }
            if ((vecI[0] & 0xff) == 0)
              depth += 1;
            else if ((vecI[0] & 0xff) == 0xff)
              depth -= 1;
            size += 12 + vecI[2];
            if (optseek(input,vecI[2],SEEK_CUR) < 0)
              { report_error("Seek within lsm scan information sub-block failed");
                goto invalid;
              }
          }
        while (depth != 0);

        lsm_subsizes[LSM_SCAN_INFORMATION] = size;
        vsize += size;
      }

      // Allocate a space big enough for all the blocks and move them into it

      slot = ifd->veof;

      ifd->vsize += (vsize - LSM_TAG_COUNT);
      ifd->veof  += WORD_MULTIPLE(vsize);
      ifd->vmax   = ifd->veof + 1024;
  
      allocate_tifd_values(ifd,ifd->vmax,"Read_Tiff_IFD");

      valI = (uint32 *) (ifd->values + slot);
      memcpy(valI,ifd->values + lsm_tag->value,LSM_TAG_COUNT);
      lsm_tag->value = slot;
      slot += LSM_TAG_COUNT;
      lsm_tag->count = vsize;
  
      { int i, idx, offset;

        for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
          if ((offset = valI[idx]) != 0)
            { optseek(input,offset,SEEK_SET);
              fread(ifd->values+slot,lsm_subsizes[idx],1,input);
              valI[idx] = slot-lsm_tag->value;
              slot += lsm_subsizes[idx];
            }
      }

      if (lflip)
        { int i, idx;

          for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
            flip_long(valI+idx);
          flip_long(valI+LSM_NO_CHANNELS);
          flip_short(((uint16 *) valI) + LSM_SCAN_TYPE);
        }
    }

  //  Capture the image data in block "data" using the offset and byte count tags

  { int     ctype, otype;
    uint32 *cntI = NULL, *offI = NULL;
    uint16 *cntS = NULL, *offS = NULL;
    uint8  *decode = NULL;

    { uint32 i, csize, osize, dsize, cmax;

      ctype = (cnt_tag->type == TIFF_SHORT);
      csize = type_sizes[cnt_tag->type] * cnt_tag->count;

      otype = (off_tag->type == TIFF_SHORT);
      osize = type_sizes[off_tag->type] * off_tag->count;

      dsize = 0;
      cmax  = 0;
      if (ctype)
        { if (csize <= 4)                            // careful the arrays can be 'in place'!
            cntS = SHORT_PTR(cnt_tag);               //  (unlikely but possibly by spec.)
          else
            cntS = (uint16 *) (ifd->values + cnt_tag->value);
          for (i = 0; i < cnt_tag->count; i++)
            { uint32 c = cntS[i];
              dsize += c;
              if (c > cmax) cmax = c; 
            }
        }
      else
        { if (csize <= 4)
            cntI = LONG_PTR(cnt_tag);
          else
            cntI = (uint32 *) (ifd->values + cnt_tag->value);
          for (i = 0; i < cnt_tag->count; i++)
            { uint32 c = cntI[i];
              dsize += c;
              if (c > cmax) cmax = c; 
            }
        }

      if (lsm_lzw)
        decode = get_lsm_decode(cmax,"Read_Tiff_IFD");

      if (otype)
        { if (osize <= 4)                            // careful the arrays can be 'in place'!
            offS = SHORT_PTR(off_tag);               //  (unlikely but possibly by spec.)
          else
            offS = (uint16 *) (ifd->values + off_tag->value);
        }
      else
        { if (osize <= 4)
            offI = LONG_PTR(off_tag);
          else
            offI = (uint32 *) (ifd->values + off_tag->value);
        }
  
      allocate_tifd_data(ifd,dsize,"Read_Tiff_IFD");
      ifd->dsize = dsize;
    }

    { uint32 i, doff, csize, offset;
      uint32 fsize = tif->file_size;

      doff = 0;
      for (i = 0; i < off_tag->count; i++)
       { if (otype)
           { offset  = offS[i];
             offS[i] = doff;
           }
         else
           { offset  = offI[i];
             offI[i] = doff;
           }
         if (ctype)
           csize = cntS[i];
         else
           csize = cntI[i];

         if (offset + csize > fsize)   //  LSM correction: counts can be over-estimates
           csize = fsize-offset;       //  Trim them back if they go beyond end of file

         if (optseek(input,offset,SEEK_SET) < 0)
           goto invalid;
         if (fread(ifd->data + doff,csize,1,input) != 1)
           goto invalid;

         if (lsm_lzw)
           { csize = LZW_Decoder(ifd->data+doff,decode);
             if (ctype)
               cntS[i] = csize;
             else
               cntI[i] = csize;
           }

         doff += csize;
       }

      ifd->dsize = doff;
    }
  }

  // LSM corrections to BITS_PER_SAMPLE and COLOR_MAP, and add ROWS_PER_STRIP tag

  if (tif->lsm)

    { uint32 count = 1;

      if (bit_tag != NULL)    //  Fix the BITS_PER_SAMPLE tag if 2-channel
        { if (spp_tag != NULL)  
            count = *SHORT_PTR(spp_tag);
          else
            count = bit_tag->count;
          if (count <= 2)
            { uint16 *valS = (uint16 *) (ifd->values+bit_tag->value);
              uint16 *valT = SHORT_PTR(bit_tag);
              valT[0] = valS[0];
              valT[1] = valS[1];
              ifd->vsize -= bit_tag->count*2;
            }
          bit_tag->count = count;
        }

      if (map_tag != NULL && ! has_row)   //  Rescale the color map if the upper byte was not used.
        { int     highbit, i;
          uint16 *valS = (uint16 *) (ifd->values+map_tag->value);

          highbit = 0;
          for (i = 0; i < (int) map_tag->count; i++)
            if (valS[i] > 256)
              highbit = 1;
          if ( ! highbit)
            { for (i = 0; i < (int) map_tag->count; i++)
                valS[i] <<= 8;
            }
        }

      if ( ! has_row)
        { Tiff_Type type;
          int       count;
          uint32   *ptr;

          ptr  = (uint32 *) Get_Tiff_Tag(ifd,TIFF_IMAGE_LENGTH,&type,&count);
          if (ptr == NULL)
            goto invalid;
          Set_Tiff_Tag(ifd,TIFF_ROWS_PER_STRIP,TIFF_LONG,1,ptr);
        }

      if (spp_tag != NULL)
        count = *SHORT_PTR(spp_tag);
      if (count != 3)
        { count = TIFF_VALUE_BLACK_IS_ZERO;
          Set_Tiff_Tag(ifd,TIFF_PHOTOMETRIC_INTERPRETATION,TIFF_SHORT,1,&count);
        }
    }

  return ((Tiff_IFD *) ifd);

invalid:
  if (ifd != NULL)
    Free_Tiff_IFD(ifd);
  return (NULL);
}


/****************************************************************************************
 *                                                                                      *
 *  TIFF_IFD ROUTINES                                                                   *
 *                                                                                      *
 ****************************************************************************************/

Tiff_IFD *Create_Tiff_IFD(int num_tags)
{ TIFD *ifd;
  
  ifd = new_tifd((num_tags+10)*sizeof(Tif_Tag),1024,0,"Create_Tiff_IFD");

  ifd->data_flip = 0;
  ifd->numtags   = 0;
  ifd->initags   = 0;
  ifd->maxtags   = num_tags+10;

  ifd->vmax   = 1024;
  ifd->veof   = 0;
  ifd->vsize  = 0;

  ifd->dsize  = 0;

  return ((Tiff_IFD *) ifd);
}

void *Get_Tiff_Tag(Tiff_IFD *eifd, int label, Tiff_Type *type, int *count)
{ TIFD *ifd = (TIFD *) eifd;
  int   i;

  for (i = 0; i < ifd->numtags; i++)
    if (ifd->tags[i].label == label)
      { Tif_Tag *tag = ifd->tags+i;

        if (type != NULL)
          *type  = (Tiff_Type)tag->type;
        if (count != NULL)
          *count = tag->count; 
        if (type_sizes[tag->type]*tag->count <= 4)
          return (VOID_PTR(tag));
        else
          return (ifd->values + tag->value);
      }
  report_error("No such tag in IFD");
  return (NULL);
}

void Delete_Tiff_Tag(Tiff_IFD *eifd, int label)
{ TIFD *ifd = (TIFD *) eifd;
  int   i, esize;

  for (i = 0; i < ifd->numtags; i++)
    if (ifd->tags[i].label == label)
      { ifd->tags[i].label = 0;
        esize = ifd->tags[i].count * type_sizes[ifd->tags[i].type];
        if (esize > 4)
          ifd->vsize -= esize;
        break;
      }
}

int Set_Tiff_Tag(Tiff_IFD *eifd, int label, Tiff_Type type, int count, void *data)
{ TIFD    *ifd = (TIFD *) eifd;

  Tif_Tag *tag;
  void    *ptr = NULL;
  int      nsize, osize;
  int      i;

  if (count <= 0)
    { report_error("Non-positive count");
      return (1);
    }
  if (type < TIFF_BYTE || type > TIFF_DOUBLE)
    { report_error("Invalid type");
      return (1);
    }

  nsize = type_sizes[type]*count;
  for (i = 0; i < ifd->numtags; i++)
    if (ifd->tags[i].label == label)
      { tag   = ifd->tags+i;
        osize = type_sizes[tag->type]*tag->count;
        if (osize > 4)
          ifd->vsize -= osize;
        else
          ptr = ifd->values + tag->value;
        break;
      }
  if (i >= ifd->numtags)
    { if (i >= ifd->maxtags)
        { ifd->maxtags += 10;
          allocate_tifd_tags(ifd,ifd->maxtags*sizeof(Tif_Tag),"Set_Tiff_Tag");
        }
      ifd->numtags += 1;
      tag = ifd->tags + i; 
      tag->label = label;
      osize = 0;
    }
  tag->type  = type;
  tag->count = count; 

  if (nsize <= 4)
    ptr = VOID_PTR(tag);
  else
    { if (osize < nsize)
        { osize = WORD_MULTIPLE(nsize);
          if (ifd->veof + osize >= ifd->vmax) 
            { ifd->vmax = ifd->veof + osize + 1024;
              allocate_tifd_values(ifd,ifd->vmax,"Set_Tiff_Tag");
            }
          tag->value = ifd->veof;
          ifd->veof += osize;
        }
      ifd->vsize += nsize;
      ptr = ifd->values + tag->value; 
    }
  memcpy(ptr,data,nsize);

  return (0);
}

//  Seen by tiff_image.p but not publicly declared

unsigned char *Tiff_IFD_Data(Tiff_IFD *ifd)
{ return (((TIFD *) ifd)->data); }

void Allocate_Tiff_IFD_Data(Tiff_IFD *eifd, int size)
{ TIFD *ifd = (TIFD *) eifd;
  allocate_tifd_data(ifd,size,"Make_IFD_For_Image");
  ifd->dsize = size;
}

int Tiff_IFD_Data_Flip(Tiff_IFD *ifd)
{ return (((TIFD *) ifd)->data_flip); }


/****************************************************************************************
 *                                                                                      *
 *  TIFF_WRITER ROUTINES                                                                *
 *                                                                                      *
 ****************************************************************************************/

Tiff_Writer *Open_Tiff_Writer(char *name, int lsm)
{ Twriter *tif;
  FILE    *output;

  output = fopen(name,"wb");
  if (output == NULL)
    { report_error("Cannot open file for writing");
      return (NULL);
    }

  tif = new_twriter(0,"Open_Tiff_Writer");

  tif->ifd_no = 1;
  tif->output = output;
  tif->lsm    = lsm;

  return ((Tiff_Writer *) tif);
}

static void write_tiff_header(Twriter *tif, int flip)
{ static int firstime = 1;
  static int mach_endian;

  uint32   offset;
  uint16   order;
  FILE    *output;

  if (firstime)
    { firstime = 0;
      mach_endian = Native_Endian();
    }

  output = tif->output;
 
  if (flip == mach_endian)
    order = 0x4949;
  else
    order = 0x4D4D;
  fwrite(&order,2,1,output);

  OUTPUT_SHORT(0x002A)

  offset = 8;
  OUTPUT_LONG(offset)

  tif->flip       = flip;
  tif->eof_offset = 8;
  tif->ifd_linko  = 4;
  tif->ano_linko  = 0;
  tif->ano_count  = 0;
}

static int TAG_SORT(const void *x, const void *y)
{ Tif_Tag *a = (Tif_Tag *) x;
  Tif_Tag *b = (Tif_Tag *) y;
  return (a->label - b->label);
}

int Write_Tiff_IFD(Tiff_Writer *etif, Tiff_IFD *eifd)
{ TIFD    *ifd = (TIFD *) eifd;
  Twriter *tif = (Twriter *) etif;
  
  Tif_Tag *cnt_tag = NULL;
  Tif_Tag *off_tag = NULL;
  Tif_Tag *lsm_tag = NULL;
  Tif_Tag *bps_tag = NULL;

  int      ctype = 0;
  int      otype = 0;
  uint32  *cntI = NULL;
  uint16  *cntS = NULL;
  uint32  *offI = NULL;
  uint16  *offS = NULL;
  uint32   datasize;

  int      flip;
  uint32   offset;
  FILE    *output;
  uint32   vsize;

  if (tif->ifd_no == 1)
    write_tiff_header(tif,ifd->data_flip);
  else if (tif->flip != ifd->data_flip)
    { report_error("Writing IFD's with different endians to same file");
      return (1);
    }
 
  output = tif->output;
  flip   = tif->flip;

  { char     *desc;         //  Hack, ImageJ makes assumptions about the arrangement of blocks
    Tiff_Type type;         //    if it finds a description that it produced!  So get rid of it.
    int       count = 0;

    desc = (char*)Get_Tiff_Tag(eifd,TIFF_IMAGE_DESCRIPTION,&type,&count);
    if (count >= 6 && strncmp(desc,"ImageJ",6) == 0)
      Delete_Tiff_Tag(eifd,TIFF_IMAGE_DESCRIPTION);
  }

  { int i;

    for (i = 0; i < ifd->numtags; i++)   //  Remove any 0'd (deleted) tags
      if (ifd->tags[i].label == 0)
        { if (i < ifd->numtags-1)
            ifd->tags[i] = ifd->tags[ifd->numtags-1];
          ifd->numtags -= 1;
        }
  }

  if (ifd->numtags != ifd->initags)
    qsort(ifd->tags,ifd->numtags,sizeof(Tif_Tag),TAG_SORT);
  ifd->initags = ifd->numtags;

  { int i, j;

    vsize    = ifd->vsize;
    datasize = 0;
    for (i = 0; i < ifd->numtags; i++)
      { Tif_Tag *tag  = ifd->tags+i;
        int      label = tag->label;

        if (label == TIFF_STRIP_BYTE_COUNTS || label == TIFF_TILE_BYTE_COUNTS)
          { int csize;

            cnt_tag = tag;
            ctype   = (cnt_tag->type == TIFF_SHORT);
            csize   = type_sizes[cnt_tag->type] * cnt_tag->count;

            if (ctype)
              { if (csize <= 4)                          // careful the arrays can be 'in place'!
                  cntS = SHORT_PTR(cnt_tag);
                else
                  cntS = (uint16 *) (ifd->values + cnt_tag->value);
                for (j = 0; j < (int) cnt_tag->count; j++)
                  datasize += cntS[j];
              }
            else
              { if (csize <= 4)
                  cntI = LONG_PTR(cnt_tag);
                else
                  cntI = (uint32 *) (ifd->values + cnt_tag->value);
                for (j = 0; j < (int) cnt_tag->count; j++)
                  datasize += cntI[j];
              }
          }
        else if (label == TIFF_STRIP_OFFSETS || label == TIFF_TILE_OFFSETS)
          { int osize;

            off_tag = tag;
            otype   = (off_tag->type == TIFF_SHORT);
            osize   = type_sizes[off_tag->type] * off_tag->count;

            if (otype)
              { if (osize <= 4)                          // careful the arrays can be 'in place'!
                  offS = SHORT_PTR(off_tag);
                else
                  offS = (uint16 *) (ifd->values + off_tag->value);
              }
            else
              { if (osize <= 4)
                  offI = LONG_PTR(off_tag);
                else
                  offI = (uint32 *) (ifd->values + off_tag->value);
              }
          }
        else if (tif->ifd_no == 1 && label == TIFF_JF_TAGGER && tag->count > 4)
          vsize -= ifd->tags[i].count;
        else if (tif->lsm && label == TIFF_BITS_PER_SAMPLE && tag->count == 2)
          { bps_tag = tag;
            vsize  += 4;
          }
        else if (tif->lsm && label == TIFF_CZ_LSMINFO && tif->ifd_no == 1)
          lsm_tag = tag;
      }
  }

  if (tif->lsm && tif->ifd_no == 1 && lsm_tag == NULL)
    { report_error("First IFD does not contain an LSM info tag");
      return (1);
    }

  OUTPUT_SHORT(ifd->numtags)

  offset = tif->eof_offset + ifd->numtags*12 + 6;

  { int i, doff;

    doff = offset + vsize;
    for (i = 0; i < ifd->numtags; i++)
      { Tif_Tag *tag  = ifd->tags+i;
        int      tsize = type_sizes[tag->type];
        int      esize = tsize * tag->count;
        int      label = tag->label;

        OUTPUT_SHORT(tag->label)
        OUTPUT_SHORT(tag->type)
        if (tag == lsm_tag)
          OUTPUT_LONG(LSM_TAG_COUNT)
        else
          OUTPUT_LONG(tag->count)

        if (label == TIFF_JF_TAGGER && tif->ifd_no == 1)
          if (esize <= 4)
            fwrite(&(tag->value),4,1,output);
          else
            { tif->ano_linko = tif->eof_offset + 12*i + 10;
              tif->ano_count = tag->count;
              allocate_twriter_annotation(tif,tag->count,"Write_Tiff_IFD");
              memcpy(tif->annotation,ifd->values + tag->value,tag->count);
              OUTPUT_LONG(doff)   // place holder, will be filled in at end
            }
        else if (esize <= 4 && bps_tag != tag)
          { if (tag == off_tag)
              if (otype)
                { OUTPUT_SHORT(doff)
                  if (ctype)
                    doff += cntS[0];
                  else
                    doff += cntI[0];
                  OUTPUT_SHORT(doff)
                }
              else
                OUTPUT_LONG(doff)
            else if (tsize == 4)
              OUTPUT_LONG(tag->value)
            else if (tsize == 2)
              { uint16 *valS = SHORT_PTR(tag);
                OUTPUT_SHORT(valS[0])
                OUTPUT_SHORT(valS[1])
              }
            else
              fwrite(&(tag->value),4,1,output);
          }
        else
          { OUTPUT_LONG(offset)
            offset += esize;
          }
      }

    doff = offset + datasize;
    OUTPUT_LONG(doff)
  }

  { uint32  j, doff;
    int     i;
    uint32 *valI;
    uint16 *valS;
    double *valD;

    doff = offset;
    for (i = 0; i < ifd->numtags; i++)
      { Tif_Tag *tag   = ifd->tags+i;
        int      tsize = type_sizes[tag->type];
        int      esize = tsize * tag->count;

        if (tag->label == TIFF_JF_TAGGER && tif->ifd_no == 1)
          continue;

        if (tag == bps_tag)
          { uint16 *valS = SHORT_PTR(tag);
            OUTPUT_SHORT(valS[0])
            OUTPUT_SHORT(valS[1])
          }

        else if (tag == lsm_tag)
          { uint32 *valI = (uint32 *) (ifd->values + tag->value);
            int     i, idx, lflip, displace;
        
            lflip = (valI[0] != 0x0300494C && valI[0] != 0x0400494C);

            if (lflip)
              for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
                flip_long(valI+idx);

            displace = ftell(output);
            for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
              if (valI[idx] != 0)
                valI[idx] += displace;

            if (lflip)
              for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
                flip_long(valI+idx);

            fwrite(ifd->values + tag->value,esize,1,output); 

            if (lflip)
              for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
                flip_long(valI+idx);

            for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
              if (valI[idx] != 0)
                valI[idx] -= displace;

            if (lflip)
              for (i = 0; (idx = lsm_offset_list[i]) != 0; i++)
                flip_long(valI+idx);
          }

        else if (esize > 4)

          { if (tag == off_tag)
              for (j = 0; j < tag->count; j++)
                { if (otype)
                    OUTPUT_SHORT(doff)
                  else
                    OUTPUT_LONG(doff)
                  if (ctype)
                    doff += cntS[j];
                  else
                    doff += cntI[j];
                }

            else if (!flip || tsize == 1)
              fwrite(ifd->values + tag->value,esize,1,output); 

            else
              switch (tsize)
              { case 2:
                  valS = (uint16 *) (ifd->values + tag->value);
                  for (j = 0; j < tag->count; j++)
                    OUTPUT_SHORT(valS[j])
                  break;
                case 4:
                  valI = (uint32 *) (ifd->values + tag->value);
                  for (j = 0; j < tag->count; j++)
                    OUTPUT_LONG(valI[j])
                  break;
                case 8:
                  if (tag->type == TIFF_DOUBLE)
                    { valD = (double *) (ifd->values + tag->value);
                      for (j = 0; j < tag->count; j++)
                        { double d = valD[j];
                          flip_quad(&d);
                          fwrite(&d,8,1,output);
                        }
                    }
                  else  // tag->type in { TIFF_RATIONAL, TIFF_SRATIONAL }
                    { valI = (uint32 *) (ifd->values + tag->value);
                      for (j = 0; j < 2*tag->count; j++)
                        OUTPUT_LONG(valI[j])
                    }
                  break;
              }
          }
      }
  }

  { uint32 i, csize, off;
 
    for (i = 0; i < off_tag->count; i++)
      { if (ctype)
          csize = cntS[i];
        else
          csize = cntI[i];
        if (otype)
          off = offS[i];
        else
          off = offI[i];
        fwrite(ifd->data + off, csize, 1, output);
      }

    offset += datasize;
  }

  tif->ifd_linko  = tif->eof_offset + ifd->numtags*12 + 2;
  tif->eof_offset = offset;
  tif->ifd_no    += 1;

  return (0);
}

void Close_Tiff_Writer(Tiff_Writer *etif)
{ Twriter *tif = (Twriter *) etif;
  FILE    *output;
  int      flip;
  int      zero = 0;

  if (tif->ifd_no == 1)
    write_tiff_header(tif,0);

  flip   = tif->flip;
  output = tif->output;

  if (tif->ano_count > 4)
    { fwrite(tif->annotation,tif->ano_count,1,tif->output);
      optseek(tif->output,tif->ano_linko,SEEK_SET);
      OUTPUT_LONG(tif->eof_offset)
    }

  optseek(tif->output,tif->ifd_linko,SEEK_SET);
  zero = 0;
  OUTPUT_LONG(zero)

  fclose(tif->output);
}

Tiff_IFD *Convert_LSM_2_RGB(Tiff_IFD *eifd, int source, int target)
{ TIFD *ifd = (TIFD *) eifd;

  static int     StripMax = 0;
  static uint32 *StripVec = NULL;
  static uint16 *StripSht = NULL;

  uint32   *valI, spp;
  uint16   *valS, bps[3], get[3];
  Tiff_Type type;
  int       count, hcount;

  valI = (uint32 *) Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
  if (valI == NULL)
    { report_error("LSM IFD does not contain a New_Sub_File_Type tag");
      return (NULL);
    }
  if ((*valI & TIFF_VALUE_REDUCED_RESOLUTION) != 0)
    return (NULL);

  valI = (uint32 *) Get_Tiff_Tag(ifd,TIFF_SAMPLES_PER_PIXEL,&type,&count);
  if (valI == NULL)
    { report_error("LSM IFD does not contain a Samples_Per_Pixel tag");
      return (NULL);
    }
  spp = *valI;
  if (spp == 1 || spp == 3)
    return (eifd);
  if (spp > 3)
    { report_error("LSM IFD contains more than 3 channels");
      return (NULL);
    }
  *valI = 3;

  if (source >= 2 || target >= 3)
    { report_error("Arguments to Convert_LSM_2_RGB out of range");
      return (NULL);
    }
  if (source < 0 || target < 0)
    { report_error("Arguments to Convert_LSM_2_RGB out of range");
      return (NULL);
    }
  get[0] = get[1] = get[2] = 1-source;
  get[target] = source;

  valS = (uint16 *) Get_Tiff_Tag(ifd,TIFF_BITS_PER_SAMPLE,&type,&count);
  if (valS == NULL)
    { report_error("LSM IFD does not contain a Bits_Per_Sample tag");
      return (NULL);
    }
  bps[0] = valS[get[0]];
  bps[1] = valS[get[1]];
  bps[2] = valS[get[2]];
  Set_Tiff_Tag(ifd,TIFF_BITS_PER_SAMPLE,type,3,bps);

  bps[0] = TIFF_VALUE_RGB; 
  Set_Tiff_Tag(ifd,TIFF_PHOTOMETRIC_INTERPRETATION,TIFF_SHORT,1,bps);

  valI = (uint32 *) Get_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,&type,&count);
  if (valI == NULL)
    { report_error("LSM IFD does not contain a Strip_Btye_Counts tag");
      return (NULL);
    }
  hcount = count/2;
  if (StripMax < 3*hcount)
    { StripMax = 3.3*hcount+128;
      StripVec = (uint32 *) Guarded_Realloc(StripVec,sizeof(int)*StripMax,"Convert_LSM_2_RGB");
      StripSht = (uint16 *) StripVec;
    }
  if (type == TIFF_SHORT)
    { valS = (uint16 *) valI;
      memcpy(StripSht,valS+get[0]*hcount,sizeof(uint16)*hcount);
      memcpy(StripSht+hcount,valS+get[1]*hcount,sizeof(uint16)*hcount);
      memcpy(StripSht+2*hcount,valS+get[2]*hcount,sizeof(uint16)*hcount);
    }
  else
    { memcpy(StripVec,valI+get[0]*hcount,sizeof(uint32)*hcount);
      memcpy(StripVec+hcount,valI+get[1]*hcount,sizeof(uint32)*hcount);
      memcpy(StripVec+2*hcount,valI+get[2]*hcount,sizeof(uint32)*hcount);
    }
  Set_Tiff_Tag(ifd,TIFF_STRIP_BYTE_COUNTS,type,3*hcount,StripVec);
  
  valI = (uint32 *) Get_Tiff_Tag(ifd,TIFF_STRIP_OFFSETS,&type,&count);
  if (valI == NULL)
    { report_error("LSM IFD does not contain a Strip_Offsets tag");
      return (NULL);
    }
  if (type == TIFF_SHORT)
    { valS = (uint16 *) valI;
      memcpy(StripSht,valS+get[0]*hcount,sizeof(uint16)*hcount);
      memcpy(StripSht+hcount,valS+get[1]*hcount,sizeof(uint16)*hcount);
      memcpy(StripSht+2*hcount,valS+get[2]*hcount,sizeof(uint16)*hcount);
    }
  else
    { memcpy(StripVec,valI+get[0]*hcount,sizeof(uint32)*hcount);
      memcpy(StripVec+hcount,valI+get[1]*hcount,sizeof(uint32)*hcount);
      memcpy(StripVec+2*hcount,valI+get[2]*hcount,sizeof(uint32)*hcount);
    }
  Set_Tiff_Tag(ifd,TIFF_STRIP_OFFSETS,type,3*hcount,StripVec);

  return (eifd);
}

int *Get_LSM_Colors(Tiff_IFD *eifd, int *nchannels)
{ TIFD *ifd = (TIFD *) eifd;

  static int  LSM_Color_Max = 0;
  static int *LSM_Color_Array = NULL;

  uint32   *lsmarr, *colarr, coloff, locoff;
  int       count, lflip, i;
  Tiff_Type type;

  lsmarr = (uint32 *) Get_Tiff_Tag(ifd,TIFF_CZ_LSMINFO,&type,&count);
  if (lsmarr == NULL)
    { report_error("IFD does not contain LSM tag");
      return (NULL);
    }
  
  lflip  = (lsmarr[0] != 0x0300494C && lsmarr[0] != 0x0400494C);

  coloff = lsmarr[LSM_CHANNEL_COLORS];
  if (lflip)
    flip_long(&coloff);
  colarr = (uint32 *) (lsmarr + coloff);
  *nchannels = colarr[1];
  if (lflip)
    flip_long((uint32 *) nchannels);
  locoff = colarr[3];
  if (lflip)
    flip_long(&locoff);
  colarr = (uint32 *) (colarr + locoff);

  if (*nchannels > LSM_Color_Max)
    { LSM_Color_Max = *nchannels;
      LSM_Color_Array = (int*)Guarded_Realloc(LSM_Color_Array,sizeof(int)**nchannels,"Get_LSM_Colors");
    }

  for (i = 0; i < *nchannels; i++)
    { LSM_Color_Array[i] = colarr[i];
      if (lflip)
        flip_long((uint32 *) (LSM_Color_Array+i));
    }
  return (LSM_Color_Array);
}

/****************************************************************************************
 *                                                                                      *
 *  TIFF_ANNOTATOR ROUTINES                                                             *
 *                                                                                      *
 ****************************************************************************************/

static Tiff_Annotator *open_annotator(char *name, Annotator_Status *good)
{ static int firstime = 1;
  static int mach_endian;

  uint16 initblock[5];

  static uint8 *ifdblock = NULL;
  static int    ifdmax = 0;

  Tannotator *tif = NULL;
  uint32      flip, offset;
  uint16      order, ntags;
  FILE       *infile;
  int         input;
  int         nocheck;

  static struct stat fdesc;

  if (firstime)
    { firstime = 0;
      mach_endian = Native_Endian();
    }

  if (good != NULL)
    { nocheck = 0;
      *good = ANNOTATOR_CANT_OPEN;
    }
  else
    nocheck = 1;

  infile = fopen(name,"rb+");
  if (infile == NULL)
    { report_error("Cannot open file for reading");
      return (NULL);
    }
  input = fileno(infile);

  if (!nocheck)
    *good = ANNOTATOR_GIBBERISH;

  if (read(input,initblock,10) != 10)
    { report_error("File ends prematurely");
      goto closeout;
    }

  order = initblock[0];
  if (order == 0x4949)
    flip = mach_endian;
  else if (order == 0x4D4D)
    flip = 1-mach_endian;
  else
    { report_error("Does not contain valid endian value");
      goto closeout;
    }

  order = initblock[1];
  if (flip)
    flip_short(&order);
  if (order != 0x002A)
    { report_error("Does not contain magic key");
      goto closeout;
    }

  offset = ((int *) initblock)[1];
  if (flip)
    flip_long(&offset);

  if (nocheck)
    { tif = new_tannotator(0,"Open_Tiff_Anotator");

      tif->flip  = flip;
      tif->inout = infile;
    }

  if (offset == 0)
    { report_error("Trying to advance at end-of-ifd-list");
      goto invalid;
    }

  // Seek to the next IFD, see how many tags it has,
  //   and allocate the ifd record and its tag array

  if (offset != 8)
    { if (lseek(input,offset,SEEK_SET) < 0)
        { report_error("Seek for first IFD failed");
          goto invalid;
        }
      if (read(input,&ntags,2) != 2)
        { report_error("File ends prematurely");
          goto invalid;
        }
    }
  else
    ntags = initblock[4];
  if (flip)
    flip_short(&ntags);

  // Read the tags, looking for ours and when find it setup additional fields of Tannotator

  { int i;

    ntags *= 12;
    if (ntags > ifdmax)
      { ifdmax = ntags + 120;
        ifdblock = (uint8 *) Guarded_Realloc(ifdblock,ifdmax,"Open_Tiff_Annotator");
      }

    read(input,ifdblock,ntags);

    for (i = 0; i < ntags; i += 12)

      { uint16 label;
        uint32 count, value, *valptr;

        label = *((uint16 *) (ifdblock+i));
        if (flip)
          flip_short(&label);

        if (label == TIFF_JF_TAGGER)
          { if (!nocheck)
              *good = ANNOTATOR_NOT_FORMATTED;

            count = *((uint32 *) (ifdblock+(i+4)));
            if (flip)
              flip_long(&count);

            fstat(input,&fdesc);
            valptr = (uint32 *) (ifdblock+(i+8));
            if (count > 4)
              { if (flip)
                  flip_long(valptr);
                value = *valptr;
                if (value + count != fdesc.st_size)
                  { report_error("File is not formated for annotation");
                    goto invalid;
                  }
              }

            if (nocheck)
              { allocate_tannotator_annotation(tif,count,"Open_Tiff_Annotator");
                tif->ano_count  = count;
                tif->ano_cnto   = offset + i + 6;
                if (count <= 4)
                  { tif->ano_offset = fdesc.st_size;
                    memcpy(tif->annotation,(char *) valptr,count);
                  }
                else
                  { tif->ano_offset = value;
                    lseek(input,value,SEEK_SET);
                    read(input,tif->annotation,count);
                  }
              }

            break;
          }
      }
    if (!nocheck)
      *good = ANNOTATOR_NOT_FORMATTED;
    if (i >= ntags)
      { report_error("File is not formated for annotation");
        goto invalid;
      }
  }

  if (nocheck)
    return ((Tiff_Annotator *) tif);
  else
    { *good = ANNOTATOR_FORMATTED;
      fclose(infile);
      return (NULL);
    }

invalid:
  if (nocheck)
    Free_Tiff_Annotator(tif);
closeout:
  fclose(infile);
  return (NULL);
}

Tiff_Annotator *Open_Tiff_Annotator(char *name)
{ return (open_annotator(name,NULL)); }

Annotator_Status Tiff_Annotation_Status(char *name)
{ Annotator_Status good;
  open_annotator(name,&good);
  return (good);
}

char *Get_Tiff_Annotation(Tiff_Annotator *etif, int *count)
{ Tannotator *tif = (Tannotator *) etif;

  *count = tif->ano_count-1;
  return (tif->annotation);
}

void Set_Tiff_Annotation(Tiff_Annotator *etif, char *anno, int count)
{ Tannotator *tif = (Tannotator *) etif;
 
  allocate_tannotator_annotation(tif,count+1,"Set_Tiff_Annotation");
  tif->ano_count  = count+1;
  tif->annotation[count] = '\0';
  memcpy(tif->annotation,anno,count);
}

void Close_Tiff_Annotator(Tiff_Annotator *etif)
{ Tannotator *tif = (Tannotator *) etif;
  FILE       *output;
  int         flip;

  flip   = tif->flip;
  output = tif->inout;

  optseek(tif->inout,tif->ano_cnto,SEEK_SET);
  OUTPUT_LONG(tif->ano_count);

  if (tif->ano_count <= 4)
    { fwrite(tif->annotation,tif->ano_count,1,tif->inout);
      ftruncate(fileno(tif->inout),tif->ano_offset);
    }
  else
    { OUTPUT_LONG(tif->ano_offset);
      optseek(tif->inout,tif->ano_offset,SEEK_SET);
      fwrite(tif->annotation,tif->ano_count,1,tif->inout);
      ftruncate(fileno(tif->inout),tif->ano_offset+tif->ano_count);
    }
}

int Format_Tiff_For_Annotation(char *name)
{ Tiff_Reader *rtif;
  Tiff_Writer *wtif;
  Tiff_IFD    *ifd;
  Tiff_Type    type;
  int          endian, count, status, i, lsm;
  static char *templat = "$jf_tagger.XXXXXX";
  static char *tempname;

  status = 1;

  lsm = (strcmp(name+(strlen(name)-4),".lsm") == 0);

  rtif = Open_Tiff_Reader(name,&endian,lsm);
  if (rtif == NULL)
    return (1);

  if (End_Of_Tiff(rtif))
    { report_error("Empty tiff file");
      goto invalid;
    }

  tempname = (char*)Guarded_Malloc(strlen(templat)+strlen(name)+2,"Format_Tiff_For_Annotation");

  strcpy(tempname,name);
  for (i = strlen(name)-1; i >= 0; i--)
    if (tempname[i] == '/')
      break;
  strcpy(tempname+(i+1),templat);
 
  #if !defined(_WIN64) && !defined(_WIN32) 
  
               if (mkstemp(tempname) < 0)
    { report_error("Could not create temporary file");
      goto invalid1;
    }

               #else
	FILE *stream;
  
               if( ( tempname = _tempnam( name, "$jf_tagger." ) ) != NULL )
    
	stream = fopen( tempname, "wt" );
  
	else
    {
      report_error( "Cannot create a unique filename\n" );
      goto invalid1;
    }

  
	if ( stream == NULL ) 
    
	{ report_error("Could not create temporary file");
      goto invalid1;
    }
  
	else
    
	fclose(stream);
               

#endif  

  wtif = Open_Tiff_Writer(tempname,lsm);
  if (wtif == NULL)
    goto invalid2;

  ifd = Read_Tiff_IFD(rtif);
  if (ifd == NULL)
    goto invalid3;

  if (Get_Tiff_Tag(ifd,TIFF_JF_TAGGER,&type,&count) == NULL)
    Set_Tiff_Tag(ifd,TIFF_JF_TAGGER,TIFF_ASCII,1,"\0");

  Write_Tiff_IFD(wtif,ifd);
  Free_Tiff_IFD(ifd);

  while ( ! End_Of_Tiff(rtif))
    { ifd = Read_Tiff_IFD(rtif);
      if (ifd == NULL)
        goto invalid3;
      Write_Tiff_IFD(wtif,ifd);
      Free_Tiff_IFD(ifd);
    }
  Close_Tiff_Writer(wtif);
  Free_Tiff_Reader(rtif);

  remove(name);
  rename(tempname,name);

  status = 0;

invalid3:
  Free_Tiff_Writer(wtif);
invalid2:
  if (status)
    remove(tempname);
invalid1:
  free(tempname);
invalid:
  if (status)
    Free_Tiff_Reader(rtif);
  return (status);
}


/****************************************************************************************
 *                                                                                      *
 *  IFD PRINTING ROUTINE                                                                *
 *                                                                                      *
 ****************************************************************************************/

static char *tiff_label[] =
      { "NEW_SUB_FILE_TYPE",
        "SUB_FILE_TYPE",
        "IMAGE_WIDTH",
        "IMAGE_LENGTH",
        "BITS_PER_SAMPLE",
        "COMPRESSION",
        "???",
        "???",
        "PHOTOMETRIC_INTERPRETATION",
        "THRESHHOLDING",
        "CELL_WIDTH",
        "CELL_LENGTH",
        "FILL_ORDER",
        "???",
        "???",
        "DOCUMENT_NAME",
        "IMAGE_DESCRIPTION",
        "MAKE",
        "MODEL",
        "STRIP_OFFSETS",
        "ORIENTATION",
        "???",
        "???",
        "SAMPLES_PER_PIXEL",
        "ROWS_PER_STRIP",
        "STRIP_BYTE_COUNTS",
        "MIN_SAMPLE_VALUE",
        "MAX_SAMPLE_VALUE",
        "X_RESOLUTION",
        "Y_RESOLUTION",
        "PLANAR_CONFIGURATION",
        "PAGE_NAME",
        "X_POSITION",
        "Y_POSITION",
        "FREE_OFFSETS",
        "FREE_BYTE_COUNTS",
        "GRAY_RESPONSE_UNIT",
        "GRAY_RESPONSE_CURVE",
        "T4_OPTIONS",
        "T6_OPTIONS",
        "???",
        "???",
        "RESOLUTION_UNIT",
        "PAGE_NUMBER",
        "???",
        "???",
        "???",
        "TRANSFER_FUNCTION",
        "???",
        "???",
        "???",
        "SOFTWARE",
        "DATE_TIME",
        "???",
        "???",
        "???",
        "???",
        "???",
        "???",
        "???",
        "???",
        "ARTIST",
        "HOST_COMPUTER",
        "PREDICTOR",
        "WHITE_POINT",
        "PRIMARY_CHROMATICITIES",
        "COLOR_MAP",
        "HALFTONE_HINTS",
        "TILE_WIDTH",
        "TILE_LENGTH",
        "TILE_OFFSETS",
        "TILE_BYTE_COUNTS",
        "???",
        "???",
        "???",
        "???",
        "???",
        "???",
        "INK_SET",
        "INK_NAMES",
        "NUMBER_OF_INKS",
        "???",
        "DOT_RANGE",
        "TARGET_PRINTER",
        "EXTRA_SAMPLES",
        "SAMPLE_FORMAT",
        "SMIN_SAMPLE_VALUE",
        "SMAX_SAMPLE_VALUE",
        "TRANSFER_RANGE"
      };

static char *tiff_type[] =
      { "",
        "BYTE",
        "ASCII",
        "SHORT",
        "LONG",
        "RATIONAL",
        "SBTYE",
        "UNDEFINED",
        "SSHORT",
        "SLONG",
        "SRATIONAL",
        "FLOAT",
        "DOUBLE"
      };

void Print_Tiff_IFD(Tiff_IFD *eifd, FILE *output)
{ TIFD *ifd = (TIFD *) eifd;
  int   i, j;

  fprintf(output,"\nIFD (%d tags):\n",ifd->numtags);
  for (i = 0; i < ifd->numtags; i++)
    { Tif_Tag *tag = ifd->tags+i;
      int label = tag->label;
      int count = tag->count;
      int type  = tag->type;

      uint8  *valB;
      char   *valA;
      uint16 *valS;
      uint32 *valI;

      if (label < TIFF_NEW_SUB_FILE_TYPE)
        fprintf(output,"  ???");
      else if (label <= TIFF_TRANSFER_RANGE)
        fprintf(output,"  %s",tiff_label[label-TIFF_NEW_SUB_FILE_TYPE]);
      else
        fprintf(output,"  ++ %d ++",label);

      printf(" %d %s: ",count,tiff_type[type]);

      switch (type)
      { case TIFF_BYTE:
          if (type_sizes[type]*count <= 4)
            valB = (uint8 *) POINTER(tag);
          else
            valB = (uint8 *) (ifd->values + tag->value);
          for (j = 0; j < count; j++)
            { fprintf(output," %u",valB[j]);
              if (j > 5)
                { fprintf(output," ...");
                  break;
                }
            }
          break;
        case TIFF_ASCII:
          if (type_sizes[type]*count <= 4)
            valA = (char *) POINTER(tag);
          else
            valA = (char *) (ifd->values + tag->value);
          fprintf(output,"\'");
          for (j = 0; j < count; j++)
            { if (valA[j] < ' ')
                fprintf(output,"\\%xx",valA[j]);
              else
                fprintf(output,"%c",valA[j]);
            }
          fprintf(output,"\'");
          break;
        case TIFF_SHORT:
          if (type_sizes[type]*count <= 4)
            valS = SHORT_PTR(tag);
          else
            valS = (uint16 *) (ifd->values + tag->value);
          for (j = 0; j < count; j++)
            { fprintf(output," %u",valS[j]);
              if (j > 5)
                { fprintf(output," ...");
                  break;
                }
            }
          break;
        case TIFF_LONG:
          if (type_sizes[type]*count <= 4)
            valI = LONG_PTR(tag);
          else
            valI = (uint32 *) (ifd->values + tag->value);
          for (j = 0; j < count; j++)
            { fprintf(output," %u",valI[j]);
              if (j > 5)
                { fprintf(output," ...");
                  break;
                }
            }
          break;
        case TIFF_RATIONAL:
          valI = (uint32 *) (ifd->values + tag->value);
          for (j = 0; j < count; j++)
            { fprintf(output," %d/%d",valI[2*j],valI[2*j+1]);
              if (j > 5)
                { fprintf(output," ...");
                  break;
                }
            }
          break;
        default:
          fprintf(output,"...");
          break;
      }
      fprintf(output,"\n");
    }
}
