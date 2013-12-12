#include "zsinglechannelstack.h"
#include <string.h>
#include "tz_utilities.h"
#include "tz_image_io.h"
#include "tz_stack_lib.h"
#include "tz_int_histogram.h"
#include "tz_stack_stat.h"
#include "tz_stack_threshold.h"
#include "tz_stack_bwmorph.h"
#include "tz_fimage_lib.h"
#include "tz_xml_utils.h"
#include "tz_stack_relation.h"
#include "tz_stack_attribute.h"
#include "tz_stack_watershed.h"

ZSingleChannelStack::ZSingleChannelStack()
{
  init();
}

//ZSingleChannelStack::ZSingleChannelStack(
//    int kind, int width, int height, int depth, bool isVirtual)
//{
//  init();
//  if (!isVirtual) {
//    Stack *stack = Make_Stack(kind, width, height, depth);
//    Zero_Stack(stack);
//    setData(stack);
//  } else {
//    Stack *stack = new Stack;
//    stack->kind = kind;
//    stack->width = width;
//    stack->height = height;
//    stack->depth = depth;
//    stack->array = NULL;
//    setData(stack, C_Stack::cppDelete);
//  }
//}

//ZSingleChannelStack::ZSingleChannelStack(
//    Stack *stack, C_Stack::Stack_Decllocator *delloc)
//{
//  init();
//  setData(stack, delloc);
//}

//ZSingleChannelStack::ZSingleChannelStack(const ZSingleChannelStack &src)
//{
//  Stack *stack = Copy_Stack(src.data());
//  init();
//  setData(stack);
//}

ZSingleChannelStack::~ZSingleChannelStack()
{
  clean();
}


bool ZSingleChannelStack::isDeprecated(EComponent component) const
{
  switch (component) {
  case STACK:
    return m_stack == NULL;
  case STACK_PROJ:
    return m_proj == NULL;
  case STACK_STAT:
    return m_stat == NULL;
  }

  return false;
}

void ZSingleChannelStack::deprecateDependent(EComponent component)
{
  switch (component) {
  case STACK:
    deprecate(STACK_PROJ);
    deprecate(STACK_STAT);
    break;
  case STACK_PROJ:
    break;
  case STACK_STAT:
    break;
  }
}

void ZSingleChannelStack::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case STACK:
    if (m_delloc != NULL && m_stack != NULL) {
      m_delloc(m_stack);
    }
    m_stack = NULL;
    m_delloc = NULL;
    break;
  case STACK_PROJ:
    delete m_proj;
    m_proj = NULL;
    break;
  case STACK_STAT:
    delete m_stat;
    m_stat = NULL;
    break;
  }
}

/*
bool ZSingleChannelStack::releaseOwnership()
{
  if (m_isOwner) {
    m_isOwner = false;
    return true;
  } else {
    return false;
  }
}
*/

void ZSingleChannelStack::subMostCommonValue()
{
  if (!isVirtual()) {
    Pixel_Range* pr = Stack_Range(m_stack, 0);
    Stack_Sub_Common(m_stack, 0, (int)((pr->minval+pr->maxval)/2));
  }
}

size_t ZSingleChannelStack::voxelNumber()
{
  return (size_t)m_stack->depth * (size_t)m_stack->height * (size_t)m_stack->width;
}

size_t ZSingleChannelStack::dataByteCount()
{
  return voxelNumber()*(size_t)m_stack->kind;
}

ZStack_Stat* ZSingleChannelStack::getStat() const
{
  if (m_stack == NULL) {
    return NULL;
  }

  if (isVirtual()) {
    return NULL;
  }

  if (isDeprecated(STACK_STAT)) {
    m_stat = new ZStack_Stat;
    m_stat->update(m_stack);
  }

  return m_stat;
}

ZStack_Projection* ZSingleChannelStack::getProj()
{
  if (m_stack == NULL) {
    return NULL;
  }

  if (isVirtual()) {
    return NULL;
  }

  if (isDeprecated(STACK_PROJ)) {
    m_proj = new ZStack_Projection;
    m_proj->update(m_stack);
  }

  return m_proj;
}

double ZSingleChannelStack::min() const
{
  if (getStat() == NULL) {
    return 0.0;
  }

  return getStat()->m_min;

  /*
  if (m_stat == NULL) {
    m_stat = new ZStack_Stat();
  }

  m_stat->update(m_stack, m_stamp);
  return m_stat->m_min;
  */
}

double ZSingleChannelStack::max() const
{
  if (getStat() == NULL) {
    return 0.0;
  }

  return getStat()->m_max;

  /*
  if (m_stat == NULL) {
    m_stat = new ZStack_Stat();
  }

  m_stat->update(m_stack, m_stamp);
  return m_stat->m_max;
  */
}

double ZSingleChannelStack::value(int x, int y, int z) const
{
  //must be non-virtual
  // validness of x y z is checked by ZStack
  size_t stride_x = (size_t)m_stack->kind;
  size_t stride_y = (size_t)m_stack->kind * (size_t)m_stack->width;
  size_t stride_z = (size_t)m_stack->kind * (size_t)m_stack->width * (size_t)m_stack->height;
  if (kind() == GREY || kind() == COLOR) {
    return *(uint8*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  } else if (kind() == GREY16) {
    return *(uint16*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  } else if (kind() == FLOAT32) {
    return *(float32*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  } else if (kind() == FLOAT64) {
    return *(float64*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  }

  return 0;
}


double ZSingleChannelStack::value(size_t index) const
{
  if (kind() == GREY || kind() == COLOR) {
    return *((uint8*)(m_stack->array) + index);
  } else if (kind() == GREY16) {
    return *((uint16*)(m_stack->array) + index);
  } else if (kind() == FLOAT32) {
    return *((float32*)(m_stack->array) + index);
  } else if (kind() == FLOAT64) {
    return *((float64*)(m_stack->array) + index);
  }

  return 0;
}


void ZSingleChannelStack::setValue(int x, int y, int z, double v)
{
  // validness of x y z is checked by ZStack
  if (!isVirtual()) {
    size_t stride_x = (size_t)m_stack->kind;
    size_t stride_y = (size_t)m_stack->kind * (size_t)m_stack->width;
    size_t stride_z = (size_t)m_stack->kind * (size_t)m_stack->width * (size_t)m_stack->height;
    if (kind() == GREY) {
      *(uint8*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x) = v;
    } else if (kind() == GREY16) {
      *(uint16*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x) = v;
    } else if (kind() == FLOAT32) {
      *(float32*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x) = v;
    } else if (kind() == FLOAT64) {
      *(float64*)(m_stack->array + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x) = v;
    }
  }
}

void ZSingleChannelStack::setValue(size_t index, double value)
{
  if (!isVirtual()) {
    if (kind() == GREY || kind() == COLOR) {
      *((uint8*)(m_stack->array) + index) = value;
    } else if (kind() == GREY16) {
      *((uint16*)(m_stack->array) + index) = value;
    } else if (kind() == FLOAT32) {
      *((float32*)(m_stack->array) + index) = value;
    } else if (kind() == FLOAT64) {
      *((float64*)(m_stack->array) + index) = value;
    }
  }
}

int ZSingleChannelStack::maxIntensityDepth(int x, int y) const
{
  int max_z = 0;

  if (!isVirtual()) {
    int z  = 0;
    double max_value = value(x, y, z);
    for (z = 1; z < depth(); z++) {
      double cvalue = value(x, y, z);
      if (cvalue > max_value) {
        max_value = cvalue;
        max_z = z;
      }
    }
  }

  return max_z;
}

void ZSingleChannelStack::shiftLocation(int *offset, int width, int height, int depth)
{
  if (width == -1)
    width = m_stack->width;
  if (height == -1)
    height = m_stack->height;
  if (depth == -1)
    depth = m_stack->depth;

  Stack *stack = Make_Stack(m_stack->kind, width, height, depth);
  int left = (-1) * offset[0];
  int top = (-1) * offset[1];
  int up = (-1) * offset[2];
  Crop_Stack(m_stack, left, top, up, width, height, depth, stack);

  copyData(stack);
  C_Stack::kill(stack);
}

void ZSingleChannelStack::clean()
{
  deprecate(STACK);
}

void ZSingleChannelStack::setData(Stack *stack,
                                  C_Stack::Stack_Deallocator *delloc)
{
  deprecate(STACK);
  m_stack = stack;
  m_delloc = delloc;
}

void *ZSingleChannelStack::projection(
    ZSingleChannelStack::Proj_Mode mode, ZSingleChannelStack::Stack_Axis axis)
{
  UNUSED_PARAMETER(mode);
  UNUSED_PARAMETER(axis);

  if (isVirtual()) {
    return NULL;
  }

  return getProj()->data();
}

void ZSingleChannelStack::bcAdjustHint(double *scale, double *offset)
{
  /*
  if (m_stat == NULL) {
    m_stat = new ZStack_Stat();
  }

  m_stat->update(m_stack, m_stamp);
  */

  ZStack_Stat *stat = getStat();

  if (stat != NULL) {
    *scale = stat->m_greyScale;
    *offset = stat->m_greyOffset;
  }
}

bool ZSingleChannelStack::isBinary()
{
  if (isVirtual()) {
    return false;
  }

  /*
  if (m_stat == NULL) {
    m_stat = new ZStack_Stat();
  }
  m_stat->update(m_stack, m_stamp);
  */

  ZStack_Stat *stat = getStat();

  return stat->hist()[0] < 3;
}

bool ZSingleChannelStack::binarize(int threshold)
{
  if (!isVirtual()) {
    Stack_Threshold_Binarize(m_stack, threshold);
    deprecateDependent(STACK);
    //m_stamp++;
    return true;
  }
  return false;
}

bool ZSingleChannelStack::bwsolid()
{
  if (isBinary()) {
    Stack *clean_stack = Stack_Majority_Filter_R(m_stack, NULL, 26, 4);
    Struct_Element *se = Make_Cuboid_Se(3, 3, 3);
    Stack *dilate_stack = Stack_Dilate(clean_stack, NULL, se);
    Kill_Stack(clean_stack);
    Stack *fill_stack = dilate_stack;
    Stack_Erode_Fast(fill_stack, m_stack, se);
    Kill_Stack(fill_stack);
    Kill_Struct_Element(se);
    //m_stamp++;
    deprecateDependent(STACK);
    return true;
  }

  return false;
}

bool ZSingleChannelStack::enhanceLine(
    double sigmaX, double sigmaY, double sigmaZ)
{
  if (!isVirtual()) {
    //double sigma[] = {0.5, 0.5, 1.0};
    double sigma[3];
    sigma[0] = sigmaX;
    sigma[1] = sigmaY;
    sigma[2] = sigmaZ;

    FMatrix *result = NULL;
    if (m_stack->width * m_stack->height * m_stack->depth > 1024 * 1024 * 100) {
      result = El_Stack_L_F(m_stack, sigma, NULL);
    } else {
      result = El_Stack_F(m_stack, sigma, NULL);
    }

    Stack *out = Scale_Float_Stack(result->array, result->dim[0], result->dim[1],
        result->dim[2], kind());


    Kill_FMatrix(result);
    copyData(out);
    C_Stack::kill(out);

    //m_stamp++;
    return true;
  }
  return false;
}

bool ZSingleChannelStack::enhanceLine()
{
  return enhanceLine(1.0, 1.0, 1.0);
}

bool ZSingleChannelStack::watershed()
{
  if (!isVirtual()) {
    Stack_Invert_Value(m_stack);
    Watershed_3D *shed = Build_3D_Watershed(m_stack, 0);
    Stack *out = Copy_Stack(shed->labels);
    Kill_Watershed_3D(shed);
    copyData(out);
    C_Stack::kill(out);
    //m_stamp++;
    return true;
  }
  return false;
}

void ZSingleChannelStack::init()
{
  m_stack = NULL;
  m_delloc = NULL;
  m_data.array = NULL;
  m_proj = NULL;
  m_stat = NULL;
  //m_isOwner = true;
}

void ZSingleChannelStack::copyData(const Stack *stack)
{
  Copy_Stack_Array(m_stack, stack);
}

void ZStack_Projection::update(Stack *stack)
{
  if (m_proj != NULL) {
    Kill_Image(m_proj);
    m_proj = NULL;
  }

  if (stack->array != NULL) {
    m_proj = Proj_Stack_Zmax(stack);
  }
}
/*
void ZStack_Projection::update(Stack *stack, int stamp)
{
  if (m_parent != stack) {
    m_parent = stack;
    if (m_proj != NULL) {
      Kill_Image(m_proj);
      m_proj = NULL;
    }
    m_stamp = stamp + 1;
  }
  if (m_stamp != stamp) {
    if (stack->array != NULL) {
      m_proj = Proj_Stack_Zmax(stack);
    }
    m_stamp = stamp;
  }
}
*/
void ZStack_Stat::update(Stack *stack)
{
  if (m_hist != NULL) {
    free(m_hist);
    m_hist = NULL;
  }
  m_min = 0.0;
  m_max = 0.0;
  m_greyScale = 1.0;
  m_greyOffset = 0.0;

  if (stack->array != NULL) {
    if (stack->kind != COLOR) {
      double smin, smax;
      if ((stack->kind != FLOAT32) && (stack->kind != FLOAT64)) {
        m_hist = Stack_Hist(stack);
        m_min = Int_Histogram_Min(m_hist);
        m_max = Int_Histogram_Max(m_hist);
        if (m_max > 1) {
          smin = Int_Histogram_Quantile(m_hist, 0.0035);
          smax = Int_Histogram_Quantile(m_hist, 0.9985);
        } else {
          smin = 0;
          smax = 1;
        }
      } else {
        m_min = Stack_Min(stack, NULL);
        m_max = Stack_Max(stack, NULL);
        smin = m_min;
        smax = m_max;
      }
      if (stack->kind != GREY) {
        if (smax != smin)
          m_greyScale = 255.0 / (smax - smin);
        m_greyOffset = -m_greyScale * smin;
      }
    }
  }
}

bool ZSingleChannelStack::hasSameValue(size_t index1, size_t index2)
{
  return value(index1) == value(index2);
}
