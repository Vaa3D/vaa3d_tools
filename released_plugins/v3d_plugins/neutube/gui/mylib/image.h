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
*           Dec 2009: Introduced arrays to generalize images.  Almost a complete rewrite. *
*              Also introduced layers to capture an unlimited number of channels.         *
*                                                                                         *
*  (c) June 19, '09, Dr. Gene Myers and Howard Hughes Medical Institute                   *
*      Copyrighted as per the full copy in the associated 'README' file                   *
*                                                                                         *
\*****************************************************************************************/

#ifndef _IMAGE_LIB

#define _IMAGE_LIB

#ifdef __cplusplus
extern "C" {
#endif

#include "mylib.h"
#include "array.h"

string Image_Error();
void   Image_Error_Release();

typedef struct
  { int     num_layers;    //  Number of layers in layer list
    Array **layers;        //  layers[i] is the image of the i'th layer for i in [0,num_layers-1]
  } Layer_Bundle;

typedef enum
  { DONT_PRESS     = 0,
    LZW_PRESS      = 1,
    PACKBITS_PRESS = 2
  } Image_Compress;

Array  *G(Read_Image)(string file_name, int layer);
boolean Write_Image(string file_name, Array *image, Image_Compress compress);

Layer_Bundle *Read_Images(string file_name, Layer_Bundle *R(M(bundle)));
boolean       Write_Images(string file_name, Layer_Bundle *images, Image_Compress compress);

typedef struct
  { string  prefix;
    string  suffix;      //  bundle names are "%s%0*d%s",prefix,num_width,#,suffix or
    boolean padded;      //  "%s%d%s",prefix,#,suffix if padded is false, tries both
    int     num_width;   //  when reading.  suffix points into the malloc block assigned
    int     first_num;   //  to prefix.
  } Series_Bundle;

Series_Bundle *Parse_Series_Name(Series_Bundle *R(M(files)), string file_name);

Array       *G(Read_Series)(Series_Bundle *M(files), int layer);
boolean      Write_Series(Series_Bundle *files, Array *image, Image_Compress compress);

Layer_Bundle *Read_All_Series(Series_Bundle *M(files), Layer_Bundle *R(M(layers)));
boolean       Write_All_Series(Series_Bundle *files, Layer_Bundle *images,
                               Image_Compress compress);

typedef void Tiff;
typedef Tiff RTiff;
typedef Tiff WTiff;

typedef enum { PLAIN_CHAN, MAPPED_CHAN, RED_CHAN,
               GREEN_CHAN, BLUE_CHAN,   ALPHA_CHAN
             } Channel_Kind;

Tiff   *G(Open_Tiff)(const char * file_name, const char * mode);
void    Close_Tiff(Tiff *F(tif));

void    Update_Tiff(WTiff *M(tif), Image_Compress compress);
void    Advance_Tiff(RTiff *M(tif));
void    Rewind_Tiff(RTiff *M(tif));
boolean Tiff_EOF(RTiff *tif);

boolean      Get_IFD_Shape(RTiff *tif, int *width, int *height, int *channels);
Value_Type   Get_IFD_Channel_Type(RTiff *tif, int channel);
Channel_Kind Get_IFD_Channel_Kind(RTiff *tif, int channel);
boolean      Get_IFD_Channel(RTiff *tif, int channel, Array M(*place));
string       Get_IFD_Annotation(RTiff *tif);
boolean      Get_IFD_Map(RTiff *tif, Array M(*map));

boolean Add_IFD_Channel(WTiff *M(tif), Array *channel, Channel_Kind kind);
boolean Set_IFD_Map(WTiff *M(tif), Array *map);

Tiff  *G(Copy_Tiff)(Tiff *tif);
Tiff  *Pack_Tiff(Tiff *R(M(tif)));
Tiff  *Inc_Tiff(Tiff *R(I(tif)));
void   Free_Tiff(Tiff *F(tif));
void   Kill_Tiff(Tiff *K(tif));
void   Reset_Tiff();
int    Tiff_Usage();
void   Tiff_List(void (*handler)(Tiff *));
int    Tiff_Refcount(Tiff *tif);

#ifdef __cplusplus
}
#endif

#endif
