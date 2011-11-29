/* ct3d_func.h
 * This plugin will invoke ct3d program, which track and segment cells from more than two images
 * 2011-06-27 : by Hang Xiao & Axel Mosig
 */
 
#ifndef __CT3D_FUNC_H__
#define __CT3D_FUNC_H__

#include <v3d_interface.h>

int open_ct3d(V3DPluginCallback2 &callback, QWidget *parent);

#endif

