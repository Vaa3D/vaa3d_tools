#include "zstackprojector.h"
#include "biocytin.h"
#include "zstack.hxx"
#include "tz_stack_lib.h"
#include "tz_stack.h"
#include "tz_fimage_lib.h"
#include "zstackstatistics.h"
#include "tz_color.h"
#include "tz_utilities.h"
#include "tz_math.h"
#include "neutubeconfig.h"
#include "tz_stack_lib.h"
#include "zstring.h"
#include "biocytin/zbiocytinfilenameparser.h"

Biocytin::ZStackProjector::ZStackProjector() :
  m_adjustingConstrast(true), m_smoothingDepth(true), m_usingExisted(false)
{

}

double Biocytin::ZStackProjector::colorToValueH(
    double sr, double sg, double sb, double /*reg*/)
{
  //return sg;

  Rgb_Color color;
  int ir = iround(sr);
  int ig = iround(sg);
  int ib = iround(sb);

  if (ir < 0) {
    color.r = 0;
  } else if (ir > 255) {
    color.r = 255;
  } else {
    color.r = ir;
  }

  if (ig < 0) {
    color.g = 0;
  } else if (ig > 255) {
    color.g = 255;
  } else {
    color.g = ig;
  }

  if (ib < 0) {
    color.b = 0;
  } else if (ib > 255) {
    color.b = 255;
  } else {
    color.b = ib;
  }
/*
  double h, s, v;
  Rgb_Color_To_Hsv(&color, &h, &s, &v);
  */

  double r = color.r / 255.0;
  double g = color.g / 255.0;
  double b = color.b / 255.0;

  double max_color = MAX2(r, g);
  double min_color = b;

  double delta = max_color - min_color;
  double h = 0.0;
  if (delta > 0.0) {
    h = 1.0 - (r - g) / delta;
  }

  h = dmin2(h, 6.0 - h);
  if (h < 0.0) {
    h = 0.0;
  }
  return dmin2(sr, sg) * h * h;
}

ZStack* Biocytin::ZStackProjector::project(
    const ZStack *stack, bool includingDepth)
{
#ifdef _DEBUG_
  tic();
#endif

  if (m_usingExisted) {
    std::string resultPath = getDefaultResultFilePath(stack->sourcePath());
    if (fexist(resultPath.c_str())) {
      ZStackFile stackFile;
      stackFile.import(resultPath);
      ZStack *proj = stackFile.readStack();
      return proj;
    }
  }

  ZStack *proj = NULL;
  //int width = stack->width();
  //int height = stack->height();
  int depth = stack->depth();

  startProgress();

  if (stack != NULL) {
    if (m_speedLevel == 3) {
      proj = new ZStack(stack->kind(), stack->width(), stack->height(), 1,
                        stack->channelNumber());
      for (int channel = 0; channel  < stack->channelNumber(); ++channel) {
        Image *projBuffer = Proj_Stack_Zmin(stack->c_stack(channel));
        proj->copyValue(projBuffer->array,
                         proj->getByteNumber(ZStack::SINGLE_PLANE), channel);
        Kill_Image(projBuffer);
      }
    } else {
      if (stack->channelNumber() >= 2) {
        advanceProgress(0.05);

        //filter->ndim = 2;

        FMatrix *smoothedRed = NULL;
        FMatrix *smoothedGreen = NULL;
        FMatrix *smoothedBlue = NULL;

        FMatrix *smoothed[3] = { NULL, NULL, NULL };

        for (int ch = 0; ch  < stack->channelNumber(); ++ch) {
          Stack *tmpStack = C_Stack::clone(stack->c_stack(ch));
          switch (m_speedLevel) {
          case 0:
            smoothed[ch] = smoothStackGaussian(tmpStack);
            break;
          case 1:
            smoothed[ch] = smoothStack(tmpStack);
            break;
          default:
            smoothed[ch] = smoothStackNull(tmpStack);
          }

          C_Stack::kill(tmpStack);
        }

        smoothedRed = smoothed[0];
        smoothedGreen = smoothed[1];
        smoothedBlue = smoothed[2];

        dim_type dim[2];
        dim[0] = smoothedRed->dim[0];
        dim[1] = smoothedRed->dim[1];
        FMatrix *projMat = Make_FMatrix(dim, 2);

        int pwidth = dim[0];
        int pheight = dim[1];

        const double regularizer = 0.1;

        m_depthArray.resize(pwidth * pheight);

        //Construct first slice
        size_t index = 0;
        for (int y = 0; y < pheight; ++y) {
          for (int x = 0; x < pwidth; ++x) {
            double red = smoothedRed->array[index];
            double green = smoothedGreen->array[index];
            double blue = 0.0;
            if (smoothedBlue != NULL) {
              blue = smoothedBlue->array[index];
            }
            projMat->array[index] = colorToValueH(
                  red, green, blue, regularizer);
            m_depthArray[index] = 0;
            ++index;
          }
        }

        advanceProgress(0.1);

        for (int z = 1; z < depth; ++z) {
          size_t projIndex = 0;
          for (int y = 0; y < pheight; ++y) {
            for (int x = 0; x < pwidth; ++x) {
              double red = smoothedRed->array[index];
              double green = smoothedGreen->array[index];
              double blue = 0.0;
              if (smoothedBlue != NULL) {
                blue = smoothedBlue->array[index];
              }

              double v = colorToValueH(
                    red, green, blue, regularizer);
              if (projMat->array[projIndex] > v) {
                projMat->array[projIndex] = v;
                m_depthArray[projIndex] = z;
              }
              ++index;
              ++projIndex;
            }
          }
        }

        advanceProgress(0.1);

#ifdef _DEBUG_2
        Stack *tmp = Scale_Float_Stack(smoothedGreen->array, width, height, depth,
                                       GREY);
        C_Stack::write(NeutubeConfig::getInstance().getPath(NeutubeConfig::DATA)
                       + "/test.tif", tmp);
#endif

        //Turn projMat into 8-bit proj
        Stack *projData = Scale_Float_Stack(
              projMat->array, projMat->dim[0], projMat->dim[1], 1, GREY16);
        advanceProgress(0.1);

        if (m_speedLevel > 1) {
          Kill_FMatrix(projMat);
          projMat = smoothStack(projData);
          C_Stack::kill(projData);
          projData = Scale_Float_Stack(
                projMat->array, projMat->dim[0], projMat->dim[1], 1, GREY16);
        }

        Stack *depthImage = NULL;
        if (includingDepth) {
          depthImage = C_Stack::make(GREY16, projMat->dim[0], projMat->dim[1], 1);
          uint16_t *array = (uint16_t*) depthImage->array;
          size_t index = 0;
          for (int y = 0; y < pwidth; ++y) {
            for (int x = 0; x < pheight; ++x) {
              array[index] = m_depthArray[index];
              ++index;
            }
          }
        }

        if (projMat->dim[0] > stack->width() ||
            projMat->dim[1] > stack->height()) {
          Stack *proj2 = Crop_Stack(
                projData, (projMat->dim[0] - stack->width()) / 2,
              (projMat->dim[1] - stack->height()) / 2, 0,
              stack->width(), stack->height(), 1, NULL);
          C_Stack::kill(projData);
          projData = proj2;
          Stack *depth2 = Crop_Stack(
                depthImage, (projMat->dim[0] - stack->width()) / 2,
              (projMat->dim[1] - stack->height()) / 2, 0,
              stack->width(), stack->height(), 1, NULL);
          C_Stack::kill(depthImage);
          depthImage = depth2;
        }

        if (m_smoothingDepth) {
          Stack *tmpImage = C_Stack::make(depthImage->kind, depthImage->width,
                                          depthImage->height, depthImage->depth);
          Stack_Running_Median(depthImage, 0, tmpImage);
          Stack_Running_Median(tmpImage, 1, depthImage);
          C_Stack::kill(tmpImage);
        }

        proj = new ZStack();
        proj->load(projData, depthImage, NULL);

        C_Stack::kill(projData);
        C_Stack::kill(depthImage);

        for (int ch = 0; ch  < stack->channelNumber(); ++ch) {
          Kill_FMatrix(smoothed[ch]);
        }
      }
    }
  }

  endProgress();

#ifdef _DEBUG_
  ptoc();
#endif

  return proj;
}

FMatrix* Biocytin::ZStackProjector::smoothStackNull(Stack *stack)
{
  FMatrix *dm = Get_Float_Matrix3(stack);
  advanceProgress(0.2);

  return dm;
}

FMatrix* Biocytin::ZStackProjector::smoothStack(Stack *stack)
{
  if (m_adjustingConstrast) {
    Stretch_Stack_Value_Q(stack, 1.0);
  }
  advanceProgress(0.05);

  FMatrix *smoothed = Smooth_Stack_Fast_F(stack, 5, 5, 1, NULL);
  advanceProgress(0.15);

  return smoothed;
}

FMatrix* Biocytin::ZStackProjector::smoothStackGaussian(Stack *stack)
{
  if (m_adjustingConstrast) {
    Stretch_Stack_Value_Q(stack, 1.0);
  }
  advanceProgress(0.05);

  FMatrix *smoothed;

  double sigma[3] = {3, 3, 0};
  FMatrix *filter = Gaussian_3D_Filter_2x_F(sigma, NULL);

  if (filter->dim[2] == 1) {
    smoothed = Filter_Stack_Slice_F(stack, filter, NULL);
  } else {
    smoothed = Filter_Stack_Fast_F(stack, filter, NULL, 0);
  }
  advanceProgress(0.15);

  Correct_Filter_Stack_F(filter, smoothed);
  advanceProgress(0.05);

  Kill_FMatrix(filter);

  return smoothed;
}

std::string Biocytin::ZStackProjector::getDefaultResultFilePath(
    const std::string &basePath)
{
  return ZString::removeFileExt(basePath) +
      ZBiocytinFileNameParser::getSuffix(ZBiocytinFileNameParser::PROJECTION) +
      ".tif";
}
