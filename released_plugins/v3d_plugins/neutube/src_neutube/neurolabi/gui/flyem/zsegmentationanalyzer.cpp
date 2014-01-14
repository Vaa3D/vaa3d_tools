#include "zsegmentationanalyzer.h"

#include <iostream>
#include <cmath>
#include <sys/stat.h>

#include "tz_math.h"
#include "tz_int_histogram.h"
#include "tz_iarray.h"
#include "tz_stack_neighborhood.h"
#include "tz_stack_lib.h"
#include "tz_stack_code.h"
#include "tz_stack_utils.h"
#include "tz_geo3d_utils.h"
#include "c_stack.h"
#include "tz_constant.h"
#include "zpoint.h"
#include "zhdf5writer.h"
#include "zhdf5reader.h"

using namespace std;

map<string, FlyEm::ZSegmentationAnalyzer::BcfSet>
FlyEm::ZSegmentationAnalyzer::m_bcfSetNameMap =
    FlyEm::ZSegmentationAnalyzer::createBcfSetNameMap();

map<string, FlyEm::ZSegmentationAnalyzer::BcfSet>
FlyEm::ZSegmentationAnalyzer::createBcfSetNameMap()
{
  map<string, BcfSet> bcfMap;
  bcfMap["Body_Grow"] = BCF_BODY_GROW;
  bcfMap["Body_Size"] = BCF_BODY_SIZE;
  bcfMap["Body_Intensity"] = BCF_BODY_INTENSITY;
  bcfMap["Boundary_Grow"] = BCF_BOUNDARY_GROW;
  bcfMap["Rayburst"] = BCF_RAYBURST;
  bcfMap["Border_Intensity"] =BCF_BORDER_INTENSITY;
  bcfMap["Border_Size"] = BCF_BORDER_SIZE;
  bcfMap["Gala_Segmentation"] = BCF_GALA_SEGMENTATION;
  bcfMap["Boundary_Prob"] = BCF_BOUNDARY_PROB;
  bcfMap["Mito_Prob"] = BCF_MITO_PROB;
  bcfMap["Cyto_Prob"] = BCF_CYTO_PROB;

  return bcfMap;
}

FlyEm::ZSegmentationAnalyzer::ZSegmentationAnalyzer()
{
}

vector<uint8_t> FlyEm::ZSegmentationAnalyzer::idToChannelCode(
    int id, int channelNumber)
{
  vector<uint8_t> channelCode(channelNumber);

  for (size_t i = 0; i < channelCode.size(); i++) {
    channelCode[i] = id % 256;
    id /= 256;
  }

  return channelCode;
}

int FlyEm::ZSegmentationAnalyzer::channelCodeToId(
    const std::vector<uint8_t> &channelCode)
{
  int id = 0;

  for (int i = channelCode.size() - 1; i >= 0; --i) {
    id = id * 256 + channelCode[i];
  }

  return id;
}

int FlyEm::ZSegmentationAnalyzer::channelCodeToId(
    const std::vector<double> &channelCode)
{
  int id = 0;

  for (int i = channelCode.size() - 1; i >= 0; --i) {
    id = id * 256 + iround(channelCode[i]);
  }

  return id;
}

int FlyEm::ZSegmentationAnalyzer::channelCodeToId(
    const uint8_t *array, int channelNumber,
    size_t index, size_t stride)
{
  int id = 0;

  for (int i = channelNumber - 1; i >= 0; --i) {
    id = id * 256 + array[index + i * stride];
  }

  return id;
}

vector<double> FlyEm::ZSegmentationAnalyzer::computeTouchFeature(
    Stack *stack, const vector<int> &seed)
{
  vector<double> feat;

  Stack *code= Stack_Level_Code(stack, NULL, NULL, &(seed[0]),
      seed.size());

  int *hist = Stack_Hist(code);
  Kill_Stack(code);

  int *histValue = Int_Histogram_Array(hist);
  int histLength = Int_Histogram_Length(hist);
  if (histLength > 2) {
    size_t index;
    int maxArea = iarray_max(histValue + 1, min(histLength - 1, 10), &index);
    feat.push_back(sqrt(maxArea) - sqrt(histValue[1]));
    feat.push_back(feat.back() / (index + 1));
    feat.push_back(maxArea - histValue[1]);
    feat.push_back(feat.back() / (index + 1));
  } else {
    feat.push_back(0.0);
    feat.push_back(0.0);
    feat.push_back(0.0);
    feat.push_back(0.0);
  }
  free(hist);

  return feat;
}

vector<double> FlyEm::ZSegmentationAnalyzer::computeRayburstTouchFeature(
    Stack **stack, std::vector<int> seed[])
{
  vector<double> direction[2];
  ZPoint normal[2];
  for (size_t k = 0; k < 2; ++k) {
    direction[k] = computeRayburstDirection(
          stack[k], &(seed[k][0]), seed[k].size());
    Geo3d_Orientation_Normal(direction[k][0], direction[k][1], normal[k].xRef(),
        normal[k].yRef(), normal[k].zRef());
  }

  vector<double> feature;
  feature.push_back(normal[0].dot(normal[1]));

  return feature;
}

vector<double> FlyEm::ZSegmentationAnalyzer::computeRayburstTouchFeature(
    Stack **stack, std::vector<int> seed)
{
  vector<double> direction[2];
  ZPoint normal[2];
  for (size_t k = 0; k < 2; ++k) {
    direction[k] = computeRayburstDirection(
          stack[k], &(seed[0]), seed.size());
    Geo3d_Orientation_Normal(direction[k][0], direction[k][1], normal[k].xRef(),
        normal[k].yRef(), normal[k].zRef());
  }

#ifdef _DEBUG_
  cout << normal[0].toString() << endl;
  cout << normal[1].toString() << endl;
#endif

  vector<double> feature;
  feature.push_back(normal[0].dot(normal[1]));

  return feature;
}

vector<double> FlyEm::ZSegmentationAnalyzer::computeRayburstDirection(
    const Stack *stack, const int *seedArray, size_t seedNumber)
{
  double currentLength = 0;
  vector<double> direction(2, 0.0);

  //For each seed
  for (size_t i = 0; i < seedNumber; ++i) {
    if (stack->array[seedArray[i]] > 0) {
      //Find the direction of the longest ray
      vector<double> rayburst = computeRayburstDirection(stack, seedArray[i]);
      //If it's longer than the current ray
      if (rayburst[0] > currentLength) {
        //Update the direction and ray length
        currentLength = rayburst[0];
        direction[0] = rayburst[1];
        direction[1] = rayburst[2];
      }
    }
  }

  return direction;
}

vector<double> FlyEm::ZSegmentationAnalyzer::computeRayburstDirection(
    const Stack *stack, int seed)
{
  uint8_t *array8 = C_Stack::array8(stack);

  vector<double> result(3, 0.0);
  if (array8[seed] == 0) {
    cout << "Seed on background." << endl;
    return result;
  }

  double theta, psi;
  double bestTheta, bestPsi;
  double dx, dy, dz;
  double currX, currY, currZ;
  int x, y, z;

  Stack_Util_Coord(seed, C_Stack::width(stack), C_Stack::height(stack),
                   &x, &y, &z);

  int maxCount = 0;
  double deltaPsi = 0.0;

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  //For each direction
  for (theta = 0.0; theta <= TZ_PI; theta += 0.0872665) {
    if (theta < 0.005) {
      deltaPsi = TZ_2PI;
    } else {
      deltaPsi = 0.1 / sin(theta);
    }

    for (psi = 0.0; psi < TZ_2PI; psi += deltaPsi) {
      currX = x;
      currY = y;
      currZ = z;
      int count = 0;

      bool inside = true;
      Geo3d_Orientation_Normal(theta, psi, &dx, &dy, &dz);

#ifdef _DEBUG_2
      cout << "Step: " << dx << " " << dy << " " << dz << endl;
#endif

      while (inside) {
        ++count;
        //Step forward until it hits a backgound voxel
        currX += dx;
        currY += dy;
        currZ += dz;

#ifdef _DEBUG_2
        cout << currX << " " << currY << " " << currZ << endl;
#endif

        int voxelIndex = Stack_Util_Offset(
              iround(currX), iround(currY), iround(currZ),
              width, height, depth);
        if (voxelIndex >= 0) {
          if (array8[voxelIndex] == 0) {
            inside = false;
          }
        } else {
          inside = false;
        }
      }

      if (count > maxCount) {
        maxCount = count;
        bestTheta = theta;
        bestPsi = psi;
      }

#ifdef _DEBUG_2
      if (count == 15) {
        cout << "debug here" << endl;
      }
#endif
    }
  }

  //return voxel count and voxel direction
  result[0] = maxCount;
  result[1] = bestTheta;
  result[2] = bestPsi;

  return result;
}

vector<double> FlyEm::ZSegmentationAnalyzer::touchFeature(
    const ZStack &stack, int id1, int id2)
{
  vector<int> seedArray[2];
  int neighbor_offset[26];
  int is_in_bound[26];

  Stack_Neighbor_Offset(26, stack.width(), stack.height(), neighbor_offset);

  int seedId[2];
  seedId[0] = id1;
  seedId[1] = id2;

  vector<uint8_t> seedColor[2];

  Stack *objStack[2];
  for (int i = 0; i < 2; ++i) {
    objStack[i] = Make_Stack(GREY, stack.width(), stack.height(), stack.depth());
    Zero_Stack(objStack[i]);
    seedColor[i] = idToChannelCode(seedId[i], stack.channelNumber());
  }

  size_t volume = stack.getVoxelNumber();
  size_t channelNumber = seedColor[0].size();
  size_t channelByteNumber = stack.getByteNumber(ZStack::SINGLE_CHANNEL);

  for (size_t offset = 0; offset < volume; ++offset) {
    for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex) {
      bool isSeed = false;
      //if (stack.equalColor(offset, &(seedColor[bodyIndex][0]), channelNumber)) {
      if (stack.equalColor(offset, channelByteNumber, &(seedColor[bodyIndex][0]), channelNumber)) {
        objStack[bodyIndex]->array[offset] = 1;
        int nnbr = Stack_Neighbor_Bound_Test_I(
              26, stack.width(), stack.height(), stack.depth(), offset,
              is_in_bound);

        vector<uint8_t> neighborColor = seedColor[(bodyIndex + 1) % 2];
        for (int i = 0; i < 26; ++i) {
          if (nnbr == 26 || is_in_bound[i]) {
            /*
            if (stack.equalColor(offset + neighbor_offset[i],
                                 &(neighborColor[0]), channelNumber)) {
                                 */
            if (stack.equalColor(offset + neighbor_offset[i], channelByteNumber,
                                 &(neighborColor[0]), channelNumber)) {
              isSeed = true;
              break;
            }
          }
        }
      }
      if (isSeed) {
        seedArray[bodyIndex].push_back(offset);
      }
    }
  }

  vector<double> feature;

  for (int i = 0; i < 2; i++) {
    vector<double> newfeat = computeTouchFeature(objStack[i], seedArray[i]);
    feature.insert(feature.end(), newfeat.begin(), newfeat.end());
  }

  Kill_Stack(objStack[0]);
  Kill_Stack(objStack[1]);

  return feature;
}

std::vector<double> FlyEm::ZSegmentationAnalyzer::touchFeature(
    FlyEm::ZSegmentationBundle &bundle, int id1, int id2)
{
  ZStack *bodyStack = bundle.getBodyStack();

  int seedId[2];
  seedId[0] = id1;
  seedId[1] = id2;


  Stack *objStack[2];
  for (int i = 0; i < 2; ++i) {
    objStack[i] = Make_Stack(GREY, bodyStack->width(), bodyStack->height(), bodyStack->depth());
    Zero_Stack(objStack[i]);
    bundle.getBodyObject(seedId[i])->labelStack(objStack[i], 1);
  }

  vector<int> seedArray = bundle.getBodyBorderObject(id1, id2)->toIndexArray<int>(
        bodyStack->width(), bodyStack->height(), bodyStack->depth());

  vector<double> feature;

  for (int i = 0; i < 2; i++) {
    vector<double> newfeat = computeTouchFeature(objStack[i], seedArray);
    feature.insert(feature.end(), newfeat.begin(), newfeat.end());
  }

  vector<double> newfeat = computeRayburstTouchFeature(objStack, seedArray);
  feature.insert(feature.end(), newfeat.begin(), newfeat.end());

  Kill_Stack(objStack[0]);
  Kill_Stack(objStack[1]);

  return feature;
}

std::vector<std::vector<double> >
FlyEm::ZSegmentationAnalyzer::touchFeature(FlyEm::ZSegmentationBundle &bundle)
{
  ZGraph *bodyGraph = bundle.getBodyGraph();

  vector<vector<double> > feature;
  for (size_t i = 0; i < bodyGraph->size(); ++i) {
#ifdef _DEBUG_
    cout << i << "/" << bodyGraph->size() << ": " << bodyGraph->edgeStart(i)
         << " " << bodyGraph->edgeEnd(i) << endl;
#endif

    feature.push_back(
          touchFeature(bundle, bodyGraph->edgeStart(i), bodyGraph->edgeEnd(i)));
  }

  return feature;
}

std::vector<std::vector<double> >
FlyEm::ZSegmentationAnalyzer::computeBcf(
    FlyEm::ZSegmentationBundle &bundle, BcfSet setName)
{
  ZGraph *bodyGraph = bundle.getBodyGraph();

  vector<vector<double> > feature;
  for (size_t i = 0; i < bodyGraph->size(); ++i) {
#ifdef _DEBUG_
    cout << i << "/" << bodyGraph->size() << ": " << bodyGraph->edgeStart(i)
         << " " << bodyGraph->edgeEnd(i) << endl;
#endif

    feature.push_back(
          computeBcf(bundle, bodyGraph->edgeStart(i), bodyGraph->edgeEnd(i), setName));
  }

  return feature;
}

std::vector<std::vector<double> >
FlyEm::ZSegmentationAnalyzer::computeBcf(
    FlyEm::ZSegmentationBundle &bundle, const std::vector<string> &setName)
{
  ZGraph *bodyGraph = bundle.getBodyGraph();

  vector<vector<double> > feature;
  for (size_t i = 0; i < bodyGraph->size(); ++i) {
#ifdef _DEBUG_
    cout << i << "/" << bodyGraph->size() << ": " << bodyGraph->edgeStart(i)
         << " " << bodyGraph->edgeEnd(i) << endl;
#endif

    feature.push_back(
          computeBcf(bundle, bodyGraph->edgeStart(i), bodyGraph->edgeEnd(i), setName));
  }

  return feature;
}

void FlyEm::ZSegmentationAnalyzer::generateBcf(
    FlyEm::ZSegmentationBundle &bundle, const string &setName)
{
  if (!fexist(bundle.getBcfPath(setName).c_str())) {
    vector<vector<double> > feature =
        FlyEm::ZSegmentationAnalyzer::computeBcf(bundle, setName);
#if defined(_WIN32) || defined(_WIN64)
    mkdir(bundle.getBcfPath().c_str());
#else
    mkdir(bundle.getBcfPath().c_str(), 0777);
#endif

    ZHdf5Writer writer;
    writer.open(bundle.getBcfPath(setName));
    mylib::Dimn_Type dims[2];
    dims[0] = feature.size();
    dims[1] = feature[0].size();
    mylib::Array *array = mylib::Make_Array(
          mylib::PLAIN_KIND, mylib::FLOAT64_TYPE, 2, dims);

    double *darray = (double*) array->data;
    int offset = 0;
    for (size_t row = 0; row < feature.size(); ++row) {
      for (size_t col = 0; col < feature[0].size(); ++col) {
        darray[offset++] = feature[row][col];
      }
    }

    writer.writeArray(std::string("/") + setName, array);
    mylib::Kill_Array(array);
  }
}

vector<double> FlyEm::ZSegmentationAnalyzer::touchFeature(
    const ZStack &stack, int id1, int id2, const ZStack &mask)
{
  vector<int> seedArray[2];
  int neighbor_offset[26];
  int is_in_bound[26];

  Stack_Neighbor_Offset(26, stack.width(), stack.height(), neighbor_offset);

  int seedId[2];
  seedId[0] = id1;
  seedId[1] = id2;

  vector<uint8_t> seedColor[2];

  Stack *objStack[2];
  for (int i = 0; i < 2; ++i) {
    objStack[i] = Make_Stack(GREY, stack.width(), stack.height(), stack.depth());
    Zero_Stack(objStack[i]);
    seedColor[i] = idToChannelCode(seedId[i], stack.channelNumber());
  }

  size_t volume = stack.getVoxelNumber();
  size_t channelNumber = seedColor[0].size();
  size_t channelByteNumber = stack.getByteNumber(ZStack::SINGLE_CHANNEL);

  const uint8_t *maskArray = mask.array8();

  for (size_t offset = 0; offset < volume; ++offset) {
    for (int bodyIndex = 0; bodyIndex < 2; ++bodyIndex) {
      bool isSeed = false;
      //if (channelCodeToId(stack.color(offset)) == seedId[bodyIndex]) {
      //if (stack.equalColor(offset, seedColor[bodyIndex])) {

      if (stack.equalColor(offset, channelByteNumber, &(seedColor[bodyIndex][0]), channelNumber)) {
        objStack[bodyIndex]->array[offset] = 1;
        if (maskArray[offset] > 0) {
          int nnbr = Stack_Neighbor_Bound_Test_I(
                26, stack.width(), stack.height(), stack.depth(), offset,
                is_in_bound);

          vector<uint8_t> &neighborColor = seedColor[(bodyIndex + 1) % 2];
          for (int i = 0; i < 26; ++i) {
            if (nnbr == 26 || is_in_bound[i]) {
              /*
            if (channelCodeToId(stack.color(offset + neighbor_offset[i])) ==
                seedId[(bodyIndex + 1) % 2]) {
                */
              /*
            if (stack.equalColor(offset + neighbor_offset[i],
                                 seedColor[(bodyIndex + 1) % 2])) {
                                 */
              if (stack.equalColor(offset + neighbor_offset[i], channelByteNumber,
                                   &(neighborColor[0]), channelNumber)) {
                isSeed = true;
                break;
              }
            }
          }
        }
      }
      if (isSeed) {
        seedArray[bodyIndex].push_back(offset);
      }
    }
  }

#ifdef _DEBUG_2
  cout << "Seed size: " << seedArray[0].size() << " " << seedArray[1].size()
       << endl;
#endif

  vector<double> feature;

  for (int i = 0; i < 2; i++) {
    vector<double> newfeat = computeTouchFeature(objStack[i], seedArray[i]);
    feature.insert(feature.end(), newfeat.begin(), newfeat.end());
  }

  /*
  vector<double> newfeat = computeRayburstTouchFeature(objStack, seedArray);
  feature.insert(feature.end(), newfeat.begin(), newfeat.end());
*/

  Kill_Stack(objStack[0]);
  Kill_Stack(objStack[1]);

  return feature;
}

vector<double> FlyEm::ZSegmentationAnalyzer::touchFeature(
    const ZStack &stack, const vector<double> &c1, const vector<double> &c2)
{
  return touchFeature(stack, channelCodeToId(c1), channelCodeToId(c2));
}

vector<double> FlyEm::ZSegmentationAnalyzer::touchFeature(
    const ZStack &stack, const vector<double> &c1, const vector<double> &c2,
    const ZStack &mask)
{
  return touchFeature(stack, channelCodeToId(c1), channelCodeToId(c2), mask);
}


vector<vector<double> > FlyEm::ZSegmentationAnalyzer::touchFeature(
    const ZStack &stack, const ZGraph &bodyGraph)
{
  vector<vector<double> > feature;
  for (size_t i = 0; i < bodyGraph.size(); ++i) {
#ifdef _DEBUG_
    cout << i << "/" << bodyGraph.size() << ": " << bodyGraph.edgeStart(i)
         << " " << bodyGraph.edgeEnd(i) << endl;
#endif

    feature.push_back(
          touchFeature(stack, bodyGraph.edgeStart(i), bodyGraph.edgeEnd(i)));
  }

  return feature;
}

vector<vector<double> > FlyEm::ZSegmentationAnalyzer::touchFeature(
    const ZStack &stack, const ZGraph &bodyGraph, const ZStack &mask)
{
  vector<vector<double> > feature;
  for (size_t i = 0; i < bodyGraph.size(); ++i) {
#ifdef _DEBUG_
    cout << i << "/" << bodyGraph.size() << ": " << bodyGraph.edgeStart(i)
         << " " << bodyGraph.edgeEnd(i) << endl;
#endif

    feature.push_back(
          touchFeature(stack, bodyGraph.edgeStart(i), bodyGraph.edgeEnd(i), mask));

#ifdef _DEBUG_2
    if (i > 1000) {
      break;
    }
#endif
  }

  return feature;
}

ZIntPairMap FlyEm::ZSegmentationAnalyzer::computeOverlap(
    ZStack *stack1, ZStack *stack2, ZStack *excluded)
{
  ZIntPairMap overlap;

  size_t volume = stack1->getVoxelNumber();

  if (excluded == NULL) {
    for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
      int id1 = channelCodeToId(stack1->color(voxelIndex));
      int id2 = channelCodeToId(stack2->color(voxelIndex));

      overlap.incPairCount(id1, id2);
    }
  } else {
    for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
      if (excluded->value(voxelIndex) == 0) {
        int id1 = channelCodeToId(stack1->color(voxelIndex));
        int id2 = channelCodeToId(stack2->color(voxelIndex));

        overlap.incPairCount(id1, id2);
      }
    }
  }

  return overlap;
}

ZIntMap FlyEm::ZSegmentationAnalyzer::computeBodySize(ZStack *stack,
                                                      ZStack *excluded)
{
  ZIntMap bodySize;

  size_t volume = stack->getVoxelNumber();

  if (excluded == NULL) {
    for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
      int bodyId = channelCodeToId(stack->color(voxelIndex));
      bodySize.incValue(bodyId);
    }
  } else {
    for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
      if (excluded->value8(voxelIndex) == 0) {
        int bodyId = channelCodeToId(stack->color(voxelIndex));
        bodySize.incValue(bodyId);
      }
    }
  }

  return bodySize;
}

map<int, double> FlyEm::ZSegmentationAnalyzer::computeAverageIntensity(
    ZStack *bodyStack, ZStack *intensityStack)
{
  map<int, double> intensity;
  ZIntMap bodySize;

  size_t volume = bodyStack->getVoxelNumber();

  for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
    int bodyId = channelCodeToId(bodyStack->color(voxelIndex));
    bodySize.incValue(bodyId);

    double value = intensityStack->value(voxelIndex);
    if (intensity.count(bodyId) == 0) {
      intensity[bodyId] = value;
    } else {
      intensity[bodyId] += value;
    }
  }

  for (map<int, double>::iterator iter = intensity.begin();
       iter != intensity.end(); ++iter) {
    iter->second /= bodySize[iter->first];
  }

  return intensity;
}

ZIntMap FlyEm::ZSegmentationAnalyzer::inferBodyCorrespondence(
    const ZIntPairMap &overlap, const ZIntMap &bodySize)
{
  ZIntMap corr;

  for (ZIntPairMap::const_iterator iter = overlap.begin();
       iter != overlap.end(); ++iter) {
#ifdef _DEBUG_2
    cout << (iter->first).first << " " << (iter->first).second
         << ": " << iter->second << endl;
#endif
    int overlapSize = iter->second * 2;

    if (bodySize.at(iter->first.first) < overlapSize) {
      corr[iter->first.first] = iter->first.second;
    }
  }

  return corr;
}

std::vector<double> FlyEm::ZSegmentationAnalyzer::computeTouchFeature(
    FlyEm::ZSegmentationBundle &bundle, int id1, int id2)
{
  ZStack *bodyStack = bundle.getBodyStack();

  int seedId[2];
  seedId[0] = id1;
  seedId[1] = id2;


  Stack *objStack[2];
  for (int i = 0; i < 2; ++i) {
    objStack[i] = Make_Stack(GREY, bodyStack->width(), bodyStack->height(), bodyStack->depth());
    Zero_Stack(objStack[i]);
    bundle.getBodyObject(seedId[i])->labelStack(objStack[i], 1);
  }

  vector<int> seedArray = bundle.getBodyBorderObject(id1, id2)->toIndexArray<int>(
        bodyStack->width(), bodyStack->height(), bodyStack->depth());

  vector<double> feature;

  for (int i = 0; i < 2; i++) {
    vector<double> newfeat = computeTouchFeature(objStack[i], seedArray);
    feature.insert(feature.end(), newfeat.begin(), newfeat.end());
  }

  Kill_Stack(objStack[0]);
  Kill_Stack(objStack[1]);

  return feature;
}

vector<double> FlyEm::ZSegmentationAnalyzer::computeRayburstTouchFeature(
    ZSegmentationBundle &bundle, int id1, int id2)
{
  ZStack *bodyStack = bundle.getBodyStack();

  int seedId[2];
  seedId[0] = id1;
  seedId[1] = id2;

  Stack *objStack[2];
  for (int i = 0; i < 2; ++i) {
    objStack[i] = Make_Stack(GREY, bodyStack->width(), bodyStack->height(), bodyStack->depth());
    Zero_Stack(objStack[i]);
    bundle.getBodyObject(seedId[i])->labelStack(objStack[i], 1);
  }

  vector<int> seedArray = bundle.getBodyBorderObject(id1, id2)->toIndexArray<int>(
        bodyStack->width(), bodyStack->height(), bodyStack->depth());

  vector<double> feature = computeRayburstTouchFeature(objStack, seedArray);

  Kill_Stack(objStack[0]);
  Kill_Stack(objStack[1]);

  return feature;
}


std::vector<double> FlyEm::ZSegmentationAnalyzer::computeBoundaryGrowFeature(
      FlyEm::ZSegmentationBundle &bundle, int id1, int id2)
{
  std::vector<double> feature;

  ZObject3d *border = bundle.getBodyBorderObject(id1, id2);

  ZObject3d *obj[2] = { NULL, NULL };
  ZObject3dArray *objArray[2] = { NULL, NULL };

  obj[0] = bundle.getBodyBoundaryObject(id1);
  obj[1] = bundle.getBodyBoundaryObject(id2);

  int growLevel = iround(sqrt(border->size() / TZ_PI));

  ZPoint vec[2];
  for (int i = 0; i < 2; ++i) {
    objArray[i] = obj[i]->growLabel(*border, growLevel);
    feature.push_back(objArray[i]->radiusVariance());
    feature.push_back(objArray[i]->angleShift());
    vec[i] = objArray[i]->averageDirection();
  }

  feature.push_back(vec[0].cosAngle(vec[1]));

  return feature;
}

double FlyEm::ZSegmentationAnalyzer::computeBorderIntensity(
    FlyEm::ZSegmentationBundle &bundle, int id1, int id2)
{
  ZStack *greyStack = bundle.getGreyScaleStack();
  ZObject3d *border = bundle.getBodyBorderObject(id1, id2);

  double intensity = border->averageIntensity(greyStack->c_stack());

  return intensity;
}

double FlyEm::ZSegmentationAnalyzer::computeBodyIntensity(
    FlyEm::ZSegmentationBundle &bundle, int id)
{
  ZStack *greyStack = bundle.getGreyScaleStack();
  ZObject3d *obj = bundle.getBodyObject(id);

  double intensity = obj->averageIntensity(greyStack->c_stack());

  return intensity;
}

std::vector<double> FlyEm::ZSegmentationAnalyzer::computeBcf(
    FlyEm::ZSegmentationBundle &bundle, int id1, int id2,
    BcfSet setName)
{
  std::vector<double> feature;

  switch (setName) {
  case BCF_BODY_GROW:
    feature = computeTouchFeature(bundle, id1, id2);
    break;
  case BCF_BODY_SIZE:
  {
    feature.resize(2);
    int s1 = bundle.getBodyObject(id1)->size();
    int s2 = bundle.getBodyObject(id2)->size();
    feature[0] = s1 + s2;
    feature[1] = abs(s1 - s2);
  }
    break;
  case BCF_BODY_INTENSITY:
  {
    feature.resize(2);
    double s1 = computeBodyIntensity(bundle, id1);
    double s2 = computeBodyIntensity(bundle, id2);
    feature[0] = s1 + s2;
    feature[1] = abs(s1 - s2);
  }
    break;
  case BCF_BOUNDARY_GROW:
    feature = computeBoundaryGrowFeature(bundle, id1, id2);
    break;
  case BCF_RAYBURST:
    feature = computeRayburstTouchFeature(bundle, id1, id2);
    break;
  case BCF_BORDER_INTENSITY:
    feature.push_back(computeBorderIntensity(bundle, id1, id2));
    break;
  case BCF_BORDER_SIZE:
    feature.push_back(bundle.getBodyBorderObject(id1, id2)->size());
    break;
  case BCF_GALA_SEGMENTATION:
    feature.push_back(bundle.getGalaProbability(id1, id2));
    break;
  case BCF_BOUNDARY_PROB:
    feature.push_back(bundle.getBoundaryProbability(id1, id2));
    break;
  case BCF_MITO_PROB:
  {
    feature.resize(2);
    double p1 = bundle.getMitoProbability(id1);
    double p2 = bundle.getMitoProbability(id2);
    feature[0] = p1 + p2;
    feature[1] = fabs(p1 - p2);
  }
    break;
  default:
    break;
  }

  return feature;
}

std::vector<double> FlyEm::ZSegmentationAnalyzer::computeBcf(
    FlyEm::ZSegmentationBundle &bundle, int id1, int id2,
    const vector<BcfSet> &setName)
{
  std::vector<double> feature;
  for (vector<BcfSet>::const_iterator iter = setName.begin();
       iter != setName.end(); ++iter) {
    std::vector<double> newfeat = computeBcf(bundle, id1, id2, *iter);
    feature.insert(feature.end(), newfeat.begin(), newfeat.end());
  }

  return feature;
}

std::vector<double> FlyEm::ZSegmentationAnalyzer::computeBcf(
    FlyEm::ZSegmentationBundle &bundle, int id1, int id2,
    const vector<string> &setName)
{
  std::vector<double> feature;
  for (vector<string>::const_iterator iter = setName.begin();
       iter != setName.end(); ++iter) {
    std::vector<double> newfeat =
        computeBcf(bundle, id1, id2, m_bcfSetNameMap[*iter]);
    feature.insert(feature.end(), newfeat.begin(), newfeat.end());
  }

  return feature;
}


std::vector<std::vector<double> >
FlyEm::ZSegmentationAnalyzer::computeBcf(
    FlyEm::ZSegmentationBundle &bundle, const string &setName)
{
  return computeBcf(bundle, m_bcfSetNameMap[setName]);
  /*
  if (setName == "Body_Grow") {
    return computeBcf(bundle, BCF_BODY_GROW);
  } else if (setName == "Body_Size") {
    return computeBcf(bundle, BCF_BODY_SIZE);
  } else if (setName == "Boundary_Grow") {
    return computeBcf(bundle, BCF_BOUNDARY_GROW);
  } else if (setName == "Rayburst") {
    return computeBcf(bundle, BCF_RAYBURST);
  } else if (setName == "Border_Intensity") {
    return computeBcf(bundle, BCF_BORDER_INTENSITY);
  } else if (setName == "Border_Size") {
    return computeBcf(bundle, BCF_BORDER_SIZE);
  } else if (setName == "Gala_Segmentation") {
    return computeBcf(bundle, BCF_GALA_SEGMENTATION);
  }
  */

  return vector<vector<double> >(0);
}
