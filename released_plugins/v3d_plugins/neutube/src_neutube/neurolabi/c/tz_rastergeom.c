#include <stdio.h>
#include <stdlib.h>
#include "tz_error.h"
#include "tz_utilities.h"
#include "tz_rastergeom.h"

int Raster_Line_Map(int m, int n, int x)
{
  TZ_ASSERT(x >= 0, "Invalid x");

  if (m == n) {
    return x;
  } else {
    if (m > n) {
      int qt = m / n;
      int mod = m % n;

      if (mod == 0) {
	return x / qt;
      } else if (x < mod * (qt + 1)) {
	return x / (qt + 1);
      } else {
	return (x - mod) / qt;
      }
    } else {
      int mod = n % m;

      return (n / m) * x + MIN2(x, mod);
    }
  }
}

void Raster_Ratio_Scale(int w1, int h1, int w2, int h2, int *nw2, int *nh2)
{
  int f1 = w1 * h2;
  int f2 = w2 * h1;
  if (f1 >= f2) {
    *nw2 = w2;
    *nh2 = f2 / w1;
  } else {
    *nh2 = h2;
    *nw2 = f1 / h1;
  }
}

int Round_Div(int x, int y)
{
  TZ_ASSERT(y != 0, "zero denominator");

  if (x == 0) {
    return 0;
  }

  int q = x / y;
  if (abs((q * y - x) * 2) >= abs(y)) {
    q += (x * y >= 0) ? 1 : -1;
  }

  return q;
}

int Raster_Linear_Map(int x, int a, int m, int b, int n)
{
  if (n == 1) {
    return 0;
  }

  return Round_Div((x - a) * (n - 1), (m - 1)) + b;
}

int Raster_Point_Zoom_Offset(int x0, int r0, int cx, int w1, int w2, int r)
{
  //int x = x0 + Round_Div(cx * (w1 / r0 - w1 / r), w2 - 1);
  int x = x0 + Round_Div(cx * (w1 / r0 - 1), w2 - 1) - 
    Round_Div(cx * (w1 / r - 1), w2 - 1);

  if (x < 0) {
    x = 0;
  }

  return imin2(x, w1 - w1 / r);
}
