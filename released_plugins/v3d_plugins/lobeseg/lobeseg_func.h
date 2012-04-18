/* lobeseg_func.h
 * This plugin sperate the two optic lobes (OLs) and the center brain (CB) of fluit fly brain. Or seperate just one lobe and the center brain with suitable parameters.
 * June 20, 2011 : by Hanchuan Peng and Hang Xiao
 */

#ifndef __LOBESEG_FUNC_H__
#define __LOBESEG_FUNC_H__

#include <v3d_interface.h>

int lobeseg_two_sides(V3DPluginCallback2 &callback, QWidget *parent);
int lobeseg_one_side_only(V3DPluginCallback2 &callback, QWidget *parent);
bool lobeseg(const V3DPluginArgList & input, V3DPluginArgList & output);
void printHelp();
#endif

