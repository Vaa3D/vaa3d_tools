#ifndef __SQB_TREES_H__
#define __SQB_TREES_H__


#include <SQB/Core/RegTree.h>
#include <SQB/Core/Utils.h>

#include <SQB/Core/Booster.h>

#include <SQB/Core/LineSearch.h>

#include <fstream>

#include <v3d_interface.h>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageDuplicator.h"
#include "itkTIFFImageIOFactory.h"
#include "itkPNGImageIOFactory.h"

#include "../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h"

#include "../libs/regression/sep_conv.h"
#include "../libs/regression/util.h"

int mockTrainAndTest(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);

#endif

