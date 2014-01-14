#include <cmath>
#include "plotsettings.h"

PlotSettings::PlotSettings()
{
  minX = 0.0;
  maxX = 11.0;
  numXTicks = 5;

  minY = 0.0;
  maxY = 1.0;
  numYTicks = 5;
}

void PlotSettings::scroll(int dx, int dy)
{
  double stepX = spanX() / numXTicks;
  minX += dx * stepX;
  maxY += dx * stepX;

  double stepY = spanY() / numYTicks;
  minY += dy * stepY;
  maxY += dy * stepY;
}

void PlotSettings::adjust()
{
  adjustAxis(minX, maxX, numXTicks);
  adjustAxis(minY, maxY, numYTicks);
}

void PlotSettings::adjustAxis(double &min, double &max, int &numTicks)
{
  const int MinTicks = 4;
  double grossStep = (max - min) / MinTicks;

  /* set step values to 10^n, 2*10^n or 5*10^n */
  double step = std::pow(10.0, std::floor(std::log10(grossStep)));

  if (5 * step < grossStep) {
    step *= 5;
  } else if (2 * step < grossStep) {
    step *= 2;
  }

  /* recalculate number of ticks */
  numTicks = int(std::ceil(max / step) - std::floor(min / step));
  if (numTicks < MinTicks) {
    numTicks = MinTicks;
  }

  /* recalculate min and max */
  min = std::floor(min / step) * step;
  max = std::ceil(max / step) * step;
}



