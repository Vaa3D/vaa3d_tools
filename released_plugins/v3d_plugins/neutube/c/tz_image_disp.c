/* tz_image_disp.c
 *
 * 16-Nov-2007 Initial write: Ting Zhao
 */

#ifdef HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <Imlib.h>
#include <Imlib_private.h>
#include "tz_error.h"
#include "tz_image_lib.h"
#include "tz_image_disp.h"

INIT_EXCEPTION

/* image_to_imdata(): Turn an image to a format easier for displaying.
 */
static Image* image_to_imdata(Image *image, int *t)
{
  Image *rgb;
  
  rgb = Translate_Image(image, COLOR, 0);
  
  *t = 0;

  return rgb;
}

/* lim_load_image: load an image to an ImlibData.
 *
 * Note: This function is to replace the interface of loading an image file
 *       directly.
 */
ImlibImage* lim_load_image(ImlibData *id, Image *image)
{
  unsigned char *data;
  ImlibImage *im;

  int trans;

  data = NULL;

  trans = 0;

  Image *rgb = image_to_imdata(image, &trans);

  data = rgb->array;

  if (!data) {
    THROW(ERROR_POINTER_NULL);
  }

  im = (ImlibImage *) malloc(sizeof(ImlibImage));

  if (!im) {
    THROW(ERROR_POINTER_NULL);
  }

  im->alpha_data = NULL;
  if (trans) {
    im->shape_color.r = 255;
    im->shape_color.g = 0;
    im->shape_color.b = 255;
  } else {
    im->shape_color.r = -1;
    im->shape_color.g = -1;
    im->shape_color.b = -1;
  }

  im->border.left = 0;
  im->border.right = 0;
  im->border.top = 0;
  im->border.bottom = 0;
  im->cache = 1;
  im->rgb_data = data;
  im->rgb_width = image->width;
  im->rgb_height = image->height;
  im->pixmap = 0;
  im->shape_mask = 0;

  im->mod.gamma = id->mod.gamma;
  im->mod.brightness = id->mod.brightness;
  im->mod.contrast = id->mod.contrast;
  im->rmod.gamma = id->rmod.gamma;
  im->rmod.brightness = id->rmod.brightness;
  im->rmod.contrast = id->rmod.contrast;
  im->gmod.gamma = id->gmod.gamma;
  im->gmod.brightness = id->gmod.brightness;
  im->gmod.contrast = id->gmod.contrast;
  im->bmod.gamma = id->bmod.gamma;
  im->bmod.brightness = id->bmod.brightness;
  im->bmod.contrast = id->bmod.contrast;

  Unpack_Image(rgb);

  calc_map_tables(id, im);

  return im;
}

/* Display_Image(): Display an image
 * 
 * Args: image - image to display
 *
 * Return: void.
 */
void Display_Image(Image *image)
{
  Display *disp;
  ImlibData *id;
  XSetWindowAttributes attr;
  Window win;
  ImlibImage *im;
  Pixmap p,m;
  int w,h;

   /* Connect to the default Xserver */
  disp=XOpenDisplay(NULL);

  /* Immediately afterwards initialize Imlib */
  id=Imlib_init(disp);

  /* turn the image into Imlib image data */
  im = lim_load_image(id, image);

  printf("here\n");

  /* Suck the image's original width and height out of the Image structure */
  w=im->rgb_width;h=im->rgb_height;

   /* Create a Window to display in */
  win=XCreateWindow(disp,DefaultRootWindow(disp),0,0,w,h,0,id->x.depth,
		    InputOutput,id->x.visual,0,&attr);
  XSelectInput(disp,win,StructureNotifyMask);

  /* Render the original 24-bit Image data into a pixmap of size w * h */
  Imlib_render(id,im,w,h);
  /* Extract the Image and mask pixmaps from the Image */
  p=Imlib_move_image(id,im);
  /* The mask will be 0 if the image has no transparency */
  m=Imlib_move_mask(id,im);
  /* Put the Image pixmap in the background of the window */
  XSetWindowBackgroundPixmap(disp,win,p);
  /* If there was a mask to the image, set the Image's mask to it */
  if (m) XShapeCombineMask(disp,win,ShapeBounding,0,0,m,ShapeSet);
  /* Actually display the window */
  XMapWindow(disp,win);
  /* Synchronise with the Xserver */
  XSync(disp,False);  

  for(;;) {
    XEvent ev;
     
    /* Sit and wait for an event to happen */ 
    XNextEvent(disp,&ev);
    if (ev.type==ConfigureNotify) {
      w=ev.xconfigure.width;h=ev.xconfigure.height;
      /* Re-render the Image to the new size */ 
      Imlib_render(id,im,w,h);
      /* Free the previous pixmap used for the window - note ONLY the pixmap is */
      /* freed - the mask is marked automatically to be freed along with the */
      /* pixmap. There is no need to free it as well - in fact it is advised you do */
      /* not. You must use the Imlib free function because of Imlib's caching. Do */
      /* not use any other free functions. You can use this function for Pixmaps */
      /* not created by Imlib - and it will just go free them as expected. */
      Imlib_free_pixmap(id,p);
      p=Imlib_move_image(id,im);
      /* The mask will be 0 if the image has no transparency */
      m=Imlib_move_mask(id,im);
      /* Put the Image pixmap in the background of the window */
      XSetWindowBackgroundPixmap(disp,win,p);
      /* If there was a mask to the image, set the Image's mask to it */
      if (m) XShapeCombineMask(disp,win,ShapeBounding,0,0,m,ShapeSet);
      /* Clear the window to update the background change */
      XClearWindow(disp,win);
      /* Synchronise with the Xserver */
      XSync(disp,False);
    }
  }
}
#endif /* ifdef HAVE_X11 */
