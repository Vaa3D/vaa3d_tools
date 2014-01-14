#include "tz_tiff_image.h"
#define _TIFF_IMAGE
#include <tiff_image.c>
#include "tz_image_io.h"
#include "tz_error.h"

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

#if defined(MAX_CHANNEL_NUMBER)
#  undef MAX_CHANNEL_NUMBER
#endif
#define MAX_CHANNEL_NUMBER 4
Tiff_IFD *Make_IFD_For_Lsm_Image(Tiff_Image *image, int compression, 
				 Tiff_IFD *templat, int depth)
{
  TZ_ASSERT((image->number_channels >= 1) && (image->number_channels <=
	MAX_CHANNEL_NUMBER),
	    "Invalid channel number");

  unsigned char *encode;

  encode = get_code_vector(1.1*image->width*image->height*sizeof(int) + 4096,
			   "Make_IFD_For_Lsm_Image");

  Tiff_IFD *ifd = NULL;

  if (compression == 1) {
    ifd = Create_Tiff_IFD(12);
  } else {
    ifd = Create_Tiff_IFD(11);
  }

  uint32_t newsubfile_type = 0; /* not a thumbnail*/
  Set_Tiff_Tag(ifd, TIFF_NEW_SUB_FILE_TYPE, TIFF_LONG, 1, &(newsubfile_type));

  uint32_t image_width = image->width;
  Set_Tiff_Tag(ifd, TIFF_IMAGE_WIDTH, TIFF_LONG, 1, &(image_width));

  uint32_t image_length = image->height;
  Set_Tiff_Tag(ifd, TIFF_IMAGE_LENGTH, TIFF_LONG, 1, &(image_length));

  uint16_t bits_per_sample[MAX_CHANNEL_NUMBER];
  int bits_per_sample_count = image->number_channels;
  int i;
  //for (i = 0; i < 3; i++) {
  for (i = 0; i < image->number_channels; i++) {
    bits_per_sample[i] = image->channels[0]->bytes_per_pixel * 8;
  }
  Set_Tiff_Tag(ifd, TIFF_BITS_PER_SAMPLE, TIFF_SHORT, bits_per_sample_count,
	       bits_per_sample);

  uint16_t tif_compression;
  if (compression == 0) {
    tif_compression = TIFF_VALUE_UNCOMPRESSED; /* no compression */
  } else {
    tif_compression = TIFF_VALUE_LZW; /* LZW */
  }
  Set_Tiff_Tag(ifd, TIFF_COMPRESSION, TIFF_SHORT, 1, &tif_compression);
  
  uint16_t photometric_interpretion;
  if (image->number_channels > 1) {
    photometric_interpretion = TIFF_VALUE_RGB; /* TIF_RGB24 */
  } else {
    if (image->channels[0]->interpretation == CHAN_MAPPED) {
      photometric_interpretion = TIFF_VALUE_RGB_PALETTE; /* one channel with color map */
    } else {
      photometric_interpretion = TIFF_VALUE_BLACK_IS_ZERO; /* one channel without color map*/
    }
  }
  Set_Tiff_Tag(ifd, TIFF_PHOTOMETRIC_INTERPRETATION, TIFF_SHORT, 1, 
	       &photometric_interpretion);

  uint32_t strip_offsets[MAX_CHANNEL_NUMBER]; /* determine later */
  uint32_t strip_byte_counts[MAX_CHANNEL_NUMBER]; /* determine later */

  int data_size = 0;
  int remain;
  for (i = 0; i < image->number_channels; i++) {
    data_size += tiff_channel_psize(image->channels[i]);
  }
  
  Allocate_Tiff_IFD_Data(ifd,data_size);

  unsigned char *data, *stream;

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
          if (byte_count < 0) {
	    // Compressed result longer than uncompressed result!  
	    // Turn itoff and redo it.
	    tif_compression = TIFF_VALUE_UNCOMPRESSED;   
	    Set_Tiff_Tag(ifd, TIFF_COMPRESSION, TIFF_SHORT, 1, &tif_compression);
	    compression = 0;             
	    //printf("Compression failed\n");
	    goto redo;                   
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

      //strip_byte_counts[i] = byte_count;
      strip_offsets[i]     = stream-data;
      stream += byte_count;
      remain -= byte_count;
    }
 

  Set_Tiff_Tag(ifd, TIFF_STRIP_OFFSETS, TIFF_LONG, image->number_channels,
	       strip_offsets);  

  uint32_t samples_per_pixel = image->number_channels;
  Set_Tiff_Tag(ifd, TIFF_SAMPLES_PER_PIXEL, TIFF_LONG, 1, &samples_per_pixel);

  /* LSM stores actual image size even the image is compressed */
  strip_byte_counts[0] = 
    image->width * image->height * image->channels[0]->bytes_per_pixel;
  for (i = 0; i < image->number_channels; i++) {
    strip_byte_counts[i] = strip_byte_counts[0];
  }

  Set_Tiff_Tag(ifd, TIFF_STRIP_BYTE_COUNTS, TIFF_LONG, samples_per_pixel, 
	       strip_byte_counts);
  
  uint16_t planar_configuration = TIFF_VALUE_PLANAR;
  Set_Tiff_Tag(ifd, TIFF_PLANAR_CONFIGURATION, TIFF_SHORT, 1, 
	       &planar_configuration);

  uint16_t predictor = 2;
  if (compression != 0) {
    Set_Tiff_Tag(ifd, TIFF_PREDICTOR, TIFF_SHORT, 1, &predictor);
  }

  /* The color map should not be used due to some changes in the different
   * versions */
  /*
  if (template != NULL) {
    void *color_map;
    int type, count;
    if ((tmpl_data = Get_Tiff_Tag(template, TIFF_COLORMAP, &type, &count))
	!= NULL) {
      Set_Tiff_Tag(ifd, TIFF_COLORMAP, type, count, color_map);
    }
  }
  */

  Tiff_Type type;
  int count;
  if (templat != NULL) {
    uint8_t *lsmfield= (uint8_t*)Get_Tiff_Tag(templat, TIFF_CZ_LSMINFO, &type, &count);
    Cz_Lsminfo *lsminfo = (Cz_Lsminfo*) lsmfield; 
    lsminfo->s32DimensionX = image_width;
    lsminfo->s32DimensionY = image_length;
    lsminfo->s32DimensionZ = depth;
    lsminfo->s32DimensionTime = 1;
    if (image->number_channels == 1) {
      switch (image->channels[0]->bytes_per_pixel) {
      case 1:
	lsminfo->s32DataType = 1;
	break;
      case 2:
	lsminfo->s32DataType = 2;
	break;
      case 4:
	lsminfo->s32DataType = 5;
	break;
      default:
	TZ_ERROR(ERROR_DATA_TYPE);
      }
    } else {
      lsminfo->s32DataType = 0;
      uint32_t *channelDataTypes = 
	(uint32_t *) (lsmfield + lsminfo->u32OffsetChannelDataTypes);
      int i;
      for (i = 0; i < image->number_channels; i++) {
	switch (image->channels[i]->bytes_per_pixel) {
	case 1:
	  channelDataTypes[i] = 1;
	  break;
	case 2:
	  channelDataTypes[i] = 2;
	  break;
	case 4:
	  channelDataTypes[i] = 5;
	  break;
	default:
	  TZ_ERROR(ERROR_DATA_TYPE);
	}
      }
    }
    Set_Tiff_Tag(ifd, TIFF_CZ_LSMINFO, TIFF_BYTE, count, lsmfield);
  }  

  return ifd;
}


