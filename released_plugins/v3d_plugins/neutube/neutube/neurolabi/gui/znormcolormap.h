#ifndef ZNORMCOLORMAP_H
#define ZNORMCOLORMAP_H

#include <QColor>

class ZNormColorMap
{
public:
  ZNormColorMap();

public:
  QColor mapColor(double v);
};

#endif // ZNORMCOLORMAP_H
