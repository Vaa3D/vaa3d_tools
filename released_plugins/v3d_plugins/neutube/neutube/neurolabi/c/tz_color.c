/* @file tz_color.c
 * @author Ting Zhao
 * @date 24-Sep-2008
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <utilities.h>
#include "tz_color.h"
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_math.h"

Rgb_Color* New_Rgb_Color()
{
  return (Rgb_Color *) Guarded_Malloc(sizeof(Rgb_Color), "New_Rgb_Color");
}

void Delete_Rgb_Color(Rgb_Color *color)
{
  free(color);
}

Rgb_Color* Make_Rgb_Color(uint8_t r, uint8_t g, uint8_t b)
{
  Rgb_Color *color = New_Rgb_Color();
  Set_Color(color, r, g, b);

  return color;
}

void Set_Color(Rgb_Color *color, uint8_t r, uint8_t g, uint8_t b)
{
  color->r = r;
  color->g = g;
  color->b = b;
}

/*
 * See http://alvyray.com/Papers/hsv2rgb.htm for more details.
 */
void Set_Color_Hsv(Rgb_Color *color, double h, double s, double v)
{
  /* Convert h, s, v to valid range */
  if (h < 0.0) {
    h = 0.0;
  } else if (h > 6.0) {
    h = 6.0;
  }

  if (s < 0.0) {
    s = 0.0;
  } else if (s > 1.0) {
    s = 1.0;
  }

  if (v < 0.0) {
    v = 0.0;
  } else if (v > 1.0) {
    v = 1.0;
  }

  int m, n, iv;
  double f;
  long int i;
  i = (int) floor(h);
  f = h - i;
  if (!(i & 1)) {
    f = 1.0 - f;
  }

  m = iround(v * (1.0 - s) * 255.0);
  n = iround(v *(1.0 - s * f) * 255.0);
  
  if (m > 255) {
    m = 255;
  }

  if (n > 255) {
    n = 255;
  }

  iv = iround(v * 255.0);

  if (iv > 255) {
    iv = 255;
  }
  
  switch (i) {
  case 6:
  case 0:
    Set_Color(color, iv, n, m);
    break;
  case 1:
    Set_Color(color, n, iv, m);
    break;
  case 2:
    Set_Color(color, m, iv, n);
    break;
  case 3: 
    Set_Color(color, m, n, iv);
    break;
  case 4:
    Set_Color(color, n, m, iv);
    break;
  case 5:
    Set_Color(color, iv, m, n);
    break;
  default:
    TZ_ERROR(ERROR_DATA_VALUE);
  }
}

void Set_Color_Jet(Rgb_Color *color, int index)
{
  index %= 64;
  color->r = Jet_Colormap[index * 3];
  color->g = Jet_Colormap[index * 3 + 1];
  color->b = Jet_Colormap[index * 3 + 2];
}

void Print_Rgb_Color(const Rgb_Color *color)
{
  printf("R: %u, G: %u, B: %u\n", color->r, color->g, color->b);
}

Tiff_Colormap* New_Tiff_Colormap()
{
  Tiff_Colormap *cmap = (Tiff_Colormap *) 
    Guarded_Malloc(sizeof(Tiff_Colormap), "New_Tiff_Colormap");
  cmap->length = 0;
  cmap->array = NULL;

  return cmap;
}

void Delete_Tiff_Colormap(Tiff_Colormap *cmap)
{
  free(cmap);
}

void Rgb_Color_To_Hsv(const Rgb_Color *color, double *h, double *s, double *v)
{
  double r = (double) color->r / 255.0;
  double g = (double) color->g / 255.0;
  double b = (double) color->b / 255.0;

  double min_color = MIN3(r, g, b);
  double max_color = MAX3(r, g, b);

  *v = max_color;
  *s = 0;
  *h = 0;

  double delta = max_color - min_color;
  if (delta > 0.0) {
    *s = delta / max_color;
    if (r == min_color) {
      *h = 3.0 - (g - b) / delta;
    } else if (g == min_color) {
      *h = 5.0 - (b - r) / delta;
    } else {
      *h = 1.0 - (r - g) / delta;
    }
  }
}

double Rgb_Color_Hsv(const Rgb_Color *color)
{
  double r = (double) color->r / 255.0;
  double g = (double) color->g / 255.0;
  double b = (double) color->b / 255.0;

  double min_color = MIN3(r, g, b);
  double max_color = MAX3(r, g, b);

  double h = 0;

  double delta = max_color - min_color;
  if (delta > 0.0) {
    if (r == min_color) {
      h = 3.0 - (g - b) / delta;
    } else if (g == min_color) {
      h = 5.0 - (b - r) / delta;
    } else {
      h = 1.0 - (r - g) / delta;
    }
  }

  return h;
}

double Rgb_Color_Hue_Diff(const Rgb_Color *color1, const Rgb_Color *color2)
{
  double h1, h2, s, v;

  Rgb_Color_To_Hsv(color1, &h1, &s, &v);
  Rgb_Color_To_Hsv(color2, &h2, &s, &v);

  return dmin2(fabs(h1 - h2), 6.0 + dmin2(h1, h2) - dmax2(h1, h2));
}
