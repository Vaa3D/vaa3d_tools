/**@file tz_rastergeom.h
 * @brief raster geometry
 * @author Ting Zhao
 * @date 10-Feb-2009
 */

#ifndef _TZ_RASTERGEOM_H_
#define _TZ_RASTERGEOM_H_

#include "tz_cdefs.h"

__BEGIN_DECLS

int Raster_Line_Map(int m, int n, int x);
void Raster_Ratio_Scale(int w1, int h1, int w2, int h2, int *nw2, int *nh2);

/* round(x / y) */
int Round_Div(int x, int y);

int Raster_Linear_Map(int x, int a, int m, int b, int n);

/* x0: current offset; cx: selected point; w1: canvas size; w2: screen size;
 * r: zooming ratio
 */
int Raster_Point_Zoom_Offset(int x0, int r0, int cx, int w1, int w2, int r);

__END_DECLS

#endif
