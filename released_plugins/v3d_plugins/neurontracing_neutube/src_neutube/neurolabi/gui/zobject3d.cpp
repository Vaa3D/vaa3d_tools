#include "zobject3d.h"
#if _QT_GUI_USED_
#include <QtGui>
#endif

#include <string.h>
#include <fstream>
#include <algorithm>
#include "tz_stack_attribute.h"
#include "tz_error.h"
#include "tz_voxel_graphics.h"
#include "tz_tvoxel.h"
#include "tz_math.h"
#include "c_stack.h"
#include "zobject3darray.h"
//#include "zstack.hxx"

using namespace std;

ZObject3d::ZObject3d(Object_3d *obj) : m_conn(0)
{
  if (obj != NULL) {
    m_voxelArray.resize(obj->size * 3);
    for (size_t i = 0; i < obj->size; i++) {
      set(i, obj->voxels[i]);
    }
  }

#if _QT_GUI_USED_
  m_color.setRed(100);
  m_color.setGreen(200);
  m_color.setBlue(0);
  m_color.setAlpha(100);
#endif
}

ZObject3d::ZObject3d(const vector<size_t> &indexArray, int width, int height,
                     int dx, int dy, int dz) : m_conn(0)
{
  setSize(indexArray.size());

  size_t i = 0;
  for (vector<size_t>::const_iterator iter = indexArray.begin();
       iter != indexArray.end(); ++iter, ++i) {
    set(i, *iter, width, height, dx, dy, dz);
  }
}

ZObject3d::~ZObject3d()
{
  clear();
}

void ZObject3d::set(int index, int x, int y, int z)
{
  index *= 3;
  m_voxelArray[index] = x;
  m_voxelArray[index + 1] = y;
  m_voxelArray[index + 2] = z;
}

void ZObject3d::set(int index, Voxel_t voxel)
{
  index *= 3;
  m_voxelArray[index] = voxel[0];
  m_voxelArray[index + 1] = voxel[1];
  m_voxelArray[index + 2] = voxel[2];
}

void ZObject3d::set(int index, size_t voxelIndex, int width, int height,
                    int dx, int dy, int dz)
{
  int x, y, z;
  Stack_Util_Coord(voxelIndex, width, height, &x, &y, &z);
  set(index, x + dx, y + dy, z + dz);
}

void ZObject3d::append(int x, int y, int z)
{
  m_voxelArray.push_back(x);
  m_voxelArray.push_back(y);
  m_voxelArray.push_back(z);
}

Object_3d* ZObject3d::c_obj() const
{
  m_objWrapper.conn = m_conn;
  m_objWrapper.voxels = (Voxel_t *) (&(m_voxelArray[0]));
  m_objWrapper.size = size();

  return &m_objWrapper;
}

void ZObject3d::setLine(ZPoint start, ZPoint end)
{
  Voxel_t startVoxel;
  Voxel_t endVoxel;

  Set_Tvoxel(startVoxel,
             iround(start.x()), iround(start.y()), iround(start.z()));
  Set_Tvoxel(endVoxel,
             iround(end.x()), iround(end.y()), iround(end.z()));

  Object_3d *obj = Line_To_Object_3d(startVoxel, endVoxel);

  clear();
  for (size_t i = 0; i < OBJECT_3D_SIZE(obj); ++i) {
    append(OBJECT_3D_X(obj, i), OBJECT_3D_Y(obj, i), OBJECT_3D_Z(obj, i));
  }

  Kill_Object_3d(obj);
}

void ZObject3d::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZObject3d::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZObject3d::display(QPainter &painter, int z, Display_Style option) const
{  
  UNUSED_PARAMETER(option);
#if _QT_GUI_USED_
  QPen pen(m_color);
  painter.setPen(pen);
  Object_3d *obj= c_obj();
  std::vector<QPoint> pointArray;
  for (size_t i = 0; i < obj->size; i++) {
    if (((obj->voxels[i][2] == z) || (z < 0))) {
      pointArray.push_back(QPoint(obj->voxels[i][0], obj->voxels[i][1]));
      //pointArray[i].setX(obj->voxels[i][0]);
      //pointArray[i].setY(obj->voxels[i][1]);
      /*
      uchar *pixel = image->scanLine(obj->voxels[i][1])
                     + 4 * obj->voxels[i][0];

      pixel[RED] = m_color.red();
      pixel[GREEN] = m_color.green();
      pixel[BLUE] = m_color.blue();
      pixel[ALPHA] = m_color.alpha();
      */
    }
  }
  painter.drawPoints(&(pointArray[0]), pointArray.size());
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);
#endif
}

void ZObject3d::labelStack(Stack *stack, int label)
{
  Object_3d *obj = c_obj();

  Image_Array ima;
  ima.array = stack->array;

  switch (Stack_Kind(stack)) {
  case GREY:
    label = (label - 1) % 255 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ima.array8[Stack_Util_Offset(obj->voxels[i][0], obj->voxels[i][1],
                                   obj->voxels[i][2], stack->width,
                                   stack->height, stack->depth)] = label;
    }
    break;
  case GREY16:
    label = (label - 1) % 65535 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ima.array16[Stack_Util_Offset(obj->voxels[i][0], obj->voxels[i][1],
                                    obj->voxels[i][2], stack->width,
                                    stack->height, stack->depth)] = label;
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

void ZObject3d::labelStack(Stack *stack, int label, int dx, int dy, int dz)
{
  Object_3d *obj = c_obj();

  Image_Array ima;
  ima.array = stack->array;

  switch (Stack_Kind(stack)) {
  case GREY:
    label = (label - 1) % 255 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ssize_t index = Stack_Util_Offset(obj->voxels[i][0] + dx,
          obj->voxels[i][1] + dy,
          obj->voxels[i][2] + dz, stack->width,
          stack->height, stack->depth);
      if (index >= 0) {
        ima.array8[index] = label;
      }
    }
    break;
  case GREY16:
    label = (label - 1) % 65535 + 1;
    for (size_t i = 0; i < obj->size; i++) {
      ssize_t index = Stack_Util_Offset(obj->voxels[i][0] + dx,
          obj->voxels[i][1] + dy,
          obj->voxels[i][2] + dz, stack->width,
          stack->height, stack->depth);
      if (index >= 0) {
        ima.array16[index] = label;
      }
    }
    break;
  default:
    TZ_ERROR(ERROR_DATA_TYPE);
  }
}

ZPoint ZObject3d::computeCentroid(FMatrix *matrix)
{
  ZPoint center(0.0, 0.0, 0.0);
  double totalWeight = 0.0;

  vector<size_t> indexArray = toIndexArray<size_t>(matrix->dim[0], matrix->dim[1],
                                           matrix->dim[2]);
  for (size_t i = 0; i < size(); i++) {
    /*
    double weight = matrix->array[
        Stack_Util_Offset(x(i) - 1, y(i) - 1, z(i) - 1,
                          )];
                          */
    double weight = matrix->array[indexArray[i]];
    center += ZPoint(x(i) * weight, y(i) * weight, z(i) * weight);
    totalWeight += weight;
  }

  center /= totalWeight;

  return center;
}

void ZObject3d::translate(int dx, int dy, int dz)
{
  for (size_t i = 0; i < size(); i++) {
    set(i, x(i) + dx, y(i) + dy, z(i) + dz);
  }
}

void ZObject3d::exportSwcFile(string filePath)
{
  ofstream stream(filePath.c_str());

  for (size_t i = 0; i < size(); i++) {
    stream << i + 1 << " " << 2 << " " << x(i) << " " << y(i) << " " << z(i)
           << " " << 3.0 << " " << -1 << endl;
  }

  stream.close();
}

void ZObject3d::exportCsvFile(string filePath)
{
  FILE *fp = fopen(filePath.c_str(), "w");

  Object_3d_Csv_Fprint(c_obj(), fp);

  fclose(fp);
}

ZObject3d* ZObject3d::clone() const
{
  ZObject3d *obj = new ZObject3d();

  *obj = *this;

  return obj;
}

double ZObject3d::averageIntensity(const Stack *stack) const
{
  double mu = 0.0;

  for (size_t i = 0; i < size(); ++i) {
    mu += Get_Stack_Pixel(const_cast<Stack*>(stack), x(i), y(i), z(i), 0);
    //mu += stack->value(x(i), y(i), z(i));
  }

  mu /= size();

  return mu;
}

void ZObject3d::print()
{
  cout << "3d object (" << size() << " voxels):" << endl;
  for (size_t i = 0; i < size(); ++i) {
    cout << "  " << x(i) << " " << y(i) << " " << z(i) << endl;
  }
}

Stack* ZObject3d::toStack(int *offset)
{
  Object_3d *obj = c_obj();

  int corners[6];
  Object_3d_Range(obj, corners);

  if (offset != NULL) {
    memcpy(offset, corners, sizeof(int) * 3);
  }

  int width = corners[3] - corners[0] + 1;
  int height = corners[4] - corners[1] + 1;
  int depth = corners[5] - corners[2] + 1;

  Stack *stack = C_Stack::make(GREY, width, height, depth);
  C_Stack::setZero(stack);

  for (size_t i = 0; i < obj->size; ++i) {
    int x = obj->voxels[i][0] - corners[0];
    int y = obj->voxels[i][1] - corners[1];
    int z = obj->voxels[i][2] - corners[2];

    stack->array[Stack_Util_Offset(x, y, z, width, height, depth)] = 1;
  }

  return stack;
}

ZObject3dArray* ZObject3d::growLabel(const ZObject3d &seed, int growLevel)
{
  int offset[3];

  Stack *stack = toStack(offset);
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  vector<size_t> currentSeed;

  for (size_t i = 0; i < seed.size(); ++i) {
    int x = seed.x(i) - offset[0];
    int y = seed.y(i) - offset[1];
    int z = seed.z(i) - offset[2];

    ssize_t index = Stack_Util_Offset(x, y, z, width, height, depth);

    if (index >= 0) {
      currentSeed.push_back(index);
    }
  }

  ZObject3dArray *objArray = new ZObject3dArray;

  //While the current seed is not empty
  while (!currentSeed.empty()) {
    //Update the label
    ZObject3d *obj =
        new ZObject3d(currentSeed, width, height, offset[0], offset[1], offset[2]);

#ifdef _DEBUG_2
    Print_Object_3d(obj->c_obj());
#endif

    objArray->push_back(obj);

    if (growLevel >= 0) {
      if (objArray->size() > (size_t) growLevel) {
        break;
      }
    }

    C_Stack::setStackValue(stack, currentSeed, 0);

    //Update the seed
    currentSeed = C_Stack::getNeighborIndices(stack, currentSeed, 26, 1);
  }

  C_Stack::kill(stack);

  return objArray;
}

void ZObject3d::getRange(int *corner) const
{
  Object_3d_Range(c_obj(), corner);
}

ZPoint ZObject3d::getCenter() const
{
  ZPoint center;

  double pos[3];
  Object_3d_Centroid(c_obj(), pos);

  center.set(pos[0], pos[1], pos[2]);

  return center;
}

double ZObject3d::getRadius() const
{
  double radius = 0.0;

  if (size() > 1) {
    ZPoint center = getCenter();
    Object_3d *obj = c_obj();
    for (size_t i = 0; i < obj->size; ++i) {
      radius += center.distanceTo(obj->voxels[i][0], obj->voxels[i][1],
          obj->voxels[i][2]);
    }
    radius /= size();
  }

  return radius;
}

bool ZObject3d::isEmpty()
{
  return m_voxelArray.empty();
}

bool ZObject3d::loadStack(const Stack *stack, int threshold)
{
  if (stack == NULL) {
    return false;
  }

  if (C_Stack::kind(stack) != GREY) {
    return false;
  }

  clear();

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  size_t offset = 0;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        if (stack->array[offset] > threshold) {
          append(x, y, z);
        }
        ++offset;
      }
    }
  }

  if (isEmpty()) {
    return false;
  }

  return true;
}

void ZObject3d::duplicateAcrossZ(int depth)
{
  size_t originalSize = m_voxelArray.size();


  m_voxelArray.resize(m_voxelArray.size() * depth);

  for (int i = 1; i < depth; ++i) {
    copy(m_voxelArray.begin(), m_voxelArray.begin() + originalSize - 1,
         m_voxelArray.begin() + originalSize * i);
  }

  for (size_t i = 2; i < m_voxelArray.size(); i += 3) {
    m_voxelArray[i] = i / originalSize;
  }
}

ZINTERFACE_DEFINE_CLASS_NAME(ZObject3d)
