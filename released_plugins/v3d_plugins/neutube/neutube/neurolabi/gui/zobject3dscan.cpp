#include "zobject3dscan.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <cmath>
#if _QT_GUI_USED_
#include <QtGui>
#endif

#include "zobject3d.h"
#include "tz_error.h"
#include "zgraph.h"
#include "tz_stack_objlabel.h"
#include "zerror.h"
#include "tz_constant.h"
#include "neutubeconfig.h"
#include "tz_stack_bwmorph.h"
#include "c_stack.h"
#include "tz_stack_stat.h"

using namespace std;

int ZObject3dStripe::getMinX() const
{
  if (isEmpty()) {
    return 0;
  }

  return m_segmentArray.front();
}

int ZObject3dStripe::getMaxX() const
{
  if (isEmpty()) {
    return 0;
  }

  return m_segmentArray.back();
}

void ZObject3dStripe::addSegment(int x1, int x2, bool canonizing)
{
   if (x1 > x2) {
     int tmp;
     SWAP2(x1, x2, tmp);
   }

   if (!m_segmentArray.empty()) {
     if (x1 > m_segmentArray.back() + 1) {
       m_segmentArray.push_back(x1);
       m_segmentArray.push_back(x2);
     } else {
       if (x1 >= m_segmentArray[m_segmentArray.size() - 2]) {
         m_segmentArray.back() = imax2(x2, m_segmentArray.back());
       } else {
         m_segmentArray.push_back(x1);
         m_segmentArray.push_back(x2);
         m_isCanonized = false;
       }
     }
   } else {
     m_segmentArray.push_back(x1);
     m_segmentArray.push_back(x2);
   }

   if (canonizing) {
     canonize();
   }
 }

int ZObject3dStripe::getVoxelNumber() const
{
  int voxelNumber = 0;
  size_t segmentNumber = m_segmentArray.size();
  for (size_t i = 0; i < segmentNumber; i += 2) {
    voxelNumber += m_segmentArray[i + 1] - m_segmentArray[i] + 1;
  }

  return voxelNumber;
}


const int* ZObject3dStripe::getSegment(size_t index) const
{
  if (index >= getSize()) {
    return NULL;
  }

  return &(m_segmentArray[index * 2]);
}

int ZObject3dStripe::getSegmentStart(size_t index) const
{
  return m_segmentArray[index * 2];
}

int ZObject3dStripe::getSegmentEnd(size_t index) const
{
  return m_segmentArray[index * 2 + 1];
}

void ZObject3dStripe::clearSegment()
{
  m_segmentArray.clear();
}

void ZObject3dStripe::translate(int dx, int dy, int dz)
{
  m_y += dy;
  m_z += dz;
  for (size_t i = 0; i < m_segmentArray.size(); ++i) {
    m_segmentArray[i] += dx;
  }
}

void ZObject3dStripe::addZ(int dz)
{
  m_z += dz;
}

bool ZObject3dStripe::isCanonizedActually()
{
  for (int i = 0; i < getSegmentNumber(); ++i) {
    if (getSegmentStart(i) > getSegmentEnd(i)) {
#ifdef _DEBUG_
      std::cout << "Segment start (" << getSegmentStart(i) << ") > end("
                << getSegmentEnd(i) << ")" << std::endl;
#endif
      return false;
    }

    if (i > 0) {
      if (getSegmentEnd(i - 1) >= getSegmentStart(i)) {
#ifdef _DEBUG_
        std::cout << "Previous segement is greater: " << getSegmentEnd(i - 1)
                  << " " << getSegmentEnd(i) << endl;
#endif
        return false;
      }
    }
  }

  return true;
}


void ZObject3dStripe::write(FILE *fp) const
{
  if (fp != NULL) {
    fwrite(&(m_z), sizeof(int), 1, fp);
    fwrite(&(m_y), sizeof(int), 1, fp);
    int nseg = getSegmentNumber();
    fwrite(&(nseg), sizeof(int), 1, fp);
    fwrite(&(m_segmentArray[0]), sizeof(int), m_segmentArray.size(), fp);
  }
}

void ZObject3dStripe::read(FILE *fp)
{
  if (fp != NULL) {
    fread(&(m_z), sizeof(int), 1, fp);
    fread(&(m_y), sizeof(int), 1, fp);
    int nseg = 0;
    fread(&(nseg), sizeof(int), 1, fp);
    TZ_ASSERT(nseg > 0, "Invalid segment number");
    m_segmentArray.resize(nseg * 2);
    fread(&(m_segmentArray[0]), sizeof(int), m_segmentArray.size(), fp);
    m_isCanonized = false;
  }
}

size_t ZObject3dStripe::countForegroundOverlap(
    Stack *stack, const int *offset) const
{
  if (stack == NULL) {
    return 0;
  }

  if (C_Stack::kind(stack) != GREY) {
    RECORD_ERROR(true, "Unsupported kind");
    return 0;
  }

  size_t count = 0;
  int y = getY();
  int z = getZ();

  if (offset != NULL) {
    y += offset[1];
    z += offset[2];
  }

  if (y < 0 || z < 0 || y >= C_Stack::height(stack) ||
      z >= C_Stack::depth(stack)) {
    return 0;
  }

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  size_t area = width * height;
  size_t arrayOffset = area * z + width * y;

  Image_Array ima;
  ima.array = stack->array;
  ima.array8 += arrayOffset;
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    int x0 = m_segmentArray[i];
    int x1 = m_segmentArray[i + 1];
    if (offset != NULL) {
      x0 += offset[0];
      x1 += offset[0];
    }
    if (x1 >= 0 && x0 < width) {
      if (x0 < 0) {
        x0 = 0;
      }
      if (x1 >= width) {
        x1 = width - 1;
      }

      for (int x = x0; x <= x1; ++x) {
        if (ima.array8[x] > 0) {
          ++count;
        }
      }
    }
  }

  return count;
}
void ZObject3dStripe::drawStack(Stack *stack, int v, const int *offset) const
{
  if (C_Stack::kind(stack) != GREY && C_Stack::kind(stack) != GREY16) {
    RECORD_ERROR(true, "Unsupported kind");
    return;
  }

  Image_Array ima;
  ima.array = stack->array;

  int y = getY();
  int z = getZ();

  if (offset != NULL) {
    y += offset[1];
    z += offset[2];
  }

  if (y >= C_Stack::height(stack)) {
    return;
  }

  if (z >= C_Stack::depth(stack)) {
    return;
  }

  size_t area = C_Stack::width(stack) * C_Stack::height(stack);
  size_t arrayOffset = area * z + C_Stack::width(stack) * y;

  switch (C_Stack::kind(stack)) {
  case GREY:
    ima.array8 += arrayOffset;
    v = (v < 0) ? 0 : ((v > 255) ? 255 : v);
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i + 1];
      if (offset != NULL) {
        x0 += offset[0];
        x1 += offset[0];
      }
      if (x0 < C_Stack::width(stack)) {
        x1 = min(x1, C_Stack::width(stack) - 1);
        for (int x = x0; x <= x1; ++x) {
          ima.array8[x] = v;
        }
      }
    }
    break;
  case GREY16:
    ima.array16 += arrayOffset;
    v = (v < 0) ? 0 : ((v > 65535) ? 65535 : v);
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i + 1];
      if (offset != NULL) {
        x0 += offset[0];
        x1 += offset[0];
      }
      if (x0 < C_Stack::width(stack)) {
        x1 = min(x1, C_Stack::width(stack) - 1);
        for (int x = x0; x <= x1; ++x) {
          ima.array16[x] = v;
        }
      }
    }
    break;
  default:
    break;
  }
}

void ZObject3dStripe::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue, const int *offset) const
{
  if (C_Stack::kind(stack) != COLOR) {
    RECORD_ERROR(true, "Unsupported kind");
    return;
  }

  Image_Array ima;
  ima.array = stack->array;

  int y = getY();
  int z = getZ();

  if (offset != NULL) {
    y += offset[1];
    z += offset[2];
  }

  if (y >= C_Stack::height(stack)) {
    RECORD_ERROR(true, "y too large");
    return;
  }

  if (z >= C_Stack::depth(stack)) {
    RECORD_ERROR(true, "z too large");
    return;
  }
  //TZ_ASSERT(y < C_Stack::height(stack), "y too large");
  //TZ_ASSERT(z < C_Stack::depth(stack), "z too large");

  size_t area = C_Stack::width(stack) * C_Stack::height(stack);
  size_t arrayOffset = area * z + C_Stack::width(stack) * y;

  ima.arrayc += arrayOffset;
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    int x0 = m_segmentArray[i];
    int x1 = m_segmentArray[i + 1];
    if (offset != NULL) {
      x0 += offset[0];
      x1 += offset[0];
    }
    TZ_ASSERT(x0 < C_Stack::width(stack), "x too large");
    TZ_ASSERT(x1 < C_Stack::width(stack), "x too large");
    for (int x = x0; x <= x1; ++x) {
      ima.arrayc[x][0] = red;
      ima.arrayc[x][1] = green;
      ima.arrayc[x][2] = blue;
    }
  }
}

void ZObject3dScan::labelStack(Stack *stack, int startLabel, const int *offset)
{
  std::vector<ZObject3dScan> objArray = getConnectedComponent();

#ifdef _DEBUG_
  std::cout << "Number of components: " << objArray.size() << std::endl;
#endif

  if (objArray.size() + startLabel > 256) {
    Translate_Stack(stack, GREY16, 1);
  }
  for (size_t i = 0; i < objArray.size(); ++i) {
    objArray[i].drawStack(stack, startLabel  + i, offset);
  }
}

static int ZObject3dSegmentCompare(const void *e1, const void *e2)
{
  int *v1 = (int*) e1;
  int *v2 = (int*) e2;

  if (v1[0] < v2[0]) {
    return -1;
  } else if (v1[0] > v2[0]) {
    return 1;
  } else {
    if (v1[1] < v2[1]) {
      return -1;
    } else if (v1[1] > v2[1]) {
      return 1;
    }
  }

  return 0;
}

void ZObject3dStripe::sort()
{
  if (!m_segmentArray.empty()) {
    qsort(&m_segmentArray[0], m_segmentArray.size() / 2, sizeof(int) * 2,
        ZObject3dSegmentCompare);
  }
}

void ZObject3dStripe::canonize()
{
  if (!m_isCanonized) {
    if (!m_segmentArray.empty()) {
      sort();
      vector<int> newSegmentArray;
      //newSegmentArray.reserve(m_segmentArray.size());
      for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
        if (newSegmentArray.empty()) {
          newSegmentArray.push_back(m_segmentArray[i]);
          newSegmentArray.push_back(m_segmentArray[i + 1]);
        } else {
          if (newSegmentArray.back() + 1 >= m_segmentArray[i]) {
            if (newSegmentArray.back() < m_segmentArray[i + 1]) {
              newSegmentArray.back() = m_segmentArray[i + 1];
            }
          } else {
            newSegmentArray.push_back(m_segmentArray[i]);
            newSegmentArray.push_back(m_segmentArray[i + 1]);
          }
        }
      }
      m_segmentArray.swap(newSegmentArray);
      //m_segmentArray = newSegmentArray;
    }

    m_isCanonized = true;
  }
}

bool ZObject3dStripe::unify(const ZObject3dStripe &stripe, bool canonizing)
{
  if (getY() == stripe.getY() && getZ() == stripe.getZ()) {
    if (isCanonized() && stripe.isCanonized()) {
      if (!isEmpty() && !stripe.isEmpty()) {
        if (m_segmentArray.back() + 1 >= stripe.m_segmentArray.front()) {
          m_isCanonized = false;
        }
      }
    } else {
      m_isCanonized = false;
    }

    m_segmentArray.insert(m_segmentArray.end(), stripe.m_segmentArray.begin(),
                          stripe.m_segmentArray.end());

    if (canonizing) {
      canonize();
    }

    return true;
  }

  return false;
}

bool ZObject3dStripe::equalsLiterally(const ZObject3dStripe &stripe) const
{
  if (getZ() != stripe.getZ()) {
    return false;
  }

  if (getY() != stripe.getY()) {
    return false;
  }

  if (getSegmentNumber() != stripe.getSegmentNumber()) {
    return false;
  }

  for (size_t i = 0; i < m_segmentArray.size(); ++i) {
    if (m_segmentArray[i] != stripe.m_segmentArray[i]) {
      return false;
    }
  }

  return true;
}

void ZObject3dStripe::dilate()
{
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    m_segmentArray[i] -= 1;
    m_segmentArray[i + 1] += 1;
  }
  setCanonized(false);
  canonize();
}

//Use the fact that x0 <= x0p*t <= x1p*t <= x1 and
// (x0p - 1)*t < x0, (x1p + 1)*t > x1
void ZObject3dStripe::downsample(int xintv)
{
  if (xintv > 0) {
    vector<int> newSegmentArray;
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i+1];
      int t = xintv + 1;
      int x0p = x0 / t;
      int x1p = x1 / t;
      if (x0p * t < x0) {
        ++x0p;
      }

      if (x1p * t > x1) {
        --x1p;
      }

      if (x0p <= x1p) {
        newSegmentArray.push_back(x0p);
        newSegmentArray.push_back(x1p);
      }
    }

    m_segmentArray = newSegmentArray;
    m_isCanonized = false;

    canonize();
  }
}

//Use the fact that: x0p*t <= x0 <= x1 <= x1t and
//  (x0p + 1)*t > x0, (x1p - 1)*t < x1
void ZObject3dStripe::downsampleMax(int xintv)
{
  if (xintv > 0) {
    vector<int> newSegmentArray;
    for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
      int x0 = m_segmentArray[i];
      int x1 = m_segmentArray[i+1];
      int t = xintv + 1;
      int x0p = x0 / t;
      int x1p = x1 / t;

      if (x1p % t > 0) {
        ++x1p;
      }

      newSegmentArray.push_back(x0p);
      newSegmentArray.push_back(x1p);
    }

    m_segmentArray = newSegmentArray;
    m_isCanonized = false;
    canonize();
  }
}

void ZObject3dStripe::print() const
{
  cout << "Segments at " << "." << m_z << "|" << m_y << ": " << endl;
  for (size_t i = 0; i < m_segmentArray.size(); i += 2) {
    cout << "  " << m_segmentArray[i] << " - " << m_segmentArray[i+1] << endl;
  }
}

ZObject3dScan::ZObject3dScan() : m_isCanonized(true)
{
}

ZObject3dScan::~ZObject3dScan()
{
  deprecate(ALL_COMPONENT);
}

bool ZObject3dScan::isDeprecated(EComponent comp) const
{
  switch (comp) {
  case STRIPE_INDEX_MAP:
    return m_stripeMap.empty();
  case INDEX_SEGMENT_MAP:
    return m_indexSegmentMap.empty();
  case ACCUMULATED_STRIPE_NUMBER:
    return m_accNumberArray.empty();
  default:
    break;
  }

  return false;
}

void ZObject3dScan::deprecate(EComponent comp)
{
  deprecateDependent(comp);

  switch (comp) {
  case STRIPE_INDEX_MAP:
    m_stripeMap.clear();
    break;
  case INDEX_SEGMENT_MAP:
    m_indexSegmentMap.clear();
    break;
  case ACCUMULATED_STRIPE_NUMBER:
    m_accNumberArray.clear();
    break;
  case ALL_COMPONENT:
    deprecate(STRIPE_INDEX_MAP);
    deprecate(INDEX_SEGMENT_MAP);
    deprecate(ACCUMULATED_STRIPE_NUMBER);
    break;
  }
}

void ZObject3dScan::deprecateDependent(EComponent comp)
{
  switch (comp) {
  default:
    break;
  }
}

bool ZObject3dScan::isEmpty() const
{
  return m_stripeArray.empty();
}

size_t ZObject3dScan::getStripeNumber() const
{
  return m_stripeArray.size();
}

size_t ZObject3dScan::getVoxelNumber() const
{
  size_t voxelNumber = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    voxelNumber += m_stripeArray[i].getVoxelNumber();
  }

  return voxelNumber;
}

size_t ZObject3dScan::getVoxelNumber(int z) const
{
  size_t voxelNumber = 0;
  size_t stripeNumber = getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    if (m_stripeArray[i].getZ() == z) {
      voxelNumber += m_stripeArray[i].getVoxelNumber();
    }
  }

  return voxelNumber;
}

std::vector<size_t> ZObject3dScan::getSlicewiseVoxelNumber() const
{
  std::vector<size_t> voxelNumber;
  size_t stripeNumber = getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    int z = m_stripeArray[i].getZ();
    if (z >= (int) voxelNumber.size()) {
      voxelNumber.resize(z + 1, 0);
    }
    voxelNumber[z] += m_stripeArray[i].getVoxelNumber();
  }

  return voxelNumber;
}

ZObject3dStripe ZObject3dScan::getStripe(size_t index) const
{
  return m_stripeArray[index];
}

void ZObject3dScan::addStripe(int z, int y, bool canonizing)
{
  bool adding = true;
  if (!m_stripeArray.empty()) {
    if (y == m_stripeArray.back().getY() &&
        z == m_stripeArray.back().getZ()) {
      adding = false;
    }
  }

  if (adding) {
    ZObject3dStripe stripe;
    stripe.setY(y);
    stripe.setZ(z);

    addStripe(stripe, canonizing);
  }
}

void ZObject3dScan::addStripe(const ZObject3dStripe &stripe, bool canonizing)
{
  bool lastStripeMergable = false;

  if (!m_stripeArray.empty()) {
    if (stripe.getY() != m_stripeArray.back().getY() ||
        stripe.getZ() != m_stripeArray.back().getZ()) {
      if (m_isCanonized) {
        if (stripe.getZ() < m_stripeArray.back().getZ()) {
          m_isCanonized = false;
        } else if (stripe.getZ() == m_stripeArray.back().getZ()) {
          if (stripe.getY() < m_stripeArray.back().getY()) {
            m_isCanonized = false;
          }
        }
      }
    } else {
      lastStripeMergable = true;
    }
  }

  if (lastStripeMergable) {
    for (int i = 0; i < stripe.getSegmentNumber(); ++i) {
      m_stripeArray.back().addSegment(
            stripe.getSegmentStart(i), stripe.getSegmentEnd(i), canonizing);
    }
  } else {
    m_stripeArray.push_back(stripe);
  }

  if (canonizing && !m_isCanonized) {
    canonize();
  }

  deprecate(ALL_COMPONENT);
}

void ZObject3dScan::addSegment(int x1, int x2, bool canonizing)
{
  if (!isEmpty()) {
    m_stripeArray.back().addSegment(x1, x2, canonizing);
    if (!m_stripeArray.back().isCanonized()) {
      m_isCanonized = false;
    }

    deprecate(INDEX_SEGMENT_MAP);
    deprecate(ACCUMULATED_STRIPE_NUMBER);

    if (canonizing) {
      canonize();
    }
  }
}

void ZObject3dScan::addSegment(int z, int y, int x1, int x2, bool canonizing)
{
  addStripe(z, y, false);
  addSegment(x1, x2, canonizing);
}

/*
const int* ZObject3dScan::getLastStripe() const
{
  if (isDeprecated(LAST_STRIPE)) {
    size_t numStripe = getStripeNumber();
    const int *stripe = getFirstStripe();

    for (size_t i = 0; i < numStripe - 1; i++) {
      int numScanLine = getStripeSize(stripe);
      const int *scanLine = getSegment(stripe);
      for (int j = 0; j < numScanLine; j++) {
        if (j < numScanLine - 1) {
          scanLine += 2;
        }
      }
      stripe = scanLine + 1;
    }

    m_lastStripe = const_cast<int*>(stripe);
  }

  return m_lastStripe;
}

int* ZObject3dScan::getLastStripe()
{
  return const_cast<int*>(static_cast<const ZObject3dScan&>(*this).getLastStripe());
}
*/
ZObject3d* ZObject3dScan::toObject3d() const
{
  if (isEmpty()) {
    return NULL;
  }

  ZObject3d *obj = new ZObject3d();

  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    int y = iter->getY();
    int z = iter->getZ();
    int nseg = iter->getSegmentNumber();
    for (int i = 0; i < nseg; ++i) {
      const int *seg = iter->getSegment(i);
      for (int x = seg[0]; x <= seg[1]; ++x) {
        obj->append(x, y, z);
      }
    }
  }

/*
  size_t numStripe = getStripeNumber();

  for (size_t i = 0; i < numStripe; i++) {
    int z = getY(stripe);
    int y = getZ(stripe);
    int numScanLine = getStripeSize(stripe);
    const int *scanLine = getSegment(stripe);
    for (int j = 0; j < numScanLine; j++) {
      int x1 = scanLine[0];
      int x2 = scanLine[1];
      for (int x = x1; x <= x2; x++) {
        obj->append(x, y, z);
      }
      if (j < numScanLine - 1) {
        scanLine += 2;
      }
    }
    stripe = scanLine + 1;
  }
*/
  return obj;
}

const std::map<size_t, std::pair<size_t, size_t> >&
ZObject3dScan::getIndexSegmentMap() const
{
  if (isDeprecated(INDEX_SEGMENT_MAP)) {
    m_indexSegmentMap.clear();
    size_t currentIndex = 0;
    for (size_t stripeIndex = 0; stripeIndex < getStripeNumber(); ++stripeIndex) {
      size_t segmentNumber = m_stripeArray[stripeIndex].getSegmentNumber();
      for (size_t segIndex = 0; segIndex < segmentNumber; ++segIndex) {
        m_indexSegmentMap[currentIndex++] = std::pair<size_t, size_t>(stripeIndex,
                                                                    segIndex);
      }
    }
  }

  return m_indexSegmentMap;
}

bool ZObject3dScan::getSegment(size_t index, int *z, int *y, int *x1, int *x2)
{
  const std::map<size_t, std::pair<size_t, size_t> >&segMap =
      getIndexSegmentMap();
  if (segMap.count(index) > 0) {
    std::pair<size_t, size_t> location = segMap.at(index);
    *z = m_stripeArray[location.first].getZ();
    *y = m_stripeArray[location.first].getY();
    *x1 = m_stripeArray[location.first].getSegmentStart(location.second);
    *x2 = m_stripeArray[location.first].getSegmentEnd(location.second);
    return true;
  }

  return false;
}

void ZObject3dScan::loadStack(const Stack *stack)
{
  if (stack == NULL) {
    RECORD_ERROR(true, "null stack");
    return;
  }

  if (C_Stack::kind(stack) != GREY) {
    RECORD_ERROR(true, "unsupported kind");
    return;
  }

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  uint8_t *array = stack->array;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      int x1 = -1;
      int x2 = -1;
      int scanState = 0; //0: no ON pixel found;
                         //1: in ON region; 2: in OFF region
      for (int x = 0; x < width; ++x) {
        if (*array > 0) {
          switch (scanState) {
          case 0:
            addStripe(z, y);
            x1 = x;
            break;
          case 1:
            break;
          case 2:
            x1 = x;
            break;
          }
          scanState = 1;
        } else {
          if (scanState == 1) {
            x2 = x - 1;
            scanState = 2;
          }
        }
        if (x1 >= 0 && x2 >= 0) {
          addSegment(x1, x2);
          x1 = -1;
          x2 = -1;
        }
        ++array;
      }
      if (x1 >= 0) {
        addSegment(x1, width - 1);
      }
    }
  }
}

void ZObject3dScan::print() const
{
  cout << getStripeNumber() << " stripes" << endl;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    cout << "  Z:" << iter->getZ() << " Y:" << iter->getY() << " #S:"
         << iter->getSegmentNumber() << " #V:" << iter->getVoxelNumber() << endl;
    iter->print();
  }
}

bool ZObject3dScan::load(const string &filePath)
{
  FILE *fp = fopen(filePath.c_str(), "rb");
  if (fp != NULL) {
    int stripeNumber = 0;
    fread(&stripeNumber, sizeof(int), 1, fp);

    PROCESS_ERROR(stripeNumber < 0, "Invalid stripe number", return false);
    /*
    if (stripeNumber < 0) {
      RECORD_ERROR(true, "Invalid stripe number");
      return false;
    }
    */

    m_stripeArray.resize(stripeNumber);
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->read(fp);
    }
    fclose(fp);

    if (isCanonizedActually()) {
      m_isCanonized = true;
    } else {
      m_isCanonized = false;
    }

    deprecate(ALL_COMPONENT);

    return true;
  } else {
    RECORD_WARNING(true, "Cannont open file " + filePath);
  }

  return false;
}

void ZObject3dScan::save(const string &filePath) const
{
  FILE *fp = fopen(filePath.c_str(), "wb");
  if (fp != NULL) {
    int stripeNumber = (int) getStripeNumber();
    fwrite(&stripeNumber, sizeof(int), 1, fp);
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->write(fp);
    }
    fclose(fp);
  } else {
    RECORD_WARNING(true, "Cannont open file " + filePath);
  }
}

size_t ZObject3dScan::countForegroundOverlap(Stack *stack, const int *offset)
{
  size_t count = 0;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    count += iter->countForegroundOverlap(stack, offset);
  }

  return count;
}

void ZObject3dScan::drawStack(Stack *stack, int v, const int *offset) const
{
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    iter->drawStack(stack, v, offset);
  }
}

void ZObject3dScan::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue, const int *offset) const
{
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    iter->drawStack(stack, red, green, blue, offset);
  }
}

static int ZObject3dStripeCompare(const void *e1, const void *e2)
{
  ZObject3dStripe *v1 = (ZObject3dStripe*) e1;
  ZObject3dStripe *v2 = (ZObject3dStripe*) e2;

  if (v1->getZ() < v2->getZ()) {
    return -1;
  } else if (v1->getZ() > v2->getZ()) {
    return 1;
  } else {
    if (v1->getY() < v2->getY()) {
      return -1;
    } else if (v1->getY() > v2->getY()) {
      return 1;
    }
  }

  return 0;
}

void ZObject3dScan::sort()
{
  if (!isEmpty()) {
    qsort(&m_stripeArray[0], m_stripeArray.size(), sizeof(ZObject3dStripe),
        ZObject3dStripeCompare);
    deprecate(ALL_COMPONENT);
  }
}

void ZObject3dScan::canonize()
{
  if (!isEmpty() && !isCanonized()) {
    sort();
    vector<ZObject3dStripe> newStripeArray;
    //newStripeArray.reserve(m_stripeArray.size());
    m_stripeArray[0].canonize();
    newStripeArray.push_back(m_stripeArray[0]);
    for (size_t i = 1; i < m_stripeArray.size(); ++i) {
      if (!newStripeArray.back().unify(m_stripeArray[i])) {
        m_stripeArray[i].canonize();
        newStripeArray.push_back(m_stripeArray[i]);
      }
    }

    //m_stripeArray = newStripeArray;
    m_stripeArray.swap(newStripeArray);
  }
}

void ZObject3dScan::unify(const ZObject3dScan &obj)
{
  concat(obj);
  canonize();
}

void ZObject3dScan::concat(const ZObject3dScan &obj)
{
  m_stripeArray.insert(m_stripeArray.end(), obj.m_stripeArray.begin(),
                       obj.m_stripeArray.end());
  m_isCanonized = false;
  deprecate(ALL_COMPONENT);
}

void ZObject3dScan::downsample(int xintv, int yintv, int zintv)
{
  if (yintv > 0 || zintv > 0) {
    vector<ZObject3dStripe> newStripeArray;
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      if ((iter->getY() % (yintv + 1) == 0) &&
          (iter->getZ() % (zintv + 1) == 0)) {
        newStripeArray.push_back(*iter);
      }
    }
    m_stripeArray = newStripeArray;
    m_isCanonized = false;
  }

  if (xintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->downsample(xintv);
      iter->setY(iter->getY() / (yintv + 1));
      iter->setZ(iter->getZ() / (zintv + 1));
    }
    m_isCanonized = false;
  }

  if (xintv > 0 || yintv > 0 || zintv > 0) {
    deprecate(ALL_COMPONENT);
  }
}

void ZObject3dScan::downsampleMax(int xintv, int yintv, int zintv)
{
  if (yintv > 0 || zintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->setY(iter->getY() / (yintv + 1));
      iter->setZ(iter->getZ() / (zintv + 1));
    }
    m_isCanonized = false;
  }

  if (xintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->downsampleMax(xintv);
    }
    m_isCanonized = false;
  }

  canonize();

  deprecate(ALL_COMPONENT);
}

Stack* ZObject3dScan::toStack(int *offset) const
{
  if (isEmpty()) {
    return NULL;
  }

  ZCuboid boundBox = getBoundBox();
  if (offset != NULL) {
    offset[0] = boundBox.firstCorner().x();
    offset[1] = boundBox.firstCorner().y();
    offset[2] = boundBox.firstCorner().z();
  }

  Stack *stack = C_Stack::make(GREY, boundBox.width() + 1,
                               boundBox.height() + 1,
                               boundBox.depth() + 1);
  C_Stack::setZero(stack);


  int drawingOffet[3];
  drawingOffet[0] = -boundBox.firstCorner().x();
  drawingOffet[1] = -boundBox.firstCorner().y();
  drawingOffet[2] = -boundBox.firstCorner().z();

  drawStack(stack, 1, drawingOffet);

  return stack;
}

ZCuboid ZObject3dScan::getBoundBox() const
{
  ZCuboid boundBox;

  bool isFirst = true;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    if (!iter->isEmpty()) {
      if (isFirst) {
        boundBox.set(iter->getMinX(), iter->getY(), iter->getZ(),
                     iter->getMaxX(), iter->getY(), iter->getZ());
        isFirst = false;
      } else {
        boundBox.joinY(iter->getY());
        boundBox.joinZ(iter->getZ());
        boundBox.joinX(iter->getMinX());
        boundBox.joinX(iter->getMaxX());
      }
    }
  }

  return boundBox;
}

void ZObject3dScan::getBoundBox(Cuboid_I *box) const
{
  ZCuboid boundBox = getBoundBox();

  Cuboid_I_Set_S(box, boundBox.firstCorner().x(), boundBox.firstCorner().y(),
                 boundBox.firstCorner().z(), boundBox.width() + 1,
                 boundBox.height() + 1, boundBox.depth() + 1);
}

const std::vector<size_t>& ZObject3dScan::getStripeNumberAccumulation() const
{
  if (isDeprecated(ACCUMULATED_STRIPE_NUMBER)) {
    m_accNumberArray.resize(getStripeNumber() + 1);
    m_accNumberArray[0] = 0;
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      m_accNumberArray[i + 1] =
          m_accNumberArray[i] + m_stripeArray[i].getSegmentNumber();
    }
  }

  return m_accNumberArray;
}

const std::map<std::pair<int, int>, size_t> &ZObject3dScan::getStripeMap() const
{
  if (isDeprecated(STRIPE_INDEX_MAP)) {
    m_stripeMap.clear();
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      m_stripeMap[std::pair<int, int>(
            m_stripeArray[i].getZ(), m_stripeArray[i].getY())] = i;
    }
  }

  return m_stripeMap;
}

ZGraph* ZObject3dScan::buildConnectionGraph()
{
  ZGraph *graph = new ZGraph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  canonize();

  const std::vector<size_t>& stripeNumberAccumulation =
      getStripeNumberAccumulation();

  const std::map<std::pair<int, int>, size_t> &stripeMap = getStripeMap();

  for (size_t i = 0; i < getStripeNumber() - 1; ++i) {
    //Check along Y
    for (int j = 0; j < m_stripeArray[i].getSegmentNumber(); ++j) {
      const int *stripe = m_stripeArray[i].getSegment(j);
      int v1 = stripeNumberAccumulation[i] + j;

      //Check along Y
      if ((m_stripeArray[i].getZ() == m_stripeArray[i + 1].getZ()) &&
          (m_stripeArray[i].getY() + 1 == m_stripeArray[i + 1].getY())) {
        for (int k = 0; k < m_stripeArray[i + 1].getSegmentNumber(); ++k) {
          const int *downStairStripe = m_stripeArray[i + 1].getSegment(k);

          if (IS_IN_CLOSE_RANGE(stripe[0], downStairStripe[0] - 1, downStairStripe[1] + 1) ||
              IS_IN_CLOSE_RANGE(downStairStripe[0], stripe[0] - 1, stripe[1] + 1)) {
            int v2 = stripeNumberAccumulation[i + 1] + k;
            graph->addEdgeFast(v1, v2);
            //break;
          }
        }
      }

      //Check along Z

      for (int dy = -1; dy <= 1; ++dy) {
        std::pair<int, int> neighborStripeZY(
              m_stripeArray[i].getZ() + 1, m_stripeArray[i].getY() + dy);
#ifdef _DEBUG_2
        std::cout << "Neighbor stripe position: Y=" << neighborStripeZY.second
                  << ", Z=" << neighborStripeZY.first << endl;
#endif
        if (stripeMap.count(neighborStripeZY) > 0) {
          size_t neighborStripeIndex = stripeMap.at(neighborStripeZY);
          for (int k = 0;
               k < m_stripeArray[neighborStripeIndex].getSegmentNumber(); ++k) {
            const int *downStairStripe =
                m_stripeArray[neighborStripeIndex].getSegment(k);

            if (IS_IN_CLOSE_RANGE(stripe[0], downStairStripe[0] - 1, downStairStripe[1] + 1) ||
                IS_IN_CLOSE_RANGE(downStairStripe[0], stripe[0] - 1, stripe[1] + 1)) {
              int v2 = stripeNumberAccumulation[neighborStripeIndex] + k;
              graph->addEdgeFast(v1, v2);
              //break;
            }
          }
        }
      }
    }
  }

  return graph;
}

void ZObject3dScan::clear()
{
  m_stripeArray.clear();
  m_isCanonized = true;
  deprecate(ALL_COMPONENT);
}

std::vector<size_t> ZObject3dScan::getConnectedObjectSize()
{
  std::vector<size_t> sizeArray;
#if 0
  ZObject3d *obj3d = toObject3d();
  if (obj3d != NULL) {
    Stack *stack = obj3d->toStack();
    C_Stack::translate(stack, GREY16, 1);

    Stack_Label_Objects_N(stack, NULL, 1, 2, 26);

    std::vector<size_t> hist(65535, 0);
    uint16_t *array16 = (uint16_t*) stack->array;
    size_t voxelNumber = C_Stack::voxelNumber(stack);
    for (size_t i = 0; i < voxelNumber; ++i) {
      hist[array16[i]]++;
    }

    for (size_t i = 2; i < 65535; i++) {
      if (hist[i] > 0) {
        sizeArray.push_back(hist[i]);
      }
    }

    delete obj3d;
    C_Stack::kill(stack);
  }
#else
  if (!isEmpty()) {
    std::vector<ZObject3dScan> objArray = getConnectedComponent();

    sizeArray.resize(objArray.size());
    for (size_t i = 0; i < objArray.size(); ++i) {
      sizeArray[i] = objArray[i].getVoxelNumber();
    }
  }
#endif

  if (!sizeArray.empty()) {
    std::sort(sizeArray.begin(), sizeArray.end());
    std::reverse(sizeArray.begin(), sizeArray.end());
  }

  return sizeArray;
}

std::vector<ZObject3dScan> ZObject3dScan::getConnectedComponent()
{
  ZGraph *graph = buildConnectionGraph();

  const std::vector<ZGraph*> &subGraph = graph->getConnectedSubgraph();

  const std::map<size_t, std::pair<size_t, size_t> >&segMap =
      getIndexSegmentMap();

  std::vector<ZObject3dScan> objArray;
  std::vector<bool> isAdded(segMap.size(), false);

  for (std::vector<ZGraph*>::const_iterator iter = subGraph.begin();
       iter != subGraph.end(); ++iter) {
    ZObject3dScan subobj;
    for (int edgeIndex = 0; edgeIndex < (*iter)->getEdgeNumber(); ++edgeIndex) {
      int v1 = (*iter)->edgeStart(edgeIndex);
      int v2 = (*iter)->edgeEnd(edgeIndex);
      int z, y, x1, x2;
      getSegment(v1, &z, &y, &x1, &x2);
      subobj.addSegment(z, y, x1, x2, false);
      getSegment(v2, &z, &y, &x1, &x2);
      subobj.addSegment(z, y, x1, x2, false);
      isAdded[v1] = true;
      isAdded[v2] = true;
    }
    subobj.canonize();
    objArray.push_back(subobj);
  }

  delete graph;

  for (size_t i = 0; i < isAdded.size(); ++i) {
    if (!isAdded[i]) {
      ZObject3dScan subobj;
      int z, y, x1, x2;
      getSegment(i, &z, &y, &x1, &x2);
      subobj.addSegment(z, y, x1, x2);
      objArray.push_back(subobj);
    }
  }


  return objArray;
}

size_t ZObject3dScan::getSegmentNumber()
{
  const std::vector<size_t>& accArray = getStripeNumberAccumulation();

  return accArray.back();
}

void ZObject3dScan::translate(int dx, int dy, int dz)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].translate(dx, dy, dz);
  }
}

void ZObject3dScan::addZ(int dz)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].addZ(dz);
  }
}

bool ZObject3dScan::isCanonizedActually()
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    if (i > 0) {
      if (m_stripeArray[i - 1].getZ() > m_stripeArray[i].getZ()) {
#ifdef _DEBUG_
        std::cout << "Previous Z is bigger: " << m_stripeArray[i - 1].getZ()
                  << " " << m_stripeArray[i].getZ() << std::endl;
#endif
        return false;
      } else if (m_stripeArray[i - 1].getZ() == m_stripeArray[i].getZ()) {
        if (m_stripeArray[i - 1].getY() >= m_stripeArray[i].getY()) {
#ifdef _DEBUG_
          std::cout << "Previous Z/Y is bigger: " << m_stripeArray[i - 1].getY()
                    << " " << m_stripeArray[i].getY() << std::endl;
#endif
          return false;
        }
      }
    }
    if (!m_stripeArray[i].isCanonizedActually()) {
      return false;
    }
  }

  return true;
}

void ZObject3dScan::duplicateAcrossZ(int depth)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].setZ(0);
  }
  setCanonized(false);
  canonize();

  size_t orignalStripeNumber = m_stripeArray.size();

  for (int z = 1; z < depth; ++z) {
    for (size_t i = 0; i < orignalStripeNumber; ++i) {
      ZObject3dStripe stripe = m_stripeArray[i];
      stripe.setZ(z);
      addStripe(stripe, false);
    }
  }
  deprecate(ALL_COMPONENT);
}

void ZObject3dScan::display(QPainter &painter, int z, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if _QT_GUI_USED_
  QPen pen(m_color, .7);
  painter.setPen(pen);
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    ZObject3dStripe stripe = getStripe(i);
    if (stripe.getZ() == z) {
      for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
        painter.drawLine(stripe.getSegmentStart(j), stripe.getY(),
                         stripe.getSegmentEnd(j), stripe.getY());
      }
    }
  }
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(style);
#endif
}

void ZObject3dScan::dilate()
{
  size_t oldStripeNumber = getStripeNumber();
  for (size_t i = 0; i < oldStripeNumber; ++i) {
    ZObject3dStripe stripe = m_stripeArray[i];
    stripe.setY(stripe.getY() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = m_stripeArray[i];
    stripe.setY(stripe.getY() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = m_stripeArray[i];
    stripe.setZ(stripe.getZ() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = m_stripeArray[i];
    stripe.setZ(stripe.getZ() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    m_stripeArray[i].dilate();
  }

  setCanonized(false);
  canonize();
}

ZObject3dScan ZObject3dScan::getSlice(int z) const
{
  ZObject3dScan slice;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    if (m_stripeArray[i].getZ() == z) {
      slice.addStripe(m_stripeArray[i], false);
    }
  }

  return slice;
}

ZObject3dScan ZObject3dScan::getSlice(int minZ, int maxZ) const
{
  ZObject3dScan slice;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    if (m_stripeArray[i].getZ() >= minZ) {
      if (m_stripeArray[i].getZ() <= maxZ) {
        slice.addStripe(m_stripeArray[i], false);
      }
    }
  }

  return slice;
}

ZPoint ZObject3dScan::getCentroid() const
{
  ZPoint center(0, 0, 0);
  int count = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    int nseg = m_stripeArray[i].getSegmentNumber();
    for (int j = 0; j < nseg; ++j) {
      int x1 = m_stripeArray[i].getSegmentStart(j);
      int x2 = m_stripeArray[i].getSegmentEnd(j);
      int len = x2 - x1 + 1;
      count += len;
      center.translate((x1 + x2) * len /2, m_stripeArray[i].getY() * len,
                       m_stripeArray[i].getZ() * len);
    }
  }

  if (count > 0) {
    center /= count;
  }

  return center;
}

ZObject3dScan ZObject3dScan::makeZProjection(int minZ, int maxZ)
{
  return getSlice(minZ, maxZ).makeZProjection();
}

ZHistogram ZObject3dScan::getRadialHistogram(int z) const
{
  ZHistogram hist;
  hist.setInterval(1.0);
  hist.setStart(0.0);

  ZObject3dScan slice = getSlice(z);

  ZPoint center = slice.getCentroid();
#ifdef _DEBUG_2
  std::cout << center.toString() << std::endl;
#endif
  for (size_t i = 0; i < slice.getStripeNumber(); ++i) {
    int nseg = slice.m_stripeArray[i].getSegmentNumber();
    int y = slice.m_stripeArray[i].getY();
    for (int j = 0; j < nseg; ++j) {
      int x1 = slice.m_stripeArray[i].getSegmentStart(j);
      int x2 = slice.m_stripeArray[i].getSegmentEnd(j);
      for (int x = x1; x <= x2; ++x) {
        double dist = center.distanceTo(x, y, z);
        hist.addCount(dist, 1.0 / (dist + 0.5));
      }
    }
  }

  return hist;
}

ZObject3dScan ZObject3dScan::makeZProjection() const
{
  ZObject3dScan proj;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    proj.addStripe(0, m_stripeArray[i].getY());
    int nseg = m_stripeArray[i].getSegmentNumber();
    for (int j = 0; j < nseg; ++j) {
      int x1 = m_stripeArray[i].getSegmentStart(j);
      int x2 = m_stripeArray[i].getSegmentEnd(j);
      proj.addSegment(x1, x2, false);
    }
  }

  proj.canonize();

  return proj;
}

int ZObject3dScan::getMinZ() const
{
  int minZ = 0;
  if (isCanonized()) {
    if (!isEmpty()) {
      minZ = m_stripeArray[0].getZ();
    }
  } else {
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      int z = m_stripeArray[i].getZ();
      if (i == 0) {
        minZ = z;
      } else {
        if (z < minZ) {
          minZ = z;
        }
      }
    }
  }

  return minZ;
}

int ZObject3dScan::getMaxZ() const
{
  int maxZ = 0;
  if (isCanonized()) {
    if (!isEmpty()) {
      maxZ = m_stripeArray.back().getZ();
    }
  } else {
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      int z = m_stripeArray[i].getZ();
      if (i == 0) {
        maxZ = z;
      } else {
        if (z > maxZ) {
          maxZ = z;
        }
      }
    }
  }

  return maxZ;
}

ZVoxel ZObject3dScan::getMarker() const
{
  if (isEmpty()) {
    return ZVoxel(-1, -1, -1);
  }

  int minZ = getMinZ();
  int maxZ = getMaxZ();
  int offset[3];
  ZVoxel voxel;
  for (int z = minZ; z < maxZ; ++z) {
    ZObject3dScan slice = getSlice(z);
    if (!slice.isEmpty()) {
      Stack *stack = slice.toStack(offset);
      Stack *dist = Stack_Bwdist_L_U16P(stack, NULL, 0);
      size_t index;
      double v = sqrt(Stack_Max(dist, &index));
      if (v > voxel.value()) {
        int x, y, tmpz;
        C_Stack::indexToCoord(index, C_Stack::width(stack), C_Stack::height(stack),
                              &x, &y, &tmpz);

        voxel.set(x + offset[0], y + offset[1], z, v);
      }
      Kill_Stack(stack);
    }
  }

  return voxel;
}

bool ZObject3dScan::equalsLiterally(const ZObject3dScan &obj) const
{
  if (m_stripeArray.size() != obj.m_stripeArray.size()) {
    return false;
  }

  for (size_t i = 0; i < m_stripeArray.size(); ++i) {
    if (!m_stripeArray[i].equalsLiterally(obj.m_stripeArray[i])) {
      return false;
    }
  }

  return true;
}

ZINTERFACE_DEFINE_CLASS_NAME(ZObject3dScan)
