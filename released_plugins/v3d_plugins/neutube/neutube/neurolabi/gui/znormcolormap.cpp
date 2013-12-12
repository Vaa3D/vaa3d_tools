#include "znormcolormap.h"
#include "tz_color.h"
#include "tz_utilities.h"
#include "tz_math.h"

ZNormColorMap::ZNormColorMap()
{
}

QColor ZNormColorMap::mapColor(double v)
{
  Rgb_Color color;
  int maxColor = 63;
  int index = iround(v * maxColor);

  Set_Color_Jet(&color, index);

  return QColor(color.r, color.g, color.b);
}
