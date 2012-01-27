/*
 * Copyright (c)2006-2010  Hanchuan Peng (Janelia Farm, Howard Hughes Medical Institute).  
 * All rights reserved.
 */


/************
                                            ********* LICENSE NOTICE ************

This folder contains all source codes for the V3D project, which is subject to the following conditions if you want to use it. 

You will ***have to agree*** the following terms, *before* downloading/using/running/editing/changing any portion of codes in this package.

1. This package is free for non-profit research, but needs a special license for any commercial purpose. Please contact Hanchuan Peng for details.

2. You agree to appropriately cite this work in your related studies and publications.

Peng, H., Ruan, Z., Long, F., Simpson, J.H., and Myers, E.W. (2010) “V3D enables real-time 3D visualization and quantitative analysis of large-scale biological image data sets,” Nature Biotechnology, Vol. 28, No. 4, pp. 348-353, DOI: 10.1038/nbt.1612. ( http://penglab.janelia.org/papersall/docpdf/2010_NBT_V3D.pdf )

Peng, H, Ruan, Z., Atasoy, D., and Sternson, S. (2010) “Automatic reconstruction of 3D neuron structures using a graph-augmented deformable model,” Bioinformatics, Vol. 26, pp. i38-i46, 2010. ( http://penglab.janelia.org/papersall/docpdf/2010_Bioinfo_GD_ISMB2010.pdf )

3. This software is provided by the copyright holders (Hanchuan Peng), Howard Hughes Medical Institute, Janelia Farm Research Campus, and contributors "as is" and any express or implied warranties, including, but not limited to, any implied warranties of merchantability, non-infringement, or fitness for a particular purpose are disclaimed. In no event shall the copyright owner, Howard Hughes Medical Institute, Janelia Farm Research Campus, or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; reasonable royalties; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

4. Neither the name of the Howard Hughes Medical Institute, Janelia Farm Research Campus, nor Hanchuan Peng, may be used to endorse or promote products derived from this software without specific prior written permission.

*************/



//revised from an earlier version developed by Gene Myers
// changed the include file name by PHC 060816
//20100409: change exit to return



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>

#include "mg_utilities.h"
#include "mg_image_lib11.h"

static int Warnings = 1;

static int error(char *msg, char *arg)
{ fprintf(stderr,"\nError in TIFF library:\n   ");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  return 1;
}

/*********** STACK PLANE SELECTION ****************************/

Stack_Plane *Select_Plane(Stack *a_stack, int plane)  // Build an image for a plane of a stack
{ static Stack_Plane My_Image;

  if (plane < 0 || plane >= a_stack->depth)
    return (NULL);
  My_Image.kind   = a_stack->kind;
  My_Image.width  = a_stack->width;
  My_Image.height = a_stack->height;
  My_Image.array  = a_stack->array + plane*a_stack->width*a_stack->height*a_stack->kind;
  return (&My_Image);
}

/*********** SPACE MANAGEMENT ****************************/

// Raster working buffer

static uint32 *get_raster(int npixels, char *routine)
{ static uint32 *Raster = NULL;
  static int     Raster_Size = 0;                           //  Manage read work buffer

  if (npixels < 0)
    { free(Raster);
      Raster_Size = 0;
      Raster      = NULL;
    }
  else if (npixels > Raster_Size)
    { Raster_Size = npixels;
      Raster = (uint32 *) Guarded_Realloc(Raster,sizeof(uint32)*Raster_Size,routine);
    }
  return (Raster);
}

// Awk-generated (manager.awk) Image memory management

static inline int image_asize(Image *image)
{ return (image->height*image->width*image->kind); }

//  Image-routines: new_image, pack_image (Free|Kill)_Image, reset_image, Image_Usage

typedef struct __Image
  { struct __Image *next;
    int             asize;
    Image           image;
  } _Image;

static _Image *Free_Image_List = NULL;
static int    Image_Offset, Image_Inuse;

static inline Image *new_image(int asize, char *routine)
{ _Image *object;

  if (Free_Image_List == NULL)
    { object = (_Image *) Guarded_Malloc(sizeof(_Image),routine);
      Image_Offset = ((char *) &(object->image)) - ((char *) object);
      object->asize = asize;
      object->image.array = (uint8 *)Guarded_Malloc(asize,routine);
      Image_Inuse += 1;
    }
  else
    { object = Free_Image_List;
      Free_Image_List = object->next;
      if (object->asize < asize)
        { object->asize = asize;
          object->image.array = (uint8 *)Guarded_Realloc(object->image.array,
                                                asize,routine);
        }
    }
  return (&(object->image));
}

inline void pack_image(Image *image)
{ _Image *object  = (_Image *) (((char *) image) - Image_Offset);
  if (object->asize != image_asize(image))
    { object->asize = image_asize(image);
      object->image.array = (uint8 *)Guarded_Realloc(object->image.array,
                                            object->asize,"Pack_Image");
    }
}

void Free_Image(Image *image)
{ _Image *object  = (_Image *) (((char *) image) - Image_Offset);
  object->next = Free_Image_List;
  Free_Image_List = object;
  Image_Inuse -= 1;
}

void Kill_Image(Image *image)
{ free(image->array);
  free(((char *) image) - Image_Offset);
  Image_Inuse -= 1;
}

void reset_image()
{ _Image *object;
  while (Free_Image_List != NULL)
    { object = Free_Image_List;
      Free_Image_List = object->next;
      Kill_Image(&(object->image));
      Image_Inuse += 1;
    }
}

int Image_Usage()
{ return (Image_Inuse); }

Image *Copy_Image(Image *image)
{ Image  *copy = new_image(image_asize(image),"Copy_Image");

  copy->kind   = image->kind;
  copy->width  = image->width;
  copy->height = image->height;
  memcpy(copy->array,image->array,image_asize(image));
  return (copy);
}

void Pack_Image(Image *image)
{ pack_image(image); }

void Reset_Image()
{ reset_image();
  get_raster(-1,NULL);
}

// Awk-generated (manager.awk) Stack memory management

static inline int stack_vsize(Stack *stack)
{ return (stack->depth*stack->height*stack->width*stack->kind); }

//  Stack-routines: new_stack, pack_stack (Free|Kill)_Stack, reset_stack, Stack_Usage

typedef struct __Stack
  { struct __Stack *next;
    int             vsize;
    Stack           stack;
  } _Stack;

static _Stack *Free_Stack_List = NULL;
static int    Stack_Offset, Stack_Inuse;

static inline Stack *new_stack(int vsize, char *routine)
{ _Stack *object;

  if (Free_Stack_List == NULL)
    { object = (_Stack *) Guarded_Malloc(sizeof(_Stack),routine);
      Stack_Offset = ((char *) &(object->stack)) - ((char *) object);
      object->vsize = vsize;
      object->stack.array = (uint8 *)Guarded_Malloc(vsize,routine);
      Stack_Inuse += 1;
    }
  else
    { object = Free_Stack_List;
      Free_Stack_List = object->next;
      if (object->vsize < vsize)
        { object->vsize = vsize;
          object->stack.array = (uint8 *)Guarded_Realloc(object->stack.array,
                                                vsize,routine);
        }
    }
  return (&(object->stack));
}

inline void pack_stack(Stack *stack)
{ _Stack *object  = (_Stack *) (((char *) stack) - Stack_Offset);
  if (object->vsize != stack_vsize(stack))
    { object->vsize = stack_vsize(stack);
      object->stack.array = (uint8 *)Guarded_Realloc(object->stack.array,
                                            object->vsize,"Pack_Stack");
    }
}

void Free_Stack(Stack *stack)
{ _Stack *object  = (_Stack *) (((char *) stack) - Stack_Offset);
  object->next = Free_Stack_List;
  Free_Stack_List = object;
  Stack_Inuse -= 1;
}

void Kill_Stack(Stack *stack)
{ free(stack->array);
  free(((char *) stack) - Stack_Offset);
  Stack_Inuse -= 1;
}

void reset_stack()
{ _Stack *object;
  while (Free_Stack_List != NULL)
    { object = Free_Stack_List;
      Free_Stack_List = object->next;
      Kill_Stack(&(object->stack));
      Stack_Inuse += 1;
    }
}

int Stack_Usage()
{ return (Stack_Inuse); }

Stack *Copy_Stack(Stack *stack)
{ Stack *copy = new_stack(stack_vsize(stack),"Copy_Stack");

  copy->kind   = stack->kind;
  copy->width  = stack->width;
  copy->height = stack->height;
  copy->depth  = stack->depth;
  memcpy(copy->array,stack->array,stack_vsize(stack));
  return (copy);
}

void Pack_Stack(Stack *stack)
{ pack_stack(stack); }

void Reset_Stack()
{ reset_stack();
  get_raster(-1,NULL);
}


/*********** TIFF INTERFACE ****************************/

TIFF *Open_Tiff(char *file_name, char *mode)
{ TIFF *tif;

  if (Warnings)
    { Warnings = 0;
      TIFFSetWarningHandler(NULL);
    }

  tif = TIFFOpen(file_name,mode);
  if (tif == NULL)
    error("cannot opening TIFF file %s", file_name);
  return (tif);
}    

static int determine_kind(TIFF *tif)   //  Determine nature of current tif image
{ short bits, channels, photo;

  TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bits);
  TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &channels);
  TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photo);
  if (photo <= 1)
    { if (channels > 1)
        error("Black and white tiff has more than 1 channel!",NULL);
      if (bits == 16)
        return (GREY16);
      else
        return (GREY);
    }
  else
    return (COLOR);
}

static void read_directory(TIFF *tif, Image *image, char *routine)   //  Used by all readers// 
{ uint32 *raster;
  uint8  *row;
  int     width, height;

  width  = image->width;
  height = image->height;
  raster = get_raster(width*height,routine);

  row = image->array;

  if (image->kind != GREY16)

    { int i, j;
      uint32 *in;
      uint8  *out;

      if (TIFFReadRGBAImage(tif,width,height,raster,0) == 0)
        error("read of tif failed in read_directory()", NULL);

      in = raster;
      if (image->kind == GREY)
        { for (j = height-1; j >= 0; j--)
            { out = row;
              for (i = 0; i < width; i++)
                { uint32 pixel = *in++;
                  *out++ = TIFFGetR(pixel);
                }
              row += width;
            }
        }
      else
        { for (j = height-1; j >= 0; j--)
            { out = row;
              for (i = 0; i < width; i++)
                { uint32 pixel = *in++;
                  *out++ = TIFFGetR(pixel);
                  *out++ = TIFFGetG(pixel);
                  *out++ = TIFFGetB(pixel);
                }
              row += width*3;
            }
        }
    }

  else

    { int tile_width, tile_height;

      if (TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width))    // File is tiled  
        { int x, y;
          int i, j;
          int m, n;
          uint16 *buffer = (uint16 *) raster;
          uint16 *out, *in, *rous;
    
          TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height);

          for (y = 0; y < height; y += tile_height)
            { if (y + tile_height > height)
                n = height - y;
              else
                n = tile_height;
              for (x = 0; x < width; x += tile_width)
                { TIFFReadTile(tif, buffer, x, y, 0, 0);
                  if (x + tile_width > width)
                    m = width - x;
                  else
                    m = tile_width;
                  for (j = 0; j < n; j++)
                    { out = (uint16 *) (row + 2*(j*width + x));
                      in  = buffer + j*tile_width;
                      for (i = 0; i < m; i++)
                        *out++ = *in++; 
                    }
                }
              row += n*width*2;
            }
        }

      else    // File is striped
   
        { int     y;

          for (y = 0; y < height; y++)
            { TIFFReadScanline(tif, row, y, 0);
              row += width*2;
            }
        }
    }
}
//static void read_directory_location(TIFF *tif, Image *image, int xs,int ys,int xe,int ye,char *routine)   // modify the funtion /// yang jinzhu
//{ 
//	uint32 *raster;
//	uint8  *row;
//	int    width, height;
//	
//	width  = image->width;
//	height = image->height;
//	raster = get_raster(width*height,routine);
//	
//	row = image->array;
//	
//	if (image->kind != GREY16)
//		
//    { 
//		int i, j;
//		uint32 *in;
//		uint8  *out;
//		
//		if (TIFFReadRGBAImage(tif,width,height,raster,0) == 0)
//			error("read of tif failed in read_directory()", NULL);
//		in = raster;
//		
//		if (image->kind == GREY)
//        { 
//			for (j = ye-1; j >= ye; j--)
//			{ 
//				out = row;
//				for (i = xs; i < xe; i++)
//				{
//					uint32 pixel = (*in+xs)++;
//					*out++ = TIFFGetR(pixel);
//				}
//				row += width;
//			}
//        }
//		else
//        { 
//			for (j = ye-1; j >= ye; j--)
//			{ 
//				out = row;
//				for (i = xs; i < xe; i++)
//				{ 
//					uint32 pixel = *in++;
//					*out++ = TIFFGetR(pixel);
//					*out++ = TIFFGetG(pixel);
//					*out++ = TIFFGetB(pixel);
//				}
//				row += width*3;
//			}
//		}
//    }	
//	else
//    { 
//		int tile_width, tile_height;
//		
//		if (TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width))    // File is tiled  
//        { 
//			int x, y;
//			int i, j;
//			int m, n;
//			uint16 *buffer = (uint16 *) raster;
//			uint16 *out, *in, *rous;
//			
//			TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height);
//			
//			for (y = 0; y < height; y += tile_height)
//            {
//				if (y + tile_height > height)
//                n = height - y;
//			else
//                n = tile_height;
//				for (x = 0; x < width; x += tile_width)
//                { 
//					TIFFReadTile(tif, buffer, x, y, 0, 0);
//					if (x + tile_width > width)
//						m = width - x;
//					else
//						m = tile_width;
//					for (j = 0; j < n; j++)
//                    { 
//						out = (uint16 *) (row + 2*(j*width + x));
//						in  = buffer + j*tile_width;
//						for (i = 0; i < m; i++)
//							*out++ = *in++; 
//                    }
//                }
//				row += n*width*2;
//            }
//        }
//		
//		else    // File is striped
//        { 
//			int y;
//			for (y = 0; y < height; y++)
//            { 
//				TIFFReadScanline(tif, row, y, 0);
//				row += width*2;
//            }
//        }
//    }
//}
Image *Read_Tiff(TIFF *tif, int *lastone)
{ Image *image;

  int   width, height, kind;

  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  kind = determine_kind(tif);

  image = new_image(height*width*kind,"Read_Tiff");

  image->width  = width;
  image->height = height;
  image->kind   = kind;

  read_directory(tif,image,"Read_Tiff");

  *lastone = (! TIFFReadDirectory(tif));
  return (image);
}

void Write_Tiff(TIFF *tif, Image *a_image)
{ int   n;

  if (a_image->kind == COLOR)
    TIFFSetField(tif,TIFFTAG_PHOTOMETRIC,(uint16)  2);
  else     // ->kind == GREY or GREY16
    TIFFSetField(tif,TIFFTAG_PHOTOMETRIC,(uint16)  1);
  TIFFSetField(tif,TIFFTAG_ROWSPERSTRIP,(uint32) 0xFFFFFFFF);
  TIFFSetField(tif,TIFFTAG_XRESOLUTION,(float) 72.);
  TIFFSetField(tif,TIFFTAG_YRESOLUTION,(float) 72.);
  TIFFSetField(tif,TIFFTAG_RESOLUTIONUNIT,(uint16) 2);
  TIFFSetField(tif,TIFFTAG_PLANARCONFIG,(uint16) 1);
  TIFFSetField(tif,TIFFTAG_ORIENTATION,(uint16) 4);

  if (a_image->kind == GREY16)
    { TIFFSetField(tif,TIFFTAG_BITSPERSAMPLE,(uint16) 16);
      TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL,(uint16) 1);
    }
  else
    { TIFFSetField(tif,TIFFTAG_BITSPERSAMPLE,(uint16) 8);
      TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL,(uint16) (a_image->kind));
    }
  TIFFSetField(tif,TIFFTAG_IMAGEWIDTH,(uint32) (a_image->width));
  TIFFSetField(tif,TIFFTAG_IMAGELENGTH,(uint32) (a_image->height));

  n = TIFFWriteEncodedStrip(tif,0,a_image->array,image_asize(a_image));
  if (n < 0)
    error("error writing TIFF file",NULL);

  TIFFWriteDirectory(tif);
}

void Close_Tiff(TIFF *tif)
{ TIFFClose(tif); }


/*********** READ + WRITE INTERFACE ****************************/

void Parse_Stack_Name(char *file_name, char **prefix, int *num_width, int *first_num)
{ static char *Prefix = NULL;
  static int   Prefix_Max = 0;

  char *s, *t, c;

  s = file_name + strlen(file_name) - 4;
  if (strcmp(s,".tif") != 0)
    error("1st file, %s, in stack does not have .tif extension",file_name);
  t = s;
  while (t > file_name && isdigit(t[-1]))
    t -= 1;
  if (s-t <= 0)
    error("No number sequence in stack file names %s",file_name);

  if (t-file_name > Prefix_Max)
    { Prefix_Max = (int)((t-file_name)*1.2 + 20);
      Prefix     = (char *) Guarded_Realloc(Prefix,Prefix_Max+1,"Parse_Stack_Name");
    }

  c = *t;
  *t = '\0';
  strcpy(Prefix,file_name);
  *t = c;

  *prefix    = Prefix;
  *num_width = s-t;
  *first_num = atoi(t);
}

Image *Read_Image(char *file_name)
{ TIFF  *tif;
  Image *img;
  int    lastone;

  tif = Open_Tiff(file_name,"r");

  img = Read_Tiff(tif,&lastone);

  TIFFClose(tif);

  return (img);
}

Stack *Read_Stack(char *file_name)
{ Stack *stack;

  TIFF  *tif;
  int    depth, width, height, kind;

  tif = Open_Tiff(file_name,"r");
  depth = 1;
  while (TIFFReadDirectory(tif))
    depth += 1;
  TIFFClose(tif);

  tif = Open_Tiff(file_name,"r");
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  kind  = determine_kind(tif);
  stack = new_stack(depth*height*width*kind,"Read_Stack");

  stack->width  = width;
  stack->height = height;
  stack->depth  = depth;
  stack->kind   = kind;

  { int d;

    d = 0;
    while (1)
      { read_directory(tif,Select_Plane(stack,d),"Read_Stack");

        d += 1;
        if (!TIFFReadDirectory(tif)) break;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        if (width != stack->width || height != stack->height)
          error("Images of stack are not of the same dimensions!",NULL);

        kind = determine_kind(tif);
        if (kind != stack->kind)
          error("Images of stack are not of the same type (GREY, GREY16, or COLOR)!",NULL);
      }
  }

  TIFFClose(tif);

  return (stack);
}
Stack *Read_Stack_location(char *file_name, int zs,int ze)//modify the funtion yangjinzhu
{ 
	Stack *stack;

	TIFF  *tif;
	
	int    depth, width, height, kind;
	
	tif = Open_Tiff(file_name,"r");
	depth = 1;
	while (TIFFReadDirectory(tif))
		depth += 1;
	TIFFClose(tif);
	
	tif = Open_Tiff(file_name,"r");
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	
	kind  = determine_kind(tif);
	stack = new_stack((ze-zs)*height*width*kind,"Read_Stack");
	//stack = new_stack(depth*height*width*kind,"Read_Stack");
	
	stack->width  = width;
	stack->height = height;
	stack->depth = (ze-zs);
	//stack->depth  = depth;
	stack->kind   = kind;
	//for (int i = 0; i<zs; i++)
//	{
//		TIFFReadDirectory(tif);
//		
//	}
	TIFFSetDirectory(tif,zs);
	for( int d = 0; d < (ze-zs); d++ )
	{
		//TIFFSetDirectory(tif,d);
		
		read_directory(tif,Select_Plane(stack,(d)),"Read_Stack");
		
		//d += 1;
		if (!TIFFReadDirectory(tif)) break;
		
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
		if (width != stack->width || height != stack->height)
			error("Images of stack are not of the same dimensions!",NULL);
		
		kind = determine_kind(tif);
		if (kind != stack->kind)
			error("Images of stack are not of the same type (GREY, GREY16, or COLOR)!",NULL);
	}
	
	//{ int d;
//		
//		d = 0;
//		while (1)
//		{ read_directory(tif,Select_Plane(stack,d),"Read_Stack");
//			
//			d += 1;
//			if (!TIFFReadDirectory(tif)) break;
//			
//			TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
//			TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
//			if (width != stack->width || height != stack->height)
//				error("Images of stack are not of the same dimensions!",NULL);
//			
//			kind = determine_kind(tif);
//			if (kind != stack->kind)
//				error("Images of stack are not of the same type (GREY, GREY16, or COLOR)!",NULL);
//		}
//	}
	
	TIFFClose(tif);
	
	return (stack);
}
Stack *Read_LSM_Stack(char *file_name)
{
  Stack *stack;

  TIFF  *tif;
  int    depth, width, height, kind;

  tif = Open_Tiff(file_name,"r");
  if (!tif) return 0; //070805, bu Hanchuan Peng
	
  depth = 1;
  while (TIFFReadDirectory(tif))
    depth += 1;
  TIFFClose(tif);
  depth = depth / 2;		/* half the dirs are thumbnails */

  tif = Open_Tiff(file_name,"r");
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  kind  = determine_kind(tif);
  stack = new_stack(depth*height*width*kind,"Read_Stack");

  stack->width  = width;
  stack->height = height;
  stack->depth  = depth;
  stack->kind   = kind;

  printf("test1 done\n");

  int d;

  d = 0;
  /* read every other directory (real data, the in between are thumbnails */
  while (1)
  {
	  read_directory(tif,Select_Plane(stack,d),"Read_Stack");
	  
	  d += 1;
	  if (!TIFFReadDirectory(tif)) break;
	  /* skip the one we just read, it's a thumbnail  */

	  if (!TIFFReadDirectory(tif)) break;
	  
	  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
	  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
	  if (width != stack->width || height != stack->height)
          error("Images of stack are not of the same dimensions!",NULL);
	  
	  kind = determine_kind(tif);
	  if (kind != stack->kind)
          error("Images of stack are not of the same type (GREY, GREY16, or COLOR)!",NULL);
  }
  
  TIFFClose(tif);

  return (stack);
}

Stack *Read_Stack_Planes(char *prefix, int num_width, int first_num)
{ Stack *stack;

  char  sname[1000];
  int   width, height, depth, kind;
  TIFF *tif;

  depth = 0;
  while (1)
    { FILE *fd;

      sprintf(sname,"%s%0*d.tif",prefix,num_width,first_num+depth);
      if ((fd = fopen(sname,"r")) == NULL)
        break;
      fclose(fd);

      depth += 1;
    }

  sprintf(sname,"%s%0*d.tif",prefix,num_width,first_num);
  tif = Open_Tiff(sname,"r");
  TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

  kind  = determine_kind(tif);
  stack = new_stack(depth*height*width*kind,"Read_Stack_Planes");

  stack->width  = width;
  stack->height = height;
  stack->depth  = depth;
  stack->kind   = kind;

  { int d;

    d = 0;
    while (1)
      { read_directory(tif,Select_Plane(stack,d),"Read_Stack_Planes");
        TIFFClose(tif);

        d += 1;
        if (d >= depth) break;

        sprintf(sname,"%s%0*d.tif",prefix,num_width,first_num+d);
        tif = Open_Tiff(sname,"r");
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);
        if (width != stack->width || height != stack->height)
          error("Images of stack are not of the same dimensions!",NULL);

        kind = determine_kind(tif);
        if (kind != stack->kind)
          error("Images of stack are not of the same type (GREY, GREY16, or COLOR)!",NULL);
      }
  }

  return (stack);
}

void Write_Image(char *file_name, Image *a_image)
{ TIFF *tif;

  tif = Open_Tiff(file_name,"w");
  Write_Tiff(tif,a_image);
  TIFFClose(tif);
}

void Write_Stack(char *file_name, Stack *a_stack)
{ TIFF *tif;
  int   i;

  tif = Open_Tiff(file_name,"w");
  for (i = 0; i < a_stack->depth; i++)
    Write_Tiff(tif,Select_Plane(a_stack,i));
  TIFFClose(tif);
}

void Write_Stack_Planes(char *prefix, int num_width, int first_num, Stack *a_stack)
{ char *name;
  int   n;

  name = (char *) Guarded_Malloc(strlen(prefix)+50,"Write_Stack_Planes");
  for (n = 0; n < a_stack->depth; n++)
    { sprintf(name,"%s.%0*d.tif",prefix,num_width,first_num+n);
      Write_Image(name,Select_Plane(a_stack,n));
    }
  free(name);
}


/*********** MAKE (EMPTY) IMAGES AND STACKS ****************************/

Image *Make_Image(int kind, int width, int height)
{ Image *image;

  if (Warnings)
    { Warnings = 0;
      TIFFSetWarningHandler(NULL);
    }

  image = new_image(height*width*kind,"Make_Image");

  image->width  = width;
  image->height = height;
  image->kind   = kind;

  return (image);
}

Stack *Make_Stack(int kind, int width, int height, int depth)
{ Stack *stack;

  if (Warnings)
    { Warnings = 0;
      TIFFSetWarningHandler(NULL);
    }

  stack = new_stack(depth*height*width*kind,"Make_Stack");

  stack->width  = width;
  stack->height = height;
  stack->depth  = depth;
  stack->kind   = kind;

  return (stack);
}


/*********** COMPUTE RANGES AND SCALE IMAGES AND STACKS *********************/

//  Compute min and max values in 'array' of type 'kind' with 'length' elements

static Pixel_Range *compute_minmax(uint8 *array, int kind, int length, int channel)
{ static Pixel_Range My_Range;
  uint16 *array16;
  int     i, x;
  int     min, max;

  min = 0xFFFF;
  max = 0;
  if (kind == GREY16)
    { array16 = (uint16 *) array;
      for (i = 0; i < length; i++)
        { x = array16[i];
          if (x < min)
            min = x;
          if (x > max)
            max = x;
        }
    }
  else
    { if (kind == COLOR)
        { length *= 3;
          array  += channel;
        }
      for (i = 0; i < length; i += kind)
        { x = array[i];
          if (x < min)
            min = x;
          if (x > max)
            max = x;
        }
    }

  My_Range.maxval = max;
  My_Range.minval = min;
  return (&My_Range);
}

Pixel_Range *Image_Range(Image *image, int channel)
{ return (compute_minmax(image->array,image->kind,image->width*image->height,channel)); }

Pixel_Range *Stack_Range(Stack *stack, int channel)
{ return (compute_minmax(stack->array,stack->kind,
                         stack->width*stack->height*stack->depth,channel));
}

//  Compute min and max values in 'array' of type 'kind' with 'length' elements

static void scale_values(uint8 *array, int kind, int length, int channel,
                         double factor, double offset)
{ static Pixel_Range My_Range;
  uint16 *array16;
  int     i, x;

  if (kind == GREY16)
    { array16 = (uint16 *) array;
      for (i = 0; i < length; i++)
        { x = factor*array16[i] + offset;
          array16[i] = x;
        }
    }
  else
    { if (kind == COLOR)
        { length *= 3;
          array += channel;
        }
      for (i = 0; i < length; i += kind)
        { x = factor*array[i] + offset;
          array[i] = x;
        }
    }
}

void Scale_Image(Image *image, int channel, double factor, double offset)
{ return (scale_values(image->array,image->kind,image->width*image->height,
                       channel,factor,offset));
}

void Scale_Stack(Stack *stack, int channel, double factor, double offset)
{ return (scale_values(stack->array,stack->kind,stack->width*stack->height*stack->depth,
                       channel,factor,offset));
}


/*********** CONVERT IMAGES AND STACKS  *********************/

static void translate(int skind, uint8 *in8, int tkind, uint8 *out8, int length)
{ uint16 *in16, *out16;
  int     i, x, maxval; 
  double  c, scale;

  if (skind == GREY16)
    { maxval = compute_minmax(in8,skind,length,0)->maxval;
      if (maxval > 255)
        scale  = 255. / maxval;
      else
        scale  = 1.;
    }

  if (tkind > skind)
    { in8  += length*skind;
      out8 += length*tkind;
    }
  in16  = (uint16 *) in8;
  out16 = (uint16 *) out8;
    
     
  if (tkind == COLOR)
    if (skind == GREY)
      for (i = length; i > 0; i--)   // G->C
        { x = *--in8;
          *--out8 = x;
          *--out8 = x;
          *--out8 = x;
        }
    else
      for (i = length; i > 0; i--)   // G16->C
        { x = (*--in16) * scale;
          *--out8 = x;
          *--out8 = x;
          *--out8 = x;
        }
  else if (tkind == GREY16)
    if (skind == COLOR)
      for (i = length; i > 0; i--)   // C->G16
        { c  = .3 * (*in8++);
          c += .59 * (*in8++);
          c += .11 * (*in8++);
          *out16++ = (uint16) c;
        }
    else
      for (i = length; i > 0; i--)   // G->G16
        { *--out16 = *--in8; }
  else  // tkind == GREY
    if (skind == COLOR)
      for (i = length; i > 0; i--)   // C->G
        { c  = .3 * (*in8++);
          c += .59 * (*in8++);
          c += .11 * (*in8++);
          *out8++ = (uint8) c;
        }
    else
      for (i = length; i > 0; i--)   // G16->G
        { *out8++ = (*in16++) * scale; }
}

Image *Translate_Image(Image *image, int kind, int in_place)
{ int width, height;

  width  = image->width;
  height = image->height;

  if (in_place)
    { if (image->kind == kind)
        return (image);

      if (kind > image->kind)
        { _Image *object  = (_Image *) (((char *) image) - Image_Offset);
          if (object->asize < width * height * kind)
            { object->asize = width * height * kind;
              image->array  = (uint8 *)Guarded_Realloc(image->array,object->asize,"Translate_Image");
            }
        }

      translate(image->kind,image->array,kind,image->array,width*height);

      image->kind = kind;

      return (image);
    }
  else
    { Image  *xlate;

      if (image->kind == kind)
        return (Copy_Image(image));

      xlate  = new_image(kind*width*height,"Translate_Image");
      xlate->width  = width;
      xlate->height = height;
      xlate->kind   = kind;

      translate(image->kind,image->array,kind,xlate->array,width*height);

      return (xlate);
    }
}

Stack *Translate_Stack(Stack *stack, int kind, int in_place)
{ int width, height, depth;

  width  = stack->width;
  height = stack->height;
  depth  = stack->depth;

  if (in_place)
    { if (stack->kind == kind)
        return (stack);

      if (kind > stack->kind)
        { _Stack *object  = (_Stack *) (((char *) stack) - Stack_Offset);

          if (object->vsize < width * height * depth * kind)
            { object->vsize = width * height * depth * kind;
              stack->array  = (uint8 *)Guarded_Realloc(stack->array,object->vsize,"Translate_Stack");
            }
        }

      translate(stack->kind,stack->array,kind,stack->array,width*height*depth);

      stack->kind = kind;

      return (stack);
    }
  else
    { Stack *xlate;

      if (stack->kind == kind)
        return (Copy_Stack(stack));

      xlate  = new_stack(kind*width*height*depth,"Translate_Stack");
      xlate->depth  = depth;
      xlate->width  = width;
      xlate->height = height;
      xlate->kind   = kind;

      translate(stack->kind,stack->array,kind,xlate->array,width*height*depth);

      return (xlate);
    }
}
