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

#ifndef _TIFF_IMAGE

#define _TIFF_IMAGE

#include <stdlib.h>
#include <stdio.h>

#include "utilities.h"
#include "tiff_io.h"

//  A Tiff_Image consists of its dimensions and an array of pointer to Tiff_Channel objects
//   which it 'owns' (i.e., they are freed, killed, etc. when the Tiff_Image is freed, killed, ...)
//   There are samples_per_pixel channels, each of which has a row-major ordered array of the
//   pixel values for a given sample and some meta-data such as the # of bits in the values,
//   the # of bytes used to hold the values, how the value should be interpreted, and so on.
//   See the comments next to each field in the definition of Tiff_Channel below.  Regardless
//   of the orientation of the tiff, the planes are arranged so that (0,0) is always in the
//   upper-left corner.

typedef enum
  { CHAN_WHITE,   //  0 is imaged as white and the maximum value as black
    CHAN_BLACK,   //  0 is imaged as black and the maximum value as white
    CHAN_MAPPED,  //  Values are mapped to 48-bit RGB triples via the images color map
    CHAN_RED,     //  Values are for the red-component of an RGB rendering
    CHAN_GREEN,   //  Values are for the green-component of an RGB rendering
    CHAN_BLUE,    //  Values are for the blue-component of an RGB rendering
    CHAN_ALPHA,   //  Premultiplied opacity for each pixel
    CHAN_MATTE,   //  Unassociated (not premultiplied) opacity for each pixel
    CHAN_MASK,    //  0-1 values provide a transparency mask to be applied to other channels
    CHAN_OTHER    //  No interpretation is provided by the tiff IFD
  } Channel_Meaning;

typedef enum
  { CHAN_UNSIGNED,   //  The bits of a value are to be interpreted as an unsigned integer
    CHAN_SIGNED,     //  The bits ... as a 2's complement signed integer
    CHAN_FLOAT       //  The bits ... as an IEEE floating point value
  } Channel_Type;

typedef struct
  { int          bitshift;     // The largest non-zero bit position
    int          total;        // Sum of all counts in the histogram
    unsigned int counts[512];  // Bitshift is at least 1/2*max value, giving 8-bit
  } Tiff_Histogram;            //   precision when max >= 256

typedef struct
  { int             width;            //  The width of every channel plane
    int             height;           //  The height of every chanel plane
    Channel_Meaning interpretation;   //  Any interpretation hint (if any) provided by the tiff
    int             scale;            //  The # of bits per value in this channel
    int             bytes_per_pixel;  //  The # of bytes each value is stored in (1, 2, or 4)
    Channel_Type    type;             //  The nature of the values
    void           *plane;            //  A width x height array of the values in row major order
    Tiff_Histogram *histogram;        //  Histogram of channel values (NULL if not computed)
  } Tiff_Channel;

typedef struct
  { int             width;            //  The width of every channel plane
    int             height;           //  The height of every chanel plane
    int             number_channels;  //  The number of channels (samples_per_pixel)
    Tiff_Channel  **channels;         //  [0..number_channels-1] gives the Channel object for each
    unsigned short *map;              //  The color map for the 1st channel if it is CHAN_MAPPED
  } Tiff_Image;

/* For those routines below that have error exits, you can get a text string describing the
   particular error that occured by calling Tiff_Image_Error.  The string is managed by the
   package and should not be freed by the user.
*/

char *Tiff_Image_Error();

/* Given a Tiff_IFD ifd, a Tiff_Image is built for the image encoded therein.  This routine is
   a generator, i.e. the returned object can be copied, packed, freed, etc. with the standard
   object operators below.  A NULL pointer is returned if the contents of the ifd where not
   semantically valid.
*/

Tiff_Image *Extract_Image_From_IFD(Tiff_IFD *ifd);

/* A new Tiff_Image object, initiall with no channels, is generated by calling Create_Tiff_Image.
   Subsequent requests to add channels with Add_Tiff_Image_Channel, add channels as per the
   parameters and with the width and height of the image.  Add_Tiff_Image_Channel returns a
   non-zero value if and only if an error occured, and Create_Tiff_Image returns NULL if an
   error occured.
*/

Tiff_Image *Create_Tiff_Image(int width, int height);
int         Add_Tiff_Image_Channel(Tiff_Image *image, Channel_Meaning meaning, int scale,
                                                      Channel_Type type);

// Makes a Tiff_IFD object that encodes the Tiff_Image passed to it, compressing it if
// the parameter compression is non-zero.

Tiff_IFD *Make_IFD_For_Image(Tiff_Image *image, int compression);

// Scales the values in the channel to have the given number of bits, or call the image
//   version to scale all channels in the image to a given scale

void Scale_Tiff_Channel(Tiff_Channel *channel, int scale);
void Range_Tiff_Channel(Tiff_Channel *channel, double *minval, double *maxval);
void Shift_Tiff_Channel(Tiff_Channel *channel, int shift);

Tiff_Histogram *Histogram_Tiff_Channel(Tiff_Channel *channel);
void            Histogram_Tiff_Image_Channels(Tiff_Image *image);
void            Tiff_Histogram_Merge(Tiff_Histogram *h1, Tiff_Histogram *h2);

void Scale_Tiff_Image(Tiff_Image *image, int scale);
void Shift_Tiff_Image(Tiff_Image *image, int shift);

// The standard object operators

Tiff_Channel *Copy_Tiff_Channel(Tiff_Channel *channel);
void          Pack_Tiff_Channel(Tiff_Channel *channel);
void          Free_Tiff_Channel(Tiff_Channel *channel);
void          Kill_Tiff_Channel(Tiff_Channel *channel);
void          Reset_Tiff_Channel();
int           Tiff_Channel_Usage();

Tiff_Image *Copy_Tiff_Image(Tiff_Image *image);
void        Pack_Tiff_Image(Tiff_Image *image);
void        Free_Tiff_Image(Tiff_Image *image);
void        Kill_Tiff_Image(Tiff_Image *image);
void        Reset_Tiff_Image();
int         Tiff_Image_Usage();

Tiff_Histogram *Copy_Tiff_Histogram(Tiff_Histogram *histogram);
void            Pack_Tiff_Histogram(Tiff_Histogram *histogram);
void            Free_Tiff_Histogram(Tiff_Histogram *histogram);
void            Kill_Tiff_Histogram(Tiff_Histogram *histogram);
void            Reset_Tiff_Histogram();
int             Tiff_Histogram_Usage();

#endif //_TIFF_IMAGE
