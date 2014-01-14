#include "zobject3darray.h"

#include <math.h>
#include <fstream>
#include <iostream>
#include <map>
#include "tz_error.h"
#include "zstring.h"
#include "tz_stack_utils.h"
#include "c_stack.h"
#include "tz_darray.h"

using namespace std;

ZObject3dArray::ZObject3dArray() : m_width(1), m_height(1), m_depth(1)
{
}

ZObject3dArray::~ZObject3dArray()
{
  clearAll();
}

void ZObject3dArray::clearAll()
{
  for (size_t i = 0; i < size(); i++) {
    if ((*this)[i] != NULL) {
      delete (*this)[i];
    }
  }
  clear();
}

void ZObject3dArray::append(Object_3d_List *objList)
{
  Object_3d_List *head = objList;
  while (objList != NULL) {
    push_back(new ZObject3d(objList->data));
    objList = objList->next;
  }

  Object_3d_List_Unlinkall(&head);
}

void ZObject3dArray::append(const ZObject3d &obj)
{
  ZObject3d *newobj = new ZObject3d(obj.c_obj());
  push_back(newobj);
}

void ZObject3dArray::append(const ZObject3d &obj,
                            const std::vector<int> &labelArray)
{
  if (!labelArray.empty()) {
    map<int, ZObject3d*> objMap;

    TZ_ASSERT(obj.size() == labelArray.size(), "Unmatched size");

    for (size_t i = 0; i < labelArray.size(); i++) {
      if (objMap.count(labelArray[i]) > 0) {
        objMap[labelArray[i]]->append(obj.x(i), obj.y(i), obj.z(i));
      } else { //new object
        ZObject3d *newobj = new ZObject3d;
        newobj->append(obj.x(i), obj.y(i), obj.z(i));
        objMap[labelArray[i]] = newobj;
      }
    }

    for (map<int, ZObject3d*>::iterator iter = objMap.begin();
         iter != objMap.end(); ++iter) {
      push_back(iter->second);
    }
  } else {
    append(obj);
  }
}

void ZObject3dArray::labelStack(Stack *stack)
{
  for (size_t i = 0; i < size(); i++) {
    ZObject3d *obj = (*this)[i];
    obj->labelStack(stack, i + 1);
  }
}

void ZObject3dArray::getRange(int *corner)
{
  for (int k = 0; k < 6; ++k) {
    corner[k] = 0;
  }

  if (!empty()) {
    (*this)[0]->getRange(corner);
    int tmpCorner[6];
    for (size_t i = 0; i < size(); ++i) {
      (*this)[i]->getRange(tmpCorner);
      for (int k = 0; k < 3; ++k) {
        if (tmpCorner[k] < corner[k]) {
          corner[k] = tmpCorner[k];
        }
      }
      for (int k = 0; k < 3; ++k) {
        if (tmpCorner[k+3] > corner[k+3]) {
          corner[k+3] = tmpCorner[k+3];
        }
      }
    }
  }
}

Stack* ZObject3dArray::toStack()
{
  int corner[6];
  getRange(corner);

  int width = corner[3] - corner[0] + 1;
  int height = corner[4] - corner[1] + 1;
  int depth = corner[5] - corner[2] + 1;

  Stack *stack = NULL;

  if (size() < 255) {
    stack = C_Stack::make(GREY, width, height, depth);
  } else {
    stack = C_Stack::make(GREY16, width, height, depth);
  }
  C_Stack::setZero(stack);

  for (size_t i = 0; i < size(); ++i) {
    ZObject3d *obj = (*this)[i];
    obj->labelStack(stack, i+1, -corner[0], -corner[1], -corner[2]);
  }

  return stack;
}

void ZObject3dArray::setSourceSize(int width, int height, int depth)
{
  m_width = width;
  m_height = height;
  m_depth = depth;
}

void ZObject3dArray::readIndex(string filePath,
                               int width, int height, int depth,
                               int indexOffset)
{
  UNUSED_PARAMETER(depth);

  clearAll();

  FILE *fp = fopen(filePath.c_str(), "r");

  ZString str;
  ZObject3d obj;
  while (str.readLine(fp)) {
    //cout << str.c_str() << endl;
    if (str[0] == '{') {
      obj.clear();
    } else if (str[0] == '}') {
      cout << obj.size() << endl;
      append(obj);
    } else {
      vector<int> indexArray = str.toIntegerArray();
      for (size_t i = 0; i < indexArray.size(); i++) {
        int x, y, z;
        Stack_Util_Coord(indexArray[i] + indexOffset,
                         width, height, &x, &y, &z);
        obj.append(x, y, z);
      }
    }
  }

  fclose(fp);
}

void ZObject3dArray::writeIndex(string filePath,
                                int width, int height, int depth)
{
  ofstream stream(filePath.c_str());

  for (size_t i = 0; i < size(); i++) {
    vector<size_t> indexArray = (*this)[i]->toIndexArray<size_t>(width, height, depth);
    stream << "{" << endl;
    for (size_t j = 0; j < indexArray.size(); j++) {
      stream << indexArray[j] << endl;
    }
    stream << "}" << endl;
  }

  stream.close();
}

void ZObject3dArray::exportCsvFile(string filePath)
{
  FILE *fp = fopen(filePath.c_str(), "w");
  for (size_t i = 0; i < size(); i++) {
    Object_3d_Csv_Fprint((*this)[i]->c_obj(), fp);
  }

  fclose(fp);
}

double ZObject3dArray::radiusVariance()
{
  double var = 0.0;
  if (size() > 1) {
    std::vector<double> radius(size());
    for (size_t i = 0; i < size(); ++i) {
      radius[i] = (*this)[i]->getRadius();
    }

#ifdef _DEBUG2
    darray_print(&(radius[0]), radius.size());
#endif

    var = darray_var(&(radius[0]), radius.size());
  }

  return var;
}

double ZObject3dArray::angleShift()
{
  double angle = 0.0;

  if (size() > 1) {
    ZPoint vec = averageDirection();

#ifdef _DEBUG_
    cout << vec.toString() << endl;
#endif

    ZPoint startCenter = (*this)[0]->getCenter();

    for (size_t i = 1; i < size(); ++i) {
      ZPoint center = (*this)[i]->getCenter();
      center -= startCenter;
      center.normalize();

      angle += acos(vec.cosAngle(center));
    }

    angle /= size() - 1;
  }

  return angle;
}

ZPoint ZObject3dArray::averageDirection()
{
  ZPoint vec(0.0, 0.0, 0.0);

  if (size() > 1) {
    ZPoint startCenter = (*this)[0]->getCenter();

    for (size_t i = 1; i < size(); ++i) {
      ZPoint center = (*this)[1]->getCenter();
      center -= startCenter;
      center.normalize();
      vec += center;
    }

    vec /= size() - 1;
    vec.normalize();
  }

  return vec;
}
