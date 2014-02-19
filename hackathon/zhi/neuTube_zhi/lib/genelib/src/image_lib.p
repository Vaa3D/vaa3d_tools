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
\*****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utilities.h"
#include "tiff_io.h"
#include "tiff_image.h"
#include "image_lib.h"

static void error(char *msg, char *arg)
{ fprintf(stderr,"\nError in image library:\n   ");
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  exit(1);
}

/*********** STACK PLANE SELECTION ****************************/

static char Empty_String[1] = { 0 };

Stack_Plane *Select_Plane(Stack *a_stack, int plane)  // Build an image for a plane of a stack
{ static Stack_Plane My_Image;

  if (plane < 0 || plane >= a_stack->depth)
    return (NULL);
  My_Image.kind   = a_stack->kind;
  My_Image.width  = a_stack->width;
  My_Image.height = a_stack->height;
  My_Image.text   = Empty_String;
  My_Image.array  = a_stack->array + plane*a_stack->width*a_stack->height*a_stack->kind;
  return (&My_Image);
}

int Set_Stack_Plane(Stack *stack, int plane, Image *image)
{ uint8 *ip, *sp;
  int    i;
  int    area;

  if (plane < 0 || plane >= stack->depth)
    return (1);
  if (image->width != stack->width || image->height != stack->height)
    return (1);
  if (image->kind != stack->kind)
    return (1);

  area = image->kind * image->width * image->height;

  ip = image->array;
  sp = stack->array + area*plane;
  for (i = 0; i < area; i++)
    *sp++ = *ip++;
  return (0);
}

/*********** SPACE MANAGEMENT ****************************/

typedef struct
  { Tiff_Reader *reader;
    Tiff_Writer *writer;
    int          eof;
    Tiff_IFD    *ifd;
    Tiff_Image  *image;
  } Tio;

MANAGER Tiff(Tio) reader*Tiff_Reader writer^Tiff_Writer ifd^Tiff_IFD image^Tiff_Image

// Awk-generated (manager.awk) Image memory management

static inline int image_asize(Image *image)
{ return (image->height*image->width*image->kind); }

static inline int image_tsize(Image *image)
{ return (strlen(image->text)+1); }

MANAGER Image array:asize text:tsize

// Awk-generated (manager.awk) Stack memory management

static inline size_t stack_vsize(Stack *stack)
{ return (stack->depth*(size_t)stack->height*stack->width*stack->kind); }

static inline size_t stack_tsize(Stack *stack)
{ return (strlen(stack->text)+1); }

MANAGER Stack array:vsize text:tsize

/*********** TIFF INTERFACE ****************************/

Tiff *Open_Tiff_E(const char *file_name, char *mode, int lsm)
{ Tio *tif;

  tif = new_tio("Open_Tiff_E");

  if (strcmp(mode,"r") == 0)
    { tif->reader = Open_Tiff_Reader((char*)file_name,NULL,lsm);
      if (tif->reader == NULL)
        error("Error reading Tif: '%s'\n",Tiff_Error_String());
      tif->writer = NULL;
      tif->eof    = End_Of_Tiff(tif->reader);
      if (tif->eof)
        { tif->ifd   = NULL;
          tif->image = NULL;
        }
      else
        { tif->ifd   = Read_Tiff_IFD(tif->reader);
          if (tif->ifd == NULL)
            error("Error reading Tif IFD: '%s'\n",Tiff_Error_String());
          tif->image = Extract_Image_From_IFD(tif->ifd);
          if (tif->image == NULL)
            error("Error reading Tif Image: '%s'\n",Tiff_Image_Error());
        }
    }
  else if (strcmp(mode,"w") == 0)
    { tif->writer = Open_Tiff_Writer((char*)file_name,0);
      tif->reader = NULL;
    }
  else
    error("Mode must be either 'r' or 'w'\n",NULL);

  return ((Tiff *) tif);
}

Tiff *Open_Tiff(char *file_name, char *mode)
{ 
  return Open_Tiff_E((char*) file_name, mode, 0);
}

static int determine_kind(Tio *tif)   //  Determine nature of current tif image
{ Tiff_Image   *img;
  Tiff_Channel *chan;

  img  = tif->image;
  chan = img->channels[0];
  if (chan->interpretation == CHAN_RED || chan->interpretation == CHAN_MAPPED)
    return (COLOR);
  else if (chan->type == CHAN_FLOAT)
    return (FLOAT32);
  else if (chan->type == CHAN_SIGNED) {
    error("Tiff with signed pixel values is not supported",NULL);
    return -1; }
  else if (chan->scale > 8)
    return (GREY16);
  else
    return (GREY);
}

static void read_directory(Tio *tif, Image *image, char *routine)   //  Used by all readers
{ Tiff_Channel *chan;
  int           i, area;

  chan = tif->image->channels[0];
  area = image->width * image->height;

  switch (image->kind)
  { case FLOAT32:
      memcpy(image->array,chan->plane,area*4);
      break;
    case COLOR:
      if (chan->interpretation == CHAN_MAPPED)
        { uint8 *m, *map;
          uint8 *out, *plane;

          out   = image->array;
          plane = (uint8*)chan->plane;
          map   = (uint8 *) (tif->image->map);
          for (i = 0; i < area; i++)
            { m = map + 6*plane[i];
              *out++ = m[0];
              *out++ = m[2];
              *out++ = m[4];
            }
        }
      else
        { uint8 *red, *green, *blue;
          uint8 *out;

          out   = image->array;
          red   = (uint8*)chan->plane;
          green = (uint8*)(tif->image->channels[1]->plane);
          blue  = (uint8*)(tif->image->channels[2]->plane);
          for (i = 0; i < area; i++)
            { *out++ = *red++;
              *out++ = *green++;
              *out++ = *blue++;
            }
        }
      break;
    case GREY16:
       Shift_Tiff_Channel(chan,16-chan->scale);
       memcpy(image->array,chan->plane,area*2);
       break;
     case GREY:
       memcpy(image->array,chan->plane,area);
       break;
  }
}

void Advance_Tiff(Tiff *etif)
{ Tio *tif = (Tio *) etif;

  if (tif->eof) return;
  Free_Tiff_Image(tif->image);
  Free_Tiff_IFD(tif->ifd);
  tif->eof = End_Of_Tiff(tif->reader);
  if (tif->eof)
    { tif->ifd   = NULL;
      tif->image = NULL;
    }
  else
    { tif->ifd   = Read_Tiff_IFD(tif->reader);
      tif->image = Extract_Image_From_IFD(tif->ifd);
    }
}

int Tiff_EOF(Tiff *tif)
{ return (((Tio *) tif)->eof); }

Image *Read_Tiff(Tiff *etif)
{ Image *image;
  Tio   *tif = (Tio *) etif;

  Tiff_Type type;
  int   width, height, kind, count;
  char *text;

  if (tif->eof)
    return (NULL);

  if ((text = (char *) Get_Tiff_Tag(tif->ifd,TIFF_JF_TAGGER,&type,&count)) == NULL)
    { text  = Empty_String;
      count = 0;
    }

  kind   = determine_kind(tif);
  width  = tif->image->width;
  height = tif->image->height;

  image = new_image(height*width*kind,count+1,"Read_Tiff");

  image->width  = width;
  image->height = height;
  image->kind   = kind;
  strncpy(image->text,text,count);
  image->text[count] = '\0';

  read_directory(tif,image,"Read_Tiff");

  Advance_Tiff(tif);

  return (image);
}

void Write_Tiff(Tiff *etif, Image *a_image)
{ Tio        *tif = (Tio *) etif;
  Tiff_IFD   *ifd;
  Tiff_Image *img;
  int         area;

  area = a_image->width * a_image->height;

  img = Create_Tiff_Image(a_image->width,a_image->height);
  switch (a_image->kind)
  { case COLOR:
      Add_Tiff_Image_Channel(img,CHAN_RED,8,CHAN_UNSIGNED);
      Add_Tiff_Image_Channel(img,CHAN_GREEN,8,CHAN_UNSIGNED);
      Add_Tiff_Image_Channel(img,CHAN_BLUE,8,CHAN_UNSIGNED);

      { uint8 *red, *green, *blue;
        uint8 *out;
        int    i;

        out   = a_image->array;
        red   = (uint8*)img->channels[0]->plane;
        green = (uint8*)img->channels[1]->plane;
        blue  = (uint8*)img->channels[2]->plane;
        for (i = 0; i < area; i++)
          { *red++ = *out++;
            *green++ = *out++;
            *blue++ = *out++;
          }
      }

      break;
    case GREY:
      Add_Tiff_Image_Channel(img,CHAN_BLACK,8,CHAN_UNSIGNED);
      memcpy(img->channels[0]->plane,a_image->array,area);
      break;
    case GREY16:
      Add_Tiff_Image_Channel(img,CHAN_BLACK,16,CHAN_UNSIGNED);
      memcpy(img->channels[0]->plane,a_image->array,2*area);
      break;
    case FLOAT32:
      Add_Tiff_Image_Channel(img,CHAN_BLACK,32,CHAN_FLOAT);
      memcpy(img->channels[0]->plane,a_image->array,4*area);
      break;
  }

  ifd = Make_IFD_For_Image(img,0);
  
  if (a_image->text[0] != '\0')
    Set_Tiff_Tag(ifd,TIFF_JF_TAGGER,TIFF_BYTE,strlen(a_image->text),a_image->text);

  Write_Tiff_IFD(tif->writer,ifd);

  Free_Tiff_IFD(ifd);
  Free_Tiff_Image(img);
}

void Close_Tiff(Tiff *etif)
{ Tio *tif = (Tio *) etif;

  if (tif->reader != NULL)
    { if ( ! tif->eof)
        { Free_Tiff_Image(tif->image);
          Free_Tiff_IFD(tif->ifd);
        }
      Free_Tiff_Reader(tif->reader);
    }
  else
    { Close_Tiff_Writer(tif->writer);
      Free_Tiff_Writer(tif->writer);
    }
}


/*********** READ + WRITE INTERFACE ****************************/

File_Bundle *Parse_Stack_Name(char *file_name)
{ static File_Bundle my_bundle;

  static char *Prefix = NULL;
  static int   Prefix_Max = 0;

  char *s, *t, c;

  s = file_name + strlen(file_name) - 4;
  if (strcmp(s,".tif") != 0 && strcmp(s,".TIF") != 0)
    error("1st file, %s, in stack does not have .tif extension",file_name);
  t = s;
  while (t > file_name && isdigit(t[-1]))
    t -= 1;
  if (s-t <= 0)
    error("No number sequence in stack file names %s",file_name);

  if (t-file_name > Prefix_Max)
    { Prefix_Max = (t-file_name)*1.2 + 20;
      Prefix     = (char *) Guarded_Realloc(Prefix,Prefix_Max+1,"Parse_Stack_Name");
    }

  c = *t;
  *t = '\0';
  strcpy(Prefix,file_name);
  *t = c;

  my_bundle.prefix    = Prefix;
  my_bundle.num_width = s-t;
  my_bundle.first_num = atoi(t);
  return (&my_bundle);
}

Image *Read_Image(char *file_name)
{ Tiff  *tif;
  Image *img;

  tif = Open_Tiff(file_name,"r");

  img = Read_Tiff(tif);

  Close_Tiff(tif);

  return (img);
}

int tz_determine_kind(Tio *tif)
{
  int kind;
  Tiff_Image   *img;
  img  = tif->image;
  if ((img->number_channels == 1) && 
      (img->channels[0]->bytes_per_pixel == 1)) {
    kind = GREY;
  } else {
    kind   = determine_kind(tif);
  }

  return kind;
}

int Determine_Kind(void *tif)
{
  return tz_determine_kind((Tio*) tif);
}

int Determine_Width(void *tif)
{
  return ((Tio*) tif)->image->width;
}

int Determine_Height(void *tif)
{
  return ((Tio*) tif)->image->height;
}

Stack *Read_Stack(char *file_name)
{ Stack *stack;

  Tio   *tif;
  Tiff_Type type;
  int    depth, width, height, kind, count;
  char  *text;

  { Tiff_Reader *reader;

    reader = Open_Tiff_Reader(file_name,NULL,0);
    depth  = 0;
    while ( ! End_Of_Tiff(reader) )
      { depth += 1;
        Advance_Tiff_Reader(reader);
      }
    Free_Tiff_Reader(reader);
  }

  tif = (Tio *) Open_Tiff(file_name,"r");


  /* tz+ for color image*/
  kind = tz_determine_kind(tif);
  /* tz++ */
  /* tz-
  kind   = determine_kind(tif);
  tz-- */

  width  = tif->image->width;
  height = tif->image->height;

  if ((text = (char *) Get_Tiff_Tag(tif->ifd,TIFF_JF_TAGGER,&type,&count)) == NULL)
    { text  = Empty_String;
      count = 0;
    }

  stack = new_stack(depth*height*width*kind,count+1,"Read_Stack");

  stack->width  = width;
  stack->height = height;
  stack->depth  = depth;
  stack->kind   = kind;
  strncpy(stack->text,text,count);
  stack->text[count] = '\0';

  { int d;

    d = 0;
    while (1)
      { read_directory(tif,Select_Plane(stack,d),"Read_Stack");

        d += 1;
        Advance_Tiff(tif);
        if (tif->eof) break;

        width  = tif->image->width;
        height = tif->image->height;
        if (width != stack->width || height != stack->height)
          error("Images of stack are not of the same dimensions!",NULL);

	/* tz+ for color image*/
	kind = tz_determine_kind(tif);
	/* tz++ */
	/* tz-
	   kind   = determine_kind(tif);
	 tz-- */

        if (kind != stack->kind)
          error("Images of stack are not of the same type (GREY, GREY16, or COLOR)!",NULL);
      }
  }

  Close_Tiff((Tiff *) tif);

  return (stack);
}

Stack *Read_Stack_Planes(File_Bundle *bundle)
{ Stack *stack;

  char  sname[1000];
  Tiff_Type type;
  int   width, height, depth, kind, count;
  Tio  *tif;
  char *text;

  depth = 0;
  while (1) 
    { FILE *fd;
  
      sprintf(sname,"%s%0*d.tif",bundle->prefix,bundle->num_width,bundle->first_num+depth);
      if ((fd = fopen(sname,"r")) == NULL)
        break;
      fclose(fd);
  
      depth += 1;
    }

  sprintf(sname,"%s%0*d.tif",bundle->prefix,bundle->num_width,bundle->first_num);

  tif = (Tio *) Open_Tiff(sname,"r");

  kind   = determine_kind(tif);
  width  = tif->image->width;
  height = tif->image->height;

  if ((text = (char *) Get_Tiff_Tag(tif->ifd,TIFF_JF_TAGGER,&type,&count)) == NULL)
    { text  = Empty_String;
      count = 0;
    }

  stack = new_stack(depth*height*width*kind,strlen(text)+1,"Read_Stack_Planes");

  stack->width  = width;
  stack->height = height;
  stack->depth  = depth;
  stack->kind   = kind;
  strncpy(stack->text,text,count);
  stack->text[count] = '\0';

  { int d;

    d = 0;
    while (1)
      { read_directory(tif,Select_Plane(stack,d),"Read_Stack_Planes");
        Close_Tiff((Tiff *) tif);

        d += 1;
        if (d >= depth) break;

        sprintf(sname,"%s%0*d.tif",bundle->prefix,bundle->num_width,bundle->first_num+d);

        tif = (Tio *) Open_Tiff(sname,"r");

        width  = tif->image->width;
        height = tif->image->height;
        kind = determine_kind(tif);

        if (width != stack->width || height != stack->height)
          error("Images of stack are not of the same dimensions!",NULL);
        if (kind != stack->kind)
          error("Images of stack are not of the same type (GREY, GREY16, or COLOR)!",NULL);
      }
  }

  return (stack);
}

void Write_Image(char *file_name, Image *a_image)
{ Tiff *tif;

  tif = Open_Tiff(file_name,"w");
  Write_Tiff(tif,a_image);
  Close_Tiff(tif);
}

void Write_Stack(char *file_name, Stack *a_stack)
{ Tiff *tif;
  int   i;

  tif = Open_Tiff(file_name,"w");
  if (a_stack->text[0] != '\0')
    Set_Tiff_Tag(((Tio *) tif)->ifd,TIFF_JF_TAGGER,TIFF_BYTE,strlen(a_stack->text),a_stack->text);
  for (i = 0; i < a_stack->depth; i++)
    Write_Tiff(tif,Select_Plane(a_stack,i));
  Close_Tiff(tif);
}

void Write_Stack_Planes(File_Bundle *bundle, Stack *a_stack)
{ char  *name;
  Image *plane;
  int    n;

  name = (char *) Guarded_Malloc(strlen(bundle->prefix)+50,"Write_Stack_Planes");
  for (n = 0; n < a_stack->depth; n++)
    { sprintf(name,"%s.%0*d.tif",bundle->prefix,bundle->num_width,bundle->first_num+n);
      plane = Select_Plane(a_stack,n);
      if (n == 0)
        plane->text = a_stack->text;
      Write_Image(name,plane);
    }
  free(name);
}


/*********** MODIFY IMAGE/TEXT DESCRIPTIONS ****************************/

void Set_Image_Text(Image *image, char *text)
{ _Image *object = (_Image *) (((char *) image) - Image_Offset);
  int     len    = strlen(text)+1;

  image->text = (char*)Guarded_Realloc(image->text,len,"Set_Image_Text");
  strcpy(image->text,text);
  object->tsize = len;
}

void Append_To_Image_Text(Image *image, char *text)
{ _Image *object = (_Image *) (((char *) image) - Image_Offset);
  int     sen    = strlen(image->text);
  int     len    = sen + strlen(text)+1;

  image->text = (char*)Guarded_Realloc(image->text,len,"Append_To_Image_Text");
  strcpy(image->text+sen,text);
  object->tsize = len;
}

void Set_Stack_Text(Stack *stack, char *text)
{ _Stack *object = (_Stack *) (((char *) stack) - Stack_Offset);
  int     len    = strlen(text)+1;

  stack->text = (char*)Guarded_Realloc(stack->text,len,"Set_Stack_Text");
  strcpy(stack->text,text);
  object->tsize = len;
}

void Append_To_Stack_Text(Stack *stack, char *text)
{ _Stack *object = (_Stack *) (((char *) stack) - Stack_Offset);
  int     sen    = strlen(stack->text);
  int     len    = sen + strlen(text)+1;

  stack->text = (char*)Guarded_Realloc(stack->text,len,"Append_To_Stack_Text");
  strcpy(stack->text+sen,text);
  object->tsize = len;
}

/*********** MAKE (EMPTY) IMAGES AND STACKS ****************************/

Image *Make_Image(int kind, int width, int height)
{ Image *image;

  image = new_image(height*width*kind,1,"Make_Image");

  image->width   = width;
  image->height  = height;
  image->kind    = kind;
  image->text[0] = '\0';

  return (image);
}

Stack *Make_Stack(int kind, int width, int height, int depth)
{ Stack *stack;

  stack = new_stack((size_t)depth*(size_t)height*(size_t)width*(size_t)kind,1,"Make_Stack");

  stack->width   = width;
  stack->height  = height;
  stack->depth   = depth;
  stack->kind    = kind;
  stack->text[0] = '\0';

  return (stack);
}


/*********** COMPUTE RANGES AND SCALE IMAGES AND STACKS *********************/

//  Compute min and max values in 'array' of type 'kind' with 'length' elements

static Pixel_Range *compute_minmax(uint8 *array, int kind, int length, int channel)
{ static Pixel_Range My_Range;
  int    i;

  if (kind == FLOAT32)
    { float32 *array32 = (float32 *) array;
      float    x, min, max;

      min = max = array32[0];
      for (i = 0; i < length; i++)
        { x = array32[i];
          if (x < min)
            min = x;
          else if (x > max)
            max = x;
        }
      My_Range.maxval = max;
      My_Range.minval = min;
    }
  else
    { int x, min, max;

      if (kind == GREY16)
        { uint16 *array16 = (uint16 *) array;
          min = max = array16[0];
          for (i = 0; i < length; i++)
            { x = array16[i];
              if (x < min)
                min = x;
              else if (x > max)
                max = x;
            }
        }
      else
        { if (kind == COLOR)
            { length *= 3;
              if (channel > 2)
                kind = 1;
              else
                array += channel;
            }
          min = max = array[0];
          for (i = 0; i < length; i += kind)
            { x = array[i];
              if (x < min)
                min = x;
              else if (x > max)
                max = x;
            }
        }
      My_Range.maxval = max;
      My_Range.minval = min;
    }

  return (&My_Range);
}

Pixel_Range *Image_Range(Image *image, int channel)
{ static Pixel_Range My_Range;
  My_Range = *compute_minmax(image->array,image->kind,image->width*image->height,channel);
  return (&My_Range);
}

Pixel_Range *Stack_Range(Stack *stack, int channel)
{ static Pixel_Range My_Range;
  My_Range = *compute_minmax(stack->array,stack->kind,
                             stack->width*stack->height*stack->depth,channel);
  return (&My_Range);
}

//  Scale values in 'array' of type 'kind' with 'length' elements by factor*(x)+offset

static void scale_values(uint8 *array, int kind, int length, int channel,
                         double factor, double offset)
{ int    i;

  if (kind == FLOAT32)
    { float32 *array32 = (float32 *) array;

      for (i = 0; i < length; i++)
        array32[i] = factor * (array32[i] + offset);
    }
  else if (kind == GREY16)
    { uint16 *array16 = (uint16 *) array;
      for (i = 0; i < length; i++)
        array16[i] = (uint16) (factor * (array16[i] + offset));
    }
  else
    { if (kind == COLOR)
        { length *= 3;
          if (channel > 2)
            kind = 1;
          else
            array += channel;
        }
      for (i = 0; i < length; i += kind)
        array[i] = (uint8) (factor * (array[i] + offset));
    }
}

void Scale_Image(Image *image, int channel, double factor, double offset)
{ scale_values(image->array,image->kind,image->width*image->height,channel,factor,offset); }

void Scale_Stack(Stack *stack, int channel, double factor, double offset)
{ scale_values(stack->array,stack->kind,stack->width*stack->height*stack->depth,
               channel,factor,offset);
}

void Scale_Image_To_Range(Image *image, int channel, double min, double max)
{ Pixel_Range crn; 
  crn = *compute_minmax(image->array,image->kind,image->width*image->height,channel);
  if (crn.maxval == crn.minval)
    { fprintf(stderr,"Warning: image is monotone and so cannot be scaled!\n");
      return;
    }
  Scale_Image(image,channel,(max-min)/(crn.maxval-crn.minval),min-1.*crn.minval);
}

void Scale_Stack_To_Range(Stack *stack, int channel, double min, double max)
{ Pixel_Range crn; 
  crn = *compute_minmax(stack->array,stack->kind,stack->width*stack->height*stack->depth,channel);
  if (crn.maxval == crn.minval)
    { fprintf(stderr,"Warning: stack is monotone and so cannot be scaled!\n");
      return;
    }
  Scale_Stack(stack,channel,(max-min)/(crn.maxval-crn.minval),min-1.*crn.minval);
}


/*********** CONVERT IMAGES AND STACKS  *********************/

static void translate(int skind, uint8 *in8, int tkind, uint8 *out8, int length)
{ uint16  *in16, *out16;
  float32 *in32, *out32;
  int     i, x; 
  double  c, scale, maxval;

  if (skind == GREY16 || skind == FLOAT32)
    { maxval = compute_minmax(in8,skind,length,0)->maxval;
      if (tkind == GREY16 && maxval > 65535.)
        scale  = 65535. / maxval;
      else if ((tkind == GREY || tkind == COLOR) && maxval > 255.)
        scale  = 255. / maxval;
      else
        scale  = 1.;
    }

  if (tkind > skind)
    { in8  += length*skind;
      out8 += length*tkind;
    }
  in16  = (uint16  *) in8;
  out16 = (uint16  *) out8;
  in32  = (float32 *) in8;
  out32 = (float32 *) out8;
    
     
  if (tkind == COLOR)
    if (skind == GREY)
      for (i = length; i > 0; i--)   // G->C
        { x = *--in8;
          *--out8 = x;
          *--out8 = x;
          *--out8 = x;
        }
    else if (skind == GREY16)
      for (i = length; i > 0; i--)   // G16->C
        { x = (*--in16) * scale;
          *--out8 = x;
          *--out8 = x;
          *--out8 = x;
        }
    else
      for (i = length; i > 0; i--)   // F32->C
        { x = (*in32++) * scale;
          *out8++ = x;
          *out8++ = x;
          *out8++ = x;
        }

  else if (tkind == GREY16)
    if (skind == COLOR)
      for (i = length; i > 0; i--)   // C->G16
        { c  = .3 * (*in8++);
          c += .59 * (*in8++);
          c += .11 * (*in8++);
          *out16++ = (uint16) c;
        }
    else if (skind == GREY)
      for (i = length; i > 0; i--)   // G->G16
        { *--out16 = *--in8; }
    else
      for (i = length; i > 0; i--)   // F32->G16
        { *out16++ = scale * (*in32++); }

  else if (tkind == GREY)
    if (skind == COLOR)
      for (i = length; i > 0; i--)   // C->G
        { c  = .3 * (*in8++);
          c += .59 * (*in8++);
          c += .11 * (*in8++);
          *out8++ = (uint8) c;
        }
    else if (skind == GREY16)
      for (i = length; i > 0; i--)   // G16->G
        { *out8++ = (*in16++) * scale; }
    else
      for (i = length; i > 0; i--)   // F32->G
        { *out8++ = (*in32++) * scale; }

  else // tkind == FLOAT32
    if (skind == COLOR)
      for (i = length; i > 0; i--)   // C->F32
        { c  = .3 * (*--in8);
          c += .59 * (*--in8);
          c += .11 * (*--in8);
          *--out32 = c;
        }
    else if (skind == GREY16)
      for (i = length; i > 0; i--)   // G16->F32
        { *--out32 = *--in16; }
    else
      for (i = length; i > 0; i--)   // G->F32
        { *--out32 = *--in8; }
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
              image->array  = (uint8*)Guarded_Realloc(image->array,object->asize,"Translate_Image");
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

      xlate = new_image(kind*width*height,1,"Translate_Image");
      xlate->width   = width;
      xlate->height  = height;
      xlate->kind    = kind;
      xlate->text[0] = '\0';

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
              stack->array  = (uint8*)Guarded_Realloc(stack->array,object->vsize,"Translate_Stack");
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

      xlate = new_stack(kind*width*height*depth,1,"Translate_Stack");
      xlate->depth   = depth;
      xlate->width   = width;
      xlate->height  = height;
      xlate->kind    = kind;
      xlate->text[0] = '\0';

      translate(stack->kind,stack->array,kind,xlate->array,width*height*depth);

      return (xlate);
    }
}

//  Floor values less than floor to the value floor

static void floor_values(uint8 *array, int kind, int length, int channel, double floor)
{ int    i;
  double x;

  if (kind == FLOAT32)
    { float32 *array32 = (float32 *) array;

      for (i = 0; i < length; i++)
        { x = array32[i];
          if (x < floor)
            array32[i] = floor;
        }
    }
  else if (kind == GREY16)
    { uint16 *array16 = (uint16 *) array;
      uint16  floor16  = floor;
      for (i = 0; i < length; i++)
        { x = array16[i];
          if (x < floor)
            array16[i] = floor16;
        }
    }
  else
    { uint8 floor8  = floor;
      if (kind == COLOR)
        { length *= 3;
          if (channel > 2)
            kind = 1;
          else
            array += channel;
        }
      for (i = 0; i < length; i += kind)
        { x = array[i];
          if (x < floor)
            array[i] = floor8;
        }
    }
}

void Floor_Image(Image *image, int channel, double floor)
{ floor_values(image->array,image->kind,image->width*image->height,channel,floor); }

void Floor_Stack(Stack *stack, int channel, double floor)
{ floor_values(stack->array,stack->kind,stack->width*stack->height*stack->depth,
                  channel,floor);
}

//  Ceiling values greater than ceiling to the value ceiling

static void ceiling_values(uint8 *array, int kind, int length, int channel, double ceiling)
{ int    i;
  double x;

  if (kind == FLOAT32)
    { float32 *array32 = (float32 *) array;

      for (i = 0; i < length; i++)
        { x = array32[i];
          if (x > ceiling)
            array32[i] = ceiling;
        }
    }
  else if (kind == GREY16)
    { uint16 *array16 = (uint16 *) array;
      uint16  ceil16  = ceiling;
      for (i = 0; i < length; i++)
        { x = array16[i];
          if (x > ceiling)
            array16[i] = ceil16;
        }
    }
  else
    { uint8 ceil8  = ceiling;
      if (kind == COLOR)
        { length *= 3;
          if (channel > 2)
            kind = 1;
          else
            array += channel;
        }
      for (i = 0; i < length; i += kind)
        { x = array[i];
          if (x > ceiling)
            array[i] = ceil8;
        }
    }
}

void Ceiling_Image(Image *image, int channel, double ceiling)
{ ceiling_values(image->array,image->kind,image->width*image->height,channel,ceiling); }

void Ceiling_Stack(Stack *stack, int channel, double ceiling)
{ ceiling_values(stack->array,stack->kind,stack->width*stack->height*stack->depth,
                  channel,ceiling);
}

//  Threshold values less than cutoff to black, all others to white

static void threshold_values(uint8 *array, int kind, int length, int channel, double cutoff)
{ int    i;
  double x;

  if (kind == FLOAT32)
    { float32 *array32 = (float32 *) array;

      for (i = 0; i < length; i++)
        { x = array32[i];
          if (x < cutoff)
            array32[i] = 0.;
          else
            array32[i] = 1.;
        }
    }
  else if (kind == GREY16)
    { uint16 *array16 = (uint16 *) array;
      for (i = 0; i < length; i++)
        { x = array16[i];
          if (x < cutoff)
            array16[i] = 0;
          else
            array16[i] = 0xFFFF;
        }
    }
  else
    { if (kind == COLOR)
        { length *= 3;
          if (channel > 2)
            kind = 1;
          else
            array += channel;
        }
      for (i = 0; i < length; i += kind)
        { x = array[i];
          if (x < cutoff)
            array[i] = 0;
          else
            array[i] = 0xFF;
        }
    }
}

void Threshold_Image(Image *image, int channel, double cutoff)
{ threshold_values(image->array,image->kind,image->width*image->height,channel,cutoff); }

void Threshold_Stack(Stack *stack, int channel, double cutoff)
{ threshold_values(stack->array,stack->kind,stack->width*stack->height*stack->depth,
                  channel,cutoff);
}

void split_values(uint8 *s, int length, uint8 *r, uint8 *g, uint8 *b)
{ int i, p;

  p = 0;
  for (i = 0; i < length; i++)
    { r[i] = s[p++];
      g[i] = s[p++];
      b[i] = s[p++];
    }
}

void Split_Color_Image(Image *image, Image **red, Image **green, Image **blue)
{ if (image->kind != COLOR)
    error("Split_Color_Image requires a color stack as input",NULL);

  *red   = Make_Image(GREY,image->width,image->height);
  *green = Make_Image(GREY,image->width,image->height);
  *blue  = Make_Image(GREY,image->width,image->height);

  split_values(image->array,image->width*image->height,
               (*red)->array,(*green)->array,(*blue)->array);
}

void Split_Color_Stack(Stack *stack, Stack **red, Stack **green, Stack **blue)
{ if (stack->kind != COLOR)
    error("Split_Color_Stack requires a color stack as input",NULL);

  *red   = Make_Stack(GREY,stack->width,stack->height,stack->depth);
  *green = Make_Stack(GREY,stack->width,stack->height,stack->depth);
  *blue  = Make_Stack(GREY,stack->width,stack->height,stack->depth);

  split_values(stack->array,stack->width*stack->height*stack->depth,
               (*red)->array,(*green)->array,(*blue)->array);
}

void merge_values(uint8 *s, int length, int offset, uint8 *r)
{ int i, p;

  p = offset;
  if (r == NULL)
    for (i = 0; i < length; i++)
      { s[p] = 0;
        p += 3;
      }
  else
    for (i = 0; i < length; i++)
      { s[p] = r[i];
        p += 3;
      }
}

Image *Make_Color_Image(Image *red, Image *green, Image *blue)
{ Image *image;
  int    w, h;

  if (red != NULL)
    { w = red->width; h = red->height; }
  else if (green != NULL)
    { w = green->width; h = green->height; }
  else if (blue != NULL)
    { w = blue->width; h = blue->height; }
  else
    error("At least one of 3 images to Make_Color_Image must be non-NULL",NULL);

  if (red != NULL && red->kind != GREY)
    error("Red image is not GREY",NULL);
  if (green != NULL && green->kind != GREY)
    error("Green image is not GREY",NULL);
  if (blue != NULL && blue->kind != GREY)
    error("Blue image is not GREY",NULL);
  if (green != NULL && (green->width != w || green->height != h))
    error("Dimensions of green image does not match that of others",NULL);
  if (blue != NULL && (blue->width != w || blue->height != h))
    error("Dimensions of blue image does not match that of others",NULL);

  image = Make_Image(COLOR,w,h);

  merge_values(image->array,w*h,0,(red==NULL) ? NULL : red->array);
  merge_values(image->array,w*h,1,(green==NULL) ? NULL : green->array);
  merge_values(image->array,w*h,2,(blue==NULL) ? NULL : blue->array);

  return (image);
}

Stack *Make_Color_Stack(Stack *red, Stack *green, Stack *blue)
{ Stack *stack;
  int    w, h, d;

  if (red != NULL)
    { w = red->width; h = red->height; d = red->depth; }
  else if (green != NULL)
    { w = green->width; h = green->height; d = green->depth; }
  else if (blue != NULL)
    { w = blue->width; h = blue->height; d = blue->depth; }
  else
    error("At least one of 3 stacks to Make_Color_Stack must be non-NULL",NULL);

  if (red != NULL && red->kind != GREY)
    error("Red stack is not GREY",NULL);
  if (green != NULL && green->kind != GREY)
    error("Green stack is not GREY",NULL);
  if (blue != NULL && blue->kind != GREY)
    error("Blue stack is not GREY",NULL);
  if (green != NULL && (green->width != w || green->height != h || green->depth != d))
    error("Dimensions of green stack does not match that of others",NULL);
  if (blue != NULL && (blue->width != w || blue->height != h || blue->depth != d))
    error("Dimensions of blue stack does not match that of others",NULL);

  stack = Make_Stack(COLOR,w,h,d);

  merge_values(stack->array,w*h*d,0,(red==NULL) ? NULL : red->array);
  merge_values(stack->array,w*h*d,1,(green==NULL) ? NULL : green->array);
  merge_values(stack->array,w*h*d,2,(blue==NULL) ? NULL : blue->array);

  return (stack);
}
