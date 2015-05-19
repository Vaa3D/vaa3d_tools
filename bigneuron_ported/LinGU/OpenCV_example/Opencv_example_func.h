/* Opencv_example_func.h
 * This plugin will load image and swc in domenu and dofunc
 * 2012-02-21 : by Hang Xiao
 */

#ifndef __Opencv_example_FUNC_H__
#define __Opencv_example_FUNC_H__

#include <v3d_interface.h>



#include <vector>
#include <iostream>

#include <v3d_interface.h>
#include "v3d_message.h"

#include "basic_surf_objs.h"
#include "stackutil.h"


#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/ml.hpp"
#include "opencv2/highgui.hpp"





#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <numeric>


int Opencv_example(V3DPluginCallback2 &callback, QWidget *parent);
bool Opencv_example(const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

