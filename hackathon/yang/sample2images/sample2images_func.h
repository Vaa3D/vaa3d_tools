/* sample2images_func.h
 * a plugin to sample 2 images
 * 6/13/2018 : by Yang Yu
 */
 
#ifndef __NEURONRECON_FUNC_H__
#define __NEURONRECON_FUNC_H__

//
#include <QtGlobal>
#include <vector>
#include "v3d_interface.h"
#include "v3d_message.h"
#include <iostream>
#include "basic_4dimage.h"
#include "algorithm"
#include <string.h>
#include <cmath>
#include <climits>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <string>
#include <tuple>
#include <stack>
#include <fstream>
#include <iostream>
#include <stdio.h>

#include "tiffio.h"

using namespace std;

// menus

// functions
bool sample2images_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback);

char *tiffread(char* filename, unsigned char *&p, uint32 &sz0, uint32  &sz1, uint32  &sz2, uint16 &datatype, uint16 &comp);
char *tiffwrite(char* filename, unsigned char *p, uint32 sz0, uint32  sz1, uint32  sz2, uint16 datatype, uint16 comp);

#endif

